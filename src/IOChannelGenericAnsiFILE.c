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


#include <sys/types.h>
#include <sys/stat.h>

#include <IOChannelGenericAnsiFILE.h>

#if !defined(__windows__)

#include <unistd.h>

#endif


void *IOChannelGenericAnsiFILE_new( void )
{
    IOChannelGenericAnsiFILE *self = (IOChannelGenericAnsiFILE *)NULL;

    self = ANY_TALLOC( IOChannelGenericAnsiFILE );

    ANY_REQUIRE( self );

    return self;
}


bool IOChannelGenericAnsiFILE_init( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    bool retVal = true;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->fp = NULL;

    return retVal;
}


void IOChannelGenericAnsiFILE_setFp( IOChannel *self, void *fp )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;

    ANY_REQUIRE( self );

    /*
     * that's not needed because we can set it as NULL
     ANY_REQUIRE( fp );
     */

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->fp = (FILE *)fp;
}


void *IOChannelGenericAnsiFILE_getFp( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    FILE *retVal = (FILE *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = (void *)streamPtr->fp;

    return retVal;
}


long IOChannelGenericAnsiFILE_read( IOChannel *self, void *buffer, long size )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = fread( buffer, 1, size, streamPtr->fp );

    if( feof( streamPtr->fp ) != 0 )
    {
        IOCHANNEL_SET_EOF( self );
    }

    if(( retVal < size ) && ( self->foundEof == false ))
    {
        ANY_LOG( 5, "IOChannelGenericAnsiFILE_read(). Read bytes are less than "
                "size arg, but eof wasn't reached!", ANY_LOG_INFO );
    }

    return retVal;
}


long IOChannelGenericAnsiFILE_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    if( IOChannel_usesWriteBuffering( self ))
    {
        retVal = IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        streamPtr = IOChannel_getStreamPtr( self );
        ANY_REQUIRE( streamPtr );

        retVal = fwrite( buffer, 1, size, streamPtr->fp );

        if( retVal < size )
        {
            IOChannel_setError( self, IOCHANNELERROR_BLLW );
        }
    }
    return retVal;
}


long IOChannelGenericAnsiFILE_flush( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    long retVal = -1;
    void *ptr = (void *)NULL;
    long nBytes = 0;

    nBytes = IOChannel_getWriteBufferedBytes( self );
    ptr = IOChannel_getInternalWriteBufferPtr( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = fwrite( ptr, 1, nBytes, streamPtr->fp );

    if( retVal < nBytes )
    {
        IOChannel_setError( self, IOCHANNELERROR_BLLW );
    }

    return retVal;
}


long long IOChannelGenericAnsiFILE_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    long long retVal = -1;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

#if !defined(__windows__)
    retVal = (long long)fseek( streamPtr->fp, offset, whence );
#else
    retVal = (long long)_fseeki64( streamPtr->fp, offset, whence );
#endif

    return retVal;
}


long long IOChannelGenericAnsiFILE_tell( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    long long retVal = -1;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = ftell( streamPtr->fp );

    return retVal;
}


void *IOChannelGenericAnsiFILE_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( AnsiFile )
        {
            retVal = streamPtr->fp;
        }
        IOCHANNELPROPERTY_PARSE_END( AnsiFile )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


bool IOChannelGenericAnsiFILE_setProperty( IOChannel *self,
                                           const char *propertyName,
                                           void *property )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    return retVal;
}


void IOChannelGenericAnsiFILE_clear( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->fp = (FILE *)NULL;
}


void IOChannelGenericAnsiFILE_delete( IOChannel *self )
{
    IOChannelGenericAnsiFILE *streamPtr = (IOChannelGenericAnsiFILE *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


/* EOF */
