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


#ifndef BASE2DX_H
#define BASE2DX_H


#include <Any.h>
#include <BaseTypes.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct Base2DPoint
{
    BaseI32 x;
    BaseI32 y;
}
Base2DPoint;

typedef struct Base2DSize
{
    BaseI32 width;
    BaseI32 height;
}
Base2DSize;

typedef struct Base2DFloat
{
    BaseF32 x;
    BaseF32 y;
}
Base2DFloat;

typedef struct Base2DRect
{
    Base2DPoint upperLeft;
    Base2DSize size;
}
Base2DRect;

typedef struct Base1DI8
{
    BaseI8 x;
}
Base1DI8;

typedef struct Base2DI8
{
    BaseI8 x;
    BaseI8 y;
}
Base2DI8;

typedef struct Base3DI8
{
    BaseI8 x;
    BaseI8 y;
    BaseI8 z;
}
Base3DI8;

typedef struct Base4DI8
{
    BaseI8 x;
    BaseI8 y;
    BaseI8 z;
    BaseI8 t;
}
Base4DI8;

typedef struct Base1DUI8
{
    BaseUI8 x;
}
Base1DUI8;

typedef struct Base2DUI8
{
    BaseUI8 x;
    BaseUI8 y;
}
Base2DUI8;

typedef struct Base3DUI8
{
    BaseUI8 x;
    BaseUI8 y;
    BaseUI8 z;
}
Base3DUI8;

typedef struct Base4DUI8
{
    BaseUI8 x;
    BaseUI8 y;
    BaseUI8 z;
    BaseUI8 t;
}
Base4DUI8;

typedef struct Base1DI16
{
    BaseI16 x;
}
Base1DI16;

typedef struct Base2DI16
{
    BaseI16 x;
    BaseI16 y;
}
Base2DI16;

typedef struct Base3DI16
{
    BaseI16 x;
    BaseI16 y;
    BaseI16 z;
}
Base3DI16;

typedef struct Base4DI16
{
    BaseI16 x;
    BaseI16 y;
    BaseI16 z;
    BaseI16 t;
}
Base4DI16;

typedef struct Base1DUI16
{
    BaseUI16 x;
}
Base1DUI16;

typedef struct Base2DUI16
{
    BaseUI16 x;
    BaseUI16 y;
}
Base2DUI16;

typedef struct Base3DUI16
{
    BaseUI16 x;
    BaseUI16 y;
    BaseUI16 z;
}
Base3DUI16;

typedef struct Base4DUI16
{
    BaseUI16 x;
    BaseUI16 y;
    BaseUI16 z;
    BaseUI16 t;
}
Base4DUI16;

typedef struct Base1DI32
{
    BaseI32 x;
}
Base1DI32;

typedef struct Base2DI32
{
    BaseI32 x;
    BaseI32 y;
}
Base2DI32;

typedef struct Base3DI32
{
    BaseI32 x;
    BaseI32 y;
    BaseI32 z;
}
Base3DI32;

typedef struct Base4DI32
{
    BaseI32 x;
    BaseI32 y;
    BaseI32 z;
    BaseI32 t;
}
Base4DI32;

typedef struct Base1DUI32
{
    BaseUI32 x;
}
Base1DUI32;

typedef struct Base2DUI32
{
    BaseUI32 x;
    BaseUI32 y;
}
Base2DUI32;

typedef struct Base3DUI32
{
    BaseUI32 x;
    BaseUI32 y;
    BaseUI32 z;
}
Base3DUI32;

typedef struct Base4DUI32
{
    BaseUI32 x;
    BaseUI32 y;
    BaseUI32 z;
    BaseUI32 t;
}
Base4DUI32;

typedef struct Base1DF32
{
    BaseF32 x;
}
Base1DF32;

typedef struct Base2DF32
{
    BaseF32 x;
    BaseF32 y;
}
Base2DF32;

typedef struct Base3DF32
{
    BaseF32 x;
    BaseF32 y;
    BaseF32 z;
}
Base3DF32;

typedef struct Base4DF32
{
    BaseF32 x;
    BaseF32 y;
    BaseF32 z;
    BaseF32 t;
}
Base4DF32;

typedef struct Base1DF64
{
    BaseF64 x;
}
Base1DF64;

typedef struct Base2DF64
{
    BaseF64 x;
    BaseF64 y;
}
Base2DF64;

typedef struct Base3DF64
{
    BaseF64 x;
    BaseF64 y;
    BaseF64 z;
}
Base3DF64;

typedef struct Base4DF64
{
    BaseF64 x;
    BaseF64 y;
    BaseF64 z;
    BaseF64 t;
}
Base4DF64;

typedef struct Base1DI32vF32
{
    BaseI32 x;
    BaseF32 v;
}
Base1DI32vF32;

typedef struct Base2DI32vF32
{
    BaseI32 x;
    BaseI32 y;
    BaseF32 v;
}
Base2DI32vF32;

typedef struct Base3DI32vF32
{
    BaseI32 x;
    BaseI32 y;
    BaseI32 z;
    BaseF32 v;
}
Base3DI32vF32;

typedef struct Base4DI32vF32
{
    BaseI32 x;
    BaseI32 y;
    BaseI32 z;
    BaseI32 t;
    BaseF32 v;
}
Base4DI32vF32;


