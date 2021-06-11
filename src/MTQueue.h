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


#ifndef MTQUEUE_H
#define MTQUEUE_H

#include <Any.h>
#include <Mutex.h>
#include <Cond.h>


/*!
 * \page MTQueue_About Queues (FIFO / LIFO)
 *
 * The MTQueue library implements a queue for generic elements.
 * The queue holding logic can be actually FIFO or LIFO and the user can
 * choose to make it working in a multithreaded environment or not,
 * and also he can specify a his own classification for each pushed element.
 */

#if defined(__cplusplus)
extern "C" {
#endif


#define MTQUEUE_NOCLASS       0UL


typedef enum MTQueueType
{
    MTQUEUE_FIFO = 1,
    MTQUEUE_LIFO
}
MTQueueType;


typedef unsigned long MTQueueUserClass;


typedef struct MTQueue MTQueue;


MTQueue *MTQueue_new( void );

int MTQueue_init( MTQueue *self, MTQueueType type, bool multiThread );

void MTQueue_push( MTQueue *self, void *data, MTQueueUserClass userClass );

void *MTQueue_pop( MTQueue *self, MTQueueUserClass *userClass );

void *MTQueue_popWait( MTQueue *self, MTQueueUserClass *userClass, const long microsecs );

void MTQueue_setQuit( MTQueue * self, bool status );

void MTQueue_wakeUpAll( MTQueue * self );

unsigned long MTQueue_numElements( MTQueue *self );

void MTQueue_clear( MTQueue *self );

void MTQueue_delete( MTQueue *self );

#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
