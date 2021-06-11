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


IOCHANNELINTERFACE_CREATE_PLUGIN( Udp );

#define IOCHANNELUDP_SOCKET_TIMEOUT        (10)

#define IOCHANNELUDP_SOCKET_LINGERTIMEOUT  (1)

#define IOCHANNELUDP_SOCKET_BUFFSIZE       (16*1024)


static void *IOChannelUdp_new( void )
{
    return IOChannelGenericSocket_new();
}


static bool IOChannelUdp_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericSocket_init( self );
}


static bool IOChannelUdp_open( IOChannel *self, char *infoString,
                               IOChannelMode mode,
                               IOChannelPermissions permissions,
                               va_list varArg )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *ptr = (char *)NULL;
    bool retVal = false;
    int i = 0;
    char hostName[256];
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE_MSG( infoString, "IOChannelUdp_open(). Not a valid "
            "info string to open the connection" );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelUdp_open(). Not valid info string to open the connection. "
                         "Udp stream needs an hostname and a port.",
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
    IOCHANNELREFERENCEVALUE_ADDSET( srcport, "%d", 0 );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelUdp_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


static bool IOChannelUdp_openFromString( IOChannel *self,
                                         IOChannelReferenceValue **referenceVector )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *port = NULL;
    bool retVal = false;
    char *hostName = NULL;
    char ipAddress[128];
    int ipPort;
    int srcPortNo = 0;
    char *broadcastPtr = NULL;
    BerkeleySocketType protocol = BERKELEYSOCKET_UDP;
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
    if( ipPort <= 0 || ipPort >= 65536 )
    {
        ANY_LOG( 0, "Bad port number was passed![%d]", ANY_LOG_ERROR, ipPort );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto outLabel;
    }

    srcPortNo = IOChannelReferenceValue_getInt( referenceVector, IOCHANNELREFERENCEVALUE_SRCPORT );

    if( srcPortNo < 0 || srcPortNo >= 65536 )
    {
        ANY_LOG( 0, "Bad src port number was passed![%d]", ANY_LOG_ERROR, srcPortNo );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto outLabel;
    }

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    broadcastPtr = IOChannelReferenceValue_getString( referenceVector, "broadcast" );

    if( broadcastPtr )
    {
        if( Any_strcasecmp( broadcastPtr, "true" ) == 0 )
        {
            BerkeleySocketClient_setBroadcast( streamPtr->socketClient, true );
        }
    }

    /* make a connect to the requested udp address/port */
    socket = BerkeleySocketClient_connectEx( streamPtr->socketClient, protocol, ipAddress, ipPort, srcPortNo );

    if( socket != NULL )
    {
        streamPtr->socket = socket;
        BerkeleySocket_setDefaultTimeout( streamPtr->socket,
                                          BERKELEYSOCKET_TIMEOUT_SECONDS(IOCHANNELUDP_SOCKET_TIMEOUT));

        /* set the socket lingering for preventing to loose data on socket closing */
        BerkeleySocket_setLinger( streamPtr->socket, true, IOCHANNELUDP_SOCKET_LINGERTIMEOUT);

        retVal = IOChannelGenericSocket_setSocket( self, socket );
    }
    else
    {
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        ANY_LOG( 5, "Unable to connect the socket!( Udp Stream )", ANY_LOG_WARNING );
    }

    outLabel:
    return retVal;
}


static long IOChannelUdp_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    /* Read up to IOCHANNELUDP_SOCKET_BUFFSIZE bytes */
    return IOChannelGenericSocket_read( self, buffer,
                                        size > IOCHANNELUDP_SOCKET_BUFFSIZE ?
                                        IOCHANNELUDP_SOCKET_BUFFSIZE : size );
}


static long IOChannelUdp_write( IOChannel *self, const void *buffer, long size )
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
        /* Write up to IOCHANNELUDP_SOCKET_BUFFSIZE bytes to prevent Udp buffer problems */
        return IOChannelGenericSocket_write( self, buffer,
                                             size > IOCHANNELUDP_SOCKET_BUFFSIZE ?
                                             IOCHANNELUDP_SOCKET_BUFFSIZE : size );
    }
}


static long IOChannelUdp_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;
    long leftBytes = 0;
    long wrOnSocket = 0;
    long toWrOnSocket = 0;

    ANY_REQUIRE( self );

    leftBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    /* Flush the buffer writing blocks of IOCHANNELUDP_SOCKET_BUFFER size */
    while( wrOnSocket < leftBytes )
    {
        toWrOnSocket = (IOCHANNELUDP_SOCKET_BUFFSIZE < leftBytes ) ?
                       IOCHANNELUDP_SOCKET_BUFFSIZE : leftBytes;
        if( IOChannel_isWritePossible( self ))
        {
            nBytes = IOChannelGenericSocket_write( self,
                                                   (unsigned char *)ptr + wrOnSocket,
                                                   toWrOnSocket );
        }
        if( nBytes == -1 )
        {
            break;
        }
        wrOnSocket += nBytes;
        leftBytes -= nBytes;
    }

    return wrOnSocket;
}


static long long IOChannelUdp_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelUdp_close( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        return true;
    }
    else
    {
        BerkeleySocketClient_disconnect( streamPtr->socketClient );
        return IOChannelGenericSocket_unsetSocket( self );
    }
}


static void *IOChannelUdp_getProperty( IOChannel *self, const char *propertyName )
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


static bool IOChannelUdp_setProperty( IOChannel *self, const char *propertyName, void *property )
{
    return false;
}


static void IOChannelUdp_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_clear( self );
}


static void IOChannelUdp_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_delete( self );
}


/* EOF */
