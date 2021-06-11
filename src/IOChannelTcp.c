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


#include <IOChannelGenericSocket.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( Tcp );

#define IOCHANNELTCP_SOCKET_TIMEOUT  10
#define IOCHANNELTCP_SOCKET_LINGERTIMEOUT  1


static void *IOChannelTcp_new( void )
{
    return IOChannelGenericSocket_new();
}


static bool IOChannelTcp_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericSocket_init( self );
}


static bool IOChannelTcp_open( IOChannel *self, char *infoString,
                               IOChannelMode mode,
                               IOChannelPermissions permissions, va_list varArg )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *ptr = (char *)NULL;
    bool retVal = false;
    int i = 0;
    char hostName[256];
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE_MSG( infoString, "IOChannelTcp_open(). Not a valid info string" );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelTcp_open(). Not valid info string to open the connection. "
                         "Tcp stream needs an hostname and a port.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    ptr = infoString;
    for( i = 0; *ptr; i++, ptr++ )
    {
        if( *ptr == ':' )
        {
            hostName[ i ] = '\0';
            ptr++;
            break;
        }

        hostName[ i ] = *ptr;
    }

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( host, "%s", hostName );
    IOCHANNELREFERENCEVALUE_ADDSET( port, "%s", ptr );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelTcp_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


static bool IOChannelTcp_openFromString( IOChannel *self,
                                         IOChannelReferenceValue **referenceVector )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *port = (char *)NULL;
    bool retVal = false;
    char *hostName = (char *)NULL;
    char ipAddress[128];
    int ipPort;
    BerkeleySocketType protocol = BERKELEYSOCKET_TCP;
    BerkeleySocket *socket;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    /* No any check on modes are needed... */
    if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        self->mode = IOCHANNEL_MODE_RW;
    }

    hostName = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_HOST );

    if( !hostName )
    {
        /* No hostName specified. Set the default "localhost" value. */
        hostName = "localhost";
    }

    if( BerkeleySocket_host2Addr( hostName, ipAddress, 128 ) == NULL )
    {
        ANY_LOG( 1, "Unable to resolve the hostname: %s", ANY_LOG_WARNING, hostName );
        goto outLabel;
    }

    port = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PORT );

    if( !port )
    {
        ANY_LOG( 5, "Error. Port not found or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto outLabel;
    }
    ipPort = atoi( port );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* make a connect to the requested tcp address/port */
    socket = BerkeleySocketClient_connect( streamPtr->socketClient, protocol, ipAddress, ipPort );

    if( socket != NULL )
    {
        streamPtr->socket = socket;
        BerkeleySocket_setDefaultTimeout( streamPtr->socket,
                                          BERKELEYSOCKET_TIMEOUT_SECONDS( IOCHANNELTCP_SOCKET_TIMEOUT ));

        /* set the socket lingering for preventing to loose data on socket closing */
        BerkeleySocket_setLinger( streamPtr->socket, true, IOCHANNELTCP_SOCKET_LINGERTIMEOUT );

        retVal = IOChannelGenericSocket_setSocket( self, socket );
    }
    else
    {
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        ANY_LOG( 5, "Unable to connect the socket!( Tcp Stream )", ANY_LOG_WARNING );
        retVal = false;
    }

    outLabel:
    return retVal;
}


static long IOChannelTcp_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericSocket_read( self, buffer, size );
}


static long IOChannelTcp_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    if( IOChannel_usesWriteBuffering( self ) )
    {
        return IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        return IOChannelGenericSocket_write( self, buffer, size );
    }
}


static long IOChannelTcp_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericSocket_write( self, ptr, nBytes );
}


static long long IOChannelTcp_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelTcp_close( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( IOCHANNEL_MODEIS_NOTCLOSE( self->mode ) )
    {
        return true;
    }
    else
    {
        BerkeleySocketClient_disconnect( streamPtr->socketClient );
        return IOChannelGenericSocket_unsetSocket( self );
    }
}


static void *IOChannelTcp_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            retVal = (void *)&( streamPtr->socketFd );
        }
        IOCHANNELPROPERTY_PARSE_END( Fd )

        IOCHANNELPROPERTY_PARSE_BEGIN( Socket )
        {
            retVal = BerkeleySocketClient_getSocket( streamPtr->socketClient );
        }
        IOCHANNELPROPERTY_PARSE_END( Socket )

        IOCHANNELPROPERTY_PARSE_BEGIN( SocketClient )
        {
            retVal = streamPtr->socketClient;
        }
        IOCHANNELPROPERTY_PARSE_END( SocketClient )

    }
    IOCHANNELPROPERTY_END;

    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelTcp_setProperty( IOChannel *self, const char *propertyName, void *property )
{
    return false;
}


static void IOChannelTcp_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_clear( self );
}


static void IOChannelTcp_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_delete( self );
}


/* EOF */
