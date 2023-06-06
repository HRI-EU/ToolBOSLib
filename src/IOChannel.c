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


#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

#if !defined(__windows__)

#include <unistd.h>
#include <sys/select.h>

#else

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#include <time.h>
#include <io.h>
#include <direct.h>
#endif

#include <IOChannel.h>
#include <DynamicLoader.h>
#include <BerkeleySocketClient.h>
#include <IOChannelReferenceValue.h>
#include <ToolBOSLib.h>


#define IOCHANNEL_VALID                       ( 0x1cb5d117 )
#define IOCHANNEL_INVALID                     ( 0xff6bc8a7 )


#define IOCHANNEL_WRITEBUFFER_DEFAULT         ( 1024 )
#define IOCHANNEL_UNGETBUFFER_DEFAULT         ( 1024 )

#define IOCHANNEL_SELECT_TIMEOUT_USEC         ( 1000 )

#define IOCHANNELINTERFACE_NEW( __self )\
(*__self->currInterface->indirectNew)()

#define IOCHANNELINTERFACE_INIT( __self )\
(*__self->currInterface->indirectInit)( __self )

#define IOCHANNELINTERFACE_OPEN( __self, __infoString, __mode, __permissions, __varArg )\
(*__self->currInterface->indirectOpen)( __self, __infoString, __mode, __permissions, __varArg )

#define IOCHANNELINTERFACE_OPENFROMSTRING( __self, __referenceValueVector )\
(*__self->currInterface->indirectOpenFromString)( __self, __referenceValueVector )

#define IOCHANNELINTERFACE_READ( __self, __buffer, __size )\
(*__self->currInterface->indirectRead)( __self, __buffer, __size )

#define IOCHANNELINTERFACE_WRITE( __self, __buffer, __size )\
(*__self->currInterface->indirectWrite)( __self, __buffer, __size )

#define IOCHANNELINTERFACE_FLUSH( __self )\
(*__self->currInterface->indirectFlush)( __self )

#define IOCHANNELINTERFACE_SEEK( __self, __offset, __whence )\
(*__self->currInterface->indirectSeek)( __self, __offset, __whence )

#define IOCHANNELINTERFACE_CLOSE( __self )\
(*self->currInterface->indirectClose)( __self )

#define IOCHANNELINTERFACE_GETPROPERTY( __self, __propertyName )\
(*self->currInterface->indirectGetProperty)( __self, __propertyName )

#define IOCHANNELINTERFACE_SETPROPERTY( __self, __propertyName, __value )\
(*self->currInterface->indirectSetProperty)( __self, __propertyName, __value )

#define IOCHANNELINTERFACE_CLEAR( __self )\
(*self->currInterface->indirectClear)( __self )

#define IOCHANNELINTERFACE_DELETE( __self )\
(*self->currInterface->indirectDelete)( __self )

#define IOCHANNEL_MODEIS_VALID( __mode )                                \
    ( (__mode & IOCHANNEL_CLOSEFLAGS_MASK ) != IOCHANNEL_CLOSEFLAGS_MASK )


#define IOCHANNEL_ISPRINT( __ch )\
  ( ((__ch  >= 32 ) && (__ch <= 126)) )


#define IOCHANNEL_ISDIGIT( __ch )\
  ( ((__ch >= '0' ) && (__ch <= '9')) )


#define IOCHANNEL_ISSPACE( __ch )\
  ( ((__ch == ' ' )||(__ch == '\t' )||\
    (__ch == '\r' )||(__ch == '\n' )||\
    (__ch == '\v' )) )


#define IOCHANNEL_ISFLOATALLOWED( __isFloat, __ch )\
  ( ((__isFloat ) &&\
    ((__ch == '+') || (__ch == '-') ||\
     (__ch == 'e') || (__ch == '.'))) )


#define IOCHANNEL_REQUIRE_INTERFACE( __self, __indirectFun )\
  ANY_REQUIRE_MSG( __self->currInterface,\
    "Not valid interface loaded" );\
  ANY_REQUIRE_MSG( __self->currInterface->__indirectFun,\
    "Interface indirect function not loaded!" )


#define IOCHANNEL_READSPACES( __self, __buffer )\
  do{\
      *__buffer = ' ';                                              \
      while( ( IOCHANNEL_ISSPACE( *__buffer ) ) &&                  \
             ( IOChannel_eof( __self ) == false ) &&                \
             ( IOChannel_isErrorSet( __self ) == false ) )          \
      {                                                             \
          if( IOChannel_readInternal( __self, __buffer, 1 ) != 1 )  \
          {                                                         \
              break;                                                \
          }                                                         \
      }                                                             \
  } while( 0 )



#define IOCHANNEL_SCAN_ITEM( __self, __spec, __buffer,                  \
                             __type, __varArg, __isFloat,               \
                             __format )                                 \
    do{                                                                 \
        char __tmpBuffer[40];                                           \
        __type *__param = va_arg( __varArg, __type * );                 \
        IOChannel_scanItemInternal( __self, __buffer, __isFloat,        \
                                    &__format, __tmpBuffer );           \
        Any_sscanf( __tmpBuffer, __spec, __param );                     \
    } while( 0 )


#define IOCHANNEL_PRINT_ITEM( __self, __spec, __type, __varArg )\
  do{\
    char __tmpBuffer[40];\
    long __bufferLen = 0;\
    __type *__buffer = va_arg( __varArg, __type* );\
  \
    __bufferLen = Any_snprintf( __tmpBuffer, 40, __spec, *__buffer );\
    ANY_REQUIRE( __bufferLen > 0 );\
    IOChannel_writeInternal( __self, __tmpBuffer, __bufferLen );\
  } while( 0 )


/*---------------------------------------------------------------------------*/
/* Global Data                                                               */
/*---------------------------------------------------------------------------*/


struct IOChannelSysError;
struct IOChannelErrorType;
struct IOChannelPlugin;


/*
 * Define the IOChannelPlugin dynamic loader interface
 */
typedef struct IOChannelPlugin
{
    DynamicLoader *libHandle;
    /*< Handle of the dynamic library */
    IOChannelInterface *currInterface;   /*< Associated plugin interface */
}
        IOChannelPlugin;

/*
 * Define the mapping between the system error and IOChannel error
 * representation
 */
typedef struct IOChannelSysError
{
    int sysError;
    /*< Operating System error number */
    IOChannelError errorNumber;        /*< Paired IOChannel error */
}
        IOChannelSysError;

/*
 * Declare the mapping O.S -> IOChannel relation
 */
#define IOCHANNELSYSERROR_MAP( __sysError, __errorNumber ) \
{ __sysError, __errorNumber }

/*
 * The static table for mapping system errors and IOChannel errors
 */
static IOChannelSysError IOChannelSysErrorTable[] =
        {
                IOCHANNELSYSERROR_MAP( ENAMETOOLONG, IOCHANNELERROR_ENAMETOOLONG ),
                IOCHANNELSYSERROR_MAP( EEXIST, IOCHANNELERROR_EEXIST ),
                IOCHANNELSYSERROR_MAP( EISDIR, IOCHANNELERROR_EISDIR ),
                IOCHANNELSYSERROR_MAP( EACCES, IOCHANNELERROR_EACCES ),
                IOCHANNELSYSERROR_MAP( ENOENT, IOCHANNELERROR_ENOENT ),
                IOCHANNELSYSERROR_MAP( ENOTDIR, IOCHANNELERROR_ENOTDIR ),
                IOCHANNELSYSERROR_MAP( ENXIO, IOCHANNELERROR_ENXIO ),
                IOCHANNELSYSERROR_MAP( ENODEV, IOCHANNELERROR_ENODEV ),
                IOCHANNELSYSERROR_MAP( EROFS, IOCHANNELERROR_EROFS ),
#if !defined(__windows__)
                IOCHANNELSYSERROR_MAP( ETXTBSY, IOCHANNELERROR_ETXTBSY ),
#endif
                IOCHANNELSYSERROR_MAP( EFAULT, IOCHANNELERROR_EFAULT ),
#if !defined(__windows__)
                IOCHANNELSYSERROR_MAP( ELOOP, IOCHANNELERROR_ELOOP ),
#endif
                IOCHANNELSYSERROR_MAP( ENOSPC, IOCHANNELERROR_ENOSPC ),
                IOCHANNELSYSERROR_MAP( ENOMEM, IOCHANNELERROR_ENOMEM ),
                IOCHANNELSYSERROR_MAP( EMFILE, IOCHANNELERROR_EMFILE ),
                IOCHANNELSYSERROR_MAP( ENFILE, IOCHANNELERROR_ENFILE ),
                IOCHANNELSYSERROR_MAP( EINTR, IOCHANNELERROR_EINTR ),
                IOCHANNELSYSERROR_MAP( EAGAIN, IOCHANNELERROR_EAGAIN ),
                IOCHANNELSYSERROR_MAP( EIO, IOCHANNELERROR_EIO ),
                IOCHANNELSYSERROR_MAP( EBADF, IOCHANNELERROR_EBADF ),
                IOCHANNELSYSERROR_MAP( EINVAL, IOCHANNELERROR_EINVAL ),
                IOCHANNELSYSERROR_MAP( EFAULT, IOCHANNELERROR_EFAULT ),
                IOCHANNELSYSERROR_MAP( EFBIG, IOCHANNELERROR_EFBIG ),
                IOCHANNELSYSERROR_MAP( EPIPE, IOCHANNELERROR_EPIPE ),
                IOCHANNELSYSERROR_MAP( ENOSPC, IOCHANNELERROR_ENOSPC ),
                IOCHANNELSYSERROR_MAP( ESPIPE, IOCHANNELERROR_ESPIPE )
#if !defined(__windows__)
                , IOCHANNELSYSERROR_MAP( EOVERFLOW, IOCHANNELERROR_EOVERFLOW )
#endif
        };

/*
 * Maps the IOChannel error into a description
 */
typedef struct IOChannelErrorType
{
    IOChannelError errorNumber;
    char *errorDescription;
}
        IOChannelErrorType;

static IOChannelErrorType IOChannelErrorTypeTable[] =
        {
                { IOCHANNELERROR_NONE,          "!No error occurred!" },
                { IOCHANNELERROR_ACCV,          "Trying to read in write only mode or vice versa." },
                { IOCHANNELERROR_INCR,          "Check printf/scanf infoString string after format specifier." },
                { IOCHANNELERROR_BBUF,          "Intenal Buffer must be greather than zero." },
                { IOCHANNELERROR_BIST,          "Bad infostring: no stream was recognized." },
                { IOCHANNELERROR_BSEK,          "Trying to seek  stdin or stdout stream " },
                { IOCHANNELERROR_BSIZE,         "Function was called with size <= 0" },
                { IOCHANNELERROR_BMEMPTR,       "Mem open() was called with an invalid pointer: maybe it was not allocated or it is NULL" },
                { IOCHANNELERROR_BMMPSIZE,      "Bad size passed to MemMapFd open function. You passed a <= 0 value" },
                { IOCHANNELERROR_BWHESEK,       "IOChannelMemMapFd_seek: Unrecognized IOCHANNELWHENCE TYPE!" },
                { IOCHANNELERROR_BNDSEK,        "IOChannelMemMapFd_seek: IOCHANNELWHENCE_END not allowed on memory stream" },
                { IOCHANNELERROR_BIOCALL,       "Don't call IOChannel I/O functions if stream is not open" },
                { IOCHANNELERROR_BSL,           "Bad infoString on Open function. Check slash separator -StreamType://-" },
                { IOCHANNELERROR_BMODE,         "Bad mode values where used on open function.  They are not R_ONLY, nor W_ONLY nor RW" },
                { IOCHANNELERROR_BWNC,          "Bad WHENCE flag was used on IOChannel_seek" },
                { IOCHANNELERROR_BSHMNAME,      "The name of named shm must start with \" / \". A valid open infoString is e.g. Shm:///myName" },
                { IOCHANNELERROR_BSOCKR,        "Socket read returned -1 " },
                { IOCHANNELERROR_BSOCKW,        "Socket write returned -1 " },
                { IOCHANNELERROR_NOTDEF,        "Error not defined!" },
                { IOCHANNELERROR_BLLW,          "Low level write() wrote less bytes than requested" },
                { IOCHANNELERROR_BSINAM,        "For StdIn open, you must use IOCHANNEL_MODE_R_ONLY" },
                { IOCHANNELERROR_BSOUAM,        "For StdOut open, you must use IOCHANNEL_MODE_W_ONLY" },
                { IOCHANNELERROR_BFLGS,         "Specified flags are NOT VALID to open this stream" },
                { IOCHANNELERROR_BOARG,         "Not valid optional argument(s) for open the stream. Check argument(s) (and if they're present ) in your IOChannel_open " },
                { IOCHANNELERROR_BMMFL,         "Bad flags for memory stream" },
                { IOCHANNELERROR_UCONCL,        "Unable to connect the internal socket" },
                { IOCHANNELERROR_SOCKETTIMEOUT, "Internal socket connection timed out" },
                { IOCHANNELERROR_BCLLBKW,       "IOChannel_printf() callback returned -1" },
                { IOCHANNELERROR_BCLLBKR,       "IOChannel_scanf() callback returned -1" },
                { IOCHANNELERROR_EEXIST,        "Pathname already exists and CREAT and EXCL were used." },
                { IOCHANNELERROR_EISDIR,        "Pathname refers to a directory and the access requested involved writing (that is, O_WRONLY or O_RDWR is set)." },
                { IOCHANNELERROR_EACCES,        "Permission to access the file, or a directory component in the path, denied." },
                { IOCHANNELERROR_ENAMETOOLONG,  "Name too long." },
                { IOCHANNELERROR_ENOENT,        "No such file entry, and the CREAT flag was not specified." },
                { IOCHANNELERROR_ENOTDIR,       "No such directory entry, or the O_DIRECTORY flag was specified but the entry was not a directory." },
                { IOCHANNELERROR_ENXIO,         "The device or FIFO represented by the specified file was not found" },
                { IOCHANNELERROR_ENODEV,        "No such device." },
                { IOCHANNELERROR_EROFS,         "Read only filesystem, access denied." },
                { IOCHANNELERROR_ETXTBSY,       "Text file already open." },
                { IOCHANNELERROR_EFAULT,        "Invalid pointer detected, bad address." },
                { IOCHANNELERROR_ELOOP,         "Too many symbolic link levels were encountered." },
                { IOCHANNELERROR_ENOSPC,        "Pathname was to be created but the device containing pathname has no room for the new file." },
                { IOCHANNELERROR_ENOMEM,        "Insufficient kernel memory was available." },
                { IOCHANNELERROR_EMFILE,        "The process already has the maximum number of files open." },
                { IOCHANNELERROR_ENFILE,        "The limit on the total number of files open on the system has been reached." },
                { IOCHANNELERROR_EINTR,         "The call was interrupted by a signal before any data was read/write." },
                { IOCHANNELERROR_EAGAIN,        "Non-blocking I/O has been selected using O_NONBLOCK and the write would block( Reading: data was not immediately available )" },
                { IOCHANNELERROR_EIO,           "A low-level I/O error occurred while modifying the inode." },
                { IOCHANNELERROR_EBADF,         "Fd is not a valid file descriptor or( maybe you passed a negative fd ) the generic stream is not open for reading VS writing.( e.g. write on StdIn! )" },
                { IOCHANNELERROR_EINVAL,        "Fd is attached to an object which is unsuitable for reading. VS writing( After a seek it means that whence is not one of SEEK_SET, SEEK_CUR, SEEK_END, or the resulting file offset would be negative.)" },
                { IOCHANNELERROR_EFAULT,        "Buf is outside your accessible address space." },
                { IOCHANNELERROR_EFBIG,         "An attempt was made to write a file that exceeds the implementation-defined maximum file size or the process' file size limit or to  write at a position past than the maximum allowed offset." },
                { IOCHANNELERROR_EPIPE,         "Fd is connected to a pipe or socket whose reading end is closed. When this happens the writing process will also receive a SIGPIPE signal. (Thus, the write return value is seen only if the program catches blocks or ignores this signal.)" },
                { IOCHANNELERROR_ENOSPC,        "The device containing the file referred to by fd has no room for the data." },
                { IOCHANNELERROR_ESPIPE,        "Fildes is associated with a pipe, socket or FIFO." },
                { IOCHANNELERROR_EOVERFLOW,     "The resulting file offset cannot be represented in an off_t" },
                { IOCHANNELERROR_TOOUNGET,      "Trying to unget more bytes than buffer size can allow" },
                { IOCHANNELERROR_ENOTSUP,       "The requested functionality is not currently supported" }
        };

