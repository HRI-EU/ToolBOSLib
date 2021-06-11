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


#ifndef ANYLOG_H
#define ANYLOG_H


#include <stdio.h>

#if !defined(__msvc__) && !defined(__windows__)

#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>


#if defined(__cplusplus)
#include <iostream>
#include <iomanip>
#include <string>
#endif

#else

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

/*
 * used for va_list on ANY_LOG() and ANY_LOG_MSG()
 */
#include <stdarg.h>

#endif

#include <AnyStdBool.h>

#include <AnyString.h>
#include <AnyDef.h>
#include <AnyTime.h>


#if defined(__msvc__)
#include <Winsock.h>
#else
#include <sys/time.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief This variable is true if the log format is the complete one.
 */
extern bool longLogFormat;

/*!
 * \brief Struct used for ANY event information.
 */
typedef struct AnyEventInfo
{
    void (*function)( void * );

    /**< Function callback */
    void *functionParam;
    /**< Function callback param */
    struct AnyEventInfo *next;          /**< Next element in the linked list */
}
        AnyEventInfo;

/*!
 * \brief Change the current debug level
 * \param newlevel New debug level
 *
 * Change the current debug level.
 * Only the message with a debug level in [ANY_LOG_MIN_DEBUGLEVEL, ANY_LOG_CURRENT_DEBUGLEVEL] are shown.
 * The debug level takes value from 0 to MAXINT.
 * Messages with debug level equal to 0 are allways displayed.
 *
 */
void Any_setDebugLevel( int newlevel );

/*!
 * \brief Return the current debug level
 *
 * Return the current debug level.
 * Only the message with a debug level in [ANY_LOG_MIN_DEBUGLEVEL, ANY_LOG_CURRENT_DEBUGLEVEL] are shown.
 * The debug level takes value from 0 to MAXINT.
 * Messages with debug level equal to 0 are allways displayed.
 *
 * \return Return the current debug level
 */
int Any_getDebugLevel( void );

/*!
 * \brief Change the minimum debug level
 * \param newlevel New minimum debug level
 *
 * Change the minimum debug level.
 * Only the message with a debug level in [ANY_LOG_MIN_DEBUGLEVEL, ANY_LOG_CURRENT_DEBUGLEVEL] are shown.
 * The minimum debug level takes value from 0 to MAXINT.
 * Messages with debug level equal to 0 are allways displayed.
 *
 */
void Any_setMinDebugLevel( int newlevel );

/*!
 * \brief Return the minimum debug level
 *
 * Return the minimum debug level.
 * Only the message with a debug level in [ANY_LOG_MIN_DEBUGLEVEL, ANY_LOG_CURRENT_DEBUGLEVEL] are shown.
 * The minimum debug level takes value from 0 to MAXINT.
 * Messages with debug level equal to 0 are allways displayed.
 *
 * \return Return the minimum debug level
 */
int Any_getMinDebugLevel( void );

/*!
 * \brief Sets the long format for log messages
 */
void Any_setLongLogFormat( void );

/*!
 * \brief Sets the short format for log messages
 */
void Any_setShortLogFormat( void );

/*!
 * \brief Register a global event function for all the ANY_REQUIRE() and friends
 * \param function Function callback
 * \param functionParam Function callback param
 *
 * This function register a global function callback for all the ANY_REQUIRE() and friends.
 * Whenever the ANY_REQUIRE() fails all the registered handlers will be called in reverse
 * order of the registration
 *
 * \see Any_fireRequire()
 */
void Any_onRequire( void (*function)( void * ), void *functionParam );

/*!
 * \brief Fires all the globally registered ANY_REQUIRE() function callbacks
 */
void Any_fireRequire( void );

/*!
 * \brief Define a system specific getpid()
 */
#ifndef ANY_LOG_GETPID

#if defined(__windows__) || defined(__msvc__)

#define ANY_LOG_GETPID GetCurrentThreadId()

#else

#define ANY_LOG_GETPID getpid()

#endif

#endif

#ifndef ANY_LOG_FILE
/*!
 * \brief Output stream for ANY_LOG and ANY_TRACE macros
 */
#define ANY_LOG_FILE \
  stderr
#endif

/*!
 * \brief Default debug level (100)
 */
#define ANY_LOG_DEBUGLEVEL_DEFAULT 100

/*!
 * \brief Default minimum debug level (0)
 */
#define ANY_LOG_MIN_DEBUGLEVEL_DEFAULT 0


