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


#ifndef USTRING_H
#define USTRING_H


#include <Any.h>
#include <stdarg.h>

#ifdef __windows__
#include <string.h>

#ifndef __msvc__
int _stricmp( const char *string1, const char *string2 );
int _strnicmp( const char *string1, const char *string2, size_t len );
char *_strdup( const char *strSource );
#endif

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup

#endif


ANY_BEGIN_C_DECLS


/*!
 * \brief Split the \a string at each \a token and copy the
 *        resulting substrings onto the elements of the \a list.
 *
 * \param string String to be split;
 * \param list Pointer to the list receiving the substrings;
 * \param listSize Maximum number of elements of the list;
 * \param listElementSize Size of each element of the list;
 * \param token Char representing the delimiter of each substring.
 *
 * \return 0 in case of success; an error code in other cases.
 */
int UString_explode( const char *string,
                     char *list,
                     const unsigned int listSize,
                     const unsigned int listElementSize,
                     const char token );

/*!
 * \brief Replace all occurences of Chr with Str from the string srcString
 *
 * \param destString Destination buffer where the result is saved
 * \param destStringLen Destination buffer length
 * \param srcString String from where the replace should be done
 * \param chr Character that have to be replaced
 * \param str String that replace the character 'chr'
 *
 * \return Return 0 if the result string can be copied in the destString buffer
 *
 */
int UString_replaceChrWithStr( char *destString,
                               int destStringLen,
                               char *srcString,
                               char chr,
                               char *str );

/*!
 * \brief Replace all occurence of a list of chars with the corresponding string
 * contained in another list
 *
 * \param destString Destination buffer where the result is saved
 * \param destStringLen Destination buffer length
 * \param srcString String on which the replacing is performed
 * \param findChr[] Array of chars to be replaced
 * \param replaceString[] Array of string replacing every char
 *
 * \return Return 0 if the result string can be copyed in the destString buffer,
 * 1 otherwise (i.e. the destination buffer is too short).
 * If no occurrences of patternString is found on srcString, its value is
 * returned with no modifies.
 */
int UString_replaceChrArrayWithStrArray( char *destString,
                                         int destStringLen,
                                         char *srcString,
                                         char findChr[],
                                         char *replaceString[] );


/*!
 * \brief Test if a string starts with a certain character.
 *
 * \param string String to be tested
 * \param ch The character to test.
 *
 * \return Return 0 if the char is the beginning of the string, 1 otherwise
 */
int UString_startsWithChr( const char *string, const char ch );

/*!
 * \brief Test if a string ends with a certain character.
 *
 * \param string String to be tested
 * \param ch The character to test.
 *
 * \return Return 0 if the char is the end of the string, 1 otherwise
 */
int UString_endsWithChr( const char *string, const char ch );

/*!
 * \brief Check if a string starts with some characters
 *
 * \param string Source string
 * \param startStr Characters that should start the string
 *
 * \return Return 0 if the startStr represent the beginning of the string, 1 otherwise
 */
int UString_startsWith( const char *string, const char *startStr );

/*!
 * \brief Check if a string ends with some characters
 *
 * \param string Source string
 * \param endStr Characters that should end the string
 *
 * \return Return 0 if the endStr represent the ending of the string, 1 otherwise
 */
int UString_endsWith( const char *string, const char *endStr );

/*!
 * \brief [OBSOLETE] Returns the position of the first occurrence of a character
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns position of first occurrence (might be zero for beginning of string)
 *          or -1 if not found
 */
int UString_getFirstPos( const char *string, const char sign );

/*!
 * \brief Returns the position of the first occurrence of a character
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns position of first occurrence (might be zero for beginning of string)
 *          or -1 if not found
 */
int UString_getFirstPosChr( const char *string, const char sign );

/*!
 * \brief [OBSOLETE] Returns the position of the last occurrence of a character
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns position of last occurrence (might be zero for beginning of string)
 *          or -1 if not found
 */
int UString_getLastPos( const char *string, const char sign );

/*!
 * \brief Returns the position of the last occurrence of a character
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns position of last occurrence (might be zero for beginning of string)
 *          or -1 if not found
 */
int UString_getLastPosChr( const char *string, const char sign );

/*!
 * \brief Find an occurrence of a character in an char string
 *
 * \param string Pointer to the char string
 * \param searchedChar Character to look for.
 * \param start Start position for the search.
 *
 * \return Position of the next occurrence, -1 if none is found.
 *
 * Note that the character at position \a start is included in the search.
 */
int UString_findNextOccurrenceChr( const char *string,
                                   char searchedChar,
                                   int start );

/*!
 * \brief Find an occurrence of a character in an char string
 *
 * \param string Pointer to a char string
 * \param searchedChar Character to look for.
 * \param start Start position for the search.
 *        (-1 starts at the end of the string.)
 *
 * \return Position of the previous occurrence, -1 if none is found.
 *
 * Note that the character at position \a start is included in the search.
 */
int UString_findPreviousOccurrenceChr( const char *string,
                                       char searchedChar,
                                       int start );

/*!
 * \brief Find the next occurrence of a char string in an char string
 *
 * \param string Pointer to a char string
 * \param searchedStr Pointer to a char string to look for
 * \param start Start position for the search.
 *        (-1 starts at the end of the string.)
 *
 * \return Position of the next occurrence, -1 if none is found.
 *
 */
int UString_findNextOccurrence( const char *string,
                                const char *searchedStr,
                                int start );

