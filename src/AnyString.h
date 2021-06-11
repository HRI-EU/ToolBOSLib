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


#ifndef ANYSTRING_H
#define ANYSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__windows__) && !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <string.h>

#if defined(__windows__) && !defined(__mingw__)
#pragma warning( pop )
#endif

#if !defined(__windows__)

#include <strings.h>

#endif /* !defined(__windows__) */


/*!
 * \page AnyString_About String handling
 *
 * The string functions are splitted over two headerfiles:
 *
 * \li \ref AnyString.h (multi-platform support)
 * \li \ref UString.h (string manipulation)
 *
 * The UString (UString.h) library provides various utility functions for
 * common char* strings like replacing a char with a substring,
 * finding if a string begins with another substring, trimming and so on.
 *
 * These functions do not require an instance of the library itself to
 * work (i.e. they can be invoked directly to any char* string).
 *
 * In the following example a string gets trimmed, then a second
 * string is concatenated to it and a single char is concatenated
 * to the resulting string.
 *
 * \code
 * char *string1       = argv[ 1 ];
 * char *string2       = argv[ 2 ];
 * char *singlechar    = argv[ 3 ];
 * char *trimmedStr    = NULL;
 * int originalLength1 = -1;
 * int originalLength2 = -1;
 *
 * originalLength1 = strlen( string1 );
 * originalLength2 = strlen( string2 );
 *
 * printf( "\nFirst string:  '%s', original length %d\n", string1, originalLength1 );
 * printf( "\nSecond string:  '%s', original length %d\n", string2, originalLength2 );
 *
 * trimmedStr = UString_trim( string1 );
 *
 * if( originalLength1 - ( int )strlen( string1 ) >= originalLength2 + 1 )
 * {
 *   UString_append( string1, originalLength1, string2 );
 *   printf( "\nSecond string appended to the first:  '%s'\n", string1 );
 *   printf( "\nFirst string is now: '%s'\n", string1 );
 * }
 *
 * if( originalLength1 - strlen( string1 ) >= 2 )
 * {
 *   UString_appendChr( string1, singlechar[ 0 ], originalLength1 );
 * }
 * \endcode
 */


/*-------------------------------------------------------------------------------*/
/* String Operations                                                             */
/*-------------------------------------------------------------------------------*/


/*!
 * \brief compare two strings
 *
 * The Any_strcmp function compares the string pointed by \a __s1 with the
 * string pointed by \a __s2.
 *
 * \return An integer greater than, equal to or less then 0 if the string
 *         pointed to by \a __s1 is greater than, equal to or less then the string
 *         pointed to by __s2.
 *
 *
 */
#define Any_strcmp( __s1, __s2 )           strcmp( __s1, __s2 )

/*!
 * \brief compare the first \a __size bytes of two strings
 *
 * The Any_strncmp function works as the Any_strcmp function, except it only
 * compares the first \a __size characters of the strings \a __s1 and
 * \a __s2
 *
 * \return An integer greater than, equal to or less then 0 if the string
 *         pointed by \a __s1 is greater than, equal to or less then the string
 *         pointed by __s2.
 */
#define Any_strncmp( __s1, __s2, __size )  strncmp( __s1, __s2, __size )

/*!
 * \brief case-insensitive string comparison
 *
 * The Any_strcasecmp function compares the string pointed to by \a __s1
 * to the string pointed to by \a __s2 ignoring differences in case.
 *
 * \return An integer greater than, equal to or less then 0 if the string
 *         pointed by \a __s1 is greater than, equal to or less then the string
 *         pointed by __s2.
 *
 */
#define Any_strcasecmp( __s1, __s2 )       strcasecmp( __s1, __s2 )

/*!
 * \brief case-insensitive string comparisons of \a __size bytes
 *
 * The Any_strncasecmp function compares not more than \a __size bytes from
 * the string pointed to by \a __s1
 * to the string pointed to by \a __s2 ignoring differences in case.
 *
 * \return An integer greater than, equal to or less then 0 if the string
 *         pointed by \a __s1 is greater than, equal to or less then the string
 *         pointed by __s2.
 */
