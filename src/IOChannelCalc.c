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


#include <IOChannel.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( Calc );


typedef struct IOChannelCalc
{
    long numWriteCalls;
    long maxSize;
    long minSize;
}
        IOChannelCalc;


static void *IOChannelCalc_new( void )
{
    IOChannelCalc *self;

    self = ANY_TALLOC( IOChannelCalc );

    ANY_REQUIRE( self );

    return self;
}


static bool IOChannelCalc_init( IOChannel *self )
{
    IOChannelCalc *streamPtr = (IOChannelCalc *)NULL;

    ANY_REQUIRE( self );

    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->numWriteCalls = 0;
    streamPtr->maxSize = 0;
    streamPtr->minSize = 0;

    return true;
}


static bool IOChannelCalc_open( IOChannel *self, char *infoString,
                                IOChannelMode mode,
                                IOChannelPermissions permissions,
                                va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelCalc_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );


    return retVal;
}


static bool IOChannelCalc_openFromString( IOChannel *self,
                                          IOChannelReferenceValue **referenceVector )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    if( IOCHANNEL_MODEIS_W_ONLY( self->mode ))
    {
        return true;
    }
    else
    {
        ANY_LOG( 5, "IOChannelCalc_open() accepts IOCHANNEL_MODE_W_ONLY flag only",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BFLGS );
        return false;
    }
}


static long IOChannelCalc_read( IOChannel *self, void *buffer, long size )
{
    return -1;
}


static long IOChannelCalc_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelCalc *streamPtr = (IOChannelCalc *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    /* Comment the block to disable statistics */
    {
        streamPtr = IOChannel_getStreamPtr( self );
        ANY_REQUIRE( streamPtr );

        ( streamPtr->numWriteCalls )++;
        streamPtr->maxSize = ( size > streamPtr->maxSize ?
                               size : streamPtr->maxSize );
        streamPtr->minSize = ( size < streamPtr->minSize ?
                               size : streamPtr->minSize );
    }

    return size;
}


static long IOChannelCalc_flush( IOChannel *self )
{
    return 0;
}


static long long IOChannelCalc_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelCalc_close( IOChannel *self )
{
    ANY_REQUIRE( self );
    IOChannel_valid( self );

    return true;
}


static void *IOChannelCalc_getProperty( IOChannel *self, const char *propertyName )
{
    return (void *)NULL;
}


static bool IOChannelCalc_setProperty( IOChannel *self, const char *propertyName,
                                       void *property )
{
    return false;
}


static void IOChannelCalc_clear( IOChannel *self )
{
    IOChannelCalc *streamPtr = (IOChannelCalc *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->numWriteCalls = 0;
    streamPtr->maxSize = 0;
    streamPtr->minSize = 0;
}


static void IOChannelCalc_delete( IOChannel *self )
{
    IOChannelCalc *streamPtr = (IOChannelCalc *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


/* EOF */