/*!
 * \brief Find the previous occurrence of a char string in an char string
 *
 * \param string Pointer to a char string
 * \param searchedStr Pointer to a char string to look for
 * \param start Start position for the search.
 *        (-1 starts at the end of the string.)
 *
 * \return Position of the previous occurrence, -1 if none is found.
 *
 */
int UString_findPreviousOccurrence( const char *string,
                                    const char *searchedStr,
                                    int start );

/*!
 * \brief [OBSOLETE] Returns the amount of a certain char within a string
 *
 * Use this function to count how many times a certain character
 * exists within a string. For example, this might be helpful to
 * explode a directory path into tokens.
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns amount of 'signs' within 'string'
 */
int UString_getOccurrences( const char *string, const char sign );

/*!
 * \brief Returns the amount of a certain char within a string
 *
 * Use this function to count how many times a certain character
 * exists within a string. For example, this might be helpful to
 * explode a directory path into tokens.
 *
 * \param string Source string
 * \param sign Character to search for
 *
 * \returns amount of 'signs' within 'string'
 */
int UString_getNumberOccurrencesChr( const char *string, const char sign );

/*!
 * \brief Returns the amount of a certain char string within a char string
 *
 * Use this function to count how many times a certain character
 * exists within a string. For example, this might be helpful to
 * explode a directory path into tokens.
 *
 * \param string Source string
 * \param searchedStr Char string to search for
 *
 * \returns amount of 'searchedStr' within 'string'
 */
int UString_getNumberOccurrences( const char *string, const char *searchedStr );

/*!
 * \brief removes leading and trailing whitespaces from a string
 *
 * e.g. "  Hello World!  " will be trimmed to "Hello World!".
 *
 * \param string Source string
 * \returns trimmed string (basically a pointer to the first
 *          non-whitespace, and the string is terminated
 *          after the last non-whitespace)
 *
 * \code
 * // to overwrite a string with the trimmed string e.g. do like this:
 *
 * strncpy( s, UString_trim( s ), strlen( s ) );
 * \endcode
 */
char *UString_trim( char *string );

/*!
 * \brief Removes only leading whitespaces from a string
 *
 * e.g. "  Hello World!  " will be trimmed to "Hello World!  ".
 *
 * \param string Source string
 * \returns trimmed string (basically a pointer to the first
 *          non-whitespace)
 *
 * \code
 * // to overwrite a string with the trimmed string e.g. do like this:
 *
 * strncpy( s, UString_trimLeft( s ), strlen( s ) );
 * \endcode
 */
char *UString_trimLeft( char *string );

/*!
 * \brief Removes only trailing whitespaces from a string
 *
 * e.g. "  Hello World!  " will be trimmed to "  Hello World!".
 *
 * \param string Source string
 * \returns trimmed string (basically a pointer to the first
 *          element, and the string is terminated after the last
 *          non-whitespace)
 *
 * \code
 * // to overwrite a string with the trimmed string e.g. do like this:
 *
 * strncpy( s, UString_trimRight( s ), strlen( s ) );
 * \endcode
 */
char *UString_trimRight( char *string );

/*!
 * \brief Get a substring of char string.
 *
 * \param destString Pointer to the destination string
 * \param destStringSize Size available into the destination string
 * \param sourceString Source string
 * \param start start position of the substring
 * \param end end position of the substring (zero-based).
 *
 * \return The substring that starts at position \a start and ends at
 *         position \a end.<br>
 *         If \a start is below zero it is automatically set to zero.
 *         If \a end is too large it is set to last postion of \a other.
 *         If destStringSize is too short so just 'destStringSize' chars are copied into destString
 */
char *UString_getSubStr( char *destString, int destStringSize, const char *sourceString, int start, int end );

/*!
 * \brief Compare two char strings (case sensitive)
 *
 * \param string1 Pointer to the first char string.
 * \param string2 Pointer to the second char string.
 *
 * \return A negative/positive value, if \a string1 is lexicographically
 *         smaller/greater than \a string2; and zero, if both strings are equal.
 */
int UString_compare( const char *string1, const char *string2 );

/*!
 * \brief Compare two char strings (case insensitive)
 *
 * \param string1 Pointer to the first char string.
 * \param string2 Pointer to the second char string.
 *
 * \return A negative/positive value, if \a string1 is lexicographically
 *         smaller/greater than \a string2; and zero, if both strings are equal.
 */

int UString_compareCase( const char *string1, const char *string2 );

int UString_compareCaseN( const char *string1, const char *string2, const int len );

int UString_getLength( char *string );

/*!
 * \brief Concatenation of a char string with another char string.
 *
 * \param destString Pointer to a char string
 * \param string Pointer to the other character string.
 * \param destStringSize Size available on destString

 * \return Appended char string
 */
char *UString_append( char *destString, int destStringSize, char *string );

/*!
 * \brief Concatenation of a char string with a single character
 *
 * \param destString Pointer to a char string
 * \param ch The character to append
 * \param size length of the string

 * \return Appended char string
 */
char *UString_appendChr( char *destString, char ch, int size );

/*!
 * \brief Write formatted output using a pointer to a list of arguments
 *
 * \param str Storage location for output.
 * \param size Maximum number of characters to write.
 * \param format Format specification.
 *
 * \return The number of characters written if the number of characters
 *         to write is less than or equal to count;
 */
int UString_snprintf( char *str, unsigned int size,
                      const char *format, ... );


ANY_END_C_DECLS


#endif


/* EOF */
