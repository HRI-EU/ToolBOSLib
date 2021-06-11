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


#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <Any.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define HASHTABLE_DEFAULT_LOADFACTOR    65


/*!
 * \page HashTable_About Hash tables
 *
 * HashTable.h contains an implementation of a key-value pair data
 * structure which uses a hash function for retrieving the corresponding
 * value to a search key.
 *
 * The djb2 hashing algorithm is used. This algorithm ( k=33 ) was first
 * reported by Dan Bernstein many years ago in comp.lang.c. Another version
 * of this algorithm (now favored by Bernstein) uses xor:
 *
 * \code
 * hash(i) = hash(i - 1) * 33 ^ str[i];
 * \endcode
 *
 * The magic of number 33 (why it works better than many other constants,
 * prime or not) has never been adequately explained.
 */


typedef struct HashTableEntry
{
    void *key;
    void *value;
    unsigned int hash;
    struct HashTableEntry *next;
}
HashTableEntry;


/*!
 * \brief Define a hashing function
 */
typedef unsigned int ( HashTableHashFunc )( void * );

/*!
 * \brief Define a comparison function
 */
typedef int ( HashTableEqFunc )( void *, void * );

/*!
 * \brief Define a key destructor function
 */
typedef void ( HashTableKeyDtor )( void *, void * );

/*!
 * \brief Define a value destructor function
 */
typedef void ( HashTableValueDtor )( void *, void * );

typedef struct HashTable
{
    unsigned long valid;
    unsigned int tableLength;
    HashTableEntry **table;
    unsigned int entryCount;
    unsigned int loadLimit;
    unsigned int loadFactor;
    unsigned int primeIndex;
    HashTableHashFunc *hashFunc;
    HashTableEqFunc *eqFunc;
    HashTableKeyDtor *keyDtor;
    HashTableValueDtor *valueDtor;
    void *userKeyValue;
}
HashTable;


#define HASHTABLE_FOREACH_BEGIN( __self )               \
  do                                                    \
  {                                                     \
    unsigned int __i = 0;                               \
    for( __i = 0; __i < __self->tableLength; __i++ )    \
    {                                                   \
      HashTableEntry *__current = __self->table[__i];    \
      while ( __current != NULL )                       \
      {

#define HASHTABLE_FOREACH_VALUE_PTR __current->value
#define HASHTABLE_FOREACH_KEY_PTR   __current->key

#define HASHTABLE_FOREACH_END()         \
          __current = __current->next;  \
      }                                 \
    }                                   \
  }                                     \
  while( 0 )


HashTable *HashTable_new( void );

/*!
 * \brief Create an HashTable
 *
 * \param self HashTable instance pointer
 * \param minSize Minimum initial size of HashTable
 * \param hashFunc Pointer to keys hashing function
 * \param eqFunc Pointer to function for determining key equality
 * \param keyDtor Pointer to function for key destruction
 * \param valueDtor Pointer to function for value destruction
 * \param userKeyValue Pointer to user data passed to both key and value destructors
 *
 * This function initialize an HashTable with an initial minSize buckets. The HashTable will grow
 * again once it reaches the specified percentage of load factor (see HASHTABLE_DEFAULT_LOADFACTOR).
 * It will be possible to pass an hashing function so that the user can hash any kind of data as needed
 * and also the relative hash equality function. Finally, it will be also possible to pass both key and
 * value function pointer destructors, used once the HashTable being cleared.
 *
 * \return Pointer to a newly created HashTable, NULL on failure
 */
bool HashTable_init( HashTable *self,
                     unsigned int minSize,
                     HashTableHashFunc *hashFunc,
                     HashTableEqFunc *eqFunc,
                     HashTableKeyDtor *keyDtor,
                     HashTableValueDtor *valueDtor,
                     void *userKeyValue );

/*!
 * \brief Set the percentage of load factor to trigger the HashTable resize
 *
 * \param self HashTable instance pointer
 * \param loadFactor Load factor percentage
 *
 * \return Nothing
 */
