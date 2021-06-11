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


/*
 * This file contains code that has been published in the Public Domain by Bob Jenkins in May 2006,
 * available here: http://www.burtleburtle.net/bob/c/lookup3.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if !defined(__windows__)

#include <sys/param.h>  /* attempt to define endianness */
# include <endian.h>    /* attempt to define endianness */

/*
 * Based on the work of Bob Jenkins, May 2006, Public Domain.
 *
 * try to guess the endian where we are running
 */
#if ( defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
     __BYTE_ORDER == __LITTLE_ENDIAN ) || \
    ( defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0
#elif ( defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN ) || \
      ( defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 1
#else
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 0
#endif

#else

# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0

#endif


#include <Any.h>
#include <HashTable.h>

#define HASHTABLE_VALID      0xea12bb4d
#define HASHTABLE_INVALID    0x0c4fd348
#define HASHTABLE_MAGIC      0xdeadbeef

#define HASHTABLE_INDEXFOR( __tableLength, __hashValue ) ( __hashValue % __tableLength );
#define HASHTABLE_ROT( x, k ) ( ( ( x ) << ( k ) ) | ( ( x ) >> ( 32 - ( k ) ) ) )


static bool HashTable_expand( HashTable *self );

static int HashTable_eqFunc( HashTable *self, void *key1, void *key2 );


/*
 * Based on the work of Bob Jenkins, May 2006, Public Domain.
 *
 * HASHTABLE_MIX() -- mix 3 32-bit values reversibly.
 *
 * This is reversible, so any information in (a,b,c) before HASHTABLE_MIX() is
 * still in (a,b,c) after HASHTABLE_MIX().
 *
 * If four pairs of (a,b,c) inputs are run through HASHTABLE_MIX(), or through
 * HASHTABLE_MIX() in reverse, there are at least 32 bits of the output that
 * are sometimes the same for one pair and different for another pair.
 * This was tested for:
 * pairs that differed by one bit, by two bits, in any combination
 * of top bits of (a,b,c), or in any combination of bottom bits of
 * (a,b,c).
 * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
 * the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
 * is commonly produced by subtraction) look like a single 1-bit
 * difference.
 * the base values were pseudorandom, all zero but one bit set, or
 * all zero plus a counter that starts at zero.
 *
 * Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
 * satisfy this are
 *
 *   4  6  8 16 19  4
 *   9 15  3 18 27 15
 *  14  9  3  7 17  3
 *
 * Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
 * for "differ" defined as + with a one-bit base and a two-bit delta.  I
 * used http://burtleburtle.net/bob/hash/avalanche.html to choose
 * the operations, constants, and arrangements of the variables.
 *
 * This does not achieve avalanche.  There are input bits of (a,b,c)
 * that fail to affect some output bits of (a,b,c), especially of a.  The
 * most thoroughly mixed value is c, but it doesn't really even achieve
 * avalanche in c.
 *
 * This allows some parallelism.  Read-after-writes are good at doubling
 * the number of bits affected, so the goal of mixing pulls in the opposite
 * direction as the goal of parallelism.  I did what I could.  Rotates
 * seem to cost as much as shifts on every machine I could lay my hands
 * on, and rotates are much kinder to the top and bottom bits, so I used
 * rotates.
 */
#define HASHTABLE_MIX( a, b, c ) \
{ \
  a -= c;  a ^= HASHTABLE_ROT(c, 4);  c += b; \
  b -= a;  b ^= HASHTABLE_ROT(a, 6);  a += c; \
  c -= b;  c ^= HASHTABLE_ROT(b, 8);  b += a; \
  a -= c;  a ^= HASHTABLE_ROT(c,16);  c += b; \
  b -= a;  b ^= HASHTABLE_ROT(a,19);  a += c; \
  c -= b;  c ^= HASHTABLE_ROT(b, 4);  b += a; \
}

/*
 * HASHTABLE_FINAL -- final mixing of 3 32-bit values (a,b,c) into c
 *
 * Pairs of (a,b,c) values differing in only a few bits will usually
 * produce values of c that look totally different.  This was tested for
 * pairs that differed by one bit, by two bits, in any combination
 * of top bits of (a,b,c), or in any combination of bottom bits of
 * (a,b,c).
 * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
 * the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
 * is commonly produced by subtraction) look like a single 1-bit
 * difference.
 * the base values were pseudorandom, all zero but one bit set, or
 * all zero plus a counter that starts at zero.
 *
 * These constants passed:
 *
 *  14 11 25 16 4 14 24
 *  12 14 25 16 4 14 24
 *
 * and these came close:
 *
 *   4  8 15 26 3 22 24
 *  10  8 15 26 3 22 24
 *  11  8 15 26 3 22 24
 */
#define HASHTABLE_FINAL( a, b, c ) \
{ \
  c ^= b; c -= HASHTABLE_ROT(b,14); \
  a ^= c; a -= HASHTABLE_ROT(c,11); \
  b ^= a; b -= HASHTABLE_ROT(a,25); \
  c ^= b; c -= HASHTABLE_ROT(b,16); \
  a ^= c; a -= HASHTABLE_ROT(c,4);  \
  b ^= a; b -= HASHTABLE_ROT(a,14); \
  c ^= b; c -= HASHTABLE_ROT(b,24); \
}


static const unsigned int HashTable_primes[] =
        {
                53, 97, 193, 389,
                769, 1543, 3079, 6151,
                12289, 24593, 49157, 98317,
                196613, 393241, 786433, 1572869,
                3145739, 6291469, 12582917, 25165843,
                50331653, 100663319, 201326611, 402653189,
                805306457, 1610612741
        };


const unsigned int HashTable_primeTableLength = sizeof( HashTable_primes ) / sizeof( HashTable_primes[ 0 ] );


HashTable *HashTable_new( void )
{
    HashTable *self = (HashTable *)NULL;

    self = ANY_TALLOC( HashTable );
    ANY_REQUIRE( self );

    return self;
}


bool HashTable_init( HashTable *self,
                     unsigned int minSize,
                     HashTableHashFunc *hashFunc,
                     HashTableEqFunc *eqFunc,
                     HashTableKeyDtor *keyDtor,
                     HashTableValueDtor *valueDtor,
                     void *userKeyValue )
{
    bool retVal = false;
    unsigned int size = HashTable_primes[ 0 ];
    unsigned int i = 0;

    ANY_REQUIRE( self );

    ANY_REQUIRE_MSG( minSize < ( 1u << 30 ), "The minimum hash size is too large" );

    /* Enforce size as prime */
    for( i = 0; i < HashTable_primeTableLength; i++ )
    {
        if( HashTable_primes[ i ] > minSize )
        {
            size = HashTable_primes[ i ];
            break;
        }
    }

    ANY_LOG( 5, "The requested hash size '%u' has been trimmed to '%u'",
             ANY_LOG_INFO, minSize, size );

    self->valid = HASHTABLE_INVALID;

    self->table = (HashTableEntry **)ANY_NTALLOC( size, HashTableEntry* );

    if( self->table == NULL)
    {
        ANY_LOG( 0, "Unable to allocate '%u' HashTableEntry", ANY_LOG_ERROR, size );
        goto out;
    }

    self->tableLength = size;
    self->loadFactor = HASHTABLE_DEFAULT_LOADFACTOR;
    self->primeIndex = i;
    self->entryCount = 0;
    self->hashFunc = hashFunc;
    self->eqFunc = eqFunc;
    self->keyDtor = keyDtor;
    self->valueDtor = valueDtor;
    self->userKeyValue = userKeyValue;
    self->loadLimit = (unsigned int)(( size * self->loadFactor ) / 100 );

    self->valid = HASHTABLE_VALID;

    retVal = true;

    out:
    return retVal;
}


void HashTable_setLoadFactor( HashTable *self, unsigned int loadFactor )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    self->loadFactor = loadFactor;
}


unsigned int HashTable_getLoadFactor( HashTable *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    return ( self->loadFactor );
}


unsigned int HashTable_hash( HashTable *self, void *key )
{
    unsigned int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );
    ANY_REQUIRE( key );

    if( self->hashFunc )
    {
        i = ( *self->hashFunc )( key );
    }
    else
    {
        i = HashTable_hashGeneric( key, sizeof( key ), 0 );
    }

    return ( i );
}


