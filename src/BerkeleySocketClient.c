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


#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#if defined(__linux__)
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#endif


#include <Any.h>
#include <BerkeleySocket.h>
#include <BerkeleySocketClient.h>


#define BERKELEYSOCKETCLIENT_VALID  0x0232f9b0
#define BERKELEYSOCKETCLIENT_INVALID  0xc9647f03


static int BerkeleySocketClient_initTcpClient( BerkeleySocketClient *self, char *serverHostAddr, int serverPortNo );

static int BerkeleySocketClient_initUdpClient( BerkeleySocketClient *self, char *serverHostAddr, int serverPortNo,
                                               int srcPortNo );


BerkeleySocketClient *BerkeleySocketClient_new( void )
{
    BerkeleySocketClient *self = ANY_TALLOC( BerkeleySocketClient );

    return self;
}


bool BerkeleySocketClient_init( BerkeleySocketClient *self, BerkeleySocket *sock )
{
    bool result = false;

    ANY_REQUIRE( self );

    self->valid = BERKELEYSOCKETCLIENT_INVALID;

    /* if we doesn't specify our socket than we create a new one */
    if( sock == (BerkeleySocket *)NULL)
    {
        self->socket = BerkeleySocket_new();
        if( self->socket == (BerkeleySocket *)NULL)
        {
            goto out;
        }

        BerkeleySocket_init( self->socket );
        self->created = true;
    }
    else
    {
        self->socket = sock;
        self->created = false;
    }

    self->valid = BERKELEYSOCKETCLIENT_VALID;
    result = true;

    out:

    return result;
}


void BerkeleySocketClient_setBroadcast( BerkeleySocketClient *self, bool broadcast )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );

    ANY_LOG( 5, "Setting broadcast to '%s'", ANY_LOG_INFO, broadcast ? "true" : "false" );
    self->broadcast = broadcast;
}


BerkeleySocket *BerkeleySocketClient_getSocket( BerkeleySocketClient *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );

    return ( self->socket );
}


BerkeleySocket *BerkeleySocketClient_connect( BerkeleySocketClient *self, BerkeleySocketType type, char *serverIp,
                                              int portNo )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );
    ANY_REQUIRE( serverIp );
    ANY_REQUIRE( portNo > 0 );
    ANY_REQUIRE( self->socket );

    /* in the standard _connect() source port is choosen by the O.S. */
    return BerkeleySocketClient_connectEx( self, type, serverIp, portNo, 0 );
}


BerkeleySocket *BerkeleySocketClient_connectEx( BerkeleySocketClient *self, BerkeleySocketType type, char *serverIp,
                                                int portNo, int srcPortNo )
{
    BerkeleySocket *result = (BerkeleySocket *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );
    ANY_REQUIRE( serverIp );
    ANY_REQUIRE( portNo > 0 );
    ANY_REQUIRE( srcPortNo >= 0 );
    ANY_REQUIRE( self->socket );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_NULL );

    switch( type )
    {
        case BERKELEYSOCKET_TCP:
            BerkeleySocketClient_initTcpClient( self, serverIp, portNo );
            break;

        case BERKELEYSOCKET_UDP:
            BerkeleySocketClient_initUdpClient( self, serverIp, portNo, srcPortNo );
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, type );
            break;
    }

    if( BerkeleySocket_getFd( self->socket ) != BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_setOptions( self->socket );
        result = self->socket;
    }

    return result;
}


void BerkeleySocketClient_disconnect( BerkeleySocketClient *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );

    if( BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL )
    {
        BerkeleySocket_disconnect( self->socket );
    }
    // else was disconnected before
}


void BerkeleySocketClient_clear( BerkeleySocketClient *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );

    BerkeleySocket_disconnect( self->socket );

    if( self->created )
    {
        BerkeleySocket_clear( self->socket );
        BerkeleySocket_delete( self->socket );
        self->socket = (BerkeleySocket *)NULL;
        self->created = false;
    }

    self->valid = BERKELEYSOCKETCLIENT_INVALID;
}


