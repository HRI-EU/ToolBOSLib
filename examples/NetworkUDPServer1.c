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


#include <memory.h>

#include <Any.h>
#include <BerkeleySocketServer.h>


static bool clientReadyCallBack( BerkeleySocket *self, void *data );

static bool timeoutCallBack( BerkeleySocket *self, void *data );


int main( int argc, char *argv[] )
{
    int                  serverPort = 60002;
    int                  maxClient  = 5;
    BerkeleySocketServer *server    = (BerkeleySocketServer *)NULL;

    /* alloc a new BerkeleySocketServer */
    server = BerkeleySocketServer_new();

    /* initialize the BerkeleySocketServer */
    if( BerkeleySocketServer_init( server, NULL ) == false )
    {
        ANY_LOG( 5, "Unable to initialize the BerkeleySocketServer",
                 ANY_LOG_FATAL );
        return ( 1 );
    }

    /* connect the BerkeleySocketServer */
    if( BerkeleySocketServer_connect( server, BERKELEYSOCKET_UDP, serverPort,
                                      maxClient ) == NULL )
    {
        ANY_LOG( 0, "Unable to connect the server", ANY_LOG_FATAL );
        goto serverExit;
    }

    ANY_LOG( 0, "Waiting a client ...", ANY_LOG_INFO );

    /* call the server main loop */
    BerkeleySocketServer_loop( server, clientReadyCallBack, NULL,
                               timeoutCallBack, NULL,
                               BERKELEYSOCKET_TIMEOUT_SECONDS( 1 ) );

    serverExit:

    ANY_LOG( 0, "Disconnecting the server ...", ANY_LOG_INFO );

    BerkeleySocketServer_disconnect( server );
    BerkeleySocketServer_clear( server );
    BerkeleySocketServer_delete( server );

    return ( 0 );
}


static bool clientReadyCallBack( BerkeleySocket *self, void *data )
{
    static int count      = 100;
    char       remoteIp[128];
    int        remotePort = 0;
    int        status     = 0;
    int        data1      = 0L;

    status = BerkeleySocket_read( self, (BaseUI8 *)&data1, sizeof( data1 ) );

    if( status == sizeof( data1 ) )
    {
        BerkeleySocket_getRemoteAddr( self, remoteIp, 128, &remotePort );

        ANY_LOG( 0, "New data is available from %s:%d", ANY_LOG_INFO, remoteIp,
                 remotePort );

        ANY_LOG( 0, "New data is: %d", ANY_LOG_INFO, (int)ntohl( data1 ) );
    }
    else
    {
        ANY_LOG( 0, "Error reading data", ANY_LOG_WARNING );
#if defined(__windows__)
        {
        char sockError[512];
        int errorCode = WSAGetLastError();

        BerkeleySocket_strerror( errorCode, sockError, 512);
        ANY_LOG( 0, "Error code is: %d ('%s')", ANY_LOG_ERROR, errorCode, sockError );
        }
#endif
    }

    /* we exit from the server loop only when timeout reach 0 */
    return ( ( --count ? false : true ) );
}


static bool timeoutCallBack( BerkeleySocket *self, void *data )
{
    static int count = 10;

    ANY_LOG( 0, "No data is available", ANY_LOG_INFO );

    /* we exit from the server loop only when timeout reach 0 */
    return ( ( --count ? false : true ) );
}


/* EOF */
