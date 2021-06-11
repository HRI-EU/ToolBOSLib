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


#ifndef IOCHANNELGENERICFD_H
#define IOCHANNELGENERICFD_H


#include <IOChannel.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct IOChannelGenericFd
{
    bool isRegularFile;
    int fd;
}
        IOChannelGenericFd;


void *IOChannelGenericFd_new( void );

bool IOChannelGenericFd_init( IOChannel *self );

bool IOChannelGenericFd_setFd( IOChannel *self, int fd );

int IOChannelGenericFd_getFd( IOChannel *self );

int *IOChannelGenericFd_getFdPtr( IOChannel *self );

bool IOChannelGenericFd_unSet( IOChannel *self );

long IOChannelGenericFd_read( IOChannel *self, void *buffer, long size );

long IOChannelGenericFd_write( IOChannel *self, const void *buffer, long size );

long long IOChannelGenericFd_seek( IOChannel *self,
                                   long long offset, IOChannelWhence whence );

bool IOChannelGenericFd_truncate( IOChannel *self, long size );

bool IOChannelGenericFd_close( IOChannel *self );

void IOChannelGenericFd_clear( IOChannel *self );

void IOChannelGenericFd_delete( IOChannel *self );

#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
