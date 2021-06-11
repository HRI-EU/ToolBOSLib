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


#include <BaseSerialize.h>


void Base2DPoint_serialize( Base2DPoint *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DPoint" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base2DRect_serialize( Base2DRect *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DRect" );

    Base2DPoint_serialize( &( self )->upperLeft, "upperLeft", serializer );
    Base2DSize_serialize( &( self )->size, "size", serializer );

    Serialize_endType( serializer );
}


void Base2DSize_serialize( Base2DSize *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DSize" );

    BaseI32_serialize( &( self )->width, "width", serializer );
    BaseI32_serialize( &( self )->height, "height", serializer );

    Serialize_endType( serializer );
}


void BaseC32_serialize( BaseC32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseC32" );

    BaseF32_serialize( &( self )->real, "real", serializer );
    BaseF32_serialize( &( self )->imag, "imag", serializer );

    Serialize_endType( serializer );
}


void BaseBool_serialize( BaseBool *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseBool" );

    Int_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseI8_serialize( BaseI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseBool" );

    SChar_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseUI8_serialize( BaseUI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseUI8" );

    UChar_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseI16_serialize( BaseI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseI16" );

    SInt_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseUI16_serialize( BaseUI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseUI16" );

    USInt_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseI32_serialize( BaseI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseI32" );

    Int_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseUI32_serialize( BaseUI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseUI32" );

    UInt_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseI64_serialize( BaseI64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseI64" );

    LL_serialize((long long *)self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseUI64_serialize( BaseUI64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseUI64" );

    ULL_serialize((unsigned long long *)self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseF32_serialize( BaseF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseF32" );

    Float_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseF64_serialize( BaseF64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginBaseType( serializer, name, "BaseF64" );

    Double_serialize( self, name, serializer );

    Serialize_endBaseType( serializer );
}


void BaseBool_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseBool_serialize((BaseBool *)self, name, serializer );
}


void BaseI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseI8_serialize((BaseI8 *)self, name, serializer );
}


void BaseUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseUI8_serialize((BaseUI8 *)self, name, serializer );
}


void BaseI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseI16_serialize((BaseI16 *)self, name, serializer );
}


void BaseUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseUI16_serialize((BaseUI16 *)self, name, serializer );
}


void BaseI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseI32_serialize((BaseI32 *)self, name, serializer );
}


void BaseUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseUI32_serialize((BaseUI32 *)self, name, serializer );
}


void BaseI64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseI64_serialize((BaseI64 *)self, name, serializer );
}


void BaseUI64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseUI64_serialize((BaseUI64 *)self, name, serializer );
}


void BaseF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseF32_serialize((BaseF32 *)self, name, serializer );
}


void BaseF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseF64_serialize((BaseF64 *)self, name, serializer );
}


void Base2DPoint_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DPoint_serialize((Base2DPoint *)self, name, serializer );
}


void Base2DRect_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DRect_serialize((Base2DRect *)self, name, serializer );
}


void Base2DSize_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DSize_serialize((Base2DSize *)self, name, serializer );
}


void BaseC32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    BaseC32_serialize((BaseC32 *)self, name, serializer );
}