#define Any_strncasecmp( __s1, __s2, __size )  strncasecmp( __s1, __s2, __size )

/*!
 * \brief concatenate two strings
 *
 * The Any_strcat function appends a copy of the string pointed to by \a __src
 * (including the terminating null byte) to the end of the string pointed to by
 * \a __dst.
 *
 * \return \a __dst. No return value is reserved to indicate an error.
 *
 */
#define Any_strcat( __dst, __src )    strcat( __dst, __src )

/*!
 * \brief concatenate a string with part of another
 *
 * The Any_strncat function appends not more than \a __size bytes from the string
 * pointed to by \a __src to the end of the string pointed to by \a __dst.
 *
 * \return \a __dst. No return value is reserved to indicate an error.
 *
 */
#define Any_strncat( __dst, __src, __size )   strncat( __dst, __src, __size )

/*!
 * \brief string scanning operation
 *
 * The Any_strchr function locates the \e first occurrence of \a __c (converted to a
 * char) in the string pointed to by \a __s. The terminating null byte is
 * considered part of the string.
 *
 * return A pointer to the byte, of a null pointer if the bytes was not found.
 *
 */
#define Any_strchr( __s, __c )    strchr( __s, __c )

/*!
 * \brief string scanning operation
 *
 * The Any_strrchr function locates the \e last occurrence of \a __c (converted to a
 * char) in the string pointed to by \a __s. The terminating null byte is
 * considered part of the string.
 *
 * return A pointer to the byte, of a null pointer if the bytes was not found.
 *
 */
#define Any_strrchr( __s, __c )   strrchr( __s, __c )

/*!
 * \brief find a substring
 *
 * The Any_strstr function locates the first occurrence in the string
 * pointed to by \a __s1 of the sequence of bytes (excluding the terminating
 * null byte) in the string pointed to by \a __s2.
 *
 * \return A pointer to the located string or a null pointer if the string is
 *         not found. If \a __s2 points to a string with zero length, the
 *         function returns \a __s1.
 *
 */
#define Any_strstr( __haystack, __needle )    strstr( __haystack, __needle )

/*!
 * \brief get string length
 *
 * The Any_strlen function computes the number of bytes in the string to which
 * \a __s points, not including the terminating null byte.
 *
 * \return The length of \a __s; no return value is reserved to indicate an
 *         error.
 */
#define Any_strlen( __s )   strlen( __s )

/*!
 * \brief determine the length of a fixed-size string.
 *
 * The Any_strnlen function returns the number of characters in the string pointed
 * to by \a __s, not including the terminating null character, but at most
 * \a __maxlen characters.
 *
 * \return The lenght of \a __s, or __maxlen if there is no null character in
 *         \a __s; no return value is reserved to indicate an error.
 */
#ifdef __macos__
#define Any_strnlen( __s, __maxlen )  strlen( __s )
#else
#define Any_strnlen( __s, __maxlen )  strnlen( __s, __maxlen )
#endif

/*!
 * \brief copy a string
 *
 * The Any_strcpy function copies the string pointed to by \a __src (including the
 * terminating null byte) into the array pointed to by \a __dst.
 *
 * \return \a __dst; no value is reserved to indicate an error.
 */
#define Any_strcpy( __dst, __src )  strcpy( __dst, __src )

/*!
 * \brief copy part of a string
 *
 * The Any_strncpy function copies not more than \a __size bytes from the array
 * pointed to by \a __src to the array pointed to by \a __dst. If the array pointed
 * to by \a __dst is a string that is shorter than \a __size bytes, null bytes
 * will be appended to the copy in the array pointed to by \a __dst, until
 * \a __size bytes are written.
 *
 * \return \a dst; no return value is reserved to indicate an error.
 */
#define Any_strncpy( __dst, __src, __size )   strncpy( __dst, __src, __size )

