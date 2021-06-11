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


#ifndef SERIALIZEREFERENCEVALUE_H
#define SERIALIZEREFERENCEVALUE_H

/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/

#include <Any.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Data structures                                                           */
/*---------------------------------------------------------------------------*/

/**!
   \brief SerializeReferenceValue structure definition
*/
typedef struct SerializeReferenceValue
{
    char *reference;
    /**< Reference */
    char *value;
    /**< Value */
    long valueLen;
    /**< Value string length */
    long referenceLen;
    /**< Reference string length */
    struct SerializeReferenceValue *next;  /**< Used to implement a linked list */
} SerializeReferenceValue;

/*---------------------------------------------------------------------------*/
/* Defines                                                                   */
/*---------------------------------------------------------------------------*/

/*!
 * \brief Define for string termination character "\0"
 */
#define SERIALIZEREFERENCEVALUE_EOF ( ( char )'\0' )

/*!
 * \brief Check if character is a space, a new line, a tab etc.
 */
#define SERIALIZEREFERENCEVALUE_ISSPACE( __ch ) \
  ( ( ( __ch == ' ' )  || ( __ch == '\t' ) ||   \
      ( __ch == '\r' ) || ( __ch == '\n' ) ||   \
      ( __ch == '\v' ) ) ? true : false )

/*!
 * \brief Check if character is a numerical digit
 */
#define SERIALIZEREFERENCEVALUE_ISDIGIT( __ch )             \
  ( ( ( __ch >= '0' ) && ( __ch <= '9') ) ? true : false )

/*!
 * \brief Check if character is a space, a newline, a tab etc.
 */
#define SERIALIZEREFERENCEVALUE_ISSPACE( __ch ) \
  ( ( ( __ch == ' ' )  || ( __ch == '\t' ) ||   \
      ( __ch == '\r' ) || ( __ch == '\n' ) ||   \
      ( __ch == '\v' ) ) ? true : false )

/*!
 * \brief Check if character is a valid hexadecimal digit
 */
#define SERIALIZEREFERENCEVALUE_ISXDIGIT( __ch )                    \
  ( ( ( ( __ch >= 'a' ) && ( __ch <= 'f' ) )                        \
      || ( ( __ch >= 'A' ) && ( __ch <= 'F' ) )                     \
      || SERIALIZEREFERENCEVALUE_ISDIGIT( __ch ) ) ? true : false )

/*!
 * \brief Check if character is a valid octal digit
 */
#define SERIALIZEREFERENCEVALUE_ISOCTALDIGIT( __ch )        \
  ( ( ( __ch >= '0' ) && ( __ch <= '7' ) ) ? true : false )

/*!
 * \brief Check if character is a '-' or a '+' sign
 */
#define SERIALIZEREFERENCEVALUE_ISSIGN( __ch )              \
  ( ( ( __ch == '+' ) || ( __ch == '-' ) ) ? true : false )

/*!
 * \brief Check if character is an alphabetical lower case character
 */
#define SERIALIZEREFERENCEVALUE_ISALPHALOWER( __ch )        \
  ( ( ( __ch >= 'a' ) && ( __ch <= 'z' ) ) ? true : false )

/*!
 * \brief Check if character is an alphabetical upper case character
 */
#define SERIALIZEREFERENCEVALUE_ISALPHAUPPER( __ch )        \
  ( ( ( __ch >= 'A' ) && ( __ch <= 'Z' ) ) ? true : false )

/*!
 * \brief Skip all spaces from a string until next valid character that differs from a space
 */
#define SERIALIZEREFERENCEVALUE_SKIPSPACES( __string )          \
  do                                                            \
  {                                                             \
    ANY_REQUIRE( __string );                                    \
    while( SERIALIZEREFERENCEVALUE_ISSPACE( *__string )         \
           && ( *__string != SERIALIZEREFERENCEVALUE_EOF ) )    \
    {                                                           \
      __string++;                                               \
    }                                                           \
  }                                                             \
  while( 0 )

/*!
 * \brief Check if a character is in the admitted range of characters for headerString references
 *
 * The admitted range of characters for a reference is the standard range admitted for
 * names of variables in standard C:
 *  - alphabetical character, lower or upper case ( <b>a-z</b> and <b>A-Z</b> );
 *  - numerical characters ( <b>0-9</b> );
 *  - dollar character ( <b>$</b> ).
 *  - underscore character ( <b>_</b> ).
 */
