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


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <IOChannelGenericFd.h>
#include <IOChannelReferenceValue.h>

/*
 * On windows platform the open() behaves diffently because
 * instead to open() a file in binary mode it will open()
 * in text mode, so let make it working as unix does
 */
#if defined(__windows__)
#include <io.h>
#define IOCHANNEL_MODE_O_BINARY O_BINARY
#else
#define IOCHANNEL_MODE_O_BINARY 0
#endif


IOCHANNELINTERFACE_CREATE_PLUGIN( File );


static void *IOChannelFile_new( void )
{
    return IOChannelGenericFd_new();
}


static bool IOChannelFile_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericFd_init( self );
}


static bool IOChannelFile_open( IOChannel *self, char *infoString,
                                IOChannelMode mode,
                                IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "File stream needs a file name.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( name, "%s", infoString );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelFile_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


static bool IOChannelFile_openFromString( IOChannel *self,
                                          IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;
    int fd = -1;
    char *fileName = (char *)NULL;
    IOChannelPermissions permissions = (IOChannelPermissions)0;
    char *value = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    fileName = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_NAME );

    if( !fileName )
    {
        ANY_LOG( 5, "Error. File name not found in openString or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    value = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PERM );
    if( value )
    {
        permissions = IOChannelReferenceValue_getAccessPermissions( value );
    }
    else
    {
        permissions = IOCHANNEL_PERMISSIONS_ALL;
    }

    if( !IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        self->mode = IOCHANNEL_MODE_R_ONLY;
    }

#if defined(__windows__)
    fd = _open( fileName, self->mode | IOCHANNEL_MODE_O_BINARY );
#else
    fd = open( fileName, self->mode | IOCHANNEL_MODE_O_BINARY | O_LARGEFILE, permissions );
#endif

    retVal = IOChannelGenericFd_setFd( self, fd );

    outLabel:;
    return retVal;
}


static long IOChannelFile_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size >= 0 );

    return IOChannelGenericFd_read( self, buffer, size );
}


static long IOChannelFile_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size >= 0 );

    if( IOChannel_usesWriteBuffering( self ) )
    {
        return IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        return IOChannelGenericFd_write( self, buffer, size );
    }
}


static long IOChannelFile_flush( IOChannel *self )
{
    void *ptr = (void *)NULL;
    long nBytes = 0;

    ANY_REQUIRE( self );

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    return IOChannelGenericFd_write( self, ptr, nBytes );
}


static long long IOChannelFile_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    ANY_REQUIRE( self );

    return IOChannelGenericFd_seek( self, offset, whence );
}


static bool IOChannelFile_close( IOChannel *self )
{
    ANY_REQUIRE( self );

    if( IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        return IOChannelGenericFd_unSet( self );
    }
    else
    {
        return IOChannelGenericFd_close( self );
    }
}


static void *IOChannelFile_getProperty( IOChannel *self, const char *propertyName )
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


static bool IOChannelFile_setProperty( IOChannel *self, const char *propertyName,
                                       void *property )
{
    return false;
}


static void IOChannelFile_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_clear( self );
}


static void IOChannelFile_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericFd_delete( self );
}


/* EOF */
