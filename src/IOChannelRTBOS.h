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


#ifndef IOCHANNELRTBOS_H
#define IOCHANNELRTBOS_H


#include <IOChannel.h>
#include <BerkeleySocket.h>
#include <BerkeleySocketClient.h>


#define IOCHANNELRTBOS_PATHSIZE_MAXLEN                                       512
#define IOCHANNELRTBOS_SOCKET_TIMEOUT                                         10
#define IOCHANNELRTBOS_REPOSITORYPATH_PREFIX               "/Repository/Output/"
#define IOCHANNELRTBOS_CMDREAD                          "Serialize( %s, \"\")\n"
#define IOCHANNELRTBOS_CMDBLOCKINGREAD           "PassiveSerialize( %s, \"\")\n"
#define IOCHANNELRTBOS_CMDWRITE                                  "Deserialize\n"
#define IOCHANNELRTBOS_FORMATLENGTH                                           64


typedef struct IOChannelRTBOS
{
    bool isBeginType;
    bool isBlocking;
    char format[IOCHANNELRTBOS_FORMATLENGTH];
    BerkeleySocket *socket;
    BerkeleySocketClient *socketClient;
    int socketFd;
    AnyEventInfo *onEndSerialize;
} IOChannelRTBOS;


#endif


/* EOF */
