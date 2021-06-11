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


#ifndef HASHTABLEITERATOR_H
#define HASHTABLEITERATOR_H

#include <Any.h>
#include <HashTable.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief The HashTableIterator
 */
typedef struct HashTableIterator
{
    unsigned long valid;
    /**< Valid flag */
    HashTable *hashTable;
    /**< Our current hash table */
    HashTableEntry *entry;
    /**< Current entry */
    HashTableEntry *parent;
    /**< Parent entry */
    unsigned int index;            /**< last entry index */
    bool initialized;      /**< interator initialization status */
}
        HashTableIterator;

/*!
 * \brief HashTableIterator allocator
 */
HashTableIterator *HashTableIterator_new( void );

/*!
 * \brief HashTableIterator constructor
 *
 * \param self HashTableIterator pointer
 * \param hashTable A valid HashTable pointer
 *
 * Initialize a new HashTableIterator for a give valid HashTable
 *
 * \return Return true on success otherwise false
 */
bool HashTableIterator_init( HashTableIterator *self, HashTable *hashTable );

/*!
 * \brief Return the pointer to key of the (key, value) pair at the current position
 *
 * \param self A valid HashTableIterator pointer
 *
 * Return the pointer to key of the (key, value) pair at the current position
 *
 * \return Return the pointer to key, NULL when at end of the HashTable
 *
 * \see HashTableIterator_getValue()
 */
void *HashTableIterator_getKey( HashTableIterator *self );

/*!
 * \brief Return the pointer to value of the (key, value) pair at the current position
 *
 * \param self A valid HashTableIterator pointer
 *
 * Return the pointer to value of the (key, value) pair at the current position
 *
 * \return Return the pointer to value, NULL when at end of the HashTable
 *
 * \see HashTableIterator_getKey()
 */
void *HashTableIterator_getValue( HashTableIterator *self );

/*!
 * \brief Set the HashTableIterator to the first element in the HashTable
 *
 * \param self A valid HashTableIterator pointer
 *
 * Set the HashTableIterator to the first element in the HashTable
 *
 * \return Nothing
 *
 * \see HashTableIterator_next()
 */
void HashTableIterator_first( HashTableIterator *self );

/*!
 * \brief Goes to the next element in the HashTable
 *
 * \param self A valid HashTableIterator pointer
 *
 * Goes to the next element in the HashTable
 *
 * \return Returns 0 when at end of the HashTable
 *
 * \see HashTableIterator_first()
 * \see HashTableIterator_getKey()
 * \see HashTableIterator_getValue()
 */
int HashTableIterator_next( HashTableIterator *self );

/*!
 * \brief Remove the curren element in the HashTable
 * \param self A valid HashTableIterator pointer
 *
 * Remove the current element in the HashTable
 *
 * \return Nothing
 */
void HashTableIterator_remove( HashTableIterator *self );

/*!
 * \brief Clear the HashTableIterator
 *
 * \param self A valid HashTableIterator pointer
 *
 * Clear the HashTableIterator
 *
 * \return Nothing
 */
void HashTableIterator_clear( HashTableIterator *self );

/*!
 * \brief Delete the HashTableIterator
 * \param self A valid HashTableIterator pointer
 *
 * Delete the HashTableIterator
 *
 * \return Nothing
 */
void HashTableIterator_delete( HashTableIterator *self );

#if defined(__cplusplus)
}
#endif

#endif /* HASHTABLEITERATOR_H */
