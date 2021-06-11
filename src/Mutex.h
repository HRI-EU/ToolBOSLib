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


#ifndef MUTEX_H
#define MUTEX_H

#if ( _XOPEN_SOURCE - 0 ) < 600 && !defined(__windows__)
#error "_XOPEN_SOURCE must be >= 600 otherwise Mutex will not work properly"
#endif

#include <stdlib.h>
#include <pthread.h>

#include <Any.h>
#include <Base.h>

#define MUTEX_EINVAL    EINVAL
#define MUTEX_EAGAIN    EAGAIN
#define MUTEX_ESRCH     ESRCH
#define MUTEX_ENOSYS    ENOSYS
#define MUTEX_ENOMEM    ENOMEM
#define MUTEX_EBUSY     EBUSY
#define MUTEX_EPERM     EPERM
#define MUTEX_ETIMEDOUT ETIMEDOUT
#define MUTEX_ENOTSUP   ENOTSUP
#define MUTEX_EINTR     EINTR
#define MUTEX_EDEADLK   EDEADLK


#if defined(__cplusplus)
extern "C" {
#endif


#define MUTEX_PRIVATE  0x00000001
#define MUTEX_SHARED   0x00000002


typedef struct Mutex
{
    unsigned long valid;
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
} Mutex;


Mutex *Mutex_new( void );

/*!
 * \brief Initialize a mutex
 * \param self Pointer to a mutex
 * \param flags should always be MUTEX_PRIVATE
 */
BaseBool Mutex_init( Mutex *self, const long flags );

/*!
 * \brief Try to lock a mutex
 *
 * \return \c 0 upon success, \c EBUSY if the mutex is already locked
 */
int Mutex_tryLock( Mutex *self );

/*!
 * \brief Lock a mutex
 *
 * If the lock is already locked by another thread the calling thread
 * will be suspended.
 *
 * \return \c 0 upon success, \c EINVAL if mutex is invalid
 */
int Mutex_lock( Mutex *self );

int Mutex_unlock( Mutex *self );

void Mutex_clear( Mutex *self );

void Mutex_delete( Mutex *self );

#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