unsigned int HashTable_count( HashTable *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    return ( self->entryCount );
}


unsigned int HashTable_length( HashTable *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    return ( self->tableLength );
}


bool HashTable_insert( HashTable *self, void *key, void *value )
{
    /*
     * This method allows duplicate keys - but they shouldn't be used
     */
    bool retVal = true;
    unsigned int index = 0;
    HashTableEntry *entry = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    /* add a new element */
    self->entryCount++;

    /*
     * if we reach the load limit than we try to expand the hash table
     */
    if( self->entryCount > self->loadLimit )
    {
        /*
         * Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.
         */
        HashTable_expand( self );
    }

    entry = ANY_TALLOC( HashTableEntry );

    if( !entry )
    {
        self->entryCount--;
        retVal = false;
        goto out;
    }

    entry->hash = HashTable_hash( self, key );
    index = HASHTABLE_INDEXFOR( self->tableLength, entry->hash );
    entry->key = key;
    entry->value = value;
    entry->next = self->table[ index ];
    self->table[ index ] = entry;

    out:
    return retVal;
}


void *HashTable_search( HashTable *self, void *key )
{
    void *retVal = NULL;
    HashTableEntry *entry = NULL;
    unsigned int hashValue = 0;
    unsigned int index = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    hashValue = HashTable_hash( self, key );
    index = HASHTABLE_INDEXFOR( self->tableLength, hashValue );
    entry = self->table[ index ];

    /* search in the entry */
    while( entry != NULL)
    {
        /* Check hash value to short circuit heavier comparison */
        if(( hashValue == entry->hash ) && ( HashTable_eqFunc( self, key, entry->key )))
        {
            retVal = entry->value;
            break;
        }
        entry = entry->next;
    }

    return retVal;
}


