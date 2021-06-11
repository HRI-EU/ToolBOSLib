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


#include <Any.h>
#include <CalcSizeSerializer.h>
#include <PQueue.h>
#include <Atomic.h>

/* ISO C restricts enumerator values to range of 'int' */
#define PQUEUE_VALID      0x900d
#define PQUEUE_INVALID    0xdead

#define PQUEUE_MAX_LIBNAME_LEN 1024

typedef enum PQueueValidFlags
{
    PQueue_clean = 0,
    PQueue_lockAlloc = 1,
    PQueue_mutexNew,
    PQueue_mutexInit,
    PQueue_pushCondNew,
    PQueue_pushCondInit,
    PQueue_memLockAlloc,
    PQueue_memLockNew,
    PQueue_memLockInit,
    PQueue_dynamicLoaderNew,
    PQueue_dynamicLoaderInit,
    PQueue_getSerializeFunc,
    PQueue_memChannelWriteNew,
    PQueue_memChannelWriteInit,
    PQueue_memChannelReadNew,
    PQueue_memChannelReadInit,
    PQueue_memChannelWriteOpen,
    PQueue_memChannelReadOpen,
    PQueue_serializeWriteNew,
    PQueue_serializeWriteInit,
    PQueue_serializeReadNew,
    PQueue_serializeReadInit,
    PQueue_miscInit,
    PQueue_sizeInit,
    PQueue_elementMemoryAlloc,
    PQueue_valid = PQUEUE_VALID,
    PQueue_invalid = PQUEUE_INVALID
}
        PQueueValidFlags;


static void PQueue_lock( PQueue *self, PQueueMutexIndex index );

static void PQueue_unlock( PQueue *self, PQueueMutexIndex index );

static void PQueue_lockSlot( PQueue *self, BaseI32 slot );

static void PQueue_unlockSlot( PQueue *self, BaseI32 slot );

static BaseI32 PQueue_storeData( PQueue *self,
                                 void *data );

static BaseI32 PQueue_retrieveData( PQueue *self,
                                    void *data );

static PQueueStatus PQueue_initMutex( PQueue *self );

static void PQueue_clearMutex( PQueue *self );

static PQueueStatus PQueue_initMemLock( PQueue *self,
                                        BaseI32 maxLength );

static void PQueue_clearMemLock( PQueue *self );

static PQueueStatus PQueue_initDynamicLoader( PQueue *self,
                                              const BaseChar *elementType,
                                              const BaseChar *libName );

static void PQueue_clearDynamicLoader( PQueue *self );

static PQueueStatus PQueue_initIOChannel( PQueue *self );

static void PQueue_clearIOChannel( PQueue *self );

static PQueueStatus PQueue_initSerialize( PQueue *self );

static void PQueue_clearSerialize( PQueue *self );

static PQueueStatus PQueue_initMisc( PQueue *self );

static void PQueue_clearMisc( PQueue *self );

static PQueueStatus PQueue_initSize( PQueue *self,
                                     void *data );

static void PQueue_clearSize( PQueue *self );

static PQueueStatus PQueue_initElementMemory( PQueue *self );

static void PQueue_clearElementMemory( PQueue *self );


PQueue *PQueue_new( void )
{
    PQueue *self = ANY_TALLOC( PQueue );
    ANY_REQUIRE( self );

    self->valid = PQUEUE_INVALID;

    return self;
}


