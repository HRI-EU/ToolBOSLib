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


#include <Any.h>
#include <BerkeleySocket.h>
#include <BerkeleySocketServer.h>

#if !defined(__windows__)
    #include <string.h>
    #include <time.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <ctype.h>
#endif


#define BERKELEYSOCKETSERVER_VALID  0xc980b3a8
#define BERKELEYSOCKETSERVER_INVALID  0x9e5285c4


static int BerkeleySocketServer_initTcpServer( BerkeleySocketServer *self, int serverPortNo, int maxClient );

static int BerkeleySocketServer_initUdpServer( BerkeleySocketServer *self, int serverPortNo, int maxClient );

static BerkeleySocketHandle BerkeleySocketServer_acceptTcpClient( BerkeleySocketServer *self,
                                                                  BerkeleySocket *newBerkeleySocket );

static BerkeleySocketHandle BerkeleySocketServer_acceptUdpClient( BerkeleySocketServer *self,
                                                                  BerkeleySocket *newBerkeleySocket );


BerkeleySocketServer *BerkeleySocketServer_new( void )
{
    BerkeleySocketServer *self = ANY_TALLOC( BerkeleySocketServer );

    return self;
}


bool BerkeleySocketServer_init( BerkeleySocketServer *self, BerkeleySocket *sock )
{
    bool result = false;

    ANY_REQUIRE( self );

    self->valid = BERKELEYSOCKETSERVER_INVALID;

    BerkeleySocketServer_setServerAddr( self, htonl(INADDR_ANY));

    /* if we doen't specify our socket than we create a new one */
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

        ANY_REQUIRE( sock->valid );
        ANY_REQUIRE( BerkeleySocket_getFd( sock ) == BERKELEYSOCKETHANDLE_INVALID );
        self->socket = sock;
        self->created = false;
    }

    self->broadcast = false;

    self->valid = BERKELEYSOCKETSERVER_VALID;
    result = true;

    out:

    return ( result );
}


void BerkeleySocketServer_setBroadcast( BerkeleySocketServer *self, bool broadcast )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );

    ANY_LOG( 5, "Setting broadcast to '%s'", ANY_LOG_INFO, broadcast ? "true" : "false" );
    self->broadcast = broadcast;
}


BerkeleySocket *BerkeleySocketServer_getSocket( BerkeleySocketServer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );

    return ( self->socket );
}


BerkeleySocket *BerkeleySocketServer_connect( BerkeleySocketServer *self, BerkeleySocketType type, int portNo,
                                              int maxClient )
{
    BerkeleySocket *retVal = (BerkeleySocket *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( portNo > 0 );
    ANY_REQUIRE( maxClient > 0 );

    switch( type )
    {
        case BERKELEYSOCKET_TCP:
            BerkeleySocketServer_initTcpServer( self, portNo, maxClient );
            break;

        case BERKELEYSOCKET_UDP:
            BerkeleySocketServer_initUdpServer( self, portNo, maxClient );
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, type );
            break;
    }

    if( BerkeleySocket_getFd( self->socket ) != BERKELEYSOCKETHANDLE_INVALID &&
        BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL )
    {
        retVal = self->socket;
    }

    return retVal;
}


bool BerkeleySocketServer_acceptClient( BerkeleySocketServer *self, BerkeleySocket *newBerkeleySocket )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL );
    ANY_REQUIRE( BerkeleySocket_getFd( self->socket ) != BERKELEYSOCKETHANDLE_INVALID );
    ANY_REQUIRE( newBerkeleySocket );

    switch( BerkeleySocket_getType( self->socket ))
    {
        case BERKELEYSOCKET_TCP:
            BerkeleySocketServer_acceptTcpClient( self, newBerkeleySocket );
            break;

        case BERKELEYSOCKET_UDP:
            BerkeleySocketServer_acceptUdpClient( self, newBerkeleySocket );
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, BerkeleySocket_getType( self->socket ));
            break;
    }

    if( newBerkeleySocket->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_cloneProperties( self->socket, newBerkeleySocket );
        retVal = true;
    }
    else
    {
        ANY_LOG( 0, "Error in accept()", ANY_LOG_WARNING );
    }

    return retVal;
}


bool BerkeleySocketServer_waitClient( BerkeleySocketServer *self, const long microsecs )
{
    int ret = 0;
    BerkeleySocketHandle socketFd = BERKELEYSOCKETHANDLE_INVALID;
    fd_set rfd;
    struct timeval timeout;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL );

    /* setup the timeout event */
    timeout.tv_sec = ( microsecs / 1000000L );
    timeout.tv_usec = ( microsecs % 1000000L );

    socketFd = BerkeleySocket_getFd( self->socket );
    ANY_REQUIRE( socketFd != BERKELEYSOCKETHANDLE_INVALID );

    /* reset the fd set */
    FD_ZERO( &rfd );
    FD_SET( socketFd, &rfd );

    /*
     * wait some connection or timeout
     */
    ret = select( socketFd + 1, &rfd, NULL, NULL, &timeout );

    return ret > 0 && FD_ISSET( socketFd, &rfd );
}


