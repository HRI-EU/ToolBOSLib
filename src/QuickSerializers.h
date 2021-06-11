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


/*!
 * \page ToolBOS_HowTo_Serialize (De-)Serialize
 *
 * The aim of the Quick Serializers is to make the opening and usage of
 * serialization channels simple, and to reduce the amount of code.
 *
 * The openForReading/Writing() functions return a Serialize object which
 * is passed to the serialization function of your used datatype. This
 * serialization function will do the job with the help of the passed
 * Serialize instance (the "serializer" or "deserializer").
 *
 * \li \subpage QuickSerializers_File
 * \li \subpage QuickSerializers_Memory
 * \li \subpage QuickSerializers_StdOut
 * \li \subpage QuickSerializers_CalcSize
 * \li \subpage QuickSerializers_RTBOS
 * \li \subpage ToolBOS_HowTo_SerializeToPython
 *
 * \note Each Quick Serializer internally operates in the IOChannel
 *       stream-mode SERIALIZE_STREAMMODE_NORMAL. If this is inappropriate
 *       for your case, you can change this directly on the Serialize
 *       instance which you get from the openForReading() and openForWriting()
 *       functions.
 *
 *
 * \page QuickSerializers_CalcSize Calculate the serialized size
 *
 * Aim of the CalcSizeSerializer library is to provide an easy and quick way to open and
 * to use a serialization stream set in CalcSize mode. The users can use it to know the
 * size of the serialized objects.
 * The library is, basically, a wrapper around the Serialize and IOChannel libraries.
 *
 * <h3>Example:</h3>
 *
 * \code
 * BlockF32* block0 = (BlockF32*)NULL;
 * BaseF32* array = (BaseF32*)NULL;
 *
 * CalcSizeSerializer* serializer = (CalcSizeSerializer*)NULL;
 * Serialize* stream = (Serialize*)NULL;
 *
 * [ BlockF32 allocation and initialization ]
 *
 * serializer = CalcSizeSerializer_new();
 * ANY_REQUIRE_MSG( serializer, "Impossible to allocate a new CalcSizeSerializer" );
 * result = CalcSizeSerializer_init( serializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the CalcSizeSerializer" );
 *
 * stream = CalcSizeSerializer_open( serializer, "Binary" );
 *
 * BlockF32_serialize( block0, "MyBlock", stream );
 *
 * ANY_LOG( 0, "HEADER SIZE = %ld", ANY_LOG_INFO, CalcSizeSerializer_getHeaderSize( serializer ) );
 * ANY_LOG( 0, "PAYLOAD SIZE = %ld", ANY_LOG_INFO, CalcSizeSerializer_getPayloadSize( serializer ) );
 * ANY_LOG( 0, "TOTAL SIZE = %ld", ANY_LOG_INFO, CalcSizeSerializer_getTotalSize( serializer ) );
 *
 * CalcSizeSerializer_close( serializer );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 * CalcSizeSerializer_clear( serializer );
 * CalcSizeSerializer_delete( serializer );
 * \endcode
 *
 *
 * \page QuickSerializers_File Serializing from/to files
 *
 * Aim of the FileSerializer library is to provide an easy and quick way to open and
 * to use a serialization stream for a file.
 * The library is, basically, a wrapper around the Serialize and IOChannel libraries.
 *
 * <h3>Example (Reading):</h3>
 *
 * \code
 * BlockF32* block = (BlockF32*)NULL;
 *
 * FileSerializer* readSerializer = (FileSerializer*)NULL;
 * Serialize* readStream = (Serialize*)NULL;
 *
 * readSerializer = FileSerializer_new();
 * ANY_REQUIRE_MSG( readSerializer, "Impossible allocate a new FileSerializer" );
 * result = FileSerializer_init( readSerializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the FileSerializer" );
 *
 * [ BlockF32 instantiation ]
 *
 * readStream = FileSerializer_openForReading( readSerializer, FILENAME0 );
 * ANY_REQUIRE_MSG( readStream, "Impossible to open the file " FILENAME0 );
 *
 * FileSerializer_setInitMode( readSerializer, true );
 *
 * BlockF32_serialize( block, "MyBlock", readStream );
 *
 * result = FileSerializer_close( readSerializer );
 * ANY_REQUIRE_MSG( result, "Error closing the readSerializer" );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 * FileSerializer_clear( readSerializer );
 * FileSerializer_delete( readSerializer );
 * \endcode
 *
 * <h3>Example (Writing):</h3>
 *
 * \code
 * BlockF32* block = (BlockF32*)NULL;
 *
 * FileSerializer* writeSerializer = (FileSerializer*)NULL;
 * Serialize* writeStream = (Serialize*)NULL;
 *
 * writeSerializer = FileSerializer_new();
 * ANY_REQUIRE_MSG( writeSerializer, "Impossible allocate a new FileSerializer" );
 * result = FileSerializer_init( writeSerializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the FileSerializer" );
 *
 * [ BlockF32 instantiation and initialization ]
 *
 * writeStream = FileSerializer_openForWriting( writeSerializer, FILENAME1, "Ascii" );
 * ANY_REQUIRE_MSG( writeStream, "Impossible to open the file " FILENAME1 );
 *
 * BlockF32_serialize( block, "MyBlock", writeStream );
 *
 * result = FileSerializer_close( writeSerializer );
 * ANY_REQUIRE_MSG( result, "Error closing the writeSerializer" );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 * FileSerializer_clear( writeSerializer );
 * FileSerializer_delete( writeSerializer );
 * \endcode
 *
 *
 * \page QuickSerializers_Memory Serializing from/to memory
 *
 * Aim of the MemorySerializer library is to provide an easy and quick way to open and
 * to use a serialization stream for a block of memory.
 * The library is, basically, a wrapper around the Serialize and IOChannel libraries.
 *
 * <h3>Example (Reading):</h3>
 *
 * \code
 * char buffer[ BUFFER_SIZE ] = { 0, };
 * const size_t bufferSize = BUFFER_SIZE;
 *
 * Base2DSize size = { BLOCK_HEIGHT, BLOCK_WIDTH };
 * BlockF32* block = (BlockF32*)NULL;
 *
 * MemorySerializer* memSerializer = (MemorySerializer*)NULL;
 * Serialize* readStream = (Serialize*)NULL;
 *
 * memSerializer = MemorySerializer_new();
 * ANY_REQUIRE_MSG( memSerializer, "Impossible to allocate a new MemorySerializer" );
 * result = MemorySerializer_init( memSerializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the MemorySerializer" );
 *
 * [ BlockF32 allocation ]
 * [ buffer initialization ]
 *
 * readStream = MemorySerializer_openForReading( memSerializer, buffer, bufferSize );
 * ANY_REQUIRE_MSG( readStream, "Impossible to work with the given block of memory" );
 *
 * MemorySerializer_setInitMode( memSerializer, true );
 *
 * BlockF32_serialize( block, "MyBlock", readStream );
 *
 * result = MemorySerializer_close( memSerializer );
 * ANY_REQUIRE_MSG( result, "Error closing the memSerializer" );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 *  MemorySerializer_clear( memSerializer );
 *  MemorySerializer_delete( memSerializer );
 * \endcode
 *
 * <h3>Example (Writing):</h3>
 *
 * \code
 * char buffer[ BUFFER_SIZE ] = { 0, };
 * const size_t bufferSize = BUFFER_SIZE;
 *
 * Base2DSize size = { BLOCK_HEIGHT, BLOCK_WIDTH };
 * BlockF32* block = (BlockF32*)NULL;
 *
 * MemorySerializer* memSerializer = (MemorySerializer*)NULL;
 * Serialize* writeStream = (Serialize*)NULL;
 *
 * memSerializer = MemorySerializer_new();
 * ANY_REQUIRE_MSG( memSerializer, "Impossible to allocate a new MemorySerializer" );
 * result = MemorySerializer_init( memSerializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the MemorySerializer" );
 *
 * [ BlockF32 allocation and initialization ]
 *
 * writeStream = MemorySerializer_openForWriting( memSerializer, buffer, bufferSize, "Binary" );
 * ANY_REQUIRE_MSG( readStream, "Impossible to work with the given block of memory" );
 *
 * BlockF32_serialize( block, "MyBlock", writeStream );
 *
 * result = MemorySerializer_close( memSerializer );
 * ANY_REQUIRE_MSG( result, "Error closing the memSerializer" );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 * MemorySerializer_clear( memSerializer );
 * MemorySerializer_delete( memSerializer );
 * \endcode
 *
 *
 * \page QuickSerializers_RTBOS Serializing to RTBOS
 *
 * The RTBOSSerializer is used to prepare a channel optimized for best performance
 * for performing serialization to and from an RTBOS connection.
 *
 * <h3>Example:</h3>
 *
 * \code
 * BBDMBaseI32 data = BBDMBaseI32_new();
 * BBDMBaseI32_init( data, 0 );
 *
 * RTBOSSerializer* rtbosSerializer = RTBOSSerializer_new();
 * RTBOSSerializer_init( rtbosSerializer );
 *
 * Serialize* serializer = RTBOSSerializer_openForWriting( rtbosSerializer, "localhost", 2000, "bBDMBaseI32" );
 *
 * BBDMBaseI32_serialize( data, "MyData", serializer );
 *
 * RTBOSSerializer_close( rtbosSerializer );
 * RTBOSSerializer_clear( rtbosSerializer );
 * RTBOSSerializer_delete( rtbosSerializer );
 * \endcode
 *
 *
 * \page QuickSerializers_StdOut Serializing to stdout
 *
 * Aim of the StdOutSerializer library is to provide an easy and quick way to open and
 * to use a serialization stream associated with the standard output.
 * The library is, basically, a wrapper around the Serialize and IOChannel libraries.
 *
 * <h3>Example:</h3>
 *
 * \code
 * Base2DSize size = { BLOCK_HEIGHT, BLOCK_WIDTH };
 * BlockF32* block0 = (BlockF32*)NULL;
 *
 * StdOutSerializer* serializer = (StdOutSerializer*)NULL;
 * Serialize* stream = (Serialize*)NULL;
 *
 * serializer = StdOutSerializer_new();
 * ANY_REQUIRE_MSG( serializer, "Impossible to allocate a new StdOutSerializer" );
 * result = StdOutSerializer_init( serializer );
 * ANY_REQUIRE_MSG( result == 0, "Impossible to initialize the StdOutSerializer" );
 *
 * [ BlockF32 allocation and initialization ]
 *
 * stream = StdOutSerializer_openForWriting( serializer, "Ascii" );
 *
 * BlockF32_serialize( block0, "MyBlock", stream );
 *
 * StdOutSerializer_close( serializer );
 *
 * [ BlockF32 cleaning and deallocation ]
 *
 * StdOutSerializer_clear( serializer );
 * StdOutSerializer_delete( serializer );
 * \endcode
 */


