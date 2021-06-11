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


#ifndef ANYDEF_H
#define ANYDEF_H

#include <stdio.h>
#include <stdlib.h>

#include <AnyString.h>
#include <AnyExit.h>


/* superfluous macros, only kept for source compatibility */

#ifndef ANY_HOSTINFO
#define ANY_HOSTINFO
#endif

#ifndef ANY_DEF_SRCTAG
#define ANY_DEF_SRCTAG( str )
#endif

#ifndef ANY_DEF_BINTAG
#define ANY_DEF_BINTAG
#endif

#ifndef ANY_INLINE
#define ANY_INLINE
#endif


/* public API */

#ifndef ANY_REQUIRE_EXIT_DEFAULT
#define ANY_REQUIRE_EXIT_DEFAULT() AnyExit_exit( EXIT_FAILURE )
#endif

#ifndef ANY_REQUIRE_EXIT
#define ANY_REQUIRE_EXIT() ANY_REQUIRE_EXIT_DEFAULT()
#endif


#define ANY_OPTIONAL( cond )


#if __GNUC__ >= 3

#define ANY_LIKELY( __predicate )               \
__builtin_expect( !!( __predicate ), 1 )
#else
#define ANY_LIKELY( __predicate )             \
( __predicate )
#endif /* __GNUC__ >= 3 */


#if __GNUC__ >= 3

#define ANY_UNLIKELY( __predicate )             \
__builtin_expect( !!( __predicate ), 0 )
#else
#define ANY_UNLIKELY( __predicate )             \
( __predicate )
#endif /* __GNUC__ >= 3 */


