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


/*
 * This file should make boolean types available for both older (C89) and
 * newer (C99) compilers. Some compilers implement C99 only partially, in
 * this case define either HAS_STDBOOL or HAS_NO_STDBOOL in the makefile.
 *
 * This source code has been placed into the PUBLIC DOMAIN by its author.
 * Implements subclause 7.16 of ISO/IEC 9899:1999 (E).
 */


#ifndef ANYSTDBOOL_H
#define ANYSTDBOOL_H

#if !defined(__cplusplus)       /* c++ uses builtin bool type */

#if   defined(HAS_STDBOOL)      /* please specify in makfile */

#include <stdbool.h>

#elif defined(HAS_NO_STDBOOL)   /* please specify in makefile */

#define DEFINE_BOOL_STUFF

#elif defined(ISO_C99) || ( defined(__STDC_VERSION__) && ( __STDC_VERSION__ > 199900 ))

#include <stdbool.h>

#else

#define DEFINE_BOOL_STUFF

#endif

#if defined(DEFINE_BOOL_STUFF) && !defined(_STDBOOL_H) && !defined(_INC_STDBOOL) /* usual lock names */

#define _STDBOOL_H
#define _INC_STDBOOL

#undef bool
#undef true
#undef false
#undef __bool_true_false_are_defined

#define bool                            _Bool
#define true                            1
#define false                           0
#define __bool_true_false_are_defined   1


/* The Microsoft Visual C compiler is pre-C99
 *
 * WARNING: MSVC in C++ mode sizeof(bool) == sizeof(char) so we are going
 * map it to char.
 */
#if defined( __msvc__ ) && defined( _MSC_VER ) && _MSC_VER < 1910  // VC pre 2017
typedef char _Bool;
#endif


#undef DEFINE_BOOL_STUFF

#endif

#endif

#endif


/* EOF */
