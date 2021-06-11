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


#include <PQueueArray.h>


#define PQUEUEARRAY_VALID    (0xb9b78f89)
#define PQUEUEARRAY_INVALID  (0x74120c55)


static PQueueStatus PQueueArray_initQueues( PQueueArray *self,
                                            BaseI32 arraySize,
                                            BaseI32 maxLength,
                                            const BaseChar *elementType,
                                            const BaseChar *libName );

static void PQueueArray_clearQueues( PQueueArray *self );

static PQueueStatus PQueueArray_initMutex( PQueueArray *self );

static void PQueueArray_clearMutex( PQueueArray *self );

static void PQueueArray_lock( PQueueArray *self );

static void PQueueArray_unlock( PQueueArray *self );


PQueueArray *PQueueArray_new( void )
{
    PQueueArray *self = (PQueueArray *)NULL;

    self = ANY_TALLOC( PQueueArray );

    return self;
}


PQueueStatus PQueueArray_init( PQueueArray *self,
                               BaseI32 arraySize,
                               BaseI32 maxLength,
                               const BaseChar *elementType,
                               const BaseChar *libName )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );

    if( self->valid == PQUEUEARRAY_INVALID)
    {
        self->valid = 0;
    }

    switch( self->valid )
    {
        case 0:
            Any_memset((void *)self, 0, sizeof( PQueueArray ));
            status = PQueueArray_initQueues( self,
                                             arraySize,
                                             maxLength,
                                             elementType,
                                             libName );
            if( status != PQueue_ok ) return status;
        case 1:
            status = PQueueArray_initQueues( self,
                                             arraySize,
                                             maxLength,
                                             elementType,
                                             libName );
            if( status != PQueue_ok ) return status;
        case 2:
            status = PQueueArray_initMutex( self );
            if( status != PQueue_ok ) return status;
            break;
        case PQUEUEARRAY_VALID:
            ANY_LOG( 2, "PQueueArray is already set up properly!", ANY_LOG_WARNING );
            return PQueue_alreadySetup;
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    return PQueue_ok;
}


PQueueStatus PQueueArray_setupElement( PQueueArray *self,
                                       void *data )
{
    PQueueStatus status = PQueue_ok;
    BaseI32 i = 0;
    BaseBool error = false;

    ANY_REQUIRE( self );

    PQueueArray_lock( self );

    switch( self->valid )
    {
        case 4:
            for( i = 0; i < self->arraySize; ++i )
            {
                status = PQueue_setupElement( self->queues[ i ], data );
                switch( status )
                {
                    case PQueue_ok:
                        break;
                    case PQueue_alreadySetup:
                        ANY_LOG( 5, "Queue #%i already set up.", ANY_LOG_INFO, i );
                        break;
                    default:
                        ANY_LOG( 0, "Could not set up element for queue #%i", ANY_LOG_ERROR, i );
                        error = true;
                        break;
                }
                if( error == true )
                {
                    break;
                }
            }
            if( error == true )
            {
                break;
            }
            self->valid = PQUEUEARRAY_VALID;
            break;
        case PQUEUEARRAY_VALID:
            ANY_LOG( 2, "PQueueArray is already set up properly!", ANY_LOG_WARNING );
            status = PQueue_alreadySetup;
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    PQueueArray_unlock( self );

    return status;
}


void PQueueArray_clear( PQueueArray *self )
{
    ANY_REQUIRE( self );
    if( self->valid == PQUEUEARRAY_VALID)
    {
        self->valid = 4;
    }

    switch( self->valid )
    {
        case 4:
            PQueueArray_clearMutex( self );
        case 3:
            PQueueArray_clearMutex( self );
        case 2:
            PQueueArray_clearQueues( self );
        case 1:
            PQueueArray_clearQueues( self );
            break;
        default:
            ANY_LOG( 0, "Init-state is: %i. Something's gone wrong! Aborting...", ANY_LOG_FATAL, self->valid );
            ANY_REQUIRE( 0 );
            break;
    }

    Any_memset((void *)self, 0, sizeof( PQueueArray ));

    self->valid = PQUEUEARRAY_INVALID;
}


void PQueueArray_delete( PQueueArray *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_INVALID );

    ANY_FREE( self );
}


PQueueStatus PQueueArray_pop( PQueueArray *self,
                              BaseI32 index,
                              void *data )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        status = PQueue_pop( self->queues[ index ], data );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        status = PQueue_noSuchQueue;
    }

    return status;
}


PQueueStatus PQueueArray_popWait( PQueueArray *self,
                                  BaseI32 index,
                                  void *data,
                                  BaseI32 microsecs )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        status = PQueue_popWait( self->queues[ index ], data, microsecs );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        status = PQueue_noSuchQueue;
    }

    return status;
}


PQueueStatus PQueueArray_purge( PQueueArray *self,
                                BaseI32 index,
                                void **data,
                                BaseI32 *numPurged )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        status = PQueue_purge( self->queues[ index ], data, numPurged );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        status = PQueue_noSuchQueue;
    }

    return status;
}


