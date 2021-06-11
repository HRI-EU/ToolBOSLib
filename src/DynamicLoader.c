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


#include <stdlib.h>

#if !defined(__windows__)

#include <dlfcn.h>

#endif

#include <Any.h>
#include <DynamicLoader.h>

#if defined(__windows__) || defined(__msvc__)

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>


#define DYNAMICLOADEREXT_VALID   0x947cbfeb
#define DYNAMICLOADEREXT_INVALID 0xe598c19f


static void *DynamicLoader_findInAnyModuleUsingToolhelp( const char *symbolName );
static void *DynamicLoader_findInAnyModuleUsingPSApi( const char *symbolName );
static char *DynamicLoader_getLastWindowMessage( DynamicLoader *self );

#endif


#define DYNAMICLOADER_VALID   0xe2d5b04d
#define DYNAMICLOADER_INVALID 0x226da021
#define DYNAMICLOADER_SYMBOLNAME_MAXLEN ( 256 )


DynamicLoader *DynamicLoader_new( void )
{
    DynamicLoader *self = ANY_TALLOC( DynamicLoader );
    ANY_REQUIRE_MSG( self, "memory allocation in DynamicLoader_new() failed" );

    self->valid = DYNAMICLOADER_INVALID;

    return self;
}


int DynamicLoader_init( DynamicLoader *self, const char *libraryName )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == DYNAMICLOADER_INVALID );

#if !defined(__windows__)
    self->libraryHandle = dlopen( libraryName, RTLD_GLOBAL | RTLD_NOW );
#else
    if ( libraryName )
    {
      self->libraryHandle = (void *)LoadLibrary( libraryName );
    }
#endif

    if( self->libraryHandle || libraryName == NULL)
    {
        if( libraryName )
        {
            int len = Any_strlen( libraryName );
            self->libraryName = ANY_BALLOC( len + 1 );

            ANY_REQUIRE_MSG( self->libraryName,
                             "Unable to allocate memory for the libraryName" );

            Any_strcpy( self->libraryName, libraryName );
        }

        self->valid = DYNAMICLOADER_VALID;
    }

    return (( self->libraryHandle != NULL || libraryName == NULL) ? 0 : -1 );
}


/* some API parameters unused but kept for consistency */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

char *DynamicLoader_getError( DynamicLoader *self )
{
    char *retVal = NULL;

#if !defined(__windows__) && !defined(__msvc__)
    retVal = dlerror();
#else
    retVal = DynamicLoader_getLastWindowMessage( self );
#endif

    return retVal;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


char *DynamicLoader_getLibraryName( DynamicLoader *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == DYNAMICLOADER_VALID );

    return ( self->libraryName );
}


/* function-pointer to data-pointer casts which are not compatible with "-pedantic" */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


DynamicLoaderFunction DynamicLoader_getSymbolByName( DynamicLoader *self,
                                                     const char *symbolName )
{
    DynamicLoaderFunction retVal = (DynamicLoaderFunction)NULL;

    ANY_OPTIONAL( self );
    ANY_REQUIRE( symbolName );


#if defined(__windows__) || defined(__msvc__)
    /*
     * self || libraryName == NULL means a global symbol so our executable
     * but since cyg/windows doesn't have any support for searching
     * symbols in the current process than we have to find a different
     * way for emulating the dl library behaviours
     */
    if ( self == NULL || ( self && self->libraryName == NULL ) )
    {
      if ( ( retVal = DynamicLoader_findInAnyModuleUsingToolhelp( symbolName ) ) == NULL &&
           ( retVal = DynamicLoader_findInAnyModuleUsingPSApi( symbolName ) ) == NULL )
      {
        return( retVal );
      }
    }
    else
    {
      ANY_REQUIRE( self->valid == DYNAMICLOADER_VALID );

      retVal = (DynamicLoaderFunction)GetProcAddress( (HMODULE)self->libraryHandle,
                                                      symbolName );
    }
#else /* !windows arch */
    if( self )
    {
        ANY_REQUIRE( self->valid == DYNAMICLOADER_VALID );

        retVal = (DynamicLoaderFunction)dlsym( self->libraryHandle, symbolName );
    }
    else
    {
        void *handle = NULL;

        handle = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL );

        if( handle )
        {
            retVal = (DynamicLoaderFunction)dlsym( handle, symbolName );
            dlclose( handle );
        }
        else
        {
            ANY_LOG( 1, "dlopen() on the current executable return an error",
                     ANY_LOG_ERROR );
        }
    }
#endif

    return retVal;
}


DynamicLoaderFunction DynamicLoader_getSymbolByClassAndMethodName( DynamicLoader *self,
                                                                   const char *className,
                                                                   const char *methodName )
{
    char symbolName[DYNAMICLOADER_SYMBOLNAME_MAXLEN] = "";

    ANY_OPTIONAL( self );
    ANY_REQUIRE( className );
    ANY_REQUIRE( methodName );

    Any_snprintf( symbolName,
                  DYNAMICLOADER_SYMBOLNAME_MAXLEN - 1,
                  "%s_%s",
                  className,
                  methodName );

    return DynamicLoader_getSymbolByName( self, symbolName );
}


void DynamicLoader_clear( DynamicLoader *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == DYNAMICLOADER_VALID );

    self->valid = DYNAMICLOADER_INVALID;

