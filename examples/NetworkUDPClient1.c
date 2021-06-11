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


#include <errno.h>
#include <memory.h>

#include <Any.h>
#include <BerkeleySocketClient.h>


int main( int argc, char *argv[] )
{
    char                 localhost[] = "localhost";
    char                 *serverName = localhost;
    char                 hostName[128];
    char                 *serverIp   = (char *)NULL;
    int                  serverPort  = 60002;
    int                  count       = 20;
    int                  retry       = 20;  // UDP has no "connection"-state
    bool                 connected   = false;
    int                  status      = 0;
    int                  data        = 0L;
    int                  r           = 0L;
    BerkeleySocketClient *client     = (BerkeleySocketClient *)NULL;
    BerkeleySocket       *sock       = (BerkeleySocket *)NULL;

    if( argc > 1 )
    {
        serverName = argv[ 1 ];
    }

    serverIp = BerkeleySocket_host2Addr( serverName, hostName, 128 );
    ANY_REQUIRE( serverIp );

    /* alloc a new BerkeleySocketClient */
    client = BerkeleySocketClient_new();

    /* initialize the BerkeleySocketServer */
    if( BerkeleySocketClient_init( client, NULL ) == false )
    {
        ANY_LOG( 5, "Unable to initialize the BerkeleySocketClient",
                 ANY_LOG_FATAL );
        return ( 1 );
    }

    BerkeleySocket_setDefaultTimeout( BerkeleySocketClient_getSocket( client ),
                                      BERKELEYSOCKET_TIMEOUT_SECONDS( 10 ) );

    ANY_LOG( 0, "Connecting to %s:%d (%s:%d)...", ANY_LOG_INFO, serverName,
             serverPort, serverIp, serverPort );

    sock = BerkeleySocketClient_connect( client, BERKELEYSOCKET_UDP,
                                         serverIp, serverPort );

    do
    {
        if( sock )
        {
            connected = true;

            while( count-- )
            {
                r = rand();

                ANY_LOG( 0, "Sending random number %d ...", ANY_LOG_INFO, r );

                data   = htonl( r );
                status = BerkeleySocket_write( sock, (BaseUI8 *)&data,
                                               sizeof( data ) );

                // with UDP packages we typically do not care about delivery
//                if( status != sizeof( data ) )
//                {
//                    char error[128];
//
//                    BerkeleySocket_strerror( errno, error, 128 );
//
//                    ANY_LOG( 0,
//                             "Unable to send data to the server %s:%d, error '%s'",
//                             ANY_LOG_FATAL, serverName, serverPort, error );
//                }

                Any_sleepSeconds( 1 );
            }

            ANY_LOG( 0, "Disconnecting the client ...", ANY_LOG_INFO );
            BerkeleySocketClient_disconnect( client );
        }
        else
        {
            ANY_LOG( 0, "Unable to connect to the server %s:%d", ANY_LOG_FATAL,
                     serverName, serverPort );

            Any_sleepSeconds( 1 );

            retry--;
        }
    }
    while( ! connected && retry > 0 );

    BerkeleySocketClient_clear( client );
    BerkeleySocketClient_delete( client );

    return ( 0 );
}


/* EOF */
