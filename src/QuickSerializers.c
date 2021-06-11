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


#include <string.h>

#include <QuickSerializers.h>


#define SERIALIZER_VALID    (0xcec0110c)
#define SERIALIZER_INVALID   (0xeffecca3)

#define CALCSIZESERIALIZER_VALID    (0x20cdf605)
#define CALCSIZESERIALIZER_INVALID   (0x5fada7b0)

#define FILESERIALIZER_VALID    (0xdcba6908)
#define FILESERIALIZER_INVALID   (0x6c950f50)

#define FILESERIALIZER_DEFAULT_ACCESSFLAFS IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC
#define FILESERIALIZER_DEFAULT_PERMISSIONS IOCHANNEL_PERMISSIONS_ALL

#define MEMORYSERIALIZER_VALID    (0xd2491dfb)
#define MEMORYSERIALIZER_INVALID   (0x3817d3d4)

#define STDOUTSERIALIZER_VALID    (0x42c2ac27)
#define STDOUTSERIALIZER_INVALID   (0xb2026f5c)

#define RTBOSSERIALIZER_VALID     (0xd2c74c03)
#define RTBOSSERIALIZER_INVALID   (0x5faacfed)


static Serialize *RTBOSSerializer_internalOpen( RTBOSSerializer *self,
                                                const char *host,
                                                int port,
                                                const char *data );


Serializer *Serializer_new( void )
{
    Serializer *self = (Serializer *)NULL;

    self = ANY_TALLOC( Serializer );
    ANY_REQUIRE( self );

    return self;
}


int Serializer_init( Serializer *self )
{
    int result = -1;

    ANY_REQUIRE( self );

    Any_bzero((void *)self, sizeof( Serializer ));

    self->channel = IOChannel_new();
    if( self->channel == (IOChannel *)NULL )
    {
        ANY_LOG( 0, "Impossible to allocate a new IOChannel object", ANY_LOG_ERROR );
        goto exit_0;
    }

    if( !IOChannel_init( self->channel ))
    {
        ANY_LOG( 0, "Impossible to initialize the IOChannel object", ANY_LOG_ERROR );
        goto exit_1;
    }

    self->serialize = Serialize_new();
    if( self->serialize == (Serialize *)NULL )
    {
        ANY_LOG( 0, "Impossible to allocate a new Serialize object", ANY_LOG_ERROR );
        goto exit_2;
    }

    if( !Serialize_init( self->serialize, self->channel, SERIALIZER_DEFAULT_MODE ))
    {
        ANY_LOG( 0, "Impossible to initialize the Serialize instance", ANY_LOG_ERROR );
        goto exit_3;
    }

    result = 0;
    self->valid = SERIALIZER_VALID;

    return result;

    exit_3:
    Serialize_delete( self->serialize );
    exit_2:
    IOChannel_clear( self->channel );
    exit_1:
    IOChannel_delete( self->channel );
    exit_0:
    return result;
}


Serialize *Serializer_getSerializePtr( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    return self->serialize;
}


IOChannel *Serializer_getIOChannelPtr( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    return self->channel;
}


bool Serializer_isOpen( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    return self->channel && IOChannel_isOpen( self->channel );
}

bool Serializer_close( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    return IOChannel_close( self->channel );
}


void Serializer_setInitMode( Serializer *self, bool status )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    Serialize_setInitMode( self->serialize, status );
}


bool Serializer_isErrorOccurred( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    return Serialize_isErrorOccurred( self->serialize );
}


void Serializer_clear( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZER_VALID);

    Serialize_clear( self->serialize );
    Serialize_delete( self->serialize );
    IOChannel_clear( self->channel );
    IOChannel_delete( self->channel );

    /* Clear the structure */
    Any_bzero((void *)self, sizeof( Serializer ));
    self->valid = SERIALIZER_INVALID;
}


