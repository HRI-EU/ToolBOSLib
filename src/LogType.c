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
#include <LogType.h>


static const char *const LogType_dictionary[MSG_TYPE_TypeCount] =
        {
                "Unknown",
                "Incomplete",
                "Trace",
                ANY_LOG_INFO,
                ANY_LOG_WARNING,
                ANY_LOG_ERROR,
                ANY_LOG_FATAL,
                ANY_LOG_DATA,
                ANY_LOG_DATA_CHECK,
                "UserInfo",
                "Cached",
                "Require"
        };


LogType LogType_fromString( const char *logType )
{
    int returnValue = MSG_TYPE_Unknown;
    int i = 0;

    ANY_REQUIRE( logType );

    for( i = 0; i < MSG_TYPE_TypeCount; i++ )
    {
        if( Any_strcasecmp( LogType_dictionary[ i ], logType ) == 0 )
        {
            returnValue = i;
            break;
        }
    }
    return returnValue;
}


const char *LogType_toString( LogType logType )
{
    const char *returnValue;
    if( logType < MSG_TYPE_TypeCount )
    {
        returnValue = LogType_dictionary[ logType ];
    }
    else
    {
        returnValue = LogType_dictionary[ MSG_TYPE_Unknown ];
    }
    return returnValue;
}

// END LOGTYPE_C
