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


#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <Any.h>
#include <Atomic.h>
#include <Barrier.h>
#include <Cond.h>
#include <MTQueue.h>
#include <Mutex.h>
#include <Threads.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef enum WorkQueueTaskStatus
{
    WORKQUEUE_TASK_SUCCESS,
    WORKQUEUE_TASK_FAILURE
} WorkQueueTaskStatus;

typedef struct WorkQueueTask WorkQueueTask;

typedef WorkQueueTaskStatus ( *WorkQueueTaskFn )( void *instance, void *userdata );

typedef void                ( *WorkQueueTaskCallback )( WorkQueueTaskStatus status, WorkQueueTask *task );

typedef struct WorkQueue       WorkQueue;
typedef struct WorkQueueWorker WorkQueueWorker;

WorkQueue *WorkQueue_new();

bool WorkQueue_init( WorkQueue *self, unsigned int minWorkers, unsigned int maxWorkers );

void WorkQueue_clear( WorkQueue *self );

void WorkQueue_delete( WorkQueue *self );

WorkQueueTask *WorkQueue_getTask( WorkQueue *self );

void WorkQueue_disposeTask( WorkQueue *self, WorkQueueTask *task );

void WorkQueue_enqueue( WorkQueue *self, WorkQueueTask *task );

bool WorkQueueTask_init( WorkQueueTask *self, WorkQueueTaskFn taskFn, void *instance,
                         void *userData, WorkQueueTaskCallback callback );


void WorkQueueTask_wait( WorkQueueTask *self );

void *WorkQueueTask_getInstance( WorkQueueTask *self );

void *WorkQueueTask_getUserData( WorkQueueTask *self );


#if defined(__cplusplus)
}
#endif

#endif /* WORKQUEUE_H */


/* EOF */
