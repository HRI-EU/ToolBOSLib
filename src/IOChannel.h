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


/*!
 * \page IOChannel_About IOChannel
 *
 * <h2>Overview</h2>
 *
 * The IOChannel (IOChannel.h) library provides a set of functions to manage
 * generic streams. These functions allow users to perform operations that
 * are typically done on file streams (binary streaming, formatted I/O, ...)
 * also on generic resources such as memory and sockets.
 *
 * <h3>Example</h3>
 * \code
 * IOChannel *stream = (IOChannel*)NULL;
 * stream = IOChannel_new();
 * IOChannel_init( stream );
 *
 * IOChannel_openFromString( stream, "File://output.txt" );
 *
 * IOChannel_printf( stream, "Hello, World!\n" );
 *
 * IOChannel_close(stream);
 * IOChannel_clear( stream );
 * IOChannel_delete( stream );
 * \endcode
 *
 * <center>
 * \image html IOChannel.png
 * </center>
 *
 * <h2>Opening a stream</h2>
 *
 * <h3>About info strings</h3>
 *
 * \li space-separated key=value pairs
 * \li undefined keys are ignored
 * \li order does not matter
 * \li upon multiple occurrence, the last one is significant
 *
 * <h3>Access modes and flags</h3>
 *
 * When you open a stream you have to set an access mode. You can choose
 * between either of them:
 * \li \c IOCHANNEL_MODE_R_ONLY (read only)
 * \li \c IOCHANNEL_MODE_W_ONLY (write only)
 * \li \c IOCHANNEL_MODE_RW (read + write)
 *
 * You may \c OR the access mode with any combination of the following flags:
 * \li \c IOCHANNEL_MODE_CREAT (create resource if not existing)
 * \li \c IOCHANNEL_MODE_TRUNC (overwrite existing files)
 * \li \c IOCHANNEL_MODE_APPEND (append at end of existing content)
 *
 * Additionally you may enforce:
 * \li \c IOCHANNEL_MODE_CLOSE (flush resource and close)
 * \li \c IOCHANNEL_MODE_NOTCLOSE (neither flush nor close)
 *
 * <table>
 *   <tr>
 *     <th>stream type</th>
 *     <th>infoString example</th>
 *     <th>accepted parameters</th>
 *     <th>notes</th>
 *   </tr>
 *
 *   <tr>
 *     <td colspan="4" style="text-align: center;"><h3>mostly used</h3></td>
 *   </tr>
 *   <tr>
 *     <td>regular file</td>
 *     <td>File://filename.dat</td>
 *     <td>
 *        name = %%s<br>
 *        mode = 'IOCHANNEL_MODE_RW | IOCHANNEL_MODE_CREAT'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>TCP socket</td>
 *     <td>Tcp://192.168.0.52:60008</td>
 *     <td>
 *        host = %%s<br>
 *        port = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>UDP socket</td>
 *     <td>Udp://192.168.0.52:60008</td>
 *     <td>
 *        host = %%s<br>
 *        port = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>TCP server socket</td>
 *     <td>ServerTcp://6001</td>
 *     <td>
 *        port = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'<br>
 *        waitClientTimeout = %%s [usec]<br>
 *        lingerTimeout = %%s<br>
 *        reuseAddr = %%s
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>UDP server socket</td>
 *     <td>ServerUdp://60001</td>
 *     <td>
 *        port = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>memory pointer</td>
 *     <td>Mem://</td>
 *     <td>
 *        pointer = %%p<br>
 *        size = %%ld<br>
 *        mode = 'IOCHANNEL_MODE_R_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CREAT: allocates the specified size of memory,
 *        pass "pointer=0" in such case.
 *
 *        IOCHANNEL_MODE_TRUNC: fill memory with zeros
 *
 *        IOCHANNEL_MODE_CLOSE: free memory when closing, by default it
 *        won't be freed
 *     </td>
 *   </tr>
 *
 *   <tr>
 *     <td colspan="4" style="text-align: center;"><h3>console I/O</h3></td>
 *   </tr>
 *   <tr>
 *     <td>stdin</td>
 *     <td>StdIn://</td>
 *     <td>
 *        mode = 'IOCHANNEL_MODE_R_ONLY'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>stdout</td>
 *     <td>StdOut://</td>
 *     <td>
 *        mode = 'IOCHANNEL_MODE_W_ONLY'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *   <tr>
 *     <td>stderr</td>
 *     <td>StdErr://</td>
 *     <td>
 *        mode = 'IOCHANNEL_MODE_W_ONLY'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 *
 *   <tr>
 *     <td colspan="4" style="text-align: center;"><h3>legacy code
 *         integration: work with low-level FILE*- and fd-based streams</h3></td>
 *   </tr>
 *   <tr>
 *     <td>file handle (FILE*)</td>
 *     <td>AnsiFILE://</td>
 *     <td>
 *        pointer = %%p (already opened FILE* handle)<br>
 *        mode    = 'IOCHANNEL_MODE_RW | IOCHANNEL_MODE_TRUNC'<br>
 *        perm    = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CREAT: not supported
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>low-level file descriptor (int)</td>
 *     <td>Fd://</td>
 *     <td>
 *        key  = %%d (already opened fd file handle)<br>
 *        mode = 'IOCHANNEL_MODE_RW | IOCHANNEL_MODE_TRUNC'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CREAT: not supported
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>low-level socket fd (int)</td>
 *     <td>Socket://</td>
 *     <td>
 *        pointer = %%p<br>
 *        mode    = 'IOCHANNEL_MODE_RW | IOCHANNEL_MODE_TRUNC'<br>
 *        perm    = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CREAT: not supported<p>
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>memory-mapped file descriptor (int)</td>
 *     <td>MemMapFd://</td>
 *     <td>
 *        size = %%ld<br>
 *        key  = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW | IOCHANNEL_MODE_TRUNC'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CREAT: not supported<p>
 *
 *        IOCHANNEL_MODE_CLOSE: unmap the fd when closing, by default it
 *        is kept
 *     </td>
 *   </tr>
 *
 *   <tr>
 *     <td colspan="4" style="text-align: center;"><h3>special use cases</h3></td>
 *   </tr>
 *   <tr>
 *     <td>shared memory</td>
 *     <td>Shm://</td>
 *     <td>
 *        size = %%ld<br>
 *        key  = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        IOCHANNEL_MODE_CLOSE: unmap the shared memory when closing,
 *        by default it is kept
 *
 *        Use ftok() to create a SysV IPC key.
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>named shared memory</td>
 *     <td>Shm:///shmName</td>
 *     <td>
 *        size = %%ld<br>
 *        key  = %%d<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td>
 *        Named shared memories appear as files under /dev/shm.
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>Null stream</td>
 *     <td>Null://</td>
 *     <td>ignored</td>
 *     <td>
 *        Never reads any bytes, hence IOChannel_scanf() with pattern
 *        matching will always fail.
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>randomizer</td>
 *     <td>Rand://Integers</td>
 *     <td>
 *        key  = %%d (seed)<br>
 *        name = {Integers,Floats,Chars,Printables}<br>
 *        mode = IOCHANNEL_MODE_R_ONLY<br>
 *        perm = IOCHANNEL_PERMISSIONS_ALL
 *     </td>
 *     <td>
 *        When calling IOChannel_scanf() random data (as specified using
 *        'name') are returned.<p>
 *
 *        'Chars' randomizes any character in the ASCII set whileas
 *        'Printables' omits whitespaces etc.
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>pipe to other process</td>
 *     <td>PipeCmd://</td>
 *     <td>
 *        name = %%s (command to execute)<br>
 *        mode = 'IOCHANNEL_MODE_R_ONLY'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'<br>
 *     </td>
 *     <td>
 *        Opens a process by creating a pipe, forking, and invoking the
 *        shell (/bin/sh with -c flag).<p>
 *
 *        IOCHANNEL_MODE_RW: not supported (pipes are unidirectional)
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>RTBOS VFS connection</td>
 *     <td>RTBOS://localhost:2000/bBDMBlockF32@Binary</td>
 *     <td>
 *        host = %%s<br>
 *        port = %%d<br>
 *        data = &lt;instanceName&gt;<br>
 *        format = {Ascii,Binary}<br>
 *        retry = %%d (number of connection attempts)<br>
 *        retryTimeout = %%d (wait time before next retry [ms])<br>
 *        mode = 'IOCHANNEL_MODE_RW'<br>
 *        perm = 'IOCHANNEL_PERMISSIONS_ALL'
 *     </td>
 *     <td><br></td>
 *   </tr>
 * </table>
 *
 * <h3>infoString examples</h3>
 * \code
   // connect to remote host
   "Tcp://192.168.0.1:2000"

   // listen for incoming connections
   "ServerTcp://2000 waitClientTimeout=10000000 reuseAddr=1"

   // allocate memory chunk and free at close
   "Mem:// ptr=0 size=1024 mode='IOCHANNEL_MODE_RW | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_CLOSE'"

   // retrieve command output:
   "PipeCmd://'ls -lh'"
   \endcode
 *
 */

