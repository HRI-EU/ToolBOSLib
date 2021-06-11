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


/*--------------------------------------------------------------------------*/
/* Private includes                                                         */
/*--------------------------------------------------------------------------*/

#include <BBDMSerialize.h>


/*--------------------------------------------------------------------------*/
/* Public functions                                                         */
/*--------------------------------------------------------------------------*/

void BBDMTag_serialize( BBDMTag *self, char *name, Serialize *stream )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( stream );

    Serialize_beginType( stream,
                         name == (char *)NULL ? (char *)"tag" : name,
                         (char *)"BBDMTag" );

    // we decided to not (yet) serialize the instance name

    BaseI64_serialize( &( self->timestep ), (char *)"timestep", stream );

    Serialize_endType( stream );
}


/* EOF */
