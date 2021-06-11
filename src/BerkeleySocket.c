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


#include <stdio.h>
#include <stdlib.h>


#if defined(__msvc__) || defined(__windows__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#include <winsock2.h>

#if defined( __mingw__ )
#include <ws2tcpip.h>
#else
#include <Ws2tcpip.h>
#endif

#else

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/times.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#endif


#include <Any.h>
#include <BerkeleySocket.h>


/* Under Windows MSG_NOSIGNAL is not present, so we define it with a value
 * that's safe to be or'ed */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifdef __windows__

/* On Win32 we have to initialize the socket.dll */
static int socketInitialized = 0;

#endif


#define BERKELEYSOCKET_VALID    0x02f3dfe0
#define BERKELEYSOCKET_INVALID  0x9f860dcf

#define BERKELEYSOCKET_CONNECT_MAXRETRY 10


static void BerkeleySocket_setDefaultValue( BerkeleySocket *self );

static bool BerkeleySocket_checkUDPClosed( BerkeleySocket *self );

#ifdef __windows__
static void BerkeleySocket_initializeWinSock( void );
#endif


BerkeleySocket *BerkeleySocket_new( void )
{
    BerkeleySocket *self = ANY_TALLOC( BerkeleySocket );

    return self;
}


int BerkeleySocket_init( BerkeleySocket *self )
{
    ANY_REQUIRE( self );

#ifdef __windows__
    BerkeleySocket_initializeWinSock();
#endif

    self->valid = BERKELEYSOCKET_INVALID;

    self->type = BERKELEYSOCKET_NULL;
    self->socketFd = BERKELEYSOCKETHANDLE_INVALID;
    self->valid = BERKELEYSOCKET_VALID;

    BerkeleySocket_setDefaultValue( self );

    return ( BERKELEYSOCKET_INIT_SUCCESS );
}


long BerkeleySocket_getConnectTimeout( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return self->connectTimeout;
}


long BerkeleySocket_getIsReadPossibleTimeout( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return self->readTimeout;
}


long BerkeleySocket_getIsWritePossibleTimeout( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return self->writeTimeout;
}


int BerkeleySocket_getLingerTimeout( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return self->lingerTimeout;
}


bool BerkeleySocket_getReadStatus( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return (( BerkeleySocket_isDisconnected( self ) == false ) &&
            BerkeleySocket_isReadDataAvailable( self ) == true );
}


bool BerkeleySocket_getWriteStatus( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return (( BerkeleySocket_isDisconnected( self ) == false ) &&
            BerkeleySocket_isWritePossible( self ) == true );
}


void BerkeleySocket_setDefaultTimeout( BerkeleySocket *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->connectTimeout = usecs;
    self->readTimeout = usecs;
    self->writeTimeout = usecs;
}


void BerkeleySocket_setConnectTimeout( BerkeleySocket *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->connectTimeout = usecs;
}


void BerkeleySocket_setIsWritePossibleTimeout( BerkeleySocket *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->writeTimeout = usecs;
}


void BerkeleySocket_setIsReadDataAvailableTimeout( BerkeleySocket *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->readTimeout = usecs;
}

void BerkeleySocket_setCloseOnExec( BerkeleySocket *self, bool stat )
{
    int fd;
    int flags;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

#if !defined(__windows__)
    fd = self->socketFd;
    flags = fcntl( fd , F_GETFD );
    if (stat)
    {
        fcntl( fd, F_SETFD, flags | FD_CLOEXEC );
    }
    else
    {
        fcntl( fd, F_SETFD, flags & ~FD_CLOEXEC );
    }
#endif
}

void BerkeleySocket_setReadWriteTimeout( BerkeleySocket *self, long rusecs, long wusecs )
{
    int error;
    struct timeval timeout;
    char s[512];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

#if defined(SO_RCVTIMEO)
    /* setup the socket read timeout */
    timeout.tv_sec = ( rusecs / 1000000L );
    timeout.tv_usec = ( rusecs % 1000000L );

    error = setsockopt( self->socketFd, SOL_SOCKET, SO_RCVTIMEO,
                        (void *)&timeout, sizeof( timeout ));

    if( error == -1 )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 0, "Can't set SO_RCVTIMEO, error: '%s'", ANY_LOG_ERROR, s );
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_RCVTIMEO", ANY_LOG_WARNING );
#endif

#if defined(SO_SNDTIMEO)
    /* setup the socket read timeout */
    timeout.tv_sec = ( wusecs / 1000000L );
    timeout.tv_usec = ( wusecs % 1000000L );

    error = setsockopt( self->socketFd, SOL_SOCKET, SO_SNDTIMEO,
                        (void *)&timeout, sizeof( timeout ));

    if( error == -1 )
    {
        BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
        ANY_LOG( 0, "Can't set SO_SNDTIMEO, error: '%s'", ANY_LOG_ERROR, s );
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_SNDTIMEO", ANY_LOG_WARNING );
#endif

    out:;
}


char *BerkeleySocket_host2Addr( const char *hostName, char *buff, int buffsize )
{
    struct in_addr in;
    struct hostent *remoteHost = (struct hostent *)NULL;
    struct hostent myHost;
    char tmpBuff[1024];
    int locError = 0;
    char *ptr = (char *)NULL;

    ANY_REQUIRE( hostName );
    ANY_REQUIRE( buff );
    ANY_REQUIRE( buffsize > 0 );

#ifdef __windows__
    BerkeleySocket_initializeWinSock();
#endif


#if defined(__windows__) || defined(__macos__)
    remoteHost = gethostbyname( hostName );
#else

    if( BerkeleySocket_inetaton( hostName, &in ) != 0 )
    {
        Any_strncpy( buff, hostName, buffsize );
        buff[ buffsize - 1 ] = '\0';

        ptr = (char *)buff;

        /* got it */
        goto out;
    }
    else if( gethostbyname_r( hostName, &myHost, tmpBuff, 1024, &remoteHost, &locError ) != 0 )
    {
        remoteHost = NULL;
    }
#endif

    /* if we have found the hostname */
    if( remoteHost )
    {
        ANY_REQUIRE( remoteHost->h_addr_list );
        ANY_REQUIRE( remoteHost->h_addr_list[ 0 ] );

        in.s_addr = *(unsigned int *)remoteHost->h_addr_list[ 0 ];

        ptr = BerkeleySocket_inetntoa( in, buff, buffsize );
    }

    out:

    return ( ptr );
}


