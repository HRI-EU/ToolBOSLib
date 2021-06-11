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


#ifndef LOGTYPE_H
#define LOGTYPE_H

#include <Any.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    MSG_TYPE_Unknown,
    MSG_TYPE_Incomplete,
    MSG_TYPE_Trace,
    MSG_TYPE_Info,
    MSG_TYPE_Warning,
    MSG_TYPE_Error,
    MSG_TYPE_Fatal,
    MSG_TYPE_Data,
    MSG_TYPE_DataCheck,
    MSG_TYPE_UserInfo,
    MSG_TYPE_Cached,
    MSG_TYPE_Require,
    MSG_TYPE_TypeCount
} LogType;

extern const char *LogTypeDictionary[MSG_TYPE_TypeCount];

#define Any_logTypeToString( logType ) LogType_toString( logType )
#define Any_logTypeFromString( logType ) LogType_fromString( logType )

/*!
 * \brief  Returns the textual LogMessage type
 * \param  logType the message type ID
 * \return the message type string or 'Unknown' if the id can not be
 *         resolved
 */
const char *LogType_toString( LogType logType );

/*!
 * \brief  Returns the ID to a textual LogMessage type
 * \param  logType the log type as string
 * \return the ID to the string. If the string is unknown MSG_Unknown will be returned
 */
LogType LogType_fromString( const char *logType );

#if defined(__cplusplus)
}
#endif

#endif // LOGTYPE_H
