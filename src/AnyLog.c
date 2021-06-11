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


#include <AnyLog.h>
#include <AnyDef.h>
#include <AnyMem.h>

#if defined(__msvc__)

#include <stdio.h>

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#include <time.h>

// for calculations in get time of day
#define EPOCHFILETIME (116444736000000000i64)

#else

// for nanosleep
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif

#include <time.h>
#include <errno.h>

#endif


/*
 * Current debug level.
 * All the message with a debug level below the current level are displayed.
 * The debugLevel variable takes value from 0 to MAXINT.
 * Message with debugLevel 0 should be always displayed.
 * Please do not modify this variable directly but use get/set functions.
 */
int Any_debugLevel = ANY_LOG_DEBUGLEVEL_DEFAULT;
int Any_minDebugLevel = ANY_LOG_MIN_DEBUGLEVEL_DEFAULT;
bool longLogFormat = true;
/* bool Any_veryShortLogFormat = false; */

/*
 * this object hold all the callbacks setted for the ANY_REQUIRE()
 */
static AnyEventInfo *Any_infoRequire = NULL;

/*
 * functions
 */

ANY_INLINE void Any_setDebugLevel( int newlevel )
{
    Any_debugLevel = ( newlevel >= 0 ? newlevel : 0 );
}


ANY_INLINE int Any_getDebugLevel( void )
{
    return ( Any_debugLevel );
}


ANY_INLINE void Any_setMinDebugLevel( int newlevel )
{
    Any_minDebugLevel = ( newlevel >= 0 ? newlevel : 0 );
}


ANY_INLINE int Any_getMinDebugLevel( void )
{
    return ( Any_minDebugLevel );
}


ANY_INLINE void Any_setLongLogFormat( void )
{
    longLogFormat = true;
}


ANY_INLINE void Any_setShortLogFormat( void )
{
    longLogFormat = false;
}


ANY_INLINE void Any_onRequire( void (*function)( void * ), void *functionParam )
{
    AnyEventInfo *info = NULL;

    ANY_REQUIRE( function );

    info = ANY_TALLOC( AnyEventInfo );

    if( ANY_LIKELY( info ))
    {
        info->function = function;
        info->functionParam = functionParam;
        info->next = Any_infoRequire;
        Any_infoRequire = info;
    }
    else
    {
        ANY_LOG( 0, "Unable to allocate an AnyEventInfo struct", ANY_LOG_ERROR );
    }
}


ANY_INLINE void Any_fireRequire( void )
{
    if( Any_infoRequire )
    {
        AnyEventInfo *info = Any_infoRequire;

        while( info )
        {
            if( info->function )
            {
                ( *info->function )( info->functionParam );
            }
            info = info->next;
        }
    }
}

/* EOF */
