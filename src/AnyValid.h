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


#ifndef ANYVALID_H
#define ANYVALID_H

#include <AnyDef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief Macro to declare the valid attribute in object structs.
 *
 * This macro should be called in the struct declaration.
 */
#ifndef ANY_VALID_DECLARE
#define ANY_VALID_DECLARE ANY_VALID_TYPE valid
#endif

/*!
 * \brief Type of the validation attribute
 */
#ifndef ANY_VALID_TYPE
#define ANY_VALID_TYPE  void*
#endif

/*!
 * \brief Macro to clear the validation attribute.
 * \param __self Object where validation attribute will be cleared
 *
 * This macro unsets the validation pointer of an object instance to
 * type. This macro should be invoked at the beginning of _init methods
 * and in _clear methods.
 *
 * \see ANY_VALID_SET(), ANY_VALID_REQUIRE()
 */
#ifndef ANY_VALID_UNSET
#define ANY_VALID_UNSET( __self ) \
do {  \
  __self->valid = NULL;\
} while( 0 )
#endif


/*!
 * \brief Macro to set the validation attribute to type.
 * \param __self Object where valid attribute will be setted
 * \param __type Type of the object
 *
 * This macro sets the validation pointer of an object instance to
 * type. This macro should be invoked at the end of _init methods.
 *
 * \see ANY_VALID_UNSET(), ANY_VALID_REQUIRE()
 */
#ifndef ANY_VALID_SET
#define ANY_VALID_SET( __self, __type ) \
do {  \
  __self->valid = (ANY_VALID_TYPE)#__type;\
} while( 0 )
#endif


/*!
 * \brief Macro to validate an Object istance
 * \param __self Object to check
 * \param __type Type that object should match
 *
 * This macro is used to check if an object instance is valid.
 * Where:
 *
 *  <B>__self</B> - is the object to check.
 *
 *  <B>__type</B> - is the type the object must match
 *
 * Example:
 * \code
 *    ANY_VALID_REQUIRE( self, TestType )
 *
 * \endcode
 *
 * In case of error an ANY_REQUIRE is fired up.
 */
#ifndef ANY_VALID_REQUIRE
#define ANY_VALID_REQUIRE( __self, __type ) \
  ANY_REQUIRE( __self->valid == (ANY_VALID_TYPE)#__type )
#endif


#if defined(__cplusplus)
}
#endif

#endif /* END ANYVALID.H */