BerkeleySocketHandle BerkeleySocket_connectToFd( BerkeleySocket *self, int fd )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( fd != -1 );
    ANY_REQUIRE( self->type == BERKELEYSOCKET_NULL || self->type == BERKELEYSOCKET_FD );

    self->socketFd = (BerkeleySocketHandle)fd;
    self->type = BERKELEYSOCKET_FD;

    return ( self->socketFd );
}


void BerkeleySocket_setBlocking( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, BLOCKING );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, BLOCKING );
    }

#if defined(FIONBIO)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, BLOCKING ) == true ? 0 : 1 );
        int result = 0;
        char s[512];

#if !defined(__msvc__) && !defined(__windows__)
        /* sets the socket un/blocking */
        result = ioctl( self->socketFd, FIONBIO, (void *)&val );
#else
        /* sets the socket un/blocking */
        result = ioctlsocket( self->socketFd, FIONBIO, (void*)&val );
#endif

        if( result == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Unable to set '%s' for nonblocking socket, error: '%s'",
                     ANY_LOG_ERROR, ( BERKELEYSOCKET_OPTION_GET( self, BLOCKING ) == true ? "true" : "false" ), s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: FIONBIO", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setTcpNoDelay( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, NAGLE );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, NAGLE );
    }

#if defined(TCP_NODELAY)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, NAGLE ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, IPPROTO_TCP, TCP_NODELAY, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set TCP_NODELAY, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: TCP_NODELAY", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setTcpCork( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, CORK );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, CORK );
    }

#if defined(TCP_CORK)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, CORK ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, IPPROTO_TCP, TCP_CORK, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set TCP_CORK, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: TCP_CORK", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setIpTosLowDelay( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, IPTOSLOWDELAY );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, IPTOSLOWDELAY );
    }

#if defined(IPTOS_LOWDELAY)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID && BERKELEYSOCKET_OPTION_GET( self, IPTOSLOWDELAY ) == true )
    {
        int val = IPTOS_LOWDELAY;
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, IPPROTO_IP, IP_TOS, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set IPTOS_LOWDELAY, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: IPTOS_LOWDELAY", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setIpTosThroughput( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, IPTOSTHROUGHPUT );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, IPTOSTHROUGHPUT );
    }

#if defined(IPTOS_THROUGHPUT)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID && BERKELEYSOCKET_OPTION_GET( self, IPTOSTHROUGHPUT ) == true )
    {
        int val = IPTOS_THROUGHPUT;
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, IPPROTO_IP, IP_TOS, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set IPTOS_THROUGHPUT, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: IPTOS_THROUGHPUT", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setTcpMss( BerkeleySocket *self, unsigned int mss )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->mss = mss;

#if defined(TCP_MAXSEG)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        unsigned int val = 0;
        int error = 0;
        char s[512];

        if( self->mss != 0 )
        {
            error = setsockopt( self->socketFd, IPPROTO_TCP, TCP_MAXSEG,
                                (void *)&self->mss, sizeof( self->mss ));

            if( error == -1 )
            {
                BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
                ANY_LOG( 0, "Can't set TCP_MAXSEG, error: '%s'", ANY_LOG_ERROR, s );
            }
        }

        val = sizeof( self->mss );

        error = getsockopt( self->socketFd, IPPROTO_TCP, TCP_MAXSEG,
                            (void *)&self->mss, &val );

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't get TCP_MAXSEG, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: TCP_MAXSEG", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setReuseAddr( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, REUSEADDR );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, REUSEADDR );
    }

#if defined(SO_REUSEADDR)
    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, REUSEADDR ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set SO_REUSEADDR, error: '%s'", ANY_LOG_ERROR, s );
        }
        else
        {
            BerkeleySocket_setLinger( self, true, self->lingerTimeout );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_REUSEADDR", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setKeepAlive( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, KEEPALIVE );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, KEEPALIVE );
    }

#if defined(SO_KEEPALIVE)
    if( self->socketFd >= 0 )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, KEEPALIVE ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, SOL_SOCKET, SO_KEEPALIVE, (void *)&val, sizeof( val ));

#ifndef __windows__
        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set SO_KEEPALIVE, error: '%s'", ANY_LOG_ERROR, s );
        }
#endif
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_KEEPALIVE", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setOobinline( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, OOBINLINE );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, OOBINLINE );
    }

#if defined(SO_OOBINLINE)
    if( self->socketFd >= 0 )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, OOBINLINE ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, SOL_SOCKET, SO_OOBINLINE, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set SO_OOBINLINE, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_OOBINLINE", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setDontroute( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, DONTROUTE );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, DONTROUTE );
    }

