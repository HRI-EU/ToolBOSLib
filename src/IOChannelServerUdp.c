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


IOCHANNELINTERFACE_CREATE_PLUGIN( ServerUdp );

#define IOCHANNELSERVERUDP_SOCKET_TIMEOUT          (60)
#define IOCHANNELSERVERUDP_SOCKET_LINGERTIMEOUT    (1)
#define IOCHANNELSERVERUDP_WAITCLIENTTIMEOUTSTRING "waitClientTimeout"
#define IOCHANNELSERVERUDP_SOCKET_BUFFSIZE         (16*1024)


static void *IOChannelServerUdp_new( void )
{
    return IOChannelGenericSocket_new();
}


static bool IOChannelServerUdp_init( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );

    IOChannel_valid( self );

    retVal = IOChannelGenericSocket_init( self );
    return retVal;
}


static bool IOChannelServerUdp_open( IOChannel *self, char *infoString,
                                     IOChannelMode mode,
                                     IOChannelPermissions permissions,
                                     va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelServerUdp_open(). Not valid info string to open server connection. "
                         "ServerUdp stream needs a port.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( port, "%s", infoString );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelServerUdp_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
#undef HOSTNAME_MAXLEN
#undef IPADDRESS_MAXLEN
}


static bool IOChannelServerUdp_openFromString( IOChannel *self,
                                               IOChannelReferenceValue **referenceVector )
{
#define HOSTNAME_MAXLEN    256
#define IPADDRESS_MAXLEN   128
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *port = (char *)NULL;
    char *readTimeout = (char *)NULL;
    bool retVal = false;
    int ipPort;
    long timeout = 0;
    BerkeleySocketType protocol = BERKELEYSOCKET_UDP;
    BerkeleySocket *socket;
    int maxClient = 1;
    char *broadcastPtr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    /* No any check on modes are needed... */
    if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        self->mode = IOCHANNEL_MODE_RW;
    }

    port = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PORT );

    if( !port )
    {
        ANY_LOG( 5, "Error. Port not found or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    ipPort = atoi( port );
    if( ipPort <= 0 || ipPort >= 65536 )
    {
        ANY_LOG( 0, "Bad port number was passed![%d]", ANY_LOG_ERROR, ipPort );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    readTimeout = IOChannelReferenceValue_getString( referenceVector, IOCHANNELSERVERUDP_WAITCLIENTTIMEOUTSTRING );
    if( !readTimeout )
    {
        /* No user specified client timeout, defaulting to IOCHANNELSERVERUDP_SOCKET_TIMEOUT */
        timeout = BERKELEYSOCKET_TIMEOUT_SECONDS(IOCHANNELSERVERUDP_SOCKET_TIMEOUT);
    }
    else
    {
        timeout = atol( readTimeout );
    }

    ANY_LOG( 7, "Incoming client timeout: %ld", ANY_LOG_INFO, timeout );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    socket = BerkeleySocket_new();
    ANY_REQUIRE( socket );


    /* unreachable code: BerkeleySocket_init() always returns
     *                   BERKELEYSOCKET_INIT_SUCCESS
     *
    if( BerkeleySocket_init( socket ) == false )
    {
      ANY_LOG( 0, "Unable to initilize the internal socket", ANY_LOG_ERROR );
      IOChannel_setError( self, IOCHANNELERROR_UCONCL );
      goto exitLabel;
    }
     * changed to:
     */
    BerkeleySocket_init( socket );


    broadcastPtr = IOChannelReferenceValue_getString( referenceVector, "broadcast" );

    if( broadcastPtr )
    {
        if( Any_strcasecmp( broadcastPtr, "true" ) == 0 )
        {
            BerkeleySocketServer_setBroadcast( streamPtr->socketServer, true );
        }
    }

    ANY_REQUIRE( streamPtr->socketServer );
    if( BerkeleySocketServer_connect( streamPtr->socketServer, protocol, ipPort, maxClient ) == NULL )
    {
        ANY_LOG( 0, "Unable to connect the server", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        BerkeleySocket_clear( socket );
        BerkeleySocket_delete( socket );
        goto exitLabel;
    }

    ANY_REQUIRE( streamPtr->socketServer );
    if( BerkeleySocketServer_waitClient( streamPtr->socketServer, timeout ) == false )
    {
        ANY_LOG( 5, "No incoming client.", ANY_LOG_INFO );
        BerkeleySocketServer_disconnect( streamPtr->socketServer );
        BerkeleySocket_clear( socket );
        BerkeleySocket_delete( socket );
        IOChannel_setError( self, IOCHANNELERROR_SOCKETTIMEOUT );
        goto exitLabel;
    }

    BerkeleySocketServer_acceptClient( streamPtr->socketServer, socket );
    ANY_REQUIRE( streamPtr->socketServer );
    ANY_REQUIRE( socket );

    BerkeleySocket_setDefaultTimeout( socket, timeout );

    /* set the socket lingering for preventing to loose data on socket closing */
    BerkeleySocket_setLinger( socket, true, IOCHANNELSERVERUDP_SOCKET_LINGERTIMEOUT);

    retVal = IOChannelGenericSocket_setSocket( self, socket );
    ANY_REQUIRE( streamPtr->socket );

    exitLabel:;
    return retVal;
#undef HOSTNAME_MAXLEN
#undef IPADDRESS_MAXLEN
}


static long IOChannelServerUdp_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericSocket_read( self, buffer, size );
}


static long IOChannelServerUdp_write( IOChannel *self, const void *buffer, long size )
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
        return IOChannelGenericSocket_write( self, buffer,
                                             size > IOCHANNELSERVERUDP_SOCKET_BUFFSIZE ?
                                             IOCHANNELSERVERUDP_SOCKET_BUFFSIZE : size );
    }
}


static long IOChannelServerUdp_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericSocket_write( self, ptr, nBytes );
}


static long long IOChannelServerUdp_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelServerUdp_close( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* mode close */
    if( !IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        /* ANY_LOG( 0, "Disconnectiong the socket", ANY_LOG_INFO ); */
        if( streamPtr->socket )
        {
            BerkeleySocket_disconnect( streamPtr->socket );
            retVal = IOChannelGenericSocket_unsetSocket( self );
        }

        /* finally close the server binding */
        BerkeleySocketServer_disconnect( streamPtr->socketServer );
    }
    else
    {
        retVal = true;
    }
    return retVal;
}


static void *IOChannelServerUdp_getProperty( IOChannel *self, const char *propertyName )
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
            retVal = streamPtr->socket;
        }
        IOCHANNELPROPERTY_PARSE_END( Socket )

        IOCHANNELPROPERTY_PARSE_BEGIN( SocketServer )
        {
            retVal = streamPtr->socketServer;
        }
        IOCHANNELPROPERTY_PARSE_END( SocketServer )

    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelServerUdp_setProperty( IOChannel *self, const char *propertyName, void *property )
{
    return false;
}


static void IOChannelServerUdp_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_clear( self );
}


static void IOChannelServerUdp_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_delete( self );
}


/* EOF */
