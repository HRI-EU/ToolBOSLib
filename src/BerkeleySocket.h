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


#ifndef BERKELEYSOCKET_H
#define BERKELEYSOCKET_H

#include <Any.h>
#include <Base.h>

#define BerkeleySocket_errno() errno

#if !defined(__msvc__) && !defined(__windows__)

#include <errno.h>
#include <netinet/in.h>

#endif

#if defined(__msvc__) || defined(__windows__)
/*
 * As suggested by microsoft that the preferred method for checking
 * socket error code goes through the WSAGetLastError() since
 * errno doesn't collect it
 *
 * see http://msdn.microsoft.com/en-us/library/ms737828(VS.85).aspx
 */
#undef BerkeleySocket_errno
#define BerkeleySocket_errno() WSAGetLastError()

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#include <winsock.h>

/*
 * Below there some posix -> windows mappings
 */
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK       WSAEWOULDBLOCK
#endif

#if !defined(EINPROGRESS)
#define EINPROGRESS       WSAEINPROGRESS
#endif

#if !defined(EALREADY)
#define EALREADY          WSAEALREADY
#endif

#if !defined(ENOTSOCK)
#define ENOTSOCK          WSAENOTSOCK
#endif

#if !defined(EMSGSIZE)
#define EMSGSIZE          WSAEMSGSIZE
#endif

#if !defined(EPROTOTYPE)
#define EPROTOTYPE        WSAEPROTOTYPE
#endif

#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT       WSAENOPROTOOPT
#endif

#if !defined(EPROTONOSUPPORT)
#define EPROTONOSUPPORT   WSAEPROTONOSUPPORT
#endif

#if !defined(ESOCKTNOSUPPORT)
#define ESOCKTNOSUPPORT   WSAESOCKTNOSUPPORT
#endif

#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP        WSAEOPNOTSUPP
#endif

#if !defined(EADDRINUSE)
#define EADDRINUSE        WSAEADDRINUSE
#endif

#if !defined(EADDRNOTAVAIL)
#define EADDRNOTAVAIL     WSAEADDRNOTAVAIL
#endif

#if !defined(ENETDOWN)
#define ENETDOWN          WSAENETDOWN
#endif

#if !defined(ENETUNREACH)
#define ENETUNREACH       WSAENETUNREACH
#endif

#if !defined(ENETRESET)
#define ENETRESET         WSAENETRESET
#endif

#if !defined(ECONNABORTED)
#define ECONNABORTED      WSAECONNABORTED
#endif

#if !defined(ECONNRESET)
#define ECONNRESET        WSAECONNRESET
#endif

#if !defined(ENOBUFS)
#define ENOBUFS           WSAENOBUFS
#endif

#if !defined(EISCONN)
#define EISCONN           WSAEISCONN
#endif

#if !defined(ENOTCONN)
#define ENOTCONN          WSAENOTCONN
#endif

#if !defined(ESHUTDOWN)
#define ESHUTDOWN         WSAESHUTDOWN
#endif

#if !defined(ECONNREFUSED)
#define ECONNREFUSED      WSAECONNREFUSED
#endif

#if !defined(EHOSTDOWN)
#define EHOSTDOWN         WSAEHOSTDOWN
#endif

#if !defined(EHOSTUNREACH)
#define EHOSTUNREACH      WSAEHOSTUNREACH
#endif

#endif


