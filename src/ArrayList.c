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


#include <ArrayList.h>


void ArrayList_allocate( ArrayList *self )
{
    int newSize;
    void **newBuffer;

    ANY_REQUIRE( self );

    newSize = self->count * 2;
    if( newSize == 0 ) newSize = 50;

    newBuffer = (void **)ANY_BALLOC( newSize * sizeof( void * ));
    ANY_REQUIRE( newBuffer );

    if( self->buffer )
    {
        ANY_REQUIRE( self->count < newSize );
        //ANY_LOG(0,"ArrayList_allocate: memcpy", ANY_LOG_INFO);
        Any_memcpy( newBuffer, self->buffer, self->count * sizeof( void * ));
        ANY_FREE( self->buffer );

        self->buffer = newBuffer;
    }
    self->buffer = newBuffer;
    self->free = newSize - self->count;
}


ArrayList *ArrayList_new()
{
    return (ANY_TALLOC( ArrayList ));
}


void ArrayList_delete( ArrayList *self )
{
    ANY_FREE( self );
}


void ArrayList_init( ArrayList *self )
{
    bool status;

    ANY_REQUIRE( self );

    self->free = 0;
    self->count = 0;
    self->buffer = NULL;

    self->mutex = Mutex_new();
    ANY_REQUIRE( self->mutex );

    status = Mutex_init( self->mutex, MUTEX_PRIVATE );
    ANY_REQUIRE( status == true );

    ANY_VALID_SET( self, ArrayList );
}


void ArrayList_clear( ArrayList *self )
{
    ANY_REQUIRE( self );

    //ANY_LOG(0,"ArrayList_allocate: clear", ANY_LOG_INFO);
    ANY_FREE( self->buffer );
    self->buffer = NULL;
    self->free = 0;
    self->count = 0;

    Mutex_clear( self->mutex );
    Mutex_delete( self->mutex );
    self->mutex = NULL;
}


int ArrayList_length( ArrayList *self )
{
    int returnValue;
    int status;

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    returnValue = self->count;

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );

    return returnValue;
}


void ArrayList_push( ArrayList *self, void *item )
{
    int status;

    ANY_REQUIRE( self );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    if( self->free <= 0 )
    {
        ArrayList_allocate( self );
    }

    self->buffer[ self->count++ ] = item;
    self->free--;

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );
}


void *ArrayList_pop( ArrayList *self )
{
    void *returnValue = 0;
    int status;

    ANY_REQUIRE( self );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    if( self->count > 0 )
    {
        returnValue = self->buffer[ --self->count ];
        self->free++;
    }

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );

    return returnValue;
}


void ArrayList_remove( ArrayList *self, void *item )
{
    int i;
    int status;

    ANY_REQUIRE( self );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    for( i = 0; i < self->count; i++ )
    {
        if( self->buffer[ i ] == item )
        {
            Any_memmove( &self->buffer[ i ], &self->buffer[ i + 1 ], self->count - i );
            self->count--;
            self->free++;
            break;
        }
    }

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );
}


void *ArrayList_get( ArrayList *self, int index )
{
    int status;
    void *returnValue = NULL;

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    if( index >= 0 && index < self->count )
    {
        returnValue = self->buffer[ index ];
    }

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );

    return returnValue;
}


void ArrayList_reset( ArrayList *self )
{
    int status;

    ANY_REQUIRE( self );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    self->free += self->count;
    self->count = 0;

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );
}

