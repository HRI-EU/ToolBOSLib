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

#include <Any.h>
#include <HashTableIterator.h>

/*
 * Validation flag
 */
#define HASHTABLEITERATOR_VALID      0x5fda75e1
#define HASHTABLEITERATOR_INVALID    0xafab5a3b


HashTableIterator *HashTableIterator_new( void )
{
    HashTableIterator *self = (HashTableIterator *)NULL;

    self = ANY_TALLOC( HashTableIterator );
    ANY_REQUIRE( self );

    return self;
}


bool HashTableIterator_init( HashTableIterator *self, HashTable *hashTable )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( hashTable );

    self->valid = HASHTABLEITERATOR_INVALID;

    self->hashTable = hashTable;
    self->entry = NULL;
    self->parent = NULL;
    self->index = 0;
    self->initialized = false;

    self->valid = HASHTABLEITERATOR_VALID;
    retVal = true;

    return retVal;
}


void HashTableIterator_first( HashTableIterator *self )
{
    unsigned int i = 0;
    unsigned int tableLength = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );
    ANY_REQUIRE( self->hashTable );

    tableLength = HashTable_length( self->hashTable );
    self->index = tableLength;
    self->entry = NULL;
    self->parent = NULL;
    self->initialized = false;

    if( HashTable_count( self->hashTable ))
    {
        for( i = 0; i < tableLength; i++ )
        {
            if( self->hashTable->table[ i ] )
            {
                self->entry = self->hashTable->table[ i ];
                self->index = i;
                break;
            }
        }
        self->initialized = true;
    }
}


void *HashTableIterator_getKey( HashTableIterator *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );

    return (( self->initialized == true ? self->entry->key : NULL));
}


void *HashTableIterator_getValue( HashTableIterator *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );

    return (( self->initialized == true ? self->entry->value : NULL));
}


int HashTableIterator_next( HashTableIterator *self )
{
    unsigned int j = 0;
    unsigned int tableLength = 0;
    HashTableEntry **table = NULL;
    HashTableEntry *next = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );

    if( self->initialized == false )
    {
        HashTableIterator_first( self );
        return ( self->entry ? -1 : 0 );
    }

    if( self->entry == NULL)
    {
        return ( 0 );
    }

    next = self->entry->next;

    if( next != NULL)
    {
        self->parent = self->entry;
        self->entry = next;
        return ( -1 );
    }

    tableLength = HashTable_length( self->hashTable );

    self->parent = NULL;
    if( tableLength <= ( j = ++self->index ))
    {
        self->entry = NULL;
        return ( 0 );
    }

    table = self->hashTable->table;

    while(( next = table[ j ] ) == NULL)
    {
        if( ++j >= tableLength )
        {
            self->index = tableLength;
            self->entry = NULL;
            return ( 0 );
        }
    }

    self->index = j;
    self->entry = next;

    return ( -1 );
}


void HashTableIterator_remove( HashTableIterator *self )
{
    HashTableEntry *remember_e = NULL;
    HashTableEntry *remember_parent = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );

    /* Do the removal */
    if( self->parent == NULL)
    {
        /* element is head of a chain */
        self->hashTable->table[ self->index ] = self->entry->next;
    }
    else
    {
        /* element is mid-chain */
        self->parent->next = self->entry->next;
    }

    /* itr->entry is now outside the hashtable */
    remember_e = self->entry;
    self->hashTable->entryCount--;

    /* Advance the iterator, correcting the parent */
    remember_parent = self->parent;
    HashTableIterator_next( self );

    if( self->parent == remember_e )
    {
        self->parent = remember_parent;
    }

    ANY_FREE( remember_e );
}


void HashTableIterator_clear( HashTableIterator *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLEITERATOR_VALID );

    self->valid = HASHTABLEITERATOR_INVALID;

    self->hashTable = NULL;
    self->entry = NULL;
    self->parent = NULL;
    self->index = -1;
}


void HashTableIterator_delete( HashTableIterator *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}