#if defined(SO_DONTROUTE)
    if( self->socketFd >= 0 )
    {
        int val = (BERKELEYSOCKET_OPTION_GET( self, DONTROUTE ) == true ? 1 : 0 );
        int error = 0;
        char s[512];

        error = setsockopt( self->socketFd, SOL_SOCKET, SO_DONTROUTE, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set SO_DONTROUTE, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_DONTROUTE", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setLinger( BerkeleySocket *self, bool stat, int secs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, LINGER );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, LINGER );
    }
    self->lingerTimeout = secs;

#if defined(SO_LINGER)
    if( self->socketFd >= 0 )
    {
        struct linger val;
        int error = 0;
        char s[512];

#if defined(__windows__)
        /* windows doesn't permit to set socket LINGERING on UDP sockets */
        if ( self->type == BERKELEYSOCKET_TCP )
        {
#endif
        val.l_onoff = (BERKELEYSOCKET_OPTION_GET( self, LINGER ) == true ? 1 : 0 );
        val.l_linger = self->lingerTimeout;

        error = setsockopt( self->socketFd, SOL_SOCKET, SO_LINGER, (void *)&val, sizeof( val ));

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set SO_LINGER, error: '%s'", ANY_LOG_ERROR, s );
        }
#if defined(__windows__)
        }
        else
        {
           ANY_LOG( 1, "SO_LINGER socket option is not available for UDP protocol under Windows", ANY_LOG_INFO );
        }
#endif
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_LINGER", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setRcvBuffer( BerkeleySocket *self, unsigned int size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( size > BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE)
    {
        ANY_LOG( 5, "Invalid receive buffer size, max %d",
                 ANY_LOG_WARNING, BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE );
        size = BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE;
    }
    self->rcvBuffSize = size;

#if defined(SO_RCVBUF)
    if( self->socketFd >= 0 )
    {
        unsigned int len = 0;
        int error = 0;
        char s[512];

        if( self->rcvBuffSize != 0 )
        {
            error = setsockopt( self->socketFd, SOL_SOCKET, SO_RCVBUF,
                                (void *)&self->rcvBuffSize, sizeof( self->rcvBuffSize ));
            if( error == -1 )
            {
                BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
                ANY_LOG( 0, "Can't set SO_RCVBUF, error: '%s'", ANY_LOG_ERROR, s );
            }
        }

        len = sizeof( self->rcvBuffSize );

        error = getsockopt( self->socketFd, SOL_SOCKET, SO_RCVBUF,
                            (void *)&self->rcvBuffSize, &len );

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't get SO_RCVBUF, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_RCVBUF", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setSndBuffer( BerkeleySocket *self, unsigned int size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( size > BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE)
    {
        ANY_LOG( 5, "Invalid send buffer size, max %d",
                 ANY_LOG_WARNING, BERKELEYSOCKET_OPTION_MAX_SNDBUFSIZE );
        size = BERKELEYSOCKET_OPTION_MAX_SNDBUFSIZE;
    }
    self->sndBuffSize = size;

#if defined(SO_SNDBUF)
    if( self->socketFd >= 0 )
    {
        unsigned int len = 0;
        int error = 0;
        char s[512];

        if( self->sndBuffSize != 0 )
        {
            error = setsockopt( self->socketFd, SOL_SOCKET, SO_SNDBUF,
                                (void *)&self->sndBuffSize, sizeof( self->sndBuffSize ));
            if( error == -1 )
            {
                BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
                ANY_LOG( 0, "Can't set SO_SNDBUF, error: '%s'", ANY_LOG_ERROR, s );
            }
        }

        len = sizeof( self->sndBuffSize );

        error = getsockopt( self->socketFd, SOL_SOCKET, SO_SNDBUF,
                            (void *)&self->sndBuffSize, &len );

        if( error == -1 )
        {
            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't get SO_SNDBUF, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_SNDBUF", ANY_LOG_WARNING );
#endif
}


void BerkeleySocket_setCloseOnDisconnect( BerkeleySocket *self, bool stat )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, CLOSEONDISCONNECT );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, CLOSEONDISCONNECT );
    }
}


void BerkeleySocket_setBufferedWriteBulk( BerkeleySocket *self, bool stat, int bufferSize )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, BUFFEREDWRITEBULK );
        self->writeBulkBufferSize = bufferSize;
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, BUFFEREDWRITEBULK );
    }
}


void BerkeleySocket_setIpRcvError( BerkeleySocket *self, bool stat )
{
    int on = 0;
    int error = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( self->type == BERKELEYSOCKET_UDP )
    {
        on = stat ? 1 : 0;

#if defined( IP_RECVERR ) && !defined( __windows__ )  // SOL_IP not available under MSVC 2017
        error = setsockopt( self->socketFd, SOL_IP, IP_RECVERR, &on, sizeof( on ));
#else
        ANY_LOG( 5, "IP_RECVERR isn't supported in this system, ignoring", ANY_LOG_WARNING );
#endif

        if( error == -1 )
        {
            char s[512];

            BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
            ANY_LOG( 0, "Can't set IP_RECVERR, error: '%s'", ANY_LOG_ERROR, s );
        }
    }
    else
    {
        ANY_LOG( 5, "IP_RECVERR can be set only on UDP socket, ignoring", ANY_LOG_WARNING );
    }
}


void BerkeleySocket_setBroadcast( BerkeleySocket *self, bool stat, int port )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( stat )
    {
        BERKELEYSOCKET_OPTION_SET( self, BROADCAST );
    }
    else
    {
        BERKELEYSOCKET_OPTION_RESET( self, BROADCAST );
    }

#if defined(SO_BROADCAST)
    if( self->type == BERKELEYSOCKET_UDP )
    {
        if( self->socketFd >= 0 )
        {
            int error = 0;
            char s[512];
            int broadcast = (BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) == true ? 1 : 0 );

            error = setsockopt( self->socketFd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast, sizeof( broadcast ));

            if( error == -1 )
            {
                BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
                ANY_LOG( 0, "Can't set SO_BROADCAST, error: '%s'", ANY_LOG_ERROR, s );
                return;
            }

            self->port = port;
        }
    }
    else
    {
        ANY_LOG( 0, "Can't set SO_BROADCAST on non UDP socket", ANY_LOG_WARNING );
    }
#else
    ANY_LOG( 0, "Undefined socket option: SO_BROADCAST", ANY_LOG_WARNING );
