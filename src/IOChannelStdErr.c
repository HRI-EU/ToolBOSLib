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
#undef STDERR_FILENO
#define STDERR_FILENO   _fileno( stderr )
#endif
#endif


IOCHANNELINTERFACE_CREATE_PLUGIN( StdErr );


static void *IOChannelStdErr_new( void )
{
    IOChannelGenericFd *self = (IOChannelGenericFd *)NULL;

    self = IOChannelGenericFd_new();

    ANY_REQUIRE( self );

    return self;
}


static bool IOChannelStdErr_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericFd_init( self );
}


static bool IOChannelStdErr_open( IOChannel *self, char *infoString,
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

    retVal = IOChannelStdErr_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );


    return retVal;
}


static bool IOChannelStdErr_openFromString( IOChannel *self,
                                            IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;
    bool isWrOnly = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    if( IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        isWrOnly = IOCHANNEL_MODEIS_W_ONLY( self->mode );

        if( IOCHANNEL_MODEIS_CREAT( self->mode ) ||
            IOCHANNEL_MODEIS_TRUNC( self->mode ) ||
            IOCHANNEL_MODEIS_APPEND( self->mode ) ||
            !isWrOnly )
        {
            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            retVal = false;
        }
        else
        {
            retVal = IOChannelGenericFd_setFd( self, STDERR_FILENO );
        }
    }
    else
    {
        self->mode = IOCHANNEL_MODE_W_ONLY;
        retVal = IOChannelGenericFd_setFd( self, STDERR_FILENO );
    }

    return retVal;
}


static long IOChannelStdErr_read( IOChannel *self, void *buffer, long size )
{
    return -1;
}


static long IOChannelStdErr_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    return IOChannelGenericFd_write( self, buffer, size );
}


static long IOChannelStdErr_flush( IOChannel *self )
{
    return 0;
}


static long long IOChannelStdErr_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    return -1;
}


static bool IOChannelStdErr_close( IOChannel *self )
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


static void *IOChannelStdErr_getProperty( IOChannel *self, const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            retVal = IOChannelGenericFd_getFdPtr( self );
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


static bool IOChannelStdErr_setProperty( IOChannel *self, const char *propertyName,
                                         void *property )
{
    return false;
}


static void IOChannelStdErr_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_clear( self );
}


static void IOChannelStdErr_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_delete( self );
}


/* EOF */
