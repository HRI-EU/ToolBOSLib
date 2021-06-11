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
#include <iostream>
#include <fstream>

#include <Any.h>
#include <ArgvParser.h>
#include <BBDMSerialize.h>
#include <CalcSizeSerializer.h>
#include <FileSystem.h>

#include <CuTest.h>

#if defined(__windows__)
#define sleep Sleep
#endif

#define SIZEOFARRAYS  10

#define ALLTYPES_INIT( __self )\
do{\
  __self->ch = '1';\
  __self->sch = -100;\
  __self->uch = 200;\
  __self->si = 20;\
  __self->usi = 128;\
  __self->i = 10;\
  __self->ui = 500;\
  __self->li = 700;\
  __self->uli = 1000;\
  __self->ll = 1000000;\
  __self->ull = 1000000;\
  __self->f = 1.45;\
}while( 0 )

#define ARRAY_INIT( __self, __type, __name )\
do{\
  int __i;\
  for ( __i = 0; __i < SIZEOFARRAYS; ++__i )\
    __self->__name##Array[__i] = (__type)((int)'0'+__i);\
}while(0)

#define ALLTYPES_ISEQUAL( __s1, __s2 )\
do{\
  if( __s1->ch != __s2->ch )\
    return false;\
  if( __s1->sch != __s2->sch)\
    return false;\
  if( __s1->uch != __s2->uch)\
    return false;\
  if( __s1->si != __s2->si )\
    return false;\
  if( __s1->usi != __s2->usi)\
    return false;\
  if( __s1->i != __s2->i )\
    return false;\
  if( __s1->ui != __s2->ui )\
    return false;\
  if( __s1->li != __s2->li )\
    return false;\
  if( __s1->uli != __s2->uli )\
    return false;\
  if( __s1->ll != __s2->ll )\
    return false;\
  if( __s1->ull != __s2->ull )\
    return false;\
}while( 0 )

