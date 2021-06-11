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


#include <MemX.h>
#include <Any.h>


MemI8 *MemI8_new( void )
{
    MemI8 *self = (MemI8 *)NULL;

    self = ANY_TALLOC( MemI8 );
    ANY_REQUIRE( self );

    return self;
}


int MemI8_init( MemI8 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseI8 ));

    if( self->buffer == (BaseI8 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemI8_copy( MemI8 *self, const MemI8 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );
    ANY_REQUIRE( self->buffer );
    ANY_REQUIRE( src->buffer );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


int MemI8_copyConstr( MemI8 *self, const MemI8 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemI8_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


BaseI8 *MemI8_getBuffer( MemI8 *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    return self->buffer;
}


const BaseI8 *MemI8_getConstBuffer( const MemI8 *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    return self->buffer;
}


BaseUI32 MemI8_getLength( const MemI8 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemI8_clear( MemI8 *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    ANY_FREE( self->buffer );

    self->buffer = (BaseI8 *)NULL;
    self->length = 0;
}


void MemI8_delete( MemI8 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemUI8 *MemUI8_new( void )
{
    MemUI8 *self = (MemUI8 *)NULL;

    self = ANY_TALLOC( MemUI8 );
    ANY_REQUIRE( self );

    return self;
}


int MemUI8_init( MemUI8 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseUI8 ));

    if( self->buffer == (BaseUI8 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemUI8_copyConstr( MemUI8 *self, const MemUI8 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemUI8_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemUI8_copy( MemUI8 *self, const MemUI8 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseUI8 *MemUI8_getBuffer( MemUI8 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseUI8 *MemUI8_getConstBuffer( const MemUI8 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemUI8_getLength( const MemUI8 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemUI8_clear( MemUI8 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseUI8 *)NULL;
    self->length = 0;
}


void MemUI8_delete( MemUI8 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemI16 *MemI16_new( void )
{
    MemI16 *self = (MemI16 *)NULL;

    self = ANY_TALLOC( MemI16 );
    ANY_REQUIRE( self );

    return self;
}


int MemI16_init( MemI16 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseI16 ));

    if( self->buffer == (BaseI16 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemI16_copyConstr( MemI16 *self, const MemI16 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemI16_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemI16_copy( MemI16 *self, const MemI16 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseI16 *MemI16_getBuffer( MemI16 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseI16 *MemI16_getConstBuffer( const MemI16 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemI16_getLength( const MemI16 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemI16_clear( MemI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseI16 *)NULL;
    self->length = 0;
}


void MemI16_delete( MemI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemUI16 *MemUI16_new( void )
{
    MemUI16 *self = (MemUI16 *)NULL;

    self = ANY_TALLOC( MemUI16 );
    ANY_REQUIRE( self );

    return self;
}


int MemUI16_init( MemUI16 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseUI16 ));

    if( self->buffer == (BaseUI16 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemUI16_copyConstr( MemUI16 *self, const MemUI16 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemUI16_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemUI16_copy( MemUI16 *self, const MemUI16 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseUI16 *MemUI16_getBuffer( MemUI16 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseUI16 *MemUI16_getConstBuffer( const MemUI16 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemUI16_getLength( const MemUI16 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemUI16_clear( MemUI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseUI16 *)NULL;
    self->length = 0;
}


void MemUI16_delete( MemUI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemI32 *MemI32_new( void )
{
    MemI32 *self = (MemI32 *)NULL;

    self = ANY_TALLOC( MemI32 );
    ANY_REQUIRE( self );

    return self;
}


int MemI32_init( MemI32 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseI32 ));

    if( self->buffer == (BaseI32 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemI32_copyConstr( MemI32 *self, const MemI32 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemI32_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemI32_copy( MemI32 *self, const MemI32 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseI32 *MemI32_getBuffer( MemI32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseI32 *MemI32_getConstBuffer( const MemI32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemI32_getLength( const MemI32 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemI32_clear( MemI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseI32 *)NULL;
    self->length = 0;
}


void MemI32_delete( MemI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemUI32 *MemUI32_new( void )
{
    MemUI32 *self = (MemUI32 *)NULL;

    self = ANY_TALLOC( MemUI32 );
    ANY_REQUIRE( self );

    return self;
}


int MemUI32_init( MemUI32 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseUI32 ));

    if( self->buffer == (BaseUI32 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemUI32_copyConstr( MemUI32 *self, const MemUI32 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemUI32_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemUI32_copy( MemUI32 *self, const MemUI32 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseUI32 *MemUI32_getBuffer( MemUI32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseUI32 *MemUI32_getConstBuffer( const MemUI32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemUI32_getLength( const MemUI32 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemUI32_clear( MemUI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseUI32 *)NULL;
    self->length = 0;
}


void MemUI32_delete( MemUI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemF32 *MemF32_new( void )
{
    MemF32 *self = (MemF32 *)NULL;

    self = ANY_TALLOC( MemF32 );
    ANY_REQUIRE( self );

    return self;
}


int MemF32_init( MemF32 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseF32 ));

    if( self->buffer == (BaseF32 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemF32_copyConstr( MemF32 *self, const MemF32 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemF32_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemF32_copy( MemF32 *self, const MemF32 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseF32 *MemF32_getBuffer( MemF32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseF32 *MemF32_getConstBuffer( const MemF32 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemF32_getLength( const MemF32 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemF32_clear( MemF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseF32 *)NULL;
    self->length = 0;
}


void MemF32_delete( MemF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


MemF64 *MemF64_new( void )
{
    MemF64 *self = (MemF64 *)NULL;

    self = ANY_TALLOC( MemF64 );
    ANY_REQUIRE( self );

    return self;
}


int MemF64_init( MemF64 *self, BaseUI32 length )
{
    ANY_REQUIRE( self );

    self->length = length;
    self->buffer = ANY_MALLOC( length, sizeof( BaseF64 ));

    if( self->buffer == (BaseF64 *)NULL)
    {
        return -1;
    }

    return 0;
}


int MemF64_copyConstr( MemF64 *self, const MemF64 *src )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( src->buffer );

    MemF64_init( self, src->length );
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->buffer );

    self->length = src->length;
    memcpy( self->buffer, src->buffer, src->length );

    return (( self->length == src->length ) &&
            ( memcmp( self->buffer, src->buffer, self->length ) == 0 )
            ? 0 : -1 );
}


int MemF64_copy( MemF64 *self, const MemF64 *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    if( self->length < src->length )
    {
        return -1;
    }

    memcpy( self->buffer, src->buffer, src->length );
    return 0;
}


BaseF64 *MemF64_getBuffer( MemF64 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


const BaseF64 *MemF64_getConstBuffer( const MemF64 *self )
{
    ANY_REQUIRE( self );

    return self->buffer;
}


BaseUI32 MemF64_getLength( const MemF64 *self )
{
    ANY_REQUIRE( self );

    return self->length;
}


void MemF64_clear( MemF64 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self->buffer );

    self->buffer = (BaseF64 *)NULL;
    self->length = 0;
}


void MemF64_delete( MemF64 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* EOF */
