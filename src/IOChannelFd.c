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


#if defined(__windows__)
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#endif

#include <IOChannelGenericFd.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( Fd );


static void *IOChannelFd_new( void )
{
    return IOChannelGenericFd_new();
}


static bool IOChannelFd_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericFd_init( self );
}


static bool IOChannelFd_open( IOChannel *self, char *infoString,
                              IOChannelMode mode,
                              IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    int fd = -1;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( infoString );

    IOCHANNEL_GET_ARGUMENT( fd, int, varArg );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( key, "%d", fd );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelFd_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    return retVal;
}


static bool IOChannelFd_openFromString( IOChannel *self,
                                        IOChannelReferenceValue **referenceVector )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    bool retVal = false;
    int fd = -1;
    char *permissions = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    if( IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        if( IOCHANNEL_MODEIS_CREAT( self->mode ) || IOCHANNEL_MODEIS_APPEND( self->mode ))
        {
            IOChannel_setError( self, IOCHANNELERROR_BMODE );
            goto outLabel;
        }
    }
    else
    {
        ANY_LOG( 5, "Error. Access mode not specified.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto outLabel;
    }

    permissions = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PERM );
    if( !permissions )
    {
        ANY_LOG( 5, "No access permissions were specified for this stream.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto outLabel;
    }

    fd = IOChannelReferenceValue_getInt( referenceVector, IOCHANNELREFERENCEVALUE_KEY );
    ANY_REQUIRE_MSG(( fd != 0 ),
                    "Not valid fd parameter( or not present on IOChannel_open() )" );
    retVal = IOChannelGenericFd_setFd( self, fd );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( streamPtr->isRegularFile )
    {
        if( IOCHANNEL_MODEIS_TRUNC( self->mode ) && retVal )
        {
            retVal = IOChannelGenericFd_truncate( self, 0 );
        }
        else
        {
            /* Setting stream position */
            long long offset = -1;
#if !defined(__windows__)
            offset = (long long)lseek( streamPtr->fd, 0, SEEK_CUR );
#else
            offset = (long long) _lseeki64( streamPtr->fd, 0LL, SEEK_CUR );
#endif
            if( offset == -1 )
            {
                ANY_LOG( 5, "IOChannelFd. Unable to align regular file fd offset with stream position",
                         ANY_LOG_ERROR );
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
                retVal = false;
            }
            else
            {
                self->currentIndexPosition = offset;
            }
        }
    }

    outLabel:
    return retVal;
}


static long IOChannelFd_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericFd_read( self, buffer, size );
}


static long IOChannelFd_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    if( IOChannel_usesWriteBuffering( self ))
    {
        return IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        return IOChannelGenericFd_write( self, buffer, size );
    }
}


static long IOChannelFd_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericFd_write( self, ptr, nBytes );
}


static long long IOChannelFd_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    ANY_REQUIRE( self );

    return IOChannelGenericFd_seek( self, offset, whence );
}


static bool IOChannelFd_close( IOChannel *self )
{
    ANY_REQUIRE( self );

    if( IOCHANNEL_MODEIS_CLOSE( self->mode ))
    {
        return IOChannelGenericFd_close( self );
    }
    else
    {
        return IOChannelGenericFd_unSet( self );
    }
}


static void *IOChannelFd_getProperty( IOChannel *self, const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            retVal = (void *)IOChannelGenericFd_getFdPtr( self );
        }
        IOCHANNELPROPERTY_PARSE_END( Fd )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelFd_setProperty( IOChannel *self, const char *propertyName,
                                     void *propertyValue )
{
    return false;
}


static void IOChannelFd_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_clear( self );
}


static void IOChannelFd_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_delete( self );
}


/* EOF */
