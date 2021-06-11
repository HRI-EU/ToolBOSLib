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


#include <Any.h>
#include <AnyTime.h>
#include <stdlib.h>
#include "BerkeleySocketServer.h"


int main( int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    BerkeleySocketServer *server;
    BerkeleySocket       *connection;
    BerkeleySocket       *serverSocket;
    char                 message[]   = "Hello World\n";
    size_t               len         = strlen( message ) + 1;
    bool                 isConnected = false;
    int                  numWritten;

    ANY_LOG( 0, "Start up", ANY_LOG_INFO );

    /* Create socket for "acceptClient" */
    connection = BerkeleySocket_new();
    BerkeleySocket_init( connection );

    /* Create socket for BerkeleySocketServer_connect, as setReuseAddr is desired */
    serverSocket = BerkeleySocket_new();
    BerkeleySocket_init( serverSocket );
    BerkeleySocket_setReuseAddr( serverSocket, true );

    /* Init server */
    server = BerkeleySocketServer_new();
    BerkeleySocketServer_init( server, serverSocket );
    if( BerkeleySocketServer_connect( server,
                                      BERKELEYSOCKET_TCP,
                                      12345,
                                      1 ) == NULL )
    {
        ANY_LOG( 0, "Can not bind to port", ANY_LOG_ERROR );
        return 1;
    }

    ANY_LOG( 0, "Entering main loop", ANY_LOG_INFO );
    for( int time = 0; time < 10; time++ )
    {
        if( isConnected ) /* If connected, send message. Upon error set isConnected to false */
        {
            numWritten = BerkeleySocket_write( connection,
                                               (BaseUI8 *)message,
                                               len );
            if( numWritten != (int)len )
            {
                ANY_LOG( 0, "Connection lost", ANY_LOG_INFO );
                /* Don't we need to do something here to signal the server that we
                   "close" the connection? */
                BerkeleySocket_disconnect( connection );
                isConnected = false;
            }
            Any_sleepSeconds( 1 );
        }
        else
        {
            /* Wait for a new client and set some flags */
            if( BerkeleySocketServer_waitClient( server, 1000000 ) )
            {
                if( BerkeleySocketServer_acceptClient( server, connection ) )
                {
                    ANY_LOG( 0, "New client", ANY_LOG_INFO );
                    BerkeleySocket_setBlocking( connection, false );
                    BerkeleySocket_setTcpNoDelay( connection, true );
                    isConnected = true;
                }
            }
        }
    }
    ANY_LOG( 0, "Exiting main loop", ANY_LOG_INFO );

    /* Clean up */
    if( isConnected )
    {
        BerkeleySocketServer_disconnect( server );
    }

    BerkeleySocketServer_clear( server );
    BerkeleySocketServer_delete( server );

    BerkeleySocket_clear( connection );
    BerkeleySocket_delete( connection );

    return EXIT_SUCCESS;
}


/* EOF */