static int IOChannelErrorTypeTable_len = sizeof( IOChannelErrorTypeTable ) /
                                         sizeof( IOChannelErrorType );

static int IOChannelSysErrorTable_len = sizeof( IOChannelSysErrorTable ) /
                                        sizeof( IOChannelSysError );


/*---------------------------------------------------------------------------*/
/*  Private Prototypes                                                       */
/*---------------------------------------------------------------------------*/

static IOChannelInterface *IOChannel_findInterface( IOChannel *self, const char *infoString, char **subInfoString );

static long IOChannel_printFormatting( IOChannel *self, const char *format, va_list varArg );

static long IOChannel_scanFormatting( IOChannel *self, long *nBytes, char *format, va_list varArg );

static void IOChannel_resetValuesForNewOpen( IOChannel *self );

static void IOChannel_resetObject( IOChannel *self );

static bool IOChannel_isErrorSet( IOChannel *self );

static bool IOChannel_isCallAllowedCheck( IOChannel *self );

static bool IOChannel_isOpenCheck( IOChannel *self );

static bool IOChannel_isNotWrOnlyCheck( IOChannel *self );

static bool IOChannel_isNotRdOnlyCheck( IOChannel *self );

static long IOChannel_popFromUngetBuffer( IOChannel *self, void *buff, long size );

static long IOChannel_pushIntoUngetBuffer( IOChannel *self, void *buff, long size );

static IOChannelBuffer *IOChannelBuffer_new( void );

static bool IOChannelBuffer_init( IOChannelBuffer *self, long defaultSize );

static void IOChannelBuffer_atOpen( IOChannelBuffer *self );

static void IOChannelBuffer_set( IOChannelBuffer *self, void *ptr, long size );

static void IOChannelBuffer_atClose( IOChannelBuffer *self );

static void IOChannelBuffer_clear( IOChannelBuffer *self );

static void IOChannelBuffer_delete( IOChannelBuffer *self );

static IOChannelPlugin *IOChannelPlugin_new( void );

static int IOChannelPlugin_init( IOChannelPlugin *self, char *libraryName );

static IOChannelInterface *IOChannelPlugin_getInterface( IOChannelPlugin *self );

static IOChannelInterface *IOChannel_loadInterface( IOChannel *self, char *typeStream );

static void IOChannelPlugin_clear( IOChannelPlugin *self );

static void IOChannelPlugin_delete( IOChannelPlugin *self );

static bool IOChannel_internalReadSelect( IOChannel *self, int fd, struct timeval *timeout );

static bool IOChannel_internalWriteSelect( IOChannel *self, int fd, struct timeval *timeout );

static int IOChannel_writeEscapedChar( IOChannel *self, char ch );

static IOChannelInterface *IOChannel_findStaticStream( const char *streamName );

static void IOChannel_scanItemInternal( IOChannel *self, char *buffer, bool isFloat, char **format, char *tmpBuffer );

static long IOChannel_readInternal( IOChannel *self, void *buffer, long size );
static long IOChannel_writeInternal( IOChannel *self, const void *buffer, long size );

#if defined(__windows__)
static int IOChannel_isSocket( int fd );
#endif

/*---------------------------------------------------------------------------*/
/* Available stream formats                                                  */
/*---------------------------------------------------------------------------*/

/*
 * we want to use already existing Ops
 */
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( AnsiFILE );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Calc );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Fd );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( File );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Mem );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( MemMapFd );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Null );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( PipeCmd );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Rand );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( RTBOS );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( ServerTcp );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( ServerUdp );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Shm );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Socket );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( StdErr );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( StdIn );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( StdOut );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Tcp );
extern IOCHANNELINTERFACE_DECLARE_OPTIONS( Udp );

/*
 * Declare a static vector of pointer to Ops
 */
static IOChannelInterface *IOChannel_internalStreams[] =
        {
                &IOCHANNELINTERFACE_OPTIONS( AnsiFILE ),
                &IOCHANNELINTERFACE_OPTIONS( Calc ),
                &IOCHANNELINTERFACE_OPTIONS( Fd ),
                &IOCHANNELINTERFACE_OPTIONS( File ),
                &IOCHANNELINTERFACE_OPTIONS( Mem ),
                &IOCHANNELINTERFACE_OPTIONS( MemMapFd ),
                &IOCHANNELINTERFACE_OPTIONS( Null ),
                &IOCHANNELINTERFACE_OPTIONS( PipeCmd ),
                &IOCHANNELINTERFACE_OPTIONS( Rand ),
                &IOCHANNELINTERFACE_OPTIONS( RTBOS ),
                &IOCHANNELINTERFACE_OPTIONS( ServerTcp ),
                &IOCHANNELINTERFACE_OPTIONS( ServerUdp ),
                &IOCHANNELINTERFACE_OPTIONS( Shm ),
                &IOCHANNELINTERFACE_OPTIONS( Socket ),
                &IOCHANNELINTERFACE_OPTIONS( StdErr ),
                &IOCHANNELINTERFACE_OPTIONS( StdIn ),
                &IOCHANNELINTERFACE_OPTIONS( StdOut ),
                &IOCHANNELINTERFACE_OPTIONS( Tcp ),
                &IOCHANNELINTERFACE_OPTIONS( Udp ),

                /* termination */
                NULL
        };

/*---------------------------------------------------------------------------*/
/* Public Functions                                                          */
/*---------------------------------------------------------------------------*/
IOChannel *IOChannel_new( void )
{
    IOChannel *self = ANY_TALLOC( IOChannel );
    return self;
}


bool IOChannel_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    self->valid = IOCHANNEL_INVALID;

    /* Will contain user streams ... */
    self->userStream = MTList_new();
    ANY_REQUIRE_MSG( self->userStream, "Unable to allocate internal user stream list!" );
    MTList_init( self->userStream );

    /* IOChannel_clear will free the list manually... */
    MTList_setDeleteMode( self->userStream, MTLIST_DELETEMODE_MANUAL );

    self->ungetBuffer = IOChannelBuffer_new();
    IOChannelBuffer_init( self->ungetBuffer, IOCHANNEL_UNGETBUFFER_DEFAULT);

    self->writeBuffer = IOChannelBuffer_new();
    IOChannelBuffer_init( self->writeBuffer, IOCHANNEL_WRITEBUFFER_DEFAULT);

    self->valid = IOCHANNEL_VALID;

    IOChannel_resetValuesForNewOpen( self );

    return true;
}


bool IOChannel_open( IOChannel *self,
                     const char *infoString,
                     IOChannelMode mode,
                     IOChannelPermissions permissions, ... )
{
    va_list varArg;
    bool retVal = false;
    char *subInfoString = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( infoString, "Info String is not valid" );

    /* Check Close e NotClose Flags */
    if( !IOCHANNEL_MODEIS_VALID( mode ))
    {
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        goto outLabel;
    }
    /* Force to clean the instance */
    IOChannel_resetValuesForNewOpen( self );
    self->mode = mode;

    IOChannelBuffer_atOpen( self->ungetBuffer );
    IOChannelBuffer_atOpen( self->writeBuffer );

    va_start( varArg, permissions );
    /* Vararg is needed to load an user interface argument */
    self->currInterface = IOChannel_findInterface( self, infoString, &subInfoString );

    if( IOCHANNEL_ISSPACE( *subInfoString ))
    {
        ANY_LOG( 5, "Error in infoString format. Found unadmitted space character.",
                 ANY_LOG_ERROR );
        goto outLabel;
    }

    if( self->currInterface == NULL)
    {
        ANY_LOG( 5, "IOChannel_open(). Specified stream not exists or you typed a bad info string!",
                 ANY_LOG_FATAL );
        goto outLabel;
    }
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectNew );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectInit );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectOpen );

    /* Creating Stream! */
    self->streamPtr = IOCHANNELINTERFACE_NEW( self );
    if( IOCHANNELINTERFACE_INIT( self ) == false )
    {
        IOCHANNELINTERFACE_DELETE( self );
        goto outLabel;
    }

    retVal = IOCHANNELINTERFACE_OPEN( self, subInfoString,
                                      mode, permissions, varArg );
    va_end( varArg );

    /* Final check to set isOpen flag */
    if( retVal )
    {
        self->isOpen = true;
    }

    outLabel:
    return retVal;
}


bool IOChannel_openFromString( IOChannel *self, const char *openString, ... )
{
    va_list varArg;
    char internalBuffer[2048];
    bool retVal = false;
    char *subInfoString = (char *)NULL;
    char *typeStream = (char *)NULL;
    IOChannelReferenceValue **vectorOfReferencesValues = (IOChannelReferenceValue **)NULL;
    char *value = (char *)NULL;
    int len = 0;
    IOChannelMode mode = IOCHANNEL_MODE_UNDEFINED;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( openString, "openString is not valid" );

    va_start( varArg, openString );
    Any_vsnprintf( internalBuffer, 2048, openString, varArg );
    va_end( varArg );

    len = IOChannelReferenceValue_parseReferenceValue( internalBuffer, &vectorOfReferencesValues );

    if( len <= 0 )
    {
        return false;
    };

    ANY_REQUIRE( vectorOfReferencesValues );

    /* searching for stream type and modes in openString */
    value = IOChannelReferenceValue_getString( vectorOfReferencesValues, IOCHANNELREFERENCEVALUE_STREAM );

    if( !value )
    {
        ANY_LOG( 5, "Warning, stream type not found.", ANY_LOG_WARNING );
        return false;
    }

    len = Any_strlen( value ) + Any_strlen( IOCHANNELREFERENCEVALUE_TYPESTREAMTERMINATINGSEQUENCE );
    typeStream = ANY_NTALLOC( len + 1, char );
    ANY_REQUIRE( typeStream );
    Any_sprintf( typeStream, "%s%s", value, IOCHANNELREFERENCEVALUE_TYPESTREAMTERMINATINGSEQUENCE );

    /* Force to clean the instance */
    IOChannel_resetValuesForNewOpen( self );

    value = IOChannelReferenceValue_getString( vectorOfReferencesValues, IOCHANNELREFERENCEVALUE_MODE );
    if( value )
    {
        mode = IOChannelReferenceValue_getAccessMode( value );

        if( IOCHANNEL_MODEIS_DEFINED( mode ))
        {
            /* Check Close e NotClose Flags */
            if( !IOCHANNEL_MODEIS_VALID( mode ))
            {
                IOChannel_setError( self, IOCHANNELERROR_BFLGS );
                IOChannelReferenceValue_freeReferenceValueVector( &vectorOfReferencesValues );
                ANY_FREE( typeStream );
                return false;
            }
            self->mode = mode;
        }
        else
        {
            self->mode = IOCHANNEL_MODE_UNDEFINED;
        }

    }
    else
    {
        self->mode = mode;
    }

    IOChannelBuffer_atOpen( self->ungetBuffer );
    IOChannelBuffer_atOpen( self->writeBuffer );

    self->currInterface = IOChannel_findInterface( self, typeStream, &subInfoString );

    if( self->currInterface == NULL)
    {
        ANY_LOG( 5, "IOChannel_openFromString(). Specified stream not exists or you typed a bad info string!",
                 ANY_LOG_FATAL );
        return false;
    }

    IOCHANNEL_REQUIRE_INTERFACE( self, indirectNew );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectInit );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectOpenFromString );

    /* Creating Stream! */
    self->streamPtr = IOCHANNELINTERFACE_NEW( self );
    if( IOCHANNELINTERFACE_INIT( self ) == false )
    {
        IOCHANNELINTERFACE_DELETE( self );
        return false;
    }
    retVal = IOCHANNELINTERFACE_OPENFROMSTRING( self, vectorOfReferencesValues );

    /* Final check to set isOpen flag */
    if( retVal )
    {
        self->isOpen = true;
    }


    IOChannelReferenceValue_freeReferenceValueVector( &vectorOfReferencesValues );
    ANY_FREE( typeStream );

    return retVal;
}