void BaseBoolArray_serialize( BaseBool *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseBoolArray" );

    IntArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseI8Array_serialize( BaseI8 *self, const char *name,
                            BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseI8Array" );

    SCharArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseUI8Array_serialize( BaseUI8 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseUI8Array" );

    UCharArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseI16Array_serialize( BaseI16 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseI16Array" );

    SIntArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseUI16Array_serialize( BaseUI16 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseUI16Array" );

    USIntArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseI32Array_serialize( BaseI32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseI32Array" );

    IntArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseUI32Array_serialize( BaseUI32 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseUI32Array" );

    UIntArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseF32Array_serialize( BaseF32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseF32Array" );

    FloatArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseF64Array_serialize( BaseF64 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseF64Array" );

    DoubleArray_serialize( self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseI64Array_serialize( BaseI64 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseI64Array" );

    LLArray_serialize((long long *)self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseUI64Array_serialize( BaseUI64 *self, const char *name,
                              BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseUI64Array" );

    ULLArray_serialize((unsigned long long *)self, name, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseBoolArray_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer )
{
    BaseBoolArray_serialize((BaseBool *)self, name, arrayLength, serializer );
}


void BaseI8Array_indirectSerialize( void *self, const char *name,
                                    BaseUI32 arrayLength, Serialize *serializer )
{
    BaseI8Array_serialize((BaseI8 *)self, name, arrayLength, serializer );
}


void BaseUI8Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseUI8Array_serialize((BaseUI8 *)self, name, arrayLength, serializer );
}


void BaseI16Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseI16Array_serialize((BaseI16 *)self, name, arrayLength, serializer );
}


void BaseUI16Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer )
{
    BaseUI16Array_serialize((BaseUI16 *)self, name, arrayLength, serializer );
}


void BaseI32Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseI32Array_serialize((BaseI32 *)self, name, arrayLength, serializer );
}


void BaseUI32Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer )
{
    BaseUI32Array_serialize((BaseUI32 *)self, name, arrayLength, serializer );
}


void BaseF32Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseF32Array_serialize((BaseF32 *)self, name, arrayLength, serializer );
}


void BaseF64Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseF64Array_serialize((BaseF64 *)self, name, arrayLength, serializer );
}


void BaseI64Array_indirectSerialize( void *self, const char *name,
                                     BaseUI32 arrayLength, Serialize *serializer )
{
    BaseI64Array_serialize((BaseI64 *)self, name, arrayLength, serializer );
}


void BaseUI64Array_indirectSerialize( void *self, const char *name,
                                      BaseUI32 arrayLength, Serialize *serializer )
{
    BaseUI64Array_serialize((BaseUI64 *)self, name, arrayLength, serializer );
}


void Base2DPointArray_serialize( Base2DPoint *self, const char *name,
                                 BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DPointArray" );

    STRUCT_ARRAY_SERIALIZE( self, name, "Base2DPoint",
                            Base2DPoint_serialize, arrayLength, serializer );

    Serialize_endType( serializer );
}


void Base2DRectArray_serialize( Base2DRect *self, const char *name,
                                BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DRectArray" );

    STRUCT_ARRAY_SERIALIZE( self, name, "Base2DRect",
                            Base2DRect_serialize, arrayLength, serializer );

    Serialize_endType( serializer );
}


void Base2DSizeArray_serialize( Base2DSize *self, const char *name,
                                BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DSizeArray" );

    STRUCT_ARRAY_SERIALIZE( self, name, "Base2DSize",
                            Base2DSize_serialize, arrayLength, serializer );

    Serialize_endType( serializer );
}


void BaseC32Array_serialize( BaseC32 *self, const char *name,
                             BaseUI32 arrayLength, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "BaseC32Array" );

    STRUCT_ARRAY_SERIALIZE( self, name, "BaseC32",
                            BaseC32_serialize, arrayLength, serializer );

    Serialize_endType( serializer );
}


void Base2DPointArray_indirectSerialize( void *self,
                                         const char *name,
                                         BaseUI32 arrayLength,
                                         Serialize *serializer )
{
    Base2DPointArray_serialize((Base2DPoint *)self, name, arrayLength, serializer );
}


void Base2DRectArray_indirectSerialize( void *self,
                                        const char *name,
                                        BaseUI32 arrayLength,
                                        Serialize *serializer )
{
    Base2DRectArray_serialize((Base2DRect *)self, name, arrayLength, serializer );
}


void Base2DSizeArray_indirectSerialize( void *self,
                                        const char *name,
                                        BaseUI32 arrayLength,
                                        Serialize *serializer )
{
    Base2DSizeArray_serialize((Base2DSize *)self, name, arrayLength, serializer );
}


void BaseC32Array_indirectSerialize( void *self,
                                     const char *name,
                                     BaseUI32 arrayLength,
                                     Serialize *serializer )
{
    BaseC32Array_serialize((BaseC32 *)self, name, arrayLength, serializer );
}


/* * * * * * * * *   START OF AUTO-GENERATED CODE   * * * * * * * * * */


void Base1DI8_serialize( Base1DI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DI8" );

    BaseI8_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DI8_serialize( Base2DI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DI8" );

    BaseI8_serialize( &( self )->x, "x", serializer );
    BaseI8_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DI8_serialize( Base3DI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DI8" );

    BaseI8_serialize( &( self )->x, "x", serializer );
    BaseI8_serialize( &( self )->y, "y", serializer );
    BaseI8_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DI8_serialize( Base4DI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DI8" );

    BaseI8_serialize( &( self )->x, "x", serializer );
    BaseI8_serialize( &( self )->y, "y", serializer );
    BaseI8_serialize( &( self )->z, "z", serializer );
    BaseI8_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DUI8_serialize( Base1DUI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DUI8" );

    BaseUI8_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DUI8_serialize( Base2DUI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DUI8" );

    BaseUI8_serialize( &( self )->x, "x", serializer );
    BaseUI8_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DUI8_serialize( Base3DUI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DUI8" );

    BaseUI8_serialize( &( self )->x, "x", serializer );
    BaseUI8_serialize( &( self )->y, "y", serializer );
    BaseUI8_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DUI8_serialize( Base4DUI8 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DUI8" );

    BaseUI8_serialize( &( self )->x, "x", serializer );
    BaseUI8_serialize( &( self )->y, "y", serializer );
    BaseUI8_serialize( &( self )->z, "z", serializer );
    BaseUI8_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DI16_serialize( Base1DI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DI16" );

    BaseI16_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DI16_serialize( Base2DI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DI16" );

    BaseI16_serialize( &( self )->x, "x", serializer );
    BaseI16_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DI16_serialize( Base3DI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DI16" );

    BaseI16_serialize( &( self )->x, "x", serializer );
    BaseI16_serialize( &( self )->y, "y", serializer );
    BaseI16_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DI16_serialize( Base4DI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DI16" );

    BaseI16_serialize( &( self )->x, "x", serializer );
    BaseI16_serialize( &( self )->y, "y", serializer );
    BaseI16_serialize( &( self )->z, "z", serializer );
    BaseI16_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DUI16_serialize( Base1DUI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DUI16" );

    BaseUI16_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DUI16_serialize( Base2DUI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DUI16" );

    BaseUI16_serialize( &( self )->x, "x", serializer );
    BaseUI16_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DUI16_serialize( Base3DUI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DUI16" );

    BaseUI16_serialize( &( self )->x, "x", serializer );
    BaseUI16_serialize( &( self )->y, "y", serializer );
    BaseUI16_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DUI16_serialize( Base4DUI16 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DUI16" );

    BaseUI16_serialize( &( self )->x, "x", serializer );
    BaseUI16_serialize( &( self )->y, "y", serializer );
    BaseUI16_serialize( &( self )->z, "z", serializer );
    BaseUI16_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DI32_serialize( Base1DI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DI32" );

    BaseI32_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DI32_serialize( Base2DI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DI32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DI32_serialize( Base3DI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DI32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );
    BaseI32_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DI32_serialize( Base4DI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DI32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );
    BaseI32_serialize( &( self )->z, "z", serializer );
    BaseI32_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DUI32_serialize( Base1DUI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DUI32" );

    BaseUI32_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DUI32_serialize( Base2DUI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DUI32" );

    BaseUI32_serialize( &( self )->x, "x", serializer );
    BaseUI32_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DUI32_serialize( Base3DUI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DUI32" );

    BaseUI32_serialize( &( self )->x, "x", serializer );
    BaseUI32_serialize( &( self )->y, "y", serializer );
    BaseUI32_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DUI32_serialize( Base4DUI32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DUI32" );

    BaseUI32_serialize( &( self )->x, "x", serializer );
    BaseUI32_serialize( &( self )->y, "y", serializer );
    BaseUI32_serialize( &( self )->z, "z", serializer );
    BaseUI32_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DF32_serialize( Base1DF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DF32" );

    BaseF32_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DF32_serialize( Base2DF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DF32" );

    BaseF32_serialize( &( self )->x, "x", serializer );
    BaseF32_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DF32_serialize( Base3DF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DF32" );

    BaseF32_serialize( &( self )->x, "x", serializer );
    BaseF32_serialize( &( self )->y, "y", serializer );
    BaseF32_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DF32_serialize( Base4DF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DF32" );

    BaseF32_serialize( &( self )->x, "x", serializer );
    BaseF32_serialize( &( self )->y, "y", serializer );
    BaseF32_serialize( &( self )->z, "z", serializer );
    BaseF32_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DF64_serialize( Base1DF64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DF64" );

    BaseF64_serialize( &( self )->x, "x", serializer );

    Serialize_endType( serializer );
}


void Base2DF64_serialize( Base2DF64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DF64" );

    BaseF64_serialize( &( self )->x, "x", serializer );
    BaseF64_serialize( &( self )->y, "y", serializer );

    Serialize_endType( serializer );
}


void Base3DF64_serialize( Base3DF64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DF64" );

    BaseF64_serialize( &( self )->x, "x", serializer );
    BaseF64_serialize( &( self )->y, "y", serializer );
    BaseF64_serialize( &( self )->z, "z", serializer );

    Serialize_endType( serializer );
}


void Base4DF64_serialize( Base4DF64 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DF64" );

    BaseF64_serialize( &( self )->x, "x", serializer );
    BaseF64_serialize( &( self )->y, "y", serializer );
    BaseF64_serialize( &( self )->z, "z", serializer );
    BaseF64_serialize( &( self )->t, "t", serializer );

    Serialize_endType( serializer );
}


void Base1DI32vF32_serialize( Base1DI32vF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base1DI32vF32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseF32_serialize( &( self )->v, "v", serializer );

    Serialize_endType( serializer );
}


void Base2DI32vF32_serialize( Base2DI32vF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base2DI32vF32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );
    BaseF32_serialize( &( self )->v, "v", serializer );

    Serialize_endType( serializer );
}


void Base3DI32vF32_serialize( Base3DI32vF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base3DI32vF32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );
    BaseI32_serialize( &( self )->z, "z", serializer );
    BaseF32_serialize( &( self )->v, "v", serializer );

    Serialize_endType( serializer );
}


void Base4DI32vF32_serialize( Base4DI32vF32 *self, const char *name, Serialize *serializer )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, "Base4DI32vF32" );

    BaseI32_serialize( &( self )->x, "x", serializer );
    BaseI32_serialize( &( self )->y, "y", serializer );
    BaseI32_serialize( &( self )->z, "z", serializer );
    BaseI32_serialize( &( self )->t, "t", serializer );
    BaseF32_serialize( &( self )->v, "v", serializer );

    Serialize_endType( serializer );
}


void MemI8_serialize( MemI8 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemI8" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseI8 ));
    }

    BaseI8Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemUI8_serialize( MemUI8 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemUI8" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseUI8 ));
    }

    BaseUI8Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemI16_serialize( MemI16 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemI16" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseI16 ));
    }

    BaseI16Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemUI16_serialize( MemUI16 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemUI16" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseUI16 ));
    }

    BaseUI16Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemI32_serialize( MemI32 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemI32" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseI32 ));
    }

    BaseI32Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemUI32_serialize( MemUI32 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemUI32" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseUI32 ));
    }

    BaseUI32Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemF32_serialize( MemF32 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemF32" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseF32 ));
    }

    BaseF32Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void MemF64_serialize( MemF64 *self, const char *name, Serialize *serializer )
{
    BaseUI32 length = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    length = self->length;

    Serialize_beginType( serializer, name, "MemF64" );

    BaseUI32_serialize( &length, "length", serializer );

    if( Serialize_isReading( serializer ) && !Serialize_isInitMode( serializer ) )
    {
        ANY_REQUIRE_VMSG( length == self->length, "Read size %" BASEUI32_PRINT " does not match expected size %" BASEUI32_PRINT,
                          length, self->length );
    }

    if( Serialize_isInitMode( serializer ) == true )
    {
        self->length = length;

        self->buffer = ANY_MALLOC( self->length, sizeof( BaseF64 ));
    }

    BaseF64Array_serialize( self->buffer, "buffer", self->length, serializer );

    Serialize_endType( serializer );
}


void Base1DI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DI8_serialize((Base1DI8 *)self, name, serializer );
}


void Base2DI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DI8_serialize((Base2DI8 *)self, name, serializer );
}


void Base3DI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DI8_serialize((Base3DI8 *)self, name, serializer );
}


void Base4DI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DI8_serialize((Base4DI8 *)self, name, serializer );
}


void Base1DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DUI8_serialize((Base1DUI8 *)self, name, serializer );
}


void Base2DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DUI8_serialize((Base2DUI8 *)self, name, serializer );
}


void Base3DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DUI8_serialize((Base3DUI8 *)self, name, serializer );
}


void Base4DUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DUI8_serialize((Base4DUI8 *)self, name, serializer );
}


void Base1DI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DI16_serialize((Base1DI16 *)self, name, serializer );
}


void Base2DI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DI16_serialize((Base2DI16 *)self, name, serializer );
}


void Base3DI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DI16_serialize((Base3DI16 *)self, name, serializer );
}


void Base4DI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DI16_serialize((Base4DI16 *)self, name, serializer );
}


void Base1DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DUI16_serialize((Base1DUI16 *)self, name, serializer );
}


void Base2DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DUI16_serialize((Base2DUI16 *)self, name, serializer );
}


void Base3DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DUI16_serialize((Base3DUI16 *)self, name, serializer );
}


void Base4DUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DUI16_serialize((Base4DUI16 *)self, name, serializer );
}


void Base1DI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DI32_serialize((Base1DI32 *)self, name, serializer );
}


void Base2DI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DI32_serialize((Base2DI32 *)self, name, serializer );
}


void Base3DI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DI32_serialize((Base3DI32 *)self, name, serializer );
}


void Base4DI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DI32_serialize((Base4DI32 *)self, name, serializer );
}


void Base1DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DUI32_serialize((Base1DUI32 *)self, name, serializer );
}


void Base2DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DUI32_serialize((Base2DUI32 *)self, name, serializer );
}


void Base3DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DUI32_serialize((Base3DUI32 *)self, name, serializer );
}


void Base4DUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DUI32_serialize((Base4DUI32 *)self, name, serializer );
}


void Base1DF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DF32_serialize((Base1DF32 *)self, name, serializer );
}


void Base2DF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DF32_serialize((Base2DF32 *)self, name, serializer );
}


void Base3DF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DF32_serialize((Base3DF32 *)self, name, serializer );
}