PQueueStatus PQueue_init( PQueue *self,
                          BaseI32 maxLength,
                          const BaseChar *elementType,
                          const BaseChar *libName )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    if( self->valid == PQUEUE_INVALID )
    {
        self->valid = PQueue_clean;
    }

    switch( self->valid )
    {
        case PQueue_clean:
        case PQueue_lockAlloc:
        case PQueue_mutexNew:
        case PQueue_mutexInit:
        case PQueue_pushCondNew:
            Any_memset((void *)self, 0, sizeof( PQueue ));
            status = PQueue_initMutex( self );
            if( status != PQueue_ok ) return status;
        case PQueue_pushCondInit:
        case PQueue_memLockAlloc:
        case PQueue_memLockNew:
            status = PQueue_initMemLock( self, maxLength );
            if( status != PQueue_ok ) return status;
        case PQueue_memLockInit:
        case PQueue_dynamicLoaderNew:
        case PQueue_dynamicLoaderInit:
            status = PQueue_initDynamicLoader( self, elementType, libName );
            if( status != PQueue_ok ) return status;
        case PQueue_getSerializeFunc:
        case PQueue_memChannelWriteNew:
        case PQueue_memChannelWriteInit:
        case PQueue_memChannelReadNew:
            status = PQueue_initIOChannel( self );
            if( status != PQueue_ok ) return status;
        case PQueue_memChannelReadInit:
        case PQueue_serializeWriteNew:
            status = PQueue_initSerialize( self );
            if( status != PQueue_ok ) return status;
        case PQueue_serializeReadNew:
            status = PQueue_initMisc( self );
            if( status != PQueue_ok ) return status;
            break;
        case PQueue_valid:
            ANY_LOG( 2, "PQueue is already set up properly!", ANY_LOG_WARNING );
            return PQueue_alreadySetup;
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    return PQueue_ok;
}


PQueueStatus PQueue_setupElement( PQueue *self,
                                  void *data )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_setupMutex );

    switch( self->valid )
    {
        case PQueue_miscInit:
            status = PQueue_initSize( self, data );
            if( status != PQueue_ok ) break;
        case PQueue_sizeInit:
            status = PQueue_initElementMemory( self );
            if( status != PQueue_ok ) break;
        case PQueue_elementMemoryAlloc:
        case PQueue_memChannelWriteOpen:
            status = PQueue_initIOChannel( self );
            if( status != PQueue_ok ) break;
        case PQueue_memChannelReadOpen:
        case PQueue_serializeWriteNew:
            status = PQueue_initSerialize( self );
            if( status != PQueue_ok ) break;
            break;
        case PQueue_valid:
            ANY_LOG( 2, "PQueue is already set up properly!", ANY_LOG_WARNING );
            status = PQueue_alreadySetup;
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    if( self->valid == PQueue_serializeReadInit )
    {
        self->valid = PQUEUE_VALID;
    }

    PQueue_unlock( self, PQueue_setupMutex );

    return status;
}


void PQueue_clear( PQueue *self )
{
    ANY_REQUIRE( self );

    if( self->valid == PQUEUE_VALID )
    {
        self->valid = PQueue_serializeReadInit;
    }

    switch( self->valid )
    {
        case PQueue_serializeReadInit:
        case PQueue_serializeWriteInit:
            PQueue_clearSerialize( self );
        case PQueue_memChannelReadOpen:
        case PQueue_memChannelWriteOpen:
            PQueue_clearIOChannel( self );
        case PQueue_elementMemoryAlloc:
            PQueue_clearElementMemory( self );
        case PQueue_sizeInit:
            PQueue_clearSize( self );
        case PQueue_miscInit:
            PQueue_clearMisc( self );
        case PQueue_serializeReadNew:
        case PQueue_serializeWriteNew:
            PQueue_clearSerialize( self );
        case PQueue_memChannelReadInit:
        case PQueue_memChannelReadNew:
        case PQueue_memChannelWriteInit:
        case PQueue_memChannelWriteNew:
            PQueue_clearIOChannel( self );
        case PQueue_getSerializeFunc:
        case PQueue_dynamicLoaderInit:
        case PQueue_dynamicLoaderNew:
            PQueue_clearDynamicLoader( self );
        case PQueue_memLockInit:
        case PQueue_memLockNew:
        case PQueue_memLockAlloc:
            PQueue_clearMemLock( self );
        case PQueue_pushCondInit:
        case PQueue_pushCondNew:
        case PQueue_mutexInit:
        case PQueue_mutexNew:
        case PQueue_lockAlloc:
            PQueue_clearMutex( self );
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    Any_memset((void *)self, 0, sizeof( PQueue ));

    self->valid = PQUEUE_INVALID;
}


void PQueue_delete( PQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_INVALID );

    ANY_FREE( self );
}


