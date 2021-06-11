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


#ifndef IOCHANNELGENERICMEM_H
#define IOCHANNELGENERICMEM_H


#include <IOChannel.h>

#if !defined(__windows__)

#include <sys/mman.h>

#endif


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct IOChannelGenericMem
{
    int fd;
    void *ptr;
    long size;
    bool isMapped;
}
        IOChannelGenericMem;


void *IOChannelGenericMem_new( void );

bool IOChannelGenericMem_init( IOChannel *self );

void IOChannelGenericMem_setPtr( IOChannel *self, void *ptr,
                                 int fd, long size, bool isMapped );

long IOChannelGenericMem_read( IOChannel *self, void *buffer, long size );

long IOChannelGenericMem_write( IOChannel *self, const void *buffer, long size );

long IOChannelGenericMem_flush( IOChannel *self );

long long IOChannelGenericMem_seek( IOChannel *self, long long offset, IOChannelWhence whence );

bool IOChannelGenericMem_mapFd( IOChannel *self, int fd, long size );

bool IOChannelGenericMem_unmapFd( IOChannel *self );

void IOChannelGenericMem_clear( IOChannel *self );

void IOChannelGenericMem_delete( IOChannel *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