void BerkeleySocketServer_loop( BerkeleySocketServer *self,
                                bool (*clientReadyCallBack)( BerkeleySocket *, void * ),
                                void *data1,
                                bool (*timeoutCallBack)( BerkeleySocket *, void * ),
                                void *data2,
                                long timeout )
{
    bool quit = false;
    BerkeleySocket newBerkeleySocket;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL );

    BerkeleySocket_init( &newBerkeleySocket );


    while( ! quit )
    {
        /* wait a new client */
        if( BerkeleySocketServer_waitClient( self, timeout ) )
        {
            /* accept the client filling out BerkeleySocket with its data */
            BerkeleySocketServer_acceptClient( self, &newBerkeleySocket );

            /* check if we have to call the dataReadyCallBack function */
            if( clientReadyCallBack )
            {
                quit = ( *clientReadyCallBack )( &newBerkeleySocket, data1 );
            }
            else
            {
                ANY_LOG( 5, "Data available but clientReadyCallBack function undefined. Closing socket", ANY_LOG_INFO );
                BerkeleySocket_disconnect( &newBerkeleySocket );
            }
        }
        else
        {
            /* else check if we have to call the timeoutCallBack function */
            if( timeoutCallBack )
            {
                quit = ( *timeoutCallBack )( self->socket, data2 );
            }
            else
            {
                ANY_LOG( 5, "Got a timeout but timeoutCallBack function undefined", ANY_LOG_INFO );
            }
        }
    }

    BerkeleySocket_disconnect( &newBerkeleySocket );
    BerkeleySocket_clear( &newBerkeleySocket );
}


void BerkeleySocketServer_disconnect( BerkeleySocketServer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) != BERKELEYSOCKET_NULL );
    ANY_REQUIRE( BerkeleySocket_getFd( self->socket ) > -1 );

    BerkeleySocket_disconnect( self->socket );
}


void BerkeleySocketServer_clear( BerkeleySocketServer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );

    BerkeleySocket_disconnect( self->socket );
    BerkeleySocket_clear( self->socket );

    if( self->created )
    {
        BerkeleySocket_delete( self->socket );
        self->socket = (BerkeleySocket *)NULL;
        self->created = false;
    }

    self->valid = BERKELEYSOCKETSERVER_INVALID;
}


void BerkeleySocketServer_delete( BerkeleySocketServer *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static int BerkeleySocketServer_initTcpServer( BerkeleySocketServer *self,
                                               int serverPortNo,
                                               int maxClient )
{
#define ERROR_BUFFER_SIZE 512
    BerkeleySocket *sock = (BerkeleySocket *)NULL;
    char errStr[ERROR_BUFFER_SIZE];
    BerkeleySocketHandle mySockFd = BERKELEYSOCKETHANDLE_INVALID;
    int rVal = 1;
    BerkeleySocketHandle oldSocketFd;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( serverPortNo > 0 );
    ANY_REQUIRE( maxClient > 0 );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_NULL );

    sock = self->socket;

    Any_memset( &sock->remoteAddr, 0, sizeof( struct sockaddr_in ));
    sock->remoteAddr.sin_family = AF_INET;
    sock->remoteAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock->remoteAddr.sin_port = htons( serverPortNo );

    mySockFd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( mySockFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, ERROR_BUFFER_SIZE );
        ANY_LOG( 0, "Unable to open socket on port %d (%s)", ANY_LOG_ERROR,
                 serverPortNo, errStr );
        goto out;
    }

    /* SO_REUSEADDR ( and possibly other options too ) has to be set BEFORE the
     * bind() call. */
    oldSocketFd = sock->socketFd;
    sock->socketFd = mySockFd;
    BerkeleySocket_setOptions( sock );
    sock->socketFd = oldSocketFd;

    rVal = bind( mySockFd, (struct sockaddr *)&sock->remoteAddr,
                 sizeof( sock->remoteAddr ));

    if( rVal == BERKELEYSOCKET_ERROR )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, ERROR_BUFFER_SIZE );
        ANY_LOG( 0, "Unable to bind server socket to port %d (%s)",
                 ANY_LOG_ERROR, serverPortNo, errStr );
#if !defined(__msvc__) && !defined(__windows__)
        close( mySockFd );
#else
        closesocket( mySockFd );
#endif
        mySockFd = BERKELEYSOCKETHANDLE_INVALID;
        goto out;
    }

    rVal = listen( mySockFd, maxClient );

    if( rVal == BERKELEYSOCKET_ERROR )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, ERROR_BUFFER_SIZE );
        ANY_LOG( 0, "Can't listen on server address (%s)",
                 ANY_LOG_ERROR, errStr );
#if !defined(__msvc__) && !defined(__windows__)
        close( mySockFd );