#ifndef ANY_LOG_CURRENT_DEBUGLEVEL
/*!
 * \brief Current debug level vaule for ANY_LOG and ANY_TRACE macros
 */
#define ANY_LOG_CURRENT_DEBUGLEVEL \
  Any_getDebugLevel()
#endif

#ifndef ANY_LOG_MIN_DEBUGLEVEL
/*!
 * \brief Minimum debug level vaule for ANY_LOG and ANY_TRACE macros
 */
#define ANY_LOG_MIN_DEBUGLEVEL \
  Any_getMinDebugLevel()
#endif


/*!
 * \brief Error message type for ANY_LOG
 */
#define ANY_LOG_ERROR       "Error"

/*!
 * \brief (obsolete) Fatal error message type for ANY_LOG
 */
#define ANY_LOG_FATAL       "FatalError"

/*!
 * \brief Warning message type for ANY_LOG
 */
#define ANY_LOG_WARNING     "Warning"

/*!
 * \brief Data message type for ANY_LOG
 */
#define ANY_LOG_DATA        "Data"

/*!
 * \brief Data message type for ANY_LOG
 */
#define ANY_LOG_DATA_CHECK  "DataCheck"

/*!
 * \brief Info message type for ANY_LOG
 */
#define ANY_LOG_INFO        "Info"


#ifndef ANY_LOG_MODULE_ID
/*!
 * \brief Define the module id (unsigned long)
 *
 * This macro shoul be redefined by use in order to identify the
 * id (unsigned long) of each module of an application.
 */
#define ANY_LOG_MODULE_ID  (0UL)
#endif

/*
 * In case the __BASENAME_FILE__ definition isn't given from the
 * compiler's command line then define a fallback to __FILE__
 */
#ifndef __BASENAME_FILE__
#define __BASENAME_FILE__ __FILE__
#endif

#ifndef ANY_LOG

/*!
 * \brief Log function
 * \param debugLevel debug level for the message
 * \param message message to be displayed
 * \param ... message type (ANY_LOG_*) and optional parameters
 *
 * This macro has to be used with the following syntax:
 *
 *   ANY_LOG( debugLevel, message, msgType, ... )
 *
 * Where:
 *
 *  <B>debugLevel</B> - is an integer that reppresents the debugging level.
 *
 *  <B>message</B> - is a string that reppresents the message that has to be
 *            displayed. The message could contains placeholder (like
 *            "\%d", "\%s", ...) for variables that should be listed after
 *            the msgType parameter.
 *
 *  <B>msgType</B> - is a string; in standard case one of the macros:
 *            ANY_LOG_FATAL, ANY_LOG_WARNING, ANY_LOG_DATA,
 *            ANY_LOG_INFO or any user msgType (as string).
 *
 * Example:
 * \code
 *    ANY_LOG( 5, "begin of the loop", ANY_LOG_INFO );
 *
 *    ANY_LOG( 1, "Wrong address: %p", ANY_LOG_WARNING, myPointer );
 * \endcode
 *
 * The output is structured like follows:
 *
 * \code
 * [<Timestamp> <ProcessID>:<ModuleID> <FileName>:<Line> <MsgType>] <Message>
 * \endcode
 *
 * <b>Example:</b>
 * \code
 * [1634017.186216 47d0:0 ProcessManager.c:138 Info] RTBOS started
 * \endcode
 *
 * \see Any_setDebugLevel
 * \see Any_setMinDebugLevel
 */

#define ANY_LOG( debugLevel, message, ... )                                  \
do {                                                                         \
     if ( ( debugLevel == 0 ) ||                                             \
          ( ( debugLevel >= ANY_LOG_MIN_DEBUGLEVEL ) &&                      \
            ( debugLevel <= ANY_LOG_CURRENT_DEBUGLEVEL ) ) )                 \
     {                                                                       \
        if ( longLogFormat )                                                 \
        {                                                                    \
          Any_fprintf( ANY_LOG_FILE,                                         \
                       "[%f %x:%lx " __BASENAME_FILE__ ":%d %s] " message "\n", \
                       Any_time() / 1000000000.0,                            \
                       ANY_LOG_GETPID,                                       \
                       ANY_LOG_MODULE_ID,                                    \
                       __LINE__,                                             \
                       __VA_ARGS__ );                                        \
        }                                                                    \
        else                                                                 \
        {                                                                    \
          Any_fprintf( ANY_LOG_FILE,                                         \
                       "[" __BASENAME_FILE__ ":%d %s] " message "\n",        \
                       __LINE__,                                             \
                       __VA_ARGS__ );                                        \
        }                                                                    \
        fflush( ANY_LOG_FILE );                                              \
     }                                                                       \
} while( 0 )

