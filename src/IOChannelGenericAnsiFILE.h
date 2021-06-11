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


#ifndef IOCHANNELGENERICANSIFILE_H
#define IOCHANNELGENERICANSIFILE_H


#include <IOChannel.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct IOChannelGenericAnsiFILE
{
    FILE *fp;
}
        IOChannelGenericAnsiFILE;


void *IOChannelGenericAnsiFILE_new( void );

bool IOChannelGenericAnsiFILE_init( IOChannel *self );

void IOChannelGenericAnsiFILE_setFp( IOChannel *self, void *fp );

void *IOChannelGenericAnsiFILE_getFp( IOChannel *self );

long IOChannelGenericAnsiFILE_read( IOChannel *self, void *buffer, long size );

long IOChannelGenericAnsiFILE_write( IOChannel *self, const void *buffer, long size );

long IOChannelGenericAnsiFILE_flush( IOChannel *self );

long long IOChannelGenericAnsiFILE_seek( IOChannel *self, long long offset, IOChannelWhence whence );

long long IOChannelGenericAnsiFILE_tell( IOChannel *self );

void *IOChannelGenericAnsiFILE_getProperty( IOChannel *self, const char *propertyName );

bool IOChannelGenericAnsiFILE_setProperty( IOChannel *self,
                                           const char *propertyName,
                                           void *property );

void IOChannelGenericAnsiFILE_clear( IOChannel *self );

void IOChannelGenericAnsiFILE_delete( IOChannel *self );

#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