#else
        closesocket( mySockFd );
#endif
        mySockFd = BERKELEYSOCKETHANDLE_INVALID;
        goto out;
    }

    sock->socketFd = mySockFd;
    sock->type = BERKELEYSOCKET_TCP;

    out:

    return ( mySockFd );

#undef ERROR_BUFFER_SIZE

}


static int BerkeleySocketServer_initUdpServer( BerkeleySocketServer *self,
                                               int serverPortNo,
                                               int maxClient )
{
#define ERROR_BUFFER_SIZE 512
    BerkeleySocket *sock = (BerkeleySocket *)NULL;
    char errStr[ERROR_BUFFER_SIZE];
    BerkeleySocketHandle mySockFd = BERKELEYSOCKETHANDLE_INVALID;
    BerkeleySocketHandle oldSocketFd;
    int rVal = 1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( serverPortNo > 0 );
    ANY_REQUIRE( maxClient > 0 );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_NULL );

    sock = self->socket;

    Any_memset( &sock->remoteAddr, 0, sizeof( struct sockaddr_in ));
    sock->remoteAddr.sin_family = AF_INET;
    sock->remoteAddr.sin_addr.s_addr = htonl( self->broadcast ? INADDR_BROADCAST : INADDR_ANY);
    sock->remoteAddr.sin_port = htons( serverPortNo );

    mySockFd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if( mySockFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, ERROR_BUFFER_SIZE );
        ANY_LOG( 0, "Can't open datagram socket, error: '%s'", ANY_LOG_ERROR, errStr );
        goto out;
    }

    sock->socketFd = mySockFd;
    sock->type = BERKELEYSOCKET_UDP;

    if( self->broadcast )
    {
        BerkeleySocket_setBroadcast( sock, true, serverPortNo );
    }

    /* SO_REUSEADDR ( and SO_EXCLUSIVEADDRUSE, and possibly other options too )
     * has to be set BEFORE the bind() call. */
    oldSocketFd = sock->socketFd;
    sock->socketFd = mySockFd;
    BerkeleySocket_setOptions( sock );
    sock->socketFd = oldSocketFd;

    rVal = bind( mySockFd, (struct sockaddr *)&sock->remoteAddr,
                 sizeof( sock->remoteAddr ));

    if( rVal == BERKELEYSOCKET_ERROR )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, ERROR_BUFFER_SIZE );
        ANY_LOG( 0, "Can't bind() datagram socket on server address, error: '%s'",
                 ANY_LOG_ERROR, errStr );
#if !defined(__msvc__) && !defined(__windows__)
        close( mySockFd );
#else
        closesocket( mySockFd );
#endif
        mySockFd = BERKELEYSOCKETHANDLE_INVALID;
        goto out;
    }

    /* by default all the UDP sockets must report all the ICMP errors to the user's application */
    BerkeleySocket_setIpRcvError( sock, true );

    out:

    return ( mySockFd );

#undef ERROR_BUFFER_SIZE
}


static BerkeleySocketHandle BerkeleySocketServer_acceptTcpClient( BerkeleySocketServer *self,
                                                                  BerkeleySocket *newBerkeleySocket )
{
    BerkeleySocket *sock = (BerkeleySocket *)NULL;
#if !defined(__windows__)
    socklen_t clientLength = 0;
#else
    int clientLength = 0;
#endif

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_TCP );
    ANY_REQUIRE( newBerkeleySocket );

    clientLength = sizeof( newBerkeleySocket->remoteAddr );
    sock = self->socket;

    newBerkeleySocket->socketFd = accept( sock->socketFd,
                                          (struct sockaddr *)&newBerkeleySocket->remoteAddr,
                                          &clientLength );

    if( newBerkeleySocket->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        char errStr[512];

        BerkeleySocket_strerror(BerkeleySocket_errno(), errStr, 512 );
        ANY_LOG( 0, "Error on accept(), error: '%s'", ANY_LOG_WARNING, errStr );
        return -1;
    }

    return ( newBerkeleySocket->socketFd );
}


static BerkeleySocketHandle BerkeleySocketServer_acceptUdpClient( BerkeleySocketServer *self,
                                                                  BerkeleySocket *newBerkeleySocket )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKETSERVER_VALID );
    ANY_REQUIRE( BerkeleySocket_getType( self->socket ) == BERKELEYSOCKET_UDP );
    ANY_REQUIRE( newBerkeleySocket );

    newBerkeleySocket->socketFd = BerkeleySocket_getFd( self->socket );

    return ( BerkeleySocket_getFd( self->socket ));
}


bool BerkeleySocketServer_setServerAddr( BerkeleySocketServer *self, int serverAddr )
{
    ANY_REQUIRE( self );
    /* do not ANY_REQUIRE( serverAddr ) here because INADDR_ANY is defined as 0 */

    self->serverAddr = serverAddr;

    return self->serverAddr == serverAddr;
}


/* EOF */
