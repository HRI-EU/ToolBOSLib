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


#ifndef SERIALIZETYPES_H
#define SERIALIZETYPES_H


#if defined(__cplusplus)
extern "C" {
#endif


/*! \brief Generic serialization function type
 *
 * This type can be used to pass serialization functions. In particular,
 * each datatype should provide a Datatype_indirectSerialize() function
 * which takes a void pointer. Internally, the void* is casted to the
 * specific datatype and the typed serialization function - usually
 * Datatype_serialize() - will be called by the Datatype_indirectSerialize()
 * function.
 *
 * \note Serialization functions for standard C arrays also need the size
 *       information, thus can't be mapped to this generic datatype.
 */
typedef void (*SerializeFunction)( void *, const char *, struct Serialize * );


void Char_serialize( char *value, const char *name, Serialize *serialize );

void SChar_serialize( signed char *value, const char *name, Serialize *serialize );

void UChar_serialize( unsigned char *value, const char *name, Serialize *serialize );

void SInt_serialize( short int *value, const char *name, Serialize *serialize );

void USInt_serialize( unsigned short int *value, const char *name, Serialize *serialize );

void Int_serialize( int *value, const char *name, Serialize *serialize );

void UInt_serialize( unsigned int *value, const char *name, Serialize *serialize );

void LInt_serialize( long int *value, const char *name, Serialize *serialize );

void LL_serialize( long long *value, const char *name, Serialize *serialize );

void ULL_serialize( unsigned long long *value, const char *name, Serialize *serialize );

void ULInt_serialize( unsigned long int *value, const char *name, Serialize *serialize );

void Float_serialize( float *value, const char *name, Serialize *serialize );

void Double_serialize( double *value, const char *name, Serialize *serialize );

/*
 * Check the gcc flag "-m128bit-long-double" for intel architecture
 * Note: not really working!!!
 */
void LDouble_serialize( long double *value, const char *name, Serialize *serialize );

void String_serialize( char *value, const char *name, int stringLen, Serialize *serialize );


void CharArray_serialize( char *value, const char *name, int arrayLen, Serialize *serialize );

void SCharArray_serialize( signed char *value, const char *name, int arrayLen, Serialize *serialize );

void UCharArray_serialize( unsigned char *value, const char *name, int arrayLen, Serialize *serialize );

void SIntArray_serialize( short int *value, const char *name, int arrayLen, Serialize *serialize );

void USIntArray_serialize( unsigned short int *value, const char *name, int arrayLen, Serialize *serialize );

void IntArray_serialize( int *value, const char *name, int arrayLen, Serialize *serialize );

void UIntArray_serialize( unsigned int *value, const char *name, int arrayLen, Serialize *serialize );

void LIntArray_serialize( long int *value, const char *name, int arrayLen, Serialize *serialize );

void LLArray_serialize( long long *value, const char *name, int arrayLen, Serialize *serialize );

void ULLArray_serialize( unsigned long long *value, const char *name, int arrayLen, Serialize *serialize );

void ULIntArray_serialize( unsigned long int *value, const char *name, int arrayLen, Serialize *serialize );

void FloatArray_serialize( float *value, const char *name, int arrayLen, Serialize *serialize );

void DoubleArray_serialize( double *value, const char *name, int arrayLen, Serialize *serialize );

/*
 * Check the gcc flag "-m128bit-long-double" for intel architecture
 */
void LDoubleArray_serialize( long double *value, const char *name, int arrayLen, Serialize *serialize );


#if defined(__cplusplus)
}
#endif

#endif /* SERIALIZETYPES_H */


/* EOF */