PQueueStatus PQueueArray_purgeWait( PQueueArray *self,
                                    BaseI32 index,
                                    void **data,
                                    BaseI32 *numPurged,
                                    BaseI32 microsecs )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        status = PQueue_purgeWait( self->queues[ index ], data, numPurged, microsecs );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        status = PQueue_noSuchQueue;
    }

    return status;
}


PQueueStatus PQueueArray_push( PQueueArray *self,
                               BaseI32 index,
                               void *data )
{
    PQueueStatus status = PQueue_ok;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        status = PQueue_push( self->queues[ index ], data );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        status = PQueue_noSuchQueue;
    }

    return status;
}


BaseI32 PQueueArray_numElements( PQueueArray *self, BaseI32 index )
{
    BaseI32 numElements = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        numElements = PQueue_numElements( self->queues[ index ] );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        numElements = -1;
    }

    return numElements;
}


BaseI32 PQueueArray_maxLength( PQueueArray *self, BaseI32 index )
{
    BaseI32 maxLength = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        maxLength = PQueue_maxLength( self->queues[ index ] );
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
        maxLength = -1;
    }

    return maxLength;
}


BaseI32 PQueueArray_getArraySize( PQueueArray *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    return self->arraySize;
}


const PQueue *PQueueArray_getQueue( PQueueArray *self, BaseI32 index )
{
    const PQueue *queue = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == PQUEUEARRAY_VALID );

    if(( index >= 0 ) && ( index < self->arraySize ))
    {
        queue = (const PQueue *)self->queues[ index ];
    }
    else
    {
        ANY_LOG( 2, "Index out of bounds!", ANY_LOG_WARNING );
    }

    return queue;
}


static PQueueStatus PQueueArray_initQueues( PQueueArray *self,
                                            BaseI32 arraySize,
                                            BaseI32 maxLength,
                                            const BaseChar *elementType,
                                            const BaseChar *libName )
{
    BaseI32 i = 0;
    PQueueStatus status = PQueue_ok;

    switch( self->valid )
    {
        case 0:
            self->arraySize = arraySize;
            self->queues = ANY_NTALLOC( arraySize, PQueue* );
            if( self->queues == NULL)
            {
                ANY_LOG( 0, "Could not allocate queues!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = 1;
            break;
        case 1:
            for( i = 0; i < self->arraySize; ++i )
            {
                self->queues[ i ] = PQueue_new();
                if( self->queues[ i ] == NULL)
                {
                    ANY_LOG( 0, "Could not allocate queue #%i!", ANY_LOG_ERROR, i );
                    return PQueue_allocMemFailed;
                }
                status = PQueue_init( self->queues[ i ], maxLength, elementType, libName );
                if( status != PQueue_ok )
                {
                    ANY_LOG( 0, "Could not init queue #%i.", ANY_LOG_ERROR, i );
                    return status;
                }
            }
            self->valid = 2;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueueArray_clearQueues( PQueueArray *self )
{
    BaseI32 i = 0;

    switch( self->valid )
    {
        case 2:
            for( i = 0; i < self->arraySize; ++i )
            {
                if( self->queues[ i ] != NULL)
                {
                    PQueue_clear( self->queues[ i ] );
                    PQueue_delete( self->queues[ i ] );
                }
            }
            self->valid = 1;
            break;
        case 1:
            ANY_FREE( self->queues );
            self->valid = 0;
            break;
        default:
            break;
    }
}


static PQueueStatus PQueueArray_initMutex( PQueueArray *self )
{
    switch( self->valid )
    {
        case 2:
            self->lock = Mutex_new();
            if( self->lock == NULL)
            {
                ANY_LOG( 0, "Unable to allocate memory for Mutex!", ANY_LOG_ERROR );
                return PQueue_allocMemFailed;
            }
            self->valid = 3;
        case 3:
            Mutex_init( self->lock, MUTEX_PRIVATE );
            self->valid = 4;
            break;
        default:
            break;
    }

    return PQueue_ok;
}


static void PQueueArray_clearMutex( PQueueArray *self )
{
    switch( self->valid )
    {
        case 4:
            Mutex_clear( self->lock );
            self->valid = 3;
        case 3:
            Mutex_delete( self->lock );
            self->valid = 2;
            break;
        default:
            break;
    }
}


static void PQueueArray_lock( PQueueArray *self )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    // no check of valid, because we need to lock at setup-time

    status = Mutex_lock( self->lock );
    ANY_REQUIRE( status == 0 );
}


static void PQueueArray_unlock( PQueueArray *self )
{
    BaseI32 status = 0;

    ANY_REQUIRE( self );
    // no check of valid, because we need to unlock at setup-time

    status = Mutex_unlock( self->lock );
    ANY_REQUIRE( status == 0 );
}


/* EOF */