/*!
 * \brief string comparison using collating information
 *
 * The Any_strcoll function compares the string pointed to by \a __s1 to the string
 * pointed to by \a __s2, both interpreted as appropriate to the \e LC_COLLATE
 * category of the current locale.
 *
 * \return An integer greater than, equal to or less than 0 if the string pointed to by
 *         \a __s1 is greater than, equal to or less than the string pointed to by
 *         \a __s2. On error, Any_strcoll sets \a errno, but no return value is
 *         reserved to indicate an error.
 */
#define Any_strcoll( __s1, __s2 )   strcoll( __s1, __s2 )

/*!
 * \brief get lenght of a substring
 *
 * The Any_strspn function computes the length of the maximum initial segment of
 * the string pointed to by \a __s which consists entirely of bytes from the
 * string pointed to by \a __accept.
 *
 * \return The lenght of \a __s; no return value is reserved to indicate an
 *         error.
 */
#define Any_strspn( __s, __accept )   strspn( __s, __accept )

/*!
 * \brief get the length of a complementary substring
 *
 * The Any_strspn function computes the length of the maximum initial segment of
 * the string pointed to by \a __s which consists entirely of bytes \e not
 * from the string pointed to by \a __accept.
 *
 * \return The lenght of \a __s; no return value is reserved to indicate an
 *         error.
 *
 */
#define Any_strcspn( __s, __reject )  strcspn( __s, __reject )

/*!
 * \brief split string into tokens
 *
 * A sequence of calls to Any_strtok breaks the string pointed to by \a __s into a
 * sequence of tokens, each of which is delimited by a byte from the string
 * pointed to by \a __delim. The first call in the sequence has \a __s as its
 * first argument, and is followed by calls with a null pointer as their first
 * argument. The separator string pointed to by \a __delim may be different
 * from call to call.
 *
 * \return A pointer to the first byte of a token. Otherwise, if there is no
 *         token, strtok returns a null pointer.
 *
 */
#define Any_strtok( __s, __delim )  strtok( __s, __delim )

/*!
 * \brief split string into tokens (reentrant)
 *
 * The Any_strtok_r function ia a \e reentrant version of Any_strtok function. The
 * \a __save_ptr argument is a pointer to a char * variable that is used
 * internally by Any_strtok_r in order to maintain context between successive
 * calls that parse the same string.
 *
 * \return A pointer to the first byte of a token. Otherwise, if there is no token
 *         Any_strtok_r return a null pointer.
 *
 */
#define Any_strtok_r( __s, __delim, __save_ptr )  strtok_r( __s, __delim, __save_ptr )

/*!
 * \brief extract token from string
 *
 * The Any_strsep function finds the first token in the string \a *__stringp,
 * where tokens are delimited by symbols in the string \a __delim. This token is
 * terminated with a \e '\\0' character (by overwriting the delimiter) and
 * \a __stringp is updated to point past the token. In case no delimiter was
 * found, the token is taken to be the entire string \a *__stringp, and
 * \a *__stringp is made NULL.
 *
 * \return A pointer to the token, that is, it return the original value of
 *         \a *__stringp.
 *
 */
#define Any_strsep( __stringp, __delim )  strsep( __stringp, __delim )

/*!
 * \brief duplicate a string
 *
 * The Any_strdup function returns a pointer to a new string, which is a duplicate of
 * the string pointed to by \a s.
 *
 * \return A pointer to a new string.
 */
#define Any_strdup( __s )   strdup( __s )


/*-------------------------------------------------------------------------------*/
/* String formatting                                                             */
/*-------------------------------------------------------------------------------*/


/*!
 * \brief print formatted output on standard output
 *
 * The Any_printf function places output on the standard output stream (stdout).
 * The arguments are converted, formatted and printed under control of the \a __format
 * string. Refer to POSIX Programmer's Manual for more information about the \a __format
 * string.
 *
 * \return The number of characters printed. On error a negative
 *         value is returned.
 */
#define Any_printf( ... )   printf( __VA_ARGS__ )