#define SERIALIZEREFERENCEVALUE_ISADMITTEDREFERENCE( __ch ) \
  ( ( SERIALIZEREFERENCEVALUE_ISDIGIT( __ch )               \
      || SERIALIZEREFERENCEVALUE_ISALPHALOWER( __ch )       \
      || SERIALIZEREFERENCEVALUE_ISALPHAUPPER( __ch )       \
      || ( __ch == ':' )                                    \
      || ( __ch == '$' )                                    \
      || ( __ch == '_' ) ) ? true : false )

/*!
 * \brief Extract a string of size __size from __source and store it in __dest
 */
#define SERIALIZEREFERENCEVALUE_EXTRACTTOKEN( __source, __dest, __size ) \
  do                                                                    \
  {                                                                     \
    ANY_REQUIRE( __source );                                            \
    ANY_REQUIRE( __size > 0 );                                          \
                                                                        \
    __dest = ANY_NTALLOC( __size + 1, char );                           \
    ANY_REQUIRE( __dest );                                              \
    Any_strncpy( __dest, __source, __size );                            \
                                                                        \
    /* Ensure that the string is NULL terminated */                     \
    *( __dest + __size ) = SERIALIZEREFERENCEVALUE_EOF;                 \
  }                                                                     \
  while(0)

/*!
 * \brief Get a token from a string
 */
#define SERIALIZEREFERENCEVALUE_GETTOKEN( __source, __dest, __SERIALIZE_ISADMITTEDMACRO ) \
  do                                                                    \
  {                                                                     \
    const char *__tmp = NULL;                                           \
                                                                        \
    ANY_REQUIRE( __source );                                            \
    __tmp = __source;                                                   \
    while( !( SERIALIZEREFERENCEVALUE_ISSPACE( *__source ) )            \
           && ( *__source != SERIALIZEREFERENCEVALUE_EOF )              \
           && ( __SERIALIZE_ISADMITTEDMACRO( *__source ) ) )            \
    {                                                                   \
      __source++;                                                       \
    }                                                                   \
    SERIALIZEREFERENCEVALUE_EXTRACTTOKEN( __tmp, __dest, ( __source - __tmp ) ); \
    ANY_REQUIRE( __dest );                                              \
  }                                                                     \
  while( 0 )

/*!
 * \brief Default number of elements in SerializeHeader list
 */
#define SERIALIZEREFERENCEVALUE_DEFAULT_LIST_SIZE 5

/*!
 * \brief Default size of new Value
 */
#define SERIALIZEREFERENCEVALUE_DEFAULT_VALUE_SIZE 256

/*!
 * \brief Default size of new Reference
 */
#define SERIALIZEREFERENCEVALUE_DEFAULT_REFERENCE_SIZE 256

/*---------------------------------------------------------------------------*/
/* Public functions                                                          */
/*---------------------------------------------------------------------------*/

/**
 * \brief Allocate a new SerializeReferenceValue
 *
 * @return Pointer to new SerializeReferenceValue instance
 */
SerializeReferenceValue *SerializeReferenceValue_new( void );

/**
 * \brief Parse a suitably formatted string and populate list with
 * reference-values pairs
 *
 * This function parses a string in the format [REFERENCE = VALUE
 * [...] ], tokenizes it and stores the resulting reference-value
 * pairs in the list of SerializeReferenceValue pointed to by `list`.
 * The function makes use of a caching system: the second parameter
 * points to a list of RVPs that acts as cache of previously used RVP.
 * When reading a new reference-value from the string this function
 * will try to reuse one of the items in the cache before allocating a
 * new one.
 *
 * @param list SerializeReferenceValue list where the RVPs will be stored
 * @param cache Cache of RVPs.
 * @param listTail Pointer to last element in the destination list
 * @param headerString String to parse
 */
void SerializeReferenceValue_getRVP( SerializeReferenceValue **list, SerializeReferenceValue **cache,
                                     SerializeReferenceValue **listTail, char *headerString );

/**
 * \brief Initialize an empty SerializeReferenceValue instance with
 * the given reference and value
 *
 * This function is used when populating a valid
 * SerializeReferenceValue instance (previously allocated).
 *
 * @param self SerializeReferenceValue instance to populate
 * @param reference Reference string
 * @param value Value string
 */
