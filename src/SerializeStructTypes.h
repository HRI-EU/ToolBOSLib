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


#ifndef SERIALIZESTRUCTTYPES_H
#define SERIALIZESTRUCTTYPES_H

#include <BaseTypes.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * \brief serialize array of structs
 *
 * \param __value pointer to the struct-array
 * \param __name name of the struct-array
 * \param __elementType type of the elements
 * \param __serializeFunction the serialize function of the elementType
 * \param __len number of elements in the array
 * \param __serialize the Serialize instance
 *
 * This macro can be used like in this example:
 *
 * \code
 * typedef struct Data
 * {
 *   .
 *   .
 *   SubStruct subs[5];
 * } Data;
 *
 * ...serializeFunction
 *
 * Serialize_beginType( s, "myData", "Data" );
 *   .
 *   .
 *   STRUCT_ARRAY_SERIALIZE( mydata->subs, "subs", "SubStruct", SubStruct_serialize, 5, s );
 * Serialize_endType( s );
 * \endcode
 */
#define STRUCT_ARRAY_SERIALIZE( __value, __name, __elementType, __serializeFunction, __len, __serialize ) \
do\
{\
  BaseUI32 __i = 0;\
\
  Serialize_beginStructArray( (__serialize), (__name), (__elementType), (__len) );\
  if ( (__serialize)->errorOccurred == true )\
  {\
    ANY_LOG( 3, "Can't find beginning of %s", ANY_LOG_INFO, (__name) );\
    break;\
  }\
\
  for ( __i = 0; __i < (__len); __i++ )\
  {\
    Serialize_beginStructArraySeparator( (__serialize), (__name), __i, __len );\
\
    __serializeFunction( &((__value)[__i]), (__name), (__serialize) );\
    if ( (__serialize)->errorOccurred == true )\
    {\
      ANY_LOG( 3, "can't find value of %s", ANY_LOG_INFO, (__name) );\
      break;\
    }\
\
    Serialize_endStructArraySeparator( (__serialize), (__name), __i, __len );\
  }\
\
  Serialize_endStructArray( __serialize );\
  if ( (__serialize)->errorOccurred == true )\
  {\
    ANY_LOG( 3, "can't find end of %s", ANY_LOG_INFO, (__name) );\
    break;\
  }\
} while( 0 )


#if defined(__cplusplus)
}
#endif


#endif /* SERIALIZESTRUCTTYPES_H */


/* EOF */