#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * \page BerkeleySocket_About Networking
 *
 * The BerkeleySocket.h handles:
 *
 * \li networking functionality for inter-process communication
 * \li TCP and UDP client and server sockets
 * \li access to raw file-descriptors inside BerkeleySocket structures
 * \li \subpage BerkeleySocket_Options
 * \li \subpage BerkeleySocket_Timeouts
 * \li \subpage BerkeleySocketServer_LoopMacro
 *
 * The library is based on a low level socket abstraction implemented
 * by the BerkeleySocket class used with two other higher level
 * pseudo-classes, the BerkeleySocketServer and the BerkeleySocketClient.
 * The high level classes usually manage the connection and then handover
 * the actual communication tasks to a lower level instance.
 *
 * <center>
 * \image html BerkeleySocket.png
 * </center>
 *
 * <h3>Example:</h3>
 * In the following example a client program connects to a server via
 * an UDP socket. An instance of the class BerkeleySocketClient connects to
 * the server and hands over the management of the connection to an instance
 * of the base BerkeleySocket class.
 *
 * \code
 * ...
 *
 * BerkeleySocketClient *client = (BerkeleySocketClient*)NULL;
 * BerkeleySocket *sock = (BerkeleySocket*)NULL;
 *
 * // Instantiate and initialize the base socket.
 * sock = BerkeleySocket_new();
 * if ( BerkeleySocket_init( sock ) == false )
 * {
 *   ANY_LOG( 0, "Error on BerkeleySocket_init()", ANY_LOG_ERROR );
 *   exit( -1 );
 * }
 *
 * // Instantiate and initialize the high level class.
 * client = BerkeleySocketClient_new();
 * if ( BerkeleySocketClient_init( client, sock ) == false )
 * {
 *   ANY_LOG( 0, "Error on BerkeleySocket_init()", ANY_LOG_ERROR );
 *   exit( -1 );
 * }
 *
 * // Connect to the server.
 * if ( BerkeleySocketClient_connect( client, SOCK_UDP, "192.168.0.1", 60001 ) == NULL )
 * {
 *   ANY_LOG( 5, "Unable to connect to server", ANY_LOG_ERROR );
 * }
 *
 * // Send some data through the low level socket.
 * BerkeleySocket_write( sock, &sockFile, statFile->st_size );
 *
 * ...
 * \endcode
 *
 * \example NetworkUDPClient1.c
 * \example NetworkUDPClient2.c
 * \example NetworkUDPServer1.c
 */


/*! \brief BerkeleySocket default port */
#define BERKELEYSOCKET_PORTNO_DEFAULT 60003

/*! \brief BerkeleySocket default server ip address */
#define BERKELEYSOCKET_SERVERIP_DEFAULT "127.0.0.1"

/*! \brief BerkeleySocket default server max client */
#define BERKELEYSOCKET_MAXCLIENT_DEFAULT 5

/*! \brief BerkeleySocket generic buffer size */
#define BERKELEYSOCKET_BUFFLEN_DEFAULT 255

/*! \brief Default BerkeleySocket timeout */
#define BERKELEYSOCKET_TIMEOUT_DEFAULT BERKELEYSOCKET_TIMEOUT_MSECONDS( 10000 )

/*! \brief Default lingering timeout in seconds */
#define BERKELEYSOCKET_LINGERTIMEOUT_DEFAULT 1


/*! \page BerkeleySocket_Timeouts Timeouts
 * \brief BerkeleySocket timeout macro
 *
 * These macros are provided in order to easily convert hours, minutes,
 * seconds and milliseconds into microseconds as required for timeout values.
 *
 * \code
 *
 *    long timeout = 0L;
 *
 *    // timeout of 5 minutes and 30 seconds
 *    timeout = BERKELEYSOCKET_TIMEOUT_MINUTES( 5 ) + BERKELEYSOCKET_TIMEOUT_SECONDS( 30 );
 * \endcode
 *
 * @{
 */


#define BERKELEYSOCKET_TIMEOUT_HOURS( __hours ) (__hours*60L*60L*1000000L)
#define BERKELEYSOCKET_TIMEOUT_MINUTES( __minutes ) (__minute*60L*1000000L)
#define BERKELEYSOCKET_TIMEOUT_SECONDS( __seconds ) (__seconds*1000000L)
#define BERKELEYSOCKET_TIMEOUT_MSECONDS( __mseconds ) (__mseconds*1000L)

#define BERKELEYSOCKET_INIT_FAILURE -1
#define BERKELEYSOCKET_INIT_UNKNOWN 0
#define BERKELEYSOCKET_INIT_SUCCESS 1

/*!
 * @}
 */

