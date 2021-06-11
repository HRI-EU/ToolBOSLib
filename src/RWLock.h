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


#ifndef RWLOCK_H
#define RWLOCK_H

#if ( _XOPEN_SOURCE - 0 ) < 600 && !defined(__windows__)
#error "_XOPEN_SOURCE must be >= 600 otherwise RWLock will not work properly"
#endif

#include <pthread.h>

#include <Any.h>


#define RWLOCK_EINVAL    EINVAL
#define RWLOCK_EAGAIN    EAGAIN
#define RWLOCK_ESRCH     ESRCH
#define RWLOCK_ENOSYS    ENOSYS
#define RWLOCK_ENOMEM    ENOMEM
#define RWLOCK_EBUSY     EBUSY
#define RWLOCK_EPERM     EPERM
#define RWLOCK_ETIMEDOUT ETIMEDOUT
#define RWLOCK_ENOTSUP   ENOTSUP
#define RWLOCK_EINTR     EINTR
#define RWLOCK_EDEADLK   EDEADLK


#if defined(__cplusplus)
extern "C" {
#endif

/**!
 *   \brief Define a private RWLock
 *
 * It's used to initialize a local process private rwlock, but
 * shared by all threads where they are part.
 *
 */
#define RWLOCK_PRIVATE   0x00000001

/**!
 *   \brief Define a public Mutex
 *
 * It's used to initialize a public rwlock available for all
 * the process that need it. Typically it's created in a shared
 * memory area.
 *
 */
#define RWLOCK_SHARED   0x00000002


/*!
 * \brief Read & Write lock
 */
typedef struct RWLock
{
    unsigned long valid;
    /**< object validity */
    pthread_rwlock_t *rwlock;
    /**< rwlock */
    pthread_rwlockattr_t *attr;    /**< rwlock attribute */
} RWLock;

/*!
 * \brief Create a new RWLock
 *
 * Create a new rwlock. The returned pointer must be
 * cleared with \a RWLock_delete
 *
 * \return Return a new rwlock pointer
 * \see RWLock_delete
 */
RWLock *RWLock_new( void );

/*!
 * \brief Initialize a RWLock
 * \param self Pointer to a mutex
 * \param flags \a RWLOCK_SHARED or \a RWLOCK_PRIVATE
 *
 * Must be called in order to initialize an RWLock
 *
 * \return Return \a true if initialized correctly, \a false otherwise
 */
bool RWLock_init( RWLock *self, const long flags );

/*!
 * \brief Lock for read a RWLock
 * \param self Pointer to a rwlock
 *
 * Lock for read a RWLock, if the lock is already locked by another thread
 * the calling thread will be suspended
 *
 * \return Return \a 0 if success, \a EINVAL if rwlock is invalid
 * \see RWLock_unlock
 */
int RWLock_readLock( RWLock *self );

/*!
 * \brief Try to lock for read
 * \param self Pointer to a rwlock
 *
 * Try to lock for read, if the lock is not locked by others
 * thread the lock will succeed, otherwise return immediatly with \a EBUSY
 *
 * \return Return \a 0 if success, \a EBUSY if the mutex is already locked
 * \see RWLock_unlock
 */
int RWLock_tryReadLock( RWLock *self );

/*!
 * \brief Lock for write a RWLock
 * \param self Pointer to a rwlock
 *
 * Lock for write a RWLock, if the lock is already locked by another thread
 * the calling thread will be suspended
 *
 * \return Return \a 0 if success, \a EINVAL if rwlock is invalid
 * \see RWLock_unlock
 */
int RWLock_writeLock( RWLock *self );

/*!
 * \brief Lock for write a RWLock
 * \param self Pointer to a rwlock
 *
 * Lock for write a RWLock, if the lock is already locked by another thread
 * the calling thread will be suspended
 *
 * \return Return \a 0 if success, \a EINVAL if rwlock is invalid
 * \see RWLock_unlock
 */
int RWLock_tryWriteLock( RWLock *self );

/*!
 * \brief Unlock a RWLock
 * \param self Pointer to a rwlock
 *
 * Unlock a previously locked rwlock.
 *
 * \return Return \a 0 if success, non zero on error
 * \see RWLock_readLock
 * \see RWLock_tryReadLock
 * \see RWLock_writeLock
 * \see RWLock_tryWriteLock
 */
int RWLock_unlock( RWLock *self );

/*!
 * \brief Clean up a RWLock
 * \param self Pointer to a rwlock
 *
 * Must be called in order to clean up an initialized rwlock
 *
 * \see RWLock_init
 */
void RWLock_clear( RWLock *self );

/*!
 * \brief Delete a RWLock
 * \param self Pointer to a RWLock
 *
 * Delete an allocated RWLock
 *
 * \see RWLock_new
 */
void RWLock_delete( RWLock *self );

#if defined(__cplusplus)
}
#endif

#endif  /* RWLOCK_H */


/* EOF */