int IOChannel_getModes( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    return self->mode;
}


void IOChannel_logMode( long debuglevel, IOChannelMode mode )
{
    ANY_LOG( debuglevel, "-------------------------------", ANY_LOG_INFO );
    ANY_LOG( debuglevel, "mode = %x", ANY_LOG_INFO, mode );

    if( IOCHANNEL_MODEIS_DEFINED( mode ) )
    {
        if( IOCHANNEL_MODEIS_R_ONLY( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_R_ONLY is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_W_ONLY( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_W_ONLY is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_RW( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_RW is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_CREAT( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_CREAT is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_TRUNC( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_TRUNC is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_APPEND( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_APPEND is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_CLOSE( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_CLOSE is set", ANY_LOG_INFO );
        }

        if( IOCHANNEL_MODEIS_NOTCLOSE( mode ) )
        {
            ANY_LOG( debuglevel, "  MODE_NOTCLOSE is set", ANY_LOG_INFO );
        }
    }
    else
    {
        ANY_LOG( debuglevel, "  Mode is undefined", ANY_LOG_INFO );
    }

    ANY_LOG( debuglevel, "-------------------------------", ANY_LOG_INFO );
}


void IOChannel_logPermission( long debuglevel, IOChannelPermissions permissions )
{
    ANY_LOG( debuglevel, "-------------------------------", ANY_LOG_INFO );
    ANY_LOG( debuglevel, "permission = %x", ANY_LOG_INFO, permissions );

    if(( permissions & IOCHANNEL_PERMISSIONS_R_U ) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_R_U is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_W_U ) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_W_U is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_X_U ) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_X_U is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_R_G) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_R_G is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_W_G ) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_W_G is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_X_G) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_X_G is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_R_O) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_R_O is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_W_O) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_W_O is set", ANY_LOG_INFO );
    }

    if(( permissions & IOCHANNEL_PERMISSIONS_X_O) != 0 )
    {
        ANY_LOG( debuglevel, "  PERMISSIONS_X_O is set", ANY_LOG_INFO );
    }

    ANY_LOG( debuglevel, "-------------------------------", ANY_LOG_INFO );
}


bool IOChannel_isInterfaceDefined( IOChannel *self, char *streamName )
{
    char *subInfoString = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    ANY_REQUIRE_MSG( streamName, "Bad streamName ptr was passed to IOChannel_isInterfaceDefined()!" );

    return IOChannel_findInterface( self, streamName, &subInfoString ) != NULL;
}


int IOChannel_getc( IOChannel *self )
{
    long status = -1;
    int retVal = -1;
    char ch;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            /* Read Char */
            status = IOChannel_readInternal( self, &ch, 1 );

            if( status == -1 )
            {
                ANY_LOG( 5, "IOChannel_getc(): low level read returned -1", ANY_LOG_WARNING );
                ANY_REQUIRE( IOChannel_isErrorSet( self ) );
                retVal = (int)-1;
            }
            else if( status == 0 )
            {
                ANY_REQUIRE(( IOChannel_eof( self ) ));
                retVal = (int)0;
            }
            else
            {
                retVal = (int)ch;
            }
        }
    }
    return retVal;
}


long IOChannel_putc( IOChannel *self, char ch )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE( self->writeBuffer );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotRdOnlyCheck( self ) )
        {
            /* Write Char */
            retVal = IOChannel_writeInternal( self, &ch, 1 );

            if( retVal != 1 )
            {
                if( retVal == -1 )
                {
                    ANY_LOG( 5, "IOChannel_putc(): low level write returned -1", ANY_LOG_WARNING );
                    ANY_REQUIRE( IOChannel_isErrorSet( self ) );
                }

                if( retVal == 0 )
                {
                    ANY_REQUIRE(( IOChannel_eof( self ) ));
                }
            }
        }
    }
    return retVal;
}


long IOChannel_gets( IOChannel *self, char *buffToStore, long strBuffSize )
{
    char *ptr = buffToStore;
    long retVal = -1;
    char ch = '\0';
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE( buffToStore );
    ANY_REQUIRE( strBuffSize > 0 );

    /* This safety checks are required to use IOChannel_readInternal */
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );


    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            for( i = 0; i < strBuffSize; i++, ptr++ )
            {
                retVal = IOChannel_readInternal( self, &ch, 1 );
                if( retVal == -1 )
                {
                    ANY_LOG( 5, "IOChannel_getS(): low level read returned -1", ANY_LOG_WARNING );
                    ANY_REQUIRE( IOChannel_isErrorSet( self ) );
                    break;
                }

                if( retVal == 0 )
                {
                    ANY_REQUIRE(( IOChannel_eof( self ) ));
                    break;
                }

                if( ch == '\n' )
                {
                    break;
                }
                *ptr = ch;
            }
            *ptr = '\0';

            if( retVal != -1 )
            {
                retVal = i;
            }
        }
    }
    return retVal;
}


long IOChannel_puts( IOChannel *self, char *buffToWrite, long strBuffSize )
{
    char *ptr = buffToWrite;
    long retVal = -1;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE( buffToWrite );
    ANY_REQUIRE( strBuffSize > 0 );
    ANY_REQUIRE( self->writeBuffer );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotRdOnlyCheck( self ) )
        {
            for( i = 0; i < strBuffSize; i++, ptr++ )
            {
                if( *ptr == '\0' )
                {
                    break;
                }

                retVal = IOChannel_writeInternal( self, ptr, 1 );
                if( retVal == -1 )
                {
                    ANY_LOG( 5, "IOChannel_putC(): low level write returned -1", ANY_LOG_WARNING );
                    ANY_REQUIRE( IOChannel_isErrorSet( self ) );
                    break;
                }

                if( retVal == 0 )
                {
                    ANY_REQUIRE(( IOChannel_eof( self ) ));
                    break;
                }
            }

            if( retVal != -1 )
            {
                retVal = i;
            }
        }
    }
    return retVal;
}


long IOChannel_scanf( IOChannel *self, long *nBytes, char *format, ... )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( format, "Not valid format string for scanf()" );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            va_list varArg;
            va_start( varArg, format );
            retVal = IOChannel_scanFormatting( self, nBytes, format, varArg );
            va_end( varArg );
        }
    }
    return retVal;
}


long IOChannel_vscanf( IOChannel *self,
                       long *nBytes,
                       char *format,
                       va_list varArg )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( format, "Not valid format string for scanf()" );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            retVal = IOChannel_scanFormatting( self, nBytes, format, varArg );
        }
    }
    return retVal;
}


long IOChannel_printf( IOChannel *self, const char *format, ... )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( format, "Not valid format string for printf()" );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotRdOnlyCheck( self ) )
        {
            va_list varArg;
            va_start( varArg, format );
            retVal = IOChannel_printFormatting( self, format, varArg );
            va_end( varArg );
        }
    }
    return retVal;
}


long IOChannel_vprintf( IOChannel *self, char *format, va_list varArg )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( format, "Not valid format string for vprintf()" );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotRdOnlyCheck( self ) )
        {
            retVal = IOChannel_printFormatting( self, format, varArg );
        }
    }
    return retVal;
}


long IOChannel_read( IOChannel *self, void *buffer, long size )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( buffer, "Not a valid buffer for read()" );
    ANY_REQUIRE_MSG( size >= 0, "Size must be a positive number" );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );

    if( IOChannel_isCallAllowedCheck( self ) && IOChannel_isNotWrOnlyCheck( self ) )
    {
        retVal = IOChannel_readInternal( self, buffer, size );
    }

    return retVal;
}


long IOChannel_readBlock( IOChannel *self, void *buffer, long size )
{
    long byteRead;
    long received;
    char *ptr = (char *)buffer;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( ptr, "Not a valid buffer for read()" );
    ANY_REQUIRE_MSG( size >= 0, "Size must be a positive number" );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectRead );

    byteRead = size;

    if( IOChannel_isCallAllowedCheck( self ) && IOChannel_isNotWrOnlyCheck( self ) )
    {
        while( byteRead )
        {
            /* XXX: The previous version, calling IOChannel_read, would check
             * for byteRead >= 0 every time it is called. Should we add an
             * ANY_REQUIRE here for safety?
             */

            received = IOChannel_readInternal( self, ptr, byteRead );

            if( received < 0 || IOChannel_eof( self ) ||
                IOChannel_isErrorOccurred( self ) )
            {
                break;
            }

            ptr += received;
            byteRead -= received;
        }
    }

    return size - byteRead;
}

long IOChannel_write( IOChannel *self, const void *buffer, long size )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( size >= 0, "Size must be a positive number" );
    ANY_REQUIRE( self->writeBuffer );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    if( IOChannel_isCallAllowedCheck( self ) && IOChannel_isNotRdOnlyCheck( self ) )
    {
        return IOChannel_writeInternal( self, buffer, size );
    }

    return retVal;
}



long IOChannel_writeBlock( IOChannel *self, const void *buffer, long size )
{
    long byteWrite;
    long sent;
    char *ptr = (char *)buffer;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( size >= 0, "Size must be a positive number" );
    ANY_REQUIRE( self->writeBuffer );
    ANY_REQUIRE( self->ungetBuffer );
    IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );

    byteWrite = size;

    if( IOChannel_isCallAllowedCheck( self ) && IOChannel_isNotRdOnlyCheck( self ) )
    {
        while( byteWrite )
        {
            sent = IOChannel_writeInternal( self, ptr, byteWrite );

            if( sent < 0 || IOChannel_eof( self ) || IOChannel_isErrorOccurred( self ) )
            {
                break;
            }

            ptr += sent;
            byteWrite -= sent;
        }
    }

    return size - byteWrite;
}


long IOChannel_unget( IOChannel *self, void *buffer, long size )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE( size >= 0 );
    ANY_REQUIRE( buffer );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            if( size == 0 )
            {
                ANY_LOG( 5, "IOChannel_unget() was called with size = 0", ANY_LOG_WARNING );
                retVal = 0;
                goto outLabel;
            }

            if( size > self->rdBytesFromLastWrite )
            {
                ANY_LOG( 5, "IOChannel_unget(). The size parameter is greater than "
                        "the number of bytes which were read from the last write. You can "
                        "unget [%ld] bytes yet.  "
                        "Be aware that even if return value is -1, no error is set..",
                         ANY_LOG_WARNING, self->rdBytesFromLastWrite );
                retVal = -1;
            }
            else
            {
                retVal = IOChannel_pushIntoUngetBuffer( self, buffer, size );
                if( retVal != -1 )
                {
                    self->rdBytesFromLastUnget = 0;
                    self->rdBytesFromLastWrite -= retVal;
                }
                else
                {
                    ANY_LOG( 5, "IOChannel_unget. Push returned -1 ", ANY_LOG_WARNING );
                    ANY_REQUIRE( IOChannel_isErrorSet( self ) );
                }
            }
        }
    }
    outLabel:
    return retVal;
}


long IOChannel_flush( IOChannel *self )
{
    IOChannelBuffer *writeBuffer = (IOChannelBuffer *)NULL;
    long retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        if( IOChannel_usesWriteBuffering( self ) )
        {
            writeBuffer = self->writeBuffer;
            ANY_REQUIRE( writeBuffer );

            ANY_REQUIRE( writeBuffer->index >= 0 );

            if( writeBuffer->index > 0 )
            {
                ANY_LOG( 12, "Flushing The buffer..", ANY_LOG_INFO );

                IOCHANNEL_REQUIRE_INTERFACE( self, indirectFlush );
                retVal = IOCHANNELINTERFACE_FLUSH( self );
                if( retVal != -1 )
                {
                    writeBuffer->index = 0;
                }
                else
                {
                    ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                                     "Low Level Flush returned -1, but error was not set!" );
                }
            }
        }
    }
    return retVal;
}


void IOChannel_setIsReadDataAvailableTimeout( IOChannel *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->readTimeout = usecs;
}


long IOChannel_getIsReadDataAvailableTimeout( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    return self->readTimeout;
}


bool IOChannel_isReadDataAvailable( IOChannel *self )
{
    bool retVal = false;
    int socketFd = -1;
    int *sockFdPtr = (int *)NULL;
    struct timeval timeout;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotWrOnlyCheck( self ) )
        {
            if( IOChannel_hasFd( self ) )
            {
                sockFdPtr = NULL;

                sockFdPtr = (int *)IOChannel_getProperty( self, (char *)"Fd" );
                ANY_REQUIRE_MSG( sockFdPtr, "Unable to retrieve the pointer to the fd used by the Socket!" );

                socketFd = *sockFdPtr;
                ANY_REQUIRE( socketFd > -1 );

                /* Set Timeout */
                timeout.tv_sec = ( self->readTimeout / 1000000L );
                timeout.tv_usec = ( self->readTimeout % 1000000L );

                retVal = IOChannel_internalReadSelect( self, socketFd, &timeout );
            }
            else
            {
                ANY_LOG( 7, "Socket_isReadDataAvailable Has effect only on Fd streams", ANY_LOG_WARNING );
                retVal = true;
            }
        }
    }

    return retVal;
}


