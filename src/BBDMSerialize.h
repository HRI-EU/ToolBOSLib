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


#ifndef BBDMSERIALIZE_H
#define BBDMSERIALIZE_H


/*--------------------------------------------------------------------------*/
/* Public includes                                                          */
/*--------------------------------------------------------------------------*/

#include <Any.h>
#include <BBDM-C.h>
#include <Serialize.h>
#include <BaseSerialize.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*--------------------------------------------------------------------------*/
/* Public functions                                                         */
/*--------------------------------------------------------------------------*/

/*!
 * \brief serialize function for the BBDMTag datatype
 *
 * \param self a valid BBDMTag instance
 * \param name human-readable name of the BBDMTag instance (only alphanumeric!)
 * \param stream a valid Serialize instance
 */
void BBDMTag_serialize( BBDMTag *self, char *name, Serialize *stream );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
