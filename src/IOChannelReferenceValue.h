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


#ifndef IOCHANNELREFERENCEVALUE_H
#define IOCHANNELREFERENCEVALUE_H


#if defined(__cplusplus)
extern "C" {
#endif


#include <Any.h>



/*! \brief Define for string termination character "\0"
 */
#define IOCHANNELREFERENCEVALUE_EOF ( ( char )'\0' )


/*! \brief Define for sequence "://" to append at stream type name
 *
 * User doesn't need to use that, because it's needed by IOChannel private function
 *
 */
#define IOCHANNELREFERENCEVALUE_TYPESTREAMTERMINATINGSEQUENCE "://"


/*! \brief Check if character is a digit
 */
#define IOCHANNELREFERENCEVALUE_ISDIGIT( __ch )\
( ( ( __ch >= '0' ) && ( __ch <= '9') ) ? true : false )


/*! \brief Check if character is a space, a new line, a tab etc.
 */
#define IOCHANNELREFERENCEVALUE_ISSPACE( __ch )\
( ( ( __ch == ' ' )  || ( __ch == '\t' ) ||\
    ( __ch == '\r' ) || ( __ch == '\n' ) ||\
    ( __ch == '\v' ) ) ? true : false )


/*! \brief Check if character is a valid exadecimal digit
 */
#define IOCHANNELREFERENCEVALUE_ISXDIGIT( __ch )\
( ( ( ( __ch >= 'a' ) && ( __ch <= 'f' ) )\
 || ( ( __ch >= 'A' ) && ( __ch <= 'F' ) )\
 || IOCHANNELREFERENCEVALUE_ISDIGIT( __ch ) ) ? true : false )


/*! \brief Check if character is a valid octal digit
 */
#define IOCHANNELREFERENCEVALUE_ISOCTALDIGIT( __ch )\
( ( ( __ch >= '0' ) && ( __ch <= '7') ) ? true : false )


/*! \brief Check if character is a '-' or a '+' sign
 */
#define IOCHANNELREFERENCEVALUE_ISSIGN( __ch )\
( ( ( __ch == '+' ) || ( __ch == '-' ) ) ? true : false )


/*! \brief Skip all spaces from a string until next valid character that differs from a space
 */
#define IOCHANNELREFERENCEVALUE_SKIPSPACES( __string )\
do\
{\
    ANY_REQUIRE( __string );\
    while( IOCHANNELREFERENCEVALUE_ISSPACE( *__string )\
            && ( *__string != IOCHANNELREFERENCEVALUE_EOF ) )\
    {\
      __string++;\
    }\
}\
while( 0 )


/*! \brief Check if character is an alphabetical lower case character
*/
#define IOCHANNELREFERENCEVALUE_ISALPHALOWER( __ch )\
( ( ( __ch >= 'a' ) && ( __ch <= 'z' ) ) ? true : false )


/*! \brief Check if character is an alphabetical upper case character
 */
#define IOCHANNELREFERENCEVALUE_ISALPHAUPPER( __ch )\
( ( ( __ch >= 'A' ) && ( __ch <= 'Z' ) ) ? true : false )

/*! \brief Check if a character is in the admitted range of characters for openString references
 *
 * The admitted range of characters for a reference is the standard range admitted for
 * names of variables in standard C:
 *  - alphabetical character, lower or upper case ( <b>a-z</b> and <b>A-Z</b> );
 *  - numerical characters ( <b>0-9</b> );
 *  - dollar character ( <b>$</b> ).
 *  - underscore character ( <b>_</b> ).
 *  - and '+', '-' and '~'
 *
 */
#define IOCHANNELREFERENCEVALUE_ISADMITTEDREFERENCE( __ch ) \
( ( IOCHANNELREFERENCEVALUE_ISDIGIT( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHALOWER( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHAUPPER( __ch )\
 || ( __ch == '$' )\
 || ( __ch == '_' )\
 || ( __ch == '+' )\
 || ( __ch == '-' )\
 || ( __ch == '~' ) ) ? true : false )


#ifdef __windows__

#define _IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE_WIN32( __ch ) \
  ( __ch == '\\')

#else

#define _IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE_WIN32( __ch ) \
  ( true )

#endif

#define _IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE_GENERAL( __ch ) \
( ( IOCHANNELREFERENCEVALUE_ISDIGIT( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHALOWER( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHAUPPER( __ch )\
 || ( __ch == '$' ) || ( __ch == '.') || ( __ch == '/' ) || ( __ch == ':')\
 || ( __ch == '_' ) || ( __ch == '-' ) )  ? true : false )


/*! \brief Check if a character is in the admitted range of characters for openString values
 *
 * The admitted range of characters for a value is:
 *  - alphabetical character, lower or upper case ( <b>a-z</b> and <b>A-Z</b> );
 *  - numerical characters ( <b>0-9</b> );
 *  - dollar character ( <b>$</b> );
 *  - underscore character ( <b>_</b> );
 *  - dot character ( <b>.</b> );
 *  - colon character( <b>:</b>);
 *  - slash character ( <b>.</b>);
 *  - backslash character ( <b>\\</b>, only on Windows);
 *  - minus character ( <b>-</b>).
 *
 *
 */
#define IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE( __ch )\
( ( _IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE_GENERAL( __ch )\
  ||  _IOCHANNELREFERENCEVALUE_ISADMITTEDVALUE_WIN32( __ch ) ) ? true : false )


/*! \brief Get a token from a string
 */
#define IOCHANNELREFERENCEVALUE_GETTOKEN( __string, __token, __IOCHANNEL_ISADMITTEDMACRO )\
do\
{\
  const char *__tmp = NULL;\
\
  ANY_REQUIRE( __string );\
  __tmp = __string;\
  while( !( IOCHANNELREFERENCEVALUE_ISSPACE( *__string ) )\
         && ( *__string != IOCHANNELREFERENCEVALUE_EOF )\
         && ( __IOCHANNEL_ISADMITTEDMACRO( *__string ) ) )\
  {\
    __string++;\
  }\
  IOCHANNELREFERENCEVALUE_EXTRACTTOKEN( __token, __tmp, ( __string - __tmp ) );\
  ANY_REQUIRE( __token );\
}\
while( 0 )


/*! \brief Extract a token from a string
 */
#define IOCHANNELREFERENCEVALUE_EXTRACTTOKEN( __token, __startPtr, __size )\
do\
{\
  ANY_REQUIRE( __startPtr );\
  ANY_REQUIRE( __size > 0 );\
  __token = ANY_NTALLOC( __size + 1, char );\
  ANY_REQUIRE( __token );\
  Any_memmove( __token, __startPtr, __size );\
  *( __token + __size ) = IOCHANNELREFERENCEVALUE_EOF;\
}\
while( 0 )


/*! \brief Check if EOS is found in a string
 */
#define IOCHANNELREFERENCEVALUE_CONTROLEOS( __string )\
do\
{\
  ANY_REQUIRE( __string );\
  if( *__string == IOCHANNELREFERENCEVALUE_EOF )\
  {\
    ANY_LOG( 5, "Warning! Probable error in string format.", ANY_LOG_WARNING );\
  }\
}\
while( 0 )


/*! \brief Get the respective value of a reference from a vector of IOChannelReferenceValue pointers
 */
#define IOCHANNELREFERENCEVALUE_GETVALUE( __value, __vect, __reference )\
do\
{\
  IOChannelReferenceValue **__tmp = ( IOChannelReferenceValue ** )NULL;\
\
  ANY_REQUIRE( __vect );\
  ANY_REQUIRE( __reference );\
  __tmp = __vect;\
  while( *__vect )\
  {\
    if( Any_strcasecmp( __reference, ( *__vect )->reference ) == 0 )\
    {\
      __value = ( *__vect )->value;\
      break;\
    }\
    __vect++;\
  }\
  __vect = __tmp;\
}\
while( 0 )


/*! \brief Creates the association of a IOChannelMode or IOChannelPermission name and respective macro value
 */
#define IOCHANNELREFERENCEVALUE_CREATEASSOCIATION( __name )\
{ #__name, __name }


/*! \brief Make a bitwise operation between __accessFlag and __currentAccessFlag
 *
 * This macro makes a bitwise operation between __accessFlag first argument
 * and __currentAccessFlag last argument, using __operator as bitwise operator.
 * Argument __operator may assume &, | and ^ value, respectively for logical AND,
 * OR and EXOR operator, as defined in standard C for bitwise definitions.
 */
#define IOCHANNELREFERENCEVALUE_GETACCESSFLAG( __accessFlag, __operator, __currentAccessFlag )\
do\
{\
  switch( __operator )\
  {\
    case '&':\
      __accessFlag &= __currentAccessFlag;\
    break;\
\
    case '|':\
      __accessFlag |= __currentAccessFlag;\
    break;\
\
    case '^':\
      __accessFlag ^= __currentAccessFlag;\
    break;\
\
    default:\
      ANY_LOG( 5, "Error. Invalid operator", ANY_LOG_ERROR );\
      __accessFlag = EOF;\
    break;\
  }\
}\
while( 0 )


/*! \brief Check if a string contains only characters admitted by ISADMITTEDMACRO
 */
#define IOCHANNELREFERENCEVALUE_ISADMITTEDCHARCHECK( __string, __IOCHANNELREFERENCEVALUE_ISADMITTEDMACRO )\
do\
{\
  char *__tmp = __string;\
  while( *__tmp )\
  {\
    if( !__IOCHANNELREFERENCEVALUE_ISADMITTEDMACRO( *__tmp ) )\
    {\
      ANY_LOG( 5, "Error while matching string."\
                  "\nFound unadmitted '%c' value for %s function.",\
               ANY_LOG_ERROR, *__tmp, #__IOCHANNELREFERENCEVALUE_ISADMITTEDMACRO );\
      return;\
    }\
    __tmp++;\
  }\
}\
while( 0 )


/*! \brief Check if a character is in the admitted range of characters for a valid (optionally signed ) number in decimal, exadecimal, octal notation, or a valid IOChannelMode or IOChannelPermissions macro value
 */
#define IOCHANNELREFERENCEVALUE_ISADMITTED( __ch )\
( ( IOCHANNELREFERENCEVALUE_ISSIGN( __ch )\
 || IOCHANNELREFERENCEVALUE_ISDIGIT( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHALOWER( __ch )\
 || IOCHANNELREFERENCEVALUE_ISALPHAUPPER( __ch )\
 || ( __ch == '$' )\
 || ( __ch == '_' ) ) ? true : false )


/*! \brief Macro used by low-level -open to begin creation of an openString to pass to low-level -openFromString
 */
#define IOCHANNELREFERENCEVALUE_BEGINSET( __referenceValueVector )\
do\
{\
  long __size = 1024;\
  char __buffer[ 1024 /* set equal to__size */ ];\
  int __position = 0;\
  int __writtenBytes = 0;\
\
  Any_memset( __buffer, '\0', sizeof( __buffer ) );\
\
  IOCHANNELREFERENCEVALUE_ADDSET( mode, "%d", mode );\
  IOCHANNELREFERENCEVALUE_ADDSET( perm, "%d", permissions );


/*! \brief Macro used by low-level -open to end creation of an openString to pass to low-level -openFromString
 */
#define IOCHANNELREFERENCEVALUE_ENDSET( __referenceValueVector )\
  IOChannelReferenceValue_parseReferenceValue( __buffer, __referenceValueVector );\
}\
while( 0 )


/*! \brief Macro used by low-level -open to create a reference-value match for low-level -openFromString
 */
#define IOCHANNELREFERENCEVALUE_ADDSET( __reference, __pattern, __value )\
do\
{\
  __writtenBytes = Any_snprintf( __buffer + __position, __size, "%s = '" __pattern"' ", #__reference, __value );\
  if( __writtenBytes < 0 || __writtenBytes > __size )\
  {\
    ANY_LOG( 5, "Error while creating openString.", ANY_LOG_ERROR );\
    *( __buffer + 0 ) = IOCHANNELREFERENCEVALUE_EOF;\
  }\
  __size -= ( long )__writtenBytes;\
  __position += __writtenBytes;\
}\
while( 0 )


/*! \brief Macro used by low-level -open to free previously allocated vector of IOChannelReferenceValue pointers
 */
#define IOCHANNELREFERENCEVALUE_FREESET( __referenceValueVector ) \
do\
{\
  IOChannelReferenceValue_freeReferenceValueVector( __referenceValueVector );\
}\
while( 0 )


/*! \brief This macro generates a warning message if any character appears after stream type terminating sequence '://' in infoString
 *
 * Used internally by IOChannel library.
 * User doesn't need to use this macro.
 *
 */
#define IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( __infoString )\
do\
{\
  ANY_REQUIRE( __infoString );\
  if( *__infoString != IOCHANNELREFERENCEVALUE_EOF )\
  {\
    ANY_LOG( 5, "Warning, found unexpected '%c' after stream name.",\
             ANY_LOG_WARNING, *__infoString );\
  }\
}\
while( 0 )


/*! \brief Reference for stream type.
 */
#define IOCHANNELREFERENCEVALUE_STREAM    "stream"

/*! \brief Reference for stream name or command, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_NAME      "name"

/*! \brief Reference for stream access mode flags.
 */
#define IOCHANNELREFERENCEVALUE_MODE      "mode"

/*! \brief Reference for stream access permissions flags.
 */
#define IOCHANNELREFERENCEVALUE_PERM      "perm"

/*! \brief Reference for stream hostname, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_HOST      "host"

/*! \brief Reference for stream size, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_SIZE      "size"

/*! \brief Reference for stream pointer, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_POINTER   "pointer"

/*! \brief Reference for stream port, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_PORT      "port"

/*! \brief Reference for source stream port, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_SRCPORT   "srcport"

/*! \brief Reference for stream key, where necessary.
 */
#define IOCHANNELREFERENCEVALUE_KEY       "key"


/*!
  \brief IOChannelReferenceValue structure definition
 */
typedef struct IOChannelReferenceValue
{
    char *reference;
    /*!< reference */
    char *value;
    /*!< value */
    struct IOChannelReferenceValue *next;  /*!< used to implement a linked list */
} IOChannelReferenceValue;


/*!
 * \brief Insert a match of reference-value in a linked list of IOChannelReferenceValue pointers.
 */
void IOChannelReferenceValue_listItemSet( IOChannelReferenceValue **headList,
                                          char *reference,
                                          char *value );

/*!
 * \brief Create a vector of IOChannelReferenceValue pointers from a linked list.
 */
int IOChannelReferenceValue_listToVector( IOChannelReferenceValue *headList,
                                          IOChannelReferenceValue ***vect );

/*!
 * \brief Create a vector of IOChannelReferenceValue pointers from a string of reference-value matches for IOChannel_openFromString() function.
 */
int IOChannelReferenceValue_parseReferenceValue( const char *openString,
                                                 IOChannelReferenceValue ***vect );

/*!
 * \brief Get a string from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 */
char *IOChannelReferenceValue_getString( IOChannelReferenceValue **vect,
                                         char *reference );

/*!
 *  \brief Get an int from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 *
 * The function returns an <em>int</em>, parsed from a value of IOChannelReferenceValue
 * pointers vector, referred by a given reference.<br>
 * Also exadecimal and octal notations are supported.<br>
 * A check of notation correctness comes before conversion.
 */
int IOChannelReferenceValue_getInt( IOChannelReferenceValue **vect,
                                    char *reference );

/*!
 *  \brief Get an unsigned int from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 *
 * The function returns an <em>unsigned int</em>, parsed from a value of IOChannelReferenceValue
 * pointers vector, referred by a given reference.<br>
 * Also exadecimal and octal notations are supported.<br>
 * A check of notation correctness comes before conversion.
 */
unsigned int IOChannelReferenceValue_getUInt( IOChannelReferenceValue **vect,
                                              char *reference );

/*!
 *  \brief Get a long from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 *
 * The function returns an <em>long int</em>, parsed from a value of IOChannelReferenceValue
 * pointers vector, referred by a given reference.<br>
 * Also exadecimal and octal notations (optionally signed) are supported.<br>
 * A check of notation correctness comes before conversion.
 */
long int IOChannelReferenceValue_getLong( IOChannelReferenceValue **vect,
                                          char *reference );

/*!
 * \brief Get an unsigned long from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 *
 * The function returns an <em>unsigned long int</em>, parsed from a value of IOChannelReferenceValue
 * pointers vector, referred by a given reference.<br>
 * Also exadecimal and octal notations are supported.<br>
 * A check of notation correctness comes before conversion.
 */
unsigned long int IOChannelReferenceValue_getULong( IOChannelReferenceValue **vect,
                                                    char *reference );

/*!
 * \brief Get a pointer from a respective value of a reference from a vector of IOChannelReferenceValue pointers.
 */
void *IOChannelReferenceValue_getPtr( IOChannelReferenceValue **vect,
                                      char *reference );

/*! \brief Get access mode flags for IOChannel_openFromSting() function.
 *
 * This function returns a IOChannelMode from a character string that could contain
 *  - default \ref IOChannel_AccessModes defines (also combined them in ORes);
 *  - exadecimal, octal, decimal values of modes flags (also combining them in ORes, ANDs, EXORes);
 *  - a miscellaneous combination of previous cases;
 *
 * Decimal values are referred by real value, without any prefix. <br>
 * Exadecimal values are referred by sequence <b>"0x"</b> or <b>"0X"</b>
 * before value. <br>
 * Octal values are referred by character <b>"0"</b> followed by value.<br>
 * Function can manage also signed values, independently by particular notation.<br>
 * Function doesn't support parenthetical expression, so the order of expression evaluation is
 * as values are found in \e value string. For example if \e value is:
 * <center><tt>IOCHANNEL_MODE_RW & IOCHANNEL_MODE_CREAT ^ IOCHANNEL_MODE_NOTCLOSE |
 * IOCHANNEL_MODE_APPEND</tt></center><br>
 * IOChannelReferenceValue_getAccessMode returns a value evaluated as:<br><br>
 * <center><tt>((IOCHANNEL_MODE_RW & IOCHANNEL_MODE_CREAT) ^ IOCHANNEL_MODE_NOTCLOSE) |
 * IOCHANNEL_MODE_APPEND</tt></center>
 *
 */
IOChannelMode IOChannelReferenceValue_getAccessMode( char *value );

/*! \brief Get access permissions flags for IOChannel_openFromSting() function.
 *
 * This function returns a IOChannelPermissions from a character string that could contain
 *  - default \ref IOChannel_Permissions defines (also combined in ORes);
 *  - exadecimal, octal, decimal values of permissions flags (also combined in ORes, ANDs, EXORes);
 *  - a miscellaneous combination of previous cases;
 *
 * Decimal values are referred by real value, without any prefix. <br>
 * Exadecimal values are referred by sequence <b>"0x"</b> or <b>"0X"</b>
 * before value. <br>
 * Octal values are referred by character <b>"0"</b> followed by value.<br>
 * Function can manage also signed values, independently by particular notation.<br>
 * Function doesn't support parenthetical expression, so the order of expression evaluation is
 * as values are found in \e value string. For example if \e value is: <br><br>
 * <tt>IOCHANNEL_PERMISSIONS_RWX_G & IOCHANNEL_PERMISSIONS_RWX_O ^ IOCHANNEL_PERMISSIONS_RWG_U |
 * IOCHANNEL_PERMISSIONS_ALL</tt> <br><br>
 * IOChannelReferenceValue_getAccessPermissions returns a value evaluated as:<br><br>
 * <tt>((IOCHANNEL_PERMISSIONS_RWX_G & IOCHANNEL_PERMISSIONS_RWX_O) ^ IOCHANNEL_PERMISSIONS_RWG_U) |
 * IOCHANNEL_PERMISSIONS_ALL</tt>
 *
 */
IOChannelPermissions IOChannelReferenceValue_getAccessPermissions( char *value );


/*! \brief Free a vector of IOChannelReferenceValue pointers.
 */
void IOChannelReferenceValue_freeReferenceValueVector( IOChannelReferenceValue ***vect );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