void HashTable_setLoadFactor( HashTable *self, unsigned int loadFactor );

/*!
 * \brief Return the current load factor percentage
 *
 * \param self HashTable instance pointer
 *
 * \return The current load factor percentage
 */
unsigned int HashTable_getLoadFactor( HashTable *self );

/*!
 * \brief Insert a new element in the HashTable
 *
 * \param self HashTable instance pointer
 * \param key Pointer to the key - HashTable claims ownership and will free on removal
 * \param value Pointer to the value - HashTable does not claim ownership
 *
 * This function will cause the table to expand if the insertion would take the ratio of
 * entries to table size over the maximum load factor.
 *
 * This function does not check for repeated insertions with a duplicate key.
 *
 * The value returned when using a duplicate key is undefined. When the HashTable changes size,
 * the order of retrieval of duplicate key entries is reversed. If in doubt, remove before insert.
 *
 * \return Return non-zero for successful insertion
 */
bool HashTable_insert( HashTable *self, void *key, void *value );

/*!
 * \brief Search a key in the HashTable
 *
 * \param self HashTable instance pointer
 * \param key Pointer to the key to search for  - does not claim ownership
 *
 * \return Return the value associated with the key, or NULL if the key doesn't exist
 */
void *HashTable_search( HashTable *self, void *key );

/*!
 * \brief Fetch and remove an element from the HashTable
 *
 * \param self HashTable instance pointer
 * \param key Pointer to th key to search for - does not claim ownership
 *
 * \return Return the value associated with the key, or NULL if the key doesn't exists
 */
void *HashTable_fetch( HashTable *self, void *key );

/*!
 * \brief Remove an element from the HashTable
 *
 * \param self HashTable instance pointer
 * \param key Pointer to th key to search for - does not claim ownership
 *
 * This function remove the value associated with requested key from the given HashTable. The
 * function will return the value instance pointer on success NULL otherwise. Note that if the
 * hash tables has set a value destructor it will return NULL
 *
 * \return Return the value associated with the key, or NULL if the key doesn't exists
 */
void *HashTable_remove( HashTable *self, void *key );

/*!
 * \brief Changes a value to the associated key in the HashTable
 *
 * \param self HashTable instance pointer
 * \param key Pointer to th key to search for - HashTable does not claim ownership
 * \param value Pointer to the value - HashTable does not claim ownership
 *
 * This function changes the value associated to the key
 *
 * \return Return 0 on success, not 0 on key not found
 */
int HashTable_change( HashTable *self, void *key, void *value );

/*!
 * \brief Return the number of items stored in the HashTable
 *
 * \param self HashTable instance pointer
 *
 * This function return the number of items stored in the HashTable
 *
 * \return Return the number of items stored in the hashtable
 */
unsigned int HashTable_count( HashTable *self );

unsigned int HashTable_length( HashTable *self );

/*!
 * \brief Generic hashing function
 *
 * \param key Pointer to the key
 * \param length Key length
 * \param initVal Initial value for generating hash values
 *
 * This function compute a generic hash value starting from a key with any given length. All the bits
 * are used in the computation, so that if the key differs only by 1 bit the hash will be different
 *
 * \return Return the hash value
 */
unsigned int HashTable_hashGeneric( const void *key,
                                    unsigned int length,
                                    unsigned int initVal );

/*!
 * \brief Specialize hash string calculation
 *
 * \param str Pointer to a \0 terminated string
 *
 * This function calculate the C string hash value. The C string hashing algorithm is the djb2.
 * This algorithm, using k=33 was first reported by dan bernstein many years ago in comp.lang.c.
 * Another version of this algorithm (now favored by bernstein) uses xor:
 *
 * hash(i) = hash(i - 1) * 33 ^ str[i];
 *
 * the magic of number 33 (why it works better than many other constants, prime or not) has never
 * been adequately explained.
 *
 * \return Return the C string hash value
 */
unsigned int HashTable_hashString( void *str );

void HashTable_clear( HashTable *self );

void HashTable_delete( HashTable *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
