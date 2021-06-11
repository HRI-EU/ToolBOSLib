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


#ifndef ESC_H
#define ESC_H


/*!
 * \page ESC_About Error status communication
 *
 * The Error Status Communication (ESC) module is designed to allow
 * a verbose yet efficient communication of error states of functions.
 *
 * The module provides macros for error handling. There are
 * two versions implemented that can be enabled/disabled via the
 * compiler flag ESC_SILENT.
 * \li ESC_SILENT is not defined: errors are printed to stderr
 * (location, error code, and error name)
 * \li ESC_SILENT is defined: errors are not printed, only return
 * values communicate an error.
 *
 * The default case is the version with printing. The silent version
 * is meant for easing the porting to ECUs without any methods for
 * communicating text messages.
 *
 * There are three macros defined:
 * \li ESC_RETURN_ON( condition, errorcode ): if 'condition' is
 * true, the current function is aborted by returning 'errorcode'
 * \li ESC_RETURN_ON_NULL_PTR( ptr, errorcode ): if 'ptr' is a
 * NULL-pointer, the function is aborted by returning 'errorcode'
 * \li ESC_RETURN_ON_ERROR( status, errorcode ): if 'status' is not
 * ESC_NO_ERROR, the function is aborted by returning
 * 'errorcode'. This allows to propagate errors from lower level
 * functions upward.
 *
 * The normal error codes are in the range 0..255, where 0 equals
 * ESC_NO_ERROR. When using ESC_RETURN_ON_ERROR,
 * the original error gets shifted by 8 bits and the error code of the
 * caller is appended. The resulting error code thus contains the
 * original error as well as the hierarchy of intermediate functions
 * (up to 7 additional levels for the 64bit return value). When
 * printing this return value in hexadecimal notation, the user can
 * easily parse the error codes of the different levels.
 *
 * <h3>Usage:</h3>
 * Error codes of own libraries are suggested to be defined in files
 * named LIBNAMEErrorCodes.h. It is also suggested to define
 * them in hexadecimal, so as to allow easier human lookup of error
 * reasons.
 * It is also suggested to use one unique error code per possible
 * error so it is easier to pinpoint a concrete error location.
 * For errors that are very common, or don't need a very exact location,
 * the usage of the ESC_GENERIC_* error codes are recommended.
 * User library error codes should then not overlap this range
 * (suggested first error code could be 0x10).
 *
 * <h3>Suggested naming convention for error codes:</h3>
 * LIBNAME_<file>_<func>_<what>
 *  \li file : unique identifier for the file
 *  \li func : unique identifier for the function in which the error occurs
 *  \li what : unique (for this function) description of the error
 *    - e.g. &lt;var>_IS_NULL: variable 'var' is a NULL pointer
 *    - <func>_FAILED: The function 'func' returned a value that is not ESC_NO_ERROR.
 *
 * Example: LIBNAME_COMM_WRITE_MESSAGE_IS_NULL:
 * \li file = LibnameCommunicationFunctions.h
 * \li func = LibnameCommunication_write()
 * \li what = MESSAGE_IS_NULL -> variable 'message' is a NULL pointer.
 *
 * <h3>Example:</h3>
 * \code
 * #define ESC_EXAMPLE_FOO_ERROR_DESCRIPTION 0xAB
 * #define ESC_EXAMPLE_BAR_FOO_FAILED        0xCD
 *
 * ESCStatus foo() {
 *   ...
 *   ESC_RETURN_ON( somecondition, ESC_EXAMPLE_FOO_ERROR_DESCRIPTION );
 *   ...
 *   return ESC_NO_ERROR;
 * }
 *
 * ESCStatus bar() {
 *   ...
 *   ESCStatus status = foo();
 *   ESC_RETURN_ON_ERROR( status, ESC_EXAMPLE_BAR_FOO_FAILED );
 *   ...
 *   return ESC_NO_ERROR;
 * }
 * \endcode
 * In case 'comecondition' is true, foo() will return 0xAB and bar()
 * will return 0xABCD, else both foo() and bar() will return
 * ESC_NO_ERROR.
 */


/*--------------------------------------------------------------------------*/
/* Includes                                                                 */
/*--------------------------------------------------------------------------*/

#include <stdint.h>

#ifndef ESC_SILENT /* do verbose error logging? */
#include <stdio.h>
#include <inttypes.h>
#endif

/*--------------------------------------------------------------------------*/
/* Public definitions and datatypes                                         */
/*--------------------------------------------------------------------------*/

/* Typedef for returning error codes */
typedef uint64_t ESCStatus;

