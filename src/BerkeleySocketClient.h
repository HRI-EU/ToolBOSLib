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


#ifndef BERKELEYSOCKETCLIENT_H
#define BERKELEYSOCKETCLIENT_H

#include <Any.h>
#include <BerkeleySocket.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct BerkeleySocketClient
{
    unsigned long valid;
    bool created;
    BerkeleySocket *socket;
    bool broadcast;
}
BerkeleySocketClient;


BerkeleySocketClient *BerkeleySocketClient_new( void );

/*!
 * \brief Initializes a client-socket
 * \param self BerkeleySocketClient instance pointer
 * \param sock BerkeleySocket instance pointer
 *
 * This function initialize a BerkeleySocketClient instance with the BerkeleySocket instance
 * parameter when specified. If the user sets the BerkeleySocket instance pointer as \a NULL,
 * the initialization function will instantiate an empty BerkeleySocket with standard default
 * values and the user may get the BerkeleySocket pointer by the \a BerkeleySocketClient_getSocket()
 * function call if needed before the \a BerkeleySocketClient_connect().
 *
 * \return Return \a true on success, \a false otherwise
 */
bool BerkeleySocketClient_init( BerkeleySocketClient *self, BerkeleySocket *sock );

void BerkeleySocketClient_setBroadcast( BerkeleySocketClient *self, bool broadcast );

BerkeleySocket *BerkeleySocketClient_getSocket( BerkeleySocketClient *self );

BerkeleySocket *BerkeleySocketClient_connect( BerkeleySocketClient *self, BerkeleySocketType type, char *serverIp,
                                              int portNo );

BerkeleySocket *BerkeleySocketClient_connectEx( BerkeleySocketClient *self,
                                                BerkeleySocketType type,
                                                char *serverIp, int portNo, int srcPortNo );

void BerkeleySocketClient_disconnect( BerkeleySocketClient *self );

void BerkeleySocketClient_clear( BerkeleySocketClient *self );

void BerkeleySocketClient_delete( BerkeleySocketClient *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
