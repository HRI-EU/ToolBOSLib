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


/* some BaseXY_clear() functions are empty but take a "self" for consistency */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <Any.h>
#include <BaseTypes.h>
#include <Base2DX.h>


Base2DSize *Base2DSize_new( void )
{
    Base2DSize *self = (Base2DSize *)NULL;

    self = ANY_TALLOC( Base2DSize );
    ANY_REQUIRE( self );

    return self;
}


int Base2DSize_init( Base2DSize *self )
{
    ANY_REQUIRE( self );

    self->width = 0;
    self->height = 0;

    return 0;
}


Base2DSize *Base2DSize_copy( Base2DSize *self, const Base2DSize *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    self->width = src->width;
    self->height = src->height;

    return self;
}


bool Base2DSize_isEQ( const Base2DSize *self, const Base2DSize *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return ( self->width == src->width ) &&
           ( self->height == src->height );
}


bool Base2DSize_isNE( const Base2DSize *self, const Base2DSize *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return !Base2DSize_isEQ( self, src );
}


void Base2DSize_clear( Base2DSize *self )
{
    ANY_REQUIRE( self );

    self->width = 0;
    self->height = 0;
}


void Base2DSize_delete( Base2DSize *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* Base2DPoint */

Base2DPoint *Base2DPoint_new( void )
{
    Base2DPoint *self = (Base2DPoint *)NULL;

    self = ANY_TALLOC( Base2DPoint );
    ANY_REQUIRE( self );

    return self;
}


int Base2DPoint_init( Base2DPoint *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;

    return 0;
}


Base2DPoint *Base2DPoint_copy( Base2DPoint *self, const Base2DPoint *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    self->x = src->x;
    self->y = src->y;

    return self;
}


bool Base2DPoint_isEQ( const Base2DPoint *self, const Base2DPoint *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return ( self->x == src->x ) &&
           ( self->y == src->y );
}


bool Base2DPoint_isNE( const Base2DPoint *self, const Base2DPoint *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return !Base2DPoint_isEQ( self, src );
}


void Base2DPoint_clear( Base2DPoint *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
}


void Base2DPoint_delete( Base2DPoint *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* Base2DFloat */

Base2DFloat *Base2DFloat_new( void )
{
    Base2DFloat *self = (Base2DFloat *)NULL;

    self = ANY_TALLOC( Base2DFloat );
    ANY_REQUIRE( self );

    return self;
}


int Base2DFloat_init( Base2DFloat *self )
{
    ANY_REQUIRE( self );

    self->x = 0.0;
    self->y = 0.0;

    return 0;
}


Base2DFloat *Base2DFloat_copy( Base2DFloat *self, const Base2DFloat *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    self->x = src->x;
    self->y = src->y;

    return self;
}


void Base2DFloat_clear( Base2DFloat *self )
{
    ANY_REQUIRE( self );

    self->x = 0.0;
    self->y = 0.0;
}


void Base2DFloat_delete( Base2DFloat *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* Base2DRect */

Base2DRect *Base2DRect_new( void )
{
    Base2DRect *self = (Base2DRect *)NULL;

    self = ANY_TALLOC( Base2DRect );
    ANY_REQUIRE( self );

    return self;
}


int Base2DRect_init( Base2DRect *self )
{
    ANY_REQUIRE( self );

    Base2DPoint_init( &( self->upperLeft ));
    Base2DSize_init( &( self->size ));

    return 0;
}


Base2DRect *Base2DRect_copy( Base2DRect *self, const Base2DRect *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    Base2DPoint_copy( &self->upperLeft, &src->upperLeft );
    Base2DSize_copy( &self->size, &src->size );

    return self;
}


bool Base2DRect_isEQ( const Base2DRect *self, const Base2DRect *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return ( Base2DPoint_isEQ( &self->upperLeft, &src->upperLeft )) &&
           ( Base2DSize_isEQ( &self->size, &src->size ));
}


bool Base2DRect_isNE( const Base2DRect *self, const Base2DRect *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    return !Base2DRect_isEQ( self, src );
}


void Base2DRect_clear( Base2DRect *self )
{
    ANY_REQUIRE( self );

    Base2DPoint_clear( &( self->upperLeft ));
    Base2DSize_clear( &( self->size ));
}


void Base2DRect_delete( Base2DRect *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


Base1DI8 *Base1DI8_new( void )
{
    Base1DI8 *self = (Base1DI8 *)NULL;

    self = ANY_TALLOC( Base1DI8 );
    ANY_REQUIRE_MSG( self, "Base1DI8_new(): memory allocation failed" );

    return self;
}


Base2DI8 *Base2DI8_new( void )
{
    Base2DI8 *self = (Base2DI8 *)NULL;

    self = ANY_TALLOC( Base2DI8 );
    ANY_REQUIRE_MSG( self, "Base2DI8_new(): memory allocation failed" );

    return self;
}


Base3DI8 *Base3DI8_new( void )
{
    Base3DI8 *self = (Base3DI8 *)NULL;

    self = ANY_TALLOC( Base3DI8 );
    ANY_REQUIRE_MSG( self, "Base3DI8_new(): memory allocation failed" );

    return self;
}


Base4DI8 *Base4DI8_new( void )
{
    Base4DI8 *self = (Base4DI8 *)NULL;

    self = ANY_TALLOC( Base4DI8 );
    ANY_REQUIRE_MSG( self, "Base4DI8_new(): memory allocation failed" );

    return self;
}


Base1DUI8 *Base1DUI8_new( void )
{
    Base1DUI8 *self = (Base1DUI8 *)NULL;

    self = ANY_TALLOC( Base1DUI8 );
    ANY_REQUIRE_MSG( self, "Base1DUI8_new(): memory allocation failed" );

    return self;
}


Base2DUI8 *Base2DUI8_new( void )
{
    Base2DUI8 *self = (Base2DUI8 *)NULL;

    self = ANY_TALLOC( Base2DUI8 );
    ANY_REQUIRE_MSG( self, "Base2DUI8_new(): memory allocation failed" );

    return self;
}


Base3DUI8 *Base3DUI8_new( void )
{
    Base3DUI8 *self = (Base3DUI8 *)NULL;

    self = ANY_TALLOC( Base3DUI8 );
    ANY_REQUIRE_MSG( self, "Base3DUI8_new(): memory allocation failed" );

    return self;
}


Base4DUI8 *Base4DUI8_new( void )
{
    Base4DUI8 *self = (Base4DUI8 *)NULL;

    self = ANY_TALLOC( Base4DUI8 );
    ANY_REQUIRE_MSG( self, "Base4DUI8_new(): memory allocation failed" );

    return self;
}


Base1DI16 *Base1DI16_new( void )
{
    Base1DI16 *self = (Base1DI16 *)NULL;

    self = ANY_TALLOC( Base1DI16 );
    ANY_REQUIRE_MSG( self, "Base1DI16_new(): memory allocation failed" );

    return self;
}


Base2DI16 *Base2DI16_new( void )
{
    Base2DI16 *self = (Base2DI16 *)NULL;

    self = ANY_TALLOC( Base2DI16 );
    ANY_REQUIRE_MSG( self, "Base2DI16_new(): memory allocation failed" );

    return self;
}


Base3DI16 *Base3DI16_new( void )
{
    Base3DI16 *self = (Base3DI16 *)NULL;

    self = ANY_TALLOC( Base3DI16 );
    ANY_REQUIRE_MSG( self, "Base3DI16_new(): memory allocation failed" );

    return self;
}


Base4DI16 *Base4DI16_new( void )
{
    Base4DI16 *self = (Base4DI16 *)NULL;

    self = ANY_TALLOC( Base4DI16 );
    ANY_REQUIRE_MSG( self, "Base4DI16_new(): memory allocation failed" );

    return self;
}


Base1DUI16 *Base1DUI16_new( void )
{
    Base1DUI16 *self = (Base1DUI16 *)NULL;

    self = ANY_TALLOC( Base1DUI16 );
    ANY_REQUIRE_MSG( self, "Base1DUI16_new(): memory allocation failed" );

    return self;
}


Base2DUI16 *Base2DUI16_new( void )
{
    Base2DUI16 *self = (Base2DUI16 *)NULL;

    self = ANY_TALLOC( Base2DUI16 );
    ANY_REQUIRE_MSG( self, "Base2DUI16_new(): memory allocation failed" );

    return self;
}


Base3DUI16 *Base3DUI16_new( void )
{
    Base3DUI16 *self = (Base3DUI16 *)NULL;

    self = ANY_TALLOC( Base3DUI16 );
    ANY_REQUIRE_MSG( self, "Base3DUI16_new(): memory allocation failed" );

    return self;
}


Base4DUI16 *Base4DUI16_new( void )
{
    Base4DUI16 *self = (Base4DUI16 *)NULL;

    self = ANY_TALLOC( Base4DUI16 );
    ANY_REQUIRE_MSG( self, "Base4DUI16_new(): memory allocation failed" );

    return self;
}


Base1DI32 *Base1DI32_new( void )
{
    Base1DI32 *self = (Base1DI32 *)NULL;

    self = ANY_TALLOC( Base1DI32 );
    ANY_REQUIRE_MSG( self, "Base1DI32_new(): memory allocation failed" );

    return self;
}


Base2DI32 *Base2DI32_new( void )
{
    Base2DI32 *self = (Base2DI32 *)NULL;

    self = ANY_TALLOC( Base2DI32 );
    ANY_REQUIRE_MSG( self, "Base2DI32_new(): memory allocation failed" );

    return self;
}


Base3DI32 *Base3DI32_new( void )
{
    Base3DI32 *self = (Base3DI32 *)NULL;

    self = ANY_TALLOC( Base3DI32 );
    ANY_REQUIRE_MSG( self, "Base3DI32_new(): memory allocation failed" );

    return self;
}


Base4DI32 *Base4DI32_new( void )
{
    Base4DI32 *self = (Base4DI32 *)NULL;

    self = ANY_TALLOC( Base4DI32 );
    ANY_REQUIRE_MSG( self, "Base4DI32_new(): memory allocation failed" );

    return self;
}


Base1DUI32 *Base1DUI32_new( void )
{
    Base1DUI32 *self = (Base1DUI32 *)NULL;

    self = ANY_TALLOC( Base1DUI32 );
    ANY_REQUIRE_MSG( self, "Base1DUI32_new(): memory allocation failed" );

    return self;
}


Base2DUI32 *Base2DUI32_new( void )
{
    Base2DUI32 *self = (Base2DUI32 *)NULL;

    self = ANY_TALLOC( Base2DUI32 );
    ANY_REQUIRE_MSG( self, "Base2DUI32_new(): memory allocation failed" );

    return self;
}


Base3DUI32 *Base3DUI32_new( void )
{
    Base3DUI32 *self = (Base3DUI32 *)NULL;

    self = ANY_TALLOC( Base3DUI32 );
    ANY_REQUIRE_MSG( self, "Base3DUI32_new(): memory allocation failed" );

    return self;
}


Base4DUI32 *Base4DUI32_new( void )
{
    Base4DUI32 *self = (Base4DUI32 *)NULL;

    self = ANY_TALLOC( Base4DUI32 );
    ANY_REQUIRE_MSG( self, "Base4DUI32_new(): memory allocation failed" );

    return self;
}


Base1DF32 *Base1DF32_new( void )
{
    Base1DF32 *self = (Base1DF32 *)NULL;

    self = ANY_TALLOC( Base1DF32 );
    ANY_REQUIRE_MSG( self, "Base1DF32_new(): memory allocation failed" );

    return self;
}


Base2DF32 *Base2DF32_new( void )
{
    Base2DF32 *self = (Base2DF32 *)NULL;

    self = ANY_TALLOC( Base2DF32 );
    ANY_REQUIRE_MSG( self, "Base2DF32_new(): memory allocation failed" );

    return self;
}


Base3DF32 *Base3DF32_new( void )
{
    Base3DF32 *self = (Base3DF32 *)NULL;

    self = ANY_TALLOC( Base3DF32 );
    ANY_REQUIRE_MSG( self, "Base3DF32_new(): memory allocation failed" );

    return self;
}


Base4DF32 *Base4DF32_new( void )
{
    Base4DF32 *self = (Base4DF32 *)NULL;

    self = ANY_TALLOC( Base4DF32 );
    ANY_REQUIRE_MSG( self, "Base4DF32_new(): memory allocation failed" );

    return self;
}


Base1DF64 *Base1DF64_new( void )
{
    Base1DF64 *self = (Base1DF64 *)NULL;

    self = ANY_TALLOC( Base1DF64 );
    ANY_REQUIRE_MSG( self, "Base1DF64_new(): memory allocation failed" );

    return self;
}


Base2DF64 *Base2DF64_new( void )
{
    Base2DF64 *self = (Base2DF64 *)NULL;

    self = ANY_TALLOC( Base2DF64 );
    ANY_REQUIRE_MSG( self, "Base2DF64_new(): memory allocation failed" );

    return self;
}


Base3DF64 *Base3DF64_new( void )
{
    Base3DF64 *self = (Base3DF64 *)NULL;

    self = ANY_TALLOC( Base3DF64 );
    ANY_REQUIRE_MSG( self, "Base3DF64_new(): memory allocation failed" );

    return self;
}


Base4DF64 *Base4DF64_new( void )
{
    Base4DF64 *self = (Base4DF64 *)NULL;

    self = ANY_TALLOC( Base4DF64 );
    ANY_REQUIRE_MSG( self, "Base4DF64_new(): memory allocation failed" );

    return self;
}


Base1DI32vF32 *Base1DI32vF32_new( void )
{
    Base1DI32vF32 *self = (Base1DI32vF32 *)NULL;

    self = ANY_TALLOC( Base1DI32vF32 );
    ANY_REQUIRE_MSG( self, "Base1DI32vF32_new(): memory allocation failed" );

    return self;
}


Base2DI32vF32 *Base2DI32vF32_new( void )
{
    Base2DI32vF32 *self = (Base2DI32vF32 *)NULL;

    self = ANY_TALLOC( Base2DI32vF32 );
    ANY_REQUIRE_MSG( self, "Base2DI32vF32_new(): memory allocation failed" );

    return self;
}


Base3DI32vF32 *Base3DI32vF32_new( void )
{
    Base3DI32vF32 *self = (Base3DI32vF32 *)NULL;

    self = ANY_TALLOC( Base3DI32vF32 );
    ANY_REQUIRE_MSG( self, "Base3DI32vF32_new(): memory allocation failed" );

    return self;
}


Base4DI32vF32 *Base4DI32vF32_new( void )
{
    Base4DI32vF32 *self = (Base4DI32vF32 *)NULL;

    self = ANY_TALLOC( Base4DI32vF32 );
    ANY_REQUIRE_MSG( self, "Base4DI32vF32_new(): memory allocation failed" );

    return self;
}


void Base1DI32_init( Base1DI32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
}


void Base2DI32_init( Base2DI32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
}


void Base3DI32_init( Base3DI32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
}


void Base4DI32_init( Base4DI32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
    self->t = 0;
}


void Base1DF32_init( Base1DF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
}


void Base2DF32_init( Base2DF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
}


void Base3DF32_init( Base3DF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
}


void Base4DF32_init( Base4DF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
    self->t = 0;
}


void Base1DI32vF32_init( Base1DI32vF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->v = 0;
}


void Base2DI32vF32_init( Base2DI32vF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->v = 0;
}


void Base3DI32vF32_init( Base3DI32vF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
    self->v = 0;
}


void Base4DI32vF32_init( Base4DI32vF32 *self )
{
    ANY_REQUIRE( self );

    self->x = 0;
    self->y = 0;
    self->z = 0;
    self->t = 0;
    self->v = 0;
}


void Base1DI32_clear( Base1DI32 *self )
{
    ANY_OPTIONAL( self );
}


void Base2DI32_clear( Base2DI32 *self )
{
    ANY_OPTIONAL( self );
}


void Base3DI32_clear( Base3DI32 *self )
{
    ANY_OPTIONAL( self );
}


void Base4DI32_clear( Base4DI32 *self )
{
    ANY_OPTIONAL( self );
}


void Base1DF32_clear( Base1DF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base2DF32_clear( Base2DF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base3DF32_clear( Base3DF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base4DF32_clear( Base4DF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base1DI32vF32_clear( Base1DI32vF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base2DI32vF32_clear( Base2DI32vF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base3DI32vF32_clear( Base3DI32vF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base4DI32vF32_clear( Base4DI32vF32 *self )
{
    ANY_OPTIONAL( self );
}


void Base1DI32_delete( Base1DI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base2DI32_delete( Base2DI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base3DI32_delete( Base3DI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base4DI32_delete( Base4DI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base1DF32_delete( Base1DF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base2DF32_delete( Base2DF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base3DF32_delete( Base3DF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base4DF32_delete( Base4DF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base1DI32vF32_delete( Base1DI32vF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base2DI32vF32_delete( Base2DI32vF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base3DI32vF32_delete( Base3DI32vF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base4DI32vF32_delete( Base4DI32vF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void Base1DI32_copy( Base1DI32 *src, Base1DI32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
}


void Base2DI32_copy( Base2DI32 *src, Base2DI32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
}


void Base3DI32_copy( Base3DI32 *src, Base3DI32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
}


void Base4DI32_copy( Base4DI32 *src, Base4DI32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->t = src->t;
}


void Base1DF32_copy( Base1DF32 *src, Base1DF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
}


void Base2DF32_copy( Base2DF32 *src, Base2DF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
}


void Base3DF32_copy( Base3DF32 *src, Base3DF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
}


void Base4DF32_copy( Base4DF32 *src, Base4DF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->t = src->t;
}


void Base1DI32vF32_copy( Base1DI32vF32 *src, Base1DI32vF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->v = src->v;
}


void Base2DI32vF32_copy( Base2DI32vF32 *src, Base2DI32vF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->v = src->v;
}


void Base3DI32vF32_copy( Base3DI32vF32 *src, Base3DI32vF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->v = src->v;
}


void Base4DI32vF32_copy( Base4DI32vF32 *src, Base4DI32vF32 *dst )
{
    ANY_REQUIRE( src );
    ANY_REQUIRE( dst );

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->t = src->t;
    dst->v = src->v;
}


/* EOF */