#endif
}


int BerkeleySocket_disconnect( BerkeleySocket *self )
{
    int temp = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( self->socketFd != BERKELEYSOCKETHANDLE_INVALID )
    {
        if( self->type == BERKELEYSOCKET_TCP ||
            self->type == BERKELEYSOCKET_UDP ||
            ( self->type == BERKELEYSOCKET_FD && BERKELEYSOCKET_OPTION_GET( self, CLOSEONDISCONNECT ) ))
        {

            /*
             * tell the other party that we are going to shutdown. This
             * will send a TCP FIN before the close(), which finally sends
             * a TCP RST
             */
#if !defined(__msvc__) && !defined(__windows__)
            shutdown( self->socketFd, SHUT_WR );

            temp = close( self->socketFd );
#else
            shutdown( self->socketFd, SD_SEND );

            temp = closesocket( self->socketFd );
#endif
        }
    }

    self->socketFd = BERKELEYSOCKETHANDLE_INVALID;
    self->type = BERKELEYSOCKET_NULL;

    BerkeleySocket_setDefaultValue( self );

    return ( temp );
}


int BerkeleySocket_write( BerkeleySocket *self, BaseUI8 *poWriteBuffer, size_t bufferSize )
{
    unsigned int len = 0;
    int retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    /* There is a chance of race condition when checking with isConnected()
     * and then calling this _write() function. So if the socket get
     * disconnected in between (self->type == NULL) we should return -1
     * and not break with ANY_REQUIRE.
     */
    /*
      ANY_REQUIRE( self->type != BERKELEYSOCKET_NULL );
      ANY_REQUIRE( self->socketFd != BERKELEYSOCKETHANDLE_INVALID );
     */

    ANY_REQUIRE( poWriteBuffer );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    if( bufferSize == 0 )
    {
        ANY_LOG( 5, "Request to write 0 byte skipped", ANY_LOG_WARNING );
        retVal = 0;
        goto out;
    }

    switch( self->type )
    {
        case BERKELEYSOCKET_TCP:
            retVal = send( self->socketFd, poWriteBuffer, bufferSize, MSG_NOSIGNAL );
            break;

        case BERKELEYSOCKET_UDP:
            len = sizeof( self->remoteAddr );

            if( BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) )
            {
                Any_memset( &self->remoteAddr, 0, len );

                self->remoteAddr.sin_family = AF_INET;
                self->remoteAddr.sin_addr.s_addr = INADDR_BROADCAST;
                self->remoteAddr.sin_port = htons( self->port );
            }

            retVal = sendto( self->socketFd, poWriteBuffer, bufferSize, MSG_NOSIGNAL,
                             (struct sockaddr *)&self->remoteAddr, len );

            if( BerkeleySocket_checkUDPClosed( self ) )
            {
                retVal = -1;
            }
            break;

        case BERKELEYSOCKET_FD:
            retVal = write( self->socketFd, poWriteBuffer, bufferSize );
            break;


            /*  unreachable code:
             *
            case BERKELEYSOCKET_NULL:
                 ANY_LOG( 0, "BerkeleySocket was disconnected", ANY_LOG_WARNING );
                 break;
             */

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, self->type );
            break;
    }

    out:

    return retVal;
}


int BerkeleySocket_writeBlock( BerkeleySocket *self, BaseUI8 *poWriteBuffer, int bufferSize )
{
    int size = -1;
    int writeTotal = 0;
    int toWrite = bufferSize;
    BaseUI8 *ptr = poWriteBuffer;
    char s[512];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poWriteBuffer );
    ANY_REQUIRE( bufferSize > 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        writeTotal = -1;
        goto out;
    }

    while( writeTotal < bufferSize )
    {
        size = BerkeleySocket_write( self, ptr, toWrite );

        if( size >= 0 )
        {
            // normal case
            writeTotal += size;
            ptr += size;
            toWrite -= size;
        }
        else
        {
            // proceed if "interrupted system call" occured
            if( BerkeleySocket_errno() != EINTR )
            {
                BerkeleySocket_strerror(BerkeleySocket_errno(), s, 512 );
                ANY_LOG( 1, "BerkeleySocket_writeBlock(): bufferSize=%d, written=%d : %s",
                         ANY_LOG_WARNING, bufferSize, size, s );
                return size;
            }
        }
    }

    out:

    return ( writeTotal );
}


int BerkeleySocket_writeUrgent( BerkeleySocket *self, BaseUI8 *poWriteBuffer, int bufferSize )
{
    unsigned int len = 0;
    int retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poWriteBuffer );
    ANY_REQUIRE( bufferSize >= 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    if( bufferSize == 0 )
    {
        ANY_LOG( 5, "Request to write 0 byte skipped", ANY_LOG_WARNING );
        retVal = 0;
        goto out;
    }

    if( BERKELEYSOCKET_OPTION_GET( self, OOBINLINE ) == false )
    {
        ANY_LOG( 0, "BerkeleySocket_setOobinline() is not set", ANY_LOG_WARNING );
    }

    switch( self->type )
    {
        case BERKELEYSOCKET_TCP:
            retVal = send( self->socketFd, poWriteBuffer, bufferSize, MSG_OOB | MSG_NOSIGNAL );
            break;

        case BERKELEYSOCKET_UDP:
            len = sizeof( self->remoteAddr );

            if( BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) )
            {
                Any_memset( &self->remoteAddr, 0, len );

                self->remoteAddr.sin_family = AF_INET;
                self->remoteAddr.sin_addr.s_addr = INADDR_BROADCAST;
                self->remoteAddr.sin_port = htons( self->port );
            }

            retVal = sendto( self->socketFd, poWriteBuffer, bufferSize, MSG_OOB | MSG_NOSIGNAL,
                             (struct sockaddr *)&self->remoteAddr, len );

            if( BerkeleySocket_checkUDPClosed( self ) )
            {
                retVal = -1;
            }
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, self->type );
            break;
    }

    out:

    return retVal;
}


