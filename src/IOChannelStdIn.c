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


#include <IOChannelGenericFd.h>
#include <IOChannelReferenceValue.h>

#if defined(__windows__)
#include <stdio.h>
#if defined(__msvc__)
#undef STDIN_FILENO
#define STDIN_FILENO _fileno( stdin )
#endif
#endif


IOCHANNELINTERFACE_CREATE_PLUGIN( StdIn );


static void *IOChannelStdIn_new( void )
{
    IOChannelGenericFd *self = (IOChannelGenericFd *)NULL;

    self = IOChannelGenericFd_new();

    ANY_REQUIRE( self );

    return self;
}


static bool IOChannelStdIn_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericFd_init( self );
}


static bool IOChannelStdIn_open( IOChannel *self, char *infoString,
                                 IOChannelMode mode,
                                 IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( infoString );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelStdIn_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    return retVal;
}


static bool IOChannelStdIn_openFromString( IOChannel *self,
                                           IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;
    bool isRdOnly = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    if( IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        isRdOnly = IOCHANNEL_MODEIS_R_ONLY( self->mode );

        if( IOCHANNEL_MODEIS_CREAT( self->mode ) ||
            IOCHANNEL_MODEIS_TRUNC( self->mode ) ||
            IOCHANNEL_MODEIS_APPEND( self->mode ) ||
            !isRdOnly )
        {
            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            retVal = false;
        }
        else
        {
            retVal = IOChannelGenericFd_setFd( self, STDIN_FILENO );
        }
    }
    else
    {
        self->mode = IOCHANNEL_MODE_R_ONLY;
        retVal = IOChannelGenericFd_setFd( self, STDIN_FILENO );
    }

    return retVal;
}


static long IOChannelStdIn_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericFd_read( self, buffer, size );
}


static long IOChannelStdIn_write( IOChannel *self, const void *buffer, long size )
{
    return -1;
}


static long IOChannelStdIn_flush( IOChannel *self )
{
    return 0;
}


static long long IOChannelStdIn_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    ANY_REQUIRE( self );

    return IOChannelGenericFd_seek( self, offset, whence );
}


static bool IOChannelStdIn_close( IOChannel *self )
{
    ANY_REQUIRE( self );

    if( IOCHANNEL_MODEIS_CLOSE( self->mode ))
    {
        return IOChannelGenericFd_close( self );
    }
    else
    {
        return IOChannelGenericFd_unSet( self );
    }
}


static void *IOChannelStdIn_getProperty( IOChannel *self, const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            retVal = (void *)IOChannelGenericFd_getFdPtr( self );
        }
        IOCHANNELPROPERTY_PARSE_END( Fd )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelStdIn_setProperty( IOChannel *self, const char *propertyName,
                                        void *property )
{
    return false;
}


static void IOChannelStdIn_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_clear( self );
}


static void IOChannelStdIn_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_delete( self );
}


/* EOF */
