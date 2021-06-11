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
#include <signal.h>
#include <string.h>
#include <pthread.h>


#if defined(__windows__)

#if defined(__windows__) && !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if defined(__windows__) && !defined(__mingw__)
#pragma warning( pop )
#endif

#include <winbase.h>
#include <dbghelp.h>
#include <string.h>

#else

#include <sys/time.h>
#include <sys/resource.h>
#include <strings.h>
#include <unistd.h>

#endif


#include <Any.h>
#include <Traps.h>


#if defined(__GLIBC__)

/*
 * This fix a compilation error on ARM crosscompiler
 */
#define PACKAGE 1
#define PACKAGE_VERSION 1

#include <dlfcn.h>
#include <execinfo.h>
#include <ucontext.h>
#include <bfd.h>
#include <link.h>
#include <sys/types.h>
#include <sys/wait.h>

#undef PACKAGE
#undef PACKAGE_VERSION

#if !defined(__64BIT__)
#define TRAPS_EIP   REG_EIP
#else
#define TRAPS_EIP   REG_RIP
#endif

#endif


#if !defined(__windows__)

static void Traps_sigFault( int sig, siginfo_t *si, void *ctx );

static void Traps_internalCallTrace( TrapsException *exception );

#if defined( __GLIBC__ )

#define DMGL_NO_OPTS     0              /* For readability... */
#define DMGL_PARAMS      (1 << 0)       /* Include function args */
#define DMGL_ANSI        (1 << 1)       /* Include const, volatile, etc */
#define DMGL_JAVA        (1 << 2)       /* Demangle as Java rather than C++. */
#define DMGL_VERBOSE     (1 << 3)       /* Include implementation details.  */
#define DMGL_TYPES       (1 << 4)       /* Also try to demangle type encodings.  */
#define DMGL_RET_POSTFIX (1 << 5)       /* Print function return types (when
                                           present) after function signature */
typedef struct BackTrace
{
    bfd *abfd;
    asymbol **symTable;
    long numSyms;
    asection *section;
    bool found;
    bfd_vma pc;
    const char *file;
    const char *func;
    unsigned int line;
}
        BackTrace;

typedef struct BackTraceMatch
{
    void *address;
    const char *file;
    void *base;
}
        BackTraceMatch;

bool BackTrace_open( BackTrace *self, const char *fileName );

void BackTrace_close( BackTrace *self );

bool BackTrace_findSymbolInfo( BackTrace *self, bfd_vma address,
                               char *symbolName, char *sourceFile, unsigned int *lineNumber );

static void BackTrace_findAddressInSection( bfd *abfd, asection *section, void *data );

static int BackTrace_findMatchingFile( struct dl_phdr_info *info, size_t size, void *data );

unsigned long BackTrace_findSymbolAddress( BackTrace *self, const char *symbol );

#if defined( HAVE_CPLUS_DEMANGLE )
extern char *cplus_demangle( const char *symbol, int flags );
#endif

#endif

#else

static LONG WINAPI Traps_sigFault( EXCEPTION_POINTERS *sig );
static LONG Traps_internalCallTrace( TrapsException *exception );

#endif  /* !__windows__ */

#define TRAPS_MAX_CALL_TRACE_INFO     256
#define TRAPS_MAX_COREDUMP_SIZE       (2*1024*1024)

/*
 * used to serialize the Traps_callTrace() function. We have it here
 * as static initialization because we need to initialize the struct
 */
//#if !defined(__windows__)
//static pthread_mutex_t Traps_callTraceLock = PTHREAD_MUTEX_INITIALIZER;
//#endif

/* used to trap a signal */
static void ( *Traps_userFaultFunc )( void * );

static void *Traps_userFaultFuncParam;

/* extended version to trap a signal */
static void ( *Traps_userExtendedFaultFunc )( void *, TrapsException * );

/* parm extentend version */
static void *Traps_userExtendedFaultFuncParam;

/*
 * declare the list of signals description
 */