void IOChannel_setIsWritePossibleTimeout( IOChannel *self, long usecs )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->writeTimeout = usecs;
}


long IOChannel_getIsWritePossibleTimeout( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    return self->writeTimeout;
}


bool IOChannel_isWritePossible( IOChannel *self )
{
    int socketFd = -1;
    struct timeval timeout;
    bool retVal = false;
    int *sockFdPtr = (int *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_isNotRdOnlyCheck( self ) )
        {
            if( IOChannel_hasFd( self ) )
            {
                sockFdPtr = (int *)NULL;

                sockFdPtr = (int *)IOChannel_getProperty( self, (char *)"Fd" );
                ANY_REQUIRE_MSG( sockFdPtr, "Unable to retrieve the pointer to the fd used by the Socket!" );

                socketFd = *sockFdPtr;
                ANY_REQUIRE( socketFd > -1 );

                /* Set Timeout */
                timeout.tv_sec = ( self->writeTimeout / 1000000L );
                timeout.tv_usec = ( self->writeTimeout % 1000000L );

                retVal = IOChannel_internalWriteSelect( self, socketFd, &timeout );
            }
            else
            {
                ANY_LOG( 7, "IOChannel_isWritePossible Has effect only on Fd streams", ANY_LOG_WARNING );
                retVal = true;
            }
        }
    }

    return retVal;
}


void IOChannel_setType( IOChannel *self, IOChannelType type )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    switch( type )
    {
        case IOCHANNELTYPE_FD:
        case IOCHANNELTYPE_SOCKET:
        case IOCHANNELTYPE_MEMPTR:
        case IOCHANNELTYPE_ANSIFILE:
        case IOCHANNELTYPE_GENERICHANDLE:
            self->type = type;
            break;
        case IOCHANNELTYPE_NOTSET:
            ANY_REQUIRE_MSG( NULL, "User cannot set IOCHANNELTYPE_NOTSET!" );
            break;
        default:
            ANY_LOG( 5, "IOChannel_setType(). Bad IOChannel type was passed[%d]", ANY_LOG_WARNING, (int)type );
            break;
    }
}


void IOChannel_setUngetBuffer( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( size > 0, "IOChannel_setUngetBuffer: bad buffer size!" );

    ANY_REQUIRE( self->ungetBuffer );
    IOChannelBuffer_set( self->ungetBuffer, buffer, size );
}


void IOChannel_setWriteBuffer( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( size > 0, "IOChannel_setWriteBuffer: bad buffer size!" );

    if( IOChannel_hasPointer( self ) == false )
    {
        if( buffer != NULL)
        {
            self->writeBufferIsExternal = true;
        }
        else
        {
            self->writeBufferIsExternal = false;
        }

        ANY_REQUIRE( self->writeBuffer );
        IOChannelBuffer_set( self->writeBuffer, buffer, size );
    }
}


bool IOChannel_setUseWriteBuffering( IOChannel *self, bool useBuffering, bool autoResize )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_hasPointer( self ) == false )
    {
        if( useBuffering == false )
        {
            IOChannel_flush( self );
        }
        self->usesWriteBuffering = useBuffering;
        self->autoResize = autoResize;
        retVal = true;
    }

    return retVal;
}


long IOChannel_getWrittenBytes( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        ANY_REQUIRE( self->rdDeployedBytes >= 0 );
        return self->wrDeployedBytes;
    }
    else
    {
        return -1;
    }
}


long IOChannel_getReadBytes( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        ANY_REQUIRE( self->rdDeployedBytes >= 0 );
        return self->rdDeployedBytes;
    }
    else
    {
        return -1;
    }
}


long IOChannel_seek( IOChannel *self, long offset, IOChannelWhence whence )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        if( IOChannel_flush( self ) != -1 )
        {
            IOCHANNEL_REQUIRE_INTERFACE( self, indirectSeek );
            retVal = (long)IOCHANNELINTERFACE_SEEK( self, offset, whence );

            if( retVal == -1 )
            {
                /* Checks interfaces bugs: if -1 is returned, error must be set */
                ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                                 "Low Level Seek returned a negative value, but error was not set!" );
            }
        }
        else
        {
            ANY_LOG( 5, "IOChannel_seek(). Function was not called, because IOChannel_flush returned -1",
                     ANY_LOG_ERROR );

            ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                             "Low Level Flush returned -1, but error was not set!" );
        }
    }

    return retVal;
}


long IOChannel_tell( IOChannel *self )
{
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    retVal = IOChannel_seek( self, 0, IOCHANNELWHENCE_CUR );

    return retVal;
}


void IOChannel_rewind( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_seek( self, 0, IOCHANNELWHENCE_SET ) == 0 )
    {
        self->foundEof = false;
    }
}



void IOChannel_resetIndexes( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isCallAllowedCheck( self ) )
    {
        IOChannel_flush( self );
        self->currentIndexPosition = 0;
        self->rdBytesFromLastWrite = 0;
        self->wrDeployedBytes = 0;
        self->rdDeployedBytes = 0;
    }
}


bool IOChannel_eof( IOChannel *self )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        ungetBuffer = self->ungetBuffer;
        ANY_REQUIRE( ungetBuffer );
        ANY_REQUIRE( ungetBuffer->index >= 0 );

        if(( self->foundEof ) && ( ungetBuffer->index == 0 ))
        {
            retVal = true;
        }
    }

    return retVal;
}


bool IOChannel_hasFd( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        retVal = self->type == IOCHANNELTYPE_FD || self->type == IOCHANNELTYPE_SOCKET;
    }

    return retVal;
}


bool IOChannel_hasBerkeleySocket( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        retVal = self->type == IOCHANNELTYPE_SOCKET;
    }

    return retVal;
}


bool IOChannel_hasPointer( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        retVal = self->type == IOCHANNELTYPE_MEMPTR;
    }
    return retVal;
}


bool IOChannel_hasAnsiFILE( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        retVal = self->type == IOCHANNELTYPE_ANSIFILE;
    }
    return retVal;
}


bool IOChannel_isOpen( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    retVal = self->isOpen;

    return retVal;
}


bool IOChannel_isErrorOccurred( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    retVal = IOChannel_isErrorSet( self );

    return retVal;
}


IOChannelError IOChannel_getErrorNumber( IOChannel *self )
{
    IOChannelError error;
    IOChannelError retVal = IOCHANNELERROR_NONE;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    error = self->errorType;
    for( i = 0; i < IOChannelErrorTypeTable_len; i++ )
    {
        if( IOChannelErrorTypeTable[ i ].errorNumber == error )
        {
            retVal = IOChannelErrorTypeTable[ i ].errorNumber;
            break;
        }
    }
    return retVal;
}


char *IOChannel_getErrorDescription( IOChannel *self )
{
    IOChannelError error;
    char *retVal = (char *)NULL;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    error = self->errorType;
    for( i = 0; i < IOChannelErrorTypeTable_len; i++ )
    {
        if( IOChannelErrorTypeTable[ i ].errorNumber == error )
        {
            retVal = IOChannelErrorTypeTable[ i ].errorDescription;
            break;
        }
    }

    ANY_REQUIRE( retVal );

    return retVal;
}


int IOChannel_getErrnoValue( IOChannel *self )
{
    int retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    retVal = self->errnoValue;

    return retVal;
}


void IOChannel_cleanError( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->errorType = IOCHANNELERROR_NONE;
}


char *IOChannel_getStreamType( IOChannel *self )
{
    IOChannelInterface *aux = (IOChannelInterface *)NULL;
    char *retVal = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        aux = self->currInterface;
        ANY_REQUIRE( aux );
        retVal = aux->streamName;
        ANY_REQUIRE( retVal );
    }

    return retVal;
}


void IOChannel_setError( IOChannel *self, IOChannelError errorNumber )
{
    IOChannelErrorType *ptr = IOChannelErrorTypeTable;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->errorType = IOCHANNELERROR_NOTDEF;

    for( i = 0; i < IOChannelErrorTypeTable_len; i++, ptr++ )
    {
        if( ptr->errorNumber == errorNumber )
        {
            self->errorType = ptr->errorNumber;
            break;
        }
    }
}


void IOChannel_setSysError( IOChannel *self, int error )
{
    IOChannelSysError *ptr = IOChannelSysErrorTable;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->errorType = IOCHANNELERROR_NOTDEF;

    for( i = 0; i < IOChannelSysErrorTable_len; i++, ptr++ )
    {
        if( ptr->sysError == error )
        {
            IOChannel_setError( self, ptr->errorNumber );
            break;
        }
    }
}


bool IOChannel_close( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    if( IOChannel_isOpenCheck( self ) )
    {
        IOCHANNEL_REQUIRE_INTERFACE( self, indirectClose );
        IOCHANNEL_REQUIRE_INTERFACE( self, indirectClear );
        IOCHANNEL_REQUIRE_INTERFACE( self, indirectDelete );

        if( IOChannel_flush( self ) == -1 )
        {
            ANY_LOG( 5, "An error occurred flushing the buffer at close time", ANY_LOG_WARNING );
            ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                             "Low Level Flush returned -1, but error was not set!" );
        }
        else
        {
            IOChannelBuffer_atClose( self->ungetBuffer );
            IOChannelBuffer_atClose( self->writeBuffer );

            /* Closing stream and relasing resouces */
            retVal = IOCHANNELINTERFACE_CLOSE( self );
            if( retVal )
            {
                IOCHANNELINTERFACE_CLEAR( self );
                IOCHANNELINTERFACE_DELETE( self );
                IOChannel_resetValuesForNewOpen( self );
            }
            else
            {
                ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                                 "Low Level Close returned false, but error was not set!" );
            }
        }
    }
    else
    {
        ANY_LOG( 5, "The specified IOChannel instance is not open", ANY_LOG_WARNING );
    }

    return retVal;
}


void *IOChannel_getStreamPtr( IOChannel *self )
{
    void *retVal = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    /* Do not check isOpen because this is used by low       */
    /* level _new, _init _open, _close _clear and _delete !  */

    /* Do Not Check For Stream Ptr: Null Stream has */
    /* not any data associated to the stream */
    retVal = self->streamPtr;

    return retVal;
}


void *IOChannel_getProperty( IOChannel *self, const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( propertyName, "Not Valid Property Name To Get" );

    if( IOChannel_isOpenCheck( self ) )
    {
        IOCHANNEL_REQUIRE_INTERFACE( self, indirectGetProperty );
        retVal = IOCHANNELINTERFACE_GETPROPERTY( self, propertyName );
    }

    return retVal;
}


bool IOChannel_setProperty( IOChannel *self, const char *propertyName,
                            void *propertyValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( propertyName, "Not Valid Property Name To Set" );

    if( IOChannel_isOpenCheck( self ) )
    {
        IOCHANNEL_REQUIRE_INTERFACE( self, indirectGetProperty );
        retVal = IOCHANNELINTERFACE_SETPROPERTY( self, propertyName, propertyValue );
    }

    return retVal;
}


bool IOChannel_addInterface( IOChannel *self, IOChannelInterface *currInterface )
{
    bool retVal = false;
    IOChannelPlugin *plugin = (IOChannelPlugin *)NULL;
    IOChannelInterface *ptr = (IOChannelInterface *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid = IOCHANNEL_VALID );
    ANY_REQUIRE_MSG( currInterface, "Cannot add a NULL iterface!" );

    ptr = ANY_TALLOC( IOChannelInterface );
    ANY_REQUIRE_MSG( ptr, "Cannot allocate space to add the interface" );

    Any_memcpy( ptr, currInterface, sizeof( IOChannelInterface ));

    ptr->streamName = Any_strdup( currInterface->streamName );
    ANY_REQUIRE_MSG( ptr->streamName, "Unable to allocate memory for interface streamName" );

    plugin = ANY_TALLOC( IOChannelPlugin );
    ANY_REQUIRE_MSG( plugin, "Unable to allocate memory for a new plugin" );

    plugin->currInterface = ptr;
    plugin->libHandle = NULL;

    retVal = MTList_insert( self->userStream, plugin );

    /* unreachable code: MTList_insert() always returns true
   *
  if( retVal == false )
  {
    ANY_LOG( 5, "IOChannel_addInterface(). Cannot add interface(MTList_insert returned -1)", ANY_LOG_WARNING );

    ANY_FREE( ptr->streamName );
    ANY_FREE( ptr );
    ANY_FREE( plugin );
  }
   */

    /*
   * effectively always returns 'true' because the return value
   * of MTList_insert() is always 'true'
   */
    return retVal;
}


bool IOChannel_usesWriteBuffering( IOChannel *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    retVal = self->usesWriteBuffering;

    return retVal;
}


