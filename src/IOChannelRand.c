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


IOCHANNELINTERFACE_CREATE_PLUGIN( Rand );


#define IOCHANNELRAND_BUFFER_SIZE     120


static void IOChannelRand_fillBuffer( IOChannel *self );

typedef enum IOChannelRandType
{
    IOCHANNELRANDTYPE_INTEGERS,
    IOCHANNELRANDTYPE_FLOATS,
    IOCHANNELRANDTYPE_CHARS,
    IOCHANNELRANDTYPE_PRINTABLES,
    IOCHANNELRANDTYPE_NONE,
}
        IOChannelRandType;

typedef struct IOChannelRand
{
    char buffer[IOCHANNELRAND_BUFFER_SIZE];
    int index;
    int size;
    IOChannelRandType type;
}
        IOChannelRand;


static void *IOChannelRand_new( void )
{
    IOChannelRand *self = (IOChannelRand *)NULL;

    self = ANY_TALLOC( IOChannelRand );

    ANY_REQUIRE( self );

    return self;
}


static bool IOChannelRand_init( IOChannel *self )
{
    IOChannelRand *streamPtr = (IOChannelRand *)NULL;

    ANY_REQUIRE( self );

    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->type = IOCHANNELRANDTYPE_NONE;
    streamPtr->index = 0;
    streamPtr->size = IOCHANNELRAND_BUFFER_SIZE;
    Any_memset( streamPtr->buffer, ' ', IOCHANNELRAND_BUFFER_SIZE );

    return true;
}


static bool IOChannelRand_open( IOChannel *self,
                                char *infoString,
                                IOChannelMode mode,
                                IOChannelPermissions permissions,
                                va_list varArg )
{
    bool retVal = false;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelRand_open(). Not valid info string. "
                         "Rand stream needs a name that specifies a valid random type.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( name, "%s", infoString );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelRand_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


static bool IOChannelRand_openFromString( IOChannel *self,
                                          IOChannelReferenceValue **referenceVector )
{
    IOChannelRand *streamPtr = (IOChannelRand *)NULL;
    bool retVal = false;
    char *randType = (char *)NULL;
    char *seed = (char *)NULL;
    unsigned int usedForSeed = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    randType = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_NAME );
    if( !randType )
    {
        ANY_LOG( 0, "Bad infoString argument was passed "
                "to open the \"Rand\" stream!", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto exitLabel;
    }

    if( IOCHANNEL_MODEIS_DEFINED( self->mode ))
    {
        if( !IOCHANNEL_MODEIS_R_ONLY( self->mode ))
        {
            ANY_LOG( 5, "IOChannelRand_open() accepts 'IOCHANNEL_MODE_R_ONLY' flag only",
                     ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_BFLGS );
            goto exitLabel;
        }
    }
    else
    {
        self->mode = IOCHANNEL_MODE_R_ONLY;
    }

    seed = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_KEY );
    if( !seed )
    {
        srand((unsigned int)time( NULL ));
    }
    else
    {
        usedForSeed = IOChannelReferenceValue_getUInt( referenceVector, IOCHANNELREFERENCEVALUE_KEY );

        srand( usedForSeed );
    }

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( Any_strncasecmp( randType, "Integers", Any_strlen( "Integers" )) == 0 )
    {
        streamPtr->type = IOCHANNELRANDTYPE_INTEGERS;
    }
    else if( Any_strncasecmp( randType, "Floats", Any_strlen( "Floats" )) == 0 )
    {
        streamPtr->type = IOCHANNELRANDTYPE_FLOATS;
    }
    else if( Any_strncasecmp( randType, "Chars", Any_strlen( "Chars" )) == 0 )
    {
        streamPtr->type = IOCHANNELRANDTYPE_CHARS;
    }
    else if( Any_strncasecmp( randType, "Printables", Any_strlen( "Printables" )) == 0 )
    {
        streamPtr->type = IOCHANNELRANDTYPE_PRINTABLES;
    }
    else
    {
        ANY_LOG( 0, "Bad type was Choosen to Generate Values! Allowed are: "
                         "Integers, Floats, Chars, Printables.",
                 ANY_LOG_INFO );
        goto exitLabel;
    }

    retVal = true;

    exitLabel:;
    return retVal;
}


