/*
 *  Copyright (c) Honda Research Institute Europe GmbH
 *
 *  This file is part of ToolBOSLib.
 *
 *  ToolBOSLib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ToolBOSLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ToolBOSLib. If not, see <http://www.gnu.org/licenses/>.
 */


#include <IOChannelGenericAnsiFILE.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( AnsiFILE );


static long long IOChannelAnsiFILE_seekBack( IOChannel *self, long long offset );

static long long IOChannelAnsiFILE_seekForward( IOChannel *self, long long offset );


static void *IOChannelAnsiFILE_new( void )
{
    return IOChannelGenericAnsiFILE_new();
}


static bool IOChannelAnsiFILE_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericAnsiFILE_init( self );
}


static bool IOChannelAnsiFILE_open( IOChannel *self, char *infoString,
                                    IOChannelMode mode,
                                    IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    FILE *fp = NULL;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( infoString );

    IOCHANNEL_GET_ARGUMENT( fp, FILE * , varArg );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( pointer, "%p", (void *)fp );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelAnsiFILE_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );


    return retVal;
}


static bool IOChannelAnsiFILE_openFromString( IOChannel *self,
                                              IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;
    long offset = -1;
    FILE *fp = NULL;
    char *permissions = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    fp = (FILE *)IOChannelReferenceValue_getPtr( referenceVector, (char *)IOCHANNELREFERENCEVALUE_POINTER );

    if( fp != NULL )
    {
        if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
        {
            ANY_LOG( 5, "Error. Access mode not specified.", ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            goto outLabel;
        }

        permissions = IOChannelReferenceValue_getString( referenceVector, (char *)IOCHANNELREFERENCEVALUE_PERM );
        if( !permissions )
        {
            ANY_LOG( 5, "No access permissions were specified for this stream", ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            goto outLabel;
        }

        offset = ftell( fp );
        if( offset == -1 )
        {
            ANY_LOG( 5, "Unable to align regular FILE offset with stream position",
                     ANY_LOG_ERROR );
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            goto outLabel;
        }
        else
        {
            self->currentIndexPosition = offset;
            IOChannel_setType( self, IOCHANNELTYPE_ANSIFILE );

            IOChannelGenericAnsiFILE_setFp( self, (void *)fp );

            retVal = true;
        }
    }
    else
    {
        IOChannel_setError( self, IOCHANNELERROR_BOARG );
        ANY_LOG( 5, "Not valid FILE pointer argument was passed to IOChannel_open",
                 ANY_LOG_ERROR );
    }

    outLabel:;
    return retVal;
}


static long IOChannelAnsiFILE_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericAnsiFILE_read( self, buffer, size );
}


static long IOChannelAnsiFILE_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericAnsiFILE_write( self, buffer, size );
}


static long IOChannelAnsiFILE_flush( IOChannel *self )
{
    ANY_REQUIRE( self );

    return IOChannelGenericAnsiFILE_flush( self );
}


static long long IOChannelAnsiFILE_seek( IOChannel *self,
                                         long long offset,
                                         IOChannelWhence whence )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;

    long long newOffset = 0;
    long long retVal = -1;

    ANY_REQUIRE( self );

    ungetBuffer = self->ungetBuffer;

    switch( whence )
    {
        case IOCHANNELWHENCE_SET:
        case IOCHANNELWHENCE_END:
        {
            if( IOChannelGenericAnsiFILE_seek( self, offset, whence ) == 0 )
            {
                newOffset = IOChannelGenericAnsiFILE_tell( self );
                if( newOffset != -1 )
                {
                    ungetBuffer->index = 0;
                    self->currentIndexPosition = newOffset;
                    retVal = self->currentIndexPosition;
                    break;
                }
            }
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            retVal = -1;
            break;
        }
        case IOCHANNELWHENCE_CUR:
        {
            if( offset == 0 )
            {
                retVal = self->currentIndexPosition -
                         ungetBuffer->index;
            }
            else if( offset < 0 )
            {
                retVal = IOChannelAnsiFILE_seekBack( self, offset );
            }
            else
            {
                retVal = IOChannelAnsiFILE_seekForward( self, offset );
            }
            break;
        }
        default:
            IOChannel_setError( self, IOCHANNELERROR_BWHESEK );
            retVal = -1;
            break;
    }
    return retVal;
}


static bool IOChannelAnsiFILE_close( IOChannel *self )
{
    void *fp = (void *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );

    if( IOCHANNEL_MODEIS_CLOSE( self->mode ))
    {
        fp = IOChannelGenericAnsiFILE_getFp( self );

        if( fclose( fp ) == EOF )
        {
            ANY_LOG( 5, "unable to close the FILE stream", ANY_LOG_WARNING );
            retVal = false;
        }
        else
        {
            retVal = true;
        }
    }
    else
    {
        IOChannelGenericAnsiFILE_setFp( self, (void *)NULL );
        retVal = true;
    }

    return retVal;
}


static void *IOChannelAnsiFILE_getProperty( IOChannel *self, const char *propertyName )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return IOChannelGenericAnsiFILE_getProperty( self, propertyName );
}


static bool IOChannelAnsiFILE_setProperty( IOChannel *self, const char *propertyName,
                                           void *property )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return IOChannelGenericAnsiFILE_setProperty( self, propertyName, property );
}


void IOChannelAnsiFILE_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericAnsiFILE_clear( self );
}


void IOChannelAnsiFILE_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericAnsiFILE_delete( self );
}


static long long IOChannelAnsiFILE_seekBack( IOChannel *self, long long offset )
{
    IOChannelBuffer *ungetBuffer = self->ungetBuffer;
    long long newOffset = 0;
    long long retVal = -1;

    ANY_REQUIRE( self );

    if( -offset <= self->rdBytesFromLastUnget )
    {
        ungetBuffer->index += -offset;
        retVal = self->currentIndexPosition - ungetBuffer->index;
    }
    else
    {
        newOffset = offset - ungetBuffer->index;
        if( newOffset == 0 )
        {
            retVal = self->currentIndexPosition;
        }
        else
        {

            if( IOChannelGenericAnsiFILE_seek( self, newOffset, SEEK_CUR ) == 0 )
            {
                retVal = IOChannelGenericAnsiFILE_tell( self );
                if( retVal != -1 )
                {
                    self->currentIndexPosition = retVal;
                    ungetBuffer->index = 0;
                }
                else
                {
                    IOCHANNEL_SETSYSERRORFROMERRNO( self );
                    retVal = -1;
                }
            }
            else
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
                retVal = -1;
            }
        }
    }
    return retVal;
}


static long long IOChannelAnsiFILE_seekForward( IOChannel *self, long long offset )
{
    IOChannelBuffer *ungetBuffer = self->ungetBuffer;
    long long newOffset = 0;
    long long retVal = -1;

    ANY_REQUIRE( self );

    if( offset < ungetBuffer->index )
    {
        ungetBuffer->index -= offset;
        retVal = self->currentIndexPosition - ungetBuffer->index;
    }
    else
    {
        newOffset = offset - ungetBuffer->index;

        if( IOChannelGenericAnsiFILE_seek( self, newOffset, SEEK_CUR ) == 0 )
        {
            self->currentIndexPosition += newOffset;
            retVal = self->currentIndexPosition;
        }
        else
        {
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            retVal = -1;
        }
    }
    return retVal;
}


/* EOF */