void SerializeReferenceValue_init( SerializeReferenceValue *self,
                                   const char *reference,
                                   const char *value );

/**
 * \brief Update RVP with new value
 *
 * Safely updates the RVP with new reference and value. If the
 * previous value was smaller than the new one, the function will take
 * care of allocating new memory for it in order to correctly store the new value.
 *
 * @param self SerializeReferenceValue instance
 * @param reference String reference to look for in the list
 * @param value New value
 *
 */
void SerializeReferenceValue_update( SerializeReferenceValue *self, char *reference, char *value );

/**
 * \brief Search list for reference `ref`
 *
 * Look for RVP with reference `ref` and if found return it
 *
 * @param list Pointer to list to search
 * @param ref Reference to look up
 *
 * @return Pointer to RVP with reference `ref` or NULL if none was found
 */
SerializeReferenceValue *SerializeReferenceValue_findReferenceValue( SerializeReferenceValue *list, char *ref );

/**
 * \brief Find value paired with given reference in
 *
 * @param list List of SerializeReferenceValue
 * @param ref Reference to search
 *
 * @return Return a string value of the associated reference
 */
char *SerializeReferenceValue_findValue( SerializeReferenceValue *list, char *ref );

/**
 * \brief Get value stored in given SerializeReferenceValue instance
 *
 * @param self SerializeReferenceValue instance
 *
 * @return Pointer to value string.
 */
char *SerializeReferenceValue_getValue( SerializeReferenceValue *self );

/**
 * \brief Get length of value stored in instance.
 *
 * @param self SerializeReferenceValue instance
 *
 * @return Length of value string.
 */
long SerializeReferenceValue_getValueLen( SerializeReferenceValue *self );

/**
 * \brief Get length of reference stored in instance.
 *
 * @param self SerializeReferenceValue instance
 *
 * @return Length of reference string.
 */
long SerializeReferenceValue_getReferenceLen( SerializeReferenceValue *self );

/**
 * \brief Get pointer to next element in the list
 *
 * @param self SerializeReferenceValue instance
 *
 * @return Pointer to next element in the list.
 */
SerializeReferenceValue *SerializeReferenceValue_getNext( SerializeReferenceValue *self );

/**
 * \brief Get reference of given SerializeReferenceValue instance
 *
 * @param self SerializeReferenceValue instance
 *
 * @return Pointer to reference string.
 */
char *SerializeReferenceValue_getReference( SerializeReferenceValue *self );

/**
 * \brief Concatenate two lists, appending the second to the first.
 *
 * @param list Destination SerializeReferenceValue list
 * @param newElement Pointer to new element to add
 */
void SerializeReferenceValue_append( SerializeReferenceValue **list, SerializeReferenceValue *newElement );

/**
 * \brief Get first element of list
 *
 * @param list SerializeReferenceValue list
 *
 * @return Pointer to SerializeReferenceValue, NULL if no elements
 * could be taken from the list
 */
SerializeReferenceValue *SerializeReferenceValue_pop( SerializeReferenceValue **list );

/**
 * \brief Add new SerializeReferenceValue element on top of the given list
 *
 * @param self SerializeReferenceValue list
 * @param item Pointer to new element to add to the list
 */
void SerializeReferenceValue_push( SerializeReferenceValue **self, SerializeReferenceValue *item );

/**
 * \brief Free list of SerializeReferenceValues
 *
 * @param list SerializeReferenceValue list
 */
void SerializeReferenceValue_destroyList( SerializeReferenceValue *list );

/**
 * \brief Reset internal fields of SerializeReferenceValue instance.
 *
 * This function zeroes the internal variables of the given instance
 * (it does not free the memory, it merely cleans it up).
 *
 * @param self SerializeReferenceValue instance
 */
void SerializeReferenceValue_reset( SerializeReferenceValue *self );

/**
 * \brief Clear contents of a SerializeReferenceValue instance
 *
 * Free memory of internal elements and invalidate pointers.
 *
 * @param self SerializeReferenceValue instance
 */
void SerializeReferenceValue_clear( SerializeReferenceValue *self );

/**
 * \brief Delete a SerializeReferenceValue instance
 *
 * @param self SerializeReferenceValue instance
 */
void SerializeReferenceValue_delete( SerializeReferenceValue *self );

#if defined(__cplusplus)
}
#endif

#endif /* SERIALIZEREFERENCEVALUE_H */