void *HashTable_fetch( HashTable *self, void *key )
{
    void *retVal = NULL;
    HashTableEntry *entry = NULL;
    HashTableEntry **pEntry = NULL;
    void *value = NULL;
    unsigned int hashValue = 0;
    unsigned int index = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    hashValue = HashTable_hash( self, key );
    index = HASHTABLE_INDEXFOR( self->tableLength, HashTable_hash( self, key ));

    pEntry = &self->table[ index ];

    entry = *pEntry;

    while( entry )
    {
        /* Check hash value to short circuit heavier comparison */
        if(( hashValue == entry->hash ) && ( HashTable_eqFunc( self, key, entry->key )))
        {
            *pEntry = entry->next;
            self->entryCount--;
            value = entry->value;

            if( self->keyDtor )
            {
                ( *self->keyDtor )( self->userKeyValue, entry->key );
            }

            ANY_FREE( entry );
            retVal = value;
            break;
        }

        pEntry = &entry->next;
        entry = entry->next;
    }

    return retVal;
}


void *HashTable_remove( HashTable *self, void *key )
{
    void *retVal = NULL;
    HashTableEntry *entry = NULL;
    HashTableEntry **pEntry = NULL;
    void *value = NULL;
    unsigned int hashValue = 0;
    unsigned int index = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    hashValue = HashTable_hash( self, key );
    index = HASHTABLE_INDEXFOR( self->tableLength, HashTable_hash( self, key ));

    pEntry = &self->table[ index ];

    entry = *pEntry;

    while( entry )
    {
        /* Check hash value to short circuit heavier comparison */
        if(( hashValue == entry->hash ) && ( HashTable_eqFunc( self, key, entry->key )))
        {
            *pEntry = entry->next;
            self->entryCount--;
            value = entry->value;

            if( self->keyDtor )
            {
                ( *self->keyDtor )( self->userKeyValue, entry->key );
            }

            if( self->valueDtor )
            {
                ( *self->valueDtor )( self->userKeyValue, entry->value );
                value = NULL;
            }

            ANY_FREE( entry );
            retVal = value;
            break;
        }

        pEntry = &entry->next;
        entry = entry->next;
    }

    return retVal;
}