#define ARRAY_ISEQUAL( __s1, __s2, __name )\
do\
{\
  int __i;\
  for( __i =0; __i< SIZEOFARRAYS ; __i++)\
  {\
    if( __s1->__name##Array[__i] != __s2->__name##Array[__i] )\
      return false;\
  }\
} while(0)

#define EXAMPLERVP_PARAM_MAXLEN 2048

const char *formatsToTest[] = { "Binary", "Ascii", "Xml", "Matlab" };

const char *defaultStringToParse = "Reference1 = Value1 Reference2 = Value2 Reference3 = Value3 Reference4 = Value4 Reference5 = Value5 Reference6 = Value6";

typedef struct BaseStructAll
{
    char               ch;
    signed char        sch;
    unsigned char      uch;
    short int          si;
    unsigned short int usi;
    int                i;
    unsigned int       ui;
    long int           li;
    unsigned long int  uli;
    long long          ll;
    unsigned long long ull;
    float              f;
}
           BaseStructAll;

typedef struct SubStructAll
{
    char               ch;
    signed char        sch;
    unsigned char      uch;
    short int          si;
    unsigned short int usi;
    int                i;
    unsigned int       ui;
    long int           li;
    unsigned long int  uli;
    long long          ll;
    unsigned long long ull;
    float              f;
    BaseStructAll      baseStructAll;
}
           SubStructAll;

typedef struct StructAll
{
    char               ch;
    signed char        sch;
    unsigned char      uch;
    short int          si;
    unsigned short int usi;
    int                i;
    unsigned int       ui;
    long int           li;
    unsigned long int  uli;
    long long          ll;
    unsigned long long ull;
    float              f;

    char               chArray[SIZEOFARRAYS];
    signed char        schArray[SIZEOFARRAYS];
    unsigned char      uchArray[SIZEOFARRAYS];
    short int          siArray[SIZEOFARRAYS];
    unsigned short int usiArray[SIZEOFARRAYS];
    int                iArray[SIZEOFARRAYS];
    unsigned int       uiArray[SIZEOFARRAYS];
    long int           liArray[SIZEOFARRAYS];
    unsigned long int  uliArray[SIZEOFARRAYS];
    long long          llArray[SIZEOFARRAYS];
    unsigned long long ullArray[SIZEOFARRAYS];
    float              fArray[SIZEOFARRAYS];

    char string[20];

    SubStructAll subStructure;
    SubStructAll subStructureArray[SIZEOFARRAYS];
}
           StructAll;

typedef struct Example
{
    /* Used Streams In The Current Example */
    IOChannel *reader;
    IOChannel *writer;

    /* Data Used For Serialize Tests */
    StructAll *structAllToWrite;
    StructAll *structAllToRead;


    Serialize *serializer;
}
           Example;

enum
{
    OPT_HELP,
    OPT_STRING
};

static ArgvParserOptionDescriptor optionDescriptors[] =
                                          {
                                                  { 'h', "help",   ARGVPARSER_NO_PARAMETER, NULL,
                                                          "display this help" },

                                                  { 's', "string", ARGVPARSER_PARAMETER_REQUIRED, "string",
                                                          "String to parse" },

                                                  { 0, NULL, 0,                             NULL, NULL }
                                          };


static void Test_BBDMSerialize( CuTest *tc );

static void Test_calcsize( CuTest *tc );

static void Test_initmode( CuTest *tc );

static void Test_parseReferences( CuTest *tc );

static void Test_ExampleCreation( CuTest *tc );

static void Test_WriteReadAllFormats( CuTest *tc );

static void Test_CalcSizeAllFormats( CuTest *tc );

static void Test_LoopData( CuTest *tc );

static void Test_FlushData( CuTest *tc );

static void Test_MatlabEvalCode( CuTest *tc );

static void Test_NoBeginType( CuTest *tc );

static void Test_MemoryStream( CuTest *tc );

static void Test_StructArray( CuTest *tc );

StructAll *StructAll_new( void );

bool StructAll_init( StructAll *self );

void StructAll_setTestValues( StructAll *self );

bool StructAll_isEqual( StructAll *s1, StructAll *s2 );

void StructAll_clear( StructAll *self );

void StructAll_delete( StructAll *self );

static void SubStructAll_serialize( SubStructAll *self, char *name, Serialize *s );

static void StructAll_serialize( StructAll *self, const char *name, Serialize *s );

static void ExampleRVP_printElement( SerializeReferenceValue *self );

static void ExampleRVP_printList( SerializeReferenceValue *self );

static bool ExampleRVP_parseArgs( ArgvParser argvParser, int argc, char *argv[], char *string );

static void usage( ArgvParser *argvParser );

static void Example_create( Example *example );

static void Example_destroy( Example *example );

static void Test_StructArray_serialize( Example *example );

static void Test_StructArray_deserialize( Example *example );

static bool compareFiles( const char *testName, const char *file1, const char *file2 );


static void Test_BBDMSerialize( CuTest *tc )
{
    Serialize   *serializer  = (Serialize *)NULL;
    IOChannel   *stream      = (IOChannel *)NULL;
    BBDMTag     *tag         = ANY_TALLOC( BBDMTag );
    bool        errorOccured = false;
    std::string streamname   = "File://";

    /*Compiler gives warning that `tempnam' is dangerous, better use `mkstemp'. After analyzing 'mkstemp()' the
     * conclusion is, it returns file descriptor which is just an integer number that uniquely represents an opened
     * file in operating system. But here we need information like file names and length which is easy to get from
     * 'tempname'. Thus 'mkstemp()' is not implemented. */
    char        *filename    = tempnam( NULL, "test-" );

    streamname += filename;

    ANY_REQUIRE( tag );

    ANY_LOG( 5, "Test_BBDMSerialize : create output stream", ANY_LOG_INFO );
    stream = IOChannel_new();
    if( stream == (IOChannel *)NULL )
    {
        errorOccured = true;
    }
    else
    {
        ANY_LOG( 5, "Test_BBDMSerialize : init output stream", ANY_LOG_INFO );
        if( !IOChannel_init( stream ) )
        {
            errorOccured = true;
        }
        else
        {
            ANY_LOG( 5, "Test_BBDMSerialize : open output stream", ANY_LOG_INFO );
            if( !IOChannel_open( stream, streamname.c_str(),
                                 IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                                 IOCHANNEL_PERMISSIONS_ALL ) )
            {
                errorOccured = true;
            }
            else
            {
                // get serializer object ready to run
                ANY_LOG( 5, "Test_BBDMSerialize : create serializer", ANY_LOG_INFO );
                serializer = Serialize_new();
                if( serializer == (Serialize *)NULL )
                {
                    errorOccured = true;
                }
                else
                {
                    ANY_LOG( 5, "Test_BBDMSerialize : init serializer", ANY_LOG_INFO );
                    if( !Serialize_init( serializer, stream,
                                         SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE ) )
                    {
                        errorOccured = true;
                    }
                    else
                    {
                        ANY_LOG( 5, "Test_BBDMSerialize : set serializer format to Ascii", ANY_LOG_INFO );
                        Serialize_setFormat( serializer, "Ascii", NULL );

                        // fill tag with data
                        tag->timestep = 123456789;

                        // print value as text on stdout
                        ANY_LOG( 5, "Test_BBDMSerialize : serialize BBDM", ANY_LOG_INFO );
                        BBDMTag_serialize( tag, (char *)"tag", serializer );

                        // release ressources
                        ANY_LOG( 5, "Test_BBDMSerialize : clear serializer", ANY_LOG_INFO );
                        Serialize_clear( serializer );
                    }
                    ANY_LOG( 5, "Test_BBDMSerialize : delete serializer", ANY_LOG_INFO );
                    Serialize_delete( serializer );
                    ANY_LOG( 5, "Test_BBDMSerialize : close stream", ANY_LOG_INFO );
                    IOChannel_close( stream );
                }
                ANY_LOG( 5, "Test_BBDMSerialize : clear stream", ANY_LOG_INFO );
                IOChannel_clear( stream );
            }
            ANY_LOG( 5, "Test_BBDMSerialize : delete stream", ANY_LOG_INFO );
            IOChannel_delete( stream );
        }
    }
    ANY_FREE( tag );

    // Finally compare generated file with the model
    CuAssertTrue( tc, compareFiles( "Test_BBDMSerialize",
                                    filename,
                                    "Reference_BBDMSerialize.txt" ) );

    // Remove the file (if it had the chance to be created - we do not check this)
    remove( filename );

    ANY_LOG( 1, "Test_BBDMSerialize : test done", ANY_LOG_INFO );
    CuAssertTrue( tc, ! errorOccured );
}


static void Test_calcsize( CuTest *tc )
{
#if !defined(__MSVC__)

    Base2DI32 data1 = { 42, 84 };
    Base2DI32 data2 = { -1, -1 };

    CalcSizeSerializer *cs           = (CalcSizeSerializer *)NULL;
    MemorySerializer   *ms           = (MemorySerializer *)NULL;
    FileSerializer     *fs           = (FileSerializer *)NULL;
    Serialize          *s            = (Serialize *)NULL;
    int                fd            = -1;
    char               filename[256] = "/tmp/test-XXXXXX";
    char               *memBuffer    = (char *)NULL;
    ssize_t            written       = 0;
    ssize_t            totalSize     = -1;


    // compute serialized size and allocate buffer

    ANY_LOG( 5, "Test_calcsize : create new CalcSizeSerializer", ANY_LOG_INFO );
    cs = CalcSizeSerializer_new();
    CuAssertTrue( tc, cs != (CalcSizeSerializer *)NULL );
    ANY_LOG( 5, "Test_calcsize : init new CalcSizeSerializer", ANY_LOG_INFO );
    CuAssertTrue( tc, CalcSizeSerializer_init( cs ) == 0 );
    ANY_LOG( 5, "Test_calcsize : open CalcSizeSerializer in Ascii mode", ANY_LOG_INFO );
    s = CalcSizeSerializer_open( cs, "Ascii" );
    CuAssertTrue( tc, s != (Serialize *)NULL );

    ANY_LOG( 5, "Test_calcsize : serialize { 42, 84 } using Base2DI32_serialize", ANY_LOG_INFO );
    Base2DI32_serialize( &data1, "store", s );
    ANY_LOG( 5, "Test_calcsize : get total size using CalcSizeSerializer_getTotalSize", ANY_LOG_INFO );
    totalSize = CalcSizeSerializer_getTotalSize( cs );
    ANY_LOG( 5, "Test_calcsize : totalSize is [%ld] and should be [134]", ANY_LOG_INFO, totalSize );

    memBuffer = (char *)ANY_BALLOC( totalSize );
    ANY_REQUIRE( memBuffer );


    // in-memory serialization

    ANY_LOG( 5, "Test_calcsize : create new MemorySerializer", ANY_LOG_INFO );
    ms = MemorySerializer_new();
    CuAssertTrue( tc, ms != (MemorySerializer *)NULL );
    ANY_LOG( 5, "Test_calcsize : init new MemorySerializer", ANY_LOG_INFO );
    CuAssertTrue( tc, MemorySerializer_init( ms ) == 0 );
    ANY_LOG( 5, "Test_calcsize : open MemorySerializer in Ascii mode", ANY_LOG_INFO );
    s = MemorySerializer_openForWriting( ms, memBuffer, totalSize, "Ascii" );
    CuAssertTrue( tc, s != (Serialize *)NULL );
    ANY_LOG( 5, "Test_calcsize : serialize { 42, 84 } using Base2DI32_serialize", ANY_LOG_INFO );
    Base2DI32_serialize( &data1, "store", s );


    // write serialized data1 from memory to file

    fd = mkstemp( filename );     /* mkstemp writes temp.filename to buffer */
    ANY_LOG( 5, "Test_calcsize : prepare temp file [%s] with handle [%d]", ANY_LOG_INFO, filename, fd );
    CuAssertTrue( tc, fd > 0 );

    ANY_LOG( 5, "Test_calcsize : write...\n[%s]\n...from memory to file", ANY_LOG_INFO, memBuffer );
    written = write( fd, memBuffer, totalSize );
    close( fd );
    ANY_LOG( 5, "Test_calcsize : wrote [%ld] bytes from memory buffer to file", ANY_LOG_INFO, written );
    CuAssertTrue( tc, written > 0 );

    ANY_LOG( 5, "Test_calcsize : filesize is [%d] bytes", ANY_LOG_INFO, FileSystem_getSize( filename ) );

    ANY_LOG( 5, "Test_calcsize : compare sizes", ANY_LOG_INFO );
    CuAssertTrue( tc, written == totalSize );
    CuAssertTrue( tc, FileSystem_getSize( filename ) == totalSize );


    // try to deserialize from file

    ANY_LOG( 5, "Test_calcsize : create new FileSerializer", ANY_LOG_INFO );
    fs = FileSerializer_new();
    CuAssertTrue( tc, fs != (FileSerializer *)NULL );
    ANY_LOG( 5, "Test_calcsize : init new FileSerializer", ANY_LOG_INFO );
    CuAssertTrue( tc, FileSerializer_init( fs ) == 0 );
    ANY_LOG( 5, "Test_calcsize : open FileSerializer to read file %s", ANY_LOG_INFO, filename );
    s = FileSerializer_openForReading( fs, filename );
    CuAssertTrue( tc, s != (Serialize *)NULL );
    ANY_LOG( 5, "Test_calcsize : set FileSerializer in Ascii mode", ANY_LOG_INFO );
    Serialize_setFormat( s, "Ascii", "" );

    ANY_LOG( 5, "Test_calcsize : get data from file", ANY_LOG_INFO );
    Base2DI32_serialize( &data2, "store", s );

    ANY_LOG( 5, "Test_calcsize : found { %d, %d } into the file", ANY_LOG_INFO, data2.x, data2.y );
    CuAssertTrue( tc, data1.x == data2.x );
    CuAssertTrue( tc, data1.y == data2.y );


    // release resources

    ANY_LOG( 5, "Test_calcsize : realease the ressources", ANY_LOG_INFO );

    ANY_FREE( memBuffer );

    CalcSizeSerializer_clear( cs );
    CalcSizeSerializer_delete( cs );

    MemorySerializer_clear( ms );
    MemorySerializer_delete( ms );

    FileSerializer_clear( fs );
    FileSerializer_delete( fs );
    ANY_LOG( 1, "Test_calcsize : test done.", ANY_LOG_INFO );
#endif
}


static void Test_initmode( CuTest *tc )
{
    MemI8       *outData   = (MemI8 *)NULL;
    MemI8       *inData    = (MemI8 *)NULL;
    BaseI8      *outBuf    = (BaseI8 *)NULL;
    std::string streamname = "File://";
    char        *filename  = tempnam( NULL, "test-" );

    streamname += filename;

    IOChannel *inStreamer  = IOChannel_new();
    IOChannel *outStreamer = IOChannel_new();

    Serialize *inSerializer  = Serialize_new();
    Serialize *outSerializer = Serialize_new();

    CuAssertTrue( tc, inStreamer != NULL );
    CuAssertTrue( tc, outStreamer != NULL );

    outData = MemI8_new();
    CuAssertTrue( tc, outData != NULL );

    MemI8_init( outData, 20 );
    outBuf = MemI8_getBuffer( outData );
    CuAssertTrue( tc, outBuf != NULL );
    Any_strncpy( (char *)outBuf, "Hello World!", 19 );

    IOChannel_init( outStreamer );
    IOChannel_init( inStreamer );

    /* the IOChannel_openFromString does not allow a tilde ("~") in filenames,
       so we have to use the usual IOChannel_open() function here */

    ANY_LOG( 5, "Test_initmode : open output stream", ANY_LOG_INFO );
    IOChannel_open( outStreamer, streamname.c_str(),
                    IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                    IOCHANNEL_PERMISSIONS_ALL );

    ANY_LOG( 5, "Test_initmode : open input stream", ANY_LOG_INFO );
    IOChannel_open( inStreamer, streamname.c_str(),
                    IOCHANNEL_MODE_R_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                    IOCHANNEL_PERMISSIONS_ALL );

    ANY_LOG( 5, "Test_initmode : init streams and assign them to serializers", ANY_LOG_INFO );
    Serialize_init( inSerializer, inStreamer, SERIALIZE_STREAMMODE_NORMAL );
    Serialize_init( outSerializer, outStreamer, SERIALIZE_STREAMMODE_NORMAL );

    ANY_LOG( 5, "Test_initmode : check for error on streams", ANY_LOG_INFO );
    CuAssertTrue( tc, !Serialize_isErrorOccurred( inSerializer ) );
    CuAssertTrue( tc, !Serialize_isErrorOccurred( outSerializer ) );

    ANY_LOG( 5, "Test_initmode : set serializer direction (out and in)", ANY_LOG_INFO );
    Serialize_setMode( outSerializer, SERIALIZE_MODE_WRITE );
    Serialize_setMode( inSerializer, SERIALIZE_MODE_READ );

    ANY_LOG( 5, "Test_initmode : set serializer format to Ascii", ANY_LOG_INFO );
    Serialize_setFormat( outSerializer, "Ascii", "" );
    Serialize_setFormat( inSerializer, "Ascii", "" );


    // write test data to outputfile

    ANY_LOG( 5, "Test_initmode : write data using output serializer", ANY_LOG_INFO );
    MemI8_serialize( outData, (char *)"myData", outSerializer );


    // read data into second variable

    ANY_LOG( 5, "Test_initmode : read data using input serializer", ANY_LOG_INFO );
    inData = MemI8_new();
    CuAssertTrue( tc, inData != (MemI8 *)NULL );
    Serialize_setInitMode( inSerializer, true );
    MemI8_serialize( inData, (char *)"myData", inSerializer );

    ANY_LOG( 5, "Test_initmode : compare serialized and de-serialized data", ANY_LOG_INFO );
    CuAssertTrue( tc, strcmp( (char *)outData->buffer, (char *)inData->buffer ) == 0 );

    // release ressources

    ANY_LOG( 5, "Test_initmode : release ressources", ANY_LOG_INFO );
    Serialize_clear( inSerializer );
    Serialize_clear( outSerializer );

    Serialize_delete( inSerializer );
    Serialize_delete( outSerializer );

    IOChannel_close( outStreamer );
    IOChannel_close( inStreamer );

    IOChannel_clear( outStreamer );
    IOChannel_clear( inStreamer );

    IOChannel_delete( outStreamer );
    IOChannel_delete( inStreamer );

    MemI8_clear( inData );
    MemI8_clear( outData );

    MemI8_delete( inData );
    MemI8_delete( outData );

    // Remove the file (if it had the chance to be created - we do not check this)
    remove( filename );

    ANY_LOG( 1, "Test_initmode : test done", ANY_LOG_INFO );
}


static void Test_parseReferences( CuTest *tc )
{
    ArgvParser              argvParser;
    SerializeReferenceValue *rvp          = (SerializeReferenceValue *)NULL;
    SerializeReferenceValue *listHead     = (SerializeReferenceValue *)NULL;
    SerializeReferenceValue *listTail     = (SerializeReferenceValue *)NULL;
    SerializeReferenceValue *cache        = (SerializeReferenceValue *)NULL;
    char                    *string       = (char *)NULL;
    char                    *ref          = (char *)"Reference4";
    char                    *val          = (char *)NULL;
    char                    references[1] = "";
    char                    *argv         = (char *)NULL;

    argv = &references[ 0 ];

    CuAssertTrue( tc, ExampleRVP_parseArgs( argvParser, 1, &argv, string ) );

    if( string == NULL )
    {
        string = (char *)defaultStringToParse;
    }

    rvp = SerializeReferenceValue_new();
    SerializeReferenceValue_init( rvp, "", NULL );
    listHead = rvp;

    listTail = listHead;

    rvp = SerializeReferenceValue_new();
    SerializeReferenceValue_init( rvp, "", NULL );
    cache = rvp;

    SerializeReferenceValue_getRVP( &listHead, &cache, &listTail, string );

    ExampleRVP_printList( listHead );

    ANY_LOG( 5, "Test_parseReferences: Calling findValue to get the value associated with reference '%s'", ANY_LOG_INFO,
             ref );
    val = SerializeReferenceValue_findValue( listHead, ref );
    ANY_REQUIRE( val );
    ANY_LOG( 5, "Test_parseReferences: %s -> %s", ANY_LOG_INFO, ref, val );

    /* Add new element on top */
    ref = (char *)"Reference7";
    val = (char *)"Value7";

    ANY_LOG( 5, "Test_parseReferences: Adding new element with ref '%s' and value '%s'", ANY_LOG_INFO, ref, val );
    rvp = SerializeReferenceValue_new();
    SerializeReferenceValue_init( rvp, ref, val );
    SerializeReferenceValue_push( &listHead, rvp );

    ExampleRVP_printList( listHead );

    SerializeReferenceValue_destroyList( listHead );
    SerializeReferenceValue_destroyList( cache );

    ANY_LOG( 1, "Test_parseReferences: test done", ANY_LOG_INFO );
}


static void Test_WriteReadAllFormats( CuTest *tc )
{
    Example      *example   = new(Example);
    bool         status     = false;
    unsigned int i          = 0;
    struct stat  stat_buf;
    int          rc;
    long int     readBytes;
    std::string  streamname = "File://";
    char         *filename  = tempnam( NULL, "test-" );

    streamname += filename;

    Example_create( example );

    ANY_LOG( 5, "Test_WriteReadAllFormats: Open the IOChannel for the writer", ANY_LOG_INFO );
    status = IOChannel_open( example->writer, streamname.c_str(),
                             IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                             IOCHANNEL_PERMISSIONS_ALL );
    CuAssertTrue( tc, status );

    ANY_LOG( 5, "Test_WriteReadAllFormats: IOChannel_setUseWriteBuffering", ANY_LOG_INFO );
    IOChannel_setUseWriteBuffering( example->writer, true, true );

    ANY_LOG( 5, "Test_WriteReadAllFormats: set serializer mode to SERIALIZE_MODE_WRITE | SERIALIZE_MODE_AUTOCALC",
             ANY_LOG_INFO );
    Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE | SERIALIZE_MODE_AUTOCALC );

    Serialize_setStream( example->serializer, example->writer );

    ANY_LOG( 5, "Test_WriteReadAllFormats: Writing Data using AutocalcSize Flag...", ANY_LOG_INFO );

    for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
    {
        ANY_LOG( 5, "Test_WriteReadAllFormats: set serializer format to [%s]", ANY_LOG_INFO, formatsToTest[ i ] );

        if( i == 1 )
        {
            ANY_LOG( 5,
                     "Test_WriteReadAllFormats: For the Ascii case, you should see a warning about String vs Char types",
                     ANY_LOG_INFO );
        }

        Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

        StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
        CuAssertTrue( tc, !Serialize_isErrorOccurred( example->serializer ) );

        ANY_LOG( 5,
                 "Test_WriteReadAllFormats: just serialized using Format[%s] HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                 ANY_LOG_INFO,
                 formatsToTest[ i ],
                 Serialize_getHeaderSize( example->serializer ),
                 Serialize_getPayloadSize( example->serializer ),
                 Serialize_getMaxSerializeSize( example->serializer ) );
    }

    ANY_LOG( 5, "Test_WriteReadAllFormats: Total written bytes = [%ld] ( Should Be Equal To File Size )", ANY_LOG_INFO,
             IOChannel_getWrittenBytes( example->writer ) );

    ANY_LOG( 5, "Test_WriteReadAllFormats: Close the IOChannel for the writer", ANY_LOG_INFO );
    IOChannel_close( example->writer );

    ANY_LOG( 5, "Test_WriteReadAllFormats: Open the IOChannel to read file out.Test_WriteReadAllFormats.txt",
             ANY_LOG_INFO );
    status = IOChannel_open( example->reader, streamname.c_str(),
                             IOCHANNEL_MODE_R_ONLY, IOCHANNEL_PERMISSIONS_ALL );
    CuAssertTrue( tc, status );

    Serialize_setMode( example->serializer, SERIALIZE_MODE_READ );

    Serialize_setStream( example->serializer, example->reader );

    ANY_LOG( 5, "Test_WriteReadAllFormats: Reading Data...", ANY_LOG_INFO );

    for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
    {
        ANY_LOG( 5, "Test_WriteReadAllFormats: set serializer format to [%s]", ANY_LOG_INFO, formatsToTest[ i ] );
        Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

        memset( example->structAllToRead, '0', sizeof( StructAll ) );

        StructAll_serialize( example->structAllToRead, "structAll", example->serializer );
        CuAssertTrue( tc, !Serialize_isErrorOccurred( example->serializer ) );

        ANY_LOG( 5,
                 "Test_WriteReadAllFormats: just deserialized using Format[%s] HeaderSize[%ld] RealSerializeSize[%ld]",
                 ANY_LOG_INFO, formatsToTest[ i ],
                 Serialize_getHeaderSize( example->serializer ),
                 Serialize_getPayloadSize( example->serializer ) );

        if( StructAll_isEqual( example->structAllToWrite, example->structAllToRead ) )
        {
            ANY_LOG( 5, "Test_WriteReadAllFormats: Structs Are Equal!", ANY_LOG_INFO );
        }
        else
        {
            ANY_LOG( 5, "Test_WriteReadAllFormats: Structs Are Different!", ANY_LOG_INFO );
        }
    }

    readBytes = IOChannel_getReadBytes( example->reader );
    rc        = stat( filename, &stat_buf );
    if( rc == 0 )
    {
        CuAssertTrue( tc, readBytes == stat_buf.st_size );
        ANY_LOG( 5, "Test_WriteReadAllFormats: Read Bytes[%ld] is equal to file size )", ANY_LOG_INFO, readBytes );
    }
    else
    {
        ANY_LOG( 5, "Test_WriteReadAllFormats: Read Bytes[%ld] ( Not sure if equal to file size or not )", ANY_LOG_INFO,
                 readBytes );
    }

    IOChannel_close( example->reader );

    Example_destroy( example );

    CuAssertTrue( tc, compareFiles( "Test_WriteReadAllFormats",
                                    filename,
                                    "Reference_WriteReadAllFormats.txt" ) );

    remove( filename );

    delete example;

    ANY_LOG( 1, "Test_WriteReadAllFormats: test done", ANY_LOG_INFO );
}