/*!
 * \brief print formatted output on stream
 *
 * The Any_fprintf function places output on the \a __file stream.
 * The arguments are converted, formatted and printed under control of the \a __format
 * string. Refer to POSIX Programmer's Manual for more information about the \a __format
 * string.
 *
 * \return The number of characters printed. On error a negative
 *         value is returned.
 */
#define Any_fprintf( __file, ... )    fprintf( __file, __VA_ARGS__ )

/*!
 * \brief print formatted output on character string
 *
 * The Any_sprintf function places output on the string pointed to by \a __string
 * argument, followed by null byte '\\0'.
 * The arguments are converted, formatted and printed under control of the \a __format
 * string. Refer to POSIX Programmer's Manual for more information about the \a __format
 * string.
 *
 * \return The number of characters which are written. On error a negative
 *         value is returned.
 */
#define Any_sprintf( __string, ... )    sprintf( __string, __VA_ARGS__ )

/*!
 * \brief print up to __size byte on character string
 *
 * The Any_snprintf function works like Any_sprintf, with the addition of the \a __size
 * argument which states the size of the buffer referred to by \a __string. \a __size
 * bytes are at most written (including the null character '\\0').
 *
 * \return The number of characters which would have been written. On error a negative
 *         value is returned.
 */
#define Any_snprintf( __string, __size, ... )     snprintf( __string, __size, __VA_ARGS__ )

/*!
 * \brief print formatted output on character string
 *
 * The Any_vsprintf function is equivalent to Any_sprintf, except that is called with
 * an argument list (the va_list \a __varargs argument) insted of being called with a
 * variable length argument.
 *
 * \return The number of characters which are written. On error a negative
 *         value is returned.
 */
#define Any_vsprintf( __string, __format, __va_list )    vsprintf( __string, __format, __va_list )

/*!
 * \brief print up to __size byte on character string
 *
 * The Any_vsnprintf function is equivalent to Any_snprintf, except that is called with
 * an argument list (the va_list \a __varargs argument) insted of being called with a
 * variable length argument.
 *
 * \return The number of characters which would have been written. On error a negative
 *         value is returned.
 */
#define Any_vsnprintf( __string, __size, __format, __va_list )    vsnprintf( __string, __size, __format, __va_list )

/*!
 * \brief read from the standard output
 *
 * The Any_scanf function reads from the standard input stream (stdin).
 * Each read byte is interpreted according to the format string \a __format. Converted
 * input are stored in a set of pointer arguments. Refer to POSIX Programmer's Manual
 * for more information about the \a __format string.
 *
 * \return The number of successfully matched and assigned input items. If an error
 *         occurs, EOF is returned and the \a errno variable set.
 */
#define Any_scanf( ... )    scanf( __VA_ARGS__ )

/*!
 * \brief read from the \a __file stream
 *
 * The Any_fscanf function reads from the \a __file stream.
 * Each read byte is interpreted according to the format string \a __format. Converted
 * input are stored in a set of pointer arguments. Refer to POSIX Programmer's Manual
 * for more information about the \a __format string.
 *
 * \return The number of successfully matched and assigned input items. If an error
 *         occurs, EOF is returned and the \a errno variable set.
 *
 */
#define Any_fscanf( __file, ... )     fscanf( __file, __VA_ARGS__ )

/*!
 * \brief read from the string \a __string
 *
 * The Any_sscanf function reads from the \a __string characters string.
 * Each read byte is interpreted according to the format string \a __format. Converted
 * input are stored in a set of pointer arguments. Refer to POSIX Programmer's Manual
 * for more information about the \a format string.
 *
 * \return The number of successfully matched and assigned input items. If an error
 *         occurs, EOF is returned and the \a errno variable set.
 *
 * \return
 */
#define Any_sscanf( __string, ... )     sscanf( __string, __VA_ARGS__ )