#if defined(__windows__) || defined(__msvc__)
    if ( self->libraryHandle )
    {
      FreeLibrary( (HMODULE)self->libraryHandle );
    }
#else
    dlclose( self->libraryHandle );
#endif

    self->libraryHandle = 0;

    ANY_FREE( self->libraryName );
    self->libraryName = NULL;
}


void DynamicLoader_delete( DynamicLoader *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == DYNAMICLOADER_INVALID );

    ANY_FREE( self );
}


DynamicLoaderFunction DynamicLoader_getFunctionSymbol( DynamicLoader *self,
                                                       const char *name )
{
    return DynamicLoader_getSymbolByName( self, name );
}


void *DynamicLoader_getDataSymbol( DynamicLoader *self, const char *name )
{
    return (void *)DynamicLoader_getSymbolByName( self, name );
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


/* static members */

#if defined(__windows__) || defined(__msvc__)

/*
 * Try to find a symbol by using the CreateToolhelp32Snapshot() and friends
 */
static void *DynamicLoader_findInAnyModuleUsingToolhelp( const char *symbolName )
{
  HANDLE snapshot = NULL;
  MODULEENTRY32 me32;
  void *retVal = NULL;

  ANY_REQUIRE_MSG( symbolName, "The symbolName must be valid" );

  /*
   * The idea is to make a snapshot of the modules in the current process
   * and than start to enumerate the modules searching the matching symbol.
   */
  snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, 0 );

  if ( snapshot == INVALID_HANDLE_VALUE )
  {
    ANY_LOG( 0, "Unable to make a snapshot", ANY_LOG_WARNING );
    goto out;
  }

  me32.dwSize = sizeof( me32 );

  /*
   * start with the firt symbol using Module32First()
   */
  if ( Module32First( snapshot, &me32 ) )
  {
    do {

      /*
       * try get the address of the requested symbolName
       */
      retVal = (void*)GetProcAddress( me32.hModule, symbolName );

      /*
       * if is valid than we got it
       */
      if ( retVal != NULL )
      {
        break;
      }

      /*
       * otherwise we go to the next one by using Module32Next()
       */
    } while ( Module32Next( snapshot, &me32 ) );
  }

  /*
   * Release the snapshot
   */
  CloseHandle( snapshot );

 out:

  return( retVal );
}


/*
 * Try to find a symbol by using the EnumProcessModules() within the psapi.dll
 */
static void *DynamicLoader_findInAnyModuleUsingPSApi( const char *symbolName )
{
  HMODULE *modules = NULL;
  HMODULE dummy = NULL;
  int size = 0;
  DWORD i = 0;
  DWORD numHandle = 0;
  DWORD needed = 0;
  void *retVal = NULL;

  /*
   * try to get the number of handle of each module in the process
   */
  if ( !EnumProcessModules( GetCurrentProcess(), &dummy,
                            sizeof( HMODULE ), &needed ) )
  {
    ANY_LOG( 0, "Unable to get the number of HMODULE's handle",
             ANY_LOG_WARNING );
    goto out;
  }

  /*
   * allocate the required space for all the HMODULE's handle
   */
  size = needed * sizeof( HMODULE );
  modules = ANY_NTALLOC( size, HMODULE );
  numHandle = needed;

  ANY_REQUIRE_MSG( modules, "Unable to allocate space for the array of HMODULE" );

  /*
   * try to get the handle of each module in the process
   */
  if ( !EnumProcessModules( GetCurrentProcess(), modules, size, &needed ) ||
       needed > size )
  {
    ANY_LOG( 0, "Unable to get the HMODULE's handle", ANY_LOG_WARNING );
    ANY_FREE( modules );
    goto out;
  }

  /*
   * finally we iterate in all HMODULE's handle trying to find
   * find the requested symbolName
   */
  for ( i = 0; i < numHandle; i++ )
  {
    if ( ( retVal = (void*)GetProcAddress( modules[i], symbolName ) ) != NULL )
    {
      break;
    }
  }

  /*
   * free the allocated space
   */
  ANY_FREE( modules );

 out:

  return( retVal );
}


static char *DynamicLoader_getLastWindowMessage( DynamicLoader *self )
{
  DWORD dwErrCode = GetLastError();
  char *errMsg = NULL;
  DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM; /* system wide message */


  /* The DynamicLoader_getError() function is useful during initialization,
   * at which the DynamicLoader itself may not be fully initialized, yet.
   *
   * In this case, we have to check not only for the self-pointer, but also
   * for the validity of the instance (see JIRA TBCORE-697).
   */
  if( ( self != (DynamicLoader*)NULL ) &&
      ( self->valid == DYNAMICLOADER_VALID ) )
  {
    errMsg = self->errMsg;
  }
  else
  {
    /*
     * if the self == NULL than we had to provide ourself
     * a space where to store the error message that has
     * to return back to the user. In that case we leak
     * that memory since the user's interface doesn't permit
     * to free the returned pointer
     */

    /* the function will allocate memory for the string */
    dwFlags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
  }

  FormatMessage( dwFlags,                    /* our flags                   */
                 NULL,                       /* no source buffer needed     */
                 dwErrCode,                  /* error code for this message */
                 0,                          /* default language ID         */
                (LPTSTR)&errMsg,             /* allocated by fcn            */
                 DYNAMICLOADER_MAXERRORSIZE, /* minimum size of buffer      */
                 NULL );                     /* no inserts                  */

  return( errMsg );
}

#endif


/* EOF */