#ifndef IOCHANNEL_H
#define IOCHANNEL_H


#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

#if !defined(__windows__)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#endif

#include <Any.h>
#include <MTList.h>


#if defined(__cplusplus)
extern "C" {
#endif


#define IOCHANNEL_STREAMNAME_MAXLEN           (   20 )
#define IOCHANNEL_INFOSTRING_MAXLEN           ( 1024 )
#define IOCHANNEL_NUMOFPLUGINS_MAX            (  128 )

#define IOCHANNEL_MODE_R_ONLY                 O_RDONLY
#define IOCHANNEL_MODE_W_ONLY                 O_WRONLY
#define IOCHANNEL_MODE_RW                     O_RDWR
#define IOCHANNEL_MODE_UNDEFINED              EOF
#define IOCHANNEL_MODE_CREAT                  O_CREAT
#define IOCHANNEL_MODE_TRUNC                  O_TRUNC
#define IOCHANNEL_MODE_APPEND                 O_APPEND
#define IOCHANNEL_MODE_CLOSE                  0x80000000
#define IOCHANNEL_MODE_NOTCLOSE               0x40000000


#if defined(S_IRUSR)
#define IOCHANNEL_PERMISSIONS_R_U             S_IRUSR
#else
#define IOCHANNEL_PERMISSIONS_R_U             0
#endif

#if defined(S_IWUSR)
#define IOCHANNEL_PERMISSIONS_W_U             S_IWUSR
#else
#define IOCHANNEL_PERMISSIONS_W_U             0
#endif

#if defined(S_IXUSR)
#define IOCHANNEL_PERMISSIONS_X_U             S_IXUSR
#else
#define IOCHANNEL_PERMISSIONS_X_U             0
#endif

#if defined(S_IRWXU)
#define IOCHANNEL_PERMISSIONS_RWX_U           S_IRWXU
#else
#define IOCHANNEL_PERMISSIONS_RWX_U           0
#endif

#if defined(S_IRGRP)
#define IOCHANNEL_PERMISSIONS_R_G             S_IRGRP
#else
#define IOCHANNEL_PERMISSIONS_R_G             0
#endif

#if defined(S_IWGR)
#define IOCHANNEL_PERMISSIONS_W_G             S_IWGRP
#else
#define IOCHANNEL_PERMISSIONS_W_G             0
#endif

#if defined(S_IXGRP)
#define IOCHANNEL_PERMISSIONS_X_G             S_IXGRP
#else
#define IOCHANNEL_PERMISSIONS_X_G             0
#endif

#if defined(S_IRWXG)
#define IOCHANNEL_PERMISSIONS_RWX_G           S_IRWXG
#else
#define IOCHANNEL_PERMISSIONS_RWX_G           0
#endif

#if defined(S_IROTH)
#define IOCHANNEL_PERMISSIONS_R_O             S_IROTH
#else
#define IOCHANNEL_PERMISSIONS_R_O             0
#endif

#if defined(S_IWOTH)
#define IOCHANNEL_PERMISSIONS_W_O             S_IWOTH
#else
#define IOCHANNEL_PERMISSIONS_W_O             0
#endif

#if defined(S_IXOTH)
#define IOCHANNEL_PERMISSIONS_X_O             S_IXOTH
#else
#define IOCHANNEL_PERMISSIONS_X_O             0
#endif

#if defined(S_IRWXO)
#define IOCHANNEL_PERMISSIONS_RWX_O           S_IRWXO
#else
#define IOCHANNEL_PERMISSIONS_RWX_O           0
#endif

#define IOCHANNEL_PERMISSIONS_ALL             (IOCHANNEL_PERMISSIONS_RWX_U|\
                                               IOCHANNEL_PERMISSIONS_RWX_G|\
                                               IOCHANNEL_PERMISSIONS_RWX_O)