static void Test_CalcSizeAllFormats( CuTest *tc )
{
    Example      *example = new(Example);
    unsigned int i        = 0;
    char         *tmp     = (char *)NULL;

    Example_create( example );

    for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
    {
        Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

        Serialize_setMode( example->serializer, SERIALIZE_MODE_CALC );

        StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
        CuAssertTrue( tc, !Serialize_isErrorOccurred( example->serializer ) );

        ANY_LOG( 5, "Test_CalcSizeAllFormats: FORMAT[%s]"
                " HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                 ANY_LOG_INFO, formatsToTest[ i ],
                 Serialize_getHeaderSize( example->serializer ),
                 Serialize_getPayloadSize( example->serializer ),
                 Serialize_getMaxSerializeSize( example->serializer ) );

        /* Retrieve some other info */
        tmp = Serialize_getHeaderTypePtr( example->serializer );
        CuAssertTrue( tc, tmp != NULL );
        ANY_REQUIRE( tmp );
        ANY_LOG( 5, "Test_CalcSizeAllFormats: Type is[%s]", ANY_LOG_INFO, tmp );
        CuAssertTrue( tc, strcmp( tmp, "StructAll" ) == 0 );

        tmp = Serialize_getHeaderNamePtr( example->serializer );
        CuAssertTrue( tc, tmp != NULL );
        ANY_REQUIRE( tmp );
        ANY_LOG( 5, "Test_CalcSizeAllFormats: Name is[%s]", ANY_LOG_INFO, tmp );
        CuAssertTrue( tc, Any_strcmp( tmp, "structAll" ) == 0 );

        tmp = Serialize_getHeaderOptsPtr( example->serializer );
        CuAssertTrue( tc, tmp != NULL );
        ANY_LOG( 5, "Test_CalcSizeAllFormats: Options [%s]", ANY_LOG_INFO, tmp );
        ANY_LOG( 5, "Test_CalcSizeAllFormats: Size of the Options [%d]", ANY_LOG_INFO, (int)Any_strlen( tmp ) );

        switch( i )
        {
            case 0:
                CuAssertTrue( tc, strcmp( tmp, "BIG_ENDIAN" ) == 0 );
                break;
            case 1:
                CuAssertTrue( tc, strcmp( tmp, "WITH_TYPE=FALSE" ) == 0 );
                break;
            case 2:
                CuAssertTrue( tc, (int)Any_strlen( tmp ) == 0 );
                break;
            case 3:
                CuAssertTrue( tc, (int)Any_strlen( tmp ) == 0 );
                break;
        }

    }

    Example_destroy( example );

    delete example;

    ANY_LOG( 1, "Test_CalcSizeAllFormats: test done", ANY_LOG_INFO );
}


