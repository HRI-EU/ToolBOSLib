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
#include <stdlib.h>


int main( int argc, char *argv[] )
{
    ANY_TRACE( 0, "%d", argc );


    if( argc > 1 )
    {
        ANY_LOG( 0, "msg=%s (from commandline)", ANY_LOG_INFO, argv[1] );
    }
    else
    {
        ANY_LOG( 0, "msg=%s (default)", ANY_LOG_INFO, "Hello, World!" );
    }


    return( EXIT_SUCCESS );
}


/* EOF */