void Serializer_delete( Serializer *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


CalcSizeSerializer *CalcSizeSerializer_new( void )
{
    return Serializer_new();
}


int CalcSizeSerializer_init( CalcSizeSerializer *self )
{
    int result = -1;

    ANY_REQUIRE( self );

    Any_bzero((void *)self, sizeof( CalcSizeSerializer ));

    self->serialize = Serialize_new();
    if( self->serialize == (Serialize *)NULL )
    {
        ANY_LOG( 0, "Impossible to allocate a new Serialize object", ANY_LOG_ERROR );
        goto exit_0;
    }

    if( !Serialize_init( self->serialize, NULL, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_CALC ))
    {
        ANY_LOG( 0, "Impossible to initialize the Serialize instance", ANY_LOG_ERROR );
        goto exit_1;
    }

    result = 0;
    self->valid = CALCSIZESERIALIZER_VALID;

    return result;

    exit_1:
    Serialize_delete( self->serialize );
    exit_0:
    return result;
}


Serialize *CalcSizeSerializer_open( CalcSizeSerializer *self, const char *format )
{
    ANY_REQUIRE( self );
    Serialize_setFormat( self->serialize, format, NULL );

    return self->serialize;
}


bool CalcSizeSerializer_close( CalcSizeSerializer *self )
{
    ANY_REQUIRE( self );

    return true;
}


Serialize *CalcSizeSerializer_getSerializePtr( CalcSizeSerializer *self )
{
    return Serializer_getSerializePtr( self );
}


IOChannel *CalcSizeSerializer_getIOChannelPtr( CalcSizeSerializer *self )
{
    return Serializer_getIOChannelPtr( self );
}


long CalcSizeSerializer_getHeaderSize( CalcSizeSerializer *self )
{
    return Serialize_getHeaderSize( self->serialize );
}


long CalcSizeSerializer_getPayloadSize( CalcSizeSerializer *self )
{
    return Serialize_getPayloadSize( self->serialize );
}


long CalcSizeSerializer_getTotalSize( CalcSizeSerializer *self )
{
    return Serialize_getTotalSize( self->serialize );
}


void CalcSizeSerializer_setInitMode( CalcSizeSerializer *self, bool status )
{
    Serializer_setInitMode( self, status );
}


bool CalcSizeSerializer_isErrorOccurred( CalcSizeSerializer *self )
{
    return Serializer_isErrorOccurred( self );
}


void CalcSizeSerializer_clear( CalcSizeSerializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == CALCSIZESERIALIZER_VALID);

    Serialize_clear( self->serialize );

    /* Clear the structure */
    Any_bzero((void *)self, sizeof( CalcSizeSerializer ));

    self->valid = CALCSIZESERIALIZER_INVALID;
}


void CalcSizeSerializer_delete( CalcSizeSerializer *self )
{
    Serializer_delete( self );
}


typedef struct FileSerializerData
{
    int accessFlags;          /* file access flag for writing mode */
}
FileSerializerData;


FileSerializer *FileSerializer_new( void )
{
    return Serializer_new();
}


int FileSerializer_init( FileSerializer *self )
{
    int result = -1;
    result = Serializer_init( self );
    if( result == -1 )
    {
        goto exit_0;
    }

    self->serializerData = ANY_TALLOC( FileSerializerData );
    if( self->serializerData == (void *)NULL )
    {
        ANY_LOG( 0, "Impossible to allocate a new FileSerializerData", ANY_LOG_ERROR );
        goto exit_1;
    }

    /*Setting default file access flag for writing mode */
    ((FileSerializerData *)self->serializerData )->accessFlags =
    IOCHANNEL_MODE_W_ONLY | FILESERIALIZER_DEFAULT_ACCESSFLAFS;

    result = 0;
    return result;

    exit_1:
    Serializer_clear( self );
    exit_0:
    return result;
}


Serialize *FileSerializer_getSerializePtr( FileSerializer *self )
{
    return Serializer_getSerializePtr( self );
}


IOChannel *FileSerializer_getIOChannelPtr( FileSerializer *self )
{
    return Serializer_getIOChannelPtr( self );
}


Serialize *FileSerializer_openForWriting( FileSerializer *self, const char *filename,
                                          const char *format )
{
    Serialize *result = (Serialize *)NULL;
    char initString[IOCHANNEL_INFOSTRING_MAXLEN] = { 0, };

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );
    ANY_REQUIRE( filename );
    ANY_REQUIRE( format );

    if( Any_strncmp( "File://", filename, 7 ) == 0 )
    {
        ANY_LOG( 0, "Please remove the 'File://' from the URL because it isn't required by this library",
                 ANY_LOG_ERROR );
        goto exit_0;
    }

    Any_snprintf( initString, IOCHANNEL_INFOSTRING_MAXLEN, "File://%s", filename );

    if( !IOChannel_open( self->channel, initString, ((FileSerializerData *)self->serializerData )->accessFlags,
                         FILESERIALIZER_DEFAULT_PERMISSIONS ))
    {
        ANY_LOG( 0, "Impossible to open the specified file %s", ANY_LOG_ERROR, filename );
        goto exit_0;
    }

    Serialize_setFormat( self->serialize, format, NULL );
    Serialize_setMode( self->serialize, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_WRITE );

    result = self->serialize;

    exit_0:
    return result;
}