static void Test_LoopData( CuTest *tc )
{
    Example    *example     = new(Example);
    const char *infoString  = "Shm:///out.Test_LoopData.out.txt";
    long       shmSize      = 10 * 1024;
    bool       status       = false;
    int        i            = 0;
    bool       errorOccured = false;

    Example_create( example );

    ANY_LOG( 5, "Test_LoopData: Writes Data Looping into %s", ANY_LOG_INFO, infoString );

    status = IOChannel_open( example->writer,
                             infoString,
                             IOCHANNEL_MODE_RW | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                             IOCHANNEL_PERMISSIONS_ALL, shmSize );

    if( !status )
    {
        errorOccured = true;
    }
    else
    {
        Serialize_setStream( example->serializer, example->writer );
        Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE | SERIALIZE_STREAMMODE_LOOP );
        Serialize_setFormat( example->serializer, "Ascii", "" );

        ANY_LOG( 5, "Test_LoopData: You should see 10 warnings about String and Char", ANY_LOG_INFO );
        while( i < 1 * 10 )
        {
            StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
            if( Serialize_isErrorOccurred( example->serializer ) )
            {
                errorOccured = true;
            }
            i++;
        }
        IOChannel_close( example->writer );
    }

    Example_destroy( example );

    delete example;

    ANY_LOG( 1, "Test_LoopData: test done", ANY_LOG_INFO );
    CuAssertTrue( tc, !errorOccured );
}


