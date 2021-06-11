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
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( Socket );


static void *IOChannelSocket_new( void )
{
    return IOChannelGenericSocket_new();
}


static bool IOChannelSocket_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericSocket_init( self );
}


static bool IOChannelSocket_open( IOChannel *self, char *infoString,
                                  IOChannelMode mode,
                                  IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    BerkeleySocket *socket = (BerkeleySocket *)NULL;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNEL_GET_ARGUMENT( socket, BerkeleySocket * , varArg );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( pointer, "%p", (void *)socket );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelSocket_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    return retVal;
}


static bool IOChannelSocket_openFromString( IOChannel *self,
                                            IOChannelReferenceValue **referenceVector )
{
    /* No any check on modes are needed... */
    bool retVal = false;
    BerkeleySocket *socket = (BerkeleySocket *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        self->mode = IOCHANNEL_MODE_RW;
    }

    socket = (BerkeleySocket *)IOChannelReferenceValue_getPtr( referenceVector,
                                                               IOCHANNELREFERENCEVALUE_POINTER );
    if( !socket )
    {
        ANY_LOG( 5, "Error. Socket pointer not found in openString or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BOARG );
        goto outLabel;
    }

    retVal = IOChannelGenericSocket_setSocket( self, socket );

    outLabel:;
    return retVal;
}


static long IOChannelSocket_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericSocket_read( self, buffer, size );
}


static long IOChannelSocket_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    if( IOChannel_usesWriteBuffering( self ) )
    {
        return IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        return IOChannelGenericSocket_write( self, buffer, size );
    }
}


static long IOChannelSocket_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericSocket_write( self, ptr, nBytes );
}


static long long IOChannelSocket_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelSocket_close( IOChannel *self )
{
    int status = -1;
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( IOCHANNEL_MODEIS_NOTCLOSE( self->mode ) )
    {
        retVal = true;
    }
    else
    {
        ANY_LOG( 3, "Disconnecting the socket", ANY_LOG_INFO );
        status = BerkeleySocket_disconnect( streamPtr->socket );
        retVal = status == 0;
    }

    IOChannelGenericSocket_unsetSocket( self );

    return retVal;
}


static void *IOChannelSocket_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelGenericSocket *streamPtr = (IOChannelGenericSocket *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            retVal = (void *)&( streamPtr->socketFd );
        }
        IOCHANNELPROPERTY_PARSE_END( Fd )

        IOCHANNELPROPERTY_PARSE_BEGIN( Socket )
        {
            retVal = BerkeleySocketClient_getSocket( streamPtr->socketClient );
        }
        IOCHANNELPROPERTY_PARSE_END( Socket )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelSocket_setProperty( IOChannel *self, const char *propertyName,
                                         void *property )
{
    return false;
}


static void IOChannelSocket_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_clear( self );
}


static void IOChannelSocket_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericSocket_delete( self );
}


/* EOF */
