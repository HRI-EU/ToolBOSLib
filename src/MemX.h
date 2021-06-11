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


#ifndef MEMX_H
#define MEMX_H

#include <Base.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif


#define MEMI8_DEFAULT_LENGTH (10)

typedef struct MemI8
{
    BaseI8 *buffer;
    BaseUI32 length;
}
MemI8;

MemI8 *MemI8_new( void );

int MemI8_init( MemI8 *self, BaseUI32 length );

int MemI8_copy( MemI8 *self, const MemI8 *src );

int MemI8_copyConstr( MemI8 *self, const MemI8 *src );

BaseI8 *MemI8_getBuffer( MemI8 *self );

const BaseI8 *MemI8_getConstBuffer( const MemI8 *self );

BaseUI32 MemI8_getLength( const MemI8 *self );

void MemI8_clear( MemI8 *self );

void MemI8_delete( MemI8 *self );


#define MEMUI8_DEFAULT_LENGTH (10)

typedef struct MemUI8
{
    BaseUI8 *buffer;
    BaseUI32 length;
}
MemUI8;

MemUI8 *MemUI8_new( void );

int MemUI8_init( MemUI8 *self, BaseUI32 length );

int MemUI8_copyConstr( MemUI8 *self, const MemUI8 *src );

int MemUI8_copy( MemUI8 *self, const MemUI8 *src );

BaseUI8 *MemUI8_getBuffer( MemUI8 *self );

const BaseUI8 *MemUI8_getConstBuffer( const MemUI8 *self );

BaseUI32 MemUI8_getLength( const MemUI8 *self );

void MemUI8_clear( MemUI8 *self );

void MemUI8_delete( MemUI8 *self );


#define MEMI16_DEFAULT_LENGTH (10)

typedef struct MemI16
{
    BaseI16 *buffer;
    BaseUI32 length;
}
MemI16;

MemI16 *MemI16_new( void );

int MemI16_init( MemI16 *self, BaseUI32 length );

int MemI16_copyConstr( MemI16 *self, const MemI16 *src );

int MemI16_copy( MemI16 *self, const MemI16 *src );

BaseI16 *MemI16_getBuffer( MemI16 *self );

const BaseI16 *MemI16_getConstBuffer( const MemI16 *self );

BaseUI32 MemI16_getLength( const MemI16 *self );

void MemI16_clear( MemI16 *self );

void MemI16_delete( MemI16 *self );


#define MEMUI16_DEFAULT_LENGTH (10)

typedef struct MemUI16
{
    BaseUI16 *buffer;
    BaseUI32 length;
}
MemUI16;

MemUI16 *MemUI16_new( void );

int MemUI16_init( MemUI16 *self, BaseUI32 length );

int MemUI16_copyConstr( MemUI16 *self, const MemUI16 *src );

int MemUI16_copy( MemUI16 *self, const MemUI16 *src );

BaseUI16 *MemUI16_getBuffer( MemUI16 *self );

const BaseUI16 *MemUI16_getConstBuffer( const MemUI16 *self );

BaseUI32 MemUI16_getLength( const MemUI16 *self );

void MemUI16_clear( MemUI16 *self );

void MemUI16_delete( MemUI16 *self );


#define MEMI32_DEFAULT_LENGTH (10)

typedef struct MemI32
{
    BaseI32 *buffer;
    BaseUI32 length;
}
MemI32;


MemI32 *MemI32_new( void );

int MemI32_init( MemI32 *self, BaseUI32 length );

int MemI32_copyConstr( MemI32 *self, const MemI32 *src );

int MemI32_copy( MemI32 *self, const MemI32 *src );

BaseI32 *MemI32_getBuffer( MemI32 *self );

const BaseI32 *MemI32_getConstBuffer( const MemI32 *self );

BaseUI32 MemI32_getLength( const MemI32 *self );

void MemI32_clear( MemI32 *self );

void MemI32_delete( MemI32 *self );


#define MEMUI32_DEFAULT_LENGTH (10)

typedef struct MemUI32
{
    BaseUI32 *buffer;
    BaseUI32 length;
}
MemUI32;

MemUI32 *MemUI32_new( void );

int MemUI32_init( MemUI32 *self, BaseUI32 length );

int MemUI32_copyConstr( MemUI32 *self, const MemUI32 *src );

int MemUI32_copy( MemUI32 *self, const MemUI32 *src );

BaseUI32 *MemUI32_getBuffer( MemUI32 *self );

const BaseUI32 *MemUI32_getConstBuffer( const MemUI32 *self );

BaseUI32 MemUI32_getLength( const MemUI32 *self );

void MemUI32_clear( MemUI32 *self );

void MemUI32_delete( MemUI32 *self );


#define MEMF32_DEFAULT_LENGTH (10)

typedef struct MemF32
{
    BaseF32 *buffer;
    BaseUI32 length;
}
MemF32;

MemF32 *MemF32_new( void );

int MemF32_init( MemF32 *self, BaseUI32 length );

int MemF32_copyConstr( MemF32 *self, const MemF32 *src );

int MemF32_copy( MemF32 *self, const MemF32 *src );

BaseF32 *MemF32_getBuffer( MemF32 *self );

const BaseF32 *MemF32_getConstBuffer( const MemF32 *self );

BaseUI32 MemF32_getLength( const MemF32 *self );

void MemF32_clear( MemF32 *self );

void MemF32_delete( MemF32 *self );


#define MEMF64_DEFAULT_LENGTH (10)

typedef struct MemF64
{
    BaseF64 *buffer;
    BaseUI32 length;
}
MemF64;

MemF64 *MemF64_new( void );

int MemF64_init( MemF64 *self, BaseUI32 length );

int MemF64_copyConstr( MemF64 *self, const MemF64 *src );

int MemF64_copy( MemF64 *self, const MemF64 *src );

BaseF64 *MemF64_getBuffer( MemF64 *self );

const BaseF64 *MemF64_getConstBuffer( const MemF64 *self );

BaseUI32 MemF64_getLength( const MemF64 *self );

void MemF64_clear( MemF64 *self );

void MemF64_delete( MemF64 *self );


#ifdef __cplusplus
}
#endif


#endif


/* EOF */