static void Test_FlushData( CuTest *tc )
{
    Example    *example     = new(Example);
    const char *infoString  = "StdOut://";
    bool       status       = false;
    int        i            = 0;
    int        modes        = SERIALIZE_MODE_WRITE | SERIALIZE_MODE_AUTOCALC;
    bool       errorOccured = false;

    Example_create( example );

    ANY_LOG( 5, "Test_FlushData: open output channel", ANY_LOG_INFO );
    status = IOChannel_open( example->writer,
                             infoString,
                             IOCHANNEL_MODE_W_ONLY,
                             IOCHANNEL_PERMISSIONS_ALL );
    CuAssertTrue( tc, status );

    ANY_LOG( 5, "Test_FlushData: use write buffering", ANY_LOG_INFO );
    IOChannel_setUseWriteBuffering( example->writer, true, true );

    Serialize_setStream( example->serializer, example->writer );
    Serialize_setMode( example->serializer, modes );

    ANY_LOG( 5, "Test_FlushData: set Matlab format", ANY_LOG_INFO );
    Serialize_setFormat( example->serializer, "Matlab", "" );

    for( i = 0; i < 3; i++ )
    {
        ANY_LOG( 5, "Test_FlushData: serialize to StdOut [%d/3]", ANY_LOG_INFO, i + 1 );
        StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
        if( Serialize_isErrorOccurred( example->serializer ) )
        {
            errorOccured = true;
        }
        ANY_LOG( 5, "Test_FlushData: System will now sleep for 1 second.", ANY_LOG_INFO );
        fflush( NULL );
        sleep( 1 );
    }

    IOChannel_close( example->writer );
    Example_destroy( example );

    delete example;

    ANY_LOG( 5, "Test_FlushData: test done.", ANY_LOG_INFO );

    CuAssertTrue( tc, !errorOccured );
}


static void Test_MatlabEvalCode( CuTest *tc )
{
    Example     *example     = new(Example);
    bool        status       = false;
    std::string streamname   = "File://";
    char        *filename    = tempnam( NULL, "test-" );
    bool        errorOccured = false;

    streamname += filename;

    Example_create( example );

    ANY_LOG( 5, "Test_MatlabEvalCode: generate Matlab Code", ANY_LOG_INFO );

    status = IOChannel_open( example->writer,
                             streamname.c_str(),
                             IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                             IOCHANNEL_PERMISSIONS_ALL );
    if( !status )
    {
        errorOccured = true;
    }
    else
    {
        Serialize_setStream( example->serializer, example->writer );

        Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE | SERIALIZE_MODE_NOHEADER );

        Serialize_setFormat( example->serializer, "Matlab", "" );

        StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
        if( Serialize_isErrorOccurred( example->serializer ) )
        {
            errorOccured = true;
        }

        IOChannel_printf( example->writer, "structAll\n" );
        IOChannel_close( example->writer );
    }

    Example_destroy( example );

    ANY_LOG( 5, "Test_MatlabEvalCode: compare generated Matlab Code with expected result", ANY_LOG_INFO );

    if( !compareFiles( "Test_MatlabEvalCode", filename,
                       "Reference_MatlabEvalCode.txt" ) )
    {
        errorOccured = true;
    }

    remove( filename );

    delete example;

    ANY_LOG( 5, "Test_MatlabEvalCode: test done", ANY_LOG_INFO );

    CuAssertTrue( tc, !errorOccured );
}


static void Test_NoBeginType( CuTest *tc )
{
    Example      *example         = new(Example);
    bool         status           = false;
    unsigned int i                = 0;
    int          myInt            = 7;
    float        myFloat          = 12.9;
    char         myString[32];
    int          myIntArray[10]   = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    float        myFloatArray[10] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
    std::string  streamname       = "File://";
    char         *filename        = tempnam( NULL, "test-" );
    bool         errorOccured     = false;

    streamname += filename;

    Example_create( example );

    ANY_LOG( 5, "Test_NoBeginType: Call Serialize Functions Directly", ANY_LOG_INFO );

    Any_strcpy( myString, "my\\x124Quoted\\nString" );

    status = IOChannel_open( example->writer,
                             streamname.c_str(),
                             IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
                             IOCHANNEL_PERMISSIONS_ALL );
    if( !status )
    {
        errorOccured = true;
    }
    else
    {
        Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE );
        Serialize_setStream( example->serializer, example->writer );

        for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
        {
            Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

            FloatArray_serialize( myFloatArray, "myFloatArray", 10, example->serializer );
            Float_serialize( &myFloat, "myFloat", example->serializer );
            Int_serialize( &myInt, "myInt", example->serializer );
            IntArray_serialize( myIntArray, "myIntArray", 10, example->serializer );

            ANY_LOG( 5, "Test_NoBeginType: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                     ANY_LOG_INFO, formatsToTest[ i ],
                     Serialize_getHeaderSize( example->serializer ),
                     Serialize_getPayloadSize( example->serializer ),
                     Serialize_getMaxSerializeSize( example->serializer ) );
        }

        ANY_LOG( 5, "Test_NoBeginType: Written Bytes[%ld] ( Should Be Equal To File Size )", ANY_LOG_INFO,
                 IOChannel_getWrittenBytes( example->writer ) );

        IOChannel_close( example->writer );

        status = IOChannel_open( example->reader, streamname.c_str(),
                                 IOCHANNEL_MODE_R_ONLY, IOCHANNEL_PERMISSIONS_ALL );
        if( !status )
        {
            errorOccured = true;
        }
        else
        {
            Serialize_setMode( example->serializer, SERIALIZE_MODE_READ | SERIALIZE_MODE_NOHEADER );
            Serialize_setStream( example->serializer, example->reader );

            for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
            {
                Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

                ANY_LOG( 5, "[%s]", ANY_LOG_INFO, myString );
                FloatArray_serialize( myFloatArray, "myFloatArray", 10, example->serializer );
                Float_serialize( &myFloat, "myFloat", example->serializer );
                Int_serialize( &myInt, "myInt", example->serializer );
                IntArray_serialize( myIntArray, "myIntArray", 10, example->serializer );

                ANY_LOG( 5, "Test_NoBeginType: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                         ANY_LOG_INFO, formatsToTest[ i ],
                         Serialize_getHeaderSize( example->serializer ),
                         Serialize_getPayloadSize( example->serializer ),
                         Serialize_getMaxSerializeSize( example->serializer ) );
            }

            ANY_LOG( 5, "Test_NoBeginType: Read Bytes[%ld] ( Should Be Equal To File Size )", ANY_LOG_INFO,
                     IOChannel_getReadBytes( example->reader ) );

            IOChannel_close( example->reader );
        }
    }

    Example_destroy( example );

    ANY_LOG( 5, "Test_NoBeginType: compare generated Matlab Code with expected result", ANY_LOG_INFO );

    if( !compareFiles( "Test_NoBeginType", filename,
                       "Reference_NoBeginType.txt" ) )
    {
        errorOccured = true;
    }

    remove( filename );

    delete example;

    ANY_LOG( 1, "Test_NoBeginType: test done", ANY_LOG_INFO );
    CuAssertTrue( tc, !errorOccured );
}


