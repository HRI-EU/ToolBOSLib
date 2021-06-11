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


#ifndef MTHREADS_H
#define MTHREADS_H

#if ( _XOPEN_SOURCE - 0 ) < 600 && !defined(__windows__)
#error "_XOPEN_SOURCE must be >= 600 otherwise MThreads wont work properly"
#endif


/*!
 * \page MThreads_About Thread handling and synchronization primitives
 *
 * The MThreads.h contains:
 *
 * \li wrapper for platform-specific thread handling
 * \li coredump functions
 * \li Mutex.h: controlled access to shared resources ("critical regions")
 *              guaranteeing that only a single thread performs a certain
 *              task at a time
 * \li Barrier.h: synchronize multiple threads at a single waiting point
 * \li Cond.h: conditional variable to allow a thread to be suspended until
 *             a certain condition is met
 * \li RWLock.h: Read/Write lock
 * \li Traps.h: print stacktrace in case of error
 *
 * <h3>Example:</h3>
 * \code
 *  Threads *thread = (Threads*)NULL;
 *  bool joinable   = false;
 *  int status      = 0;
 *  void *arg       = NULL;
 *
 *  if ( Threads_init( thread, joinable ) == false )
 *  {
 *    ANY_LOG( 0, "Thread initialization failed", ANY_LOG_ERROR );
 *    ...
 *  }
 *
 *  status = Threads_start( thread, MyThreadMain, arg );
 * \endcode
 */


#include "Barrier.h"
#include "Cond.h"
#include "Mutex.h"
#include "RWLock.h"
#include "Threads.h"
#include "Traps.h"


#endif


/* EOF */