PQueueStatus PQueue_push( PQueue *self,
                          void *data )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_pushMutex );
    if( PQueue_numElements( self ) < self->maxLength )
    {
        PQueue_lockSlot( self, self->tail );
        status = PQueue_storeData( self, data );
        PQueue_unlockSlot( self, self->tail );
        if( status != PQueue_ok )
        {
            ANY_LOG( 0, "Could not store data!", ANY_LOG_ERROR );
        }
        else
        {
            Atomic_inc( &( self->numElements ));
            self->tail = ( self->tail + 1 ) % ( self->maxLength );
        }
    }
    else
    {
        ANY_LOG( 2, "Queue is full!", ANY_LOG_WARNING );
        status = PQueue_queueFull;
    }
    PQueue_unlock( self, PQueue_pushMutex );

    return status;
}


PQueueStatus PQueue_pop( PQueue *self,
                         void *data )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_popMutex );
    if( PQueue_numElements( self ) > 0 )
    {
        PQueue_lockSlot( self, self->head );
        status = PQueue_retrieveData( self, data );
        PQueue_unlockSlot( self, self->head );
        if( status != PQueue_ok )
        {
            ANY_LOG( 0, "Could not retrieve data!", ANY_LOG_ERROR );
        }
        else
        {
            Atomic_dec( &( self->numElements ));
            self->head = ( self->head + 1 ) % ( self->maxLength );
        }
    }
    else
    {
        ANY_LOG( 2, "Queue is empty!", ANY_LOG_WARNING );
        status = PQueue_queueEmpty;
    }
    PQueue_unlock( self, PQueue_popMutex );

    return status;
}


PQueueStatus PQueue_popWait( PQueue *self,
                             void *data,
                             BaseI32 microsecs )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_popMutex );
    Cond_wait( self->pushCond, microsecs );
    if( PQueue_numElements( self ) > 0 )
    {
        PQueue_lockSlot( self, self->head );
        status = PQueue_retrieveData( self, data );
        PQueue_unlockSlot( self, self->head );
        if( status != PQueue_ok )
        {
            ANY_LOG( 0, "Could not retrieve data!", ANY_LOG_ERROR );
        }
        else
        {
            Atomic_dec( &( self->numElements ));
            self->head = ( self->head + 1 ) % ( self->maxLength );
        }
    }
    else
    {
        ANY_LOG( 2, "Queue is empty!", ANY_LOG_WARNING );
        status = PQueue_queueEmpty;
    }
    PQueue_unlock( self, PQueue_popMutex );

    return status;
}


PQueueStatus PQueue_purge( PQueue *self,
                           void **data,
                           BaseI32 *numPurged )
{
    PQueueStatus status = PQueue_ok;
    BaseI32 numElements = 0;
    BaseI32 i = 0;
    BaseI32 elemPurged = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_popMutex );
    numElements = PQueue_numElements( self );
    if( numElements > 0 )
    {
        for( i = 0; i < numElements; ++i )
        {
            PQueue_lockSlot( self, self->head );
            status = PQueue_retrieveData( self, data[ i ] );
            PQueue_unlockSlot( self, self->head );
            if( status != PQueue_ok )
            {
                ANY_LOG( 0, "Could not retrieve data!", ANY_LOG_ERROR );
                break;
            }
            else
            {
                Atomic_dec( &( self->numElements ));
                self->head = ( self->head + 1 ) % ( self->maxLength );
                elemPurged++;
            }
        }
    }
    else
    {
        ANY_LOG( 2, "Queue is empty!", ANY_LOG_WARNING );
        status = PQueue_queueEmpty;
    }
    PQueue_unlock( self, PQueue_popMutex );

    *numPurged = elemPurged;
    return status;
}