#define IOCHANNELWHENCE_SET                   SEEK_SET
#define IOCHANNELWHENCE_CUR                   SEEK_CUR
#define IOCHANNELWHENCE_END                   SEEK_END



struct IOChannel;
struct IOChannelBuffer;
struct IOChannelInterface;
struct IOChannelReferenceValue;
typedef int IOChannelMode;
typedef int IOChannelPermissions;
typedef int IOChannelWhence;


typedef void *(IOChannelNew)( void );

typedef bool (IOChannelInit)( struct IOChannel * );

typedef bool (IOChannelOpen)( struct IOChannel *, char *,
                              IOChannelMode, IOChannelPermissions, va_list );

typedef bool (IOChannelOpenFromString)( struct IOChannel *, struct IOChannelReferenceValue ** );

typedef long (IOChannelRead)( struct IOChannel *, void *, long );

typedef long (IOChannelWrite)( struct IOChannel *, const void *, long );

typedef long (IOChannelFlush)( struct IOChannel * );

typedef long long (IOChannelSeek)( struct IOChannel *, long long, IOChannelWhence );

typedef bool (IOChannelClose)( struct IOChannel * );

typedef void *(IOChannelGetProperty)( struct IOChannel *, const char * );

typedef bool (IOChannelSetProperty)( struct IOChannel *, const char *, void * );

typedef void (IOChannelClear)( struct IOChannel * );

typedef void (IOChannelDelete)( struct IOChannel * );

typedef long (IOChannelCallBack)( void *, struct IOChannel *, bool );


#define IOCHANNELINTERFACE_OPTIONS( __streamName )\
  IOChannel##__streamName##Ops

#define IOCHANNELINTERFACE_DECLARE_OPTIONS( __streamName )\
  IOChannelInterface IOCHANNELINTERFACE_OPTIONS( __streamName )

#define IOCHANNELINTERFACE_CREATE_PLUGIN( __streamName )\
  IOCHANNELINTERFACE_DECLARE( __streamName );\
  IOCHANNELINTERFACE_DECLARE_OPTIONS( __streamName ) =\
  IOCHANNELINTERFACE_CREATE( __streamName )

#define IOCHANNELINTERFACE_DECLARE( __streamName )\
static void*      IOChannel##__streamName##_new( void );\
static bool       IOChannel##__streamName##_init( struct IOChannel* );\
static bool       IOChannel##__streamName##_open( struct IOChannel*, char*, IOChannelMode, IOChannelPermissions, va_list );\
static bool       IOChannel##__streamName##_openFromString( struct IOChannel*, struct IOChannelReferenceValue** );\
static long       IOChannel##__streamName##_read( struct IOChannel*, void *, long );\
static long       IOChannel##__streamName##_write( struct IOChannel*, const void *, long );\
static long       IOChannel##__streamName##_flush( struct IOChannel* );\
static long long  IOChannel##__streamName##_seek( struct IOChannel*, long long, IOChannelWhence );\
static bool       IOChannel##__streamName##_close( struct IOChannel * );\
static void*      IOChannel##__streamName##_getProperty( struct IOChannel*, const char* );\
static bool       IOChannel##__streamName##_setProperty( struct IOChannel*, const char*, void* );\
static void       IOChannel##__streamName##_clear( struct IOChannel* );\
static void       IOChannel##__streamName##_delete( struct IOChannel* )

typedef struct IOChannelInterface
{
    char *streamName;
    IOChannelNew *indirectNew;
    IOChannelInit *indirectInit;
    IOChannelOpen *indirectOpen;
    IOChannelOpenFromString *indirectOpenFromString;
    IOChannelRead *indirectRead;
    IOChannelWrite *indirectWrite;
    IOChannelFlush *indirectFlush;
    IOChannelSeek *indirectSeek;
    IOChannelClose *indirectClose;
    IOChannelGetProperty *indirectGetProperty;
    IOChannelSetProperty *indirectSetProperty;
    IOChannelClear *indirectClear;
    IOChannelDelete *indirectDelete;
}
IOChannelInterface;


#define IOCHANNELINTERFACE_CREATE( __streamName )\
{\
  (char*)#__streamName,\
  IOChannel##__streamName##_new,\
  IOChannel##__streamName##_init,\
  IOChannel##__streamName##_open,\
  IOChannel##__streamName##_openFromString,\
  IOChannel##__streamName##_read,\
  IOChannel##__streamName##_write,\
  IOChannel##__streamName##_flush,\
  IOChannel##__streamName##_seek,\
  IOChannel##__streamName##_close,\
  IOChannel##__streamName##_getProperty,\
  IOChannel##__streamName##_setProperty,\
  IOChannel##__streamName##_clear,\
  IOChannel##__streamName##_delete\
}