/*!
 * \brief read from the string \a __string
 *
 * The Any_vsscanf function is equivalent to Any_sscanf, except that is called with
 * an argument list (the va_list \a __varargs argument) insted of being called with a
 * variable length argument.
 *
 * \return The number of successfully matched and assigned input items. If an error
 *         occurs, EOF is returned and the \a errno variable set.
 */
#define Any_vsscanf( __string, __format, __va_list )    vsscanf( __string, __format, __va_list )

/*-------------------------------------------------------------------------------*/
/* Memory operations                                                             */
/*-------------------------------------------------------------------------------*/
/*!
 * \brief compare bytes in memory
 *
 * The Any_memcmp function compares the first \a __size bytes (each interpreted
 * as unsigned char) of the object pointed to by \a __s1 to the first \a __size
 * bytes of the object pointed to by \a __s2. The sign of a non-zero return
 * value is determined by the sign of the difference between the value of the
 * pair of bytes that differ in the object being compared.
 *
 * \return An integer greater than, equal to or less than 0 if the object pointed
 *         to by \a __s1 is greater than, equal to or less than the object
 *         pointed to by \a __s2.
 *
 */
#define Any_memcmp( __s1, __s2, __size )    memcmp( __s1, __s2, __size )

/*!
 * \brief copy memory area
 *
 * The Any_memcpy function copies \a __size bytes from the object pointed to
 * by \a __src into the object pointed to by \a __dst.
 *
 * \return \a __dst; no return value is reserved to indicate an error.
 */
#define Any_memcpy( __dst, __src, __size )    memcpy( __dst, __src, __size )

/*!
 * \brief copy bytes in memory with overlapping areas
 *
 * The Any_memmove function copies \a __size bytes from the object pointed to
 * by __src into the object pointed to by \a __dst. Copying takes place as if
 * the \a __size bytes from the object pointed to by \a __s2 are first copied
 * into a temporary array of \a __size bytes that does not overlap the object
 * pointed to by \a __dst and \a __src, and then the \a __size bytes from the
 * temporary array are copied into the object pointed to by \a __dst.
 *
 * \return \a __dst; no return value is reserved to indicate an error.
 *
 */
#define Any_memmove( __dst, __src, __size )    memmove( __dst, __src, __size )

/*!
 * \brief set bytes in memory
 *
 * The Any_memset function copies \a __c (converted to usigned char) into each
 * of the first \a __size bytes of the object pointed to by \a __s.
 *
 * \return \a __s; no return value is reserved to indicate an error.
 *
 */
#define Any_memset( __s, __c, __size )  memset( __s, __c, __size )

/*!
 * \brief find byte in memory
 *
 * The Any_memchr function locates the first occurrence of \a __c (converted to
 * unsigned char) in the initial \a __size bytes of the object pointed to by
 * \a __s.
 *
 * \return A pointer to the located byte, or a null pointer if the byte
 *         does not occur in the object.
 *
 */
#define Any_memchr( __s, __c, __size )  memchr( __s, __c, __size )

/*!
 * \brief locate a substring in memory
 *
 * The Any_memmem finds the start of the first occurrence of the substring
 * \a __needle of length \a __needlelen in the memory area \a __haystack
 * of length \a __haystacklen.
 *
 * \return A pointer to the beginning of the sub-string, or NULL if the substring
 *         is not found.
 *
 */
#ifdef __macos__
void *Any_memmem( const void *buf, int buflen, const void *pattern, int len );
#else
#define Any_memmem( __haystack, __haystacklen, __needle, __needlelen )    memmem( __haystack, __haystacklen, __needle, __needlelen )
#endif

/*-------------------------------------------------------------------------------*/
/* Some BSD compatibility ontop of arch specific wrapper                         */
/*-------------------------------------------------------------------------------*/

/*!
 * \brief set bytes in memory to 0
 *
 * The Any_bzero function sets the first \a __size bytes of \a __s to 0.
 *
 * \return __s
 */
#define Any_bzero( __s, __size )  Any_memset( __s, 0, __size )