#ifndef QUICKSERIALIZERS_H
#define QUICKSERIALIZERS_H


#include <string.h>

#include <Any.h>
#include <IOChannel.h>
#include <Serialize.h>


#if defined(__cplusplus)
extern "C" {
#endif


#define SERIALIZER_DEFAULT_MODE SERIALIZE_STREAMMODE_NORMAL

#define FILESERIALIZER_MODE_APPEND IOCHANNEL_MODE_APPEND
#define FILESERIALIZER_MODE_CREAT IOCHANNEL_MODE_CREAT
#define FILESERIALIZER_MODE_TRUNC IOCHANNEL_MODE_TRUNC


typedef struct Serializer
{
    unsigned long valid;
    IOChannel *channel;
    Serialize *serialize;
    void *serializerData;
}
Serializer;


typedef Serializer CalcSizeSerializer;

typedef Serializer FileSerializer;

typedef Serializer MemorySerializer;

typedef Serializer RTBOSSerializer;

typedef Serializer StdOutSerializer;


Serializer *Serializer_new( void );

int Serializer_init( Serializer *self );

Serialize *Serializer_getSerializePtr( Serializer *self );

IOChannel *Serializer_getIOChannelPtr( Serializer *self );

bool Serializer_isOpen( Serializer *self );

bool Serializer_close( Serializer *self );

void Serializer_setInitMode( Serializer *self, bool status );

bool Serializer_isErrorOccurred( Serializer *self );

void Serializer_clear( Serializer *self );

void Serializer_delete( Serializer *self );

CalcSizeSerializer *CalcSizeSerializer_new( void );

int CalcSizeSerializer_init( CalcSizeSerializer *self );

Serialize *CalcSizeSerializer_open( CalcSizeSerializer *self, const char *format );

bool CalcSizeSerializer_close( CalcSizeSerializer *self );

Serialize *CalcSizeSerializer_getSerializePtr( CalcSizeSerializer *self );

IOChannel *CalcSizeSerializer_getIOChannelPtr( CalcSizeSerializer *self );

void CalcSizeSerializer_setInitMode( CalcSizeSerializer *self, bool status );

bool CalcSizeSerializer_isErrorOccurred( CalcSizeSerializer *self );

long CalcSizeSerializer_getHeaderSize( CalcSizeSerializer *self );

long CalcSizeSerializer_getPayloadSize( CalcSizeSerializer *self );

long CalcSizeSerializer_getTotalSize( CalcSizeSerializer *self );

void CalcSizeSerializer_clear( CalcSizeSerializer *self );

void CalcSizeSerializer_delete( CalcSizeSerializer *self );

FileSerializer *FileSerializer_new( void );

int FileSerializer_init( FileSerializer *self );

Serialize *FileSerializer_getSerializePtr( FileSerializer *self );

IOChannel *FileSerializer_getIOChannelPtr( FileSerializer *self );

Serialize *FileSerializer_openForWriting( FileSerializer *self, const char *filename,
                                          const char *format );

Serialize *FileSerializer_openForReading( FileSerializer *self, const char *filename );

bool FileSerializer_close( FileSerializer *self );

void FileSerializer_setFlagsForWriting( FileSerializer *self, int flags );

void FileSerializer_setInitMode( FileSerializer *self, bool status );

bool FileSerializer_isErrorOccurred( FileSerializer *self );

void FileSerializer_clear( FileSerializer *self );

void FileSerializer_delete( FileSerializer *self );

MemorySerializer *MemorySerializer_new( void );

int MemorySerializer_init( MemorySerializer *self );

Serialize *MemorySerializer_getSerializePtr( MemorySerializer *self );

IOChannel *MemorySerializer_getIOChannelPtr( MemorySerializer *self );

Serialize *MemorySerializer_openForWriting( MemorySerializer *self, void *memory,
                                            const size_t size, const char *format );

Serialize *MemorySerializer_openForReading( MemorySerializer *self, const void *memory,
                                            const size_t size );

bool MemorySerializer_close( MemorySerializer *self );

void MemorySerializer_setInitMode( MemorySerializer *self, bool status );

bool MemorySerializer_isErrorOccurred( MemorySerializer *self );

void MemorySerializer_clear( MemorySerializer *self );

void MemorySerializer_delete( MemorySerializer *self );

RTBOSSerializer *RTBOSSerializer_new( void );

int RTBOSSerializer_init( RTBOSSerializer *self );

Serialize *RTBOSSerializer_openForReading( RTBOSSerializer *self,
                                           const char *host,
                                           int port,
                                           const char *name );

Serialize *RTBOSSerializer_openForWriting( RTBOSSerializer *self,
                                           const char *host,
                                           int port,
                                           const char *name );

Serialize *RTBOSSerializer_getSerializePtr( RTBOSSerializer *self );

IOChannel *RTBOSSerializer_getIOChannelPtr( RTBOSSerializer *self );

bool RTBOSSerializer_close( RTBOSSerializer *self );

bool RTBOSSerializer_isErrorOccurred( RTBOSSerializer *self );

void RTBOSSerializer_clear( RTBOSSerializer *self );

void RTBOSSerializer_delete( RTBOSSerializer *self );

StdOutSerializer *StdOutSerializer_new( void );

int StdOutSerializer_init( StdOutSerializer *self );

Serialize *StdOutSerializer_getSerializePtr( StdOutSerializer *self );

IOChannel *StdOutSerializer_getIOChannelPtr( StdOutSerializer *self );

Serialize *StdOutSerializer_openForWriting( StdOutSerializer *self, const char *format );

bool StdOutSerializer_close( StdOutSerializer *self );

bool StdOutSerializer_isErrorOccurred( StdOutSerializer *self );

void StdOutSerializer_clear( StdOutSerializer *self );

void StdOutSerializer_delete( StdOutSerializer *self );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