Serialize *FileSerializer_openForReading( FileSerializer *self, const char *filename )
{
    Serialize *result = (Serialize *)NULL;
    char initString[IOCHANNEL_INFOSTRING_MAXLEN] = { 0, };

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );
    ANY_REQUIRE( filename );

    if( Any_strncmp( "File://", filename, 7 ) == 0 )
    {
        ANY_LOG( 0, "Please remove the 'File://' from the URL because it isn't required by this library",
                 ANY_LOG_ERROR );
        goto exit_0;
    }

    Any_snprintf( initString, IOCHANNEL_INFOSTRING_MAXLEN, "File://%s", filename );

    if( !IOChannel_open( self->channel, initString, IOCHANNEL_MODE_R_ONLY, FILESERIALIZER_DEFAULT_PERMISSIONS ))
    {
        ANY_LOG( 0, "Impossible to open the specified file %s", ANY_LOG_ERROR, filename );
        goto exit_0;
    }

    Serialize_setMode( self->serialize, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_READ );

    result = self->serialize;

    exit_0:
    return result;
}


bool FileSerializer_close( FileSerializer *self )
{
    return Serializer_close( self );
}


void FileSerializer_setInitMode( FileSerializer *self, bool status )
{
    Serializer_setInitMode( self, status );
}


void FileSerializer_setFlagsForWriting( FileSerializer *self, int flags )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    ((FileSerializerData *)self->serializerData )->accessFlags = IOCHANNEL_MODE_W_ONLY | flags;
}


bool FileSerializer_isErrorOccurred( FileSerializer *self )
{
    return Serializer_isErrorOccurred( self );
}


void FileSerializer_clear( FileSerializer *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    ANY_FREE((FileSerializerData *)self->serializerData );
    Serializer_clear( self );
}


void FileSerializer_delete( FileSerializer *self )
{
    Serializer_delete( self );
}


MemorySerializer *MemorySerializer_new( void )
{
    return Serializer_new();
}


int MemorySerializer_init( MemorySerializer *self )
{
    return Serializer_init( self );
}


Serialize *MemorySerializer_getSerializePtr( MemorySerializer *self )
{
    return Serializer_getSerializePtr( self );
}


IOChannel *MemorySerializer_getIOChannelPtr( MemorySerializer *self )
{
    return Serializer_getIOChannelPtr( self );
}


