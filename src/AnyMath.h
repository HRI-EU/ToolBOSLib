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


#ifndef ANYMATH_H
#define ANYMATH_H


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \brief Determine if this system is using little endian.
 *
 * \return true if the system is little endian, false otherwise
 */
bool Any_isLittleEndian( void );


#define ANY_ISLITTLEENDIAN Any_isLittleEndian()


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ANYSTRING_H */