int BerkeleySocket_writeBulk( BerkeleySocket *self, BerkeleySocket *source, int count )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( source );
    ANY_REQUIRE( source->valid == BERKELEYSOCKET_VALID );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID ||
        source->type == BERKELEYSOCKET_NULL ||
        source->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket or source type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        retVal = -1;
        goto out;
    }

#if defined(__linux__)
    if( BERKELEYSOCKET_OPTION_GET( self, BUFFEREDWRITEBULK ) == false )
    {
        off_t myOffset = 0;

        retVal = sendfile( self->socketFd, source->socketFd, &myOffset, count );
    }
    else
#endif
    {
#ifndef __msvc__
#define FAKE_MSVC_BUFFERSIZE ( self->writeBulkBufferSize ? self->writeBulkBufferSize : ( self->mss > 0 ? self->mss : 1024 ) )
#else
#define FAKE_MSVC_BUFFERSIZE 1024
#endif
        int buffSize = FAKE_MSVC_BUFFERSIZE;
        int chunckSize = 0;
        int readed = 0;
        int writted = 0;
        int myCount = count;
        BaseUI8 buff[FAKE_MSVC_BUFFERSIZE];

        /*
         * We read and write until count become == 0
         */
        while( myCount > 0 )
        {
            chunckSize = ( myCount >= buffSize ? buffSize : myCount );

            /* we read from the source */
            readed = BerkeleySocket_read( source, buff, chunckSize );

            if( readed != chunckSize )
            {
                ANY_LOG( 0, "Error reading source BerkeleySocket, readed %d instead of %d",
                         ANY_LOG_ERROR, readed, chunckSize );
                goto out;
            }

            /* and write to the destionation */
            writted = BerkeleySocket_write( self, buff, chunckSize );

            if( writted != chunckSize )
            {
                ANY_LOG( 0, "Error writing destination BerkeleySocket, writted %d instead of %d",
                         ANY_LOG_ERROR, writted, chunckSize );
                goto out;
            }

            /* update count and result */
            myCount -= chunckSize;
            retVal += chunckSize;
        }
    }

    out:

    return retVal;

#undef FAKE_MSVC_BUFFERSIZE
}


int BerkeleySocket_read( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize )
{
    unsigned int len = 0;
    int retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poReadBuffer );
    ANY_REQUIRE( bufferSize >= 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 0, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    switch( self->type )
    {
        case BERKELEYSOCKET_TCP:
            retVal = recv( self->socketFd, poReadBuffer, bufferSize, MSG_NOSIGNAL );
            break;

        case BERKELEYSOCKET_UDP:
            len = sizeof( self->remoteAddr );

            if( BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) )
            {
                Any_memset( &self->remoteAddr, 0, len );

                self->remoteAddr.sin_family = AF_INET;
                self->remoteAddr.sin_addr.s_addr = INADDR_BROADCAST;
                self->remoteAddr.sin_port = htons( self->port );
            }

            retVal = recvfrom( self->socketFd, poReadBuffer, bufferSize, MSG_NOSIGNAL,
                               (struct sockaddr *)&self->remoteAddr, &len );

            if( BerkeleySocket_checkUDPClosed( self ) )
            {
                retVal = -1;
            }
            break;

        case BERKELEYSOCKET_FD:
            retVal = read( self->socketFd, poReadBuffer, bufferSize );
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, self->type );
            break;
    }

    out:

    return retVal;
}


int BerkeleySocket_readBlock( BerkeleySocket *self, BaseUI8 *poReadBuffer, int size )
{
    int len = 0;
    int nleft = size;
    int retVal = -1;
    BaseUI8 *ptr = poReadBuffer;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poReadBuffer );
    ANY_REQUIRE( size >= 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 5, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    /*
     * we try to read nleft bytes from the stream terminating
     * when all the requested data has been readed or some error occurs
     */
    while( nleft > 0 )
    {
        /*
         * we alway checks for the BerkeleySocket's readability before to do anything
         * this may requires to set the timeout with the BerkeleySocket_setIsReadDataAvailableTimeout()
         */
        if( BerkeleySocket_isReadDataAvailable( self ) )
        {
            /*
             * we always use our standard BerkeleySocket_read() ;-) requesting nleft bytes to read
             * for every cicle until we reach the requested bufferSize size
             */
            len = BerkeleySocket_read( self, ptr, nleft );

            /*
             * if data was readed successfully than we advance buffer's pointer
             * and nleft bytes to go
             */
            if( len > 0 )
            {
                nleft -= len;
                ptr += len;
            }
                /* else some errors occurs */
            else if( len < 0 )
            {
                if( BerkeleySocket_errno() != EINTR )
                {
                    ANY_LOG( 5, "Error on BerkeleySocket_read(), bufferSize=%d, readed=%d, nleft=%d",
                             ANY_LOG_ERROR, size, ( size - nleft ), nleft );
                    retVal = -1;
                    goto out;
                }
            }
                /* or a strange situation occurs */
            else if( len == 0 )
            {
                ANY_LOG( 5, "Warning BerkeleySocket_read() returns 0, bufferSize=%d, readed=%d, nleft=%d",
                         ANY_LOG_WARNING, size, ( size - nleft ), nleft );
                break;
            }
        }
        else  /* BerkeleySocket_isReadDataAvailable() == false */
        {
            /*
            This should not be a case of error.
            If no data is read its ok!
            if ( BerkeleySocket_errno() != EINTR )
            {
              result = -1;
              goto out;
            }
            */
        }
    }

    retVal = size - nleft;

    out:

    return retVal;
}