long IOChannel_addToWriteBuffer( IOChannel *self, const void *buffer, long size )
{
    IOChannelBuffer *writeBuffer = (IOChannelBuffer *)NULL;
    const char *ptr = (char *)NULL;
    char *base = (char *)NULL;
    char *aux = (char *)NULL;
    long bytesToWrite = 0;
    long leftBytes = 0;
    long retVal = 0;
    void *newBuffer = (void *)NULL;
    long newSize = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    writeBuffer = self->writeBuffer;
    bytesToWrite = size;
    ptr = buffer;

    retVal = size;

    while( bytesToWrite )
    {
        leftBytes = (long)( writeBuffer->size - writeBuffer->index );
        base = writeBuffer->ptr;

        aux = base + writeBuffer->index;
        ANY_REQUIRE( aux );
        ANY_REQUIRE( ptr );

        if( bytesToWrite <= leftBytes )
        {
            Any_memcpy( aux, ptr, bytesToWrite );
            writeBuffer->index += bytesToWrite;
            break;
        }
        else
        {
            Any_memcpy( aux, ptr, leftBytes );
            writeBuffer->index += leftBytes;
            ptr += leftBytes;
            bytesToWrite -= leftBytes;

            if(( self->writeBufferIsExternal == false ) && ( self->autoResize ))
            {
                newBuffer = (void *)NULL;
                newSize = 0;

                /* Any Policy ? */
                newSize = ( writeBuffer->size + bytesToWrite ) * 2;

                newBuffer = ANY_BALLOC( newSize );
                ANY_REQUIRE( newBuffer );

                ANY_LOG( 12, "AutoResize is Reallocating Buffer.. "
                        "(oldBufferSize[%ld], newBufferSize[%ld])",
                         ANY_LOG_INFO, writeBuffer->size, newSize );
                ANY_REQUIRE( writeBuffer->ptr );

                Any_memcpy( newBuffer, writeBuffer->ptr, writeBuffer->size );
                ANY_FREE( writeBuffer->ptr );

                if( writeBuffer->ptr == writeBuffer->defaultBuffer )
                {
                    writeBuffer->defaultBuffer = newBuffer;
                }
                writeBuffer->index = writeBuffer->size;
                writeBuffer->ptr = newBuffer;
                writeBuffer->size = newSize;
            }
            else
            {
                /* The buffer Was created externally */
                if( IOChannel_flush( self ) == -1 )
                {
                    ANY_LOG( 5, "IOChannel_addToWriteBuffer. Unable to flush write buffer!", ANY_LOG_ERROR );

                    ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                                     "Low Level Flush returned -1, but error was not set!" );
                    retVal = -1;
                    break;
                }
            }
        }
    }
    return retVal;
}


