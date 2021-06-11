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


#include <stdio.h>
#include <signal.h>

#include <Threads.h>


#define THREADS_VALID    0x8685e2ae
#define THREADS_INVALID    0x87411d50


Threads *Threads_new( void )
{
    Threads *self = (Threads *)NULL;

    self = ANY_TALLOC( Threads );
    ANY_REQUIRE( self );

    return self;
}


bool Threads_init( Threads *self, bool joinable )
{
    int status = 0;
    int pflags = 0;

    ANY_REQUIRE( self );
    self->valid = THREADS_INVALID;

    status = pthread_attr_init( &self->attr );
    ANY_REQUIRE( status == 0 );

    pflags |= ( joinable == true ?
                PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED );

    status = pthread_attr_setdetachstate( &self->attr, pflags );
    ANY_REQUIRE( status == 0 );

    /* gets the schedule's params */
    status = pthread_attr_getschedparam( &self->attr, &self->schedulerParams );
    ANY_REQUIRE( status == 0 );

    self->valid = THREADS_VALID;

    return true;
}


int Threads_start( Threads *self,
                   void *(*start_routine)( void * ),
                   void *arg )
{
    int status = 0;
    int level = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );
    ANY_REQUIRE( start_routine );

    /*
     * every newly started thread we increase the concurrency level
     * this is absolutely required on Sun Solaris
     */
    level = pthread_getconcurrency();
    status = pthread_setconcurrency( level );
    ANY_REQUIRE( status == 0 );

    /* start the thread */
    status = pthread_create( &self->thread, &self->attr, start_routine, arg );

    return ( status );
}


int Threads_join( Threads *self, void **retValue )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    status = pthread_join( self->thread, retValue );

    return ( status );
}


int Threads_stop( Threads *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    status = pthread_cancel( self->thread );

    return ( status );
}


/*
int Threads_once( ... )
{
   int status = 0;

   ANY_REQUIRE( self );
   ANY_REQUIRE( self->valid == THREADS_VALID );

   status = pthread_once( ... );

   return( status );
}
*/

void Threads_kill( Threads *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    pthread_kill( self->thread, SIGABRT );
}


void Threads_setCancellable( Threads *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL);
}


void Threads_setUncancellable( Threads *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL);
}


void Threads_faultRecovery( void )
{
    ANY_LOG( 0, "Thread crashed!!!", ANY_LOG_FATAL );

    pthread_exit(NULL);
}


void Threads_exit( Threads *self, void *retVal )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    pthread_exit( retVal );
}


int Threads_yield( void )
{
#if defined __USE_GNU && !defined( __mingw__ )
    // Missing on mingw: https://www.gnu.org/software/gnulib/manual/html_node/pthread_005fyield.html
    return ( pthread_yield());
#else
    return( 0 );
#endif
}


unsigned long Threads_id( void )
{
#if ( defined(__msvc__) || defined(__windows__) ) && !defined( __mingw__ )
    // pthread_self().p is void*
    return( (unsigned long)(size_t)pthread_self().p );
#else
    return ((unsigned long)pthread_self());
#endif
}


bool Threads_isEqualId( Threads *self, unsigned long id )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

#if ( defined(__msvc__) || defined(__windows__) ) && !defined( __mingw__ )
    // pthread_self().p is void*
    return( (unsigned long)(size_t)self->thread.p == id );
#else
    return ( self->thread == (pthread_t)id );
#endif
}


void Threads_setPriority( Threads *self, int priority )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    if( priority > 0 )
    {
        self->schedulerParams.sched_priority = priority;
        status = pthread_attr_setschedparam( &self->attr, &self->schedulerParams );
        if( status == THREADS_EINVAL )
        {
            ANY_LOG( 0, "Threads_setPriority() can be called only after Threads_setSchedPolicy()", ANY_LOG_WARNING );
        }
        else
        {
            ANY_TRACE( 5, "Threads_setPriority() %d", status );
            ANY_REQUIRE( status == 0 );
        }
    }
}


int Threads_getPriority( Threads *self )
{
    ANY_REQUIRE ( self );
    ANY_REQUIRE ( self->valid == THREADS_VALID );

    return ( self->schedulerParams.sched_priority );
}


void Threads_clear( Threads *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    status = pthread_attr_destroy( &self->attr );
    ANY_REQUIRE( status == 0 );

    self->valid = THREADS_INVALID;
}


void Threads_delete( Threads *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


void Threads_setSchedPolicy( Threads *self, int policy, int priority )
{
    int status = 0;
    int sched_min = 0;
    int sched_max = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == THREADS_VALID );

    /*
     * sets the requested policy on the pthread_attr
     */
    status = pthread_attr_setschedpolicy( &self->attr, policy );
    ANY_REQUIRE_MSG( status == 0, "Unable to set the schedule policy for a thread" );

    /*
     * Now checks for min and max priority
     */
    sched_min = sched_get_priority_min( policy );

    if( sched_min == -1 )
    {
        ANY_LOG( 0, "Unable to get minimum priority with sched_get_priority_min()", ANY_LOG_ERROR );
        return;
    }

    sched_max = sched_get_priority_max( policy );

    if( sched_max == -1 )
    {
        ANY_LOG( 0, "Unable to get max priority with sched_get_priority_max()", ANY_LOG_ERROR );
        return;
    }

    /* checks if the requested priority is in the policy range */
    if( priority < sched_min || priority > sched_max )
    {
        ANY_LOG( 0, "Unable to set the priority to '%d' range must be in '%d' to '%d'",
                 ANY_LOG_ERROR, priority, sched_min, sched_max );
        return;
    }

    /* and than apply it on the pthread_attr */
    self->schedulerParams.sched_priority = priority;

    status = pthread_attr_setschedparam( &self->attr, &self->schedulerParams );
    ANY_REQUIRE_MSG( status == 0, "Unable to set the schedule param fora thread" );

    /*
     * Finally, since we want our how policy and priority than we have
     * to request for explicit scheduling otherwise our constraint will
     * never be satisfied ;-)!
     */
    status = pthread_attr_setinheritsched( &self->attr, PTHREAD_EXPLICIT_SCHED );
    ANY_REQUIRE_MSG( status == 0, "Unable to set the thread as explicit schedule params" );
}


/* EOF */