typedef enum IOChannelError
{
    IOCHANNELERROR_NONE,    /* No error has occurred */
    IOCHANNELERROR_ACCV,    /* Trying to write in R_ONLY stream or vice versa */
    IOCHANNELERROR_INCR,    /* A wrong format specifier was used on IOChannel_printf() or IOChannel_scanf() */
    IOCHANNELERROR_BBUF,    /* An bad Internal buffer size was passed to IOChannel_init() ( size must be >0 ) */
    IOCHANNELERROR_BIST,    /* Bad infoString or or stream not defined */
    IOCHANNELERROR_BSEK,    /* Trying to use IOChannel_seek() on a stream where this operation is not allowed */
    IOCHANNELERROR_BSIZE,   /* Trying to open a generic memory stream passing as argument size <= 0 */
    IOCHANNELERROR_BMEMPTR, /* Not valid pointer (or in contast with flags) was passed to open a memory stream  */
    IOCHANNELERROR_BMMPSIZE,/* Bad size argument for Memory stream IOChannel_open() */
    IOCHANNELERROR_BWHESEK, /* Bad IOChannelWhence type */
    IOCHANNELERROR_BNDSEK,  /* IOCHANNELWHENCE_END was used on a memory stream ( memory not allows this flag)*/
    IOCHANNELERROR_BIOCALL, /* Calling IOChannel I/O functions with a non opened stream */
    IOCHANNELERROR_BSL,     /* Slashes are incorrect in IOChannel_open() infoString*/
    IOCHANNELERROR_BMODE,   /* Bad Modes were used on IOChannel_open() */
    IOCHANNELERROR_BWNC,    /* Bad IOChannelWhence flag was used */
    IOCHANNELERROR_BSHMNAME,/* Name of Posix Shm must start with "/" */
    IOCHANNELERROR_BSOCKR,  /* Low level socket read returned -1*/
    IOCHANNELERROR_BSOCKW,  /* Low level socket write returned -1*/
    IOCHANNELERROR_NOTDEF,  /* Error description not Defined */
    IOCHANNELERROR_BLLW,    /* Low level write wrote less bytes than requested ( system resources unavailables )*/
    IOCHANNELERROR_BSINAM,  /* Trying to open StdIn with mode different from R_ONLY ( which is the only allowed )*/
    IOCHANNELERROR_BSOUAM,  /* Trying to open StdOut with mode different from W_ONLY ( which is the only allowed )*/
    IOCHANNELERROR_BFLGS,   /* Bad Access flag were user to open the stream */
    IOCHANNELERROR_BOARG,   /* Bad arguments ( after permissions )were used to open the stream */
    IOCHANNELERROR_BMMFL,   /* Bad memory flags was used in IOChannel_open() */
    IOCHANNELERROR_UCONCL,  /* Unable to connect internal socket */
    IOCHANNELERROR_SOCKETTIMEOUT, /* Internal socket connection timed out */
    IOCHANNELERROR_BCLLBKW, /* IOChannel_printf() callback returned -1 */
    IOCHANNELERROR_BCLLBKR, /* IOChannel_scanf() callback returned -1 */
    IOCHANNELERROR_EEXIST,  /* PathName already exists and CREAT was not used */
    IOCHANNELERROR_EISDIR,  /* PathName refers to a directory */
    IOCHANNELERROR_EACCES,  /* You cannot access to the specified stream */
    IOCHANNELERROR_ENAMETOOLONG, /* Specified PathName is too long */
    IOCHANNELERROR_ENOENT,  /* The stream not exists and CREAT was not specified */
    IOCHANNELERROR_ENOTDIR, /* Bad directory name in the PathName  */
    IOCHANNELERROR_ENXIO,   /* Fd is a FIFO or a special file which cannot be managed with IOChannel */
    IOCHANNELERROR_ENODEV,  /* PathName refers to a special file */
    IOCHANNELERROR_EROFS,   /* Stream is trying to write in a read only filesystem */
    IOCHANNELERROR_ETXTBSY, /* PathName refers to an executable file */
    IOCHANNELERROR_EFAULT,  /* PathName points outside your accessible address space */
    IOCHANNELERROR_ELOOP,   /* Too many symbolic links were encountered resolving PathName */
    IOCHANNELERROR_ENOSPC,  /* PathName refers to a device*/
    IOCHANNELERROR_ENOMEM,  /* Not kernel memory available */
    IOCHANNELERROR_EMFILE,  /* Process has the maximum number of files open */
    IOCHANNELERROR_ENFILE,  /* System not allows to open any other file */
    IOCHANNELERROR_EINTR,   /* The call was interrupted by a signal */
    IOCHANNELERROR_EAGAIN,  /* I/O was not correct managed at low level */
    IOCHANNELERROR_EIO,     /* System I/O error */
    IOCHANNELERROR_EBADF,   /* Bad fd was used */
    IOCHANNELERROR_EINVAL,  /* Fd refers to an unsuitable object */
    IOCHANNELERROR_EFBIG,   /* Trying to write a file too big for the system */
    IOCHANNELERROR_EPIPE,   /* Fd is connected to a pipe whose reading end is closed */
    IOCHANNELERROR_ESPIPE,  /* Fd is a pipe or a system socket */
    IOCHANNELERROR_EOVERFLOW, /* Stream resulting size is too big */
    IOCHANNELERROR_TOOUNGET,/* Too unget were done: trying to unget more bytes than buffer size can allow */
    IOCHANNELERROR_ENOTSUP  /* The requested functionality is not currently supported */
}
IOChannelError;


typedef enum IOChannelType
{
    IOCHANNELTYPE_FD,
    IOCHANNELTYPE_SOCKET,
    IOCHANNELTYPE_MEMPTR,
    IOCHANNELTYPE_ANSIFILE,
    IOCHANNELTYPE_GENERICHANDLE,
    IOCHANNELTYPE_NOTSET
}
        IOChannelType;