int HashTable_change( HashTable *self, void *key, void *value )
{
    HashTableEntry *entry = NULL;
    unsigned int hashValue = 0;
    unsigned int index = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    hashValue = HashTable_hash( self, key );
    index = HASHTABLE_INDEXFOR( self->tableLength, hashValue );
    entry = self->table[ index ];

    while( entry )
    {
        /* Check hash value to short circuit heavier comparison */
        if(( hashValue == entry->hash ) && ( HashTable_eqFunc( self, key, entry->key )))
        {
            entry->value = value;
            return ( -1 );
        }
        entry = entry->next;
    }

    return ( 0 );
}


void HashTable_clear( HashTable *self )
{
    unsigned int i = 0;
    HashTableEntry *entry = NULL;
    HashTableEntry *currEntry = NULL;
    HashTableEntry **table = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    table = self->table;

    for( i = 0; i < self->tableLength; i++ )
    {
        entry = table[ i ];

        while( entry )
        {
            currEntry = entry;
            entry = entry->next;

            if( self->keyDtor )
            {
                ( *self->keyDtor )( self->userKeyValue, currEntry->key );
            }

            if( self->valueDtor )
            {
                ( *self->valueDtor )( self->userKeyValue, currEntry->value );
            }

            ANY_FREE( currEntry );
        }
    }

    ANY_FREE( self->table );
    self->table = NULL;

    self->valid = HASHTABLE_INVALID;
}


void HashTable_delete( HashTable *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static int HashTable_eqFunc( HashTable *self, void *key1, void *key2 )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    return ( self->eqFunc ? ( *self->eqFunc )( key1, key2 ) : ( key1 == key2 ));
}


static bool HashTable_expand( HashTable *self )
{
    bool retVal = true;
    HashTableEntry **newTable = NULL;
    HashTableEntry *entry = NULL;
    unsigned int newSize = 0;
    unsigned int i = 0;
    unsigned index = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == HASHTABLE_VALID );

    /*
     * Check if we are at max capacity and in case just returns
     */
    if( self->primeIndex == ( HashTable_primeTableLength - 1 ))
    {
        ANY_LOG( 5, "The Hash Table has reached the max size", ANY_LOG_WARNING );
        return 0;
    }

    /* get its size */
    newSize = HashTable_primes[ ( self->primeIndex + 1 ) ];

    ANY_LOG( 5, "Expanding the Hash Table from '%d' to '%d' elements",
             ANY_LOG_INFO, self->tableLength, newSize );

    newTable = (HashTableEntry **)ANY_NTALLOC( newSize, HashTableEntry );

    if( newTable )
    {
        /*
         * This algorithm reverses the list when it transfers
         * entries between the tables while expanding the hash table
         */
        for( i = 0; i < self->tableLength; i++ )
        {
            while(( entry = self->table[ i ] ) != NULL)
            {
                self->table[ i ] = entry->next;
                index = HASHTABLE_INDEXFOR( newSize, entry->hash );
                entry->next = newTable[ index ];
                newTable[ index ] = entry;
            }
        }

        ANY_FREE( self->table );
        self->table = newTable;
    }
    else
    {
        ANY_LOG( 5, "The Hash Table cannot be expanded because out of memory, I'll try it later again",
                 ANY_LOG_WARNING );
        retVal = false;
        goto out;
    }

    self->primeIndex++;
    self->tableLength = newSize;

    /*
     * recalculate the new load limit looking at the setted load factor
     */
    self->loadLimit = (unsigned int)(( newSize * self->loadFactor ) / 100 );

    out:
    return retVal;
}