void BerkeleySocketClient_delete( BerkeleySocketClient *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static int BerkeleySocketClient_initUdpClient( BerkeleySocketClient *self, char *serverHostAddr, int serverPortNo,
                                               int srcPortNo )
{
    BerkeleySocket *sock = (BerkeleySocket *)NULL;
    int retVal = -1;
    BerkeleySocketHandle mySockFd = BERKELEYSOCKETHANDLE_INVALID;
    int rVal = 0;
    char s[512];
    char *ipAddr;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );
    ANY_REQUIRE( serverPortNo > 0 );
    ANY_REQUIRE( srcPortNo >= 0 );
    ANY_REQUIRE( serverHostAddr );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_NULL );

    /* take the client socket */
    sock = self->socket;

    ipAddr = BerkeleySocket_host2Addr( serverHostAddr, s, 512 );

    if( ipAddr == (char *)NULL)
    {
        ANY_LOG( 5, "Cannot resolve hostname '%s'", ANY_LOG_WARNING, serverHostAddr );
        goto out;
    }

    /* create a UDP socket */
    mySockFd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if( mySockFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 0, "Can't open datagram socket, error: '%s'", ANY_LOG_ERROR, s );
        goto out;
    }

    sock->socketFd = mySockFd;
    sock->type = BERKELEYSOCKET_UDP;

    if( self->broadcast )
    {
        BerkeleySocket_setBroadcast( sock, true, serverPortNo );
    }
    else
    {
        /*
         * in order to emulate the TCP in a connectionless environment as UDP,
         * we have to specify that we want to receive the UDP datagram from
         * any interface in a port assigned by the O.S. by the bind() function
         * call. In this way the port will remain fixed for the entire session and
         * when, finally, we call connect() to connect the socket on the server side,
         * we gain the possibility to use the standard function call write() and read()
         * instead of sendto() & recvfrom()
         */
        Any_memset( &sock->sourceAddr, 0, sizeof( sock->sourceAddr ));

        sock->sourceAddr.sin_family = AF_INET;
        sock->sourceAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock->sourceAddr.sin_port = htons( srcPortNo );

        rVal = bind( mySockFd, (struct sockaddr *)&sock->sourceAddr, sizeof( sock->sourceAddr ));

        if( rVal == BERKELEYSOCKET_ERROR )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't open datagram socket, error: '%s'", ANY_LOG_ERROR, s );
#if !defined(__msvc__) && !defined(__windows__)
            close( mySockFd );
#else
            closesocket( mySockFd );
#endif
            goto out;
        }

        /* now we connect on the remote side */
        Any_memset( &sock->remoteAddr, 0, sizeof( sock->remoteAddr ));

        sock->remoteAddr.sin_family = AF_INET;
        sock->remoteAddr.sin_addr.s_addr = inet_addr( serverHostAddr );
        sock->remoteAddr.sin_port = htons( serverPortNo );

        /* check the remote address if valid */
        if( sock->remoteAddr.sin_addr.s_addr == (unsigned)( -1 ))
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Invalid address '%s', error: '%s'", ANY_LOG_ERROR, serverHostAddr, s );
#if !defined(__msvc__) && !defined(__windows__)
            close( mySockFd );
#else
            closesocket( mySockFd );
#endif
            goto out;
        }
    }

    retVal = 0;

    /*
     * by default all the UDP sockets must report all the
     * ICMP errors to the user's application
     */
    BerkeleySocket_setIpRcvError( sock, true );

    out:

    return retVal;
}


