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


#ifndef BERKELEYSOCKETBYTEORDER_H
#define BERKELEYSOCKETBYTEORDER_H


#include <Base.h>

#if !defined(__msvc__) && !defined(__windows__)

#include <netinet/in.h>

#endif

#if defined(__msvc__) || defined(__windows__)
#include <windows.h>
#include <winsock.h>
#endif


/*!
 * \brief converts 64 bit integer value from network to host byte order
 *
 * If necessary on this platform, the 64 bit integer \c value will be
 * converted from network to host byte order. If the machine
 * uses Big Endian format, the function will return the same value.
 *
 * \param value a 64 bit integer value in network byte order
 * \returns value of \c value in host byte order
 */
BaseI64 BerkeleySocket_ntohI64( BaseI64 value );

/*!
 * \brief converts 64 bit float value from network to host byte order
 *
 * If necessary on this platform, the 64 bit float \c value will be
 * converted from network to host byte order. If the machine
 * uses Big Endian format, the function will return the same value.
 *
 * \param value a 64 bit float value in network byte order
 * \returns value of \c value in host byte order
 */
BaseF64 BerkeleySocket_ntohF64( BaseF64 value );


#endif


/* EOF */
