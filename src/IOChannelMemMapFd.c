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


#if !defined(__windows__)

#include <unistd.h>

#endif

#include <IOChannelGenericMem.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( MemMapFd );


static void *IOChannelMemMapFd_new()
{
    return IOChannelGenericMem_new();
}


static bool IOChannelMemMapFd_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericMem_init( self );
}


static bool IOChannelMemMapFd_open( IOChannel *self, char *infoString,
                                    IOChannelMode mode,
                                    IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    int fd = 0;
    long size = 0;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( infoString );

    IOCHANNEL_GET_ARGUMENT( fd, int, varArg );
    IOCHANNEL_GET_ARGUMENT( size, long, varArg );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( key, "%d", fd );
    IOCHANNELREFERENCEVALUE_ADDSET( size, "%ld", size );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelMemMapFd_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    return retVal;
}


static bool IOChannelMemMapFd_openFromString( IOChannel *self,
                                              IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;
    int fd = 0;
    long size = 0;
    char *permissions = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    /* Getting Fd arg */
    fd = IOChannelReferenceValue_getInt( referenceVector, IOCHANNELREFERENCEVALUE_KEY );

    if( fd < 0 )
    {
        IOChannel_setError( self, IOCHANNELERROR_EBADF );
        goto outLabel;
    }
    /* Getting length arg */
    size = IOChannelReferenceValue_getLong( referenceVector, IOCHANNELREFERENCEVALUE_SIZE );

    if( size <= 0 )
    {
        IOChannel_setError( self, IOCHANNELERROR_BMMPSIZE );
        goto outLabel;
    }

    if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        ANY_LOG( 5, "Error. Access mode not specified.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto outLabel;
    }

    permissions = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PERM );
    if( !permissions )
    {
        ANY_LOG( 5, "No access permissions were specified for this stream", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto outLabel;
    }

    retVal = IOChannelGenericMem_mapFd( self, fd, size );

    outLabel:
    return retVal;
}


static long IOChannelMemMapFd_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericMem_read( self, buffer, size );
}


static long IOChannelMemMapFd_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericMem_write( self, buffer, size );
}


static long IOChannelMemMapFd_flush( IOChannel *self )
{
    ANY_REQUIRE( self );

    return IOChannelGenericMem_flush( self );
}


static long long IOChannelMemMapFd_seek( IOChannel *self,
                                         long long offset,
                                         IOChannelWhence whence )
{
    ANY_REQUIRE( self );

    return IOChannelGenericMem_seek( self, offset, whence );
}


static bool IOChannelMemMapFd_close( IOChannel *self )
{
    ANY_REQUIRE( self );

    /* Close and None */
    if( !IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        return IOChannelGenericMem_unmapFd( self );
    }
    else
    {
        return true;
    }
}


static void *IOChannelMemMapFd_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );

    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( MemPointer )
        {
            retVal = streamPtr->ptr;
        }
        IOCHANNELPROPERTY_PARSE_END( MemPointer )
    }
    IOCHANNELPROPERTY_END;

    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelMemMapFd_setProperty( IOChannel *self, const char *propertyName,
                                           void *property )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return false;
}


static void IOChannelMemMapFd_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericMem_clear( self );
}


static void IOChannelMemMapFd_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericMem_delete( self );
}


/* EOF */
