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


/*!
 * \page Any_About Assertions, logging,...
 *
 * This module contains:
 *
 * \li \ref AnyDef.h Assertions
 * \li \ref AnyLog.h ASCII logging (to console / file)
 * \li \ref AnyTracing.h Binary logging
 * \li \ref AnyMath.h Endianess checks
 * \li \ref AnyMem.h Memory management
 * \li \ref AnyStdBool.h Bool-type definiton
 * \li \ref AnyTime.h Portable time functions
 *
 * \code
 *   int i = Any_strlen( "Hello World!" );                // O.S.-indep. string function
 *   ANY_REQUIRE_MSG( i < 20, "Input value too big!" );   // exit if condition failed
 *
 *   // ... some operation on i ...
 *
 *   ANY_LOG( 0, "Operation finished", ANY_LOG_INFO );    // standardized logging
 *   ANY_TRACE( 0, "%d", i );                             // standardized value tracing
 * \endcode
 */


#ifndef ANY_H
#define ANY_H

/*
 * Include bool definition for C language
 */
#include <AnyStdBool.h>

/*
 * Checks for redefinitions
 */
#ifdef ANY_REDEFINE
#include <AnyRedefine.h>
#endif

#include <AnyDef.h>
#include <AnyLog.h>
#include <AnyMem.h>
#include <AnyString.h>
#include <AnyTime.h>
#include <AnyValid.h>
#include <AnyMath.h>

#endif


/* EOF */