PQueueStatus PQueue_purgeWait( PQueue *self,
                               void **data,
                               BaseI32 *numPurged,
                               BaseI32 microsecs )
{
    PQueueStatus status = PQueue_ok;
    BaseI32 numElements = 0;
    BaseI32 i = 0;
    BaseI32 elemPurged = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    PQueue_lock( self, PQueue_popMutex );
    Cond_wait( self->pushCond, microsecs );
    numElements = PQueue_numElements( self );
    if( numElements > 0 )
    {
        for( i = 0; i < numElements; ++i )
        {
            PQueue_lockSlot( self, self->head );
            status = PQueue_retrieveData( self, data[ i ] );
            PQueue_unlockSlot( self, self->head );
            if( status != PQueue_ok )
            {
                ANY_LOG( 0, "Could not retrieve data!", ANY_LOG_ERROR );
                break;
            }
            else
            {
                Atomic_dec( &( self->numElements ));
                self->head = ( self->head + 1 ) % ( self->maxLength );
                elemPurged++;
            }
        }
    }
    else
    {
        ANY_LOG( 2, "Queue is empty!", ANY_LOG_WARNING );
        status = PQueue_queueEmpty;
    }
    PQueue_unlock( self, PQueue_popMutex );

    *numPurged = elemPurged;
    return status;
}


BaseI32 PQueue_numElements( PQueue *self )
{
    BaseI32 retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );

    retVal = Atomic_get( &( self->numElements ));

    return retVal;
}


BaseI32 PQueue_maxLength( PQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );

    return self->maxLength;
}


/*
 * private functions
 */

static void PQueue_lock( PQueue *self, PQueueMutexIndex index )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    // no check of valid, because we need to lock at setup-time

    status = Mutex_lock( self->lock[ index ] );
    ANY_REQUIRE( status == 0 );
}


static void PQueue_unlock( PQueue *self, PQueueMutexIndex index )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    // no check of valid, because we need to unlock at setup-time

    status = Mutex_unlock( self->lock[ index ] );
    ANY_REQUIRE( status == 0 );
}


static void PQueue_lockSlot( PQueue *self, BaseI32 slot )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );

    ANY_REQUIRE( slot >= 0 );
    ANY_REQUIRE( slot < self->maxLength );
    status = Mutex_lock( self->memLock[ slot ] );
    ANY_REQUIRE( status == 0 );
}


static void PQueue_unlockSlot( PQueue *self, BaseI32 slot )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );

    ANY_REQUIRE( slot >= 0 );
    ANY_REQUIRE( slot < self->maxLength );
    status = Mutex_unlock( self->memLock[ slot ] );
    ANY_REQUIRE( status == 0 );
}


static BaseI32 PQueue_storeData( PQueue *self,
                                 void *data )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    if( IOChannel_seek( self->memChannelWrite,
                        (long)( self->tail * self->maxElementSize ),
                        IOCHANNELWHENCE_SET ) == -1 )
    {
        ANY_LOG( 0, "Error writing to memChannel!", ANY_LOG_ERROR );
        ANY_LOG( 0, "Error: ", IOChannel_getErrorDescription( self->memChannelWrite ));
        return PQueue_storeDataFailed;
    }
    ( self->serializeFunc )( data, "data", self->serializeWrite );

    return PQueue_ok;
}


static BaseI32 PQueue_retrieveData( PQueue *self,
                                    void *data )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUE_VALID );
    ANY_REQUIRE( data );

    if( IOChannel_seek( self->memChannelRead,
                        (long)( self->head * self->maxElementSize ),
                        IOCHANNELWHENCE_SET ) == -1 )
    {
        ANY_LOG( 0, "Error reading from memChannel!", ANY_LOG_ERROR );
        ANY_LOG( 0, "Error: ", IOChannel_getErrorDescription( self->memChannelRead ));
        return PQueue_retrieveDataFailed;
    }
    ( self->serializeFunc )( data, "data", self->serializeRead );

    return PQueue_ok;
}


