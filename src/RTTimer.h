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


/*!
 * \page RTTimer_About High-precision time measurement
 *
 * The RTTimer (RTTimer.h) library can be used for highly precise and
 * O.S.-independent time measurements.
 *
 * After intialization, the timer can be started and stopped using
 * RTTimer_start() and RTTimer_stop(). It is also possible to pause and continue.
 * When stopped, a number of things can be queried:
 *  - RTTimer_getElapsed()
 *  - RTTimer_getMinTime(), RTTimer_getMaxTime()
 *  - RTTimer_getAverageTime(), RTTimer_getTotalTime()
 *  - RTTimer_getCount()
 *
 * All results are given in nanoseconds.
 * For every query function there is a second one (e.g. RTTimer_getElapsedExt())
 * to get the time in the RTTimerSpec struct format.
 * Note that the timer must be stopped to query it!
 * The timer does not tolerate irregular calls (e.g. two _stop or two _pause calls).
 *
 * <h3>Example:</h3>
 *
 * \code
 * int main( int argc, char *argv[] )
 * {
 *   RTTimer *rt = RTTimer_new();
 *   char formattedTime[128];
 *
 *   RTTimer_init( rt );
 *
 *   RTTimer_start( rt );
 *
 *   // ... do some processing
 *
 *   RTTimer_pause( rt );
 *   // .. get user interaction
 *   RTTimer_continue( rt );
 *
 *   RTTimer_stop( rt );
 *
 *   RTTimer_format( formattedTime, RTTimer_getElapsed( rt ) );
 *
 *   ANY_LOG(0, "Elapsed time: %s.", ANY_LOG_INFO, formattedTime );
 *
 *   // ...deletion of RTTimer instance, and exit...
 * }
 * \endcode
 */

#ifndef RTTIMER_H
#define RTTIMER_H

#include <Any.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct RTTimer
{
    unsigned long valid;
    bool started;
    bool paused;
    unsigned long long start;
    unsigned long long stop;
    unsigned long long count;
    unsigned long long minTime;
    unsigned long long maxTime;
    unsigned long long totalTime;
}
RTTimer;


typedef struct RTTimerSpec
{
    unsigned long day;
    unsigned long hour;
    unsigned long minute;
    unsigned long second;
    unsigned long microsecond;
}
RTTimerSpec;


#define RTTIMER_MICROSECONDS 1000000ULL
#define RTTIMER_NANOSECONDS 1000000000ULL


RTTimer *RTTimer_new( void );

bool RTTimer_init( RTTimer *self );

/*!
 * \brief Geneneric time function returning the relative time in nanoseconds
 *
 * This function returns a standard representation of the time starting from an unspecified point,
 * expressed as nanoseconds resolution. In order to get the proper elapsed time, the user has
 * to compute the difference from two given values.
 */
unsigned long long RTTimer_getTime( void );

void RTTimer_start( RTTimer *self );

/*!
 * \brief Copy the start time from another RTTimer instance
 * \param self RTTimer instance pointer
 * \param src RTTimer source start timer
 */
void RTTimer_copyStart( RTTimer *self, RTTimer *src );

void RTTimer_pause( RTTimer *self );

void RTTimer_continue( RTTimer *self );

void RTTimer_stop( RTTimer *self );

unsigned long long RTTimer_getElapsed( RTTimer *self );

void RTTimer_getElapsedExt( RTTimer *self, RTTimerSpec *spec );

unsigned long long RTTimer_getMinTime( RTTimer *self );

void RTTimer_getMinTimeExt( RTTimer *self, RTTimerSpec *spec );

unsigned long long RTTimer_getAverageTime( RTTimer *self );

void RTTimer_getAverageTimeExt( RTTimer *self, RTTimerSpec *spec );

unsigned long long RTTimer_getMaxTime( RTTimer *self );

void RTTimer_getMaxTimeExt( RTTimer *self, RTTimerSpec *spec );

unsigned long long RTTimer_getTotalTime( RTTimer *self );

void RTTimer_getTotalTimeExt( RTTimer *self, RTTimerSpec *spec );

/*!
 * \brief Returns how many samples have been counted by the RTTimer
 */
unsigned long long RTTimer_getCount( RTTimer *self );

/*!
 * \brief Returns the absolute start time in nanoseconds
 */
unsigned long long RTTimer_getStartTime( RTTimer *self );

/*!
 * \brief Returns the absolute stop time in nanoseconds
 */
unsigned long long RTTimer_getStopTime( RTTimer *self );

void RTTimer_reset( RTTimer *self );

/*!
 * \brief Convert a nanoseconds representation in a RTTimerSpec readable format
 * \param time Nanoseconds to convert
 * \param spec RTTimerSpec pointer
 */
void RTTimer_micro2RTTimerSpec( unsigned long long time, RTTimerSpec *spec );

/*!
 * \brief Write a formated string from an absolute value expressed in nanoseconds
 * \param buffer Pointer where to store the formatted string
 * \param value Time value expressed in nanoseconds
 *
 * This function take the \a value expressed in nanoseconds, formating it as readable format
 * on the specified \a buffer
 */
void RTTimer_format( char *buffer, double value );

/*!
 * \brief Write a formated string from an RTTimerSpec
 * \param buffer Pointer where to store the formatted string
 * \param spec Extended timer representation
 *
 * This function take and RTTimerSpec representation, formating it as readable format
 * on the specified \a buffer
 */
void RTTimer_formatExt( char *buffer, RTTimerSpec *spec );

void RTTimer_clear( RTTimer *self );

void RTTimer_delete( RTTimer *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