static struct
{
    int sig;
    /**< Signal */
    char *str;     /**< Description */
}
        Traps_sig2str[] = {
        { 0, (char *)"Invalid" },
#if defined (SIGHUP)
        { SIGHUP, (char *)"SIGHUP" },
#endif
#if defined (SIGINT)
        { SIGINT, (char *)"SIGINT" },
#endif
#if defined (SIGQUIT)
        { SIGQUIT, (char *)"SIGQUIT" },
#endif
#if defined (SIGILL)
        { SIGILL, (char *)"SIGILL" },
#endif
#if defined (SIGTRAP)
        { SIGTRAP, (char *)"SIGTRAP" },
#endif
#if defined (SIGABRT)
        { SIGABRT, (char *)"SIGABRT" },
#endif
#if defined (SIGIOT)
        { SIGIOT, (char *)"SIGIOT" },
#endif
#if defined (SIGBUS)
        { SIGBUS, (char *)"SIGBUS" },
#endif
#if defined (SIGFPE)
        { SIGFPE, (char *)"SIGFPE" },
#endif
#if defined (SIGKILL)
        { SIGKILL, (char *)"SIGKILL" },
#endif
#if defined (SIGUSR1)
        { SIGUSR1, (char *)"SIGUSR1" },
#endif
#if defined (SIGSEGV)
        { SIGSEGV, (char *)"SIGSEGV" },
#endif
#if defined (SIGUSR2)
        { SIGUSR2, (char *)"SIGUSR2" },
#endif
#if defined (SIGPIPE)
        { SIGPIPE, (char *)"SIGPIPE" },
#endif
#if defined (SIGALRM)
        { SIGALRM, (char *)"SIGALRM" },
#endif
#if defined (SIGTERM)
        { SIGTERM, (char *)"SIGTERM" },
#endif
#if defined (SIGSTKFLT)
        { SIGSTKFLT, (char *)"SIGSTKFLT" },
#endif
#if defined (SIGCHLD)
        { SIGCHLD, (char *)"SIGCHLD" },
#endif
#if defined (SIGCONT)
        { SIGCONT, (char *)"SIGCONT" },
#endif
#if defined (SIGSTOP)
        { SIGSTOP, (char *)"SIGSTOP" },
#endif
#if defined (SIGTSTP)
        { SIGTSTP, (char *)"SIGTSTP" },
#endif
#if defined (SIGTTIN)
        { SIGTTIN, (char *)"SIGTTIN" },
#endif
#if defined (SIGTTOU)
        { SIGTTOU, (char *)"SIGTTOU" },
#endif
#if defined (SIGURG)
        { SIGURG, (char *)"SIGURG" },
#endif
#if defined (SIGXCPU)
        { SIGXCPU, (char *)"SIGXCPU" },
#endif
#if defined (SIGXFSZ)
        { SIGXFSZ, (char *)"SIGXFSZ" },
#endif
#if defined (SIGVTALRM)
        { SIGVTALRM, (char *)"SIGVTALRM" },
#endif
#if defined (SIGPROF)
        { SIGPROF, (char *)"SIGPROF" },
#endif
#if defined (SIGWINCH)
        { SIGWINCH, (char *)"SIGWINCH" },
#endif
#if defined (SIGIO)
        { SIGIO, (char *)"SIGIO" },
#endif
#if defined (SIGPOLL)
        { SIGPOLL, (char *)"SIGPOLL" },
#endif
#if defined (SIGPWR)
        { SIGPWR, (char *)"SIGPWR" },
#endif
#if defined (SIGSYS)
        { SIGSYS, (char *)"SIGSYS" },
#endif
        { 0, (char *)"Not Found" }
};


/*
 * public functions
 */


void Traps_coredumpSetup( void )
{
#if defined(__msvc__) || defined(__windows__)
    ANY_LOG( 5, "Windows doesn't support coredumps", ANY_LOG_INFO );
#else
# ifdef RLIMIT_CORE
    struct rlimit rlp;

    /*
     * get the current corefile limits.
     */
    getrlimit( RLIMIT_CORE, &rlp );

    /*
     * Look for prevent to generate over 2G core dump file.
     * Linux currently don't handle core dump files large
     * than 2G - 1 bytes. If the Filesystem support large
     * files change this value accordly.
     */
    rlp.rlim_cur = TRAPS_MAX_COREDUMP_SIZE - 1;

    /*
     * setup the corefile limits to handle the coredump correctly
     */
    setrlimit( RLIMIT_CORE, &rlp );

    /* reload the limits */
    getrlimit( RLIMIT_CORE, &rlp );

    /* infos on logfile */
    ANY_LOG( 5, "Core limits now %d %d", ANY_LOG_INFO, (int)rlp.rlim_cur, (int)rlp.rlim_max );
# else
#  error Check the include file. No core limits available for coredumps!!!
# endif /* RLIMIT_CORE */
#endif
}


/*
 * setup our fault handlers
 */
void Traps_faultSetup( void (*faultHandler)( void * ), void *faultHandlerParam )
{
    Traps_userFaultFunc = faultHandler;
    Traps_userFaultFuncParam = faultHandlerParam;
}


/*
 * setup our fault handlers
 */
void Traps_faultSetupExtended( void (*faultExtendedHandler)( void *, TrapsException * ),
                               void *faultExtendedHandlerParam )
{
    Traps_userExtendedFaultFunc = faultExtendedHandler;
    Traps_userExtendedFaultFuncParam = faultExtendedHandlerParam;
}