static void Test_MemoryStream( CuTest *tc )
{
    Example      *example         = new(Example);
    char         *memoryBuffer    = (char *)NULL;
    long         memoryBufferSize = ( 1024 ) * ( 1024 );
    bool         status           = false;
    unsigned int i                = 0;
    const char   *infoString      = "Mem://";
    bool         errorOccured     = false;

    Example_create( example );

    ANY_LOG( 5, "Test_MemoryStream: Example Of Serialization on Memory Stream...\n", ANY_LOG_INFO );

    status = IOChannel_open( example->writer, infoString,
                             IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_NOTCLOSE,
                             IOCHANNEL_PERMISSIONS_ALL, NULL, memoryBufferSize );
    if( !status )
    {
        errorOccured = true;
    }
    else
    {
        Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE | SERIALIZE_MODE_AUTOCALC );
        Serialize_setStream( example->serializer, example->writer );

        ANY_LOG( 5, "Test_MemoryStream: Writing Data...", ANY_LOG_INFO );

        for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
        {
            Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );

            StructAll_serialize( example->structAllToWrite, "structAll", example->serializer );
            CuAssertTrue( tc, !Serialize_isErrorOccurred( example->serializer ) );

            ANY_LOG( 5, "Test_MemoryStream: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                     ANY_LOG_INFO,
                     formatsToTest[ i ],
                     Serialize_getHeaderSize( example->serializer ),
                     Serialize_getPayloadSize( example->serializer ),
                     Serialize_getMaxSerializeSize( example->serializer ) );
        }

        ANY_LOG( 5, "Test_MemoryStream: Written Bytes[%ld]", ANY_LOG_INFO,
                 IOChannel_getWrittenBytes( example->writer ) );

        if( IOChannel_hasPointer( example->writer ) )
        {
            memoryBuffer = (char *)IOChannel_getProperty( example->writer, (char *)"MemPointer" );
            if( memoryBuffer == NULL )
            {
                errorOccured = true;
            }
        }

        IOChannel_close( example->writer );

        if( !errorOccured )
        {
            status = IOChannel_open( example->reader, infoString,
                                     IOCHANNEL_MODE_R_ONLY | IOCHANNEL_MODE_CLOSE,
                                     IOCHANNEL_PERMISSIONS_ALL, memoryBuffer, memoryBufferSize );
            if( !status )
            {
                errorOccured = true;
            }
            else
            {
                Serialize_setMode( example->serializer, SERIALIZE_MODE_READ );
                Serialize_setStream( example->serializer, example->reader );

                ANY_LOG( 5, "Test_MemoryStream: Reading Data...", ANY_LOG_INFO );

                for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
                {
                    memset( example->structAllToRead, '0', sizeof( StructAll ) );

                    StructAll_serialize( example->structAllToRead, "structAll", example->serializer );
                    if( Serialize_isErrorOccurred( example->serializer ) )
                    {
                        errorOccured = true;
                    }
                    else
                    {
                        ANY_LOG( 5, "Test_MemoryStream: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld]",
                                 ANY_LOG_INFO, formatsToTest[ i ],
                                 Serialize_getHeaderSize( example->serializer ),
                                 Serialize_getPayloadSize( example->serializer ) );

                        if( StructAll_isEqual( example->structAllToWrite, example->structAllToRead ) )
                        {
                            ANY_LOG( 5, "Test_MemoryStream: Structs Are Equal.", ANY_LOG_INFO );
                        }
                        else
                        {
                            ANY_LOG( 5, "Test_MemoryStream: Structs Are Different!", ANY_LOG_INFO );
                        }
                    }
                }

                ANY_LOG( 5, "Test_MemoryStream: Read Bytes[%ld]", ANY_LOG_INFO,
                         IOChannel_getReadBytes( example->reader ) );
                IOChannel_close( example->reader );
            }
        }
    }

    Example_destroy( example );

    delete example;

    ANY_LOG( 1, "Test_MemoryStream: test done", ANY_LOG_INFO );

    CuAssertTrue( tc, !errorOccured );
}


static void Test_StructArray( CuTest *tc )
{
    Example      *example         = new(Example);
    const char   *memoryBuffer    = (char *)NULL;
    long         memoryBufferSize = ( 1024 ) * ( 1024 );
    bool         status           = false;
    unsigned int i                = 0;
    std::string  streamname       = "File://";
    char         *filename        = tempnam( NULL, "test-" );
    bool         errorOccured     = false;

    streamname += filename;

    Example_create( example );

    ANY_LOG( 5, "Test_StructArray: Example Of Serialization of Array Of Structures...\n", ANY_LOG_INFO );

    status = IOChannel_open( example->writer, streamname.c_str(),
                             IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_CREAT |
                             IOCHANNEL_MODE_NOTCLOSE,
                             IOCHANNEL_PERMISSIONS_ALL, NULL, memoryBufferSize );
    if( status == false )
    {
        errorOccured = true;
    }
    else
    {
        Serialize_setMode( example->serializer, SERIALIZE_MODE_WRITE | SERIALIZE_MODE_AUTOCALC );

        Serialize_setStream( example->serializer, example->writer );

        ANY_LOG( 5, "Test_StructArray: Writing Data...", ANY_LOG_INFO );

        for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
        {
            /* Serialize Data */
            Serialize_setFormat( example->serializer, formatsToTest[ i ], "" );
            Test_StructArray_serialize( example ); // Has to be in a separate function because of begin...end structure
            ANY_LOG( 5, "Test_StructArray: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld] MaxSerializeSize[%ld]",
                     ANY_LOG_INFO,
                     formatsToTest[ i ],
                     Serialize_getHeaderSize( example->serializer ),
                     Serialize_getPayloadSize( example->serializer ),
                     Serialize_getMaxSerializeSize( example->serializer ) );
            if( Serialize_isErrorOccurred( example->serializer ) )
            {
                errorOccured = true;
            }
        }

        ANY_LOG( 5, "Test_StructArray: Written Bytes[%ld]", ANY_LOG_INFO,
                 IOChannel_getWrittenBytes( example->writer ) );

        if( IOChannel_hasPointer( example->writer ) )
        {
            memoryBuffer = (char *)IOChannel_getProperty( example->writer, (char *)"MemPointer" );
            if( memoryBuffer == NULL )
            {
                errorOccured = true;
            }
        }

        IOChannel_close( example->writer );

        if( !errorOccured )
        {
            status = IOChannel_open( example->reader, streamname.c_str(),
                                     IOCHANNEL_MODE_R_ONLY | IOCHANNEL_MODE_CLOSE,
                                     IOCHANNEL_PERMISSIONS_ALL, memoryBuffer, memoryBufferSize );
            if( !status )
            {
                errorOccured = true;
            }
            else
            {
                Serialize_setMode( example->serializer, SERIALIZE_MODE_READ );
                Serialize_setStream( example->serializer, example->reader );

                ANY_LOG( 5, "Test_StructArray: Reading Data...", ANY_LOG_INFO );

                for( i = 0; i < sizeof( formatsToTest ) / sizeof( char * ); i++ )
                {
                    memset( example->structAllToRead, '0', sizeof( StructAll ) );
                    Test_StructArray_deserialize( example );
                    ANY_LOG( 5, "Test_StructArray: Format[%s] HeaderSize[%ld] RealSerializeSize[%ld]",
                             ANY_LOG_INFO, formatsToTest[ i ],
                             Serialize_getHeaderSize( example->serializer ),
                             Serialize_getPayloadSize( example->serializer ) );
                    if( Serialize_isErrorOccurred( example->serializer ) )
                    {
                        errorOccured = true;
                    }
                }

                ANY_LOG( 5, "Test_StructArray: Read Bytes[%ld]", ANY_LOG_INFO,
                         IOChannel_getReadBytes( example->reader ) );

                IOChannel_close( example->reader );
            }
        }
    }

    Example_destroy( example );
    ANY_LOG( 5, "Test_StructArray: compare generated file with expected result", ANY_LOG_INFO );

    if( !compareFiles( "Test_StructArray", filename,
                       "Reference_StructArray.txt" ) )
    {
        errorOccured = true;
    }

    delete example;

    ANY_LOG( 1, "Test_StructArray: test done", ANY_LOG_INFO );

    CuAssertTrue( tc, !errorOccured );
}


static void Test_ExampleCreation( CuTest *tc )
{
    Example *example = new(Example);
    bool    status   = false;

    example->writer = IOChannel_new();
    ANY_REQUIRE( example->writer );
    CuAssertTrue( tc, example->writer != NULL );

    status = IOChannel_init( example->writer );
    CuAssertTrue( tc, status );

    example->reader = IOChannel_new();
    CuAssertTrue( tc, example->reader != NULL );

    status = IOChannel_init( example->reader );
    CuAssertTrue( tc, status );

    example->structAllToWrite = StructAll_new();
    CuAssertTrue( tc, example->structAllToWrite != NULL );

    status = StructAll_init( example->structAllToWrite );
    CuAssertTrue( tc, status );

    example->structAllToRead = StructAll_new();
    CuAssertTrue( tc, example->structAllToRead != NULL );

    status = StructAll_init( example->structAllToRead );
    CuAssertTrue( tc, status );

    example->serializer = Serialize_new();
    CuAssertTrue( tc, example->serializer != NULL );

    status = Serialize_init( example->serializer, (IOChannel *)NULL, SERIALIZE_STREAMMODE_NORMAL );
    CuAssertTrue( tc, status );

    Example_destroy( example );

    delete example;
}


StructAll *StructAll_new( void )
{
    StructAll *self = (StructAll *)NULL;
    self = ANY_TALLOC( StructAll );
    ANY_REQUIRE( self );
    return self;
}