/* ANY_LOG not yet defined */
#endif


#if !defined (__BST_FILENAME__)
#if defined (BST_BASE_PATH_LENGTH)
#define __BST_FILENAME__ (__FILE__ + BST_BASE_PATH_LENGTH)
#else
#define __BST_FILENAME__ (__FILE__)
#endif
#endif


#define ANY_LOG_CPP( debugLevel, message, msgType )                          \
do {                                                                         \
     if ( ( debugLevel == 0 ) ||                                             \
          ( ( debugLevel >= ANY_LOG_MIN_DEBUGLEVEL ) &&                      \
            ( debugLevel <= ANY_LOG_CURRENT_DEBUGLEVEL ) ) )                 \
     {                                                                       \
        if ( longLogFormat )                                                 \
        {                                                                    \
            std::cerr << "[" << std::setprecision(6) << std::fixed           \
            << Any_time() / 1000000000.0 << " "                              \
            << std::setprecision(0) << std::hex << ANY_LOG_GETPID << ":"     \
            << ANY_LOG_MODULE_ID << std::dec << " "                          \
            << __BST_FILENAME__ << ":" << __LINE__ << " "                    \
            << msgType << "] " << message << std::endl;                      \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            std::cerr << "[" << __BST_FILENAME__ << ":"                      \
                      << __LINE__ << "] " << message << std::endl;           \
        }                                                                    \
     }                                                                       \
} while( 0 )


#ifndef ANY_TRACE
/*!
 * \brief Trace variable function
 * \param debugLevel debug level for the message
 * \param formatStr printf-string-like with the variable type
 * \param variable the variable to be displayed
 *
 * This macro can be used to log variables value. The syntax is:
 *
 *   ANY_TRACE( debugLevel, formatString, variable )
 *
 * Where:
 *
 *   <B>debugLevel</B> - is an integer that reppresents the debugging level.
 *
 *   <B>formatString</B> - is a string that reppresents the format on which
 *                  the parameter "variable" shoul be visualized.
 *                  This string must be in the "fprintf" format.
 *
 *   <B>variable</B> - this is the variable to be traced.
 *
 * Example:
 * \code
 *   ANY_TRACE( 1, "%d", userId );
 *
 *   ANY_TRACE( 5, "%s", fileName );
 * \endcode
 *
 * The output is structured like follows:
 *
 * \code
 * [<Timestamp> <ProcessID>:<ModuleID> <FileName>:<Line> Data] <variable>=<value>
 * \endcode
 *
 * <b>Example:</b>
 * \code
 * [1634350.509352 5827:0 UltimaTest_ext.cpp:124 Data] bbdmType=BBDMBlockF32
 * \endcode
 *
 * \see Any_setDebugLevel
 * \see Any_setMinDebugLevel
 */
