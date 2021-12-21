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
#include <IOChannel.h>


int main( int argc, char *argv[] )
{
    IOChannel *channel = IOChannel_new();

    IOChannel_init( channel );

    IOChannel_open( channel,
                    "File://testData~",
                    IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                    IOCHANNEL_PERMISSIONS_W_U | IOCHANNEL_PERMISSIONS_R_U );

    IOChannel_close( channel );
    IOChannel_clear( channel );
    IOChannel_delete( channel );

    return EXIT_SUCCESS;
}


/* EOF */
