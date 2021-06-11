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


#include <stdlib.h>
#include <pthread.h>

#include <RWLock.h>


#define RWLOCK_VALID   0xfda45d5a

#define RWLOCK_INVALID   0x4e374f6f


RWLock *RWLock_new( void )
{
    RWLock *self = (RWLock *)NULL;

    self = ANY_TALLOC( RWLock );
    ANY_REQUIRE( self );

    return self;
}


bool RWLock_init( RWLock *self, const long flags )
{
    int status = 0;
    int pflags = 0;

    ANY_REQUIRE( self );
    self->valid = RWLOCK_INVALID;

    self->attr = ANY_TALLOC( pthread_rwlockattr_t );
    ANY_REQUIRE_MSG( self->attr, "unable to allocate self->attr" );
    status = pthread_rwlockattr_init( self->attr );
    ANY_REQUIRE( status == 0 );

    /* setup some flags */
    pflags |= ( flags & RWLOCK_PRIVATE ? PTHREAD_PROCESS_PRIVATE : 0 );
    pflags |= ( flags & RWLOCK_SHARED ? PTHREAD_PROCESS_SHARED : 0 );

    status = pthread_rwlockattr_setpshared( self->attr, pflags );
    ANY_REQUIRE_MSG( status == 0, "failed to initialize self->attr" );


    self->rwlock = ANY_TALLOC( pthread_rwlock_t );
    ANY_REQUIRE_MSG( self->rwlock, "unable to allocate self->rwlock" );
#if !defined(_WIN32)
    status = pthread_rwlock_init( self->rwlock, self->attr );
#else
    /* pthreads-win32 doesn't support rwlock attributes */
    status = pthread_rwlock_init( self->rwlock, NULL );
#endif
    ANY_REQUIRE_MSG( status == 0, "failed to initialize self->rwlock" );

    self->valid = RWLOCK_VALID;

    return true;
}


int RWLock_readLock( RWLock *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RWLOCK_VALID );

    retVal = pthread_rwlock_rdlock( self->rwlock );

    return retVal;
}


int RWLock_tryReadLock( RWLock *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RWLOCK_VALID );

    retVal = pthread_rwlock_tryrdlock( self->rwlock );

    return retVal;
}


int RWLock_writeLock( RWLock *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RWLOCK_VALID );

    retVal = pthread_rwlock_wrlock( self->rwlock );

    return retVal;
}


int RWLock_tryWriteLock( RWLock *self )
{
    int retVal = 0;

    ANY_REQUIRE ( self );
    ANY_REQUIRE ( self->valid == RWLOCK_VALID );

    retVal = pthread_rwlock_trywrlock( self->rwlock );

    return retVal;
}


int RWLock_unlock( RWLock *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RWLOCK_VALID );

    retVal = pthread_rwlock_unlock( self->rwlock );

    return retVal;
}


void RWLock_clear( RWLock *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RWLOCK_VALID );
    ANY_REQUIRE( self->attr );
    ANY_REQUIRE( self->rwlock );

    status = pthread_rwlockattr_destroy( self->attr );
    ANY_REQUIRE( status == 0 );

    status = pthread_rwlock_destroy( self->rwlock );
    ANY_REQUIRE( status == 0 );

    ANY_FREE( self->attr );
    ANY_FREE( self->rwlock );
}


void RWLock_delete( RWLock *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* EOF */
