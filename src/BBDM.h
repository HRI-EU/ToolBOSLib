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


#ifndef BBDM_H
#define BBDM_H


/*!
 * \page BBDM_About BBDM (Data components)
 *
 * BBDM stands for "Brain-Bytes Data Model" and defines a software
 * component model in C/C++ for encapsulating data.
 *
 * With the help of that, different teams can create and ship software
 * components that can be used to assemble an application. As an example,
 * you can consider someone creating an application in Germany using some
 * piece of code created in Japan, without having to understand the way
 * it was implemented but just focussing on its interface.
 *
 * Similar to BBCM.h this headerfile defines macros commonly used in all
 * data components.
 *
 * \li BBDM-C.h
 * \li BBDM-Cpp.h
 *
 */


#if defined(__cplusplus)
#include <BBDM-Cpp.h>
#else

#include <BBDM-C.h>

#endif

#endif


/* EOF */
