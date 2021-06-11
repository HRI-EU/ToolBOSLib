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


#if !defined(__windows__)

// for nanosleep
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif

#include <time.h>
#include <errno.h>

#endif


#if defined(__macos__)
int clock_gettime_macos( int clk_id, struct timespec *tp )
{
    struct timeval tv;
    struct timezone tz;
    if( gettimeofday( &tv, &tz ) != 0 )
        {
          ANY_LOG( 0, "clock_gettime_macos(): An error occurred in gettimeofday()", ANY_LOG_ERROR );
        }
    unsigned long long nanosecs = tv.tv_usec *1000L + tv.tv_sec;

    return nanosecs;
}
#endif

// for calculations in get time of day
#if defined(__windows__) || defined(__mingw__)
#define EPOCHFILETIME ( 116444736000000000 )
#endif

/* on windows we define the compatibility layer */
#if defined(__windows__) && !defined(__mingw__)

#include <windows.h>
#include <time.h>

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

/*
 * Compatibility layer for native windows
 */
struct timespec
{
  long                tv_sec;       /**< seconds */
  long                tv_nsec;      /**< nanoseconds */
};

#endif /* HAVE_STRUCT_TIMESPEC */

#define CLOCK_REALTIME  1
#define CLOCK_MONOTONIC 2

static int clock_gettime( clockid_t clk_id, struct timespec *tp );

#endif


#define ANYTIME_NANOSECONDS 1000000000ULL


double Any_time( void )
{
    return ((double)Any_getTime());
}


unsigned long long Any_getTime( void )
{
    unsigned long long retVal = 0;
    struct timespec timespec;
    int status = 0;

    status = clock_gettime( CLOCK_MONOTONIC, &timespec );

    if( status == -1 )
    {
        ANY_LOG( 0, "An error was occurred on clock_gettime()", ANY_LOG_ERROR );
        goto out;
    }

    retVal = ( timespec.tv_sec * ANYTIME_NANOSECONDS ) +
             (unsigned long long)timespec.tv_nsec;
    out:

    return retVal;
}


#if defined(__windows__) && !defined(__mingw__)

static int clock_gettime( clockid_t clk_id, struct timespec *tp )
{
  static int initialized = 0;
  static LARGE_INTEGER frequency;
  LARGE_INTEGER t0;
  int retVal = 0;
  LONGLONG now;

  ANY_REQUIRE( tp );

  if ( !initialized )
  {
    if ( !QueryPerformanceFrequency( &frequency ) )
    {
      ANY_LOG( 0, "The HPC windows subsystem is not available in this machine", ANY_LOG_ERROR );
      retVal = -1;
      goto out;
    }
    else
    {
      initialized = 1;
    }
  }

  if ( !QueryPerformanceCounter( &t0 ) )
  {
    ANY_LOG( 0, "Unable to get the HPC counters", ANY_LOG_ERROR );
    retVal = -1;
    goto out;
  }

  now = (LONGLONG)( ( t0.QuadPart * 1000ULL ) / frequency.QuadPart );
  tp->tv_sec  = (long)now / ANYTIME_NANOSECONDS;
  tp->tv_nsec = now % ANYTIME_NANOSECONDS;

 out:

  return( retVal );
}

#endif /* __windows__ */


int Any_getTimeOfDay( struct timeval *tv )
{
    int status = 0;

#if defined(__windows__)

    /* The following code is LGPL code from the wine project */
    FILETIME      ft;
    LARGE_INTEGER li;
    __int64       t;

    if( tv )
    {
      GetSystemTimeAsFileTime( &ft );

      li.LowPart  = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      t  = li.QuadPart;       /* In 100-nanosecond intervals */
      t -= EPOCHFILETIME;     /* Offset to the Epoch time */
      t /= 10;                /* In microseconds */
      tv->tv_sec  = (long)(t / 1000000);
      tv->tv_usec = (long)(t % 1000000);
    }

    status = 0;

#else

    status = gettimeofday( tv, NULL);

#endif

    return status;
}


long long int Any_getCurrentTimeInMicroSeconds( void )
{
    struct timeval t;

    if( Any_getTimeOfDay( &t ) != 0 )
    {
        return -1;
    }

    return t.tv_sec * 1000000LL + t.tv_usec;
}


int Any_sleepSeconds( long long seconds )
{
#if defined(__windows__)

    return Any_sleepMilliSeconds( seconds * 1000 );

#else

    return sleep( seconds );

#endif
}


int Any_sleepMilliSeconds( long long milliSeconds )
{
    return Any_sleepNanoSeconds( milliSeconds * 1000000 );
}


int Any_sleepMicroSeconds( long long microSeconds )
{
    return Any_sleepNanoSeconds( microSeconds * 1000 );
}


int Any_sleepNanoSeconds( long long nanoSeconds )
{
#if defined(__windows__)

    SleepEx( ( nanoSeconds < 500 ? 1 : ( nanoSeconds + 500) / 1000000 ), TRUE );

    return 0;

#else

    int status;
    struct timespec req, rem;

    req.tv_sec = nanoSeconds / 1000000000;
    req.tv_nsec = nanoSeconds % 1000000000;

    do
    {
        status = nanosleep( &req, &rem );
        req = rem;
    }
    while( status == EINTR );

    return status;

#endif
}
