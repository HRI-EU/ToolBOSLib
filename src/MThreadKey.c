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
#include <MThreadKey.h>

#define MTHREADKEY_VALID  0x2f6301c0

#define MTHREADKEY_INVALID   0x55272900


MThreadKey *MThreadKey_new( void )
{
    return (ANY_TALLOC( MThreadKey ));
}


bool MThreadKey_init( MThreadKey *self, MThreadKeyDestructor *destructor )
{
    bool retVal = false;

    ANY_REQUIRE( self );

    self->valid = MTHREADKEY_INVALID;

    if( pthread_key_create( &self->key, destructor ) == 0 )
    {
        retVal = true;
        self->valid = MTHREADKEY_VALID;
    }

    return retVal;
}


bool MThreadKey_set( MThreadKey *self, void *value )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTHREADKEY_VALID );

    return ( pthread_setspecific( self->key, value ) == 0 ? true : false );
}


void *MThreadKey_get( MThreadKey *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTHREADKEY_VALID );

    return ( pthread_getspecific( self->key ));
}


void MThreadKey_clear( MThreadKey *self )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTHREADKEY_VALID );

    self->valid = MTHREADKEY_INVALID;

    status = pthread_key_delete( self->key );
    ANY_REQUIRE_MSG( status == 0, "Unable to destroy a pthread_key" );
}


void MThreadKey_delete( MThreadKey *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}



