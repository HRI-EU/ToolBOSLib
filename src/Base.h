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


#ifndef BASE_H
#define BASE_H


/*!
 * \page Base_About Basic datatypes
 *
 * The Base.h contains:
 *
 * \li BaseBool type
 * \li scalar types such as "BaseUI8" (unsigned integer, at least 8 bit)
 * \li datatype ranges e.g. BASEUI8_MIN and BASEUI8_MAX
 * \li simple structured types e.g. Base2DI8 (2-dimensional, BaseI8 type
 *     for each of the two coordinates)
 * \li even more structured types e.g. Base3DI32vF64 (3-dimensional, BaseF32 type
 *     [32 bit float] for each of the two coordinates, and an assigned value of
 *     type BaseF64 [64 bit float], could be e.g. used to assign a temperature to
 *     a point within a 3D room)
 * \li complex numbers (e.g. BaseC32 with a real- and img-part)
 * \li memory chunks (e.g. MemI8) with a length property
 * \li ...
 *
 * The Base types are mapped to the standard C-types depending on the current
 * CPU architecture and operating system. For example, a BaseI64 is mapped to
 * signed long long on 32 bit Linux.
 * Keep it mind that the number (e.g. 64) in the typename might be mapped to
 * a bigger C-type if there is no such exact C type available on the platform.
 * However, the BASE*_MIN/BASE*_MAX constants always point to the same numbers,
 * namely the precise values we guarentee to fit into those values. Also
 * consider that e.g. BASEI32_MAX might be smaller than INT_MAX on the platform.
 *
 * In general, not only for the base-types, keep in mind that e.g. a single 16
 * bit integer will very likely occupy more RAM than just 16 bits (depending on
 * the memory alignment optimizations of the compiler and other platform
 * specific boundaries). Do not rely on the 16, it only means the type can hold
 * at least 16 bits on each supported platform without overflowing.
 *
 * You do not need to use these types. Whenever you want, you can stick to
 * the standard-types. If you want to be sure to get a type with a certain
 * amount of bits on every architecture, it is recommended to use these types.
 *
 * <h3>Example:</h3>
 * \code
 * ...
 * BaseBool itemFound = false;
 * BaseF32  itemValue = 1.00;
 * ...
 * \endcode
 */


#include <BaseTypes.h>
#include <BaseMath.h>
#include <Base2DX.h>
#include <MemX.h>


#endif