#define ANY_TRACE( debugLevel, formatStr, variable ) \
ANY_LOG( debugLevel, #variable "=" formatStr, ANY_LOG_DATA, variable)
#endif

#if defined(__GNUC__) && defined(__cplusplus) && ( __GNUC__ < 3 )

#ifndef ANY_REQUIRE_LOG

/*!
 * \brief Macro for pre-/post-conditions with log message
 *
 * This macro is used to check pre or post-conditions. In the case the
 * condition is not satisfied then a log message is print to stderr
 * and ANY_REQUIRE_EXIT() is executed. The type of the log message is
 * automatically set to ANY_LOG_FATAL.
 *
 * The syntax is:
 *
 *   ANY_REQUIRE_LOG( condition, message, ... )
 *
 * Where:
 *
 *   <B>condition<B> - is a boolean condition that shoul be true if everithing is ok.
 *   <B>message<B> - is a message printed in case the condition is not satisfied.
 *   <B>message</B> - is a string that reppresents the message that has to be
 *            displayed. The message could contains placeholder (like
 *            "\%d", "\%s", ...) for variables that should be listed after it.
 *
 * Example:
 * \code
 *   ANY_REQUIRE_LOG( myPtr != (MyPtrType*)NULL,
 *                   "Unable to access to the pointer MyPtr = %p", myPtr );
 *
 *   ANY_REQUIRE_LOG( status == 0,
 *                    "Error in Socket.send: %s", SOCKET_GET_ERROR_MSG( status ) );
 * \endcode
 */
#define ANY_REQUIRE_LOG( __cond, __message, __msgTypeAndVaArgs... ) \
do {\
  if ( ANY_UNLIKELY( !( __cond ) ) )\
  {\
    ANY_LOG( 0, __message, ANY_LOG_FATAL, __msgTypeAndVaArgs );\
    Any_fireRequire();\
    ANY_REQUIRE_EXIT();\
  }\
} while ( 0 )

#endif  /* ANY_REQUIRE_LOG */

#else /* ( defined(__GNUC__) && ... */

#ifndef ANY_REQUIRE_LOG

/*!
 * \brief Macro for pre-/post-conditions with log message
 *
 * This macro is used to check pre or post-conditions. In the case the
 * condition is not satisfied then a log message is print to stderr
 * and ANY_REQUIRE_EXIT() is executed. The type of the log message is
 * automatically set to ANY_LOG_FATAL.
 *
 * The syntax is:
 *
 *   ANY_REQUIRE_LOG( condition, message, ... )
 *
 * Where:
 *
 *   <B>condition</B> - is a boolean condition that shoul be true if everithing is ok.
 *   <B>message</B> - is a message printed in case the condition is not satisfied.
 *   <B>message</B> - is a string that reppresents the message that has to be
 *            displayed. The message could contains placeholder (like
 *            "\%d", "\%s", ...) for variables that should be listed after it.
 *
 * Example:
 * \code
 *   ANY_REQUIRE_LOG( myPtr != (MyPtrType*)NULL,
 *                   "Unable to access to the pointer MyPtr = %p", myPtr );
 *
 *   ANY_REQUIRE_LOG( status == 0,
 *                    "Error in Socket.send: %s", SOCKET_GET_ERROR_MSG( status ) );
 * \endcode
 */

#define ANY_REQUIRE_LOG( __cond, __message, ... )                             \
do {                                                                          \
  if ( ANY_UNLIKELY( !( __cond ) ) )                                          \
  {                                                                           \
    ANY_LOG( 0, __message, ANY_LOG_FATAL, __VA_ARGS__ );                        \
    Any_fireRequire();                                                        \
    ANY_REQUIRE_EXIT();                                                       \
  }                                                                           \
} while ( 0 )

#endif

#endif /* ( defined(__GNUC__) && ... */

#ifndef ANY_LOG_ONCE

/*! \brief prints an ANY_LOG message only once */
#define ANY_LOG_ONCE( __debugLevel, __message, ... )                         \
do                                                                           \
{                                                                            \
  static short int AnyLogOnceSemaphor = 0;                                   \
                                                                             \
  if( AnyLogOnceSemaphor == 0 )                                              \
  {                                                                          \
    AnyLogOnceSemaphor = 1;                                                  \
    ANY_LOG( __debugLevel, __message, __VA_ARGS__ );                         \
  }                                                                          \
}                                                                            \
while( 0 )

#endif  /* ANY_LOG_ONCE */


/*!
 * \brief prints the name of the function (and more) where this macro is called
 *
 * \param __debugLevel at which debug level the message shall be printed
 */
#define ANY_WHERE( __debugLevel )                                            \
{                                                                            \
  ANY_LOG( __debugLevel, "in function: %s()", ANY_LOG_INFO, __FUNCTION__ );  \
}


#ifndef ANY_DEBUG_CODE
/*!
 * \brief Debug function that allows debug code definition
 * \param debugLevel debug level for the code
 *
 * This macro allows the definition of debug code that
 * must be executed only when the current debugLevel
 * is greater or equal to the the parameter.
 *
 * This macro can be used like in this example:
 *
 * \code
 * ANY_DEBUG_CODE( 2 )
 * {
 *    int i = 0;
 *
 *    // some debugging or testing code
 *
 *    if ( ( myIntVar == 100 ) && ( myBoolVar == true ) )
 *    {
 *      for ( i = 0; i < myIntVar; ++i )
 *      {
 *         // do something
 *      }
 *    }
 *
 * }
 * \endcode
 *
 *
 * \see Any_setDebugLevel
 * \see Any_setMinDebugLevel
 */
#define ANY_DEBUG_CODE( debugLevel ) \
if ( !( ( debugLevel == 0 ) || \
        ( ( debugLevel >= ANY_LOG_MIN_DEBUGLEVEL ) &&  \
          ( debugLevel <= ANY_LOG_CURRENT_DEBUGLEVEL ) ) ) ) \
{} else

#endif  /* ANY_DEBUG_CODE */


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
