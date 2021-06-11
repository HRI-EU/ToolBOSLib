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


#include <stdio.h>
#include <stdlib.h>

#include <Any.h>
#include <Base.h>
#include <BPL_Base.h>
#include <BPL_Serialize.h>
#include <BBDMBaseI32.h>
#include <BBDMMemI8.h>
#include <BBDMBlockF32.h>
#include <IOChannel.h>
#include <RTTimer.h>
#include <Serialize.h>


#define EXAMPLE_BLOCK_WIDTH  (400)
#define EXAMPLE_BLOCK_HEIGHT (300)
#define EXAMPLE_BUFFERLEN    (100)

#define ONE_HUNDRED          ( 100 )
#define ONE_THOUSAND         ( 1000 )
#define TEN_THOUSAND         ( 10000 )
#define HUNDRED_THOUSAND     ( 100000 )

#define EXAMPLE_STREAM       ( "Null://" )


#define TIMING_START( typeName, cycles )                                    \
                                                                            \
  ANY_LOG( 3, "", ANY_LOG_INFO );                                           \
  ANY_LOG( 3, "timing %s serialization...", ANY_LOG_INFO, typeName );       \
                                                                            \
  RTTimer_reset( timer );                                                   \
  RTTimer_start( timer );                                                   \
                                                                            \
  for( i = 0; i < cycles; i++ )


#define TIMING_STOP( typeName, cycles )                                     \
                                                                            \
  RTTimer_stop( timer );                                                    \
                                                                            \
  elapsed = RTTimer_getElapsed( timer );                                    \
  avg     = elapsed / cycles;                                               \
  ANY_REQUIRE_MSG( avg != 0, "must not use 0 as divisor" );                 \
  fps     = 1000000000 / avg;                                               \
                                                                            \
  ANY_LOG( 3, "%s binary serialization: %ld usec (%ld Hz)",                 \
           ANY_LOG_INFO, typeName, avg / 1000, fps );                       \
                                                                            \
  ANY_LOG( 3, "", ANY_LOG_INFO );


int main( void )
{
    RTTimer *timer = (RTTimer *)NULL;
    unsigned long elapsed = 0;
    unsigned long avg = 0;
    unsigned long fps = 0;
    unsigned int i = 0;
    BaseI32 myBaseI32 = 0;
    MemI8 *myMemI8 = (MemI8 *)NULL;
    BlockF32 *myBlockF32 = (BlockF32 *)NULL;
    Base2DSize blockSize;
    BBDMBaseI32 *myBBDMBaseI32 = (BBDMBaseI32 *)NULL;
    BBDMMemI8 *myBBDMMemI8 = (BBDMMemI8 *)NULL;
    BBDMBlockF32 *myBBDMBlockF32 = (BBDMBlockF32 *)NULL;
    IOChannel *channel = (IOChannel *)NULL;
    Serialize *serializer = (Serialize *)NULL;
    char *verbose = (char *)NULL;

    verbose = getenv((char *)"VERBOSE" );
    if( verbose != NULL && Any_strcmp( verbose, (char *)"TRUE" ) == 0 )
    {
        Any_setDebugLevel( 10 );
    }
    else
    {
        Any_setDebugLevel( 1 );
    }


    ANY_LOG( 3, "", ANY_LOG_INFO );
    ANY_LOG( 3, "SERIALIZATION TIMING", ANY_LOG_INFO );
    ANY_LOG( 3, "====================", ANY_LOG_INFO );
    ANY_LOG( 3, "", ANY_LOG_INFO );

    timer = RTTimer_new();
    RTTimer_init( timer );


    myBBDMBaseI32 = BBDMBaseI32_new();
    BBDMBaseI32_initFromString( myBBDMBaseI32, "" );

    myMemI8 = MemI8_new();
    MemI8_init( myMemI8, EXAMPLE_BUFFERLEN);

    myBBDMMemI8 = BBDMMemI8_new();
    BBDMMemI8_initFromString( myBBDMMemI8, "length=10" );

    blockSize.width = EXAMPLE_BLOCK_WIDTH;
    blockSize.height = EXAMPLE_BLOCK_HEIGHT;

    myBlockF32 = BlockF32_new();
    BlockF32_init( myBlockF32, &blockSize );

    myBBDMBlockF32 = BBDMBlockF32_new();
    BBDMBlockF32_initFromString( myBBDMBlockF32, "width=400 height=300" );


    channel = IOChannel_new();
    IOChannel_init( channel );

    //   IOChannel_open( channel, "Null://",
    IOChannel_open( channel, "File:///tmp/output.ser",
                    IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                    IOCHANNEL_PERMISSIONS_ALL);

    serializer = Serialize_new();
    Serialize_init( serializer, channel, SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );
    Serialize_setFormat( serializer, "Binary", "" );


    /*-----------------------------------------------------------------------*/

    TIMING_START( "BaseI32", TEN_THOUSAND )
    {
        BaseI32_serialize( &myBaseI32, "myBaseI32", serializer );
    }
    TIMING_STOP( "BaseI32", TEN_THOUSAND )


    TIMING_START( "BBDMBaseI32", TEN_THOUSAND )
    {
        BBDMBaseI32_serialize( myBBDMBaseI32, "myBBDMBaseI32", serializer );
    }
    TIMING_STOP( "BBDMBaseI32", TEN_THOUSAND )


    TIMING_START( "MemI8", TEN_THOUSAND )
    {
        MemI8_serialize( myMemI8, "myMemI8", serializer );
    }
    TIMING_STOP( "MemI8", TEN_THOUSAND )


    TIMING_START( "BBDMMemI8", TEN_THOUSAND )
    {
        BBDMMemI8_serialize( myBBDMMemI8, "myBBDMMemI8", serializer );
    }
    TIMING_STOP( "BBDMMemI8", TEN_THOUSAND )


    /* Note: There seems to be a bug in Serialize / IOChannel that caused
    *       an overflow leading to any ANY_REQUIRE failure when serializing
    *       more than 4472 blocks of the given size. */

    TIMING_START( "BlockF32", ONE_HUNDRED )
    {
        BlockF32_serialize( myBlockF32, "myBlockF32", serializer );
    }
    TIMING_STOP( "BlockF32", ONE_HUNDRED )


    TIMING_START( "BBDMBlockF32", ONE_HUNDRED )
    {
        BBDMBlockF32_serialize( myBBDMBlockF32, "myBBDMBlockF32", serializer );
    }
    TIMING_STOP( "BBDMBlockF32", ONE_HUNDRED )

    /*-----------------------------------------------------------------------*/


    RTTimer_clear( timer );
    RTTimer_delete( timer );

    Serialize_clear( serializer );
    Serialize_delete( serializer );

    IOChannel_close( channel );
    IOChannel_clear( channel );
    IOChannel_delete( channel );

    BBDMBlockF32_clear( myBBDMBlockF32 );
    BBDMBlockF32_delete( myBBDMBlockF32 );

    BlockF32_clear( myBlockF32 );
    BlockF32_delete( myBlockF32 );

    BBDMBaseI32_clear( myBBDMBaseI32 );
    BBDMBaseI32_delete( myBBDMBaseI32 );

    MemI8_clear( myMemI8 );
    MemI8_delete( myMemI8 );

    BBDMMemI8_clear( myBBDMMemI8 );
    BBDMMemI8_delete( myBBDMMemI8 );


    return EXIT_SUCCESS;
}


/* EOF */
