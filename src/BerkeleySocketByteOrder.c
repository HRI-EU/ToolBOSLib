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


#include <BerkeleySocketByteOrder.h>


BaseI64 BerkeleySocket_ntohI64( BaseI64 value )
{
    BaseI64 result = 0;
    BaseUI8 *value8 = (BaseUI8 *)NULL;
    BaseUI8 *result8 = (BaseUI8 *)NULL;
    BaseUI32 i = 0;

    if( ntohl( 1 ) == 1 )
    {
        result = value;
    }
    else
    {
        value8 = (BaseUI8 *)&value;
        result8 = (BaseUI8 *)&result;

        for( i = 0; i < sizeof( BaseI64 ); ++i )
        {
            result8[ sizeof( BaseI64 ) - 1 - i ] = value8[ i ];
        }
    }

    return result;
}


BaseF64 BerkeleySocket_ntohF64( BaseF64 value )
{
    BaseF64 result = 0;
    BaseUI8 *value8 = (BaseUI8 *)NULL;
    BaseUI8 *result8 = (BaseUI8 *)NULL;
    BaseUI32 i = 0;

    if( ntohl( 1 ) == 1 )
    {
        result = value;
    }
    else
    {
        value8 = (BaseUI8 *)&value;
        result8 = (BaseUI8 *)&result;

        for( i = 0; i < sizeof( BaseI64 ); ++i )
        {
            result8[ sizeof( BaseI64 ) - 1 - i ] = value8[ i ];
        }
    }

    return result;
}


/* EOF */