Serialize *MemorySerializer_openForWriting( MemorySerializer *self, void *memory,
                                            const size_t size, const char *format )
{
    Serialize *result = (Serialize *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    ANY_REQUIRE_MSG( memory, "The memory pointer cannot be NULL" );
    ANY_REQUIRE_MSG( size > 0, "The size must be greater then zero" );

    if( !IOChannel_open( self->channel, "Mem://", IOCHANNEL_MODE_W_ONLY, IOCHANNEL_PERMISSIONS_ALL, memory, size ))
    {
        ANY_LOG( 0, "Impossible to open the specified block of memory", ANY_LOG_ERROR );
        goto exit_0;
    }

    Serialize_setFormat( self->serialize, format, NULL );
    Serialize_setMode( self->serialize, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_WRITE );

    result = self->serialize;

    exit_0:
    return result;
}


Serialize *MemorySerializer_openForReading( MemorySerializer *self, const void *memory,
                                            const size_t size )
{
    Serialize *result = (Serialize *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    ANY_REQUIRE_MSG( memory, "The memory pointer cannot be NULL" );
    ANY_REQUIRE_MSG( size > 0, "The size must be greater then zero" );


    if( !IOChannel_open( self->channel, "Mem://", IOCHANNEL_MODE_R_ONLY, IOCHANNEL_PERMISSIONS_ALL, memory, size  ))
    {
        ANY_LOG( 0, "Impossible to open the specified block of memory", ANY_LOG_ERROR );
        goto exit_0;
    }

    Serialize_setMode( self->serialize, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_READ );

    result = self->serialize;

    exit_0:
    return result;
}


bool MemorySerializer_close( MemorySerializer *self )
{
    return Serializer_close( self );
}


void MemorySerializer_setInitMode( MemorySerializer *self, bool status )
{
    Serializer_setInitMode( self, status );
}


bool MemorySerializer_isErrorOccurred( MemorySerializer *self )
{
    return Serializer_isErrorOccurred( self );
}


void MemorySerializer_clear( MemorySerializer *self )
{
    Serializer_clear( self );
}


void MemorySerializer_delete( MemorySerializer *self )
{
    Serializer_delete( self );
}


StdOutSerializer *StdOutSerializer_new( void )
{
    return Serializer_new();
}


int StdOutSerializer_init( StdOutSerializer *self )
{
    return Serializer_init( self );
}


Serialize *StdOutSerializer_getSerializePtr( StdOutSerializer *self )
{
    return Serializer_getSerializePtr( self );
}


IOChannel *StdOutSerializer_getIOChannelPtr( StdOutSerializer *self )
{
    return Serializer_getIOChannelPtr( self );
}


Serialize *StdOutSerializer_openForWriting( StdOutSerializer *self, const char *format )
{
    Serialize *result = (Serialize *)NULL;

    char initString[IOCHANNEL_INFOSTRING_MAXLEN] = { 0, };

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    Any_strncpy( initString,
                 "stream = StdOut mode = IOCHANNEL_MODE_W_ONLY",
                 IOCHANNEL_INFOSTRING_MAXLEN );

    if( !IOChannel_openFromString( self->channel, initString ))
    {
        ANY_LOG( 0, "Impossible to open the stream on the standard output", ANY_LOG_ERROR );
        goto exit_0;
    }

    Serialize_setFormat( self->serialize, format, NULL );
    Serialize_setMode( self->serialize, SERIALIZER_DEFAULT_MODE | SERIALIZE_MODE_WRITE );

    result = self->serialize;

    exit_0:
    return result;
}


bool StdOutSerializer_close( StdOutSerializer *self )
{
    return Serializer_close( self );
}


bool StdOutSerializer_isErrorOccurred( StdOutSerializer *self )
{
    return Serializer_isErrorOccurred( self );
}


void StdOutSerializer_clear( StdOutSerializer *self )
{
    Serializer_clear( self );
}


void StdOutSerializer_delete( StdOutSerializer *self )
{
    Serializer_delete( self );
}


RTBOSSerializer *RTBOSSerializer_new( void )
{
    return Serializer_new();
}


int RTBOSSerializer_init( RTBOSSerializer *self )
{
    return Serializer_init( self );
}


Serialize *RTBOSSerializer_openForReading( RTBOSSerializer *self,
                                           const char *host,
                                           int port,
                                           const char *data )
{
    Serialize *result = RTBOSSerializer_internalOpen( self, host, port, data );

    if( result )
    {
        Serialize_setMode( result, SERIALIZE_STREAMMODE_FLUSH | SERIALIZE_MODE_READ );
    }

    return result;
}


Serialize *RTBOSSerializer_openForWriting( RTBOSSerializer *self,
                                           const char *host,
                                           int port,
                                           const char *data )
{
    Serialize *result = RTBOSSerializer_internalOpen( self, host, port, data );

    if( result )
    {
        Serialize_setMode( result, SERIALIZE_STREAMMODE_FLUSH | SERIALIZE_MODE_WRITE );
    }

    return result;
}


Serialize *RTBOSSerializer_getSerializePtr( RTBOSSerializer *self )
{
    return Serializer_getSerializePtr( self );
}


IOChannel *RTBOSSerializer_getIOChannelPtr( RTBOSSerializer *self )
{
    return Serializer_getIOChannelPtr( self );
}


bool RTBOSSerializer_close( RTBOSSerializer *self )
{
    return Serializer_close( self );
}


bool RTBOSSerializer_isErrorOccurred( RTBOSSerializer *self )
{
    return Serializer_isErrorOccurred( self );
}


void RTBOSSerializer_clear( RTBOSSerializer *self )
{
    Serializer_clear( self );
}


void RTBOSSerializer_delete( RTBOSSerializer *self )
{
    Serializer_delete( self );
}


static Serialize *RTBOSSerializer_internalOpen( RTBOSSerializer *self,
                                                const char *host,
                                                int port,
                                                const char *data )
{
    Serialize *result = (Serialize *)NULL;

    char initString[IOCHANNEL_INFOSTRING_MAXLEN] = { 0, };

    ANY_REQUIRE( self );
    ANY_REQUIRE( host );
    ANY_REQUIRE( port );
    ANY_REQUIRE( data );

    Any_snprintf( initString, IOCHANNEL_INFOSTRING_MAXLEN,
                  "RTBOS://%s:%d/%s@Binary", host, port, data );

    if( !IOChannel_open( self->channel, initString, IOCHANNEL_MODE_RW, IOCHANNEL_PERMISSIONS_ALL ))
    {
        ANY_LOG( 0, "An error occurred while opening the RTBOS connection %s", ANY_LOG_ERROR, initString );
        goto exit_0;
    }

    Serialize_setFormat( self->serialize, "Binary", NULL );

    IOChannel_setUseWriteBuffering( self->channel, true, true );

    result = self->serialize;

    exit_0:
    return result;
}


/* EOF */
