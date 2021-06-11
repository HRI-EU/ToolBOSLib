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


/* some API parameters unused but kept for polymorphism */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <IOChannelGenericSocket.h>


long long IOChannelGenericSocket_seekBack( IOChannel *self, long long offset );

long long IOChannelGenericSocket_seekForward( IOChannel *self, long long offset );


void *IOChannelGenericSocket_new( void )
{
    IOChannelGenericSocket *self;

    self = ANY_TALLOC( IOChannelGenericSocket );

    ANY_REQUIRE( self );

    return self;
}


bool IOChannelGenericSocket_init( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->socketFd = -1;
    streamPtr->socket = (BerkeleySocket *)NULL;
    streamPtr->socketClient = (BerkeleySocketClient *)NULL;
    streamPtr->socketServer = (BerkeleySocketServer *)NULL;

    streamPtr->socketClient = BerkeleySocketClient_new();
    ANY_REQUIRE( streamPtr->socketClient );

    streamPtr->socketServer = BerkeleySocketServer_new();
    ANY_REQUIRE( streamPtr->socketServer );

    if( BerkeleySocketClient_init( streamPtr->socketClient,
                                   streamPtr->socket ) == false )
    {
        ANY_LOG( 5, "Unable to initialize the BerkeleySocketClient",
                 ANY_LOG_ERROR );
        goto exitLabel;
    }

    if( BerkeleySocketServer_init( streamPtr->socketServer, NULL ) == false )
    {
        ANY_LOG( 5, "Unable to initialize the BerkeleySocketServer",
                 ANY_LOG_ERROR );
        goto exitLabel;
    }
    retVal = true;

    exitLabel:;
    return retVal;
}


bool IOChannelGenericSocket_setSocket( IOChannel *self, BerkeleySocket *socket )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( socket != NULL )
    {
        streamPtr->socketFd = BerkeleySocket_getFd( socket );
        ANY_REQUIRE_MSG( streamPtr->socketFd > -1, "BerkeleySocket_getFd() returned -1!" );
        streamPtr->socket = socket;
        IOChannel_setType( self, IOCHANNELTYPE_SOCKET );
        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}


bool IOChannelGenericSocket_unsetSocket( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = true;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->socket = (BerkeleySocket *)NULL;

    return retVal;
}


long IOChannelGenericSocket_read( IOChannel *self, void *buffer, long size )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = BerkeleySocket_read( streamPtr->socket, buffer, size );

    if( retVal <= 0 )
    {
        if( IOChannelGenericSocket_isEof( self ))
        {
            ANY_LOG( 10, "Reading from Socket: Eof Was found!", ANY_LOG_INFO );

            IOCHANNEL_SET_EOF( self );
            retVal = 0;
        }
        else
        {
            IOChannel_setError( self, IOCHANNELERROR_BSOCKR );
        }
    }

    return retVal;
}


long IOChannelGenericSocket_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = BerkeleySocket_write( streamPtr->socket, (void *)buffer, size );

    if( retVal <= 0 )
    {
        if( IOChannelGenericSocket_isEof( self ))
        {
            ANY_LOG( 10, "Writing on Socket: Eof Was found!", ANY_LOG_INFO );
            IOCHANNEL_SET_EOF( self );
            retVal = 0;
        }
        else
        {
            IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
        }
    }

    if( retVal < size )
    {
        IOChannel_setError( self, IOCHANNELERROR_BLLW );
    }

    return retVal;
}