/*!
 * \brief Enums all the socket types supported by the interface
 *
 * The user have to use it on \a BerkeleySocketServer_connect(), \a BerkeleySocketClient_connect()
 * and \a BerkeleySocket_getType()
 */
typedef enum BerkeleySocketType
{
    BERKELEYSOCKET_NULL = 0, /**< No sockect */
            BERKELEYSOCKET_TCP, /**< TCP BerkeleySocket */
            BERKELEYSOCKET_UDP, /**< UDP BerkeleySocket */
            BERKELEYSOCKET_FD       /**< File descriptor BerkeleySocket */
}
        BerkeleySocketType;

/*!
 * \page BerkeleySocket_Options Socket options
 *
 * These macros are used to set, get and reset a socket options. Generally
 * you should hardly use these macros, instead call the
 * \a BerkeleySocket_setOptions() if \a BERKELEYSOCKET_OPTION_SET and/or
 * \a BERKELEYSOCKET_OPTION_RESET are used. That's because these macros
 * set/reset some internal flags but don't perform the proper
 * BerkeleySocket's setting.
 *
 * \attention Do not call \a BerkeleySocket_setOptions() when the macro
 *            \a BERKELEYSOCKET_OPTION_GET() has been used, because the
 *            macro reads the flag status and thus will return its
 *            corresponding boolean value.
 *
 * \code
 *    BerkeleySocket *sock = (BerkeleySocket*)NULL;
 *    ....
 *
 *    // set a BerkeleySocket option
 *    BERKELEYSOCKET_OPTION_SET( sock, NAGLE );
 *    BerkeleySocket_setOptions( sock );
 *
 *    // get a BerkeleySocket option
 *    if ( BERKELEYSOCKET_OPTION_GET( sock, NAGLE ) == true )
 *    {
 *       ANY_LOG( 0, "TCP Nagle algorithm is set correctly", ANY_LOG_INFO );
 *    }
 *
 *    ... other code ...
 *
 *    // reset a BerkeleySocket option
 *    BERKELEYSOCKET_OPTION_RESET( sock, NAGLE );
 *    BerkeleySocket_setOptions( sock );
 * \endcode
 *
 * @{
 */

/*! \brief Returns \a true if the socket option is set, \a false otherwise
 * \param __self BerkeleySocket instance pointer
 * \param __name BerkeleySocket option name
 *
 * \a return Returns \a true if the option \a __name is set, \a false otherwise
 */
#define BERKELEYSOCKET_OPTION_GET( __self, __name ) ( __self->options & BERKELEYSOCKET_OPTION_##__name ? true : false )

/*! \brief Sets an option
 * \param __self BerkeleySocket instance pointer
 * \param __name BerkeleySocket option name
 *
 * \return Nothing
 */
#define BERKELEYSOCKET_OPTION_SET( __self, __name ) __self->options |= BERKELEYSOCKET_OPTION_##__name

/*! \brief Resets a BerkeleySocket option
 * \param __self BerkeleySocket instance pointer
 * \param __name BerkeleySocket option name
 *
 * \return Nothing
 */
#define BERKELEYSOCKET_OPTION_RESET( __self, __name ) __self->options &= ~BERKELEYSOCKET_OPTION_##__name

#define BERKELEYSOCKET_OPTION_MAX_SNDBUFSIZE  (64*1024)
#define BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE  (64*1024)

/*!
 * @}
 */

/*! \brief define an BerkeleySocket option
 * \param __name BerkeleySocket option name
 * \param __bit Bit position
 *
 * This macro is used only on BerkeleySocketOption declaration
 */
#define BERKELEYSOCKET_DEFINE_OPTION( __name, __bit ) BERKELEYSOCKET_OPTION_##__name = ( 1 << __bit )

/*! \brief BerkeleySocket options
 *
 * All the enumeration constant are used internally by BerkeleySocket library, the user shouldn't
 * have to use it directly they may changes at any time
 */
