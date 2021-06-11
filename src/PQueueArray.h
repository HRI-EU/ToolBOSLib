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


#ifndef PQUEUEARRAY_H
#define PQUEUEARRAY_H


/*!
 * \page PQueueArray_About Arrays of persistent-storage queues (PQueueArrays)
 *
 * PQueueArray.h provides a simple array of PQueues. The queues can be
 * retrieved and used separately or they can be accessed with push and pop
 * by providing an index.
 *
 * PQueue works with void-pointers only. One and only one process has to take care of the
 * actual memory/object. If you e.g. have two threads, one pushes one pops, then the one
 * doing the pushing should create/destroy objects, while the popping thread should access
 * the objects read-only! To remind you of this, PQueueArray_pop returns const void* !
 */


#include <Any.h>
#include <Base.h>
#include <PQueue.h>


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct PQueueArray
{
    BaseUI32 valid;
    PQueue **queues;
    Mutex *lock;
    BaseI32 arraySize;
}
PQueueArray;

PQueueArray *PQueueArray_new( void );

/*!
 * \brief Initialize an PQueueArray object
 * \param self Pointer to an PQueueArray object
 * \param arraySize number of queues in array
 * \param maxLength maximum length of queue
 * \param elementType name of element-type
 * \param libName name of library to search for element-type
 *
 * \return Returns zero if the object is initialized correctly,
 * negative error code otherwise.
 */
PQueueStatus PQueueArray_init( PQueueArray *self,
                               BaseI32 arraySize,
                               BaseI32 maxLength,
                               const BaseChar *elementType,
                               const BaseChar *libName );

/*!
 * \brief Setup an element for the queues
 * \param self Pointer to an PQueueArray object
 * \param data pointer to a fully initialized and setup piece of data
 *
 * The setup-step consists of checking the size of the element and allocating
 * the appropriate amount of memory.
 *
 * \return Return 0 on succes, -1 on failure
 */
PQueueStatus PQueueArray_setupElement( PQueueArray *self,
                                       void *data );

void PQueueArray_clear( PQueueArray *self );

void PQueueArray_delete( PQueueArray *self );

PQueueStatus PQueueArray_pop( PQueueArray *self,
                              BaseI32 index,
                              void *data );

PQueueStatus PQueueArray_popWait( PQueueArray *self,
                                  BaseI32 index,
                                  void *data,
                                  BaseI32 microsecs );

/*!
 * \brief Pop all elements from queue
 * \param self Pointer to an PQueueArray object
 * \param index number of queue
 * \param data pointer for storing data
 * \param numPurged number of elements actually popped
 *
 * \return 0 on success, negative error-code otherwise
 *
 * This method is used to pop all elements of the queue with the number "index".
 * This returns immediately, if there are no elements in the queue.
 */
PQueueStatus PQueueArray_purge( PQueueArray *self,
                                BaseI32 index,
                                void **data,
                                BaseI32 *numPurged );

/*!
 * \brief Pop all elements from queue after waiting for push-event
 * \param self Pointer to an PQueueArray object
 * \param index number of queue
 * \param data pointer for storing data
 * \param numPurged number of elements actually popped
 * \param microsecs microseconds to wait for push-event
 *
 * \return 0 on success, negative error-code otherwise
 *
 * This method is used to pop all elements of the queue with the number "index",
 * waiting microsecs microseconds for a push-event.
 */
PQueueStatus PQueueArray_purgeWait( PQueueArray *self,
                                    BaseI32 index,
                                    void **data,
                                    BaseI32 *numPurged,
                                    BaseI32 microsecs );

PQueueStatus PQueueArray_push( PQueueArray *self,
                               BaseI32 index,
                               void *data );

/*!
 * \brief Get number of elements in queue
 * \param self Pointer to a PQueueArray object
 * \param index number of queue
 *
 * \return number of elements in queue, -1 if queue does not exist.
 *
 * This method is used to get the number of elements in
 * the queue with the number "index".
 */
BaseI32 PQueueArray_numElements( PQueueArray *self,
                                 BaseI32 index );

/*!
 * \brief Get maximum number of elements in queue
 * \param self Pointer to a PQueueArray object
 * \param index number of queue
 *
 * \return maximum number of elements in queue, -1 if queue does not exist.
 *
 * This method is used to get the maximum number of elements in
 * the queue with the number "index".
 */
BaseI32 PQueueArray_maxLength( PQueueArray *self,
                               BaseI32 index );

/*!
 * \brief Get number of queues in array.
 * \param self Pointer to a PQueueArray object
 *
 * \return number of queues in array
 */
BaseI32 PQueueArray_getArraySize( PQueueArray *self );

/*!
 * \brief Get queue from array.
 * \param self Pointer to a PQueueArray object
 * \param index number of queue
 *
 * \return pointer to queue at position "index", NULL if queue does not exist
 *
 * This method is used to get the queue with number "index".
 */
const PQueue *PQueueArray_getQueue( PQueueArray *self,
                                    BaseI32 index );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