bool IOChannelGenericSocket_isEof( IOChannel *self )
{
    bool retVal = true;
    char *logMsg = NULL;

    /*
     * Our EOF emulation looks for the socket error below
     */
    switch( BerkeleySocket_errno())
    {
#ifdef ENETDOWN
        case ENETDOWN:
            logMsg = (char *)"ENETDOWN";
            break;
#endif

#ifdef ENETUNREACH
        case ENETUNREACH:
            logMsg = (char *)"ENETUNREACH";
            break;
#endif

#ifdef ENETRESET
        case ENETRESET:
            logMsg = (char *)"ENETRESET";
            break;
#endif

#ifdef ENOTCONN
        case ENOTCONN:
            logMsg = (char *)"ENOTCONN";
            break;
#endif

#ifdef ESHUTDOWN
        case ESHUTDOWN:
            logMsg = (char *)"ESHUTDOWN";
            break;
#endif

#ifdef EHOSTUNREACH
        case EHOSTUNREACH:
            logMsg = (char *)"EHOSTUNREACH";
            break;
#endif

#ifdef EHOSTDOWN
        case EHOSTDOWN:
            logMsg = (char *)"EHOSTDOWN";
            break;
#endif

#ifdef ECONNABORTED
        case ECONNABORTED:
            logMsg = (char *)"ECONNABORTED";
            break;
#endif

#ifdef ECONNRESET
        case ECONNRESET:
            logMsg = (char *)"ECONNRESET";
            break;
#endif

#ifdef ECONNREFUSED
        case ECONNREFUSED:
            logMsg = (char *)"ECONNREFUSED";
            break;
#endif

#ifdef EPIPE
        case EPIPE:
            logMsg = (char *)"EPIPE";
            break;
#endif

        default:
            retVal = false;
            break;
    }

    if( retVal )
    {
        ANY_LOG( 5, "Setting EOF on the Socket due to %s, errno=%d",
                 ANY_LOG_INFO, logMsg, BerkeleySocket_errno());
    }

    return retVal;
}


long long IOChannelGenericSocket_seek( IOChannel *self,
                                       long long offset, IOChannelWhence whence )
{
    long long retVal = -1;

    ANY_REQUIRE( self );

    switch( whence )
    {
        case IOCHANNELWHENCE_SET:
            self->rdBytesFromLastWrite = 0;
            retVal = 0;
            break;

        case IOCHANNELWHENCE_CUR:
        {
            if( offset == 0 )
            {
                retVal = self->currentIndexPosition;
            }
            else if( offset < 0 )
            {
                retVal = IOChannelGenericSocket_seekBack( self, offset );
            }
            else
            {
                retVal = IOChannelGenericSocket_seekForward( self, offset );
            }
            break;
        }

        case IOCHANNELWHENCE_END:
            self->rdBytesFromLastWrite = 0;
            retVal = 0;
            break;
        default:
            IOChannel_setError( self, IOCHANNELERROR_BWHESEK );
            retVal = -1;
            break;
    }

    return retVal;
}


void IOChannelGenericSocket_clear( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    BerkeleySocketClient_clear( streamPtr->socketClient );
    BerkeleySocketClient_delete( streamPtr->socketClient );

    BerkeleySocketServer_clear( streamPtr->socketServer );
    BerkeleySocketServer_delete( streamPtr->socketServer );

}


void IOChannelGenericSocket_delete( IOChannel *self )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );
    ANY_FREE( streamPtr );
}


long long IOChannelGenericSocket_seekBack( IOChannel *self, long long offset )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long retVal = -1;

    ANY_REQUIRE( self );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    if( self->rdBytesFromLastUnget > 0 )
    {
        if( -offset > ( ungetBuffer->index + self->rdBytesFromLastUnget ))
        {
            /* this assignment is never used, retVal is explicitly set below */
            /* retVal = self->currentIndexPosition; */
        }
        else
        {
            ungetBuffer->index += -offset;
            self->currentIndexPosition += offset;
        }
    }
    retVal = self->currentIndexPosition;

    return retVal;
}


long long IOChannelGenericSocket_seekForward( IOChannel *self, long long offset )
{
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long retVal = -1;

    ANY_REQUIRE( self );

    ungetBuffer = self->ungetBuffer;
    if( offset < ungetBuffer->index )
    {
        /* Seeking on unget buffer */
        ungetBuffer->index -= offset;
        retVal = self->currentIndexPosition - ungetBuffer->index;
    }
    else
    {
        ungetBuffer->index = 0;
        retVal = self->currentIndexPosition;
    }

    return retVal;
}


/* EOF */