void Traps_trapSynchronousSignal( void )
{
#if defined(__windows__)
    SetUnhandledExceptionFilter( Traps_sigFault );
#else
#ifdef SIGSEGV
    Traps_catchSignal( SIGSEGV, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGFPE
    Traps_catchSignal( SIGFPE, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGBUS
    Traps_catchSignal( SIGBUS, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGILL
    Traps_catchSignal( SIGILL, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGQUIT
    Traps_catchSignal( SIGQUIT, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGPIPE
    Traps_catchSignal( SIGPIPE, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGABRT
    Traps_catchSignal( SIGABRT, (void ( * )( int ))Traps_sigFault );
#endif
#ifdef SIGKILL
    Traps_catchSignal( SIGKILL, (void ( * )( int ))Traps_sigFault );
#endif

#endif  /* __windows__ */
}


void Traps_untrapSynchronousSignal( void )
{
#if defined(__windows__)
    SetUnhandledExceptionFilter( Traps_sigFault );
#else
#ifdef SIGSEGV
    Traps_catchSignal( SIGSEGV, SIG_DFL);
#endif
#ifdef SIGFPE
    Traps_catchSignal( SIGFPE, SIG_DFL);
#endif
#ifdef SIGBUS
    Traps_catchSignal( SIGBUS, SIG_DFL);
#endif
#ifdef SIGILL
    Traps_catchSignal( SIGILL, SIG_DFL);
#endif
#ifdef SIGQUIT
    Traps_catchSignal( SIGQUIT, SIG_DFL);
#endif
#ifdef SIGPIPE
    Traps_catchSignal( SIGPIPE, SIG_DFL);
#endif
#ifdef SIGABRT
    Traps_catchSignal( SIGABRT, SIG_DFL);
#endif
#ifdef SIGTERM
    Traps_catchSignal( SIGTERM, SIG_DFL);
#endif
#ifdef SIGKILL
    Traps_catchSignal( SIGKILL, SIG_DFL);
#endif

#endif  /* __windows__ */
}


/*
 * Block sigs.
 */
void Traps_blockSignals( int block, int signum )
{
#if !defined(__msvc__) && !defined(__windows__)
    sigset_t set;

    sigemptyset( &set );
    sigaddset( &set, signum );
    sigprocmask(( block ? SIG_BLOCK : SIG_UNBLOCK ), &set, NULL);
#endif  /* !defined(__msvc__) && !defined(__windows__) */
}


/* assignment to known incompatible pointer type */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


/*
 * Catch a signal. This should implement the following semantics:
 *
 * 1) The handler remains installed after being called.
 * 2) The signal should be blocked during handler execution.
 */
void Traps_catchSignal( int signum, void ( *handler )( int ))
{
#if !defined(__msvc__) && !defined(__windows__)
    struct sigaction act;

    /* fill with zero */
    Any_memset( &act, 0, sizeof( act ));

    if( handler == SIG_DFL)
    {
        act.sa_handler = handler;
    }
    else
    {
        /* setup the proper handler */
        act.sa_sigaction = (void *)handler;
        act.sa_flags = SA_RESETHAND | SA_SIGINFO;    /* used to get additional information */
    }

#ifdef SA_RESTART
    /*
     * We *want* SIGALRM to interrupt a system call. This for compatibility
     * reasons. SunOS doesn't restart by default the interrupted syscall
     */
    if( signum != SIGALRM )
    {
        act.sa_flags |= SA_RESTART;
    }
#endif

    /* install the handler */
    sigemptyset( &act.sa_mask );
    sigaddset( &act.sa_mask, signum );
    sigaction( signum, &act, NULL);

#endif
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


void Traps_coreDump( void )
{
#if defined(__windows__)
    *(int *)NULL = 0;
#else
    __builtin_trap();
#endif
}


#if defined(__windows__)

#define MAXSYMBOLNAMELENGTH 64

/* Pointer function to SymInitialize() */
typedef BOOL ( WINAPI *pfnSymInitialize_t )( HANDLE, PSTR , BOOL );
/* Pointer function to SymCleanup() */
typedef BOOL ( WINAPI *pfnSymCleanup_t )( HANDLE hProcess );
/* Pointer function to SymFunctionTableAccess64() */
typedef PVOID ( WINAPI *pfnSymFunctionTableAccess64_t )( HANDLE, DWORD64 );
/* Pointer function to SymModuleBase64() */
typedef DWORD ( WINAPI *pfnSymGetModuleBase64_t )( HANDLE, DWORD64 );
/* Pointer function to StackWalk64() */
typedef BOOL ( WINAPI *pfnStackWalk64_t )( DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
                                           PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64,
                                           PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64 );
/* Pointer function to SymSetOptions() */
typedef DWORD ( WINAPI *pfnSymSetOptions_t )( DWORD );
/* Pointer function to SymGetOptions() */
typedef DWORD ( WINAPI *pfnSymGetOptions_t )( VOID );
/* Pointer function to SymUnDName() */
typedef BOOL ( WINAPI *pfnSymUnDName_t)( PIMAGEHLP_SYMBOL, PSTR, DWORD );
/* Pointer function to SymGetSymFromAddr() */
typedef BOOL ( WINAPI *pfnSymGetSymFromAddr_t )( HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL );
/* Pointer function to SymGetModuleInfo() */
typedef BOOL ( WINAPI *pfnSymGetModuleInfo_t )( HANDLE, DWORD, PIMAGEHLP_MODULE );
/* Pointer function to SymGetLineFromAddr) */
typedef BOOL ( WINAPI *pfnSymGetLineFromAddr_t )( HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE );


#define WIN32FINDFUNCTION( __name )\
do { \
  if ( !( pfn##__name = (pfn##__name##_t)GetProcAddress( dbgHandle, #__name ) ) )\
  {\
    ANY_LOG( 0, "Unable to get the address of the " #__name "() function", ANY_LOG_ERROR );\
    FreeLibrary( dbgHandle );\
    goto out;\
  }\
} while( 0 )

#endif /* __windows__ */


void Traps_callTrace( void )
{
    TrapsException fake;

#if !defined(__windows__)

    Any_memset( &fake, 0, sizeof( fake ));

    /* set the address to which the current function will return */
    fake.address = __builtin_return_address( 0 );

#else

    EXCEPTION_POINTERS exception;
    EXCEPTION_RECORD exceptionRecord;
    CONTEXT context;
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    context.ContextFlags = CONTEXT_FULL;

    if ( GetThreadContext( hThread, &context ) )
    {
     /*
      * GetThreadContext might return invalid context for the current thread
      * expecially in intel archs
      */
#if defined(__windows__) && defined(__32BIT__) && !defined(__mingw__)
      __asm mov context.Ebp, ebp
#endif

      ZeroMemory( &fake, sizeof( fake ) );
      ZeroMemory( &exception, sizeof( exception ) );
      ZeroMemory( &exceptionRecord, sizeof( exception ) );

      exception.ContextRecord = &context;
      exception.ExceptionRecord = &exceptionRecord;
      exceptionRecord.ExceptionCode = 0;

#if defined(__windows__) && defined(__32BIT__) && !defined(__mingw__)
      exceptionRecord.ExceptionAddress = (void*)context.Eip;
      fake.address = (void*)context.Eip;
# endif
#if defined(__windows__) && defined(__64BIT__) && !defined(__mingw__)
      exceptionRecord.ExceptionAddress = (void*)context.Rip;
      fake.address = (void*)context.Rip;
#endif

     fake.specific1 = (void*) &exception;
    }
    else
    {
      Any_fprintf( ANY_LOG_FILE, "Error on GetThreadContext(), aborting Traps_callTrace()\n" );
      return;
    }

#endif

    Traps_internalCallTrace( &fake );
}

/*
 * this is the Traps_callTrace() implementation for both systems
 */
#if !defined(__windows__)

/*
 * Obtain a backtrace and print it to stdout.
 */
static void Traps_internalCallTrace( TrapsException *exception )
{
#if defined(__GLIBC__)
    char sourceFileName[1024];
    char sourceFunctionName[1024];
    unsigned int lineNum = 0;
    void *array[TRAPS_MAX_CALL_TRACE_INFO];
    size_t size;
    char **strings;
    size_t i;
    int faultyLine = -1;
    BackTrace trace;
    BackTraceMatch match;
    pid_t child;
    char pid[20];
    static char *gdbCommand = NULL;
    char *const argv[] = { "gdbserver", gdbCommand, "--attach", pid, NULL };
    char path[4096];
    const char *filename = NULL;
    bfd_vma normalizedAddress;
    ssize_t length = 0;

    /* get the current executable name from the /proc */
    length = readlink( "/proc/self/exe", path, -1 + sizeof( path ));

    if( length > 0 )
    {
        path[ length ] = '\0';
    }

    bfd_init();

    size = backtrace( array, TRAPS_MAX_CALL_TRACE_INFO );
    strings = backtrace_symbols( array, size );

    /* try to find a backtrace generated by the ANY_REQUIRE() */
    for( i = 1; i < size; i++ )
    {
        if( Any_strstr( strings[ i ], "Any_fireRequire" ))
        {
            faultyLine = i + 1;
            break;
        }
    }

    Any_fprintf( ANY_LOG_FILE, "-----------------------------------------------------------------\n" );
    Any_fprintf( ANY_LOG_FILE, "PID: %d, Obtained %d calls nesting.\n", (int)getpid(), (int)size );
    Any_fprintf( ANY_LOG_FILE, "Faulty address is: %p\n", exception->address );
    Any_fprintf( ANY_LOG_FILE, "-----------------------------------------------------------------\n" );

    for( i = 1; i < size; i++ )
    {
        if( faultyLine == -1 )
        {
            if( exception->address == array[ i ] )
            {
                faultyLine = i;
            }
        }

        Any_fprintf( ANY_LOG_FILE, "%2s%2d) ", ( faultyLine == (int)i ? "->" : "  " ), (int)( size - i ));

        match.address = array[ i ];

        dl_iterate_phdr( BackTrace_findMatchingFile, &match );

        normalizedAddress = (bfd_vma)((bfd_vma)array[ i ] - (bfd_vma)match.base );

        if( match.file && Any_strlen( match.file ) > 0 )
        {
            filename = match.file;
        }
        else
        {
            filename = path;
        }

        if( BackTrace_open( &trace, filename ) == true )
        {
            if( BackTrace_findSymbolInfo( &trace, normalizedAddress, sourceFunctionName, sourceFileName, &lineNum ) ==
                true )
            {
                Any_fprintf( ANY_LOG_FILE, "%p in %s() at %s:%u\n", array[ i ], sourceFunctionName, sourceFileName,
                             lineNum );
            }
            else
            {
                Any_fprintf( ANY_LOG_FILE, "%p at %s\n", array[ i ], strings[ i ] );
            }

            BackTrace_close( &trace );
        }
        else
        {
            Any_printf( "dladdr() return error at #%d @ %p\n", (int)i, array[ i ] );
            break;
        }
    }

    Any_fprintf( ANY_LOG_FILE, "-----------------------------------------------------------------\n" );

    free( strings );

    /*
     * if the environment variable is defined
     * than we call the GDBSERVER
     */
    gdbCommand = getenv( "TRAPS_GDB" );

    if( gdbCommand && *gdbCommand )
    {
        switch(( child = fork()))
        {
            case 0:
                Any_sprintf( pid, "%ld", (long)getppid());
                Any_fprintf( ANY_LOG_FILE, "Calling the gdbserver ...\n" );
                execvp( argv[ 0 ], argv );
                Any_fprintf( ANY_LOG_FILE, "Failed to start gdbserver\n" );
                _exit( -1 );

            case -1:
                Any_fprintf( ANY_LOG_FILE, "failed to fork\n" );
                break;

            default:
                waitpid( child, NULL, 0 );
                break;
        }
    }
#endif
}

#else  /* __windows__ */

/* this is the Traps_callTrace() implementation of the Win32 */
static long Traps_internalCallTrace( TrapsException *exception )
{
  EXCEPTION_POINTERS *e = NULL;
  HMODULE dbgHandle;
  STACKFRAME64 stackFrame;
  CONTEXT *context = NULL;
  IMAGEHLP_SYMBOL * pImagehlpSymbol;
  IMAGEHLP_MODULE module;
  IMAGEHLP_LINE line;
  ULONG displacement;
  BOOL fReturn;
  CHAR szUndecoratedName[MAXSYMBOLNAMELENGTH];
  HANDLE procHandle;
  DWORD dwMachineType;
  DWORD symOptions = 0;
  pfnSymInitialize_t pfnSymInitialize;
  pfnSymFunctionTableAccess64_t pfnSymFunctionTableAccess64;
  pfnSymCleanup_t pfnSymCleanup;
  pfnSymGetSymFromAddr_t pfnSymGetSymFromAddr;
  pfnSymGetModuleBase64_t pfnSymGetModuleBase64;
  pfnStackWalk64_t pfnStackWalk64;
  pfnSymSetOptions_t pfnSymSetOptions;
  pfnSymGetOptions_t pfnSymGetOptions;
  pfnSymUnDName_t pfnSymUnDName;
  pfnSymGetModuleInfo_t pfnSymGetModuleInfo;
  pfnSymGetLineFromAddr_t pfnSymGetLineFromAddr;
  int i = 0;

  dbgHandle = LoadLibrary( "dbghelp.dll" );

  if ( !dbgHandle )
  {
    Any_fprintf( ANY_LOG_FILE, "Unable to load the library dbghelp.dll\n" );
    goto out;
  }

  /* get some function from the library */
  WIN32FINDFUNCTION( SymInitialize );
  WIN32FINDFUNCTION( SymFunctionTableAccess64 );
  WIN32FINDFUNCTION( SymGetModuleBase64 );
  WIN32FINDFUNCTION( SymGetModuleInfo );
  WIN32FINDFUNCTION( SymGetSymFromAddr );
  WIN32FINDFUNCTION( SymSetOptions );
  WIN32FINDFUNCTION( SymGetOptions );
  WIN32FINDFUNCTION( SymGetLineFromAddr );
  WIN32FINDFUNCTION( StackWalk64 );
  WIN32FINDFUNCTION( SymUnDName );
  WIN32FINDFUNCTION( SymCleanup );

  /* get symbol's options */
  symOptions = (*pfnSymGetOptions)();

  /* set some options */
  symOptions |= SYMOPT_LOAD_LINES;
  symOptions &= ~SYMOPT_UNDNAME;

  /* set the symbol's options */
  (*pfnSymSetOptions)( symOptions );

  procHandle = GetCurrentProcess();

  /* initialize the symbols */
  (*pfnSymInitialize)( procHandle, NULL, TRUE );

  ZeroMemory( &stackFrame, sizeof( stackFrame ) );
  e = (EXCEPTION_POINTERS *)exception->specific1;
  context = e->ContextRecord;

  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrStack.Mode = AddrModeFlat;

#if defined(__windows__) && defined(__32BIT__) && !defined(__mingw__)
  stackFrame.AddrPC.Offset = context->Eip;
  stackFrame.AddrFrame.Offset = context->Ebp;
  stackFrame.AddrStack.Offset = context->Esp;
#endif

#if defined(__windows__) && defined(__64BIT__) && !defined(__mingw__)
  stackFrame.AddrPC.Offset = context->Rip;
  stackFrame.AddrFrame.Offset = context->Rbp;
  stackFrame.AddrStack.Offset = context->Rsp;
#endif

#if defined(__32BIT__)
  dwMachineType = IMAGE_FILE_MACHINE_I386;
#else
  dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#endif

  pImagehlpSymbol = (IMAGEHLP_SYMBOL *)ANY_BALLOC( sizeof( IMAGEHLP_SYMBOL ) + MAXSYMBOLNAMELENGTH - 1 );
  ZeroMemory( pImagehlpSymbol, sizeof ( IMAGEHLP_SYMBOL ) + MAXSYMBOLNAMELENGTH - 1 );
  pImagehlpSymbol->SizeOfStruct = sizeof( IMAGEHLP_SYMBOL );
  pImagehlpSymbol->MaxNameLength = MAXSYMBOLNAMELENGTH;

  Any_fprintf( ANY_LOG_FILE, "===============================================================\n" );
  Any_fprintf( ANY_LOG_FILE, "INTERNAL ERROR: Unhandled exception 0x%08lx at address 0x%p\n",
           e->ExceptionRecord->ExceptionCode, e->ExceptionRecord->ExceptionAddress );
  Any_fprintf( ANY_LOG_FILE, "===============================================================\n" );

  while ( true )
  {
    fReturn = (*pfnStackWalk64)( dwMachineType,                                     // DWORD
                                 procHandle,                                        // HANDLE
                                 GetCurrentThread(),                                // HANDLE
                                 &stackFrame,                                       // LPSTACKFRAME64
                                 context,                                           // PVOID
                                 NULL,                                              // PREAD_PROCESS_MEMORY_ROUTINE64
                                 pfnSymFunctionTableAccess64,                       // PFUNCTION_TABLE_ACCESS_ROUTINE64
                                 (PGET_MODULE_BASE_ROUTINE64)pfnSymGetModuleBase64, // PGET_MODULE_BASE_ROUTINE64
                                 NULL                                               // PTRANSLATE_ADDRESS_ROUTINE64
                                 );

    if ( !fReturn || stackFrame.AddrFrame.Offset == 0 )
    {
      Any_fprintf( ANY_LOG_FILE, "Quitting.... %d\n", i );
      break;
    }

    pImagehlpSymbol->Address = stackFrame.AddrPC.Offset;
    fReturn = (*pfnSymGetSymFromAddr)( procHandle, stackFrame.AddrPC.Offset, &displacement, pImagehlpSymbol );

    if ( !fReturn )
    {
      Any_fprintf( ANY_LOG_FILE, "Symbol at 0x%p not found ....\n", (void*)stackFrame.AddrPC.Offset );
      continue;
    }

    Any_fprintf( ANY_LOG_FILE, "%d) 0x%08I64x 0x%08I64x ", (int)(++i), stackFrame.AddrReturn.Offset,
                           stackFrame.AddrFrame.Offset );

    /* get the module info */
    ZeroMemory( &module, sizeof( module ) );

    if ( (*pfnSymGetModuleInfo)( procHandle, stackFrame.AddrPC.Offset, &module ) )
    {
      Any_fprintf( ANY_LOG_FILE, "%s!", module.ModuleName );
    }

    if ( fReturn )
    {
      fReturn = (*pfnSymUnDName)( pImagehlpSymbol,      // IN PIMAGEHLP_SYMBOL Symbol,
                                  szUndecoratedName,    // IN LPSTR UnDecName,
                                  MAXSYMBOLNAMELENGTH   // IN DWORD UnDecNameLength
                                );

      if ( fReturn )
      {
        Any_fprintf( ANY_LOG_FILE, "%s()+0x%lx", szUndecoratedName, displacement );
      }
      else
      {
        Any_fprintf( ANY_LOG_FILE, "*SymUnDName failed: %lu*", GetLastError() );
      }
    }
    else
    {
      Any_fprintf( ANY_LOG_FILE, "0x%08I64x", stackFrame.AddrPC.Offset );
    }

    if ( (*pfnSymGetLineFromAddr)( procHandle, stackFrame.AddrPC.Offset, &displacement, &line ) )
    {
      char symType[80];

      switch ( module.SymType )
      {
      case SymNone:
           Any_strcpy( symType, "-nosymbols-" );
           break;

      case SymCoff:
           Any_strcpy( symType, "COFF" );
           break;

      case SymCv:
           Any_strcpy( symType, "CV" );
           break;

      case SymPdb:
      case 7: /* last PDB on VC++2008 */
           Any_strcpy( symType, "PDB" );
           break;

      case SymExport:
           Any_strcpy( symType, "-exported-" );
           break;

      case SymDeferred:
           Any_strcpy( symType, "-deferred-" );
           break;

      case SymSym:
           Any_strcpy( symType, "SYM" );
           break;

      default:
           Any_snprintf( symType, sizeof( symType ), "symtype=%ld", (long)module.SymType );
           break;
      }

      Any_fprintf( ANY_LOG_FILE, "\n\tFile: %s:%lu", line.FileName, line.LineNumber );
      Any_fprintf( ANY_LOG_FILE, "\n\tModule: %s[%s] @ 0x%08I64x", module.ModuleName, module.ImageName, module.BaseOfImage );
      Any_fprintf( ANY_LOG_FILE, "\n\tSymbols: type %s, file %s", symType, module.LoadedImageName );
    }

    Any_fprintf( ANY_LOG_FILE, "\n" );

  } /* while() */

  Any_fprintf( ANY_LOG_FILE, "-----------------------------------------------------------------\n" );

  ANY_FREE( pImagehlpSymbol );

  (*pfnSymCleanup)( procHandle );

  FreeLibrary( dbgHandle );

 out:

  return EXCEPTION_EXECUTE_HANDLER;
}

#endif  /* __windows__ */


/* static functions */

/* convert the signal number in string */
static const char *Traps_sigStr( int sig )
{
    int i;

    /* search the signal */
    for( i = 0; Traps_sig2str[ i ].sig != sig; i++ )
    {
        /* check for the end of sig2str vector */
        if( i && Traps_sig2str[ i ].sig == 0 )
        {
            break;
        }

        /* nothing */
    }

    /* return the string */
    return ( Traps_sig2str[ i ].str );
}


static void Traps_faultReport( TrapsException *trap )
{
#if !defined(__windows__)
    ANY_LOG ( 0, "===============================================================", ANY_LOG_FATAL );
    ANY_LOG ( 0, "INTERNAL ERROR: Signal %s (%d) in pid %d", ANY_LOG_FATAL,
              Traps_sigStr( trap->exception ), trap->exception, (int)ANY_LOG_GETPID );
    ANY_LOG ( 0, "===============================================================", ANY_LOG_FATAL );
#endif

    /* show the call trace */
    Traps_internalCallTrace( trap );
}

/*
 * This is the real function that traps the signals in both Unix like
 * and Win32 Operating System. It converts the trapping information
 * will be unified in an omogenious way
 */
#if !defined(__windows__)

#define TRAPS_SIGFAULT_RETURN return


static void Traps_sigFault( int sig, siginfo_t *si, void *ctx )

#else

#define TRAPS_SIGFAULT_RETURN return( EXCEPTION_EXECUTE_HANDLER )

static LONG WINAPI Traps_sigFault( EXCEPTION_POINTERS *sig )

#endif  /* !__windows__ */
{
    TrapsException trap;

#if !defined(__windows__) && !defined(__macos__) && !defined(__arm__)
    /*
     * this will convert the Unix trapping into our unified way
     */
    trap.exception = sig;
    trap.address = (void *)((ucontext_t *)ctx )->uc_mcontext.gregs[ TRAPS_EIP ];
    trap.specific1 = si;
    trap.specific2 = ctx;
#endif

#if defined(__arm__)
    /*
     * this will convert the Unix trapping into our unified way
     */
    trap.exception = sig;
    trap.address = (void*)((ucontext_t*)ctx)->uc_mcontext.arm_ip;
    trap.specific1 = si;
    trap.specific2 = ctx;
#endif

#if defined(__windows__)
    /*
     * this will convert the Win32 trapping into our unified way
     */
   trap.exception = sig->ExceptionRecord->ExceptionCode;

#if defined(__windows__) && defined(__32BIT__) && !defined(__mingw__)
   trap.address = (void*)sig->ContextRecord->Eip;
#endif
#if defined(__windows__) && defined(__64BIT__) && !defined(__mingw__)
   trap.address = (void*)sig->ContextRecord->Rip;
#endif

   trap.specific1 = sig;
   trap.specific2 = NULL;
#endif

    Traps_faultReport( &trap );

    if( Traps_userFaultFunc )
    {
        ( *Traps_userFaultFunc )( Traps_userFaultFuncParam );

        /*
         * this should cause a core dump! Remember, the core dump could
         * be dumped only if we have previously setrlimit(RLIMIT_CORE, something)
         * or the application is running with 'ulimit -c greater than zero'.
         * Our implementation always setrlimit() greather than zero
         * (see Traps_faultSetup()), so we dump always the coredump for
         * our purpose ... namely, DEBUG ;-)!!!
         */
        TRAPS_SIGFAULT_RETURN;
    }

    if( Traps_userExtendedFaultFunc )
    {
        ( *Traps_userExtendedFaultFunc )( Traps_userExtendedFaultFuncParam, &trap );

        /*
         * this should cause a core dump! Remember, the core dump could
         * be dumped only if we have previously setrlimit(RLIMIT_CORE, something)
         * or the application is running with 'ulimit -c greater than zero'.
         * Our implementation always setrlimit() greather than zero
         * (see Traps_faultSetup()), so we dump always the coredump for
         * our purpose ... namely, DEBUG ;-)!!!-
         */
        TRAPS_SIGFAULT_RETURN;
    }

    exit( EXIT_FAILURE );
}


#if defined( __GLIBC__ )


/* some API parameters unused but kept for consistency */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


static int BackTrace_findMatchingFile( struct dl_phdr_info *info, size_t size, void *data )
{
    struct BackTraceMatch *match = (BackTraceMatch *)data;
    long n;
    const ElfW( Phdr ) *phdr;
    ElfW( Addr ) loadBase = info->dlpi_addr;

    phdr = info->dlpi_phdr;

    for( n = info->dlpi_phnum; --n >= 0; phdr++ )
    {
        if( phdr->p_type == PT_LOAD )
        {
            ElfW( Addr ) vaddr = phdr->p_vaddr + loadBase;

            if( match->address >= (void *)vaddr && match->address < (void *)( vaddr + phdr->p_memsz ))
            {
                /* we found a match */
                match->file = info->dlpi_name;
                match->base = (void *)info->dlpi_addr;
            }
        }
    }

    return 0;
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


bool BackTrace_open( BackTrace *self, const char *fileName )
{
    bool retVal = false;
    unsigned int storage;

    self->abfd = bfd_openr( fileName, NULL);

    if( self->abfd == NULL)
    {
        Any_fprintf( stderr, "Error opening %s\n", fileName );
        goto out;
    }

    if( !bfd_check_format( self->abfd, bfd_object ))
    {
        Any_fprintf( stderr, "%s not a bfd_object file\n", fileName );
        bfd_close( self->abfd );
        self->abfd = NULL;
        goto out;
    }

    if((bfd_get_file_flags( self->abfd ) & HAS_SYMS ) == 0 )
    {
        Any_fprintf( stderr, "The binary file %s doesn't contains symbols\n", fileName );
        bfd_close( self->abfd );
        self->abfd = NULL;
        goto out;
    }

    /* try static symbols first */
    self->numSyms = bfd_read_minisymbols( self->abfd, false, (void **)&self->symTable, &storage );

    if( self->numSyms == 0 )
    {
        self->numSyms = bfd_read_minisymbols( self->abfd, true, (void **)&self->symTable, &storage );
    }

    if( self->numSyms >= 0 )
    {
        self->pc = (bfd_vma)0;
        self->section = NULL;
        self->file = NULL;
        self->func = NULL;
        self->line = 0;
        self->found = false;

        retVal = true;
    }

    out:
    return retVal;
}


/* some API parameters unused but kept for consistency */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


static void BackTrace_findAddressInSection( bfd *abfd, asection *section, void *data )
{
    BackTrace *self = (BackTrace *)data;
    bfd_vma baseAddress;
    bfd_size_type size;

    if( self->found )
    {
        return;
    }

#ifdef NEW_BFD_API
    if((bfd_section_flags( section ) & SEC_ALLOC ) == 0 )
    {
        return;
    }

    baseAddress = bfd_section_vma( section );

    if( self->pc < baseAddress )
    {
        return;
    }

    size = bfd_section_size( section );
#else
    if((bfd_get_section_flags( self->abfd, section ) & SEC_ALLOC ) == 0 )
    {
        return;
    }

    baseAddress = bfd_get_section_vma( self->abfd, section );

    if( self->pc < baseAddress )
    {
        return;
    }

    size = bfd_section_size( self->abfd, section );
#endif

    if( self->pc >= baseAddress + size )
    {
        return;
    }

    self->found = bfd_find_nearest_line( self->abfd, section, self->symTable, self->pc - baseAddress,
                                         &self->file, &self->func, &self->line ) ? true : false;
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


bool BackTrace_findSymbolInfo( BackTrace *self, bfd_vma address,
                               char *symbolName, char *sourceFile, unsigned int *lineNumber )
{
    self->pc = address;
    self->found = false;

    /*
     * search the symbol over the sections
     */
    bfd_map_over_sections( self->abfd, BackTrace_findAddressInSection, (void *)self );

    if( self->found )
    {
        char *demangledName = NULL;

#if defined( HAVE_CPLUS_DEMANGLE )
        demangledName = cplus_demangle( self->func, DMGL_ANSI | DMGL_PARAMS );
#endif

#if defined( HAVE_BFD_DEMANGLE )
        demangledName = bfd_demangle( self->abfd, self->func, DMGL_ANSI | DMGL_PARAMS);
#endif

        if( demangledName )
        {
            Any_strcpy( symbolName, demangledName );
            free( demangledName );
        }
        else
        {
            Any_strcpy( symbolName, self->func );
        }

        if( self->file )
        {
            Any_strcpy( sourceFile, self->file );
        }
        else
        {
            Any_strcpy( sourceFile, "??" );
        }

        *lineNumber = self->line;
    }

    return ( self->found );
}


unsigned long BackTrace_findSymbolAddress( BackTrace *self, const char *symbol )
{
    int i;
    symbol_info syminfo;

    for( i = 0; i < self->numSyms; i++ )
    {
        if( Any_strcmp( self->symTable[ i ]->name, symbol ) == 0 )
        {
            bfd_symbol_info( self->symTable[ i ], &syminfo );
            return (unsigned long)syminfo.value;
        }
    }

    return 0;
}


void BackTrace_close( BackTrace *self )
{
    free( self->symTable );
    self->symTable = NULL;

    bfd_close( self->abfd );
    self->abfd = NULL;
}


#endif

/* EOF */
