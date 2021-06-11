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
#include <time.h>

#include <Any.h>
#include <RTTimer.h>


#define RTTIMER_VALID      0xd1e30279
#define RTTIMER_INVALID    0xf984177b


#if defined(__windows__) && !defined(__mingw__)

#pragma warning( push )
#pragma warning( disable : 4005 )

#include <windows.h>

#pragma warning( pop )

typedef int clockid_t;

// recent versions of time.h in windows sdk do not
// define HAVE_STRUCT_TIMESPEC, but do define the
// struct
#if _MSC_VER >= 1910  // VS 2017
#define HAVE_STRUCT_TIMESPEC 1
#endif

#if !defined(HAVE_STRUCT_TIMESPEC)

/*
 * on native windows we use the pthreads-win32 emulation
 * layer which also define internally the struct timespec.
 * The declaration is conditioned by the HAVE_STRUCT_TIMESPEC
 * definition, so below we prevent its redeclation
 */
#define HAVE_STRUCT_TIMESPEC  1


struct timespec
{
  long tv_sec;
  long tv_nsec;
};

#endif // HAVE_STRUCT_TIMESPEC


#define CLOCK_REALTIME  1
#define CLOCK_MONOTONIC 2

static int clock_gettime( clockid_t clk_id, struct timespec *tp );

#endif // __windows__


static void RTTimer_updateStatistics( RTTimer *self, bool countIncrement );


RTTimer *RTTimer_new()
{
    RTTimer *self = ANY_TALLOC( RTTimer );

    return self;
}


bool RTTimer_init( RTTimer *self )
{
    ANY_REQUIRE( self );

    self->valid = RTTIMER_VALID;

    RTTimer_reset( self );

    return true;
}


unsigned long long RTTimer_getTime( void )
{
    unsigned long long retVal = 0;
    struct timespec timespec;
    int status = 0;

    status = clock_gettime( CLOCK_MONOTONIC, &timespec );

    if( status == -1 )
    {
        ANY_LOG( 5, "An error was occurred on clock_gettime()", ANY_LOG_FATAL );
        goto out;
    }

    retVal = ( timespec.tv_sec * RTTIMER_NANOSECONDS ) +
             (unsigned long long)timespec.tv_nsec;
    out:

    return retVal;
}


void RTTimer_start( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE_MSG( self->started == false, "RTTimer already started" );

    self->start = RTTimer_getTime();

    self->started = true;
    self->paused = false;
}


void RTTimer_pause( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE_MSG( self->started == true, "RTTimer not started" );
    ANY_REQUIRE_MSG( self->paused == false, "RTTimer already paused" );

    self->stop = RTTimer_getTime();

    self->paused = true;

    RTTimer_updateStatistics( self, false );
}


void RTTimer_continue( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE_MSG( self->started == true, "RTTimer not started" );
    ANY_REQUIRE_MSG( self->paused == true, "RTTimer is already running" );

    self->start = RTTimer_getTime();
    self->stop = self->start;

    self->paused = false;
}


void RTTimer_stop( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE_MSG( self->started == true, "RTTimer not started" );

    self->stop = RTTimer_getTime();

    self->started = false;
    self->paused = false;

    RTTimer_updateStatistics( self, true );
}


unsigned long long RTTimer_getElapsed( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->count ? ( self->stop - self->start ) : 0 );
}


void RTTimer_getElapsedExt( RTTimer *self, RTTimerSpec *spec )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( spec );

    RTTimer_micro2RTTimerSpec( RTTimer_getElapsed( self ), spec );
}


unsigned long long RTTimer_getMinTime( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->minTime != ~0ULL ? self->minTime : 0ULL );
}


void RTTimer_getMinTimeExt( RTTimer *self, RTTimerSpec *spec )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( spec );

    RTTimer_micro2RTTimerSpec( RTTimer_getMinTime( self ), spec );
}


unsigned long long RTTimer_getAverageTime( RTTimer *self )
{
    unsigned long long retVal = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    if( self->count > 0 )
    {
        retVal = self->totalTime / self->count;
    }

    return retVal;
}


void RTTimer_getAverageTimeExt( RTTimer *self, RTTimerSpec *spec )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( spec );

    RTTimer_micro2RTTimerSpec( RTTimer_getAverageTime( self ), spec );
}


unsigned long long RTTimer_getMaxTime( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->maxTime );
}


void RTTimer_getMaxTimeExt( RTTimer *self, RTTimerSpec *spec )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( spec );

    RTTimer_micro2RTTimerSpec( RTTimer_getMaxTime( self ), spec );
}


