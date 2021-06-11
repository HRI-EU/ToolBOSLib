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


#include <ctype.h>
#include <IOChannelGenericSocket.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( ServerTcp );

#define IOCHANNELSERVERTCP_SOCKET_TIMEOUT  60
#define IOCHANNELSERVERTCP_SOCKET_LINGERTIMEOUT  1
#define IOCHANNELSERVERTCP_WAITCLIENTTIMEOUTSTRING "waitClientTimeout"
#define IOCHANNELSERVERTCP_REUSEADDRSTRING "reuseAddr"
#define IOCHANNELSERVERTCP_LINGERTIMEOUT   "lingerTimeout"


static void *IOChannelServerTcp_new( void )
{
    return IOChannelGenericSocket_new();
}


static bool IOChannelServerTcp_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericSocket_init( self );
}


static bool IOChannelServerTcp_open( IOChannel *self, char *infoString,
                                     IOChannelMode mode,
                                     IOChannelPermissions permissions,
                                     va_list varArg )
{
#define HOSTNAME_MAXLEN    256
#define IPADDRESS_MAXLEN   128

    unsigned int i = 0;
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );


    /* If someone by mistake passes an invalid infoString like
     * "ServerTcp://localhost:80" instead of ""ServerTcp://80",
     * the atoi() fails and will try to bind to port number 0.
     *
     * Therefore check if the infoString contains digits only.
     */
    for( i = 0; i < Any_strlen( infoString ); i++ )
    {
        if( isdigit( infoString[ i ] ) == 0 )
        {
            ANY_LOG( 0, "invalid infoString for ServerTcp channel", ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_BIST );
            goto outLabel;
        }
    }

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelServerTcp_open(). Not valid info string to open server connection. "
                         "ServerTcp stream needs a port.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( port, "%s", infoString );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelServerTcp_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
#undef HOSTNAME_MAXLEN
#undef IPADDRESS_MAXLEN
}


static bool IOChannelServerTcp_openFromString( IOChannel *self,
                                               IOChannelReferenceValue **referenceVector )
{
#define HOSTNAME_MAXLEN    256
#define IPADDRESS_MAXLEN   128
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    char *port = (char *)NULL;
    char *readTimeout = (char *)NULL;
    char *readLingerTimeout = (char *)NULL;
    bool retVal = false;
    int ipPort = 0;
    long timeout = 0;
    BerkeleySocketType protocol = BERKELEYSOCKET_TCP;
    BerkeleySocket *socket;
    int maxClient = 1;
    int reuseAddr = 0;
    int lingerTimeout = 0;
    bool lingerTimeoutProvided = false;

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
        ANY_LOG( 5, "Error. Port not found.", ANY_LOG_ERROR );
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

    readTimeout = IOChannelReferenceValue_getString( referenceVector, IOCHANNELSERVERTCP_WAITCLIENTTIMEOUTSTRING );
    if( !readTimeout )
    {
        /* No user specified client timeout, defaulting to IOCHANNELSERVERTCP_SOCKET_TIMEOUT */
        timeout = BERKELEYSOCKET_TIMEOUT_SECONDS( IOCHANNELSERVERTCP_SOCKET_TIMEOUT );
    }
    else
    {
        timeout = atol( readTimeout );
    }

    readLingerTimeout = IOChannelReferenceValue_getString( referenceVector, IOCHANNELSERVERTCP_LINGERTIMEOUT );
    if( !readLingerTimeout )
    {
        /* No user specified socket linger timeout, defaulting to IOCHANNELSERVERTCP_SOCKET_LINGERTIMEOUT */
        lingerTimeout = IOCHANNELSERVERTCP_SOCKET_LINGERTIMEOUT;
    }
    else
    {
        lingerTimeout = atol( readLingerTimeout );
        lingerTimeoutProvided = true;
    }

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    reuseAddr = IOChannelReferenceValue_getInt( referenceVector, IOCHANNELSERVERTCP_REUSEADDRSTRING );
    if( reuseAddr )
    {
        BerkeleySocket *s = BerkeleySocketServer_getSocket( streamPtr->socketServer );
        ANY_REQUIRE( s );

        ANY_LOG( 1, "Setting SO_REUSEADDR on ServerSocket", ANY_LOG_INFO );

        BerkeleySocket_setReuseAddr( s, true );
    }

    ANY_LOG( 7, "Incoming client timeout: %ld", ANY_LOG_INFO, timeout );

    socket = BerkeleySocket_new();
    ANY_REQUIRE( socket );

    /* unreachable code: BerkeleySocket_init() always returns
     *                   BERKELEYSOCKET_INIT_SUCCESS
     *
    if( BerkeleySocket_init( socket ) == false )
    {
      ANY_LOG( 0, "Unable to initialize the internal socket.", ANY_LOG_ERROR );
      IOChannel_setError( self, IOCHANNELERROR_UCONCL );
      goto exitLabel;
    }
     * changed to:
     */
    BerkeleySocket_init( socket );


    ANY_REQUIRE( streamPtr->socketServer );
    if( BerkeleySocketServer_connect( streamPtr->socketServer, protocol, ipPort, maxClient ) == NULL )
    {
        ANY_LOG( 0, "Unable to connect the server.", ANY_LOG_ERROR );
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

    /* set the socket lingering as per socket option */
    if ( lingerTimeoutProvided )
    {
        BerkeleySocket *serverSocket = BerkeleySocketServer_getSocket( streamPtr->socketServer );
        BerkeleySocket_setLinger( serverSocket, true, lingerTimeout);
        BerkeleySocket_setLinger( socket, true, lingerTimeout );
    }

    retVal = IOChannelGenericSocket_setSocket( self, socket );
    ANY_REQUIRE( streamPtr->socket );

    exitLabel:;
    return retVal;
#undef HOSTNAME_MAXLEN
#undef IPADDRESS_MAXLEN
}


static long IOChannelServerTcp_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericSocket_read( self, buffer, size );
}


static long IOChannelServerTcp_write( IOChannel *self, const void *buffer, long size )
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


static long IOChannelServerTcp_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericSocket_write( self, ptr, nBytes );
}


static long long IOChannelServerTcp_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelServerTcp_close( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* default and mode close */
    if( !IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        /*ANY_LOG( 0, "Disconnectiong the socket", ANY_LOG_INFO );*/
        /* closes the client connection if any */
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


static void *IOChannelServerTcp_getProperty( IOChannel *self, const char *propertyName )
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


static bool IOChannelServerTcp_setProperty( IOChannel *self, const char *propertyName, void *property )
{
    return false;
}


static void IOChannelServerTcp_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_clear( self );
}


static void IOChannelServerTcp_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_delete( self );
}


/* EOF */