#define ANY_REQUIRE( __cond )                                                \
do                                                                           \
{                                                                            \
  if( !( __cond ) )                                                          \
  {                                                                          \
    Any_fprintf( stderr, __BASENAME_FILE__ ":%d ANY_REQUIRE( %s ) failed!\n",\
                 __LINE__, #__cond );                                        \
                                                                             \
    Any_fireRequire();                                                       \
                                                                             \
    AnyExit_exit( EXIT_FAILURE );                                            \
  }                                                                          \
}                                                                            \
while ( 0 )


#define ANY_REQUIRE_MSG( __cond, __message )                                 \
do {                                                                         \
  if ( !(__cond) )                                                           \
  {                                                                          \
    Any_fprintf( stderr, __BASENAME_FILE__ ":%d %s\n", __LINE__, __message );\
                                                                             \
    Any_fireRequire();                                                       \
                                                                             \
    AnyExit_exit( EXIT_FAILURE );                                            \
  }                                                                          \
} while ( 0 )


/*!
 * \brief like ANY_REQUIRE(), but takes format string with variadic number of arguments
 *
 * \code
 * ANY_REQUIRE_VMSG( self, "invalid struct pointer (%p)", self );
 * \endcode
 *
 * \param __cond statement that must be true, else exit with given message
 * \param __msg error message that contains at least one format specifier like %d etc.
 * \param ... variadic argument list for format string
 *
 * \attention This macro needs to be called with at least 3 parameters.
 *            If you do not have any format specifier in the message, please
 *            use \c ANY_REQUIRE_MSG instead.
 */
#define ANY_REQUIRE_VMSG( __cond, __msg, ... )                               \
do                                                                           \
{                                                                            \
  if( ANY_UNLIKELY( !( __cond ) ) )                                          \
  {                                                                          \
    Any_fprintf( stderr, "%s:%d " __msg "\n", __BASENAME_FILE__, __LINE__, __VA_ARGS__ );\
    Any_fireRequire();                                                       \
    ANY_REQUIRE_EXIT();                                                      \
  }                                                                          \
}                                                                            \
while( 0 )


#if defined(__cplusplus)
    #define ANY_BEGIN_C_DECLS extern "C" {
    #define ANY_END_C_DECLS }
#else
    #define ANY_BEGIN_C_DECLS
    #define ANY_END_C_DECLS
#endif


#define ANY__PRIVATE_MACRO_STRING( __string ) #__string

#define ANY_MACRO_STRING( parameter ) ANY__PRIVATE_MACRO_STRING(parameter)

#ifndef ANY_MACRO_CONCAT

#define ANY__PRIVATE_MACRO_CONCAT( __param1, __param2 ) __param1##__param2

#define ANY_MACRO_CONCAT( parameter1, parameter2 ) \
  ANY__PRIVATE_MACRO_CONCAT(parameter1,parameter2)
#endif


#if defined(__windows__) || defined(__msvc__)

#define CCALL __cdecl
#if !defined(__mingw__)
#pragma section(".CRT$XCU",read)
#endif

/*!
 * \brief Macro for defining the library startup function
 * \param __library_name__ name of the library
 *
 * This macro defines the function that will be called when the library is loaded the first time.
 *
 * Example:
 * \code
 *   ANY_LIB_INIT( MyModule )
 *   {
 *     // Put here code which will be executed when library is loaded
 *     MyModule_staticAccess = true;
 *
 *
 *     MyModule_mutex = Mutex_new();
 *     Mutex_init( MyModule_mutex )
 *   }
 * \endcode
 */
#define ANY_LIBRARY_INIT( __library_name__ ) \
   static void __cdecl __library_name__##_libraryInit(void); \
   __declspec(allocate(".CRT$XCU")) void (__cdecl*__library_name__##_libraryInit##_)(void) = __library_name__##_libraryInit; \
   static void __cdecl __library_name__##_libraryInit(void)

#elif defined(__GNUC__)

#define CCALL
/*!
 * \brief Macro for defining the library startup function
 * \param __library_name__ name of the library
 *
 * This macro defines the function that will be called when the library is
 * loaded the first time.
 *
 * \note If you have multiple libraries using this macro, the execution
 *       order is undefined.
 *
 * \attention This macro is for emergency cases only as happened with IPP
 *            in April 2010.
 *
 * Example:
 * \code
 *   ANY_LIB_INIT( MyModule )
 *   {
 *     // Put here code which will be executed when library is loaded
 *     MyModule_staticAccess = true;
 *
 *
 *     MyModule_mutex = Mutex_new();
 *     Mutex_init( MyModule_mutex )
 *   }
 * \endcode
 */
#define ANY_LIBRARY_INIT( __library_name__ ) \
   static void __library_name__##_libraryInit(void) __attribute__((constructor)); \
   static void __library_name__##_libraryInit(void)

#endif


#if defined(__windows__) || defined(__msvc__)

#define CCALL __cdecl
#if !defined(__mingw__)
#pragma section(".CRT$XCU",read)
#endif

/*!
 * \brief Macro for defining the library closing function
 * \param __library_name__ name of the library
 *
 * This macro defines the function that will be called when the library is un-loaded.
 *
 * Example:
 * \code
 *   ANY_LIBRARY_CLEAR( MyModule )
 *   {
 *     // Put here code which will be executed when library is un-loaded
 *     MyModule_staticAccess = true;
 *
 *
 *     Mutex_clear( MyModule_mutex )
 *     Mutex_delete( MyModule_mutex )
 *   }
 * \endcode
 */
#define ANY_LIBRARY_CLEAR( __library_name__ )

//TODO: Define the clear function for windows


#elif defined(__GNUC__)

#define CCALL
/*!
 * \brief Macro for defining the library closing function
 * \param __library_name__ name of the library
 *
 * This macro defines the function that will be called when the library is un-loaded.
 *
 * Example:
 * \code
 *   ANY_LIBRARY_CLEAR( MyModule )
 *   {
 *     // Put here code which will be executed when library is un-loaded
 *     MyModule_staticAccess = true;
 *
 *
 *     Mutex_clear( MyModule_mutex )
 *     Mutex_delete( MyModule_mutex )
 *   }
 * \endcode
 */
#define ANY_LIBRARY_CLEAR( __library_name__ ) \
   static void __library_name__##_libraryClear(void) __attribute__((destructor)); \
   static void __library_name__##_libraryClear(void)

#endif


#endif


/* EOF */