int BerkeleySocket_readUrgent( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize )
{
    unsigned int len = 0;
    int retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poReadBuffer );
    ANY_REQUIRE( bufferSize >= 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 0, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    /* check if the OOBINLINE is already set */
    if( BERKELEYSOCKET_OPTION_GET( self, OOBINLINE ) )
    {
        ANY_LOG( 5, "BerkeleySocket_setOobinline() is already set you don't need to use BerkeleySocket_readUrgent()",
                 ANY_LOG_INFO );
    }

    switch( self->type )
    {
        case BERKELEYSOCKET_TCP:
            retVal = recv( self->socketFd, poReadBuffer, bufferSize, MSG_OOB | MSG_NOSIGNAL );
            break;

        case BERKELEYSOCKET_UDP:
            len = sizeof( self->remoteAddr );

            if( BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) )
            {
                Any_memset( &self->remoteAddr, 0, len );

                self->remoteAddr.sin_family = AF_INET;
                self->remoteAddr.sin_addr.s_addr = INADDR_BROADCAST;
                self->remoteAddr.sin_port = htons( self->port );
            }

            retVal = recvfrom( self->socketFd, poReadBuffer, bufferSize, MSG_OOB | MSG_NOSIGNAL,
                               (struct sockaddr *)&self->remoteAddr, &len );

            if( BerkeleySocket_checkUDPClosed( self ) )
            {
                retVal = -1;
            }
            break;

        default:
            ANY_LOG( 0, "Invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR, self->type );
            break;
    }

    out:

    return retVal;
}


int BerkeleySocket_peekInternal( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize, int additionalFlags )
{
    unsigned int len = 0;
    int retVal;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( poReadBuffer );
    ANY_REQUIRE( bufferSize >= 0 );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 0, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        retVal = -1;
        goto out;
    }


    // set errno = 0, because we are going to query it further down with
    // EINTR (interrupted system call)
    doitagain:


#if !defined(_WINDOWS_)
    errno = 0;
#endif

    switch( self->type )
    {
        case BERKELEYSOCKET_TCP:
            retVal = recv( self->socketFd, poReadBuffer, bufferSize, MSG_PEEK | MSG_NOSIGNAL | additionalFlags );
            break;

        case BERKELEYSOCKET_UDP:
            len = sizeof( self->remoteAddr );

            if( BERKELEYSOCKET_OPTION_GET( self, BROADCAST ) )
            {
                Any_memset( &self->remoteAddr, 0, len );

                self->remoteAddr.sin_family = AF_INET;
                self->remoteAddr.sin_addr.s_addr = INADDR_BROADCAST;
                self->remoteAddr.sin_port = htons( self->port );
            }

            retVal = recvfrom( self->socketFd, poReadBuffer, bufferSize, MSG_PEEK | MSG_NOSIGNAL | additionalFlags,
                               (struct sockaddr *)&self->remoteAddr, &len );

            if( BerkeleySocket_checkUDPClosed( self ) )
            {
                retVal = -1;
            }
            break;

        default:
            ANY_LOG( 0, "Cannot perform the MSG_PEEK because invalid BerkeleySocket Type '%d'", ANY_LOG_ERROR,
                     self->type );
            retVal = -1;
            break;
    }

    // if EINTR (interrupted system call occurs) try it again
    // if EAGAIN (timeout occurs) return 0
    // if errno !:= 0 an error occured return -1;

    if( BerkeleySocket_errno() == EINTR ) // interrupted system call
    {
        goto doitagain;
    }
    else if( BerkeleySocket_errno() == EAGAIN ) // timeout
    {
        retVal = 0;
    }
    else if( BerkeleySocket_errno() != 0 ) // any other error
    {
        retVal = -1;
    }
    else if( retVal == 0 )
    {
#if !defined(_WINDOWS_)
        if( bufferSize && !( additionalFlags & MSG_DONTWAIT ))
        {
            /* peek is blocking and we read zero bytes, cannot be ok */
            ANY_LOG ( 0, "Read 0 byte from socket and errno is not set!", ANY_LOG_ERROR );
        }
#else
        // windows doesn't support the MSG_DONTWAIT in Winsock
#endif
    }

    out:
    return retVal;
}


int BerkeleySocket_peek( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize )
{
    return BerkeleySocket_peekInternal( self, poReadBuffer, bufferSize, 0 );
}


bool BerkeleySocket_isReadDataAvailable( BerkeleySocket *self )
{
    int ret = 0;
    bool retVal = false;
    fd_set rfd;
    struct timeval timeout;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 0, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    /* setup the timeout event */
    timeout.tv_sec = ( self->readTimeout / 1000000L );
    timeout.tv_usec = ( self->readTimeout % 1000000L );

    /* reset the fd set */
    FD_ZERO( &rfd );
    FD_SET( self->socketFd, &rfd );

    /*
     * wait some connection or timeout
     */
    ret = select( self->socketFd + 1, &rfd, NULL, NULL, &timeout );

    retVal = ret > 0 && FD_ISSET( self->socketFd, &rfd );

    /*
     * UDP socket has special type handling since it has been setted
     * as IP_RECVERR, so it will reports any error as msg. In such
     * case the select will always reports there are new data to read
     * on the UDP socket
     */
    if( retVal && self->type == BERKELEYSOCKET_UDP )
    {
        retVal = ! BerkeleySocket_checkUDPClosed( self );
    }

    out:

    return retVal;
}


