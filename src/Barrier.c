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

#include <Barrier.h>
#include <Atomic.h>


#define BARRIER_VALID      0xb9f5cfb7

#define BARRIER_INVALID    0x77e0c515

/*!
 *  \brief Barrier definition
 */
struct Barrier
{
    unsigned long valid;
    /**< object validity */
    pthread_mutex_t mutex;
    /**< mutex for the condition */
    pthread_mutexattr_t mutexattr;
    /**< mutex attr */
    long required;
    /**< required waiters */
    long arrived;
    /**< arrived waiters */
    AnyAtomic gone;
    /**< waiters gone */
    pthread_cond_t cond;
    /**< condition */
    pthread_condattr_t condattr;

    /**< condition attr */
    void                 (*callBack)( void * );

    /**< callback function */
    void *callBackArg;         /**< callback argument */
};


Barrier *Barrier_new( void )
{
    Barrier *self = (Barrier *)NULL;

    self = ANY_TALLOC( Barrier );
    ANY_REQUIRE( self );

    return self;
}


bool Barrier_init( Barrier *self,
                   const long flags,
                   const long count,
                   void (*callBack)( void * ),
                   void *callBackArg )
{
    int status = 0;
    int pflags = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE(( flags & ( BARRIER_PRIVATE | BARRIER_SHARED )) != 0 );
    ANY_REQUIRE( count > 0 );
    ANY_OPTIONAL( callBack );
    ANY_OPTIONAL( callBackArg );

    self->valid = BARRIER_INVALID;

    self->required = count;
    ANY_LOG( 5, "Initializing the Barrier '%p' with '%ld' waiters'",
             ANY_LOG_INFO, (void *)self, count );
    self->arrived = 0L;
    Atomic_set( &self->gone, 0 );

    status = pthread_mutexattr_init( &self->mutexattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_condattr_init( &self->condattr );
    ANY_REQUIRE( status == 0 );

    /* setup some flags */
    pflags |= ( flags & BARRIER_PRIVATE ? PTHREAD_PROCESS_PRIVATE : 0 );
    pflags |= ( flags & BARRIER_SHARED ? PTHREAD_PROCESS_SHARED : 0 );

    status = pthread_mutexattr_setpshared( &self->mutexattr, pflags );
    ANY_REQUIRE( status == 0 );

    status = pthread_condattr_setpshared( &self->condattr, pflags );
    ANY_REQUIRE( status == 0 );

    status = pthread_mutex_init( &self->mutex, &self->mutexattr );
    ANY_REQUIRE ( status == 0 );

    status = pthread_cond_init( &self->cond, &self->condattr );
    ANY_REQUIRE ( status == 0 );

    /* function callback */
    self->callBack = callBack;
    self->callBackArg = callBackArg;

    self->valid = BARRIER_VALID;

    return true;
}


bool Barrier_wait( Barrier *self )
{
    int status = 0;
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BARRIER_VALID );

    /* lock the object */
    status = pthread_mutex_lock( &self->mutex );
    ANY_REQUIRE( status == 0 );

    /* increments the waiters */
    ++( self->arrived );

    ANY_LOG( 5, "Barrier '%p' with '%ld of %ld' waiters'",
             ANY_LOG_INFO, (void *)self, self->arrived, self->required );

    /*
     * check if we have reach the required waiters in order to
     * invoke the callback
     */
    if( self->arrived >= self->required )
    {
        /*
         * we count ourself as gone because we don't wait
         * in the cond
         */
        Atomic_set( &self->gone, 1 );

        /*
         * if the function callback is defined execute it
         */
        if( self->callBack )
        {
            ANY_LOG( 5, "Barrier '%p' calling the CallBack'", ANY_LOG_INFO, (void *)self );
            ( *self->callBack )( self->callBackArg );
        }

        /*
         * finally we can wakeup all the waiters
         */
        status = pthread_cond_broadcast( &self->cond );
        ANY_REQUIRE( status == 0 );

        /*
         * reset the barrier, so no waiter are present on after
         */
        self->arrived = 0L;

        retVal = true;
    }
    else
    {
        /*
         * wait last thread reach the barrier. It should broadcast
         * on self->cond.
         * ATTENTION HERE!!! The self->mutex is released by the
         * pthread_cond_wait() but it's reaquired after
         */
        ANY_LOG( 5, "Barrier '%p' waiting...'", ANY_LOG_INFO, (void *)self );
        status = pthread_cond_wait( &self->cond, &self->mutex );
        ANY_REQUIRE( status == 0 );

        Atomic_inc( &self->gone );

        /* if all arrived then set gone to 0 */
        Atomic_testAndSetValue( &self->gone, self->arrived, 0 );
    }

    status = pthread_mutex_unlock( &self->mutex );
    ANY_REQUIRE( status == 0 );

    return retVal;
}

bool Barrier_isEmpty( Barrier *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BARRIER_VALID );

    return Atomic_get( &self->gone ) == 0 || Atomic_get( &self->gone ) >= self->required;
}

void Barrier_clear( Barrier *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == BARRIER_VALID );
    ANY_REQUIRE_MSG( self->gone == 0 || self->gone >= self->required, "There still someone waiting in the Barrier" );

    self->valid = BARRIER_INVALID;

    self->arrived = 0;
    self->gone = 0;

    status = pthread_mutexattr_destroy( &self->mutexattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_condattr_destroy( &self->condattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_mutex_destroy( &self->mutex );
    ANY_REQUIRE( status == 0 );

    status = pthread_cond_destroy( &self->cond );
    ANY_REQUIRE( status == 0 );
}


void Barrier_delete( Barrier *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* EOF */
