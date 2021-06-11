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


#include <Any.h>
#include <Atomic.h>
#include <Mutex.h>
#include <RWLock.h>
#include <RTTimer.h>


static void TestMutex( int iteration );

static void TestRWLock( int iteration );

static void TestAtomic( int iteration );

static void PrintTimer( RTTimer *timer, char *title );

#define ITERATION   10000000


int main( int argc, char *argv[] )
{
    TestMutex( ITERATION );
    TestRWLock( ITERATION );
    TestAtomic( ITERATION );

    return EXIT_SUCCESS;
}


static void PrintTimer( RTTimer *timer, char *title )
{
    unsigned long      elapsed      = 0;
    unsigned long      minTime      = 0;
    unsigned long      averangeTime = 0;
    unsigned long      maxTime      = 0;
    unsigned long long totalTime    = 0;
    unsigned long      counter      = 0;
    char               timef[64];

    ANY_REQUIRE( timer );
    ANY_REQUIRE( title );

    elapsed      = RTTimer_getElapsed( timer );
    minTime      = RTTimer_getMinTime( timer );
    averangeTime = RTTimer_getAverageTime( timer );
    maxTime      = RTTimer_getMaxTime( timer );
    totalTime    = RTTimer_getTotalTime( timer );
    counter      = RTTimer_getCount( timer );

    ANY_LOG( 0, "Performace Statistics: %s with %d iterations", ANY_LOG_INFO, title, ITERATION );
    ANY_LOG( 0, "------------------------------------------------------", ANY_LOG_INFO );

    RTTimer_format( timef, (double)elapsed );
    ANY_LOG( 0, "Last Elapsed time is %lu microsecs (%s)", ANY_LOG_INFO, elapsed, timef );

    RTTimer_format( timef, (double)minTime );
    ANY_LOG( 0, "Minimum time is %lu microsecs (%s)", ANY_LOG_INFO, minTime, timef );

    RTTimer_format( timef, (double)averangeTime );
    ANY_LOG( 0, "Average time is %lu microsecs (%s)", ANY_LOG_INFO, averangeTime, timef );

    RTTimer_format( timef, (double)maxTime );
    ANY_LOG( 0, "Max time is %lu microsecs (%s)", ANY_LOG_INFO, maxTime, timef );

    RTTimer_format( timef, totalTime );
    ANY_LOG( 0, "Total time is %llu secs (%s)", ANY_LOG_INFO, totalTime, timef );

    ANY_LOG( 0, "Total counter is %lu", ANY_LOG_INFO, counter );
    ANY_LOG( 0, "------------------------------------------------------", ANY_LOG_INFO );
}


static void TestMutex( int iteration )
{
    Mutex   *mutex = NULL;
    RTTimer *timer = NULL;
    int     status;
    int     value  = 0;

    mutex = Mutex_new();
    ANY_REQUIRE( mutex );
    Mutex_init( mutex, MUTEX_PRIVATE );

    timer = RTTimer_new();
    ANY_REQUIRE( timer );
    RTTimer_init( timer );

    RTTimer_start( timer );

    while( iteration-- > 0 )
    {
        status = Mutex_lock( mutex );
        ANY_REQUIRE( status == 0 );

        value++;

        status = Mutex_unlock( mutex );
        ANY_REQUIRE( status == 0 );
    }

    RTTimer_stop( timer );

    Mutex_clear( mutex );
    Mutex_delete( mutex );

    PrintTimer( timer, "Mutex" );

    RTTimer_clear( timer );
    RTTimer_delete( timer );
}


static void TestRWLock( int iteration )
{
    RWLock  *lock  = NULL;
    RTTimer *timer = NULL;
    int     status;
    int     value  = 0;

    lock = RWLock_new();
    ANY_REQUIRE( lock );
    RWLock_init( lock, MUTEX_PRIVATE );

    timer = RTTimer_new();
    ANY_REQUIRE( timer );
    RTTimer_init( timer );

    RTTimer_start( timer );

    while( iteration-- > 0 )
    {
        status = RWLock_writeLock( lock );
        ANY_REQUIRE( status == 0 );

        value++;

        status = RWLock_unlock( lock );
        ANY_REQUIRE( status == 0 );
    }

    RTTimer_stop( timer );

    RWLock_clear( lock );
    RWLock_delete( lock );

    PrintTimer( timer, "RWLock" );

    RTTimer_clear( timer );
    RTTimer_delete( timer );
}


static void TestAtomic( int iteration )
{
    RTTimer   *timer = NULL;
    AnyAtomic value  = 0;

    timer = RTTimer_new();
    ANY_REQUIRE( timer );
    RTTimer_init( timer );

    RTTimer_start( timer );

    while( iteration-- > 0 )
    {
        Atomic_inc( &value );
    }

    RTTimer_stop( timer );

    PrintTimer( timer, "Atomic" );

    RTTimer_clear( timer );
    RTTimer_delete( timer );
}


/* EOF */