/* Maximum number of errors per hierarchy level (must be power of 2!) */
#define ESC_HIERARCHYSIZE (UINT64_C(256))

/* prefer to use basename, but fall back to full path if not set */
#ifdef __BASENAME_FILE__
#define ESC_FILE (__BASENAME_FILE__)
#else
#define ESC_FILE __FILE__
#endif

#if defined (WIN32) || defined (WIN64)
#define __func__ __FUNCTION__
#endif

/* Code for "no error" */
#define ESC_NO_ERROR (UINT64_C( 0x00 ))

/* Generic error codes */
#define ESC_GENERIC_SELF_IS_NULL     ( 0x01 ) /* main structure pointer is NULL */
#define ESC_GENERIC_ARGUMENT_IS_NULL ( 0x02 ) /* any function argument is a NULL pointer */
#define ESC_GENERIC_SELF_CORRUPT     ( 0x03 ) /* function detects a memory corruption */
#define ESC_GENERIC_OUT_OF_MEMORY    ( 0x04 ) /* memory allocation failed */


/* Define the main macros ESC_RETURN_ON, ESC_RETURN_ON_NULL_PTR,
   and ESC_RETURN_ON_ERROR depending on the ESC_SILENT define. */

#ifndef ESC_SILENT /* do verbose error logging? */

/* if condition is true, return errno and print string */
#define ESC_RETURN_ON( _condition_, _errno_ )                           \
    do                                                                  \
    {                                                                   \
        if ( (_condition_) )                                            \
        {                                                               \
            ESCStatus _my_errno_ = (ESCStatus)(_errno_);                \
            fprintf( stderr, "%s:%d in function %s(): condition '" #_condition_ "' occurred, error=0x%016" PRIx64 " : '" #_errno_ "'\n", \
                     ESC_FILE, __LINE__, __func__, _my_errno_ );        \
            return _my_errno_;                                          \
        }                                                               \
    }                                                                   \
    while(0)

/* if pointer is null, return errno and print string */
#define ESC_RETURN_ON_NULL_PTR( _ptr_, _errno_ )                        \
    do                                                                  \
    {                                                                   \
        if ( (_ptr_) == NULL )                                          \
        {                                                               \
            ESCStatus _my_errno_ = (ESCStatus)(_errno_);                \
            fprintf( stderr, "%s:%d in function %s(): pointer '" #_ptr_ "' is NULL, error=0x%016" PRIx64 " : '" #_errno_ "'\n", \
                     ESC_FILE, __LINE__, __func__, _my_errno_ );        \
            return _my_errno_;                                          \
        }                                                               \
    }                                                                   \
    while(0)

/* if an error is received, propagate error and print string */
#define ESC_RETURN_ON_ERROR( _recvError_, _myError_ )                   \
    do                                                                  \
    {                                                                   \
        ESCStatus _my_recvError_ = (_recvError_);                       \
        if ( _my_recvError_ != ESC_NO_ERROR )                           \
        {                                                               \
            ESCStatus _status_ = (ESC_HIERARCHYSIZE) * (_my_recvError_) + (ESCStatus)( (_myError_) & UINT64_C( 0xFF ) ); \
            fprintf( stderr, "%s:%d in function %s(), error=0x%016" PRIx64 " : '" #_myError_ "'\n", \
                     ESC_FILE, __LINE__, __func__, _status_ );          \
            return _status_;                                            \
        }                                                               \
    }                                                                   \
    while(0)

#else /* silent error logging */

/* if condition is true, return errno */
#define ESC_RETURN_ON( _condition_, _errno_ )           \
    do                                                  \
    {                                                   \
        if ( (_condition_) )                            \
        {                                               \
            return (ESCStatus)(_errno_);                \
        }                                               \
    }                                                   \
    while(0)

/* if pointer is null, return errno */
#define ESC_RETURN_ON_NULL_PTR( _ptr_, _errno_ )        \
    do                                                  \
    {                                                   \
        if ( (_ptr_) == NULL )                          \
        {                                               \
            return (ESCStatus)(_errno_);                \
        }                                               \
    }                                                   \
    while(0)

/* if an error is received, propagate error */
#define ESC_RETURN_ON_ERROR( _recvError_, _myError_ )                   \
    do                                                                  \
    {                                                                   \
        ESCStatus _my_recvError_ = (_recvError_);                       \
        if ( (_my_recvError_) != ESC_NO_ERROR )                         \
        {                                                               \
            return (ESC_HIERARCHYSIZE) * (_my_recvError_) + (ESCStatus)( (_myError_) & UINT64_C( 0xFF ) ); \
        }                                                               \
    }                                                                   \
    while(0)

#endif

#endif


/* EOF */