/*!
 * \brief compare bytes in memory
 *
 * The Any_bcmp function compares the first \a __size bytes pointed to by \a __s1 to the first \a __size
 * bytes pointed to by \a __s2.
 *
 * \return An integer greater than, equal to or less than 0 if the object pointed
 *         to by \a __s1 is greater than, equal to or less than the object
 *         pointed to by \a __s2.
 *
 */
#define Any_bcmp( __s1, __s2, __size )  Any_memcmp( __s1, __s2, __size )

/*!
 * \brief search first occurrence of a character
 *
 * The Any_index function searches the first occurrence of the character \a __c in the string pointed to
 * by \a __s.
 *
 * \return A pointer to the byte, of a null pointer if the bytes was not found.
 */
#define Any_index( __s, __c )     Any_strchr( __s, __c )

/*!
 * \brief search last occurrence of a character
 *
 * The Any_rindex function searches the last occurrence of the character \a __c in the string pointed to
 * by \a __s.
 *
 * \return A pointer to the byte, of a null pointer if the bytes was not found.
 */
#define Any_rindex( __s, __c )    Any_strrchr( __s, __c )

/* Beware!! It's like memmove() but with __dst and __src reversed */
/*!
 * \brief copy memory area
 *
 * The Any_bcopy function copies \a __len bytes from the area pointed to \a __src
 * to the area pointed to by \a __dst.
 */
#define Any_bcopy( __src, __dst, __len )    Any_memmove( __dst, __src, __len )


#if defined( __windows__ )

#include <memory.h>

#if _MSC_VER < 1910

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <shlwapi.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#else                // VC 2017
#include <Shlwapi.h>
#endif


/*
 * String operations
 */

#undef Any_strcmp
#define Any_strcmp( __s1, __s2 )  StrCmp( __s1, __s2 )

#if _MSC_VER < 1600 // Visual C/C++ >= 2010 seems not defining StrCmpN() at all but a plain strncmp()
#undef Any_strncmp
#define Any_strncmp( __s1, __s2, __size )   StrCmpN( __s1, __s2, __size )
#endif

#undef Any_strcasecmp
#define Any_strcasecmp( __s1, __s2 )  StrCmpI( __s1, __s2 )

#undef Any_strncasecmp
#define Any_strncasecmp( __s1, __s2, __size )   StrCmpNI( __s1, __s2, __size )

#undef Any_strcat
#define Any_strcat( __dst, __src )  StrCat( __dst, __src )

#undef Any_strncat
#define Any_strncat( __dst, __src, __size )   StrNCat( __dst, __src, __size )

#undef Any_strstr
#define Any_strstr( __haystack, __needle )  StrStr( __haystack, __needle )

#undef Any_strcpy
#define Any_strcpy( __dst, __src )  StrCpy( __dst, __src )

#undef Any_strspn
#define Any_strspn( __s, __accept )   StrSpn( __s, __accept )

#undef Any_strtok_r

#if defined(__msvc__)
#define Any_strtok_r( __s, __delim, __save_ptr )  strtok_s( __s, __delim, __save_ptr )
#else
#define Any_strtok_r( __s, __delim, __save_ptr ) ( *( __save_ptr ) = strtok( ( __s ), ( __delim ) ) )
#endif

#undef Any_strsep
char *Any_strsep( char **string, const char *delimiters );

#undef Any_strdup
char *Any_strdup( char *str );

/*
 * String formatting
 */
#undef Any_snprintf
#define Any_snprintf( __string, __size, __format, ... )   _snprintf( __string, __size, __format, __VA_ARGS__ )

#undef Any_vsscanf
int Any_vsscanf( const char *str, const char *format, va_list ap );

#undef Any_vsnprintf
#define Any_vsnprintf( __string, __size, __format, __va_list )    _vsnprintf( __string, __size, __format, __va_list )

/*
 * Mem operations
 */
#undef Any_memmem
void *Any_memmem( const void *buf, int buflen, const void *pattern, int len );

#endif


#ifdef __cplusplus
}
#endif

#endif


/* EOF */