typedef enum BerkeleySocketOption
{
    BERKELEYSOCKET_DEFINE_OPTION( NULL, 0 ),               /**< No BerkeleySocket Options */
    BERKELEYSOCKET_DEFINE_OPTION( BLOCKING, 1 ),           /**< Blocking BerkeleySocket if set */
    BERKELEYSOCKET_DEFINE_OPTION( NAGLE, 2 ),               /**< TCP Nagle algorithm if set */
    BERKELEYSOCKET_DEFINE_OPTION( REUSEADDR, 3 ),          /**< Reuse the TCP address if set */
    BERKELEYSOCKET_DEFINE_OPTION( KEEPALIVE, 4 ),          /**< Keep Alive the remote if set */
    BERKELEYSOCKET_DEFINE_OPTION( OOBINLINE, 5 ),          /**< Send Out-of-Band data if set */
    BERKELEYSOCKET_DEFINE_OPTION( DONTROUTE, 6 ),          /**< Don't route the packet when send it if set */
    BERKELEYSOCKET_DEFINE_OPTION( RCVBUFFSIZE, 7 ),        /**< User define receive buffer size */
    BERKELEYSOCKET_DEFINE_OPTION( SNDBUFFSIZE, 8 ),        /**< User define send buffer size */
    BERKELEYSOCKET_DEFINE_OPTION( LINGER, 9 ),             /**< User define lingering on BerkeleySocket close */
    BERKELEYSOCKET_DEFINE_OPTION( IPTOSLOWDELAY,
                                  10 ),     /**< IP are set to low delay for interactive or realtime application */
    BERKELEYSOCKET_DEFINE_OPTION( IPTOSTHROUGHPUT,
                                  11 ),   /**< IP are set to throughput for interactive or realtime application */
    BERKELEYSOCKET_DEFINE_OPTION( CORK,
                                  12 ),              /**< Speedup a packet read/write filling up a full MTU packet */
    BERKELEYSOCKET_DEFINE_OPTION( CLOSEONDISCONNECT, 13 ), /**< Always closes the file descriptor on disconnected */
    BERKELEYSOCKET_DEFINE_OPTION( BUFFEREDWRITEBULK, 14 ), /**< Always preferred buffered/soft write bulk */
    BERKELEYSOCKET_DEFINE_OPTION( BROADCAST, 15 ),         /**< Put the UDP socket in broadcast mode */
    BERKELEYSOCKET_DEFINE_OPTION( CLOSEONEXEC, 16 )        /**< Close the socket on exec */
}
        BerkeleySocketOption;

#if defined(__windows__)
typedef SOCKET BerkeleySocketHandle;

#define BERKELEYSOCKETHANDLE_INVALID    INVALID_SOCKET
#define BERKELEYSOCKET_ERROR            SOCKET_ERROR

#else
typedef int BerkeleySocketHandle;

#define BERKELEYSOCKETHANDLE_INVALID    -1
#define BERKELEYSOCKET_ERROR            -1

#endif


typedef struct BerkeleySocket
{
    unsigned long valid;
    BerkeleySocketType type;
    struct sockaddr_in sourceAddr;
    struct sockaddr_in remoteAddr;
    BerkeleySocketHandle socketFd;
    int options;
    long connectTimeout;
    long readTimeout;
    long writeTimeout;
    unsigned int mss;
    unsigned int rcvBuffSize;
    unsigned int sndBuffSize;
    int lingerTimeout;
    unsigned int writeBulkBufferSize;
    int port;
}
BerkeleySocket;

BerkeleySocket *BerkeleySocket_new( void );

/*!
 * \brief Initializes a new BerkeleySocket
 * \param self Pointer to the BerkeleySocket instance
 *
 * \return Always returns BERKELEYSOCKET_INIT_SUCCESS.
 */
int BerkeleySocket_init( BerkeleySocket *self );

long BerkeleySocket_getConnectTimeout( BerkeleySocket *self );

long BerkeleySocket_getIsReadPossibleTimeout( BerkeleySocket *self );

long BerkeleySocket_getIsWritePossibleTimeout( BerkeleySocket *self );

