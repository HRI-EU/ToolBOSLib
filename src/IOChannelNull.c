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


IOCHANNELINTERFACE_CREATE_PLUGIN( Null );


static void *IOChannelNull_new( void )
{
    return (void *)NULL;
}


static bool IOChannelNull_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return true;
}


static bool IOChannelNull_open( IOChannel *self, char *infoString,
                                IOChannelMode mode,
                                IOChannelPermissions permissions,
                                va_list varArg )
{
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_CHECKINFOSTRINGCORRECTNESS( infoString );

    return IOChannelNull_openFromString( self, vect );
}


static bool IOChannelNull_openFromString( IOChannel *self,
                                          IOChannelReferenceValue **referenceVector )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return true;
}


static long IOChannelNull_read( IOChannel *self, void *buffer, long size )
{
    return 0;
}


static long IOChannelNull_write( IOChannel *self, const void *buffer, long size )
{
    return size;
}


static long IOChannelNull_flush( IOChannel *self )
{
    return 0;
}


static long long IOChannelNull_seek( IOChannel *self, long long offset,
                                     IOChannelWhence whence )
{
    return 0;
}


static bool IOChannelNull_close( IOChannel *self )
{
    return true;
}


static void *IOChannelNull_getProperty( IOChannel *self, const char *propertyName )
{
    return (void *)NULL;
}


static bool IOChannelNull_setProperty( IOChannel *self, const char *propertyName,
                                       void *property )
{
    return false;
}


static void IOChannelNull_clear( IOChannel *self )
{
}


static void IOChannelNull_delete( IOChannel *self )
{
}


/* EOF */