void Base4DF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DF32_serialize((Base4DF32 *)self, name, serializer );
}


void Base1DF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base1DF64_serialize((Base1DF64 *)self, name, serializer );
}


void Base2DF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base2DF64_serialize((Base2DF64 *)self, name, serializer );
}


void Base3DF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base3DF64_serialize((Base3DF64 *)self, name, serializer );
}


void Base4DF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    Base4DF64_serialize((Base4DF64 *)self, name, serializer );
}


void MemI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemI8_serialize((MemI8 *)self, name, serializer );
}


void MemUI8_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemUI8_serialize((MemUI8 *)self, name, serializer );
}


void MemI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemI16_serialize((MemI16 *)self, name, serializer );
}


void MemUI16_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemUI16_serialize((MemUI16 *)self, name, serializer );
}


void MemI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemI32_serialize((MemI32 *)self, name, serializer );
}


void MemUI32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemUI32_serialize((MemUI32 *)self, name, serializer );
}


void MemF32_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemF32_serialize((MemF32 *)self, name, serializer );
}


void MemF64_indirectSerialize( void *self, const char *name, Serialize *serializer )
{
    MemF64_serialize((MemF64 *)self, name, serializer );
}


/* * * * * * * * *   END OF AUTO-GENERATED CODE   * * * * * * * * * */


/* EOF */
