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


#ifndef BASESERIALIZETYPES_H
#define BASESERIALIZETYPES_H


/*-------------------------------------------------------------------------*/
/* Includes                                                                */
/*-------------------------------------------------------------------------*/

#include <Any.h>
#include <Base.h>
#include <Serialize.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * \page BaseSerialize_About Serialization functions for basic datatypes
 *
 * BaseSerialize.h contains serialization functions for all the datatypes
 * defined in Base.h. They are all called:
 *
 * \li \<TYPENAME\>_serialize()
 * \li \<TYPENAME\>_indirectSerialize()
 *
 * The only difference between the two forms is that the "indirect" version
 * accepts a void pointer, whereas the normal serialize functions requires
 * a self-pointer of the corresponding datatype.
 *
 * <h3>Example:</h3>
 *
 * \code
 * ...
 *
 * // serialize the data with the help of a previously defined serializer
 * MemI8_serialize( myData, "myDescription", serializer );
 *
 * ...
 * \endcode
 */


/*-------------------------------------------------------------------------*/
/* basetype serialization                                                  */
/* Usage: BaseUI8_serialize( &value, "myValue", s );                       */
/*-------------------------------------------------------------------------*/

void BaseBool_serialize( BaseBool *self, const char *name, Serialize *serializer );

void BaseI8_serialize( BaseI8 *self, const char *name, Serialize *serializer );

void BaseUI8_serialize( BaseUI8 *self, const char *name, Serialize *serializer );

void BaseI16_serialize( BaseI16 *self, const char *name, Serialize *serializer );

void BaseUI16_serialize( BaseUI16 *self, const char *name, Serialize *serializer );

void BaseI32_serialize( BaseI32 *self, const char *name, Serialize *serializer );

void BaseUI32_serialize( BaseUI32 *self, const char *name, Serialize *serializer );

void BaseI64_serialize( BaseI64 *self, const char *name, Serialize *serializer );

void BaseUI64_serialize( BaseUI64 *self, const char *name, Serialize *serializer );

void BaseF32_serialize( BaseF32 *self, const char *name, Serialize *serializer );

void BaseF64_serialize( BaseF64 *self, const char *name, Serialize *serializer );

void BaseBool_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseI64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseUI64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseF64_indirectSerialize( void *self, const char *name, Serialize *serializer );


/*-------------------------------------------------------------------------*/
/* basetype-array serialization                                            */
/* Usage: BaseUI8Array_serialize( &value, "myValue", arrayLen, s );        */
/*-------------------------------------------------------------------------*/

void BaseBoolArray_serialize( BaseBool *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer );

void BaseI8Array_serialize( BaseI8 *self, const char *name,
                            BaseUI32 arrayLength, Serialize *serializer );

