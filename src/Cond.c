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

#include <AnyTime.h>
#include <Cond.h>
#include <Mutex.h>


#define COND_VALID                  0x74d328fe
#define COND_INVALID                0x793bac7a
#define COND_NANOSEC_PER_MICROSEC         1000
#define COND_MICROSEC_PER_SEC          1000000
#define COND_NANOSEC_PER_SEC        1000000000


Cond *Cond_new( void )
{
    Cond *self = (Cond *)NULL;

    self = ANY_TALLOC( Cond );
    ANY_REQUIRE( self );

    return self;
}


BaseBool Cond_init( Cond *self, const long flags )
{
    int status = 0;
    int pflags = 0;

    ANY_REQUIRE( self );

    self->valid = COND_INVALID;

    self->externalMutex = (Mutex *)NULL;

    status = pthread_mutexattr_init( &self->mutexattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_condattr_init( &self->condattr );
    ANY_REQUIRE( status == 0 );

    /* setup some flags */
    pflags |= ( flags & COND_PRIVATE ? PTHREAD_PROCESS_PRIVATE : 0 );
    pflags |= ( flags & COND_SHARED ? PTHREAD_PROCESS_SHARED : 0 );

    /* setup some attributes */
    status = pthread_condattr_setpshared( &self->condattr, pflags );
    ANY_REQUIRE( status == 0 );
    status = pthread_mutexattr_setpshared( &self->mutexattr, pflags );
    ANY_REQUIRE( status == 0 );

    /* than initialize some structures */
    status = pthread_mutex_init( &self->mutex, &self->mutexattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_cond_init( &self->cond, &self->condattr );
    ANY_REQUIRE( status == 0 );

    self->valid = COND_VALID;

    return true;
}


void Cond_setMutex( Cond *self, Mutex *mutex )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == COND_VALID );

    self->externalMutex = mutex;
}


/* this function does NOT require an mutex, because it just
sends the signal to the pthread library */
int Cond_signal( Cond *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == COND_VALID );

    retVal = pthread_cond_signal( &self->cond );

    return retVal;
}


/* obsolete function kept for compatibility with RTBOS-2.0 */
int Cond_signalSynch( Cond *self )
{
    return Cond_signal( self );
}


int Cond_broadcast( Cond *self )
{
    int retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == COND_VALID );

    retVal = pthread_cond_broadcast( &self->cond );

    return retVal;
}


int Cond_wait( Cond *self, const long microsecs )
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)NULL;
    int status = 0;
    int retVal = 0;
    long long nanosecs = 0;
    long long secs = 0;
    struct timespec abstime;
    struct timeval tv;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == COND_VALID );

    /* if we have a user's mutex than we use it otherwise use the internal one */
    mutex = ( self->externalMutex ? &self->externalMutex->mutex : &self->mutex );

    /* if an external mutex is used, the user must lock first (else we'll do this) */
    if( !self->externalMutex )
    {
        status = pthread_mutex_lock( &self->mutex );
        ANY_REQUIRE( status == 0 );
    }

    /* if we don't want wait */
    if( !microsecs )
    {
        retVal = pthread_cond_wait( &self->cond, mutex );
    }
    else
    {
        /* normalize the microsecs */
        if( microsecs >= COND_MICROSEC_PER_SEC )
        {
            secs = microsecs / COND_MICROSEC_PER_SEC;
            nanosecs = ( microsecs % COND_MICROSEC_PER_SEC ) * COND_NANOSEC_PER_MICROSEC;
        }
        else
        {
            nanosecs = microsecs * COND_NANOSEC_PER_MICROSEC;
        }

        /* take the current time */
        Any_getTimeOfDay( &tv );

        /*
         * compute the future timeout by converting microsec to nanosec\
         * required by the pthread_cond_timedwait()
         */
        secs += tv.tv_sec;
        nanosecs += tv.tv_usec * 1000;

        /* normalize the nanosecs */
        if( nanosecs >= COND_NANOSEC_PER_SEC )
        {
            secs += nanosecs / COND_NANOSEC_PER_SEC;
            nanosecs %= COND_NANOSEC_PER_SEC;
        }

        /* setup the event some microsends later */
        abstime.tv_sec = (long)secs;
        abstime.tv_nsec = (long)nanosecs;

        /* wait the condition to be signaled */
        retVal = pthread_cond_timedwait( &self->cond, mutex, &abstime );
    }

    /* if we requested the mutex we have to release after work */
    if( !self->externalMutex )
    {
        status = pthread_mutex_unlock( mutex );
        ANY_REQUIRE( status == 0 );
    }

    return retVal;
}


void Cond_clear( Cond *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == COND_VALID );

    status = pthread_mutexattr_destroy( &self->mutexattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_mutex_destroy( &self->mutex );
    ANY_REQUIRE( status == 0 );

    status = pthread_condattr_destroy( &self->condattr );
    ANY_REQUIRE( status == 0 );

    status = pthread_cond_destroy( &self->cond );
    ANY_REQUIRE( status == 0 );

    self->externalMutex = NULL;

    self->valid = COND_INVALID;
}


void Cond_delete( Cond *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* EOF */