static PQueueStatus PQueue_initMutex( PQueue *self )
{
    BaseI32 i = 0;

    switch( self->valid )
    {
        case PQueue_clean:
            self->lock = ANY_NTALLOC( PQueue_numMutexes, Mutex* );
            if( self->lock == NULL)
            {
                ANY_LOG( 0, "Could not allocate mutexes!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_lockAlloc;
        case PQueue_lockAlloc:
            for( i = 0; i < PQueue_numMutexes; ++i )
            {
                self->lock[ i ] = Mutex_new();
                if( self->lock[ i ] == NULL)
                {
                    ANY_LOG( 0, "Unable to allocate memory for Mutexes!", ANY_LOG_ERROR );
                    return PQueue_allocMemFailed;
                }
            }
            self->valid = PQueue_mutexNew;
        case PQueue_mutexNew:
            for( i = 0; i < PQueue_numMutexes; ++i )
            {
                /* unreachable code: Mutex_init() always returns true
                 *
                if ( Mutex_init( self->lock[i], MUTEX_PRIVATE ) == false )
                {
                   ANY_LOG( 0, "Unable to initialize Mutex!", ANY_LOG_ERROR );
                   return PQueue_initMutexFailed;
                }
                 * changed to:
                 */
                Mutex_init( self->lock[ i ], MUTEX_PRIVATE );
            }
            self->valid = PQueue_mutexInit;
        case PQueue_mutexInit:
            self->pushCond = Cond_new();
            if( self->pushCond == NULL)
            {
                ANY_LOG( 0, "Could not allocate Cond!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_pushCondNew;
        case PQueue_pushCondNew:
            /* unreachable code: Cond_init() always returns true
             *
            if ( Cond_init( self->pushCond, COND_PRIVATE ) == false )
            {
               ANY_LOG( 0, "Unable to initialize Cond!", ANY_LOG_ERROR );
               return PQueue_initCondFailed;
            }
             * changed to:
             */
            Cond_init( self->pushCond, COND_PRIVATE );
            self->valid = PQueue_pushCondInit;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearMutex( PQueue *self )
{
    BaseI32 i = 0;

    switch( self->valid )
    {
        case PQueue_pushCondInit:
            Cond_clear( self->pushCond );
            self->valid = PQueue_pushCondNew;
        case PQueue_pushCondNew:
            Cond_delete( self->pushCond );
            self->valid = PQueue_mutexInit;
        case PQueue_mutexInit:
            for( i = 0; i < PQueue_numMutexes; ++i )
            {
                Mutex_clear( self->lock[ i ] );
            }
            self->valid = PQueue_mutexNew;
        case PQueue_mutexNew:
            for( i = 0; i < PQueue_numMutexes; ++i )
            {
                Mutex_delete( self->lock[ i ] );
            }
            self->valid = PQueue_lockAlloc;
        case PQueue_lockAlloc:
            ANY_FREE( self->lock );
            self->valid = PQueue_clean;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initMemLock( PQueue *self,
                                        BaseI32 maxLength )
{
    BaseI32 i = 0;
    self->maxLength = maxLength;

    switch( self->valid )
    {
        case PQueue_pushCondInit:
            self->memLock = ANY_NTALLOC( maxLength, Mutex* );
            if( self->memLock == NULL)
            {
                ANY_LOG( 0, "Could not allocate memLock!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_memLockAlloc;
        case PQueue_memLockAlloc:
            for( i = 0; i < maxLength; ++i )
            {
                self->memLock[ i ] = Mutex_new();
                if( self->memLock[ i ] == NULL)
                {
                    ANY_LOG( 0, "Unable to allocate memory for mutex!", ANY_LOG_ERROR );
                    return PQueue_allocMemFailed;
                }
            }
            self->valid = PQueue_memLockNew;
        case PQueue_memLockNew:
            for( i = 0; i < maxLength; ++i )
            {
                /* unreachable code: Mutex_init() always returns true
                 *
                if ( Mutex_init( self->memLock[i], MUTEX_PRIVATE ) == false )
                {
                   ANY_LOG( 0, "Unable to initialize mutex!", ANY_LOG_ERROR );
                   return PQueue_initMutexFailed;
                }
                 * changed to:
                 */
                Mutex_init( self->memLock[ i ], MUTEX_PRIVATE );
            }
            self->valid = PQueue_memLockInit;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearMemLock( PQueue *self )
{
    BaseI32 i = 0;

    switch( self->valid )
    {
        case PQueue_memLockInit:
            for( i = 0; i < self->maxLength; ++i )
            {
                if( self->memLock[ i ] != NULL)
                {
                    Mutex_clear( self->memLock[ i ] );
                }
            }
            self->valid = PQueue_memLockNew;
        case PQueue_memLockNew:
            for( i = 0; i < self->maxLength; ++i )
            {
                if( self->memLock[ i ] != NULL)
                {
                    Mutex_delete( self->memLock[ i ] );
                }
            }
            self->valid = PQueue_memLockAlloc;
        case PQueue_memLockAlloc:
            ANY_FREE( self->memLock );
            self->valid = PQueue_pushCondInit;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initDynamicLoader( PQueue *self,
                                              const BaseChar *elementType,
                                              const BaseChar *libName )
{
    switch( self->valid )
    {
        case PQueue_memLockInit:
            if( libName != NULL)
            {
                self->dynld = DynamicLoader_new();
                if( self->dynld == NULL)
                {
                    ANY_LOG( 0, "Could not allocate memory for dynamic loader!", ANY_LOG_ERROR );
                    return PQueue_allocMemFailed;
                }
            }
            self->valid = PQueue_dynamicLoaderNew;
        case PQueue_dynamicLoaderNew:
            if( libName != NULL)
            {
                if( DynamicLoader_init( self->dynld, libName ) != 0 )
                {
                    ANY_LOG( 0, "Error initializing dynamic-loader.", ANY_LOG_ERROR );
                    return PQueue_initDynamicLoaderFailed;
                }
            }
            self->valid = PQueue_dynamicLoaderInit;
        case PQueue_dynamicLoaderInit:
            // dynamically load serialize-function for element-type
            self->serializeFunc = (PQueueSerializeFunc)DynamicLoader_getSymbolByClassAndMethodName( self->dynld,
                                                                                                    elementType,
                                                                                                    "indirectSerialize" );
            if( self->serializeFunc == NULL)
            {
                ANY_LOG( 0, "Could not load %s_indirectSerialize!", ANY_LOG_ERROR, elementType );
                return PQueue_loadIndirectSerializeFailed;
            }
            self->valid = PQueue_getSerializeFunc;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearDynamicLoader( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_getSerializeFunc:
            self->valid = PQueue_dynamicLoaderInit;
        case PQueue_dynamicLoaderInit:
            if( self->dynld != NULL)
            {
                DynamicLoader_clear( self->dynld );
            }
            self->valid = PQueue_dynamicLoaderNew;
        case PQueue_dynamicLoaderNew:
            if( self->dynld != NULL)
            {
                DynamicLoader_delete( self->dynld );
            }
            self->valid = PQueue_memLockInit;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initIOChannel( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_getSerializeFunc:
            self->memChannelWrite = IOChannel_new();
            if( self->memChannelWrite == NULL)
            {
                ANY_LOG( 0, "Could not allocate memory for IOChannel!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_memChannelWriteNew;
        case PQueue_memChannelWriteNew:
            /* unreachable code: IOChannel_init() always returns true
             *
            if ( IOChannel_init( self->memChannelWrite ) == false )
            {
               ANY_LOG( 0, "Could not init memChannelWrite!", ANY_LOG_ERROR );
               return PQueue_initMemChannelFailed;
            }
             * changed to:
             */
            IOChannel_init( self->memChannelWrite );
            self->valid = PQueue_memChannelWriteInit;
        case PQueue_memChannelWriteInit:
            self->memChannelRead = IOChannel_new();
            if( self->memChannelRead == NULL)
            {
                ANY_LOG( 0, "Could not allocate memory for IOChannel!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_memChannelReadNew;
        case PQueue_memChannelReadNew:
            /* unreachable code: IOChannel_init() always returns true
             *
            if ( IOChannel_init( self->memChannelRead ) == false )
            {
               ANY_LOG( 0, "Could not init memChannelRead!", ANY_LOG_ERROR );
               return PQueue_initMemChannelFailed;
            }
             * changed to:
             */
            IOChannel_init( self->memChannelRead );

            self->valid = PQueue_memChannelReadInit;
            break;
        case PQueue_elementMemoryAlloc:
            if( IOChannel_open( self->memChannelWrite,
                                "Mem://",
                                IOCHANNEL_MODE_W_ONLY,
                                IOCHANNEL_PERMISSIONS_ALL,
                                self->elementMemory,
                                self->elementMemorySize ) == false )
            {
                ANY_LOG( 0, "Could not open memChannelWrite!", ANY_LOG_ERROR );
                return PQueue_openMemChannelFailed;
            }
            self->valid = PQueue_memChannelWriteOpen;
        case PQueue_memChannelWriteOpen:
            if( IOChannel_open( self->memChannelRead,
                                "Mem://",
                                IOCHANNEL_MODE_R_ONLY,
                                IOCHANNEL_PERMISSIONS_ALL,
                                self->elementMemory,
                                self->elementMemorySize ) == false )
            {
                ANY_LOG( 0, "Could not open memChannelRead!", ANY_LOG_ERROR );
                return PQueue_openMemChannelFailed;
            }
            self->valid = PQueue_memChannelReadOpen;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearIOChannel( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_memChannelReadOpen:
            IOChannel_close( self->memChannelRead );
            self->valid = PQueue_memChannelWriteOpen;
        case PQueue_memChannelWriteOpen:
            IOChannel_close( self->memChannelWrite );
            self->valid = PQueue_elementMemoryAlloc;
            break;
        case PQueue_memChannelReadInit:
            IOChannel_clear( self->memChannelRead );
            self->valid = PQueue_memChannelReadNew;
        case PQueue_memChannelReadNew:
            IOChannel_delete( self->memChannelRead );
            self->valid = PQueue_memChannelWriteInit;
        case PQueue_memChannelWriteInit:
            IOChannel_clear( self->memChannelWrite );
            self->valid = PQueue_memChannelWriteNew;
        case PQueue_memChannelWriteNew:
            IOChannel_delete( self->memChannelWrite );
            self->valid = PQueue_getSerializeFunc;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initSerialize( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_memChannelReadInit:
            self->serializeWrite = Serialize_new();
            if( self->serializeWrite == NULL)
            {
                ANY_LOG( 0, "Could not allocate memory for Serialize!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_serializeWriteNew;
        case PQueue_serializeWriteNew:
            self->serializeRead = Serialize_new();
            if( self->serializeRead == NULL)
            {
                ANY_LOG( 0, "Could not allocate memory for Serialize!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_serializeReadNew;
            break;
        case PQueue_memChannelReadOpen:
            if( Serialize_init( self->serializeWrite,
                                self->memChannelWrite,
                                SERIALIZE_STREAMMODE_NORMAL ) == false )
            {
                ANY_LOG( 0, "Could not init SerializeWrite!", ANY_LOG_ERROR );
                return PQueue_initSerializeFailed;
            }
            Serialize_setMode( self->serializeWrite,
                               SERIALIZE_MODE_WRITE | SERIALIZE_STREAMMODE_NORMAL );
            Serialize_setFormat( self->serializeWrite, "Binary", NULL);
            self->valid = PQueue_serializeWriteInit;
        case PQueue_serializeWriteInit:
            if( Serialize_init( self->serializeRead,
                                self->memChannelRead,
                                SERIALIZE_STREAMMODE_NORMAL ) == false )
            {
                ANY_LOG( 0, "Could not init SerializeRead!", ANY_LOG_ERROR );
                return PQueue_initSerializeFailed;
            }
            Serialize_setMode( self->serializeRead, SERIALIZE_MODE_READ | SERIALIZE_STREAMMODE_NORMAL );
            Serialize_setFormat( self->serializeRead, "Binary", NULL);
            self->valid = PQueue_serializeReadInit;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearSerialize( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_serializeReadInit:
            Serialize_clear( self->serializeRead );
            self->valid = PQueue_serializeWriteInit;
        case PQueue_serializeWriteInit:
            Serialize_clear( self->serializeWrite );
            self->valid = PQueue_memChannelReadOpen;
            break;
        case PQueue_serializeReadNew:
            Serialize_delete( self->serializeRead );
            self->valid = PQueue_serializeWriteNew;
        case PQueue_serializeWriteNew:
            Serialize_delete( self->serializeWrite );
            self->valid = PQueue_memChannelReadInit;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initMisc( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_serializeReadNew:
            self->numElements = 0;
            self->head = 0;
            self->tail = 0;
            // mark elementMemorySize as invalid
            self->elementMemorySize = -1;
            self->valid = PQueue_miscInit;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearMisc( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_miscInit:
            self->valid = PQueue_serializeReadNew;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initSize( PQueue *self,
                                     void *data )
{
    CalcSizeSerializer *calcSize = NULL;
    Serialize *stream = NULL;

    switch( self->valid )
    {
        case PQueue_miscInit:
            calcSize = CalcSizeSerializer_new();
            if( calcSize == NULL)
            {
                ANY_LOG( 0, "Could not allocate memory for CalcSizeSerializer!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            if( CalcSizeSerializer_init( calcSize ) != 0 )
            {
                ANY_LOG( 0, "Could not initialize CalcSizeSerializer!", ANY_LOG_ERROR );
                CalcSizeSerializer_delete( calcSize );
                return PQueue_initCalcSizeSerializerFailed;
            }
            stream = CalcSizeSerializer_open( calcSize, "Binary" );
            if( stream == NULL)
            {
                ANY_LOG( 0, "Could not open CalcSizeSerializer!", ANY_LOG_ERROR );
                CalcSizeSerializer_clear( calcSize );
                CalcSizeSerializer_delete( calcSize );
                return PQueue_openCalcSizeSerializerFailed;
            }
            ( self->serializeFunc )( data, "data", stream );
            if( CalcSizeSerializer_isErrorOccurred( calcSize ) == false )
            {
                self->maxElementSize = CalcSizeSerializer_getTotalSize( calcSize );
                self->elementMemorySize = self->maxElementSize * self->maxLength;
            }
            else
            {
                ANY_LOG( 0, "An error occurred during serializing!", ANY_LOG_ERROR );
                CalcSizeSerializer_close( calcSize );
                CalcSizeSerializer_clear( calcSize );
                CalcSizeSerializer_delete( calcSize );
                return PQueue_calcSizeFailed;
            }
            CalcSizeSerializer_close( calcSize );
            CalcSizeSerializer_clear( calcSize );
            CalcSizeSerializer_delete( calcSize );
            self->valid = PQueue_sizeInit;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearSize( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_sizeInit:
            self->valid = PQueue_miscInit;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueue_initElementMemory( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_sizeInit:
            self->elementMemory = (void *)ANY_BALLOC( self->elementMemorySize );
            if( self->elementMemory == NULL)
            {
                ANY_LOG( 0, "Could not allocate element-memory!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = PQueue_elementMemoryAlloc;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueue_clearElementMemory( PQueue *self )
{
    switch( self->valid )
    {
        case PQueue_elementMemoryAlloc:
            ANY_FREE( self->elementMemory );
            self->valid = PQueue_sizeInit;
            break;
        default:
            break;
    }
}


/* EOF */