bool StructAll_init( StructAll *self )
{
    BaseStructAll *baseStructAll;
    SubStructAll  *tmp;
    int           i = 0;

    ANY_REQUIRE( self );

    ALLTYPES_INIT( self );

    ARRAY_INIT( self, char, ch );
    ARRAY_INIT( self, signed char, sch );
    ARRAY_INIT( self, unsigned char, uch );
    ARRAY_INIT( self, short int, si );
    ARRAY_INIT( self, unsigned short int, usi );
    ARRAY_INIT( self, int, i );
    ARRAY_INIT( self, long int, li );
    ARRAY_INIT( self, int, i );
    ARRAY_INIT( self, unsigned int, ui );
    ARRAY_INIT( self, long int, li );
    ARRAY_INIT( self, unsigned long int, uli );
    ARRAY_INIT( self, long long, ll );
    ARRAY_INIT( self, unsigned long long, ull );
    ARRAY_INIT( self, float, f );

    sprintf( self->string, "%s", "quotedString" );

    tmp = &self->subStructure;
    ALLTYPES_INIT( tmp );

    baseStructAll = &tmp->baseStructAll;
    ALLTYPES_INIT( baseStructAll );


    for( i = 0; i < SIZEOFARRAYS; ++i )
    {
        tmp = &self->subStructureArray[ i ];
        ALLTYPES_INIT( tmp );

        baseStructAll = &tmp->baseStructAll;
        ALLTYPES_INIT( baseStructAll );
    }

    return true;
}


bool StructAll_isEqual( StructAll *s1, StructAll *s2 )
{
    bool         retVal = true;
    SubStructAll *tmp1, *tmp2;
    int          i      = 0;

    ANY_REQUIRE( s1 );
    ANY_REQUIRE( s2 );

    ALLTYPES_ISEQUAL( s1, s2 );

    ARRAY_ISEQUAL( s1, s2, ch );
    ARRAY_ISEQUAL( s1, s2, sch );
    ARRAY_ISEQUAL( s1, s2, uch );
    ARRAY_ISEQUAL( s1, s2, si );
    ARRAY_ISEQUAL( s1, s2, usi );
    ARRAY_ISEQUAL( s1, s2, i );
    ARRAY_ISEQUAL( s1, s2, li );
    ARRAY_ISEQUAL( s1, s2, i );
    ARRAY_ISEQUAL( s1, s2, ui );
    ARRAY_ISEQUAL( s1, s2, li );
    ARRAY_ISEQUAL( s1, s2, uli );
    ARRAY_ISEQUAL( s1, s2, ll );
    ARRAY_ISEQUAL( s1, s2, ull );

    if( strcmp( s1->string, s2->string ) != 0 )
    {
        return false;
    }

    tmp1 = &s1->subStructure;
    tmp2 = &s2->subStructure;

    ALLTYPES_ISEQUAL( tmp1, tmp2 );

    for( i = 0; i < SIZEOFARRAYS; i++ )
    {
        tmp1 = &s1->subStructureArray[ i ];
        tmp2 = &s2->subStructureArray[ i ];
        ALLTYPES_ISEQUAL( tmp1, tmp2 );
    }
    return retVal;
}


void StructAll_setTestValues( StructAll *self )
{
    ANY_REQUIRE( self );
}


void StructAll_clear( StructAll *self )
{
    ANY_REQUIRE( self );

    memset( self, '\0', sizeof( StructAll ) );
}


void StructAll_delete( StructAll *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


void BaseStructAll_serialize( BaseStructAll *self, char *name, Serialize *s )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char *)"BaseStructAll" );
    Char_serialize( &( self->ch ), (char *)"ch", s );
    SChar_serialize( &( self->sch ), (char *)"sch", s );
    UChar_serialize( &( self->uch ), (char *)"uch", s );
    SInt_serialize( &( self->si ), (char *)"si", s );
    USInt_serialize( &( self->usi ), (char *)"usi", s );
    Int_serialize( &( self->i ), (char *)"i", s );
    UInt_serialize( &( self->ui ), (char *)"ui", s );
    LInt_serialize( &( self->li ), (char *)"li", s );
    ULInt_serialize( &( self->uli ), (char *)"uli", s );
    LL_serialize( &( self->ll ), (char *)"ll", s );
    ULL_serialize( &( self->ull ), (char *)"ull", s );
    Float_serialize( &( self->f ), (char *)"f", s );
    Serialize_endType( s );
}


static void SubStructAll_serialize( SubStructAll *self, char *name, Serialize *s )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char *)"SubStructAll" );
    Char_serialize( &( self->ch ), (char *)"ch", s );
    SChar_serialize( &( self->sch ), (char *)"sch", s );
    UChar_serialize( &( self->uch ), (char *)"uch", s );
    SInt_serialize( &( self->si ), (char *)"si", s );
    USInt_serialize( &( self->usi ), (char *)"usi", s );
    Int_serialize( &( self->i ), (char *)"i", s );
    UInt_serialize( &( self->ui ), (char *)"ui", s );
    LInt_serialize( &( self->li ), (char *)"li", s );
    ULInt_serialize( &( self->uli ), (char *)"uli", s );
    LL_serialize( &( self->ll ), (char *)"ll", s );
    ULL_serialize( &( self->ull ), (char *)"ull", s );
    Float_serialize( &( self->f ), (char *)"f", s );
    BaseStructAll_serialize( &( self->baseStructAll ), (char *)"baseStructAll", s );
    Serialize_endType( s );
}


static void StructAll_serialize( StructAll *self, const char *name, Serialize *s )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char *)"StructAll" );

    Char_serialize( &( self->ch ), (char *)"ch", s );
    SChar_serialize( &( self->sch ), (char *)"sch", s );
    UChar_serialize( &( self->uch ), (char *)"uch", s );
    SInt_serialize( &( self->si ), (char *)"si", s );
    USInt_serialize( &( self->usi ), (char *)"usi", s );
    Int_serialize( &( self->i ), (char *)"i", s );
    UInt_serialize( &( self->ui ), (char *)"ui", s );
    LInt_serialize( &( self->li ), (char *)"li", s );
    ULInt_serialize( &( self->uli ), (char *)"uli", s );
    LL_serialize( &( self->ll ), (char *)"ll", s );
    ULL_serialize( &( self->ull ), (char *)"ull", s );
    Float_serialize( &( self->f ), (char *)"f", s );
    CharArray_serialize( self->chArray, (char *)"chArray", 10, s );
    SCharArray_serialize( self->schArray, (char *)"schArray", 10, s );
    UCharArray_serialize( self->uchArray, (char *)"uchArray", 10, s );
    SIntArray_serialize( self->siArray, (char *)"siArray", 10, s );
    USIntArray_serialize( self->usiArray, (char *)"usiArray", 10, s );
    IntArray_serialize( self->iArray, (char *)"iArray", 10, s );
    UIntArray_serialize( self->uiArray, (char *)"uiArray", 10, s );
    LIntArray_serialize( self->liArray, (char *)"liArray", 10, s );
    ULIntArray_serialize( self->uliArray, (char *)"uliArray", 10, s );
    LLArray_serialize( self->llArray, (char *)"llArray", 10, s );
    ULLArray_serialize( self->ullArray, (char *)"ullArray", 10, s );
    FloatArray_serialize( self->fArray, (char *)"fArray", 10, s );
    String_serialize( self->string, (char *)"string", ( Any_strlen( "quotedString" ) + 1 ), s );
    SubStructAll_serialize( &( self->subStructure ), (char *)"subStructure", s );
    STRUCT_ARRAY_SERIALIZE( self->subStructureArray,
                            (char *)"subStructureArray",
                            (char *)"SubStructAll",
                            SubStructAll_serialize, 10, s );

    Serialize_endType( s );
}


static void usage( ArgvParser *argvParser )
{
    ANY_LOG( 3, "Usage: ./ExampleRVP [-s [string]]\n", ANY_LOG_INFO );

    ANY_LOG( 3, "Valid options:\n", ANY_LOG_INFO );
    ArgvParser_displayOptionHelp( argvParser, 2 );
}


