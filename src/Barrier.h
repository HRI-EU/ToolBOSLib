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


#ifndef BARRIER_H
#define BARRIER_H

#if ( _XOPEN_SOURCE - 0 ) < 600 && !defined(__windows__)
#error "_XOPEN_SOURCE must be >= 600 otherwise Barrier will not work properly"
#endif

#include <pthread.h>

#include <Any.h>

#define BARRIER_EINVAL    EINVAL
#define BARRIER_EAGAIN    EAGAIN
#define BARRIER_ESRCH     ESRCH
#define BARRIER_ENOSYS    ENOSYS
#define BARRIER_ENOMEM    ENOMEM
#define BARRIER_EBUSY     EBUSY
#define BARRIER_EPERM     EPERM
#define BARRIER_ETIMEDOUT ETIMEDOUT
#define BARRIER_ENOTSUP   ENOTSUP
#define BARRIER_EINTR     EINTR
#define BARRIER_EDEADLK   EDEADLK


#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * \brief The condition is process private
 *
 */
#define BARRIER_PRIVATE     0x00000001

/*!
 * \brief The condition is process shared
 *
 */
#define BARRIER_SHARED      0x00000002

/*!
 *  \brief Barrier definition
 */
typedef struct Barrier Barrier;

/*!
 * \brief Create a new barrier
 *
 * Create a new barrier. The returned pointer must be
 * cleared with \a Barrier_delete
 *
 * \return Return a new barrier pointer
 * \see Barrier_delete()
 */
Barrier *Barrier_new( void );

/*!
 * \brief Initialize a barrier
 * \param self Pointer to a barrier
 * \param flags \a BARRIER_SHARED or \a BARRIER_PRIVATE
 * \param count Number of thread in a barrier
 * \param callBack function to call
 * \param callBackArg Argument for \a func
 *
 * Must be called in order to initialize a barrier. \a func if specified
 * is called when all threads have reaced the barrier
 *
 * \return Return \a true if initialized correctly, \a false otherwise
 * \see Barrier_clear()
 */
bool Barrier_init( Barrier *self,
                   const long flags,
                   const long count,
                   void (*callBack)( void * ),
                   void *callBackArg );

/*!
 * \brief Wait on a barrier
 * \param self Pointer to a barrier
 *
 * When all the specified number of threads have called Barrier_wait()
 * the function return \a true only in one thread, on all the other return \a false.
 * If it's defined the callback function on Barrier_init() this function is called
 * before return
 *
 * \return Return \a true for the unique thread that could perform
 * \a EINTR if interrupted by a signal
 *
 */
bool Barrier_wait( Barrier *self );

/*!
 * \brief Check if the barrier is empty
 * \param self Pointer to a barrier
 *
 * Return \a true if the Barrier is empty, false otherwise
 *
 * \return Return nothing
 * \see Barrier_init()
 *
 */
bool Barrier_isEmpty( Barrier *self );

/*!
 * \brief Clean up an initialized barrier
 * \param self Pointer to a barrier
 *
 * Must be called in order to clean up a barrier
 *
 * \return Return nothing
 * \see Barrier_init()
 *
 */
void Barrier_clear( Barrier *self );

/*!
 * \brief Delete a barrier
 * \param self Pointer to a barrier
 *
 * Delete a barrier
 *
 * \return Return nothing
 * \see Barrier_new
 */
void Barrier_delete( Barrier *self );


#if defined(__cplusplus)
}
#endif


#endif  /* BARRIER_H */


/* EOF */