#define IOCHANNEL_GET_ARGUMENT( __var, __type, __varArg )\
__var = va_arg( __varArg , __type )


#define IOCHANNEL_GET_ACCESS_MASK (IOCHANNEL_MODE_CREAT|\
                                  IOCHANNEL_MODE_TRUNC|\
                                  IOCHANNEL_MODE_APPEND|\
                                  IOCHANNEL_CLOSEFLAGS_MASK )


#define IOCHANNEL_CLOSEFLAGS_MASK  ( IOCHANNEL_MODE_CLOSE | IOCHANNEL_MODE_NOTCLOSE )


#define IOCHANNEL_ACCESSMODES  (IOCHANNEL_MODE_RW|\
                               IOCHANNEL_MODE_R_ONLY|\
                               IOCHANNEL_MODE_W_ONLY)


#define IOCHANNEL_GET_ACCESS_MODE( __mode )\
( __mode & ~( IOCHANNEL_GET_ACCESS_MASK ) )


#define IOCHANNEL_MODEIS_DEFINED( __mode )\
( __mode != IOCHANNEL_MODE_UNDEFINED )


#define IOCHANNEL_MODEIS_R_ONLY( __mode )\
( IOCHANNEL_GET_ACCESS_MODE( __mode ) == IOCHANNEL_MODE_R_ONLY )


#define IOCHANNEL_MODEIS_W_ONLY( __mode )\
( IOCHANNEL_GET_ACCESS_MODE( __mode ) == IOCHANNEL_MODE_W_ONLY )


#define IOCHANNEL_MODEIS_RW( __mode )\
( IOCHANNEL_GET_ACCESS_MODE( __mode ) == IOCHANNEL_MODE_RW )


#define IOCHANNEL_MODEIS_CREAT( __mode )\
( ( __mode & ~( IOCHANNEL_MODE_TRUNC  | IOCHANNEL_MODE_APPEND |\
                IOCHANNEL_ACCESSMODES | IOCHANNEL_CLOSEFLAGS_MASK ) ) ==\
                IOCHANNEL_MODE_CREAT )

#define IOCHANNEL_MODEIS_TRUNC( __mode )\
( ( __mode & ~( IOCHANNEL_MODE_CREAT  | IOCHANNEL_MODE_APPEND |\
                IOCHANNEL_ACCESSMODES | IOCHANNEL_CLOSEFLAGS_MASK ) ) ==\
                IOCHANNEL_MODE_TRUNC )


#define IOCHANNEL_MODEIS_APPEND( __mode )\
( ( __mode & ~( IOCHANNEL_MODE_TRUNC  | IOCHANNEL_MODE_CREAT |\
                IOCHANNEL_ACCESSMODES | IOCHANNEL_CLOSEFLAGS_MASK ) ) ==\
                IOCHANNEL_MODE_APPEND )


#define IOCHANNEL_MODEIS_CLOSE( __mode )\
( ( __mode & IOCHANNEL_CLOSEFLAGS_MASK ) == IOCHANNEL_MODE_CLOSE )


#define IOCHANNEL_MODEIS_NOTCLOSE( __mode )\
( ( __mode & IOCHANNEL_CLOSEFLAGS_MASK ) == IOCHANNEL_MODE_NOTCLOSE )


#define IOCHANNEL_MODEIS_UNDEFINED( __mode )\
( ( __mode | ~( IOCHANNEL_MODE_CREAT  | IOCHANNEL_MODE_APPEND |\
                IOCHANNEL_ACCESSMODES | IOCHANNEL_CLOSEFLAGS_MASK ) ) ==\
                IOCHANNEL_MODE_UNDEFINED )


#define IOCHANNEL_SET_EOF( __self )\
(__self)->foundEof = true


#define IOCHANNEL_SETSYSERRORFROMERRNO( __self )    \
  do {                                              \
    int __error = errno;                            \
    __self->errnoValue = __error;                   \
    IOChannel_setSysError( __self, __error );       \
  } while( 0 )

#if defined(__macos__)
#undef IOCHANNEL_SETSYSERRORFROMERRNO
#define IOCHANNEL_SETSYSERRORFROMERRNO( __self )
#elif defined( __windows__ )
#undef IOCHANNEL_SETSYSERRORFROMERRNO
#define IOCHANNEL_SETSYSERRORFROMERRNO( __self )    \
  do  {                                             \
    int __error = WSAGetLastError();                \
    __self->errnoValue = __error;                   \
    IOChannel_setSysError( __self, __error );       \
  } while( 0 )
#endif


