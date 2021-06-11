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

#include <Mutex.h>


#define MUTEX_VALID   0xb87d8223

#define MUTEX_INVALID   0xac1cca9d


Mutex *Mutex_new( void )
{
    Mutex *self = (Mutex *)NULL;

    self = ANY_TALLOC( Mutex );
    ANY_REQUIRE( self );

    return self;
}


BaseBool Mutex_init( Mutex *self, const long flags )
{
    int status = 0;
    int pflags = 0;

    ANY_REQUIRE( self );
    self->valid = MUTEX_INVALID;

    status = pthread_mutexattr_init( &self->attr );
    ANY_REQUIRE( status == 0 );

    /* setup some flags */
    pflags |= ( flags & MUTEX_PRIVATE ? PTHREAD_PROCESS_PRIVATE : 0 );
    pflags |= ( flags & MUTEX_SHARED ? PTHREAD_PROCESS_SHARED : 0 );

    status = pthread_mutexattr_setpshared( &self->attr, pflags );
    ANY_REQUIRE( status == 0 );

#if !defined(__mingw__)
    status = pthread_mutexattr_setrobust( &self->attr, PTHREAD_MUTEX_ROBUST);
    ANY_REQUIRE( status == 0 );
#endif

    status = pthread_mutex_init( &self->mutex, &self->attr );
    ANY_REQUIRE( status == 0 );

    self->valid = MUTEX_VALID;

    return true;
}


int Mutex_tryLock( Mutex *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MUTEX_VALID );

    retVal = pthread_mutex_trylock( &self->mutex );

    return retVal;
}


int Mutex_lock( Mutex *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MUTEX_VALID );

    retVal = pthread_mutex_lock( &self->mutex );

#if !defined( __mingw__ )
    // Missing on mingw: https://www.gnu.org/software/gnulib/manual/html_node/pthread_005fmutex_005fconsistent.html
    if ( retVal == EOWNERDEAD )
    {
        retVal = pthread_mutex_consistent( &self->mutex );
    }
#endif

    return retVal;
}


int Mutex_unlock( Mutex *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MUTEX_VALID );

    retVal = pthread_mutex_unlock( &self->mutex );

    return retVal;
}


void Mutex_clear( Mutex *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MUTEX_VALID );

    status = pthread_mutexattr_destroy( &self->attr );
    ANY_REQUIRE( status == 0 );

    status = pthread_mutex_destroy( &self->mutex );
    ANY_REQUIRE( status == 0 );

    self->valid = MUTEX_INVALID;
}


void Mutex_delete( Mutex *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* EOF */
