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


#ifndef PQUEUE_H
#define PQUEUE_H

#include <Any.h>
#include <Base.h>
#include <Mutex.h>
#include <Cond.h>
#include <Serialize.h>
#include <DynamicLoader.h>


/*!
 * \page PQueue_About Persistent-storage queues (FIFO / LIFO)
 *
 * The PQueue library implements a queue for generic elements.
 * The queue holding logic can be actually FIFO or LIFO.
 * The length of the queue and the element-type have to be provided, because
 * the memory for storing the elements is allocated during init.
 * When pushed, elements are serialized to memory, when popped, they are
 * de-serialized from memory. Thus, only data-types that have a serialize-
 * function can be used.
 *
 *
 * <h3>Examples:</h3>
 *
 * \li PQueueExample1.c
 *     This example shows the usage of PQueue with a simple type,
 *     i.e. the type does not contain any pointers to external
 *     memory. Access is not concurrent.
 *
 * \li PQueueExample2.c
 *     This example shows the usage of PQueue with a complex type,
 *     i.e. the type does contain pointers to external memory.
 *     Access is not concurrent.
 *
 * \li PQueueExample3.c
 *     This example shows the usage of PQueue with a simple type,
 *     i.e. the type does not contain pointers to external memory.
 *     Access is concurrent.
 */

#if defined(__cplusplus)
extern "C" {
#endif


typedef enum PQueueStatus
{
    PQueue_retrieveDataFailed = -100,
    PQueue_storeDataFailed,
    PQueue_queueEmpty,
    PQueue_queueFull,
    PQueue_timeout,
    PQueue_allocMemFailed,
    PQueue_initMutexFailed,
    PQueue_initCondFailed,
    PQueue_initDynamicLoaderFailed,
    PQueue_initSerializeFailed,
    PQueue_loadIndirectSerializeFailed,
    PQueue_initMemChannelFailed,
    PQueue_openMemChannelFailed,
    PQueue_initCalcSizeSerializerFailed,
    PQueue_openCalcSizeSerializerFailed,
    PQueue_calcSizeFailed,
    PQueue_alreadySetup,
    PQueue_noSuchQueue,
    PQueue_ok = 0
}
PQueueStatus;


typedef enum PQueueMutexIndex
{
    PQueue_setupMutex = 0, /**< mutex for multi-threaded initialization */
    PQueue_pushMutex, /**< mutex for push (to handle multiple producers) */
    PQueue_popMutex, /**< mutex for pop (to handle multiple consumers) */
    PQueue_numMutexes
}
PQueueMutexIndex;


typedef void(*PQueueSerializeFunc)( void *, const BaseChar *, Serialize * );


typedef struct PQueue
{
    int valid;
    /**< valid flag */

    BaseI32 maxLength;
    /**< maximum number of elements in queue */

    Mutex **lock;
    /**< mutexes for init, push, pop */
    Cond *pushCond;
    /**< condition-variable for push-event */

    BaseI32 numElements;
    /**< number of elements */
    BaseI32 maxElementSize;
    /**< maximum size of an element in the queue */
    BaseI32 head;
    /**< index of head */
    BaseI32 tail;
    /**< index of tail */

    DynamicLoader *dynld;
    /**< dynamic loader to access Serialize-function */
    PQueueSerializeFunc serializeFunc;
    /**< function-pointer to serialize-function of element */
    IOChannel *memChannelWrite;
    /**< IOChannel to access memory for storing elements */
    IOChannel *memChannelRead;
    /**< IOChannel to access memory for storing elements */
    Serialize *serializeWrite;
    /**< serializer for (deep-)copying elements to memory */
    Serialize *serializeRead;
    /**< serializer for retrieving elements from memory */
    void *elementMemory;
    /**< memory for storing element-data */
    Mutex **memLock;
    /**< flags for each mem-slot showing locking status of slot */
    BaseI32 elementMemorySize;    /**< size of memory for storing element-data */
}
PQueue;

PQueue *PQueue_new( void );

/*!
 * \brief Initialize an PQueue object
 * \param self Pointer to a PQueue object
 * \param maxLength maximum length of queue
 * \param elementType name of element to be queued
 * \param libName name of library to search for elementType
 *
 * \return Return 0 on succes, -1 on failure
 *
 * Example of initialization code for a FIFO PQueue
 * \code
 *    PQueue *q = NULL;
 *
 *    q = PQueue_new();
 *    if ( !q )
 *    {
 *      ANY_LOG( 0, "Unable to allocate memory for an PQueue", ANY_LOG_FATAL );
 *      abort();
 *    }
 *
 *    if ( PQueue_init( q, 100, "MemI8", TOOLBOSLIBRARY ) )
 *    {
 *      ANY_LOG( 0, "Unable to initialize PQueue", ANY_LOG_FATAL );
 *      PQueue_delete( q );
 *      abort();
 *    }
 * \endcode
 */
PQueueStatus PQueue_init( PQueue *self,
                          BaseI32 maxLength,
                          const BaseChar *elementType,
                          const BaseChar *libName );

/*!
 * \brief Setup an element for the queue
 * \param self Pointer to a PQueue object
 * \param data pointer to a fully initialized and setup piece of data
 *
 * The setup-step consists of checking the size of the element and allocating
 * the appropriate amount of memory.
 *
 * \return Return 0 on succes, -1 on failure
 *
 * Example of initialization code for a FIFO PQueue
 * \code
 *    if ( PQueue_setupElement( queue, (void*)(&point) ) != 0 )
 *    {
 *       ANY_LOG( 0, "Unable to set up queue", ANY_LOG_FATAL );
 *       exit( 1 );
 *    }
 * \endcode
 */
PQueueStatus PQueue_setupElement( PQueue *self,
                                  void *data );

void PQueue_clear( PQueue *self );

void PQueue_delete( PQueue *self );

PQueueStatus PQueue_push( PQueue *self,
                          void *data );

PQueueStatus PQueue_pop( PQueue *self,
                         void *data );

PQueueStatus PQueue_popWait( PQueue *self,
                             void *data,
                             BaseI32 microsecs );

/*!
 * \brief Pop all elements from the queue
 * \param self Pointer to a PQueue object
 * \param data pointer to data-array (large enough to accomodate all elements of queue!)
 * \param numPurged number of elements actually purged
 * \return 0 on success, negative error-code otherwise
 *
 * This function removes all available elements from the queue and stores them in
 * the memory pointed to by data.
 */
PQueueStatus PQueue_purge( PQueue *self,
                           void **data,
                           BaseI32 *numPurged );

/*!
 * \brief Pop all elemnts from the queue after waiting for push-event
 * \param self Pointer to a PQueue object
 * \param data pointer to data-array (large enough to accomodate all elements of queue!)
 * \param numPurged number of elements actually purged
 * \param microsecs microseconds to wait for push
 * \return 0 on success, negative error-code otherwise
 *
 * This function removes all available elements from the queue and stores them in
 * the memory pointed to by data. Waits up to microsecs microseconds before doing so.
 */
PQueueStatus PQueue_purgeWait( PQueue *self,
                               void **data,
                               BaseI32 *numPurged,
                               BaseI32 microsecs );

BaseI32 PQueue_numElements( PQueue *self );

BaseI32 PQueue_maxLength( PQueue *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
