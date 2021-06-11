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


#ifndef IOCHANNELGENERICSOCKET_H
#define IOCHANNELGENERICSOCKET_H


#include <IOChannel.h>
#include <BerkeleySocket.h>
#include <BerkeleySocketClient.h>
#include <BerkeleySocketServer.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct IOChannelGenericSocket
{
    int socketFd;
    BerkeleySocket *socket;
    BerkeleySocketClient *socketClient;
    BerkeleySocketServer *socketServer;
}
        IOChannelGenericSocket;


void *IOChannelGenericSocket_new( void );

bool IOChannelGenericSocket_init( IOChannel *self );

bool IOChannelGenericSocket_setSocket( IOChannel *self, BerkeleySocket *socket );

bool IOChannelGenericSocket_unsetSocket( IOChannel *self );

long IOChannelGenericSocket_read( IOChannel *self, void *buffer, long size );

long IOChannelGenericSocket_write( IOChannel *self, const void *buffer, long size );

bool IOChannelGenericSocket_isEof( IOChannel *self );

long long IOChannelGenericSocket_seek( IOChannel *self,
                                       long long offset, IOChannelWhence whence );

void IOChannelGenericSocket_clear( IOChannel *self );

void IOChannelGenericSocket_delete( IOChannel *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