void BaseUI8Array_serialize( BaseUI8 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseI16Array_serialize( BaseI16 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseUI16Array_serialize( BaseUI16 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer );

void BaseI32Array_serialize( BaseI32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseUI32Array_serialize( BaseUI32 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer );

void BaseF32Array_serialize( BaseF32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseF64Array_serialize( BaseF64 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseI64Array_serialize( BaseI64 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void BaseUI64Array_serialize( BaseUI64 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer );


void BaseBoolArray_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer );

void BaseI8Array_indirectSerialize( void *self, const char *name,
                                    BaseUI32 arrayLength, Serialize *serializer );

void BaseUI8Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseI16Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseUI16Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer );

void BaseI32Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseUI32Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer );

void BaseF32Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseF64Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseI64Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer );

void BaseUI64Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer );


/*-------------------------------------------------------------------------*/
/* generic serialization up to 4 dimensionsions with and without value el. */
/*-------------------------------------------------------------------------*/

void BaseC32_serialize( BaseC32 *self, const char *name, Serialize *serializer );

void BaseC64_serialize( BaseC64 *self, const char *name, Serialize *serializer );

void Base1DI8_serialize( Base1DI8 *self, const char *name, Serialize *serializer );

void Base2DI8_serialize( Base2DI8 *self, const char *name, Serialize *serializer );

void Base3DI8_serialize( Base3DI8 *self, const char *name, Serialize *serializer );

void Base4DI8_serialize( Base4DI8 *self, const char *name, Serialize *serializer );

void Base1DUI8_serialize( Base1DUI8 *self, const char *name, Serialize *serializer );

void Base2DUI8_serialize( Base2DUI8 *self, const char *name, Serialize *serializer );

void Base3DUI8_serialize( Base3DUI8 *self, const char *name, Serialize *serializer );

void Base4DUI8_serialize( Base4DUI8 *self, const char *name, Serialize *serializer );

void Base1DI16_serialize( Base1DI16 *self, const char *name, Serialize *serializer );

void Base2DI16_serialize( Base2DI16 *self, const char *name, Serialize *serializer );

void Base3DI16_serialize( Base3DI16 *self, const char *name, Serialize *serializer );

void Base4DI16_serialize( Base4DI16 *self, const char *name, Serialize *serializer );

void Base1DUI16_serialize( Base1DUI16 *self, const char *name, Serialize *serializer );

void Base2DUI16_serialize( Base2DUI16 *self, const char *name, Serialize *serializer );

void Base3DUI16_serialize( Base3DUI16 *self, const char *name, Serialize *serializer );

void Base4DUI16_serialize( Base4DUI16 *self, const char *name, Serialize *serializer );

void Base1DI32_serialize( Base1DI32 *self, const char *name, Serialize *serializer );

void Base2DI32_serialize( Base2DI32 *self, const char *name, Serialize *serializer );

void Base3DI32_serialize( Base3DI32 *self, const char *name, Serialize *serializer );

void Base4DI32_serialize( Base4DI32 *self, const char *name, Serialize *serializer );

void Base1DUI32_serialize( Base1DUI32 *self, const char *name, Serialize *serializer );

void Base2DUI32_serialize( Base2DUI32 *self, const char *name, Serialize *serializer );

void Base3DUI32_serialize( Base3DUI32 *self, const char *name, Serialize *serializer );

void Base4DUI32_serialize( Base4DUI32 *self, const char *name, Serialize *serializer );

void Base1DF32_serialize( Base1DF32 *self, const char *name, Serialize *serializer );

void Base2DF32_serialize( Base2DF32 *self, const char *name, Serialize *serializer );

void Base3DF32_serialize( Base3DF32 *self, const char *name, Serialize *serializer );

void Base4DF32_serialize( Base4DF32 *self, const char *name, Serialize *serializer );

void Base1DF64_serialize( Base1DF64 *self, const char *name, Serialize *serializer );

void Base2DF64_serialize( Base2DF64 *self, const char *name, Serialize *serializer );

void Base3DF64_serialize( Base3DF64 *self, const char *name, Serialize *serializer );

void Base4DF64_serialize( Base4DF64 *self, const char *name, Serialize *serializer );

void Base1DI32vF32_serialize( Base1DI32vF32 *self, const char *name, Serialize *serializer );

void Base2DI32vF32_serialize( Base2DI32vF32 *self, const char *name, Serialize *serializer );

void Base3DI32vF32_serialize( Base3DI32vF32 *self, const char *name, Serialize *serializer );

void Base4DI32vF32_serialize( Base4DI32vF32 *self, const char *name, Serialize *serializer );

void MemI8_serialize( MemI8 *self, const char *name, Serialize *serializer );

void MemUI8_serialize( MemUI8 *self, const char *name, Serialize *serializer );

void MemI16_serialize( MemI16 *self, const char *name, Serialize *serializer );

void MemUI16_serialize( MemUI16 *self, const char *name, Serialize *serializer );

void MemI32_serialize( MemI32 *self, const char *name, Serialize *serializer );

void MemUI32_serialize( MemUI32 *self, const char *name, Serialize *serializer );

void MemF32_serialize( MemF32 *self, const char *name, Serialize *serializer );

void MemF64_serialize( MemF64 *self, const char *name, Serialize *serializer );

void Base1DI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base1DF64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DF64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base3DF64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base4DF64_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemUI8_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemUI16_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemUI32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemF32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void MemF64_indirectSerialize( void *self, const char *name, Serialize *serializer );


/*-------------------------------------------------------------------------*/
/* serialization of a bit more complex types                               */
/*-------------------------------------------------------------------------*/


void Base2DPoint_serialize( Base2DPoint *self, const char *name, Serialize *serializer );

void Base2DRect_serialize( Base2DRect *self, const char *name, Serialize *serializer );

void Base2DSize_serialize( Base2DSize *self, const char *name, Serialize *serializer );

void Base2DPoint_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DRect_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DSize_indirectSerialize( void *self, const char *name, Serialize *serializer );

void BaseC32_indirectSerialize( void *self, const char *name, Serialize *serializer );

void Base2DPointArray_serialize( Base2DPoint *self, const char *name,
                                 BaseUI32 arrayLength, Serialize *serializer );

void Base2DRectArray_serialize( Base2DRect *self, const char *name,
                                BaseUI32 arrayLength, Serialize *serializer );

void Base2DSizeArray_serialize( Base2DSize *self, const char *name,
                                BaseUI32 arrayLength, Serialize *serializer );

void BaseC32Array_serialize( BaseC32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer );

void Base2DPointArray_indirectSerialize( void *self,
                                         const char *name,
                                         BaseUI32 arrayLength,
                                         Serialize *serializer );

void Base2DRectArray_indirectSerialize( void *self,
                                        const char *name,
                                        BaseUI32 arrayLength,
                                        Serialize *serializer );

void Base2DSizeArray_indirectSerialize( void *self,
                                        const char *name,
                                        BaseUI32 arrayLength,
                                        Serialize *serializer );

void BaseC32Array_indirectSerialize( void *self,
                                     const char *name,
                                     BaseUI32 arrayLength,
                                     Serialize *serializer );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