static int BerkeleySocketClient_initTcpClient( BerkeleySocketClient *self, char *serverHostAddr, int serverPortNo )
{
    BerkeleySocket *sock = (BerkeleySocket *)NULL;
    int retVal = -1;
    BerkeleySocketHandle mySockFd = BERKELEYSOCKETHANDLE_INVALID;
    int rVal = 0;
    char s[512];
    char *ipAddr;
    struct timeval timeout;
    fd_set rfd;
    fd_set wfd;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETCLIENT_VALID );
    ANY_REQUIRE( serverPortNo > 0 );
    ANY_REQUIRE( serverHostAddr );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_NULL );

    /* take the client socket */
    sock = self->socket;

    ipAddr = BerkeleySocket_host2Addr( serverHostAddr, s, 512 );

    if( ipAddr == (char *)NULL)
    {
        ANY_LOG( 5, "Cannot resolve hostname '%s'", ANY_LOG_WARNING, serverHostAddr );
        goto out;
    }

    Any_memset( &sock->remoteAddr, 0, sizeof( sock->remoteAddr ));

    sock->remoteAddr.sin_family = AF_INET;
    sock->remoteAddr.sin_addr.s_addr = inet_addr( ipAddr );
    sock->remoteAddr.sin_port = htons( serverPortNo );

    /* check if the remote address is valid */
    if( sock->remoteAddr.sin_addr.s_addr == (unsigned)-1 )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 0, "Invalid address '%s', error: '%s'", ANY_LOG_ERROR, serverHostAddr, s );
        goto out;
    }

    /* create a TCP socket */
    mySockFd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( mySockFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 0, "Can't open stream socket, error: '%s'", ANY_LOG_ERROR, s );
        goto out;
    }

    /* sets the socket un/blocking */
    BerkeleySocket_setBlocking( sock, BERKELEYSOCKET_OPTION_GET( sock, BLOCKING ));

    /* try to connect on remote side */
    rVal = connect( mySockFd, (struct sockaddr *)&sock->remoteAddr,
                    sizeof( sock->remoteAddr ));

    if( rVal == BERKELEYSOCKET_ERROR )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 3, "Can't connect() to '%s', error: '%s'", ANY_LOG_WARNING, serverHostAddr, s );
#if !defined(__msvc__) && !defined(__windows__)
        close( mySockFd );
#else
        closesocket( mySockFd );
#endif
        goto out;
    }

    /* for unblocking we have to wait a timeout */
    if( BERKELEYSOCKET_OPTION_GET( sock, BLOCKING ) == false )
    {
        ANY_LOG( 5, "Entering on nonblocking mode", ANY_LOG_INFO );

        FD_ZERO( &rfd );
        FD_ZERO( &wfd );

        FD_SET( mySockFd, &rfd );
        FD_SET( mySockFd, &wfd );

        /* setup the timeout event */
        timeout.tv_sec = ( sock->connectTimeout / 1000000L );
        timeout.tv_usec = ( sock->connectTimeout % 1000000L );

        /*
         * wait some connection or timeout
         */
        rVal = select( mySockFd + 1, &rfd, &wfd, NULL, &timeout );

        if( rVal > 0 )
        {
            if( FD_ISSET( mySockFd, &rfd ) || FD_ISSET( mySockFd, &wfd ))
            {
                goto ok;
            }
        }
        else /* we got a timeout on connect */
        {
            ANY_LOG( 0, "Unable to connect on '%s:%d'", ANY_LOG_ERROR, serverHostAddr, serverPortNo );
#if !defined(__msvc__) && !defined(__windows__)
            close( mySockFd );
#else
            closesocket( mySockFd );
#endif
            goto out;
        }
    }


    if( rVal == BERKELEYSOCKET_ERROR )
    {
#if !defined(__msvc__) && !defined(__windows__)
        close( mySockFd );
#else
        closesocket( mySockFd );
#endif
        goto out;
    }


    ok:
    sock->socketFd = mySockFd;
    sock->type = BERKELEYSOCKET_TCP;
    retVal = 0;

    out:

    return retVal;
}


/* EOF */
