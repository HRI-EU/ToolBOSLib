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


#ifndef ANYTIME_H
#define ANYTIME_H

#include <AnyDef.h>

// for struct timeval
#if defined (__windows__)

#include <winsock.h>

#else

#include <unistd.h>
#include <sys/time.h>

#endif

#if !defined(CLOCK_MONOTONIC)
#define CLOCK_MONOTONIC 2
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief (obsolete) Return the current time
 * \see Any_getTime()
 */
double Any_time( void );

/*!
 * \brief Return the current time expressed in nanoseconds
 *
 * This is a monotonic time, with zero being the process starting point.
 *
 * \return Return the current time in nanoseconds
 */
unsigned long long Any_getTime( void );

/*!
 * \brief sleep for the specified number of seconds
 *
 * Makes the current process sleep until \c seconds seconds have elapsed
 * or a signal arrives which is not ignored.
 *
 * \param seconds number of seconds to wait, will be multiplied with the
 *                global timewarp factor
 * \returns zero if the requested time has elapsed, or the number of seconds
 *          left to sleep (if woken up before because of system signal)
 */
int Any_sleepSeconds( long long seconds );

/*!
 * \brief sleep for the specified number of milliseconds
 *
 * Makes the current process sleep until \c milliSeconds milliseconds have elapsed
 * or a signal arrives which is not ignored.
 *
 * \param milliSeconds number of milliseconds to wait, will be multiplied with the
 *                global timewarp factor
 * \returns zero if the requested time has elapsed, or the number of milliseconds
 *          left to sleep (if woken up before because of system signal)
 */
int Any_sleepMilliSeconds( long long milliSeconds );

/*!
 * \brief sleep for the specified number of microseconds
 *
 * Makes the current process sleep until \c microSeconds microseconds have elapsed
 * or a signal arrives which is not ignored.
 *
 * \param microSeconds number of microseconds to wait, will be multiplied with the
 *                global timewarp factor
 * \returns zero if the requested time has elapsed, or the number of microseconds
 *          left to sleep (if woken up before because of system signal)
 */
int Any_sleepMicroSeconds( long long microSeconds );

/*!
 * \brief sleep for (at least) the specified number of nanoseconds
 *
 * Makes the current process sleep until \c nanoSeconds naoseconds have elapsed
 * or a signal arrives which is not ignored.
 *
 * \attention The sleep may be lengthened slightly by any system activity
 *            or by the time spent processing the call or by the granularity
 *            of system timers.
 *
 * \param nanoSeconds number of nanoseconds to wait, will be multiplied with the
 *                global timewarp factor
 * \returns zero if the requested time has elapsed, or the number of nanoseconds
 *          left to sleep (if woken up before because of system signal)
 */
int Any_sleepNanoSeconds( long long nanoSeconds );

/*!
 * \brief Gets current time in micro seconds
 * (with arbitrary but fixed absolute base)
 *
 * \returns time in micro seconds
 */
long long int Any_getCurrentTimeInMicroSeconds( void );

/*!
 * \brief Return in tv the current time
 * \param tv Pointer to timeval structure where the function return the current time of the day
 */
int Any_getTimeOfDay( struct timeval *tv );

#if defined(__macos__)
#define clock_gettime clock_gettime_macos
int clock_gettime_macos( int clk_id, struct timespec *tp );
#endif

#if defined(__cplusplus)
}
#endif

#endif /* END ANYTIME_H*/