#define IOCHANNELPROPERTY_START \
do {\


#define IOCHANNELPROPERTY_PARSE_BEGIN( __propertyName )\
if( Any_strcasecmp( propertyName, #__propertyName ) == 0 )\
{


#define IOCHANNELPROPERTY_PARSE_END( __propertyName )\
break;\
}


#define IOCHANNELPROPERTY_END \
} while( 0 );


typedef struct IOChannelBuffer
{
    void *defaultBuffer;
    long defaultSize;
    void *ptr;
    long size;
    long long index;
    bool freeOnExit;
}
        IOChannelBuffer;

typedef struct IOChannel
{
    unsigned long valid;
    IOChannelInterface *currInterface;
    void *streamPtr;
    bool isOpen;
    bool foundEof;
    bool usesWriteBuffering;
    IOChannelMode mode;
    IOChannelType type;
    long readTimeout;
    long writeTimeout;
    long rdDeployedBytes;
    long wrDeployedBytes;
    IOChannelError errorType;
    int errnoValue;
    IOChannelBuffer *ungetBuffer;
    IOChannelBuffer *writeBuffer;
    bool writeBufferIsExternal;
    bool autoResize;
    long long currentIndexPosition;
    long rdBytesFromLastWrite;
    long rdBytesFromLastUnget;
    MTList *userStream;
}
        IOChannel;


IOChannel *IOChannel_new( void );


bool IOChannel_init( IOChannel *self );


bool IOChannel_open( IOChannel *self,
                     const char *infoString,
                     IOChannelMode mode,
                     IOChannelPermissions permissions, ... );

/*!
 * \brief Opens the IOChannel for reading and/or writing
 *
 * See \ref IOChannel_About for main documentation.
 *
 * \return True on success, false otherwise.
 */
bool IOChannel_openFromString( IOChannel *self, const char *openString, ... );


int IOChannel_getModes( IOChannel *self );


/*!
 * \brief Log mode variable in readable format
 * \param debuglevel debuglevel to be used for logging
 * \param mode variable to log
 *
 * This function is mostly useful for debugging. It prints
 * in a readable format the value of the mode variable (IOChannelMode type).
 * When printing it lists all the set flags the mode variable contains.
 */
void IOChannel_logMode( long debuglevel, IOChannelMode mode );


/*!
 * \brief Log permissions variable in readable format
 * \param debuglevel debuglevel to be use for logging
 * \param permissions variable to log
 *
 * This function is mostly useful for debugging. It prints
 * in a readable format the value of the permissions variable (IOChannelPermissions type).
 * When printing it lists all the set flags the permissions variable contains.
 */
void IOChannel_logPermission( long debuglevel, IOChannelPermissions permissions );


bool IOChannel_isInterfaceDefined( IOChannel *self, char *streamName );


int IOChannel_getc( IOChannel *self );


long IOChannel_putc( IOChannel *self, char ch );


/*! \brief Get a line from the stream
 *
 * \param buffToStore buffer were read data is stored
 * \param strBuffSize maximum size of the buffer
 *
 * This function allows to read at most one less than size characters
 * from the current stream position and stores them into the buffer pointed
 * by buffToStore. Reading stops after a newline or end of stream is found.
 * The '\\n' is not written into the string.
 *
 * \return The number or read characters on success, -1 otherwise
 */
long IOChannel_gets( IOChannel *self, char *buffToStore, long strBuffSize );


/*! \brief Put a line into the stream
 *
 * \param buffToWrite buffer were read data is stored
 * \param strBuffSize maximum number of character to write
 *
 * This function allows to write at most size characters from
 * the buffer buffToWrite into the stream starting at the current position.
 * Writing stops when a '\\0' is found in the string.
 * It works like the standard 'fputs', so the '\\0' is not written.
 * Do not confuse with standard 'puts', which prints the '\\n' also.
 *
 * \return The number or written characters on success, -1 otherwise
 */
long IOChannel_puts( IOChannel *self, char *buffToWrite, long strBuffSize );


/*! \brief Scan formatted input
 *
 * \param nBytes Pointer to store read the number of bytes
 * \param format string format
 * \param ... Optional parameters to pass to the stream
 *
 * \attention Formatted printing works similar but not equal to standard
 *            library printf() functions!
 *
 * \attention All optional parameters are provided by reference!
 *
 * <h3>Example</h3>
 *
 * \code
 * numberOfItems = IOChannel_scanf( s, &numberOfChars, "someChars %f %s",
 *                                  &myFloat, string );
 * \endcode
 *
 * <h3>Supported format specifiers</h3>
 *
 * \li %%c   (char)
 * \li %%u   (unsigned int)
 * \li %%d   (int)
 * \li %%f   (float)
 * \li %%s   (string)
 * \li %%Lf  (long double)
 * \li %%hu  (unsigned short int)
 * \li %%hd  (short int)
 * \li %%lu  (unsigned long int)
 * \li %%ld  (long int)
 * \li %%lf  (double)
 * \li %%qc  (quoted char, e.g. 'c')
 * \li %%qs  (quoted string, e.g. "foo")
 * \li %%*s  (string, with max. length)
 * \li %%*qs (quoted string, with max. length)
 * \li %@    (scan input according to the user callBack function)
 * \li %%p   (pointer address)
 *
 * \return number of read items, -1 if scan failed
 */
long IOChannel_scanf( IOChannel *self, long *nBytes, char *format, ... );


/*! \brief Scan formatted input using va_list
 *
 * \param nBytes Pointer to store read the number of bytes
 * \param format string format
 * \param varArg variable argument list
 *
 * This function works exactly as IOChannel_scanf(), but the the variable
 * argument list is passed through the varArg va_list type.
 *
 * \return The number of read items, -1 if cannot do scan.
 */
long IOChannel_vscanf( IOChannel *self, long *nBytes, char *format, va_list varArg );


/*! \brief Print formatted output
 *
 * \param format string format
 * \param ... Optional paramenters to pass to the stream
 *
 * \attention Formatted printing works similar but not equal to standard
 *            library printf() functions!
 *
 * \attention All parameters to print need to be provided by reference!
 *
 * \attention Advanced format specifiers such as "%10d" are not supported.
 *
 * <h3>Supported format specifiers:</h3>
 *
 * \li %%c   (char)
 * \li %%u   (unsigned int)
 * \li %%d   (int)
 * \li %%f   (float)
 * \li %%s   (string)
 * \li %%S   (string, where unprintable characters are escaped)
 * \li %%Lf  (long double)
 * \li %%hu  (unsigned short int)
 * \li %%hd  (short int)
 * \li %%lu  (unsigned long int)
 * \li %%ld  (long int)
 * \li %%lf  (double)
 * \li %%qc  (quoted char, e.g. 'c', hex if not printable)
 * \li %%qs  (quoted string, e.g. "foo", hex if not printable)
 * \li %%*s  (string, with max. length)
 * \li %%*qs (quoted string, with max. length)
 * \li %@    (scan input according to the user callBack function)
 *
 * Floating point numbers are normalized:
 *
 * \li [-]d.ddddddde+dd   (4 bytes float):  1 decimal digit before period,
 *     7 decimal digits after the period, and two digits for the exponent
 * \li [-]d.dddddddddddddddde+dd  ( 8 bytes double ): 1 decimal digit before
 *     period,  16 decimal digits after the period, and two digits for the
 *     exponent
 * \li [-]d.dddddddddddddddddde+dd: architecture dependent
 *
 * <h3>Example</h3>
 * \code
 * IOChannel_printf( s, "A char: %c, An int: %d, A float: %f ",
 *                      &myChar, &myInt, &myFloat );
 * \endcode
 *
 * \return number of printed characters, -1 if printing failed
 */
long IOChannel_printf( IOChannel *self, const char *format, ... );


/*! \brief Print formatted output using va_list
 *
 * \param format string format
 * \param varArg variable argument list
 *
 * This function works exactly as IOChannel_printf(), but the the variable
 * argument list is passed through the varArg va_list type.
 *
 * \return The number of printed characters, -1 if cannot do print.
 */
long IOChannel_vprintf( IOChannel *self, char *format, va_list varArg );


/*! \brief Read data from a stream
 *
 * \param buffer Pointer to the buffer in which store data
 * \param size Number of bytes to read
 *
 * This function reads a block of bytes of "size" length from the stream, and
 * stores the block in the given buffer. Buffer must be a valid pointer, and size
 * greater than zero. If these two requirements are not satisfied the return
 * value will be -1, and an internal error is set.
 *
 * \note Return value (= number of read bytes) could be less than size
 *
 * If an error occurred while reading, the buffer will contain the bytes
 * that have already been read, and an internal error is set. To check that
 * no error occurred during reading process, use IOChannel_isErrorOccurred():
 *
 * \code
 * long numReadBytes;
 * ...
 * numReadBytes = IOChannel_read( stream, myBuffer, size );
 *
 * //numReadBytes may be less than size, but this not mean that an error
 * //occurred
 *
 * if( IOChannel_isErrorOccurred( stream ) == true )
 * {
 *   ANY_LOG( 5, "Error reading", ANY_LOG_WARNING );
 * }
 * \endcode
 *
 * \return The number of read bytes, -1 if no bytes were read
 *        (because an error occurred).
 *
 */
long IOChannel_read( IOChannel *self, void *buffer, long size );


/*! \brief Read  block of data from a stream
 *
 * \param buffer Pointer to the buffer cointainig data to write
 * \param size Number of bytes to read
 *
 * This function is similar to IOChannel_read() but instead of returning immediately
 * when the low level read() return less number of read data, it will return
 * only when all the requested data has been read from the given stream
 *
 * \return The number of requested byte to read or less in case of error
 */
long IOChannel_readBlock( IOChannel *self, void *buffer, long size );


/*! \brief Write on a stream
 *
 * \param buffer Pointer to the buffer cointainig data to write
 * \param size Number of bytes to write
 *
 * This function writes a block of bytes of "size" length from the buffer to
 * the stream. Buffer must be valid pointer, and size greater than zero.
 * If these two requirements are not respected, return value is -1, and an
 * internal error is set. Write return value could be less than size value.
 * There are two cases:
 *
 * The end of stream has been found( no more space available ); you can test
 * it using IOChannel_eof. <br>
 *
 * An error occurred meanwhile IOChannel was writing. The number of written bytes
 * until the moment in which the error occurred is returned.
 * In this case, an internal error is set.<br>
 *
 * \code
 * long numWrittenBytes;
 * ...
 * numWrittenBytes = IOChannel_write( stream, myBuffer, size );
 *
 * //numWrittenBytes may be less than size, but this does not mean that
 * //an error occurred
 *
 * if( IOChannel_isErrorOccurred( stream ) == true )
 * {
 *   ANY_LOG( 5, "Error writing", ANY_LOG_WARNING );
 * }
 * \endcode
 *
 * \return The number of written bytes, -1 if no bytes were
 *        written( because an error occurred ).
 */
long IOChannel_write( IOChannel *self, const void *buffer, long size );


/*! \brief Write block of data on a stream
 *
 * \param buffer Pointer to the buffer cointainig data to write
 * \param size Number of bytes to write
 *
 * This function is similar to IOChannel_write() but instead to return immediately
 * when the low level write() return less number of written data, it will return
 * only when all the requested data has been written to the give stream.
 *
 * \return The number of requested byte to write or less in case of error
 */
long IOChannel_writeBlock( IOChannel *self, const void *buffer, long size );


long IOChannel_unget( IOChannel *self, void *buffer, long size );


long IOChannel_flush( IOChannel *self );


void IOChannel_setIsReadDataAvailableTimeout( IOChannel *self, long usecs );


long IOChannel_getIsReadDataAvailableTimeout( IOChannel *self );


bool IOChannel_isReadDataAvailable( IOChannel *self );


void IOChannel_setIsWritePossibleTimeout( IOChannel *self, long usecs );


long IOChannel_getIsWritePossibleTimeout( IOChannel *self );


bool IOChannel_isWritePossible( IOChannel *self );


/*! \brief Get the number of written bytes.
 *
 * \return The number of written bytes since the stream was opened,
 *         -1 upon error
 */
long IOChannel_getWrittenBytes( IOChannel *self );


long IOChannel_getReadBytes( IOChannel *self );


/*! \brief Seek the current position in the stream
 *
 * \param offset The offset
 * \param whence mode in which offset will be used

 * This function allows to reposition the index used by IOChannel to indicate
 * the current position in the stream. Repositioning is done according to the
 * offset parameter and whence directive. Use them in this way:
 *
 * IOCHANNELWHENCE_SET
 *             The offset is set to offset bytes. <br>
 * IOCHANNELWHENCE_CUR
 *             The offset is set to its current
 *              location plus offset bytes.        <br>
 * IOCHANNELWHENCE_END
 *             The offset is set to the size of
 *             the file plus offset bytes.        <br>
 *
 * \return The current position in bytes from the begin of the stream, or
 *        -1 if something went wrong: in this case
 *        an IOChannelError is set.
 */
long IOChannel_seek( IOChannel *self, long offset, IOChannelWhence whence );


/*! \brief Get current position into the stream */
long IOChannel_tell( IOChannel *self );


void IOChannel_rewind( IOChannel *self );


/*! \brief Resets internal indexes
 *
 * This function allows to reset internal indexes
 *
 * -number of written bytes
 * -number of read bytes
 * -current stream index
 *
 * User generally has not to use it.
 */
void IOChannel_resetIndexes( IOChannel *self );


/*! \brief Check if the end of stream was found
 *
 * \note UDP, ServerTCP and ServerUDP do not support a reliable detection
 *       of EOF.
 *
 * \return True if end of stream was found, false otherwise.
 */
bool IOChannel_eof( IOChannel *self );


bool IOChannel_hasFd( IOChannel *self );


bool IOChannel_hasBerkeleySocket( IOChannel *self );


bool IOChannel_hasPointer( IOChannel *self );


bool IOChannel_hasAnsiFILE( IOChannel *self );


bool IOChannel_isOpen( IOChannel *self );


bool IOChannel_isErrorOccurred( IOChannel *self );


IOChannelError IOChannel_getErrorNumber( IOChannel *self );


char *IOChannel_getErrorDescription( IOChannel *self );


int IOChannel_getErrnoValue( IOChannel *self );


void IOChannel_setError( IOChannel *self, IOChannelError error );


void IOChannel_setSysError( IOChannel *self, int error );


void IOChannel_cleanError( IOChannel *self );


char *IOChannel_getStreamType( IOChannel *self );


bool IOChannel_close( IOChannel *self );


void *IOChannel_getStreamPtr( IOChannel *self );


/*! \brief Get Stream properties
 *
 * \param propertyName The character string of the property to get
 *
 * This function can be used to get stream properties.
 * Actually for the provided streams you can use these properties:
 *
 * \code
 * IOChannel_getProperty( self, "Fd" );
 * \endcode
 * to get the pointer to the fd used by the IOChannel instance.
 *
 * \code
 * IOChannel_getProperty( self, "MemPointer" );
 * \endcode
 * to get the pointer to your memory stream
 *
 * \code
 * IOChannel_getProperty( self, "Socket" );
 * \endcode
 * to get the pointer to the socket used
 * internally.
 *
 * \return The pointer to the property, or NULL if
 *        property doesn't exist
 */
void *IOChannel_getProperty( IOChannel *self, const char *propertyName );


bool IOChannel_setProperty( IOChannel *self, const char *propertyName, void *propertyValue );


void IOChannel_setType( IOChannel *self, IOChannelType type );


bool IOChannel_addInterface( IOChannel *self, IOChannelInterface *currInterface );


void IOChannel_setUngetBuffer( IOChannel *self, void *buffer, long size );


/*! \brief Set an internal buffer for buffered mode
 *
 * \param buffer Pointer to the memory buffer
 * \param size Size of the buffer
 *
 * This functions allows the user to set IOChannel to use a buffer
 * to optimize the number of low level calls. This improves
 * performance, especially when writing formatted data.
 * When a buffer is set, the IOChannel instance do not switch automatically
 * into buffered mode. For this it is necessary to explicit inform it using
 * IOChannel_setUseWriteBuffering()
 *
 * Note that if the <em>buffer</em> is NULL, an internal buffer
 * of <em>size</em> length is allocated. This buffer is automatically freed
 * when IOChannel_close() is called.
 * If <em>buffer</em> is not NULL, it will not be freed
 * when IOChannel_close() is called.
 *
 * Remember that each time a stream is opened, it is set to work with the
 * default internal write buffer.
 *
 * Important: for memory streams, no buffering is used!
 * So a call to this functions is ignored ( no buffer is set nor any allocation is done )
 */
void IOChannel_setWriteBuffer( IOChannel *self, void *buffer, long size );


/*! \brief Enable/disable buffered mode
 *
 * \attention This function must be called after \c IOChannel_open() resp.
 *            \c IOChannel_openFromString().
 *
 * This functions set/unset IOChannel to use buffered mode for
 * write data. If a buffer was previously set by the user
 * by the IOChannel_setWriteBuffer(), the IOChannel will use this buffer,
 * otherwise it is using an internal buffer. On memory streams this
 * function has not effect.
 * Note that if you are switching from buffered mode to unbuffered, a call to
 * this function will flush automatically the internal buffer if any data
 * was still waiting to be written.
 *
 * The last parameter, if true, allows to resize the buffer automatically if it is full:
 * in this way the buffer can increase its size and all the data is buffered until
 * the user explicitly calls IOChannel_flush() or the stream is closed.
 * The flag works only if default buffer is set or if you have set the new buffer
 * calling IOChannel_setWriteBuffer() with NULL as buffer argument ( auto allocation ).
 * If you set a custom buffer this flag will not have effect.
 * The flag works also with "Mem://" stream, but only if NULL was passed for buffer
 * ( auto allocation ).
 *
 * \return True on success, false if autoResize is not allowed.
 */
bool IOChannel_setUseWriteBuffering( IOChannel *self, bool useBuffering, bool autoResize );


bool IOChannel_usesWriteBuffering( IOChannel *self );


/*! \brief Add data on the internal write buffer */
long IOChannel_addToWriteBuffer( IOChannel *self, const void *buffer, long size );


long IOChannel_getWriteBufferedBytes( IOChannel *self );


void *IOChannel_getInternalWriteBufferPtr( IOChannel *self );


long long IOChannel_getStreamPosition( IOChannel *self );


void IOChannel_clear( IOChannel *self );


void IOChannel_delete( IOChannel *self );


void IOChannel_valid( IOChannel *self );


#if defined(__cplusplus)
}
#endif
#endif


/* EOF */