unsigned long long RTTimer_getTotalTime( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->totalTime );
}


void RTTimer_getTotalTimeExt( RTTimer *self, RTTimerSpec *spec )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( spec );

    RTTimer_micro2RTTimerSpec( RTTimer_getTotalTime( self ), spec );
}


unsigned long long RTTimer_getCount( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->count );
}


unsigned long long RTTimer_getStartTime( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->start );
}


unsigned long long RTTimer_getStopTime( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    return ( self->stop );
}


void RTTimer_reset( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    self->start = 0;
    self->stop = 0;
    self->count = 0;
    self->minTime = ~0ULL;     /* minTime will have the highest time value */
    self->maxTime = 0;         /* maxTime will have the min time value */
    self->started = false;
    self->paused = false;
    self->totalTime = 0;
}


void RTTimer_micro2RTTimerSpec( unsigned long long time, RTTimerSpec *spec )
{
    ANY_REQUIRE( spec );

    if( time )
    {
        unsigned long secs = (unsigned long)( time / RTTIMER_NANOSECONDS );

        spec->microsecond = time - (((unsigned long long)secs ) * RTTIMER_NANOSECONDS );

        spec->day = secs / ( 24 * 60 * 60 );
        secs -= ( spec->day * 24 * 60 * 60 );

        spec->hour = secs / ( 60 * 60 );
        secs -= ( spec->hour * 60 * 60 );

        spec->minute = secs / 60;
        secs -= ( spec->minute * 60 );

        spec->second = secs;
    }
}


void RTTimer_format( char *buffer, double value )
{
    RTTimerSpec spec;

    ANY_REQUIRE( buffer );

    RTTimer_micro2RTTimerSpec( value, &spec );

    RTTimer_formatExt( buffer, &spec );
}


void RTTimer_formatExt( char *buffer, RTTimerSpec *spec )
{
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( spec );

    Any_sprintf( buffer, "%lu days %lu:%lu:%lu.%06lu",
                 spec->day, spec->hour, spec->minute, spec->second, spec->microsecond );
}


void RTTimer_copyStart( RTTimer *self, RTTimer *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->valid == RTTIMER_VALID );

    self->start = src->start;
    self->started = true;
    self->paused = false;
}


void RTTimer_clear( RTTimer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    RTTimer_reset( self );

    self->valid = RTTIMER_INVALID;
}


void RTTimer_delete( RTTimer *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static void RTTimer_updateStatistics( RTTimer *self, bool countIncrement )
{
    unsigned long long elapsed = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == RTTIMER_VALID );

    if( countIncrement )
    {
        self->count++;
    }

    elapsed = RTTimer_getElapsed( self );

    /* Do not update statistics for elapsed time == 0 because this may
     * happen only when the elapsed time between _start/_stop or
     * _start/_pause or _continue/_stop in less then the time spent my
     * the CPU to execute the code. This generally means that the timer
     * isn't really an "high-resolution" timer ;-)!
     */
    if( elapsed )
    {
        self->totalTime += elapsed;

        /* checks for minimum time */
        if( self->minTime > elapsed )
        {
            self->minTime = elapsed;
        }

        /* checks for max time */
        if( self->maxTime < elapsed )
        {
            self->maxTime = elapsed;
        }
    }
}


#if defined(__windows__) && !defined(__mingw__)

static LARGE_INTEGER getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;

    return (t);
}

static void initPerformanceCounter( LARGE_INTEGER *offset, double *frequency )
{
    LARGE_INTEGER performanceFrequency;

    QueryPerformanceFrequency( &performanceFrequency );
    QueryPerformanceCounter( offset );

    *frequency = (double)performanceFrequency.QuadPart / 1000000.;
}

static int clock_gettime( clockid_t C, struct timespec *tv )
{
    LARGE_INTEGER        tmp;
    double               usecs;
    static LARGE_INTEGER offset;
    static double        frequency;
    static int           initialized = 0;

    if (!initialized)
    {
        initPerformanceCounter( &offset, &frequency );
        initialized = 1;
    }

    QueryPerformanceCounter( &tmp );

    tmp.QuadPart   -= offset.QuadPart;
    usecs           = (double)tmp.QuadPart / frequency;
    tmp.QuadPart    = usecs;
    tv->tv_sec      = tmp.QuadPart / 1000000;
    tv->tv_nsec     = ( tmp.QuadPart % 1000000 ) * 1000;

    return (0);
}

#endif


/* EOF */