static bool ExampleRVP_parseArgs( ArgvParser argvParser, int argc, char *argv[], char *string )
{
    const char *argument = NULL;
    bool       retVal    = true;
    char       *str      = (char *)NULL;

    if( ArgvParser_initAndSetup( &argvParser,
                                 argc,
                                 argv,
                                 optionDescriptors ) != 0 )
    {
        ANY_LOG( 3, "Error while initializing ArgvParser.", ANY_LOG_ERROR );
        return false;
    }

    do
    {
        int        optIdx     = 0;
        const char *parameter = NULL;

        optIdx = ArgvParser_getCurrentArgument( &argvParser,
                                                NULL,
                                                NULL,
                                                &parameter );

        switch( optIdx )
        {
            case ARGVPARSER_NO_OPTION:
                if( argument == NULL )
                {
                    argument = parameter;
                }
                else
                {
                    ANY_LOG( 3, "Too many arguments: %s\n\n", ANY_LOG_INFO, parameter );
                    usage( &argvParser );
                    retVal = false;
                }
                break;

            case OPT_HELP:
                usage( &argvParser );
                retVal = false;
                break;

            case OPT_STRING:
                str = ANY_NTALLOC( EXAMPLERVP_PARAM_MAXLEN, char );
                Any_strncpy( str, parameter, EXAMPLERVP_PARAM_MAXLEN );
                string = str;
                ANY_REQUIRE( string );
                break;
        }
    }
    while( ArgvParser_advance( &argvParser ) && retVal );

    if( ArgvParser_hasErrorOccurred( &argvParser ) )
    {
        ANY_LOG( 3, "Error in command line: %s\n\n", ANY_LOG_ERROR,
                 ArgvParser_getErrorMessage( &argvParser ) );

        usage( &argvParser );

        retVal = false;
    }

    return retVal;
}


static void ExampleRVP_printElement( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    ANY_LOG( 5, "Test_parseReferences: List address: %p", ANY_LOG_INFO, (void *)self );
    ANY_LOG( 5, "Test_parseReferences: Reference:    %s", ANY_LOG_INFO, self->reference );
    ANY_LOG( 5, "Test_parseReferences: Value:        %s", ANY_LOG_INFO, self->value );
    ANY_LOG( 5, "Test_parseReferences: Next:         %p", ANY_LOG_INFO, (void *)self->next );
}


static void ExampleRVP_printList( SerializeReferenceValue *self )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;

    ANY_REQUIRE( self );

    current = self;

    while( current != NULL )
    {
        ExampleRVP_printElement( current );
        ANY_LOG( 5, "Test_parseReferences: -----------------------", ANY_LOG_INFO );

        current = current->next;
    }
}


static void Example_create( Example *example )
{
    bool status = false;

    ANY_REQUIRE( example );

    example->writer = IOChannel_new();
    ANY_REQUIRE( example->writer );

    status = IOChannel_init( example->writer );
    ANY_REQUIRE_MSG( status, "IOChannel_init for writer failed!" );

    example->reader = IOChannel_new();
    ANY_REQUIRE( example->reader );

    status = IOChannel_init( example->reader );
    ANY_REQUIRE_MSG( status, "IOChannel_init for reader failed!" );

    example->structAllToWrite = StructAll_new();
    ANY_REQUIRE( example->structAllToWrite );

    status = StructAll_init( example->structAllToWrite );
    ANY_REQUIRE_MSG( status, "StructAll_init for data failed!" );

    example->structAllToRead = StructAll_new();
    ANY_REQUIRE( example->structAllToRead );

    status = StructAll_init( example->structAllToRead );
    ANY_REQUIRE_MSG( status, "StructAll_init for data failed!" );

    example->serializer = Serialize_new();
    ANY_REQUIRE( example->serializer );

    status = Serialize_init( example->serializer, (IOChannel *)NULL, SERIALIZE_STREAMMODE_NORMAL );
    ANY_REQUIRE_MSG( status, "Serialize_init failed!" );
}


static void Example_destroy( Example *example )
{
    ANY_REQUIRE( example );

    IOChannel_clear( example->writer );
    IOChannel_delete( example->writer );

    IOChannel_clear( example->reader );
    IOChannel_delete( example->reader );

    StructAll_clear( example->structAllToWrite );
    StructAll_delete( example->structAllToWrite );

    Serialize_clear( example->serializer );
    Serialize_delete( example->serializer );

    ANY_FREE( example->structAllToRead );
}


static void Test_StructArray_serialize( Example *example )
{
    Serialize_beginType( example->serializer, "arrayOfStructures", (char *)"ArrayOfStructures" );

    STRUCT_ARRAY_SERIALIZE( example->structAllToWrite->subStructureArray,
                            (char *)"subStructureArray",
                            (char *)"SubStructAll",
                            SubStructAll_serialize, 10,
                            example->serializer );

    Serialize_endType( example->serializer );
}


static void Test_StructArray_deserialize( Example *example )
{
    Serialize_beginType( example->serializer, "arrayOfStructures", (char *)"ArrayOfStructures" );

    STRUCT_ARRAY_SERIALIZE( example->structAllToRead->subStructureArray,
                            (char *)"subStructureArray",
                            (char *)"SubStructAll",
                            SubStructAll_serialize, 10,
                            example->serializer );
    Serialize_endType( example->serializer );
}


static bool compareFiles( const char *testName, const char *file1, const char *file2 )
{
    unsigned int  i;
    unsigned char buf1[1024], buf2[1024];
    long int      totsize1 = 0;
    long int      totsize2 = 0;

    std::ifstream f1( file1, std::ifstream::in | std::ifstream::binary );
    if( !f1 )
    {
        ANY_LOG( 5, "%s: Cannot open file [%s]", ANY_LOG_FATAL, testName, file1 );
        return false;
    }

    std::ifstream f2( file2, std::ifstream::in | std::ifstream::binary );
    if( !f2 )
    {
        ANY_LOG( 5, "%s: Cannot open file [%s]", ANY_LOG_FATAL, testName, file2 );
        return false;
    }

    ANY_LOG( 5, "%s: Comparing files [%s] and [%s]...", ANY_LOG_INFO, testName, file1, file2 );

    do
    {
        f1.read( (char *)buf1, sizeof buf1 );
        f2.read( (char *)buf2, sizeof buf2 );

        totsize1 += f1.gcount();
        totsize2 += f2.gcount();
        if( totsize1 != totsize2 )
        {
            ANY_LOG( 5, "%s: Files do not have the same size [%ld] vs [%ld]", ANY_LOG_WARNING, testName, totsize1,
                     totsize2 );
            f1.close();
            f2.close();
            return false;
        }

        for( i = 0; i < f1.gcount(); i++ )
        {
            if( buf1[ i ] != buf2[ i ] )
            {
                ANY_LOG( 5, "%s: Files do not have the same content at position %d", ANY_LOG_INFO, testName, i );
                f1.close();
                f2.close();
                return false;
            }
        }
    }
    while( !f1.eof() && !f2.eof() );

    f1.close();
    f2.close();

    ANY_LOG( 5, "%s: Files are identical", ANY_LOG_INFO, testName );

    return true;
}


int main( void )
{
    CuSuite  *suite   = CuSuiteNew();
    CuString *output  = CuStringNew();
    char     *verbose = (char *)NULL;
    int       result  = EXIT_FAILURE;

    ANY_REQUIRE( suite );
    ANY_REQUIRE( output );

    verbose = getenv( (char *)"VERBOSE" );
    if( verbose != NULL && Any_strcmp( verbose, (char *)"TRUE" ) == 0 )
    {
        Any_setDebugLevel( 10 );
    }
    else
    {
        Any_setDebugLevel( 1 );
    }

    SUITE_ADD_TEST( suite, Test_BBDMSerialize );
    SUITE_ADD_TEST( suite, Test_calcsize );
    SUITE_ADD_TEST( suite, Test_initmode );
    SUITE_ADD_TEST( suite, Test_parseReferences );
    SUITE_ADD_TEST( suite, Test_ExampleCreation );
    SUITE_ADD_TEST( suite, Test_WriteReadAllFormats );
    SUITE_ADD_TEST( suite, Test_CalcSizeAllFormats );
    SUITE_ADD_TEST( suite, Test_LoopData );
    SUITE_ADD_TEST( suite, Test_FlushData );
    SUITE_ADD_TEST( suite, Test_MatlabEvalCode );
    SUITE_ADD_TEST( suite, Test_NoBeginType );
    SUITE_ADD_TEST( suite, Test_MemoryStream );
    SUITE_ADD_TEST( suite, Test_StructArray );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    CuSuiteDetails( suite, output );

    Any_fprintf( stderr, "%s\n", output->buffer );

    result = suite->failCount;

    CuSuiteDelete( suite );
    CuStringDelete( output );

    return result;
}


/* EOF */
