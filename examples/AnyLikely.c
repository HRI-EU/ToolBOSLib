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


// output:
//
//[602977.245349 4c90:0 AnyLikely.c:102 Info] i =      0
//[602977.245373 4c90:0 AnyLikely.c:102 Info] i =      1
//[602977.245377 4c90:0 AnyLikely.c:102 Info] i =      2
//[602977.245379 4c90:0 AnyLikely.c:102 Info] i =      3
//[602977.245381 4c90:0 AnyLikely.c:102 Info] i =      4
//[602977.245384 4c90:0 AnyLikely.c:102 Info] i =      5
//                      ...
//[602979.723519 4c90:0 AnyLikely.c:102 Info] i = 999990
//[602979.723521 4c90:0 AnyLikely.c:102 Info] i = 999991
//[602979.723523 4c90:0 AnyLikely.c:102 Info] i = 999993
//[602979.723526 4c90:0 AnyLikely.c:102 Info] i = 999994


#include <Any.h>
#include <stdlib.h>


/* NORMAL VERSION == WITHOUT BRANCH PREDICTION */

//int main( int argc, char *argv[] )
//{
//    for( int i = 0; i < 1000000; i++ )
//    {
//
//        // print in the likely case only, skip printing within last iterations
//
//        if( i < 999995 )
//        {
//
//            // however, as exception, omit printing if i == 999992
//
//            if( i != 999992 )
//            {
//                ANY_LOG( 0, "i = %6d", ANY_LOG_INFO, i );
//            }
//        }
//    }
//
//    return( EXIT_SUCCESS );
//}


/* OPTIMIZED VERSION == WITH BRANCH PREDICTION */
/* on my PC in average 8 % faster than original version */

int main( int argc, char *argv[] )
{
    for( int i = 0; i < 1000000; i++ )
    {

        // print in the likely case only, skip printing within last iterations

        if( ANY_LIKELY( i < 999995 ) )
        {

            // however, as exception, omit printing if i == 999992

            if( ANY_UNLIKELY( i != 999992 ) )
            {
                ANY_LOG( 0, "i = %6d", ANY_LOG_INFO, i );
            }
        }
    }

    return( EXIT_SUCCESS );
}


/* EOF */