bool BerkeleySocket_isWritePossible( BerkeleySocket *self )
{
    int ret = 0;
    fd_set wfd;
    struct timeval timeout;
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    if( self->type == BERKELEYSOCKET_NULL ||
        self->socketFd == BERKELEYSOCKETHANDLE_INVALID )
    {
        ANY_LOG( 0, "The socket type is NULL or the channel is Invalid", ANY_LOG_WARNING );
        goto out;
    }

    /* setup the timeout event */
    timeout.tv_sec = ( self->writeTimeout / 1000000L );
    timeout.tv_usec = ( self->writeTimeout % 1000000L );

    /* reset the fd set */
    FD_ZERO( &wfd );
    FD_SET( self->socketFd, &wfd );

    /* wait some connection or timeout */
    ret = select( self->socketFd + 1, NULL, &wfd, NULL, &timeout );

    retVal = ret > 0 && FD_ISSET( self->socketFd, &wfd );

    out:

    return retVal;
}


int BerkeleySocket_getFd( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return ( self->socketFd );
}


char *BerkeleySocket_inetntoa( struct in_addr in, char *buf, int bufsize )
{
#define UC( __byte ) ((int)(unsigned char)__byte)
    const unsigned char *p = (const unsigned char *)&in.s_addr;

    ANY_REQUIRE( buf );
    ANY_REQUIRE( bufsize > 0 );

#if defined(__msvc__) || defined(__windows__)
    Any_sprintf( buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
#else
    Any_snprintf( buf, bufsize,
                  "%d.%d.%d.%d", UC( p[ 0 ] ), UC( p[ 1 ] ), UC( p[ 2 ] ), UC( p[ 3 ] ));
#endif

    return ( buf );
#undef UC
}


int BerkeleySocket_inetaton( const char *buf, struct in_addr *in )
{
    unsigned char *p = (unsigned char *)&( in->s_addr );
    unsigned int val[4];
    int i;

    if( Any_sscanf( buf, "%u.%u.%u.%u", &val[ 0 ], &val[ 1 ], &val[ 2 ], &val[ 3 ] ) != 4 )
    {
        /* This function is used to check whether or not the provided 'buf'
         * contains an IPv4 address or hostname. Do not log anything at this
         * place, a message like "is invalid" might be misleading. */
        return 0;
    }

    for( i = 0; i < 4; ++i )
    {
        if( val[ i ] > 255 )
        {
            return 0;
        }
        p[ i ] = (unsigned char)val[ i ];
    }

    return 1;
}


void BerkeleySocket_strerror( int errorcode, char *buffer, int len )
{
    char *ptr = NULL;
#if defined(__windows__)
    DWORD error;
#endif

    ANY_REQUIRE( buffer );

#if defined(__windows__)

    error = WSAGetLastError();

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,
                   error,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   ptr,
                   0,
                   NULL );

#else
    ptr = strerror( errorcode );
#endif

    Any_strncpy( buffer, ptr, len );

#if defined(__windows__)
    LocalFree( ptr );
#endif
}


void BerkeleySocket_setOptions( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    /* set some BerkeleySocket & TCP options */
#if defined(TCP_NODELAY)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setTcpNoDelay( self, BERKELEYSOCKET_OPTION_GET( self, NAGLE ));
    }
#endif

#if defined(IPTOS_LOWDELAY)
    BerkeleySocket_setIpTosLowDelay( self, BERKELEYSOCKET_OPTION_SET( self, IPTOSLOWDELAY ) );
#endif

#if defined(IPTOS_THROUGHPUT)
    BerkeleySocket_setIpTosThroughput( self, BERKELEYSOCKET_OPTION_GET( self, IPTOSTHROUGHPUT ));
#endif

#if defined(TCP_MAXSEG) && !defined(__windows__)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setTcpMss( self, self->mss );
    }
#endif

#if defined(TCP_CORK)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setTcpCork( self, BERKELEYSOCKET_OPTION_GET( self, CORK ));
    }
#endif

#if defined(SO_REUSEADDR)
    BerkeleySocket_setReuseAddr( self, BERKELEYSOCKET_OPTION_GET( self, REUSEADDR ));
#endif

#if defined(SO_KEEPALIVE)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setKeepAlive( self, BERKELEYSOCKET_OPTION_GET( self, KEEPALIVE ));
    }
#endif

#if defined(SO_OOBINLINE)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setOobinline( self, BERKELEYSOCKET_OPTION_GET( self, OOBINLINE ));
    }
#endif

#if defined(SO_DONTROUTE)
    BerkeleySocket_setDontroute( self, BERKELEYSOCKET_OPTION_GET( self, DONTROUTE ));
#endif

#if defined(SO_LINGER)
    if( self->type == BERKELEYSOCKET_TCP )
    {
        BerkeleySocket_setLinger( self, BERKELEYSOCKET_OPTION_GET( self, LINGER ), self->lingerTimeout );
    }
#endif

#if defined(SO_RCVBUF)
    BerkeleySocket_setRcvBuffer( self, self->rcvBuffSize );
#endif

#if defined(SO_SNDBUF)
    BerkeleySocket_setSndBuffer( self, self->sndBuffSize );
#endif
}


void BerkeleySocket_clone( BerkeleySocket *self, BerkeleySocket *destBerkeleySocket )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( destBerkeleySocket );
    ANY_REQUIRE( destBerkeleySocket->valid == BERKELEYSOCKET_VALID );

    Any_memcpy((void *)destBerkeleySocket, (void *)self, sizeof( BerkeleySocket ));
}


void BerkeleySocket_cloneProperties( BerkeleySocket *self, BerkeleySocket *destBerkeleySocket )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( destBerkeleySocket );
    ANY_REQUIRE( destBerkeleySocket->valid == BERKELEYSOCKET_VALID );

    destBerkeleySocket->type = self->type;
    destBerkeleySocket->options = self->options;
    destBerkeleySocket->mss = self->mss;
    destBerkeleySocket->connectTimeout = self->connectTimeout;
    destBerkeleySocket->readTimeout = self->readTimeout;
    destBerkeleySocket->writeTimeout = self->writeTimeout;
    destBerkeleySocket->rcvBuffSize = self->rcvBuffSize;
    destBerkeleySocket->sndBuffSize = self->sndBuffSize;
    destBerkeleySocket->lingerTimeout = self->lingerTimeout;
    destBerkeleySocket->writeBulkBufferSize = self->writeBulkBufferSize;
}