long IOChannel_getWriteBufferedBytes( IOChannel *self )
{
    IOChannelBuffer *writeBuffer = (IOChannelBuffer *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    writeBuffer = self->writeBuffer;
    ANY_REQUIRE( writeBuffer );

    return (long)writeBuffer->index;
}


void *IOChannel_getInternalWriteBufferPtr( IOChannel *self )
{
    IOChannelBuffer *writeBuffer = (IOChannelBuffer *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    writeBuffer = self->writeBuffer;
    ANY_REQUIRE( writeBuffer );

    retVal = writeBuffer->ptr;

    return retVal;
}


long long IOChannel_getStreamPosition( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    return ( self->currentIndexPosition );
}


void IOChannel_clear( IOChannel *self )
{
    IOChannelPlugin *ptr = (IOChannelPlugin *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );

    self->valid = IOCHANNEL_INVALID;

    MTLIST_FOREACH_NOLOCK_BEGIN( self->userStream )
            {
                ptr = (IOChannelPlugin *)MTLIST_FOREACH_ELEMENTPTR;

                IOChannelPlugin_clear( ptr );
                IOChannelPlugin_delete( ptr );
            }
    MTLIST_FOREACH_NOLOCK_END;

    MTList_clear( self->userStream );
    MTList_delete( self->userStream );

    IOChannelBuffer_clear( self->ungetBuffer );
    IOChannelBuffer_delete( self->ungetBuffer );

    IOChannelBuffer_clear( self->writeBuffer );
    IOChannelBuffer_delete( self->writeBuffer );

    IOChannel_resetObject( self );
}


void IOChannel_delete( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


void IOChannel_valid( IOChannel *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
}

/*---------------------------------------------------------------------------*/
/*    Private Functions                                                      */
/*---------------------------------------------------------------------------*/

static IOChannelInterface *IOChannel_findInterface( IOChannel *self,
                                                    const char *infoString,
                                                    char **subInfoString )
{
    IOChannelInterface *retVal = NULL;
    char typeStream[IOCHANNEL_INFOSTRING_MAXLEN];
    int i = 0;
    int slashCounter = 0;

    *subInfoString = (char *)infoString;

    for( i = 0; i < IOCHANNEL_INFOSTRING_MAXLEN; i++, *subInfoString += 1 )
    {
        if( **subInfoString == '\0' )
        {
            ANY_LOG( 5, "Bad IOChannel_open() info string. No ':' was found!", ANY_LOG_WARNING );
            goto outLabel;
        }
        typeStream[ i ] = **subInfoString;

        if( **subInfoString == ':' )
        {
            slashCounter = 0;
            typeStream[ i ] = '\0';

            ( *subInfoString )++;
            for( ; slashCounter < 2; slashCounter++, ( *subInfoString )++ )
            {
                if( *( *subInfoString ) != '/' )
                {
                    break;
                }
            }
            if( slashCounter != 2 )
            {
                ANY_LOG( 5, "Bad IOChannel_open() infoString. Check Slashes after ':'!", ANY_LOG_WARNING );
                goto outLabel;
            }
            else
            {
                break;
            }
        }
    }


    retVal = IOChannel_loadInterface( self, typeStream );

    outLabel:

    return retVal;
}


static IOChannelInterface *IOChannel_loadInterface( IOChannel *self,
                                                    char *typeStream )
{

    IOChannelInterface *retVal = NULL;
    IOChannelInterface *ptr = NULL;
    IOChannelPlugin *plugin = (IOChannelPlugin *)NULL;
    bool interfaceWasFound = false;

    /* First search the stream type on the user's defined streams */
    MTLIST_FOREACH_NOLOCK_BEGIN( self->userStream )
            {
                plugin = (IOChannelPlugin *)MTLIST_FOREACH_ELEMENTPTR;

                ptr = (IOChannelInterface *)plugin->currInterface;

                if( Any_strncmp( typeStream, ptr->streamName, IOCHANNEL_STREAMNAME_MAXLEN ) == 0 )
                {
                    /* Here retVal can be set */
                    retVal = ptr;
                    MTLIST_FOREACH_NOLOCK_BREAK;
                }
            }
    MTLIST_FOREACH_NOLOCK_END;

    /* Check MTlist assignement */
    if( retVal )
    {
        goto outLabel;
    }
    else
    {
        plugin = NULL;

        plugin = IOChannelPlugin_new();
        if( !plugin )
        {
            ANY_LOG( 0, "Unable to allocate memory for a new IOChannelPlugin", ANY_LOG_ERROR );
            goto outLabel;
        }

        if( IOChannelPlugin_init( plugin, typeStream ) != 0 )
        {
            ANY_LOG( 0, "Unable to initialize a new IOChannelPlugin for the stream '%s'",
                     ANY_LOG_ERROR, typeStream );
            IOChannelPlugin_delete( plugin );
            plugin = NULL;
            goto outLabel;
        }

        /* Get the plugin's interface */
        retVal = IOChannelPlugin_getInterface( plugin );

        if( retVal == (IOChannelInterface *)NULL)
        {
            ANY_LOG( 0, "Unable To Find the requested Plugin[%s]!!!", ANY_LOG_ERROR, typeStream );

            IOChannelPlugin_clear( plugin );
            IOChannelPlugin_delete( plugin );

            goto outLabel;
        }

        /* unreachable code: MTList_insert() always returns true
     *
    if( MTList_insert( self->userStream, plugin ) == false )
    {
      ANY_LOG( 0, "Unable to add plugin into the list! MTList_insert() returned false", ANY_LOG_ERROR );
      ANY_FREE( plugin );
    }
    else
    {
      interfaceWasFound = true;
    }
     * changed to:
     */
        MTList_insert( self->userStream, plugin );
        interfaceWasFound = true;
    }

    if( !interfaceWasFound )
    {
        ANY_LOG( 5, "Stream Interface was not found on declared streams!", ANY_LOG_WARNING );
        retVal = NULL;
    }

    outLabel:

    return retVal;
}


static long IOChannel_printFormatting( IOChannel *self,
                                       const char *format,
                                       va_list varArg )
{
    long retVal = 0;
    long iniOffset = 0;
    long endOffset = 0;
    void *arg = (void *)NULL;
    char buffForAddress[32];
    long buffLen = 0;
    char *buffer = (char *)NULL;
    char *chTmp = (char *)NULL;
    int uIntLen = 0;
    char uIntBuff[20];
    unsigned int uIntTmp = 0;
    IOChannelCallBack *callBack = (IOChannelCallBack *)NULL;
    void *callBackData = (void *)NULL;
    long callBackWrBytes = -1;
    long len = 0;
    long i = 0;
    bool doEscapeChars = false;

    ANY_REQUIRE( self->writeBuffer );
    ANY_REQUIRE( self->ungetBuffer );

    /* Get initial Offset */
    iniOffset = self->wrDeployedBytes;

    for( ; ( *format ) &&
           ( IOChannel_eof( self ) == false ) &&
           ( IOChannel_isErrorSet( self ) == false ); format++ )
    {
        if( *format == '%' )
        {
            switch( *( ++format ))
            {
                case '%':
                    if( IOChannel_writeInternal( self, format, 1 ) != 1 )
                    {
                        ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                    }
                    break;

                case 'c':
                    IOCHANNEL_PRINT_ITEM( self, "%c", char, varArg );
                    break;

                case 'u':
                    IOCHANNEL_PRINT_ITEM( self, "%u", unsigned int, varArg );
                    break;

                case 'd':
                    IOCHANNEL_PRINT_ITEM( self, "%d", int, varArg );
                    break;

                case 'f':
                    IOCHANNEL_PRINT_ITEM( self, "%.7e", float, varArg );
                    break;

                case 'p':
                {
                    arg = (void *)NULL;
                    arg = va_arg( varArg, void* );
                    ANY_REQUIRE( arg );

                    buffLen = 0;

                    buffLen = Any_snprintf( buffForAddress, 32, "%p", arg );
                    ANY_REQUIRE( buffLen > 0 );

                    if( IOChannel_writeInternal( self, buffForAddress, buffLen ) != buffLen )
                    {
                        ANY_LOG( 0, "Less Bytes Than required Were written Expanding %%p!", ANY_LOG_WARNING );
                        ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                    }
                }
                    break;

                case 'S':
                    doEscapeChars = true;
                case 's':
                {
                    buffer = (char *)NULL;
                    buffer = va_arg( varArg, char* );
                    ANY_REQUIRE( buffer );

                    while(( *buffer ) &&
                          ( IOChannel_eof( self ) == false ) &&
                          ( IOChannel_isErrorSet( self ) == false ))
                    {

                        if( doEscapeChars )
                        {
                            len = IOChannel_writeEscapedChar( self, *buffer );
                        }
                        else
                        {
                            len = IOChannel_writeInternal( self, buffer, 1 );
                        }

                        if( len < 1 )
                        {
                            ANY_LOG( 0, "Less Bytes Than required Were written Expanding %%s!", ANY_LOG_WARNING );

                            ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                            break;
                        }
                        buffer++;
                    }
                }
                    doEscapeChars = false;
                    break;

                case 'L':
                    switch( *( ++format ))
                    {
                        case 'f':
                            IOCHANNEL_PRINT_ITEM( self, "%.18Le", long double, varArg );
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                    break;

                case 'h':
                {
                    switch( *( ++format ))
                    {
                        case 'u':
                            IOCHANNEL_PRINT_ITEM( self, "%hu", unsigned short int, varArg );
                            break;

                        case 'd':
                            IOCHANNEL_PRINT_ITEM( self, "%hd", short int, varArg );
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case 'l':
                {
                    switch( *( ++format ))
                    {
                        case 'u':
                            IOCHANNEL_PRINT_ITEM( self, "%lu", unsigned long int, varArg );
                            break;

                        case 'd':
                            IOCHANNEL_PRINT_ITEM( self, "%ld", long int, varArg );
                            break;

                        case 'f':
                            IOCHANNEL_PRINT_ITEM( self, "%.16e", double, varArg );
                            break;

                        case 'l':
                        {
                            switch( *( ++format ))
                            {
                                case 'd':
                                    IOCHANNEL_PRINT_ITEM( self, "%lld", long long int, varArg );
                                    break;

                                case 'u':
                                    IOCHANNEL_PRINT_ITEM( self, "%llu", unsigned long long int,
                                                          varArg );
                                    break;

                                default:
                                    IOChannel_setError( self, IOCHANNELERROR_INCR );
                                    break;
                            }
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case 'q':
                {
                    switch( *( ++format ))
                    {
                        case 'c':
                        {
                            chTmp = (char *)NULL;
                            chTmp = va_arg( varArg, char* );
                            ANY_REQUIRE( chTmp );

                            if( IOCHANNEL_ISPRINT( *chTmp ) )
                            {
                                if( IOChannel_writeInternal( self, (char *)"\'", 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                if( IOChannel_writeInternal( self, chTmp, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                if( IOChannel_writeInternal( self, (char *)"\'", 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                            }
                            else
                            {
                                uIntTmp = (unsigned int)( *chTmp );
                                uIntLen = 0;
                                uIntLen = Any_snprintf( uIntBuff, 20, "'\\x%02x'", uIntTmp );
                                ANY_REQUIRE( uIntLen > 0 );
                                if( IOChannel_writeInternal( self, uIntBuff, uIntLen ) != uIntLen )
                                {
                                    ANY_LOG( 5,
                                             "Unable to print hex quoted char: less bytes than requested number were printed",
                                             ANY_LOG_ERROR );
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                }
                            }
                            break;
                        }
                            break;

                        case 's':
                        {
                            buffer = (char *)NULL;

                            buffer = va_arg( varArg, char* );
                            ANY_REQUIRE( buffer );

                            if( IOChannel_writeInternal( self, (char *)"\"", 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            while(( *buffer ) &&
                                  ( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                //encode \ and "
                                if( *buffer == '"' || *buffer == '\\' )
                                {
                                    if( IOChannel_writeInternal( self, "\\", 1 ) != 1 )
                                    {
                                        ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                    ( IOChannel_isErrorSet( self ) ));
                                        break;
                                    }
                                }

                                if( IOChannel_writeInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                buffer++;
                            }

                            if( IOChannel_writeInternal( self, (char *)"\"", 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case '@':
                {
                    callBackWrBytes = -1;
                    callBackData = (void *)NULL;
                    callBack = (IOChannelCallBack *)NULL;

                    callBack = va_arg( varArg, IOChannelCallBack* );
                    ANY_REQUIRE( callBack );
                    callBackData = va_arg( varArg, void* );
                    /* Do not check callBackData because NULL also is allowed */

                    /* Launch callback */
                    callBackWrBytes = ( *callBack )( callBackData, self, false );
                    if( callBackWrBytes == -1 )
                    {
                        IOChannel_setError( self, IOCHANNELERROR_BCLLBKW );
                    }
                }
                    break;

                case '*':
                {
                    switch( *( ++format ))
                    {
                        case 'q':/* %*qs */
                        {
                            buffer = (char *)NULL;
                            len = 0;

                            len = va_arg( varArg, long );
                            ANY_REQUIRE_MSG( len > 0, "IOChannel_printFormatting. "
                                    "You used %*s but maybe you: "
                                    "1)forget to put the size parameter before the string pointer "
                                    "2)passed a <= size value!" );

                            buffer = va_arg( varArg, char* );
                            ANY_REQUIRE_MSG( buffer, "Not valid pointer was passed using %*qs" );

                            if( *( ++format ) != 's' )
                            {
                                ANY_LOG( 5, "IOChannel_printFormatting. You wrote [%%*q%c] instead of[%%*qs]",
                                         ANY_LOG_ERROR, *format );
                                ANY_REQUIRE( NULL );
                                break;
                            }

                            if( IOChannel_writeInternal( self, (char *)"\"", 1 ) != 1 )
                            {
                                ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                            ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            i = 0;
                            while(( i < len ) &&
                                  ( *buffer ) &&
                                  ( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                //encode \ and "
                                if( *buffer == '"' || *buffer == '\\' )
                                {
                                    if( IOChannel_writeInternal( self, "\\", 1 ) != 1 )
                                    {
                                        ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                    ( IOChannel_isErrorSet( self ) ));
                                        break;
                                    }
                                    i++;
                                    if( i >= len )
                                        break;
                                }

                                if( IOChannel_writeInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                i++;
                                buffer++;
                                ANY_REQUIRE( buffer );
                            }

                            if(( IOChannel_eof( self ) == false ) &&
                               ( IOChannel_isErrorSet( self ) == false ))
                            {
                                if(( i < ( len - 1 )) && ( *buffer != '\0' ))
                                {
                                    ANY_LOG( 5,
                                             "IOChannel_printf: [%%*qs] was used with a string whose size is greater than imposed limit",
                                             ANY_LOG_WARNING );
                                    ANY_REQUIRE( NULL );
                                }
                            }

                            if( IOChannel_writeInternal( self, (char *)"\"", 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }
                        }
                            break;

                        case 's':/* %*s */
                        {
                            len = 0;
                            buffer = (char *)NULL;

                            len = va_arg( varArg, long );
                            ANY_REQUIRE_MSG( len > 0, "IOChannel_printFormatting. "
                                    "You used %*s but maybe you: "
                                    "1)forget to put the size parameter "
                                    "before the string pointer "
                                    "2)passed a <= size value!" );

                            buffer = va_arg( varArg, char* );
                            ANY_REQUIRE_MSG( buffer, "Not valid pointer was passed using %*s" );

                            i = 0;
                            while(( i < len ) &&
                                  ( *buffer ) &&
                                  ( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                if( IOChannel_writeInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                i++;
                                buffer++;
                                ANY_REQUIRE( buffer );
                            }

                            if(( IOChannel_eof( self ) == false ) &&
                               ( IOChannel_isErrorSet( self ) == false ))
                            {
                                if(( i == ( len - 1 )) && ( *buffer != '\0' ))
                                {
                                    ANY_LOG( 5, "IOChannel_printf: [%%*s] was used with a string"
                                            "whose size is greater than imposed limit", ANY_LOG_WARNING );
                                    ANY_REQUIRE( NULL );
                                }
                            }
                            break;
                        }

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                default:
                    IOChannel_setError( self, IOCHANNELERROR_INCR );
                    break;
            }
        }
        else
        {
            /* Write char by char */
            if( IOChannel_writeInternal( self, format, 1 ) != 1 )
            {
                ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                            ( IOChannel_isErrorSet( self ) ));
                break;
            }
        }
    }

    /* Get final Offset */
    endOffset = self->wrDeployedBytes;

    if( IOChannel_isErrorSet( self ) == false )
    {
        retVal = ( endOffset - iniOffset );
        ANY_REQUIRE( retVal >= 0 );
    }
    else
    {
        retVal = -1;
    }
    return retVal;
}


static long IOChannel_scanFormatting( IOChannel *self,
                                      long *nBytes,
                                      char *format,
                                      va_list varArg )
{
    long retVal = 0;
    long nItems = 0;
    long iniOffset = 0;
    long endOffset = 0;
    char buffer[1] = "";
    long callBackRdBytes = 0;
    void *callBackData = (void *)NULL;
    char *param = (char *)NULL;
    void **arg = (void **)NULL;
    char buffForAddress[32];
    char *aux = (char *)NULL;
    long nUnget = 0;
    unsigned int uIntTmp = 0;
    char uIntBuff[20];
    long len = 0;
    long i = 0;
    IOChannelCallBack *callBack = (IOChannelCallBack *)NULL;

    /* required for IOChannel_readInternal() use */
    ANY_REQUIRE( self->ungetBuffer );

    /* Get initial Offset */
    iniOffset = self->rdBytesFromLastWrite;

    for( ; ( *format ) &&
           ( IOChannel_eof( self ) == false ) &&
           ( IOChannel_isErrorSet( self ) == false ); format++ )
    {
        /* Ignoring Spaces */
        while( *format == ' ' )
        {
            format++;
        }

        if( !*format )
        {
            break;
        }

        if( *format == '%' )
        {
            switch( *( ++format ))
            {
                case '@':
                {
                    callBack = (IOChannelCallBack *)NULL;
                    callBackRdBytes = 0;
                    callBackData = (void *)NULL;

                    callBack = va_arg( varArg, IOChannelCallBack* );
                    ANY_REQUIRE( callBack );

                    callBackData = va_arg( varArg, void* );
                    /* Do not check callBackData because NULL also is allowed */

                    /* Launch callback */
                    callBackRdBytes = ( *callBack )( callBackData, self, true );
                    if( callBackRdBytes == -1 )
                    {
                        IOChannel_setError( self, IOCHANNELERROR_BCLLBKR );
                    }
                }
                    break;

                case '%':
                {
                    IOCHANNEL_READSPACES( self, buffer );
                    if( *buffer != '%' )
                    {
                        ANY_LOG( 5, "Matching failed: Format is [%%], but was read[%c]",
                                 ANY_LOG_ERROR, *buffer );
                    }
                    --nItems;
                }
                    break;

                case 'c':
                {
                    param = (char *)NULL;

                    param = va_arg( varArg, char* );
                    ANY_REQUIRE( param );

                    IOCHANNEL_READSPACES( self, buffer );
                    *param = *buffer;
                }
                    break;

                case 'u':
                {
                    IOCHANNEL_SCAN_ITEM( self, "%u", buffer, unsigned int,
                                         varArg, false, format );
                }
                    break;

                case 'd':
                {
                    IOCHANNEL_SCAN_ITEM( self, "%d", buffer, int,
                                         varArg, false, format );
                }
                    break;

                case 'f':
                {
                    IOCHANNEL_SCAN_ITEM( self, "%f", buffer, float,
                                         varArg, true, format );
                }
                    break;

                case 'p':
                {
                    /* not null because buffForAddress is a static array declared in this function,
                       no check needed ( ANY_REQUIRE( aux ) )*/
                    aux = (char *)buffForAddress;

                    ++format;

                    arg = va_arg( varArg, void** );
                    /*ANY_REQUIRE( arg );*/

                    IOCHANNEL_READSPACES( self, aux );
                    while( !IOCHANNEL_ISSPACE( *aux ) &&
                           ( *aux != *format ) &&
                           ( IOChannel_eof( self ) == false ) &&
                           ( IOChannel_isErrorSet( self ) == false ))
                    {
                        aux++;
                        if( IOChannel_readInternal( self, aux, 1 ) != 1 )
                        {
                            ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                            break;
                        }
                        ANY_REQUIRE( aux );
                    }

                    if( *format == '\0' )
                    {
                        nUnget = 0;

                        nUnget = IOChannel_unget( self, buffer, 1 );
                        if( nUnget != 1 )
                        {
                            ANY_LOG( 0, "There's no Space in the unget buffer for "
                                    "scanf last char. Unget Retval[%ld], Lost char is[%c]",
                                     ANY_LOG_ERROR, nUnget, *buffer );
                        }
                    }
                    aux++;
                    *aux = '\0';

                    /*sprintf( buffForAddress, "0xbfba9e28" );*/
                    /*ANY_LOG( 0, "%s", ANY_LOG_INFO, buffForAddress );*/

                    Any_sscanf( buffForAddress, "%p", arg );
                }
                    break;

                case 's':
                {
                    int count = 0;
                    param = (char *)NULL;

                    ++format;

                    param = va_arg( varArg, char* );
                    ANY_REQUIRE( param );

                    IOCHANNEL_READSPACES( self, buffer );
                    while( !IOCHANNEL_ISSPACE( *buffer ) &&
                           ( *buffer != *format ) &&
                           ( IOChannel_eof( self ) == false ) &&
                           ( IOChannel_isErrorSet( self ) == false ))
                    {
                        *param = *buffer;
                        param++;
                        count++;

                        if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                        {
                            ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                            break;
                        }
                    }
                    *param = '\0';

                    /* the readed string is empty */
                    if( count == 0 )
                    {
                        nItems--;
                    }

                    if(( *format == '\0' ) && ( IOChannel_eof( self ) == false ))
                    {
                        nUnget = 0;

                        nUnget = IOChannel_unget( self, buffer, 1 );
                        if( nUnget != 1 )\

                        {
                            ANY_LOG( 0, "There's no Space in the unget buffer for "
                                    "scanf last char. Unget Retval[%ld], Lost char is[%c]",
                                     ANY_LOG_ERROR, nUnget, *buffer );
                        }
                    }
                }
                    break;

                case 'L':
                {
                    switch( *( ++format ))
                    {
                        case 'f':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%Lf", buffer, long double,
                                                 varArg, true, format );
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case 'h':
                {
                    switch( *( ++format ))
                    {
                        case 'u':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%hu", buffer, unsigned short int,
                                                 varArg, false, format );
                        }
                            break;

                        case 'd':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%hd", buffer, short int,
                                                 varArg, false, format );
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case 'l':
                {
                    switch( *( ++format ))
                    {
                        case 'u':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%lu", buffer, unsigned long int,
                                                 varArg, false, format );
                        }
                            break;

                        case 'd':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%ld", buffer, long int,
                                                 varArg, false, format );
                        }
                            break;

                        case 'f':
                        {
                            IOCHANNEL_SCAN_ITEM( self, "%lf", buffer, double,
                                                 varArg, true, format );
                        }
                            break;

                        case 'l':
                        {
                            switch( *( ++format ))
                            {
                                case 'd':
                                {
                                    IOCHANNEL_SCAN_ITEM( self, "%lld", buffer, long long int,
                                                         varArg, false, format );
                                }
                                    break;

                                case 'u':
                                {
                                    IOCHANNEL_SCAN_ITEM( self, "%llu", buffer, unsigned long long int,
                                                         varArg, false, format );
                                }
                                    break;

                                default:
                                    IOChannel_setError( self, IOCHANNELERROR_INCR );
                                    break;
                            }
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case 'q':
                {
                    switch( *( ++format ))
                    {
                        case 'c':
                        {
                            param = (char *)NULL;

                            param = va_arg( varArg, char* );
                            ANY_REQUIRE( param );

                            IOCHANNEL_READSPACES( self, buffer );
                            if( *buffer != '\'' )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check your quoted char!", ANY_LOG_INFO );
                                break;
                            }

                            if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            if( *buffer == '\\' )
                            {
                                if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }

                                if( *buffer == 'x' )
                                {
                                    uIntTmp = 0;

                                    if( IOChannel_readInternal( self, uIntBuff, 2 ) != 2 )
                                    {
                                        ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                    ( IOChannel_isErrorSet( self ) ));
                                        break;
                                    }

                                    uIntBuff[ 3 ] = '\0';
                                    Any_sscanf( uIntBuff, "%2x", &uIntTmp );
                                    *param = (char)uIntTmp;
                                }
                                else
                                {
                                    ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted char after the '\\",
                                             ANY_LOG_INFO );
                                    break;
                                }
                            }
                            else
                            {
                                *param = *buffer;
                            }

                            if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            if( *buffer != '\'' )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted char!", ANY_LOG_INFO );
                            }
                        }
                            break;

                        case 's':
                        {
                            char currQuote = '\0';
                            param = (char *)NULL;

                            param = va_arg( varArg, char* );
                            ANY_REQUIRE( param );

                            IOCHANNEL_READSPACES( self, buffer );
                            if( *buffer != '"' && *buffer != '\'' )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!", ANY_LOG_INFO );
                                break;
                            }

                            currQuote = *buffer;

                            if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            while(( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                //decode escaped \\ or \"
                                if( *buffer == '\\' )
                                {
                                    if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                    {
                                        ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                    ( IOChannel_isErrorSet( self ) ));
                                        break;
                                    }
                                    if( *buffer != '\\' && *buffer != currQuote )
                                    {
                                        ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!", ANY_LOG_INFO );
                                    }
                                }
                                else if( *buffer == currQuote )
                                {
                                    break;
                                }
                                *param = *buffer;
                                if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                param++;
                            }
                            *param = '\0';

                            if( *buffer != currQuote )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!", ANY_LOG_INFO );
                            }
                            break;
                        }
                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                case '*':   /* %*qs */
                {
                    switch( *( ++format ))
                    {
                        case 'q':
                        {
                            char currQuote = '\0';
                            param = (char *)NULL;
                            len = 0;

                            if( *( ++format ) != 's' )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. You wrote [%%*q%c] instead of[%%*qs] ",
                                         ANY_LOG_ERROR, *format );
                                break;
                            }

                            IOCHANNEL_READSPACES( self, buffer );
                            if(( *buffer != '"' && *buffer != '\'' ))
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!",
                                         ANY_LOG_INFO );
                                break;
                            }

                            currQuote = *buffer;

                            len = va_arg( varArg, long );
                            ANY_REQUIRE_MSG( len > 0, "IOChannel_scanFormatting. "
                                    "You used %*s but maybe you: "
                                    "1)forget to put the size parameter "
                                    "before the string pointer "
                                    "2)passed a <= size value!" );

                            param = va_arg( varArg, char* );
                            ANY_REQUIRE( param );

                            if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                            {
                                ANY_REQUIRE(
                                        ( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                                break;
                            }

                            i = 0;
                            while(( i < len ) &&
                                  ( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                //decode escaped \\ or \"
                                if( *buffer == '\\' )
                                {
                                    if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                    {
                                        ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                    ( IOChannel_isErrorSet( self ) ));
                                        break;
                                    }
                                    if( *buffer != '\\' && *buffer != currQuote )
                                    {
                                        ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!", ANY_LOG_INFO );
                                    }
                                    i++;
                                    if( i >= len )
                                    {
                                        break;
                                    }
                                }
                                else if( *buffer == currQuote )
                                {
                                    break;
                                }

                                *param = *buffer;
                                if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                i++;
                                param++;
                            }
                            *param = '\0';

                            if( *buffer != currQuote )
                            {
                                ANY_LOG( 5, "IOChannel_scanFormatting. Check quoted string!", ANY_LOG_INFO );
                            }
                        }
                            break;

                        case 's': /* Same as %s, but a max char limit is imposed. */
                        {
                            param = (char *)NULL;
                            len = 0;

                            ++format;

                            len = va_arg( varArg, long );
                            ANY_REQUIRE_MSG( len > 0, "IOChannel_scanFormatting. "
                                    "You used %*s but maybe you: "
                                    "1)forget to put the size parameter before the string pointer "
                                    "2)passed a <= size value!" );
                            param = va_arg( varArg, char* );
                            ANY_REQUIRE( param );

                            IOCHANNEL_READSPACES( self, buffer );
                            while(( i < len ) &&
                                  !IOCHANNEL_ISSPACE( *buffer ) &&
                                  ( IOChannel_eof( self ) == false ) &&
                                  ( IOChannel_isErrorSet( self ) == false ))
                            {
                                *param = *buffer;
                                if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                                {
                                    ANY_REQUIRE(( IOChannel_eof( self ) ) ||
                                                ( IOChannel_isErrorSet( self ) ));
                                    break;
                                }
                                i++;
                                param++;
                            }
                            *param = '\0';

                            if( !IOCHANNEL_ISSPACE( *buffer ) && ( IOChannel_eof( self ) == false ))
                            {
                                ANY_LOG( 5, "String terminator not found", ANY_LOG_WARNING );
                            }

                            if( *format == '\0' )
                            {
                                nUnget = 0;

                                nUnget = IOChannel_unget( self, buffer, 1 );
                                if( nUnget != 1 )\

                                {
                                    ANY_LOG( 0, "There's no Space in the unget buffer for scanf last char."
                                            "Unget Retval[%ld], Lost char is[%c]",
                                             ANY_LOG_ERROR, nUnget, *buffer );
                                    IOChannel_setError( self, IOCHANNELERROR_TOOUNGET );
                                }
                            }
                        }
                            break;

                        default:
                            IOChannel_setError( self, IOCHANNELERROR_INCR );
                            break;
                    }
                }
                    break;

                default:
                    IOChannel_setError( self, IOCHANNELERROR_INCR );
                    break;
            }
            /* A format specifier was expanded, so increment */
            /* the number of printed items                   */
            nItems++;
            if( !*format )
            {
                break;
            }
        }
        else
        {
            if( *format != ' ' )
            {
                if( *format == '\n' || *format == '\t' || *format == '\r' )
                {
                    *buffer = ' ';
                    while( *buffer == ' ' )
                    {
                        if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
                        {
                            ANY_REQUIRE(( IOChannel_eof( self ) ) || ( IOChannel_isErrorSet( self ) ));
                            break;
                        }
                    }
                }
                else
                {
                    IOCHANNEL_READSPACES( self, buffer );
                }

                if( IOChannel_eof( self ) == false && IOChannel_isErrorSet( self ) == false )
                {
                    if(( *format != *buffer ))
                    {
                        ANY_LOG( 5, "Pattern matching error.Format is[%c], but was read[%c]",
                                 ANY_LOG_INFO, *format, *buffer );

                        IOChannel_unget( self, buffer, 1 );
                        break;
                    }
                }
                else
                {
                    if( IOChannel_eof( self ) )
                    {
                        ANY_LOG( 1, "EOF found while pattern matching!!!", ANY_LOG_WARNING );
                    }
                    if( IOChannel_isErrorSet( self ) )
                    {
                        ANY_LOG( 1, "Error while reading a pattern match!!!", ANY_LOG_WARNING );
                    }
                    break;
                }
            }
        }
    }

    /* Get final Offset */
    endOffset = self->rdBytesFromLastWrite;

    if( nBytes != NULL)
    {
        *nBytes = ( endOffset - iniOffset );
    }

    retVal = (( IOChannel_isErrorSet( self ) ) ? -1 : nItems );

    return retVal;
}


static bool IOChannel_isCallAllowedCheck( IOChannel *self )
{
    bool retVal = true;

    if( !(( self->isOpen ) && ( self->errorType == IOCHANNELERROR_NONE )))
    {
        retVal = false;
        ANY_LOG( 5, "Call is not allowed: stream is closed or error is set", ANY_LOG_WARNING );
    }

    return retVal;
}


static bool IOChannel_isErrorSet( IOChannel *self )
{
    return self->errorType != IOCHANNELERROR_NONE;
}


static bool IOChannel_isOpenCheck( IOChannel *self )
{
    return self->isOpen;
}


static bool IOChannel_isNotWrOnlyCheck( IOChannel *self )
{
    bool retVal = true;

    /* Check if a reading function is called */
    /* when stream was set in wr only mode   */
    if( IOCHANNEL_MODEIS_W_ONLY( self->mode ) )
    {
        IOChannel_setError( self, IOCHANNELERROR_ACCV );
        ANY_LOG( 5, "Calling reading function, but stream was opened in W_ONLY mode", ANY_LOG_WARNING );
        /* Check Failed */
        retVal = false;
    }

    return retVal;
}


static bool IOChannel_isNotRdOnlyCheck( IOChannel *self )
{
    bool retVal = true;

    /* Check if a writing function is called */
    /* when stream was set in rd oly mode    */
    if( IOCHANNEL_MODEIS_R_ONLY( self->mode ) )
    {
        IOChannel_setError( self, IOCHANNELERROR_ACCV );
        ANY_LOG( 5, "Calling writing function, but stream was opened in R_ONLY mode", ANY_LOG_WARNING );
        /* Check Failed */
        retVal = false;
    }

    return retVal;
}


static long IOChannel_popFromUngetBuffer( IOChannel *self,
                                          void *buffer,
                                          long size )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    char *stackTop = (char *)NULL;
    char *ptr = (char *)NULL;
    long retVal = 0;
    long i = 0;

    ungetBuffer = self->ungetBuffer;
    /* This function is called only in readInternal. All the callers of
     * readInternal already perform this check.
     */
    /* ANY_REQUIRE( ungetBuffer ); */

    stackTop = (char *)ungetBuffer->ptr;
    ANY_REQUIRE( stackTop );

    stackTop += ungetBuffer->index;
    ANY_REQUIRE( stackTop );

    ptr = (char *)buffer;
    /* This function is called only in readInternal. All the callers of
     * readInternal already perform this check.
     */
    /* ANY_REQUIRE( ptr ); */

    while(( i < size ) && ( i <= ungetBuffer->index ))
    {
        *ptr++ = *--stackTop;
        i++;
    }

    ungetBuffer->index -= i;
    ANY_REQUIRE( ungetBuffer->index >= 0 );
    retVal = i;

    return retVal;
}


static long IOChannel_pushIntoUngetBuffer( IOChannel *self,
                                           void *buffer,
                                           long size )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long bytesToPushIndex = 0;
    char *p = (char *)NULL;
    char *q = (char *)NULL;
    long retVal = -1;
    long i = size;

    ANY_REQUIRE( size > 0 );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    p = (char *)ungetBuffer->ptr;
    ANY_REQUIRE( p );

    q = (char *)buffer;
    ANY_REQUIRE( q );

    bytesToPushIndex = ungetBuffer->index + size;

    if( bytesToPushIndex <= ungetBuffer->size )
    {
        p += bytesToPushIndex;
        while( i-- )
        {
            *( --p ) = *( q++ );
        }

        retVal = size;
        ungetBuffer->index += size;
    }
    else
    {
        ANY_LOG( 5, "Too unget were done, the internal buffer is not big enough to contain any other data!",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_TOOUNGET );
    }

    return retVal;
}


static void IOChannel_resetValuesForNewOpen( IOChannel *self )
{

    self->currInterface = (IOChannelInterface *)NULL;
    self->streamPtr = (void *)NULL;
    self->isOpen = false;
    self->usesWriteBuffering = false;
    self->writeBufferIsExternal = false;
    self->autoResize = false;
    self->mode = 0;
    self->readTimeout = IOCHANNEL_SELECT_TIMEOUT_USEC;
    self->writeTimeout = IOCHANNEL_SELECT_TIMEOUT_USEC;
    self->rdDeployedBytes = 0;
    self->wrDeployedBytes = 0;
    self->errorType = IOCHANNELERROR_NONE;
    self->currentIndexPosition = 0;
    self->foundEof = false;
    self->type = IOCHANNELTYPE_NOTSET;
    self->rdBytesFromLastUnget = 0;
    self->rdBytesFromLastWrite = 0;
}


static void IOChannel_resetObject( IOChannel *self )
{
    IOChannel_resetValuesForNewOpen( self );

    self->userStream = (MTList *)NULL;
}


static IOChannelBuffer *IOChannelBuffer_new( void )
{
    IOChannelBuffer *self = ANY_TALLOC( IOChannelBuffer );

    ANY_REQUIRE( self );

    return self;
}


static bool IOChannelBuffer_init( IOChannelBuffer *self, long defaultSize )
{
    bool retVal = true;

    ANY_REQUIRE( defaultSize > 0 );

    /* Creating default buffer */
    self->defaultBuffer = ANY_BALLOC( defaultSize );
    ANY_REQUIRE( self->defaultBuffer );

    self->defaultSize = defaultSize;
    self->ptr = NULL;
    self->size = 0;
    self->index = 0;
    self->freeOnExit = false;

    return retVal;
}


static void IOChannelBuffer_atOpen( IOChannelBuffer *self )
{
    ANY_REQUIRE( self );

    self->ptr = self->defaultBuffer;
    self->size = self->defaultSize;
    self->index = 0;
    self->freeOnExit = false;
}


static void IOChannelBuffer_set( IOChannelBuffer *self, void *ptr, long size )
{
    void *aux = (void *)NULL;

    if( ptr == NULL)
    {
        aux = ANY_BALLOC( size );
        ANY_REQUIRE( aux );
        self->ptr = aux;
        self->freeOnExit = true;
    }
    else
    {
        self->ptr = ptr;
        self->freeOnExit = false;
    }

    self->index = 0;
    self->size = size;
}


static void IOChannelBuffer_atClose( IOChannelBuffer *self )
{
    ANY_REQUIRE( self );

    if( self->freeOnExit )
    {
        ANY_REQUIRE( self->ptr != NULL );
        ANY_FREE( self->ptr );
    }

    self->ptr = self->defaultBuffer;
    self->size = self->defaultSize;
    self->index = 0;
    self->freeOnExit = false;
}


static void IOChannelBuffer_clear( IOChannelBuffer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->defaultBuffer );

    /* Freeing default buffer */
    ANY_FREE( self->defaultBuffer );
    self->defaultSize = 0;

    self->ptr = (void *)NULL;
    self->size = 0;
    self->index = 0;
    self->freeOnExit = false;
}


static void IOChannelBuffer_delete( IOChannelBuffer *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


static IOChannelPlugin *IOChannelPlugin_new( void )
{
    IOChannelPlugin *self = ANY_TALLOC( IOChannelPlugin );

    return self;
}


/* function-pointer to data-pointer casts which are not compatible with "-pedantic" */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


static int IOChannelPlugin_init( IOChannelPlugin *self, char *streamType )
{
    int retVal = 0;
    char libraryName[IOCHANNEL_INFOSTRING_MAXLEN];
    char pluginName[IOCHANNEL_INFOSTRING_MAXLEN];

    self->currInterface = IOChannel_findStaticStream( streamType );

    if( self->currInterface )
    {
        self->libHandle = NULL;
        goto out;
    }

    /* Compose the plugin name */
    Any_snprintf( pluginName, IOCHANNEL_INFOSTRING_MAXLEN, "IOChannel%sOps", streamType );

    /* Try to find the symbol in the public symbol space */
    self->currInterface = (IOChannelInterface *)DynamicLoader_getSymbolByName(NULL, pluginName );

    if( self->currInterface )
    {
        self->libHandle = NULL;
        goto out;
    }

    /* The symbol was not found, proceed to look for it within the library */

    self->libHandle = DynamicLoader_new();
    if( !self->libHandle )
    {
        ANY_LOG( 0, "Unable to allocate memory for a new DynamicLoader object", ANY_LOG_FATAL );
        retVal = -1;
        goto out;
    }

    Any_snprintf( libraryName, IOCHANNEL_INFOSTRING_MAXLEN, TOOLBOSLIBRARY );

    if( DynamicLoader_init( self->libHandle, libraryName ) != 0 )
    {
        ANY_LOG( 0, "Unable to initialize the DynamicLoader object", ANY_LOG_FATAL );
        ANY_LOG( 5, "%s", ANY_LOG_ERROR, DynamicLoader_getError( self->libHandle ));

        DynamicLoader_delete( self->libHandle );
        self->libHandle = NULL;

        retVal = -1;
        goto out;
    }

    self->currInterface = (IOChannelInterface *)DynamicLoader_getSymbolByName( self->libHandle, pluginName );

    if( self->currInterface )
    {
        /* Found the symbol */
        goto out;
    }
    else
    {
        DynamicLoader_clear( self->libHandle );
        DynamicLoader_delete( self->libHandle );
        self->libHandle = NULL;
    }

    /* Still didn't find it, try to find the related library */
    self->libHandle = DynamicLoader_new();
    if( !self->libHandle )
    {
        ANY_LOG( 0, "Unable to allocate memory for a new DynamicLoader object", ANY_LOG_FATAL );
        retVal = -1;
        goto out;
    }

    Any_snprintf( libraryName, IOCHANNEL_INFOSTRING_MAXLEN, TOOLBOSLIBRARY );

    Any_snprintf( pluginName, IOCHANNEL_INFOSTRING_MAXLEN, "IOChannel%sOps", streamType );

    if( DynamicLoader_init( self->libHandle, libraryName ) != 0 )
    {
        ANY_LOG( 0, "Unable to initialize the DynamicLoader object", ANY_LOG_FATAL );
        ANY_LOG( 5, "%s", ANY_LOG_ERROR, DynamicLoader_getError( self->libHandle ));

        DynamicLoader_delete( self->libHandle );
        self->libHandle = NULL;

        retVal = -1;
        goto out;
    }

    self->currInterface = (IOChannelInterface *)DynamicLoader_getSymbolByName( self->libHandle, pluginName );

    if( !self->currInterface )
    {
        ANY_LOG( 1, "Unable to find the IOChannel plugin interface for the stream '%s'",
                 ANY_LOG_ERROR, pluginName );

        if( self->libHandle )
        {
            DynamicLoader_clear( self->libHandle );
            DynamicLoader_delete( self->libHandle );
            self->libHandle = NULL;
        }

        retVal = -1;
    }

    out:

    return retVal;
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


static IOChannelInterface *IOChannel_findStaticStream( const char *streamName )
{
    int i;
    IOChannelInterface *retVal = NULL;

    for( i = 0; IOChannel_internalStreams[ i ]; i++ )
    {
        if( Any_strcmp( IOChannel_internalStreams[ i ]->streamName, streamName ) == 0 )
        {
            retVal = IOChannel_internalStreams[ i ];
            break;
        }
    }

    return retVal;
}


static IOChannelInterface *IOChannelPlugin_getInterface( IOChannelPlugin *self )
{
    return ( self->currInterface );
}


static void IOChannelPlugin_clear( IOChannelPlugin *self )
{
    if( self->libHandle )
    {
        DynamicLoader_clear( self->libHandle );
        DynamicLoader_delete( self->libHandle );
        self->libHandle = NULL;
    }
}


static void IOChannelPlugin_delete( IOChannelPlugin *self )
{
    ANY_FREE( self );
}


static bool IOChannel_internalReadSelect( IOChannel *self, int fd, struct timeval *timeout )
{
#if defined(__windows__)
    DWORD dwMilliseconds = 0;
#endif
    bool retVal = false;
    int ret = 0;
    fd_set rfd;

    ANY_REQUIRE( timeout );

#if defined(__windows__)
    if ( IOChannel_isSocket( fd ) )
    {
#endif

    /* Reset the fd set */
    FD_ZERO( &rfd );
    FD_SET( fd, &rfd );

    ret = select( fd + 1, &rfd, NULL, NULL, timeout );

    if( ret == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    else
    {
        retVal = ret && FD_ISSET(fd, &rfd);
    }

#if defined(__windows__)
    }
  else /* !socket == generic handle */
  {
    /* convert from micro to milliseconds */
    dwMilliseconds = ( timeout->tv_sec * 1000 ) + ( timeout->tv_usec / 1000 );

    /* windows polling check on fd require such code */
    ANY_LOG( 5, "******* entering in WaitForSingleObjectEx( %ld msecs ) call *******", ANY_LOG_INFO, dwMilliseconds );

    if( WaitForSingleObjectEx( (HANDLE)_get_osfhandle( fd ), dwMilliseconds, TRUE ) == WAIT_OBJECT_0 )
    {
      retVal = true;
    }

    ANY_LOG( 5, "******* exiting from WaitForSingleObjectEx( %ld msecs ) call *******", ANY_LOG_INFO, dwMilliseconds );
  }
#endif

    return retVal;
}


static bool IOChannel_internalWriteSelect( IOChannel *self, int fd, struct timeval *timeout )
#if !defined(__windows__)
{
    bool retVal = false;
    int ret = 0;
    fd_set wfd;

    ANY_REQUIRE( fd >= 0 );
    ANY_REQUIRE( timeout );

    /* Reset the fd set */
    FD_ZERO( &wfd );
    FD_SET( fd, &wfd );

    ret = select( fd + 1, NULL, &wfd, NULL, timeout );

    if( ret == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    else
    {
        retVal = ret && FD_ISSET( fd, &wfd );
    }

    return retVal;
}


#else
{
  return( true );
}
#endif


#if defined(__windows__)
static int IOChannel_isSocket( int fd )
{
  int ret = 0;
  int optVal = 0;
  int optLen = sizeof( optVal );

  /* try to get a socket option */
  ret = getsockopt( fd, SOL_SOCKET, SO_TYPE, (char*)&optVal, &optLen );

  return( ret == 0 );
}
#endif


static int IOChannel_writeEscapedChar( IOChannel *self, char ch )
{
#define IOCHANNEL_WRITEESCAPEDCHAR_BUFFERSIZE 16
    char *value = NULL;
    char buffer[IOCHANNEL_WRITEESCAPEDCHAR_BUFFERSIZE];

    switch( ch )
    {
        case '\b':
            value = "\\b";
            break;
        case '\f':
            value = "\\f";
            break;
        case '\n':
            value = "\\n";
            break;
        case '\r':
            value = "\\r";
            break;
        case '\t':
            value = "\\t";
            break;
        case '\v':
            value = "\\v";
            break;
        case '\a':
            value = "\\a";
            break;
        case '\\':
            value = "\\\\";
            break;
        case '\'':
            value = "\\'";
            break;
        case '\"':
            value = "\\\"";
            break;
        case '\?':
            value = "\\?";
            break;

        default:
            if( isprint( ch ))
            {
                buffer[ 0 ] = ch;
                buffer[ 1 ] = '\0';
                value = buffer;
            }
            else
            {
                Any_snprintf( buffer, IOCHANNEL_WRITEESCAPEDCHAR_BUFFERSIZE, "0x%x", (int)ch );
                value = buffer;
            }
    }

    return ( IOChannel_writeInternal( self, value, Any_strlen( value )));
#undef IOCHANNEL_WRITEESCAPEDCHAR_BUFFERSIZE
}


static long IOChannel_readInternal( IOChannel *self, void *buffer, long size )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long retVal = -1;
    long rdFromUngetBuff = 0;
    long rdFromStream = 0;
    long bytesToRead = 0;
    char *ptr = (char *)NULL;

    ungetBuffer = self->ungetBuffer;
    /* The following check has been removed to avoid performing it multiple
     *  times inside loops. It is required that functions calling this one
     *  perform this check.
     */
    /* ANY_REQUIRE( ungetBuffer ); */

    if( size == 0 )
    {
        ANY_LOG( 5, "IOChannel_readInternal was called with size = 0...", ANY_LOG_WARNING );
        retVal = 0;
        goto outLabel;
    }

    /* If R_ONLY, do not need flushing.. */
    if( !IOCHANNEL_MODEIS_R_ONLY( self->mode ))
    {
        if( IOChannel_flush( self ) == -1 )
        {
            ANY_LOG( 5, "IOChannel_readInternal. Unable to flush write buffer before read data", ANY_LOG_ERROR );
            ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                             "Low Level Flush returned -1, but error was not set!" );
            goto outLabel;
        }
    }

    if( ungetBuffer->index > 0 )
    {
        rdFromUngetBuff = IOChannel_popFromUngetBuffer( self, buffer, size );
        ANY_REQUIRE( rdFromUngetBuff >= 0 );
    }
    bytesToRead = ( size - rdFromUngetBuff );
    ANY_REQUIRE( bytesToRead >= 0 );

    if( bytesToRead == 0 )
    {
        /* Number of bytes in which can seek back! */
        self->rdBytesFromLastUnget += rdFromUngetBuff;
    }
    else
    {
        ptr = (char *)buffer;
        ptr += rdFromUngetBuff;
        /* Calling Low Level Read */
        rdFromStream = IOCHANNELINTERFACE_READ( self, ptr, bytesToRead );
        /* write( STDOUT_FILENO, buffer, size ); */
        if( rdFromStream == -1 )
        {
            /* Checks interfaces bugs: if -1 is returned, error must be set */
            ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                             "Low Level Read returned -1, but error was not set!" );
            goto outLabel;
        }
        self->currentIndexPosition += rdFromStream;
    }
    retVal = rdFromUngetBuff + rdFromStream;

    self->rdDeployedBytes += rdFromStream;
    self->rdBytesFromLastWrite += retVal;

    outLabel:
    return retVal;
}


static void IOChannel_scanItemInternal( IOChannel *self, char *buffer, bool isFloat, char **format, char *tmpBuffer )
{
    int i = 0;
    long nUnget = 0;

    char separator = *(++(*format));

    IOCHANNEL_READSPACES( self, buffer );
    if( *buffer == '-' )
    {
        tmpBuffer[i] = *buffer;
        if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
        {
            goto exit;
        }
        i++;
    }
    while( (IOCHANNEL_ISFLOATALLOWED( isFloat, *buffer ) ||
            IOCHANNEL_ISDIGIT( *buffer ) ) &&
           (*buffer != separator )  &&
           (IOChannel_eof( self ) == false ) &&
           (IOChannel_isErrorSet( self ) == false ) )
    {
        tmpBuffer[i] = *buffer;
        if( IOChannel_readInternal( self, buffer, 1 ) != 1 )
        {
            break;
        }
        i++;
    }
    if( (**format == '\0') && ( IOChannel_eof( self ) == false ) )
    {
        nUnget = IOChannel_unget( self, buffer, 1 );
        if( nUnget != 1 )
        {
            ANY_LOG( 0, "There's no Space in the unget buffer for "
                     "scanf last char. Unget Retval[%ld], Lost char is[%c]",
                     ANY_LOG_ERROR, nUnget, *buffer );

            IOChannel_setError( self, IOCHANNELERROR_TOOUNGET );
        }
    }
    tmpBuffer[i] = '\0';

  exit:
    return;
}


static long IOChannel_writeInternal( IOChannel *self, const void *buffer, long size )
{
    /* before calling this function, the following checks must be performed:
       ANY_REQUIRE( self );
       ANY_REQUIRE( self->valid == IOCHANNEL_VALID );
       ANY_REQUIRE_MSG( size >= 0, "Size must be a positive number" );
       ANY_REQUIRE( self->writeBuffer );
       ANY_REQUIRE( self->ungetBuffer );
       IOCHANNEL_REQUIRE_INTERFACE( self, indirectWrite );
    */

    IOChannelBuffer *writeBuffer = (IOChannelBuffer *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long retVal = -1;
    long bytesBack = 0;

    /* size = 0 is allowed but doesn't write anything */
    if( size == 0 )
    {
        ANY_LOG( 5, "IOChannel_write was called with size = 0...", ANY_LOG_WARNING );
        retVal = 0;
        goto outLabel;
    }

    /* NULL buffer is allowed but it isn't usefull */
    if( !buffer )
    {
        ANY_LOG( 5, "IOChannel_write was called with buffer = NULL...", ANY_LOG_WARNING );
        retVal = 0;
        goto outLabel;
    }

    writeBuffer = self->writeBuffer;
    /* The following check has been removed to avoid performing it multiple
     *  times inside loops. It is required that functions calling this one
     *  perform this check.
     */
    /* ANY_REQUIRE( writeBuffer ); */

    ungetBuffer = self->ungetBuffer;
    /* The following check has been removed to avoid performing it multiple
     *  times inside loops. It is required that functions calling this one
     *  perform this check.
     */
    /* ANY_REQUIRE( ungetBuffer ); */

    if(( writeBuffer->index > 0 ) && ( ungetBuffer->index > 0 ))
    {
        bytesBack = (long)( -ungetBuffer->index );
        /* Before write, must repositionate index back to the */
        /* ungetted bytes. Ungetted Data is Discarded... */
        if( IOChannel_seek( self, bytesBack, IOCHANNELWHENCE_CUR ) == -1 )
        {
            ANY_LOG( 0, "IOChannel_write. Unable to seek ungetted bytes before write data", ANY_LOG_ERROR );
            ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                             "Low Level Seek returned -1, but error was not set!" );
            goto outLabel;
        }
        ungetBuffer->index = 0;
    }

    /* write( STDOUT_FILENO, buffer, size ); */
    retVal = IOCHANNELINTERFACE_WRITE( self, buffer, size );
    if( retVal != -1 )
    {
        self->rdBytesFromLastWrite = 0;
        self->wrDeployedBytes += retVal;
        self->currentIndexPosition += retVal;
    }
    else
    {
        /* Checks interfaces bugs: if -1 is returned, error must be set */
        ANY_REQUIRE_MSG( IOChannel_isErrorSet( self ),
                         "Low Level Write returned -1, but error was not set!" );
    }
    outLabel:
    return retVal;
}