static long IOChannelRand_read( IOChannel *self, void *buffer, long size )
{
    IOChannelRand *streamPtr = (IOChannelRand *)NULL;
    char *ptr = (char *)NULL;
    char *aux = (char *)NULL;
    long retVal = size;
    long toReadBytes = size;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ptr = (char *)buffer;
    ANY_REQUIRE( ptr );

    aux = (char *)streamPtr->buffer;
    ANY_REQUIRE( aux );


    if( streamPtr->index == 0 || streamPtr->index == streamPtr->size )
    {
        IOChannelRand_fillBuffer( self );
    }

    while( toReadBytes > 0 )
    {
        if(( toReadBytes - ( streamPtr->size - streamPtr->index )) >= 0 )
        {
            Any_memcpy( ptr, ( aux + streamPtr->index ), ( streamPtr->size - streamPtr->index ));
            ptr += ( streamPtr->size - streamPtr->index );
            toReadBytes -= ( streamPtr->size - streamPtr->index );
            IOChannelRand_fillBuffer( self );
        }
        else
        {
            Any_memcpy( ptr, ( aux + streamPtr->index ), toReadBytes );
            streamPtr->index += toReadBytes;
            break;
        }
    }
    return retVal;
}


static long IOChannelRand_write( IOChannel *self, const void *buffer, long size )
{
    ANY_LOG( 0, "IOChannelRand_write() not supported (read-only stream)",
             ANY_LOG_ERROR );

    return -1;
}


static long IOChannelRand_flush( IOChannel *self )
{
    ANY_LOG( 0, "IOChannelRand_flush() not supported (read-only stream)",
             ANY_LOG_ERROR );

    return 0;
}


static long long IOChannelRand_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    ANY_REQUIRE( self );
    IOChannel_valid( self );

    return 0;
}


static bool IOChannelRand_close( IOChannel *self )
{
    ANY_REQUIRE( self );
    IOChannel_valid( self );

    return true;
}


static void *IOChannelRand_getProperty( IOChannel *self, const char *propertyName )
{
    ANY_LOG( 5, "No properties are defined for [Rand://] stream",
             ANY_LOG_WARNING );

    return (void *)NULL;
}


static bool IOChannelRand_setProperty( IOChannel *self, const char *propertyName,
                                       void *property )
{
    ANY_LOG( 5, "No properties are defined for [Rand://] stream",
             ANY_LOG_WARNING );

    return false;
}


static void IOChannelRand_clear( IOChannel *self )
{
    ANY_REQUIRE( self );
    IOChannel_valid( self );
}


static void IOChannelRand_delete( IOChannel *self )
{
    IOChannelRand *streamPtr = (IOChannelRand *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


static void IOChannelRand_fillBuffer( IOChannel *self )
{
    /* This includes the sign symbol.. */
#define LONGLONGINT_ASCII_SIZE      11

    IOChannelRand *streamPtr = (IOChannelRand *)NULL;
    char *ptr = (char *)NULL;
    int nChar = 0;
    int nTimes = 0;
    int i = 0;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );
    ANY_REQUIRE( streamPtr->type != IOCHANNELRANDTYPE_NONE );

    ptr = (char *)streamPtr->buffer;
    ANY_REQUIRE( ptr );

    Any_memset( ptr, ' ', IOCHANNELRAND_BUFFER_SIZE );

    /* nTimes is the number of tokens per buffer */
    nTimes = IOCHANNELRAND_BUFFER_SIZE / LONGLONGINT_ASCII_SIZE;

    if( streamPtr->type == IOCHANNELRANDTYPE_INTEGERS )
    {
        /* for integers */
        for( i = 0; i < nTimes; i++ )
        {
            nChar = Any_snprintf( ptr, IOCHANNELRAND_BUFFER_SIZE, "%lld ", (long long int)rand());
            ptr += nChar;

        }
    }
    else if( streamPtr->type == IOCHANNELRANDTYPE_FLOATS )
    {
        /* for floats */
        for( i = 0; i < nTimes; i++ )
        {
            nChar = Any_snprintf( ptr, IOCHANNELRAND_BUFFER_SIZE, "%lld ", (long long int)rand());
            *( ptr + ( rand() % nChar )) = '.';
            ptr += nChar;
        }
    }
    else if( streamPtr->type == IOCHANNELRANDTYPE_CHARS )
    {
        /* for chars */
        for( i = 0; i < nTimes; i++ )
        {
            nChar = Any_snprintf( ptr, IOCHANNELRAND_BUFFER_SIZE, "%lld ", ((long long int)rand() % 255 ));
            ptr += nChar;
        }
    }
    else if( streamPtr->type == IOCHANNELRANDTYPE_PRINTABLES )
    {
        /* for printable chars */
        int randomNumber = 0;
        for( i = 0; i < nTimes; i++ )
        {
            randomNumber = 33 + ((int)rand() % ( 126 - 33 ));
            nChar = Any_snprintf( ptr, IOCHANNELRAND_BUFFER_SIZE, "%c ", (char)randomNumber );
            ptr += nChar;
        }
    }
    else
    {
        ANY_REQUIRE_MSG( NULL, "No type selected to generate random values!" );
    }

    streamPtr->index = 0;

#undef LONGLONGINT_ASCII_SIZE
}


/* EOF */
