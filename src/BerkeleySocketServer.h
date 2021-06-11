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


#ifndef BERKELEYSOCKETSERVER_H
#define BERKELEYSOCKETSERVER_H

#include <Any.h>
#include <BerkeleySocket.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct BerkeleySocketServer
{
    unsigned long valid;
    bool created;
    int serverAddr;
    BerkeleySocket *socket;
    bool broadcast;
}
BerkeleySocketServer;


BerkeleySocketServer *BerkeleySocketServer_new( void );

/*!
 * \brief Initializes a BerkeleySocketServer
 * \param self BerkeleySocketServer instance pointer
 * \param sock BerkeleySocket instance pointer
 *
 * This function initialize a BerkeleySocketServer instance with the BerkeleySocket instance
 * parameter when specified. If the user sets the BerkeleySocket instance pointer as \a NULL,
 * the initialization function will instantiate an empty BerkeleySocket with standard default
 * values and the user may get the BerkeleySocket pointer by the \a BerkeleySocketServer_getSocket()
 * function call if needed before the \a BerkeleySocketServer_connect().
 *
 * \return Return \a true on success, \a false otherwise
 *
 * The example below shows how to initialize a BerkeleySocketServer with a standard BerkeleySocket's values:
 *
 * \code
 *         BerkeleySocketServer server;
 *
 *         if ( BerkeleySocketServer_init( &server, NULL ) == false )
 *         {
 *            ANY_LOG( 5, "Unable to initialize a BerkeleySocketServer", ANY_LOG_ERROR );
 *            exit( -1 );
 *         }
 * \endcode
 *
 * The example below shows how to initialize a BerkeleySocketServer with a user's defined BerkeleySocket's values:
 *
 * \code
 *         BerkeleySocket *mySock = (BerkeleySocket*)NULL;
 *         BerkeleySocketServer server;
 *
 *         mySock = BerkeleySocket_new();
 *         ANY_REQUIRE( mySock );
 *
 *         if ( BerkeleySocket_init( mySock ) == false )
 *         {
 *            ANY_LOG( 5, "Unable to initialize mySock", ANY_LOG_ERROR );
 *            exit( -1 );
 *         }
 *
 *         // my own BerkeleySocket settings
 *         BerkeleySocket_setBlocking( mySock, false );
 *         BerkeleySocket_setTcpNoDelay( mySock, true );
 *         BerkeleySocket_setReuseAddr( mySock, true );
 *         BerkeleySocket_setSndBuffer( mySock, 8192 );
 *         BerkeleySocket_setRcvBuffer( mySock, 8192 );
 *
 *         // initialize the BerkeleySocketClient with my own settings
 *         if ( BerkeleySocketServer_init( &server, mySock ) == false )
 *         {
 *            ANY_LOG( 5, "Unable to initialize a BerkeleySocketServer", ANY_LOG_ERROR );
 *            exit( -1 );
 *         }
 * \endcode
 */
bool BerkeleySocketServer_init( BerkeleySocketServer *self, BerkeleySocket *sock );

void BerkeleySocketServer_setBroadcast( BerkeleySocketServer *self, bool broadcast );

BerkeleySocket *BerkeleySocketServer_getSocket( BerkeleySocketServer *self );

BerkeleySocket *BerkeleySocketServer_connect( BerkeleySocketServer *self, BerkeleySocketType type, int portNo,
                                              int maxClient );

/*!
 * \brief Accept a new Client for a given BerkeleySocketServer
 * \param self BerkeleySocketServer instance pointer
 * \param newBerkeleySocket BerkeleySocket instance pointer where to store the client's parameter
 *
 * This function must be used after the BerkeleySocketServer is connected to accept a new client.
 * This function is blocking unless the user sets the \a BerkeleySocket_tcpBlocking() to false
 * on the BerkeleySocketServer specific BerkeleySocket options. To avoid to block the user may use the
 * \a BerkeleySocketServer_waitClient() function call instead to wait a new client.
 *
 * \return Return \a true on success, \a false otherwise
 */
bool BerkeleySocketServer_acceptClient( BerkeleySocketServer *self, BerkeleySocket *newBerkeleySocket );

/*!
 * \brief Wait a client connection on the specific BerkeleySocketServer
 *
 * Wait for a remote client until timeout of \a microsecs occurs. If \a microsecs
 * is \a 0 the function waits forever.
 */
bool BerkeleySocketServer_waitClient( BerkeleySocketServer *self, const long microsecs );

/*!
 * \brief Main loop server
 * \param self BerkeleySocketServer instance pointer
 * \param clientReadyCallBack Function call back when a new data is available
 * \param data1 User's data for the \a dataReadyCallBack function
 * \param timeoutCallBack Function call back when waiting data timeout expires
 * \param data2 User's data for the \a timeoutCallBack function
 * \param timeout Timeout expressed in microseconds
 *
 * The \a BerkeleySocketServer_loop() implements a mini BerkeleySocket server loop which waits
 * for a new incoming client connection with a timeout specified by the \a timeout parameter.
 * When a new incoming client connected is ready to be handled the \a BerkeleySocketServer_loop() calls
 * the \a clientReadyCallBack function if user define it. The \a clientReadyCallBack function
 * will be called by passing as first parameter (self) a reference of the instance client's BerkeleySocket and as the
 * second parameter the user's data specified by the \a data1 parameter of the \a BerkeleySocketServer_loop().
 * The user have to take in consideration when coding the \a clientReadyCallBack function that he had to
 * \a BerkeleySocket_clone() the received socket instance because the \a BerkeleySocketServer_loop() will recycle the
 * BerkeleySocket instance pointer for the next incoming client. The \a BerkeleySocket_clone() could be avoided if
 * the user creates TCP servers without the \a BerkeleySocket_setReuseAddr() option setted as true, or creates UDP
 * BerkeleySocketServer. In this case the BerkeleySocketServer doesn't have to handle more than one client at the
 * same time making the program's flow straight sequential. If the user don't specifies the \a clientReadyCallBack
 * the incoming client connection will be accepted and closed immediatly because no other work could be done.
 *
 * If no incoming client connection are present and the timeout expires the \a BerkeleySocketServer_loop() calls
 * the user's \a timeoutCallBack function if specified. The \a timeoutCallBack function will be called by
 * the \a BerkeleySocketServer_loop() by passing as first parameter the BerkeleySocket instance of the referenced BerkeleySocketServer
 * and as second parameter the user's data specified by the \a data1 BerkeleySocketServer_loop() parameter.
 *
 * The user could exit from the \a BerkeleySocketServer_loop() by returning \a false from the \a clientReadyCallBack
 * or \a timeoutCallBack
 */
void BerkeleySocketServer_loop( BerkeleySocketServer *self,
                                bool (*clientReadyCallBack)( BerkeleySocket *, void * ),
                                void *data1,
                                bool (*timeoutCallBack)( BerkeleySocket *, void * ),
                                void *data2,
                                long timeout );

void BerkeleySocketServer_disconnect( BerkeleySocketServer *self );

void BerkeleySocketServer_clear( BerkeleySocketServer *self );

void BerkeleySocketServer_delete( BerkeleySocketServer *self );

bool BerkeleySocketServer_setServerAddr( BerkeleySocketServer *self, int serverAddr );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