/*
 * HashTable_hashGeneric() -- hash a variable-length key into a 32-bit value
 * k       : the key (the unaligned variable-length array of bytes)
 * length  : the length of the key, counting by bytes
 * initval : can be any 4-byte value
 *
 * Returns a 32-bit value.  Every bit of the key affects every bit of
 * the return value.  Two keys differing by one or two bits will have
 * totally different hash values.
 *
 * If you are hashing n strings (unsigned char **)k, do it like this:
 * for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);
 *
 * Use for hash table lookup, or anything where one collision in 2^^32 is
 * acceptable.
 *
 *
 * Based on the work of Bob Jenkins, May 2006, Public Domain.
 */
unsigned int HashTable_hashGeneric( const void *key, unsigned int length, unsigned int initVal )
{
    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    union
    {
        const void *ptr;
        size_t i;
    } u;

    /*
     * Set up the internal state
     */
    a = b = c = HASHTABLE_MAGIC + ((unsigned int)length ) + initVal;

    u.ptr = key;

    if( HASH_LITTLE_ENDIAN && (( u.i & 0x3 ) == 0 ))
    {
        const unsigned int *k = (const unsigned int *)key;         /* read 32-bit chunks */

        /* all but last block: aligned reads and affect 32 bits of (a,b,c) */
        while( length > 12 )
        {
            a += k[ 0 ];
            b += k[ 1 ];
            c += k[ 2 ];

            HASHTABLE_MIX( a, b, c );

            length -= 12;
            k += 3;
        }

        /*
         * handle the last (probably partial) block
         */
        /*
         * "k[2]&0xffffff" actually reads beyond the end of the string, but
         * then masks off the part it's not allowed to read.  Because the
         * string is aligned, the masked-off tail is in the same word as the
         * rest of the string.  Every machine with memory protection I've seen
         * does it on word boundaries, so is OK with this. The masking trick
         * does make the hash noticably faster for short strings (like English words).
         */
        switch( length )
        {
            case 12:
                c += k[ 2 ];
                b += k[ 1 ];
                a += k[ 0 ];
                break;

            case 11:
                c += k[ 2 ] & 0xffffff;
                b += k[ 1 ];
                a += k[ 0 ];
                break;

            case 10:
                c += k[ 2 ] & 0xffff;
                b += k[ 1 ];
                a += k[ 0 ];
                break;

            case 9 :
                c += k[ 2 ] & 0xff;
                b += k[ 1 ];
                a += k[ 0 ];
                break;

            case 8 :
                b += k[ 1 ];
                a += k[ 0 ];
                break;

            case 7 :
                b += k[ 1 ] & 0xffffff;
                a += k[ 0 ];
                break;

            case 6 :
                b += k[ 1 ] & 0xffff;
                a += k[ 0 ];
                break;

            case 5 :
                b += k[ 1 ] & 0xff;
                a += k[ 0 ];
                break;

            case 4 :
                a += k[ 0 ];
                break;

            case 3 :
                a += k[ 0 ] & 0xffffff;
                break;

            case 2 :
                a += k[ 0 ] & 0xffff;
                break;

            case 1 :
                a += k[ 0 ] & 0xff;
                break;

            case 0 :
                return c;              /* zero length strings require no mixing */
        }

    }
    else if( HASH_LITTLE_ENDIAN && (( u.i & 0x1 ) == 0 ))
    {
        const unsigned short *k = (const unsigned short *)key;         /* read 16-bit chunks */
        const unsigned char *k8;

        /*
         * all but last block: aligned reads and different mixing
         */
        while( length > 12 )
        {
            a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
            b += k[ 2 ] + (((unsigned int)k[ 3 ] ) << 16 );
            c += k[ 4 ] + (((unsigned int)k[ 5 ] ) << 16 );

            HASHTABLE_MIX( a, b, c );

            length -= 12;
            k += 6;
        }

        /*
         * handle the last (probably partial) block
         */
        k8 = (const unsigned char *)k;

        switch( length )
        {
            case 12:
                c += k[ 4 ] + (((unsigned int)k[ 5 ] ) << 16 );
                b += k[ 2 ] + (((unsigned int)k[ 3 ] ) << 16 );
                a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
                break;

            case 11:
                c += ((unsigned int)k8[ 10 ] ) << 16;     /* fall through */
            case 10:
                c += k[ 4 ];
                b += k[ 2 ] + (((unsigned int)k[ 3 ] ) << 16 );
                a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
                break;

            case 9 :
                c += k8[ 8 ];                      /* fall through */
            case 8 :
                b += k[ 2 ] + (((unsigned int)k[ 3 ] ) << 16 );
                a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
                break;

            case 7 :
                b += ((unsigned int)k8[ 6 ] ) << 16;      /* fall through */
            case 6 :
                b += k[ 2 ];
                a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
                break;

            case 5 :
                b += k8[ 4 ];                      /* fall through */
            case 4 :
                a += k[ 0 ] + (((unsigned int)k[ 1 ] ) << 16 );
                break;

            case 3 :
                a += ((unsigned int)k8[ 2 ] ) << 16;      /* fall through */
            case 2 :
                a += k[ 0 ];
                break;

            case 1 :
                a += k8[ 0 ];
                break;

            case 0 :
                return c;                     /* zero length requires no mixing */
        }

    }
    else
    {
        /*
         * need to read the key one byte at a time
         */
        const unsigned char *k = (const unsigned char *)key;

        /*
         * all but the last block: affect some 32 bits of (a,b,c)
         */
        while( length > 12 )
        {
            a += k[ 0 ];
            a += ((unsigned int)k[ 1 ] ) << 8;
            a += ((unsigned int)k[ 2 ] ) << 16;
            a += ((unsigned int)k[ 3 ] ) << 24;
            b += k[ 4 ];
            b += ((unsigned int)k[ 5 ] ) << 8;
            b += ((unsigned int)k[ 6 ] ) << 16;
            b += ((unsigned int)k[ 7 ] ) << 24;
            c += k[ 8 ];
            c += ((unsigned int)k[ 9 ] ) << 8;
            c += ((unsigned int)k[ 10 ] ) << 16;
            c += ((unsigned int)k[ 11 ] ) << 24;

            HASHTABLE_MIX( a, b, c );

            length -= 12;
            k += 12;
        }

        /*
         * last block: affect all 32 bits of (c)
         */
        switch( length )                   /* all the case statements fall through */
        {
            case 12:
                c += ((unsigned int)k[ 11 ] ) << 24;
            case 11:
                c += ((unsigned int)k[ 10 ] ) << 16;
            case 10:
                c += ((unsigned int)k[ 9 ] ) << 8;
            case 9 :
                c += k[ 8 ];
            case 8 :
                b += ((unsigned int)k[ 7 ] ) << 24;
            case 7 :
                b += ((unsigned int)k[ 6 ] ) << 16;
            case 6 :
                b += ((unsigned int)k[ 5 ] ) << 8;
            case 5 :
                b += k[ 4 ];
            case 4 :
                a += ((unsigned int)k[ 3 ] ) << 24;
            case 3 :
                a += ((unsigned int)k[ 2 ] ) << 16;
            case 2 :
                a += ((unsigned int)k[ 1 ] ) << 8;
            case 1 :
                a += k[ 0 ];
                break;

            case 0 :
                return c;
        }
    }

    HASHTABLE_FINAL( a, b, c );

    return c;
}

/*
 * The C string hashing algorithm is the djb2. This algorithm ( k=33 ) was first reported
 * by Dan Bernstein many years ago in comp.lang.c. Another version of this algorithm
 * (now favored by Bernstein) uses xor:
 *
 * hash(i) = hash(i - 1) * 33 ^ str[i];
 *
 * The magic of number 33 (why it works better than many other constants,
 * prime or not) has never been adequately explained.
 */
#define HASHTABLE_START_HASHSTRING_FACTOR 5381


unsigned int HashTable_hashString( void *ptr )
{
    const char *str = (const char *)ptr;
    unsigned long hash = HASHTABLE_START_HASHSTRING_FACTOR;
    int c;

    ANY_REQUIRE( ptr );

    while(( c = *str++ ))
    {
        hash = (( hash << 5 ) + hash ) + c;    /* hash * 33 + c */
    }

    return hash;
}


/* EOF */