int BerkeleySocket_getLingerTimeout( BerkeleySocket *self );

void BerkeleySocket_setDefaultTimeout( BerkeleySocket *self, long usecs );

void BerkeleySocket_setConnectTimeout( BerkeleySocket *self, long usecs );

void BerkeleySocket_setIsWritePossibleTimeout( BerkeleySocket *self, long usecs );

void BerkeleySocket_setIsReadDataAvailableTimeout( BerkeleySocket *self, long usecs );

void BerkeleySocket_setReadWriteTimeout( BerkeleySocket *self, long rusecs, long wusecs );

void BerkeleySocket_setBlocking( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the TCP_NODELAY (Nagle algorithm) socket's option
 *
 * This function set the Nagle Algorithm for the specified BerkeleySocket. if the user
 * disable it, the TCP/IP stack will send immediatly the writted data on
 * a BerkeleySocket without waiting to fillup the internal send buffer or the send buffer
 * timeout expires. This will increase the network traffic because smaller packet
 * are put on the wire
 */
void BerkeleySocket_setTcpNoDelay( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the TCP_CORK socket's option
 *
 * This function sets or resets (if the Operating System supports it) the BerkeleySocket corked option.
 * Setting up this BerkeleySocket option, allows an application to tell the TCP stack not to send data
 * until a full packet, generally 1500 bytes on 10/100/1000MBit/s ethernet interface and from
 * 1,5 to 8Kbytes on fibre channel, has been collected. An application can "cork" a TCP connection
 * by enabling this option, and afterwards compose an outgoing message with several \a BerkeleySocket_write()
 * calls. When a full packet is filled than the system will put the packet on the wire.
 *
 * Generally a "corked" BerkeleySocket is usefull when sending bulk data like images of serveral megabytes
 * in size.
 *
 * Use \a BerkeleySocket_writeBulk() to further speed up via bulk transfers.
 */
void BerkeleySocket_setTcpCork( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the IPTOS_LOWDELAY BerkeleySocket's option
 *
 * Setting this option on a BerkeleySocket generally causes packets on that socket to be sent out before others.
 * This option is usefull for all the data which are required to be sent on wire with the minimun delay
 * like interactive or realtime applications
 */
void BerkeleySocket_setIpTosLowDelay( BerkeleySocket *self, bool stat );

/*!
 * \brief Close the BerkeleySocket file descriptor on exec
 */
void BerkeleySocket_setCloseOnExec( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the IPTOS_THROUGHPUT BerkeleySocket's option
 *
 * This option generally improves the throughput.
 */
void BerkeleySocket_setIpTosThroughput( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the TCP_MAXSEG (Max Transmission Unit, MSS) segment size option
 */
void BerkeleySocket_setTcpMss( BerkeleySocket *self, unsigned int mss );

/*!
 * \brief Sets the SO_REUSEADDR socket's option
 *
 * This function tells a BerkeleySocket to reuse the local address where the BerkeleySocket will
 * be connected. Normally the Operating System doesn't permit to reuse the same
 * local address. This option is usefull when coding a TCP server that accept
 * more than one clients. This option must be setup before to connect a BerkeleySocket
 * to a local address by the \a BerkeleySocketServer_connect() call.
 *
 * This option couldn't be set on normal UDP socket because an UDP socket
 * is alway connectionless so it doesn't have any fixed local address. However some Operating
 * Systems could permit to reuse the local address for UDP BerkeleySockets but only if connected on
 * local multicast address.
 *
 * This function set the socket's lingering (SO_LINGER) to the current lingering timeout value for
 * the given socket in order to reduce the timeout before the socket flushes its buffers and freeing
 * the connected port. If you want to change the socket lingering timeout, the BerkeleySocket_setLinger()
 * need to be called before to close the BerkeleySocket.
 */
void BerkeleySocket_setReuseAddr( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the SO_KEEPALIVE socket's option
 *
 * This option set a periodic transmission of keepalive packet on connected BerkeleySocket
 * when no other data is being exchanged.
 */
void BerkeleySocket_setKeepAlive( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the SO_OOBINLINE (urgent data) socket's option
 *
 * This option specifies that Out-of-Band data (urgent) should be placed in
 * the normal BerkeleySocket queue. The user receives urgent data using the normal \a BerkeleySocket_read()
 * call instead of \a BerkeleySocket_readUrgent() call
 */
void BerkeleySocket_setOobinline( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the SO_DONTROUTE socket's option
 *
 * This option specifies that all the outgoing data must bypass the normal TCP/IP routing
 * decisions. The packet will be directed to the appropriate interface as specified
 * by the network portion of the destination address.
 */
void BerkeleySocket_setDontroute( BerkeleySocket *self, bool stat );

/*!
 * \brief Sets the SO_LINGER socket's option
 *
 * This option determines what to do when unsent data exist for a BerkeleySocket when
 * the program execute a \a BerkeleySocket_disconnect(). By default the \a BerkeleySocket_disconnect()
 * returns immediately and the system attempts to deliver any unset data. But if the
 * user sets the linger at \a true and \a secs > 0 than the \a BerkeleySocket_disconnect() waits
 * the specified timeout before to discard all the unsent data. If the timeout is = 0 than
 * all the unsent data are discarded and the \a BerkeleySocket_disconnect() returns immediately.
 *
 * \note Setting socket's lingering to 0 might lead to problems like 'connection reset by the peer'
 *       if one of the end-points is Windows. In such cases the solution might be to increase the lingering timeout
 */
void BerkeleySocket_setLinger( BerkeleySocket *self, bool stat, int secs );

/*!
 * \brief Sets the socket's send buffer size
 *
 * This option sets the BerkeleySocket send buffer size. In general larger buffers
 * may improve the network performance. This value cannot be larger than
 * \a BERKELEYSOCKET_OPTION_MAX_SNDBUFSIZE which is system dependent.
 */
void BerkeleySocket_setSndBuffer( BerkeleySocket *self, unsigned int size );

/*!
 * \brief Sets the socket's receive buffer size
 *
 * This option sets the BerkeleySocket receive buffer size. In general larger buffers
 * may improve the network performance. This value cannot be larger than
 * \a BERKELEYSOCKET_OPTION_MAX_RCVBUFSIZE which is system dependent.
 */
void BerkeleySocket_setRcvBuffer( BerkeleySocket *self, unsigned int size );

/*!
 * \brief Set UDP error notification on the BerkeleySocket
 *
 * This function set or reset the error reporting on reading/writing, for the given UDP socket.
 * By default all the UDP socket will have such feature on. The user can choose to reset it, and
 * in such case the behaviours on writing/reading UDP datagram on that socket will not be notified
 * to the user's application. This means that all the BerkeleySocket_write() and related will not
 * return an error in case the other side isn't reachable. The system will reports the related ICMP
 * notification like "port unreachable" or "connection refused".
 */
void BerkeleySocket_setIpRcvError( BerkeleySocket *self, bool stat );

void BerkeleySocket_setBroadcast( BerkeleySocket *self, bool stat, int port );

/*!
 * \brief Close the BerkeleySocket file descriptor on disconnect
 */
void BerkeleySocket_setCloseOnDisconnect( BerkeleySocket *self, bool stat );

/*!
 * \brief Set the BerkeleySocket_writeBulk() for software buffering instead of system strategy
 *
 * This BerkeleySocket options sets the user's preferences for the BerkeleySocket_writeBulk() function.
 * When the user sets the option as \a true, the \a BerkeleySocket_writeBulk() will be setted to
 * use a buffered version of data transfers instead to use the fastest operative system
 * routines. Also when the user specify the \a bufferSize parameter the BerkeleySocket_writeBulk() will use
 * chucks of \a bufferSize blocks when reading from the source and writing to the destination.
 * This function become usefull if the application need to be totally compatible when performing
 * bulk data transfers and the operative system doesn't support direct streaming from socket to other
 * destination.
 */
void BerkeleySocket_setBufferedWriteBulk( BerkeleySocket *self, bool stat, int bufferSize );

/*!
 * \brief get the IP-address of a host
 * \param hostName
 * \param buff Pointer to a buffer where to store the converted address
 * \param buffsize Buffer's size
 *
 * Convert a hostname into an numerical string of ip address
 *
 * \code
 *     char ipaddress[128];
 *
 *     BerkeleySocket_host2Addr( "localhost", ipaddress, 128 );
 *
 *     ANY_LOG( 0, "localhost ip address is %s", ANY_LOG_INFO, ipaddress );
 * \endcode
 *
 * \return Return a pointer on the Ip-address as an numerical string
 */
char *BerkeleySocket_host2Addr( const char *hostName, char *buff, int buffsize );

BerkeleySocketHandle BerkeleySocket_connectToFd( BerkeleySocket *self, int fd );

/*!
 * \brief Disconnect a BerkeleySocket
 *
 * This function disconnect or close a BerkeleySocket connection. The modality how it's
 * done depends by \a BerkeleySocket_setLinger() option. Every BerkeleySocket that has been connected
 * by \a BerkeleySocket_connectToFd() will not be closed automatically. The user has to
 * manually close the file descriptor or set the \a BerkeleySocket_setCloseOnDisconnect() option
 * to close it automatically the file descriptor on disconnect.
 *
 * \return Return 0 if no errors occure during the shutdown else -1
 */
int BerkeleySocket_disconnect( BerkeleySocket *self );

int BerkeleySocket_write( BerkeleySocket *self, BaseUI8 *poWriteBuffer, size_t bufferSize );

int BerkeleySocket_writeBlock( BerkeleySocket *self, BaseUI8 *poWriteBuffer, int bufferSize );

int BerkeleySocket_writeUrgent( BerkeleySocket *self, BaseUI8 *poWriteBuffer, int bufferSize );

/*!
 * \brief Copies directly data from source socket to a destination socket
 *
 * This function copies data from the source socket to the destination socket
 * using directly the operative system features and avoiding to copy data in userspace.
 */
int BerkeleySocket_writeBulk( BerkeleySocket *self, BerkeleySocket *source, int count );

/*!
 * \brief Receive data from a connected host
 *
 * Reading content that is provided by a given BerkeleySocket. Also the BerkeleySocket_read() stores
 * the remote address/port after the data has been readed if the BerkeleySocket is UDP. This become usefull
 * when the user had to reply at the same remote address/port, in this case the user didn't have
 * to use the BerkeleySocket_setRemoteAddr() before to send any data.
 *
 * \return Return how many bytes are read, if it return 0 the socket is closed for reading
 *         -1 will be returned in case of failure
 */
int BerkeleySocket_read( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize );

int BerkeleySocket_readBlock( BerkeleySocket *self, BaseUI8 *poReadBuffer, int size );

int BerkeleySocket_readUrgent( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize );

/*!
 * \brief Receive data from a connected host without removing the data from the queue
 */
int BerkeleySocket_peek( BerkeleySocket *self, BaseUI8 *poReadBuffer, int bufferSize );

bool BerkeleySocket_isReadDataAvailable( BerkeleySocket *self );

bool BerkeleySocket_isWritePossible( BerkeleySocket *self );

int BerkeleySocket_getFd( BerkeleySocket *self );

char *BerkeleySocket_inetntoa( struct in_addr in, char *buf, int bufsize );

/*!
 * \brief Return network-format internet address given the base 256 d.d.d.d
          representation
 * \param buf string to be converted
 * \param in pointer to struct in_addr object that receives the output
 *
 * \return Return 0 on error, non-zero on success
 */
int BerkeleySocket_inetaton( const char *buf, struct in_addr *in );

/*!
 * \brief Returns the specified error string
 * \param errorcode The specific error code
 * \param buffer Pointer where to store the error string
 * \param len Buffer length
 *
 * This function could be called by the user in order to get the specific
 * error string. It uses the system's error numbers included from
 * errno.h on all platforms.
 *
 * \code
 *         ....
 *         if ( BerkeleySocket_write( sock, buffer, bufferSize ) == -1 )
 *         {
 *            char errStr[128];
 *
 *            BerkeleySocket_strerror( BerkeleySocket_errno(), errStr, 128 );
 *            ANY_LOG( 5, "Error on BerkeleySocket_write(): %s", ANY_LOG_ERROR, errStr );
 *         }
 *         ....
 * \endcode
 *
 * The function returns on \a buffer the specific error string
 */
void BerkeleySocket_strerror( int errorcode, char *buffer, int len );

/*!
 * \brief For internal use only.
 */
void BerkeleySocket_setOptions( BerkeleySocket *self );

BerkeleySocketType BerkeleySocket_getType( BerkeleySocket *self );

void BerkeleySocket_setRemoteAddr( BerkeleySocket *self, char *serverHostAddr, int serverPortNo );

/*!
 * \brief Returns the remote address and port from the last read for UDP BerkeleySocket
 * \param self BerkeleySocket instance pointer
 * \param serverHostAddr Remote IP address
 * \param bufferSize \a serverHostAddr buffer size
 * \param serverPortNo Remote port number
 *
 * This function returns the remote IP address and port number stored by the \a BerkeleySocketClient_connect()
 * or \a BerkeleySocketServer_connect() if successfully done. On UDP BerkeleySocket the last \a BerkeleySocket_read(),
 * \a BerkeleySocket_readUrgent() and \a BerkeleySocket_peek() call stores the sender address/port which can be retrieved
 * by this function call
 */
void BerkeleySocket_getRemoteAddr( BerkeleySocket *self,
                                   char *serverHostAddr,
                                   int bufferSize,
                                   int *serverPortNo );

/*!
 * \brief Clone a BerkeleySocket
 * \param self BerkeleySocket instance pointer
 * \param destBerkeleySocket Pointer to a destination socket
 *
 * This function clones a BerkeleySocket from the \a self BerkeleySocket to the \a destBerkeleySocket.
 */
void BerkeleySocket_clone( BerkeleySocket *self, BerkeleySocket *destBerkeleySocket );

/*!
 * \brief Clone all the source properties into a destionation socket
 * \param self BerkeleySocket instance pointer
 * \param destBerkeleySocket Pointer to a destination socket
 *
 * This function clones the BerkeleySocket properties, like options and timeouts, from the \a self BerkeleySocket
 * to the \a destBerkeleySocket.
 */
void BerkeleySocket_cloneProperties( BerkeleySocket *self, BerkeleySocket *destBerkeleySocket );

/*!
 * \brief Returns the remote ip address on a given BerkeleySockets
 * \param self BerkeleySocket instance pointer
 * \param buff Buffer where to store the ip address
 * \param buffsize Buffer size
 * \return Returns the remote ip address
 */
char *BerkeleySocket_getRemoteIp( BerkeleySocket *self, char *buff, int buffsize );

bool BerkeleySocket_isDisconnected( BerkeleySocket *self );

bool BerkeleySocket_isAlive( BerkeleySocket *self );

/*!
 * \brief Get the current write-status
 * \param self BerkeleySocket instance pointer
 *
 * Returns a boolean value, whether or not writing to the socket is currently
 * possible. This is the primary function that should be used to test if
 * writing is basically possible, and all buffer's are ready.
 *
 * \return true if write is allowed, else false
 */
bool BerkeleySocket_getWriteStatus( BerkeleySocket *self );

/*!
 * \brief Get the current read-status
 * \param self BerkeleySocket instance pointer
 *
 * Returns a boolean value, whether or not reading from the socket is currently
 * possible. This is the primary function that should be used to test if
 * read is basically possible, and data is available.
 *
 * \return true if write is allowed, else false
 */
bool BerkeleySocket_getReadStatus( BerkeleySocket *self );

void BerkeleySocket_clear( BerkeleySocket *self );

void BerkeleySocket_delete( BerkeleySocket *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