Base2DPoint *Base2DPoint_new( void );
int Base2DPoint_init( Base2DPoint *self );
Base2DPoint *Base2DPoint_copy( Base2DPoint *self, const Base2DPoint *src );
bool Base2DPoint_isEQ( const Base2DPoint *self, const Base2DPoint *src );
bool Base2DPoint_isNE( const Base2DPoint *self, const Base2DPoint *src );
void Base2DPoint_clear( Base2DPoint *self );
void Base2DPoint_delete( Base2DPoint *self );

Base2DSize *Base2DSize_new( void );
int Base2DSize_init( Base2DSize *self );
Base2DSize *Base2DSize_copy( Base2DSize *self, const Base2DSize *src );
bool Base2DSize_isEQ( const Base2DSize *self, const Base2DSize *src );
bool Base2DSize_isNE( const Base2DSize *self, const Base2DSize *src );
void Base2DSize_clear( Base2DSize *self );
void Base2DSize_delete( Base2DSize *self );

Base2DFloat *Base2DFloat_new( void );
int Base2DFloat_init( Base2DFloat *self );
Base2DFloat *Base2DFloat_copy( Base2DFloat *self, const Base2DFloat *src );
void Base2DFloat_clear( Base2DFloat *self );
void Base2DFloat_delete( Base2DFloat *self );

Base2DRect *Base2DRect_new( void );
int Base2DRect_init( Base2DRect *self );
Base2DRect *Base2DRect_copy( Base2DRect *self, const Base2DRect *src );
bool Base2DRect_isEQ( const Base2DRect *self, const Base2DRect *src );
bool Base2DRect_isNE( const Base2DRect *self, const Base2DRect *src );
void Base2DRect_clear( Base2DRect *self );
void Base2DRect_delete( Base2DRect *self );


Base1DI32 *Base1DI32_new( void );

Base2DI32 *Base2DI32_new( void );

Base3DI32 *Base3DI32_new( void );

Base4DI32 *Base4DI32_new( void );

Base1DF32 *Base1DF32_new( void );

Base2DF32 *Base2DF32_new( void );

Base3DF32 *Base3DF32_new( void );

Base4DF32 *Base4DF32_new( void );

Base1DI32vF32 *Base1DI32vF32_new( void );

Base2DI32vF32 *Base2DI32vF32_new( void );

Base3DI32vF32 *Base3DI32vF32_new( void );

Base4DI32vF32 *Base4DI32vF32_new( void );

void Base1DI32_init( Base1DI32 *self );

void Base2DI32_init( Base2DI32 *self );

void Base3DI32_init( Base3DI32 *self );

void Base4DI32_init( Base4DI32 *self );

void Base1DF32_init( Base1DF32 *self );

void Base2DF32_init( Base2DF32 *self );

void Base3DF32_init( Base3DF32 *self );

void Base4DF32_init( Base4DF32 *self );

void Base1DI32vF32_init( Base1DI32vF32 *self );

void Base2DI32vF32_init( Base2DI32vF32 *self );

void Base3DI32vF32_init( Base3DI32vF32 *self );

void Base4DI32vF32_init( Base4DI32vF32 *self );

void Base1DI32_clear( Base1DI32 *self );

void Base2DI32_clear( Base2DI32 *self );

void Base3DI32_clear( Base3DI32 *self );

void Base4DI32_clear( Base4DI32 *self );

void Base1DF32_clear( Base1DF32 *self );

void Base2DF32_clear( Base2DF32 *self );

void Base3DF32_clear( Base3DF32 *self );

void Base4DF32_clear( Base4DF32 *self );

void Base1DI32vF32_clear( Base1DI32vF32 *self );

void Base2DI32vF32_clear( Base2DI32vF32 *self );

void Base3DI32vF32_clear( Base3DI32vF32 *self );

void Base4DI32vF32_clear( Base4DI32vF32 *self );

void Base1DI32_delete( Base1DI32 *self );

void Base2DI32_delete( Base2DI32 *self );

void Base3DI32_delete( Base3DI32 *self );

void Base4DI32_delete( Base4DI32 *self );

void Base1DF32_delete( Base1DF32 *self );

void Base2DF32_delete( Base2DF32 *self );

void Base3DF32_delete( Base3DF32 *self );

void Base4DF32_delete( Base4DF32 *self );

void Base1DI32vF32_delete( Base1DI32vF32 *self );

void Base2DI32vF32_delete( Base2DI32vF32 *self );

void Base3DI32vF32_delete( Base3DI32vF32 *self );

void Base4DI32vF32_delete( Base4DI32vF32 *self );

void Base1DI32_copy( Base1DI32 *src, Base1DI32 *dst );

void Base2DI32_copy( Base2DI32 *src, Base2DI32 *dst );

void Base3DI32_copy( Base3DI32 *src, Base3DI32 *dst );

void Base4DI32_copy( Base4DI32 *src, Base4DI32 *dst );

void Base1DF32_copy( Base1DF32 *src, Base1DF32 *dst );

void Base2DF32_copy( Base2DF32 *src, Base2DF32 *dst );

void Base3DF32_copy( Base3DF32 *src, Base3DF32 *dst );

void Base4DF32_copy( Base4DF32 *src, Base4DF32 *dst );

void Base1DI32vF32_copy( Base1DI32vF32 *src, Base1DI32vF32 *dst );

void Base2DI32vF32_copy( Base2DI32vF32 *src, Base2DI32vF32 *dst );

void Base3DI32vF32_copy( Base3DI32vF32 *src, Base3DI32vF32 *dst );

void Base4DI32vF32_copy( Base4DI32vF32 *src, Base4DI32vF32 *dst );


#ifdef __cplusplus
}
#endif


#endif


/* EOF */
