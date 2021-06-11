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


/* some API parameters unused but kept for polymorphism */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <stdio.h>
#include <IOChannelGenericAnsiFILE.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( PipeCmd );


static void *IOChannelPipeCmd_new( void )
{
    return IOChannelGenericAnsiFILE_new();
}


static bool IOChannelPipeCmd_init( IOChannel *self )
{
    ANY_REQUIRE( self );
    IOChannel_valid( self );

    return IOChannelGenericAnsiFILE_init( self );
}


static bool IOChannelPipeCmd_open( IOChannel *self, char *infoString,
                                   IOChannelMode mode,
                                   IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelPipeCmd_open(). Not valid info string."
                         "PipeCmd stream needs a name that specifies a valid command.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( name, "%s", infoString );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelPipeCmd_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


static bool IOChannelPipeCmd_openFromString( IOChannel *self,
                                             IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The popen() is not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    FILE *fp = NULL;
    char *command = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    command = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_NAME );

    if( !command )
    {
        ANY_LOG( 5, "Error. Name for PipeCmd not found in openString or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto exitLabel;
    }

    if( IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        if( IOCHANNEL_MODEIS_R_ONLY( self->mode ))
        {
            fp = popen( command, "r" );
        }
        else if( IOCHANNEL_MODEIS_W_ONLY( self->mode ))
        {
            fp = popen( command, "w" );
        }
        else
        {
            ANY_LOG( 0, "Bad Mode was passed to \"PipeCmd://\" stream: "
                             "You Can use Only "
                             "IOCHANNEL_MODEIS_R_ONLY or IOCHANNEL_MODEIS_W_ONLY!",
                     ANY_LOG_ERROR );

            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            goto exitLabel;
        }
    }
    else
    {
        ANY_LOG( 5, "Error. Access mode not specified.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto exitLabel;
    }

    if( fp == NULL )
    {
        IOChannel_setError( self, IOCHANNELERROR_BOARG );
        ANY_LOG( 5, "IOChannelPipeCmd_open(). Not valid FILE pointer argument was passed to IOChannel_openFromString",
                 ANY_LOG_ERROR );
        goto exitLabel;
    }

    IOChannel_setType( self, IOCHANNELTYPE_ANSIFILE );

    IOChannelGenericAnsiFILE_setFp( self, (void *)fp );

    retVal = true;

    exitLabel:;

#endif
    return retVal;
}


static long IOChannelPipeCmd_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericAnsiFILE_read( self, buffer, size );
}


static long IOChannelPipeCmd_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericAnsiFILE_write( self, buffer, size );
}


static long IOChannelPipeCmd_flush( IOChannel *self )
{
    ANY_REQUIRE( self );

    return IOChannelGenericAnsiFILE_flush( self );
}


static long long IOChannelPipeCmd_seek( IOChannel *self,
                                        long long offset,
                                        IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelPipeCmd_close( IOChannel *self )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The pclose() is not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    ANY_REQUIRE( self );

    /* Default and Close */
    if( !IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        void *fp = IOChannelGenericAnsiFILE_getFp( self );
        ANY_REQUIRE( fp );

        if( pclose( fp ) == EOF )
        {
            ANY_LOG( 5, "IOChannelPipeCmd_close: "
                    "unable to close the FILE stream", ANY_LOG_WARNING );
            retVal = false;
        }
        else
        {
            /* Not Close */
            retVal = true;
        }
    }
    else
    {
        IOChannelGenericAnsiFILE_setFp( self, (void *)NULL );
        retVal = true;
    }

#endif

    return retVal;
}


static void *IOChannelPipeCmd_getProperty( IOChannel *self, const char *propertyName )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return IOChannelGenericAnsiFILE_getProperty( self, propertyName );
}


static bool IOChannelPipeCmd_setProperty( IOChannel *self, const char *propertyName,
                                          void *property )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return IOChannelGenericAnsiFILE_setProperty( self, propertyName, property );
}


static void IOChannelPipeCmd_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericAnsiFILE_clear( self );
}


static void IOChannelPipeCmd_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericAnsiFILE_delete( self );
}


/* EOF */
