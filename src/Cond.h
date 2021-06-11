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


#ifndef COND_H
#define COND_H

#if ( _XOPEN_SOURCE - 0 ) < 600 && !defined(__windows__)
#error "_XOPEN_SOURCE must be >= 600 otherwise Cond will not work properly"
#endif

#include <pthread.h>
#include <errno.h>


#include <Any.h>
#include <Base.h>
#include <Mutex.h>

#define COND_EINVAL    EINVAL
#define COND_EAGAIN    EAGAIN
#define COND_ESRCH     ESRCH
#define COND_ENOSYS    ENOSYS
#define COND_ENOMEM    ENOMEM
#define COND_EBUSY     EBUSY
#define COND_EPERM     EPERM
#define COND_ETIMEDOUT ETIMEDOUT
#define COND_ENOTSUP   ENOTSUP
#define COND_EINTR     EINTR
#define COND_EDEADLK   EDEADLK


#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief The condition is process private
 *
 */
#define COND_PRIVATE   0x00000001

/*!
 * \brief The condition is process shared
 *
 */
#define COND_SHARED   0x00000002

/*!
 *  \brief Condition definition
 */
typedef struct Cond
{
    unsigned long valid;
    /**< object validity */
    pthread_mutex_t mutex;
    /**< mutex needed by the condition */
    pthread_mutexattr_t mutexattr;
    /**< mutex's attribute */
    pthread_cond_t cond;
    /**< condition */
    pthread_condattr_t condattr;
    /**< condition attributes */
    Mutex *externalMutex; /**< User's define Mutex */
} Cond;

/*!
 * \brief Create a new condition
 *
 * Create a new condition. The returned pointer must be
 * cleared with \a Cond_delete
 *
 * \return Return a new condition pointer
 * \see Cond_delete
 */
Cond *Cond_new( void );

/*!
 * \brief Initialize a condition
 * \param self Pointer to a condition
 * \param flags \a COND_SHARED or \a COND_PRIVATE
 *
 * Must be called in order to initialize a condition
 *
 * \return Return \a true if initialized correctly, \a false otherwise
 */
BaseBool Cond_init( Cond *self, const long flags );

/*!
 * \brief Sets a user's specific Mutex
 * \param self Pointer to a condition
 * \param mutex Pointet to a user's mutex
 *
 * This function sets the user's Mutex which is used instead of the Cond's
 * internal one.
 *
 * \return Nothing
 *
 * \see Cond_signal()
 */
void Cond_setMutex( Cond *self, Mutex *mutex );

/*!
 * \brief Signal a condition with a synchronization with the internal mutex
 * \param self Pointer to a condition
 *
 * Signal a condition and wake up only one thread waiting on it.
 *
 * \return Return \a 0 on success, non zero on error
 *
 */
int Cond_signal( Cond *self );

/*!
 * \brief Obsolete (only kept for compatibility)
 * \param self Pointer to a condition
 *
 * Please use Cond_signal() instead. This function does nothing than
 * calling Cond_signal() and returning its return value.
 *
 * \return Return \a 0 on success, non zero on error
 */
int Cond_signalSynch( Cond *self );

/*!
 * \brief Broadcast signal a condition
 * \param self Pointer to a condition
 *
 * Broadcast signal a condition and wake up one or more thread waiting on it
 *
 * \return Return \a 0 on success, non zero on error
 *
 */
int Cond_broadcast( Cond *self );

/*!
 * \brief Wait a condition to be signaled
 * \param self Pointer to a condition
 * \param microsecs Micro seconds to wait
 *
 * Wait a condition to signaled, the calling thread will be put on
 * sleep waiting a \a Cond_signal or \a Cond_broadcast to occur.
 * If \a microsec is specified the calling thread will wait the
 * relative amount of microsec, if \a 0 is specified the calling
 * thread will wait forever.
 *
 * If an external mutex is used, the user is able to create atomic
 * code. But therefore, it is necessary to surround those parts
 * including the call to this function within acquiring and releasing
 * of an external mutex.
 *
 * \return Return \a 0 on success, \a ETIMEOUT if the timeout is expired or
 * \a EINTR if interrupted by a signal
 *
 */
int Cond_wait( Cond *self, const long microsecs );

/*!
 * \brief Clean up an initialized condition
 * \param self Pointer to a condition
 *
 * Must be called in order to clean up a condition.
 *
 * \return Return nothing
 * \see Cond_init
 *
 */
void Cond_clear( Cond *self );

/*!
 * \brief Delete a condition
 * \param self Pointer to a condition
 *
 * Delete a condition created by \a Cond_new
 *
 * \return Return nothing
 * \see Cond_new
 */
void Cond_delete( Cond *self );

#if defined(__cplusplus)
}
#endif

#endif  /* COND_H */


/* EOF */