BerkeleySocketType BerkeleySocket_getType( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return ( self->type );
}


void BerkeleySocket_setRemoteAddr( BerkeleySocket *self, char *serverHostAddr, int serverPortNo )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( serverHostAddr );

    Any_memset( &self->remoteAddr, 0, sizeof( self->remoteAddr ));

    self->remoteAddr.sin_family = AF_INET;
    self->remoteAddr.sin_addr.s_addr = inet_addr( serverHostAddr );
    self->remoteAddr.sin_port = htons( serverPortNo );
}


void BerkeleySocket_getRemoteAddr( BerkeleySocket *self,
                                   char *serverHostAddr,
                                   int buffSize,
                                   int *serverPortNo )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( serverHostAddr );
    ANY_REQUIRE( serverPortNo );

    BerkeleySocket_inetntoa( self->remoteAddr.sin_addr, serverHostAddr, buffSize );
    *serverPortNo = ntohs( self->remoteAddr.sin_port );
}


char *BerkeleySocket_getRemoteIp( BerkeleySocket *self, char *buff, int buffsize )
{
    char *ptr = (char *)NULL;
    struct in_addr temp;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );
    ANY_REQUIRE( buff );
    ANY_REQUIRE( buffsize > 0 );

    temp = self->remoteAddr.sin_addr;
    ptr = BerkeleySocket_inetntoa( temp, buff, buffsize );

    return ( ptr );
}


void BerkeleySocket_clear( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->valid = BERKELEYSOCKET_INVALID;

    if( self->socketFd >= 0 )
    {
#if !defined(__msvc__) && !defined(__windows__)
        close( self->socketFd );
#else
        closesocket( self->socketFd );
#endif
    }

    self->socketFd = BERKELEYSOCKETHANDLE_INVALID;

    /* resets the socket's structure */
    Any_memset((void *)&self->sourceAddr, 0, sizeof( self->sourceAddr ));
    Any_memset((void *)&self->remoteAddr, 0, sizeof( self->remoteAddr ));

#ifdef __windows__
    /* decrement the socket counter */
    socketInitialized--;

    /* checks for unbalanced BerkeleySocket_clear() */
    ANY_REQUIRE( socketInitialized >= 0 );

    /* clear Winsock.dll if socketInitialized == 0 reached */
    if ( socketInitialized == 0 )
    {
      WSACleanup();
    }
#endif

}


void BerkeleySocket_delete( BerkeleySocket *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static void BerkeleySocket_setDefaultValue( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    self->writeTimeout = BERKELEYSOCKET_TIMEOUT_DEFAULT;
    self->readTimeout = BERKELEYSOCKET_TIMEOUT_DEFAULT;
    self->options = 0UL;

    /* by default we want blocking sockets */
    BERKELEYSOCKET_OPTION_SET( self, BLOCKING );

#if !defined(__linux__)
    /* by default we want slow writeBulk for O.S != linux */
    BERKELEYSOCKET_OPTION_SET( self, BUFFEREDWRITEBULK );
#endif

    /* system defaults (will be filled on socket creation) */
    self->rcvBuffSize = 0;
    self->sndBuffSize = 0;
    self->mss = 0;
    self->lingerTimeout = BERKELEYSOCKET_LINGERTIMEOUT_DEFAULT;

    Any_memset((void *)&self->sourceAddr, 0, sizeof( self->sourceAddr ));
    Any_memset((void *)&self->remoteAddr, 0, sizeof( self->remoteAddr ));

    self->port = 0;
}


bool BerkeleySocket_isDisconnected( BerkeleySocket *self )
{
    BaseUI8 buffer[2];

    ANY_REQUIRE( self );

#if !defined(_WINDOWS_)
    return BerkeleySocket_peekInternal( self, buffer, 1, MSG_DONTWAIT ) < 0;
#else
    return BerkeleySocket_peekInternal( self, buffer, 1, 0 ) < 0;
#endif
}


bool BerkeleySocket_isAlive( BerkeleySocket *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    return ! BerkeleySocket_isDisconnected( self );
}


static bool BerkeleySocket_checkUDPClosed( BerkeleySocket *self )
{
#if !defined(__windows__) && !defined(__macos__)
    int ret = 0;

    struct msghdr msg;
    struct sockaddr_in err_addr;
    struct iovec iov;
    char cbuf[768 + 256];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BERKELEYSOCKET_VALID );

    iov.iov_base = cbuf + 256;
    iov.iov_len = 768;
    msg.msg_name = (void *)&err_addr;
    msg.msg_namelen = sizeof( err_addr );
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cbuf;
    msg.msg_controllen = 256;

    ret = recvmsg( self->socketFd, &msg, MSG_ERRQUEUE | MSG_NOSIGNAL );

    return ret >= 0;
#else
    /* TODO: checks for closed UDP socket need to be implemented correctly under Windows */
    ANY_LOG( 1, "Checks for UDP closed stream is not yet supported under Windows", ANY_LOG_WARNING );
    return false;
#endif
}


#ifdef __windows__

static void BerkeleySocket_initializeWinSock( void )
{
  int error;
  WSADATA wsaData;
  WORD wVersionRequested;

  if ( socketInitialized == 0 )
  {
    /* Ask for Winsock 1.1 functionality */
    wVersionRequested = MAKEWORD( 2, 2 );
    error = WSAStartup( wVersionRequested, &wsaData );
    if( error )
    {
       ANY_LOG( 0, "Error %d in WSAStartup()\n", ANY_LOG_ERROR, error );
       return;
    }
  }
  socketInitialized++;
}

#endif


/* EOF */
