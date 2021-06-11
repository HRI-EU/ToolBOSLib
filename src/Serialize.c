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


#if defined(__GNUC__)

#pragma GCC diagnostic push

/* ISO C forbids conversion of function pointer to object pointer type */
#pragma GCC diagnostic ignored "-Wpedantic"

/* some API parameters unused but kept for polymorphism */
#pragma GCC diagnostic ignored "-Wunused-parameter"

#endif


#include <Serialize.h>


#define SERIALIZE_VALID                           (0xc1d3adcc)
#define SERIALIZE_INVALID                         (0x89b72feb)


static void Serialize_partialReset( Serialize *self );

static void Serialize_resetSerialize( Serialize *self );

static bool Serialize_checkModes( Serialize *self );

static long long Serialize_getStreamPosition( Serialize *self );

static bool Serialize_setStreamModeFromModes( Serialize *self,
                                              int modes );

static bool Serialize_setDirectionFromModes( Serialize *self,
                                             int modes );

static bool Serialize_isTheFirstBeginTypeCall( Serialize *self );

static void Serialize_doFirstBeginTypeCallOps( Serialize *self,
                                               const char *name,
                                               const char *type );

static bool Serialize_isTheLastEndTypeCall( Serialize *self );

static void Serialize_doLastEndTypeCallOps( Serialize *self );

static void Serialize_doAutoCalcSizeOps( Serialize *self );

static long Serialize_getTypeMaxSizeAsAscii( SerializeType type );

static void Serialize_fireEventInfo( Serialize *self,
                                     AnyEventInfo *eventInfo );

static IOChannel *SerializeCalcStream_create( void );

static void SerializeCalcStream_destroy( Serialize *self );

static MTList *SerializeFormatList_create( void );

static bool SerializeFormatList_addFormat( Serialize *self,
                                           const char *format,
                                           const char *path );

static bool SerializeFormatList_setPluginLibHandle( Serialize *self,
                                                    SerializeFormatInfo *info,
                                                    DynamicLoader *libraryHandle );

static SerializeFormatInfo *SerializeFormatList_find( Serialize *self,
                                                      const char *format );

static SerializeFormat *SerializeFormat_findStaticFormat( const char *formatName );

static void SerializeFormatList_destroy( Serialize *self );

static SerializeHeader *SerializeHeader_create( void );

static SerializeReferenceValue *SerializeHeader_getReferenceValue( SerializeHeader *self,
                                                                   char *ref );

static void SerializeHeader_setInfo( Serialize *self,
                                     const char *type,
                                     const char *name,
                                     const char *opts,
                                     const char *format,
                                     const long objSize );

static void SerializeHeader_useParserV10( Serialize *self,
                                          const char *name,
                                          const char *type );

static void SerializeHeader_useParserV20( Serialize *self,
                                          const char *name,
                                          const char *type );

static void SerializeHeader_updateHeaderSize( Serialize *self );

static void SerializeHeader_destroy( Serialize *self );


#define SERIALIZE_REQUIRE_STRING( __string )                            \
  ANY_REQUIRE_MSG(  __string, "Null string pointer!" #__string );       \
  ANY_REQUIRE_MSG( *__string, "Empty string!" #__string )


#define SERIALIZE_SKIPIFERROR_START( __self )                           \
  do                                                                    \
  {                                                                     \
  if ( IOChannel_eof( __self->stream ) == false )                       \
  {                                                                     \
    if ( __self->errorOccurred == true )                                \
    {                                                                   \
      ANY_LOG( 0, "Serialization error occurred!", ANY_LOG_ERROR );     \
      /* Uncomment To Stop At First Error */                            \
      /* ANY_REQUIRE( NULL ); */                                        \
      /* break; */                                                      \
      if ( __self->recoveryJmpSet == true )                             \
      {                                                                 \
        __self->recoveryJmpSet = false;                                 \
        longjmp( __self->recoveryJmp, 1 );                              \
      }                                                                 \
      else                                                              \
      {                                                                 \
        ANY_LOG( 0, "Unable to longjmp() because it wasn't set before", ANY_LOG_ERROR ); \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    ANY_LOG( 3, "EOF Found in the stream! Skipping function...", ANY_LOG_WARNING ); \
    if ( __self->recoveryJmpSet == true )                               \
    {                                                                   \
      __self->recoveryJmpSet = false;                                   \
      longjmp( __self->recoveryJmp, 2 );                                \
    }                                                                   \
    else                                                                \
    {                                                                   \
      ANY_LOG( 0, "Unable to longjmp() because it wasn't set before", ANY_LOG_ERROR ); \
    }                                                                   \
  }


#define  SERIALIZE_SKIPIFERROR_END                                      \
  } while ( 0 )


/*---------------------------------------------------------------------------*/
/* Available serialization formats                                           */
/*---------------------------------------------------------------------------*/


extern SERIALIZEFORMAT_DECLARE_OPTIONS( Binary );
extern SERIALIZEFORMAT_DECLARE_OPTIONS( Ascii );
extern SERIALIZEFORMAT_DECLARE_OPTIONS( Matlab );
extern SERIALIZEFORMAT_DECLARE_OPTIONS( Python );
extern SERIALIZEFORMAT_DECLARE_OPTIONS( Xml );
extern SERIALIZEFORMAT_DECLARE_OPTIONS( Json );


static SerializeFormat *Serialize_internalFormats[] =
        {
                &SERIALIZEFORMAT_OPTIONS( Binary ),
                &SERIALIZEFORMAT_OPTIONS( Ascii ),
                &SERIALIZEFORMAT_OPTIONS( Matlab ),
                &SERIALIZEFORMAT_OPTIONS( Python ),
                &SERIALIZEFORMAT_OPTIONS( Xml ),
                &SERIALIZEFORMAT_OPTIONS( Json ),

                /* termination */
                NULL
        };


/*---------------------------------------------------------------------------*/
/* Public functions                                                          */
/*---------------------------------------------------------------------------*/


Serialize *Serialize_new( void )
{
    Serialize *self = (Serialize *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_new" );

    self = ANY_TALLOC( Serialize );
    ANY_REQUIRE( self );
    return self;
}


bool Serialize_init( Serialize *self, IOChannel *stream, int modes )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_init" );

    ANY_REQUIRE( self );

    Serialize_resetSerialize( self );

    /* Setting host endianess */
    self->isLittleEndian = Serialize_isLittleEndian();

    /* Simply a malloc and fields initialization */
    self->header = SerializeHeader_create();
    ANY_REQUIRE_MSG( self->header, "Can't allocate memory for header" );

    Serialize_setHeaderSizes( self, 0, 0, 0, 0 );

    /* New and Init for the reserved stream for CalcSize */
    self->calcSizeStream = SerializeCalcStream_create();
    if( self->calcSizeStream == (IOChannel *)NULL)
    {
        ANY_LOG( 0, "Unable To allocate Stream For calcsize!", ANY_LOG_ERROR );
        SerializeHeader_destroy( self );
        self->errorOccurred = true;
        goto exitLabel;
    }

    /* New And Init Of The MTList For Formats */
    self->formatList = SerializeFormatList_create();
    if( self->formatList == (MTList *)NULL)
    {
        ANY_LOG( 0, "Unable To allocate List For Formats!", ANY_LOG_ERROR );
        SerializeCalcStream_destroy( self );
        SerializeHeader_destroy( self );
        self->errorOccurred = true;
        goto exitLabel;
    }

    /* Setting Object validity */
    self->valid = SERIALIZE_VALID;

    if( stream != (IOChannel *)NULL)
    {
        Serialize_setStream( self, stream );
    }
    retVal = true;

    Serialize_setMode( self, modes );

    ANY_LOG( 7, "--- Serialize_init() Success! ---", ANY_LOG_INFO );

    exitLabel:;
    return retVal;
}


bool Serialize_isFormatDefined( Serialize *self, char *format )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isFormatDefined" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    if( SerializeFormatList_find( self, format ) != NULL)
    {
        retVal = true;
    }

    return retVal;
}


bool Serialize_addFormat( Serialize *self, SerializeFormat *plugin )
{
    SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
    SerializeFormat *formatPtr = (SerializeFormat *)NULL;
    SerializeFormatInfo *tmp = (SerializeFormatInfo *)NULL;
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_addFormat" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE( plugin );

    /* Allocate Dat Block For Plugin */
    infoPtr = ANY_TALLOC( SerializeFormatInfo );
    ANY_REQUIRE( infoPtr );

    /* Save Temporary the Current Format To Call The Option Api. Not  */
    /* very nice, but actually Macros deference only the current set  */
    /* format. do not check ptr, ( maybe no format was prev set ) */
    tmp = self->format;

    self->format = infoPtr;
    ANY_REQUIRE( self->format );

    infoPtr->ops = plugin;
    ANY_REQUIRE( infoPtr->ops );

    formatPtr = infoPtr->ops;
    ANY_REQUIRE( formatPtr );

    infoPtr->data = SERIALIZEFORMATOPTIONS_NEW( self );

    if( infoPtr->data != NULL)
    {
        SERIALIZEFORMATOPTIONS_INIT( self );
    }
    else
    {
        ANY_LOG( 7, "Format[%s] has no options", ANY_LOG_INFO,
                 formatPtr->formatName );
    }

    /* Let Put The Plugin into the MTList */
    retVal = MTList_insert( self->formatList, infoPtr );

    if( retVal == false )
    {
        ANY_LOG( 7, "Unable to add format to the internal list!( MTList_insert returned false )",
                 ANY_LOG_ERROR );
        self->errorOccurred = true;
    }
    else
    {
        /* Ok, All went Right.. */
    }

    /* Restore The Current Format */
    self->format = tmp;

    return retVal;
}


bool Serialize_setFormat( Serialize *self,
                          const char *format,
                          const char *options )
{
    SerializeFormatInfo *ptr = (SerializeFormatInfo *)NULL;
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setFormat" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    SERIALIZE_REQUIRE_STRING( format );

    ptr = SerializeFormatList_find( self, format );

    if( ptr == (SerializeFormatInfo *)NULL)
    {
        if( SerializeFormatList_addFormat( self, format, NULL) == false )
        {
            ANY_LOG( 0, "[%s]: No such serialization format",
                     ANY_LOG_ERROR, format );
            goto exitLabel;
        }

        ptr = SerializeFormatList_find( self, format );
        ANY_REQUIRE( ptr );

        ANY_LOG( 7, "%s serialization plugin loaded",
                 ANY_LOG_INFO, format );
    }

    /* All went right, set the format */
    self->format = ptr;

    /* Reset Header fields ... */
    if( Serialize_isWriting( self ))
    {
        SerializeHeader_setInfo( self, "", "", "", format, 0 );
    }

    /* Reset/set the data format using the options as reference */
    SERIALIZEFORMATOPTIONS_SET( self, options );

    retVal = true;

    exitLabel:;
    return retVal;
}


bool Serialize_setFormatProperty( Serialize *self, char *optName, void *opt )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setFormatProperty" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( optName );

    SERIALIZE_SKIPIFERROR_START( self )

        retVal = SERIALIZEFORMATOPTIONS_SETPROPERTY( self, optName, opt );

    SERIALIZE_SKIPIFERROR_END;

    return retVal;
}


void *Serialize_getFormatProperty( Serialize *self, char *optName )
{
    void *retVal = (void *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getFormatProperty" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( optName );

    SERIALIZE_SKIPIFERROR_START( self )

        retVal = SERIALIZEFORMATOPTIONS_GETPROPERTY( self, optName );

    SERIALIZE_SKIPIFERROR_END;

    return retVal;
}


void Serialize_setMode( Serialize *self, int modes )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_setMode" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    Serialize_setStreamModeFromModes( self, modes );

    Serialize_setDirectionFromModes( self, modes );

    self->offsetForLoop = 0;

    if( self->streamMode == SERIALIZE_STREAMMODE_LOOP )
    {
        if( self->stream != NULL)
        {
            /* Get Current Position if stream is already set */
            self->offsetForLoop = IOChannel_seek( self->stream, 0, IOCHANNELWHENCE_CUR );

            if( self->offsetForLoop == -1 )
            {

                ANY_LOG( 7, "SERIALIZE_STREAMMODE_LOOP was "
                        "specified, but seek() returned -1!",
                         ANY_LOG_ERROR );
                self->errorOccurred = true;
            }
        }
        else
        {
            ANY_LOG( 0, "SERIALIZE_STREAMMODE_LOOP was specified, but "
                    "Stream is Not Set!", ANY_LOG_WARNING );
        }
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_AUTOCALC ))
    {
        ANY_LOG( 7, "AutoCalc Mode Was Specified,", ANY_LOG_INFO );
        self->isAutoCalcSizeMode = true;
    }
    else
    {
        self->isAutoCalcSizeMode = false;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_TRANSLATE ))
    {
        ANY_LOG( 7, "Translate Mode Was Specified,", ANY_LOG_INFO );
        self->isTranslateMode = true;
    }
    else
    {
        self->isTranslateMode = false;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_NOHEADER ))
    {
        ANY_LOG( 7, "NoHeader Mode Was Specified,", ANY_LOG_INFO );
        self->useHeader = false;
    }
    else
    {
        self->useHeader = true;
    }

    if( self->mode == SERIALIZE_MODE_CALC )
    {
        self->stream = self->calcSizeStream;
    }
}


void Serialize_setStream( Serialize *self, IOChannel *stream )
{
    long initialOffset = -1;
    char *streamType = NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setStream" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE( stream );

    streamType = IOChannel_getStreamType( stream );

    ANY_LOG( 7, "Stream type is: %s", ANY_LOG_INFO, ( streamType ? streamType : "--" ));

    /* Initial Offset of the stream, maybe it has been already used.. */
    /* initialOffset = IOChannel_tell( stream ); */

    /* Not fully tested yet, so actually disabled... */
    initialOffset = 0;
    if( initialOffset < 0 )
    {
        ANY_LOG( 7, "IOChannel_tell() returned -1",
                 ANY_LOG_WARNING );
        initialOffset = 0;
    }

    Serialize_partialReset( self );

    self->stream = stream;
}


void *Serialize_getFormatDataPtr( Serialize *self )
{
    SerializeFormatInfo *ptr = (SerializeFormatInfo *)NULL;
    void *retVal = (void *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getFormatDataPtr" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    ptr = self->format;
    ANY_REQUIRE( ptr );

    retVal = ptr->data;

    return retVal;
}


IOChannel *Serialize_getStream( Serialize *self )
{
    IOChannel *retVal = (IOChannel *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getStream" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = self->stream;

    return retVal;
}


int Serialize_getBeginTypeNestingLevel( Serialize *self )
{
    int retVal = -1;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getBeginTypeNestingLevel" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = self->numTypeCalls;

    return retVal;
}


bool Serialize_isReading( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isReading" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = ( self->mode == SERIALIZE_MODE_READ ? true : false );

    return retVal;
}


bool Serialize_isWriting( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isWriting" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = (( self->mode == SERIALIZE_MODE_WRITE ||
                self->mode == SERIALIZE_MODE_CALC ) ? true : false );

    return retVal;
}


bool Serialize_isErrorOccurred( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isErrorOccurred" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = self->errorOccurred;

    return retVal;
}


bool Serialize_isEof( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isEof" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE( self->stream );

    retVal = IOChannel_eof( self->stream );

    return retVal;
}


void Serialize_cleanError( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_cleanError" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    Serialize_partialReset( self );
}


int Serialize_getColumnWrap( Serialize *self )
{
    int retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getColumnWrap" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = self->columnWrap;

    return retVal;
}


void Serialize_setColumnWrap( Serialize *self, unsigned int columnWrap )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_setColumnWrap" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    self->columnWrap = columnWrap;
}


static void SerializeHeader_updateHeaderSize( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    char buffer[SERIALIZE_HEADER_MAXLEN];
    char *type = (char *)NULL;
    char *name = (char *)NULL;
    char *format = (char *)NULL;
    char *opts = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_updateHeaderSize" );

    header = self->header;
    ANY_REQUIRE( header );

    type = SerializeReferenceValue_findValue( header->listHead, "type" );
    name = SerializeReferenceValue_findValue( header->listHead, "name" );
    format = SerializeReferenceValue_findValue( header->listHead, "format" );
    opts = SerializeReferenceValue_findValue( header->listHead, "opts" );

    ANY_REQUIRE( type != (char *)NULL );
    ANY_REQUIRE( name != (char *)NULL );
    ANY_REQUIRE( format != (char *)NULL );
    ANY_OPTIONAL( opts != (char *)NULL );   /* 'opts' might be NULL */

    if( opts )
    {
        header->headerSize = Any_snprintf( buffer,
                                           SERIALIZE_HEADER_MAXLEN - 1,
                                           SERIALIZE_HEADER_PREAMBLE
                                                   "%d.%d type = '%s' name = %s objSize = %10d format = %s opts = '%s'\n",
                                           header->majVersion, header->minVersion,
                                           type, name, 0, format, opts );
    }
    else
    {
        header->headerSize = Any_snprintf( buffer,
                                           SERIALIZE_HEADER_MAXLEN - 1,
                                           SERIALIZE_HEADER_PREAMBLE
                                                   "%d.%d type = '%s' name = %s objSize = %10d format = %s \n",
                                           header->majVersion, header->minVersion,
                                           type, name, 0, format );
    }
}


long Serialize_getHeaderSize( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    long retVal = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderSize" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    retVal = header->headerSize;

    return retVal;
}


long Serialize_getPayloadSize( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    long retVal = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getPayloadSize" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    retVal = header->objSize;

    return retVal;
}


long Serialize_getTotalSize( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_getPayloadSize" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    return Serialize_getHeaderSize( self ) + Serialize_getPayloadSize( self );
}


long Serialize_getMaxSerializeSize( Serialize *self )
{
    long retVal = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getMaxSerializeSize" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = Serialize_getPayloadSize( self ) + self->roundOff;

    return retVal;
}


bool Serialize_isLittleEndian( void )
{
    return Any_isLittleEndian();
}


void Serialize_setInitMode( Serialize *self, bool status )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_setInitMode" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    self->isInitMode = status;
}


bool Serialize_isInitMode( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isInitMode" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    retVal = self->isInitMode;

    return retVal;
}


void Serialize_clear( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_clear" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    SerializeHeader_destroy( self );

    SerializeCalcStream_destroy( self );

    SerializeFormatList_destroy( self );

    /* Reset Serialize Fields */
    /* TODO: Remember to enable it - 30-Jan-2012
     if ( self->onBeginSerialize )
     {
     Serialize_freeEventInfo( self->onBeginSerialize );
     }

     if ( self->onEndSerialize )
     {
     Serialize_freeEventInfo( self->onEndSerialize );
     }
  */

    Serialize_resetSerialize( self );

    ANY_LOG( 7, "Serialize_clear()", ANY_LOG_INFO );

    self->valid = SERIALIZE_INVALID;
}


void Serialize_delete( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_delete" );

    ANY_REQUIRE( self );
    ANY_FREE( self );

    ANY_LOG( 7, "Serialize_delete()", ANY_LOG_INFO );
}

/*-------------------------------------------------------------------------*/
/* Functions For Plugin Develop                                            */
/*-------------------------------------------------------------------------*/

long Serialize_printf( Serialize *self, const char *fmt, ... )
{
    long retVal = 0;
    va_list varArg;

    SERIALIZE_TRACE_FUNCTION( "Serialize_printf" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    va_start( varArg, fmt );
    retVal = IOChannel_vprintf( self->stream, (char *)fmt, varArg );
    va_end( varArg );

    return retVal;
}


long Serialize_indent( Serialize *self )
{
    char space = ' ';
    long i = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_indent" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    if( Serialize_isWriting( self ) == true )
    {
        for( i = 1; i <= self->indentLevel; ++i )
        {
            if( IOChannel_write( self->stream, &space, 1 ) != 1 )
            {
                ANY_LOG( 0, "Unable to write indentation spaces![%s]",
                         ANY_LOG_ERROR,
                         IOChannel_getErrorDescription( self->stream ));
                self->errorOccurred = true;
            }
        }
    }

    return i;
}


bool Serialize_deployDataType( Serialize *self,
                               SerializeType type,
                               SerializeDeployDataMode deployDataMode,
                               const char *spec,
                               int notYet,
                               long len,
                               void *data )
{
    long realFieldAsciiSize = 0;
    long maxFieldAsciiSize = 0;

    long rdwrBytes = 0;
    long nBytes = 0;

    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_deployDataType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    switch( self->mode )
    {
        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            switch( deployDataMode )
            {
                case SERIALIZE_DEPLOYDATAMODE_BINARY:
                {
                    /* Call IOChannel_write until all data are written */
                    while( rdwrBytes < len )
                    {
                        nBytes = IOChannel_write( self->stream, (unsigned char *)data + rdwrBytes, ( len - rdwrBytes ));
                        if( nBytes == -1 || IOChannel_eof( self->stream ) == true ||
                            IOChannel_isErrorOccurred( self->stream ) == true )
                        {
                            ANY_LOG( 0, "Unable To DEPLOY(Write) BINARY VALUE on stream: %s", ANY_LOG_ERROR,
                                     ( nBytes == -1 ) ? "-1 returned" : "EOF found or error occurred" );
                            self->errorOccurred = true;
                            break;
                        }
                        rdwrBytes += nBytes;
                    }

                    /*
            if( IOChannel_write( self->stream, data, len ) != len )
            {
            ANY_LOG( 0, "Unable To DEPLOY(Write) BINARY VALUE on stream!",
            ANY_LOG_ERROR );
            self->errorOccurred = true;
            }
          */

                }
                    break;

                case SERIALIZE_DEPLOYDATAMODE_ASCII:
                {
                    ANY_REQUIRE( spec );

                    /* Get The Max Ascii Size FOr This Type.. */
                    maxFieldAsciiSize = Serialize_getTypeMaxSizeAsAscii( type );

#if 0
                                                                                                                                            /* Test That MaxSerializeSize Really works in the rigth way */
          {
            int i = 0;
            /* Printing Data we know its real ascii size.. */
            for( i = 0; i < maxFieldAsciiSize; i++ )
            {
              if( IOChannel_write( self->stream, (char*)"0", 1 ) != 1 )
              {
                ANY_REQUIRE( NULL );
              }
            }
            realFieldAsciiSize = maxFieldAsciiSize;
          }
#else
                    /* Printing Data we know its real ascii size.. */
                    realFieldAsciiSize = IOChannel_printf( self->stream, (char *)spec, data );
#endif
                    /* If Ok, Can calc roundOff to Sum To the real seralize size: */
                    /* in this way we can obtain the max ascci serialize size */
                    if( realFieldAsciiSize <= 0 )
                    {
                        ANY_LOG( 0, "Unable To DEPLOY(Write) ASCII VALUE on stream!",
                                 ANY_LOG_ERROR );
                        self->errorOccurred = true;
                    }
                    else
                    {
                        /* Increase roundOff */
                        self->roundOff += ( maxFieldAsciiSize - realFieldAsciiSize );
                    }
                }
                    break;

                default:
                {
                    ANY_REQUIRE_MSG( NULL, "Write. Unknown SerializeDeployDataMode!" );
                }
                    break;
            }
        }
            break;

        case SERIALIZE_MODE_READ:
        {
            switch( deployDataMode )
            {
                case SERIALIZE_DEPLOYDATAMODE_BINARY:
                {
                    while( rdwrBytes < len )
                    {
                        nBytes = IOChannel_read( self->stream, (unsigned char *)data + rdwrBytes, ( len - rdwrBytes ));

                        if( nBytes == -1 || IOChannel_eof( self->stream ) == true ||
                            IOChannel_isErrorOccurred( self->stream ) == true )
                        {
                            if( IOChannel_eof( self->stream ) == false )
                            {
                                ANY_LOG( 0, "Unable To DEPLOY(while reading) BINARY VALUE on stream: %s", ANY_LOG_ERROR,
                                         ( nBytes == -1 ) ? "-1 returned" : "EOF found or error occurred" );
                                self->errorOccurred = true;
                            }
                            break;
                        }
                        rdwrBytes += nBytes;
                    }

                    /*
            long readed = IOChannel_read( self->stream, data, len );

            if( readed != len )
            {
            ANY_LOG( 0, "Unable To DEPLOY(Read) BINARY VALUE on stream, requested %ld returned %ld bytes!",
            ANY_LOG_ERROR, len, readed );
            self->errorOccurred = true;
            }
          */
                }
                    break;

                case SERIALIZE_DEPLOYDATAMODE_ASCII:
                {
                    ANY_REQUIRE( spec );

                    /* Get The Max Ascii Size FOr This Type.. */
                    maxFieldAsciiSize = Serialize_getTypeMaxSizeAsAscii( type );

                    if( IOChannel_scanf( self->stream, &realFieldAsciiSize, (char *)spec, data ) != 1 )
                    {
                        ANY_LOG( 0, "Unable To DEPLOY(Read) ASCII VALUE on stream!",
                                 ANY_LOG_ERROR );
                        self->errorOccurred = true;
                    }
                    else
                    {
                        /* Increase roundOff */
                        self->roundOff += ( maxFieldAsciiSize - realFieldAsciiSize );
                    }
                }
                    break;

                default:
                {
                    ANY_REQUIRE_MSG( NULL, "Read. Unknown SerializeDeployDataMode!" );
                }
                    break;
            }
        }
            break;

        default:
        {
            ANY_REQUIRE_MSG( NULL, "SerializeDeployDataMode: Unknown SerializeMode!" );
        }
    }

    if( self->errorOccurred == true )
    {
        if( IOChannel_eof( self->stream ) == false )
        {
            ANY_LOG( 0, "An error occurred during Serialize_deployDataType()",
                     ANY_LOG_ERROR );
        }

        retVal = false;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


long Serialize_scanf( Serialize *self, const char *fmt, ... )
{
    long retVal = 0;
    long nCh = 0;
    va_list varArg;

    SERIALIZE_TRACE_FUNCTION( "Serialize_scanf" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    va_start( varArg, fmt );
    retVal = IOChannel_vscanf( self->stream, &nCh, (char *)fmt, varArg );
    va_end( varArg );

    return retVal;
}


void Serialize_internalBeginType( Serialize *self, const char *name, const char *type )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_internalBeginType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    ANY_REQUIRE_MSG( self->stream,
                     "No valid IOChannel where to operate on has been provided, yet." );

    SERIALIZE_REQUIRE_STRING( name );
    SERIALIZE_REQUIRE_STRING( type );

    SERIALIZE_SKIPIFERROR_START( self );

        if(( Serialize_isTheFirstBeginTypeCall( self ) == true ) &&
           ( self->isTranslateMode == false ))
        {
            /* Do First Begin Call Operations */
            if( self->useHeader == true )
            {
                Serialize_doFirstBeginTypeCallOps( self, name, type );
            }

            /* If CalcSize, Prepare For Object Size Calculation */
            self->roundOff = 0;
            self->objInitialOffset = Serialize_getStreamPosition( self );
        }

        ANY_REQUIRE_MSG( self->format, "format not set" );

        /* 1)Increase the num of calls and 2)call The indirect function */
        SERIALIZEFORMAT_BEGINTYPE( self, name, type );

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_beginBaseType( Serialize *self, const char *name, const char *type )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_beginBaseType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( name );
    SERIALIZE_REQUIRE_STRING( type );

    SERIALIZE_SKIPIFERROR_START( self )

        self->baseTypeEnable = true;

        Serialize_beginType( self, name, type );

        self->baseTypeEnable = false;

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_beginArray( Serialize *self,
                           SerializeType type,
                           const char *name,
                           const int len )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_beginArray" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( name );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_BEGINARRAY( self, type, name, len );

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_beginStructArray( Serialize *self,
                                 const char *arrayName,
                                 const char *elementType,
                                 const int arrayLen )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_beginStructArray" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( arrayName );
    SERIALIZE_REQUIRE_STRING( elementType );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_BEGINSTRUCTARRAY( self, arrayName, elementType, arrayLen );

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_beginStructArraySeparator( Serialize *self,
                                          const char *arrayName,
                                          const int arrayPosition,
                                          const int arrayLen )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_beginStructArraySeparator" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( arrayName );

    ANY_REQUIRE( arrayPosition >= 0 );
    ANY_REQUIRE( arrayLen >= 0 && arrayLen > arrayPosition );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_BEGINSTRUCTARRAYSEPARATOR( self, arrayName,
                                                   arrayPosition, arrayLen );
    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_doSerialize( Serialize *self,
                            SerializeType type,
                            const char *name,
                            void *value,
                            size_t size,
                            const int len )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_doSerialize" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( name );

    SERIALIZE_SKIPIFERROR_START( self );

        /* Strings are not treated as arrays: leave */
        /* doSerialize choose how to treat it */
        //if( ( len > 1 ) && ( type != SERIALIZE_TYPE_STRING ) )
        if( SERIALIZE_IS_ARRAY_ELEMENT( type ))
        {
            Serialize_beginArray( self, type, name, len );
        }

        SERIALIZEFORMAT_DOSERIALIZE( self, type, name, value, size, len );

        //if( ( len > 1 ) && ( type != SERIALIZE_TYPE_STRING ) )
        if( SERIALIZE_IS_ARRAY_ELEMENT( type ))
        {
            Serialize_endArray( self, type, name, len );
        }

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_endStructArraySeparator( Serialize *self,
                                        const char *arrayName,
                                        const int arrayPosition,
                                        const int arrayLen )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_endStructArraySeparator" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( arrayName );

    ANY_REQUIRE( arrayPosition >= 0 );
    ANY_REQUIRE( arrayLen >= 0 && arrayLen > arrayPosition );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_ENDSTRUCTARRAYSEPARATOR( self, arrayName,
                                                 arrayPosition, arrayLen );
    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_endStructArray( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_endStructArray" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_ENDSTRUCTARRAY( self );

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_endArray( Serialize *self,
                         SerializeType type,
                         const char *name,
                         const int len )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_endArray" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    SERIALIZE_REQUIRE_STRING( name );

    SERIALIZE_SKIPIFERROR_START( self )

        SERIALIZEFORMAT_ENDARRAY( self, type, name, len );

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_endBaseType( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_endBaseType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    ANY_REQUIRE( self->numTypeCalls >= 0 );

    SERIALIZE_SKIPIFERROR_START( self )

        self->baseTypeEnable = true;

        Serialize_internalEndType( self );

        self->baseTypeEnable = false;

    SERIALIZE_SKIPIFERROR_END;
}


void Serialize_internalEndType( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_internalEndType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );
    ANY_REQUIRE_MSG( self->format, "format not set" );

    ANY_REQUIRE( self->numTypeCalls >= 0 );

    SERIALIZE_SKIPIFERROR_START( self )

        /* 1)Call The indirect function and 2)decrease the num of calls */
        SERIALIZEFORMAT_ENDTYPE( self );

        if(( Serialize_isTheLastEndTypeCall( self ) == true ) &&
           ( self->isTranslateMode == false ))
        {
            long long objectFinalOffset = 0;

            objectFinalOffset = Serialize_getStreamPosition( self );

            if( self->isTranslateMode == false )
            {
                long size = 0;

                size = Serialize_getPayloadSize( self );

                if( size == 0 )
                {
                    long objSize = 0;

                    ANY_REQUIRE( objectFinalOffset >= self->objInitialOffset );

                    objSize = objectFinalOffset - self->objInitialOffset;

                    SerializeHeader_setInfo( self, NULL, NULL, NULL, NULL, objSize );
                }
            }

            if( self->useHeader == true )
            {
                if(( self->isAutoCalcSizeMode == true ) &&
                   ( self->mode == SERIALIZE_MODE_WRITE ))
                {
                    Serialize_doAutoCalcSizeOps( self );
                }
            }
            Serialize_doLastEndTypeCallOps( self );
        }
    SERIALIZE_SKIPIFERROR_END;
}

/* Header Get Data */

bool Serialize_getHeader( Serialize *self, char *buffToStoreResult, int bufSize )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    char *type = (char *)NULL;
    char *name = (char *)NULL;
    char *opts = (char *)NULL;
    char *format = (char *)NULL;
    int status = 0;
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeader" );

    header = self->header;
    ANY_REQUIRE( header );
    ANY_REQUIRE( bufSize );

    type = SerializeReferenceValue_findValue( header->listHead, "type" );
    name = SerializeReferenceValue_findValue( header->listHead, "name" );
    format = SerializeReferenceValue_findValue( header->listHead, "format" );
    opts = SerializeReferenceValue_findValue( header->listHead, "opts" );

    ANY_REQUIRE( type != (char *)NULL );
    ANY_REQUIRE( name != (char *)NULL );
    ANY_REQUIRE( format != (char *)NULL );
    ANY_REQUIRE( opts != (char *)NULL );

    SERIALIZE_START_HEADER_SWITCH( header ) ;

            SERIALIZE_START_HANDLE_VERSION( 1, 0 )
                {
                    status = Any_snprintf( buffToStoreResult, bufSize,
                                           SERIALIZE_HEADER_PREAMBLE
                                                   "%d.%d %s %s %10ld %s %s",
                                           SERIALIZE_HEADER_MAJVERSIONDEFAULT, // 1
                                           SERIALIZE_HEADER_MINVERSIONDEFAULT, // 0
                                           type, name, header->objSize, format, opts );
                }
            SERIALIZE_STOP_HANDLE_VERSION();

            SERIALIZE_START_HANDLE_VERSION( 2, 0 )
                {
                    status = Any_snprintf( buffToStoreResult, bufSize,
                                           SERIALIZE_HEADER_PREAMBLE
                                                   "%d.%d type = \'%s\' name = %s objSize = %10ld format = \'%s\' opts = \'%s\'",
                                           SERIALIZE_HEADER_MAJVERSIONDEFAULT, // 2
                                           SERIALIZE_HEADER_MINVERSIONDEFAULT, // 0
                                           type, name, header->objSize, format, opts );

                }
            SERIALIZE_STOP_HANDLE_VERSION();

            SERIALIZE_STOP_HEADER_SWITCH();

    if( status < 0 )
    {
        ANY_LOG( 0, "Error occurred while writing the header to string.", ANY_LOG_ERROR );
        return retVal;
    }
    else if( status >= bufSize )
    {
        ANY_LOG( 0, "Warning: header string was truncated.", ANY_LOG_WARNING );
        // We still return false
        return retVal;
    }
    else
    {
        retVal = true;
        return retVal;
    }
}


void Serialize_getHeaderType( Serialize *self, char *buffToStoreResult )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *readType = (char *)NULL;
    long typeLen = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderType" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "type" );
    if( !rvp )
    {
        ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
        self->errorOccurred = true;
        return;
    }
    else
    {
        readType = SerializeReferenceValue_getValue( rvp );
        typeLen = rvp->valueLen;
        Any_strncpy( buffToStoreResult, readType, typeLen );
    }
}


char *Serialize_getHeaderTypePtr( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    char *retVal = (char *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderTypePtr" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    retVal = SerializeReferenceValue_findValue( header->listHead, "type" );
    if( !retVal )
    {
        ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
    }

    return retVal;
}


void Serialize_getHeaderName( Serialize *self, char *buffToStoreResult )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *readName = (char *)NULL;
    long nameLen = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderName" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "name" );
    if( !rvp )
    {
        ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
        self->errorOccurred = true;
        return;
    }
    else
    {
        readName = SerializeReferenceValue_getValue( rvp );
        nameLen = SerializeReferenceValue_getValueLen( rvp );
        Any_strncpy( buffToStoreResult, readName, nameLen );
    }
}


char *Serialize_getHeaderNamePtr( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    char *retVal = (char *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderNamePtr" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    retVal = SerializeReferenceValue_findValue( header->listHead, "name" );
    if( !retVal )
    {
        ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
    }

    return retVal;
}


void Serialize_getHeaderOpts( Serialize *self, char *buffToStoreResult )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *readOpts = (char *)NULL;
    long optsLen = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderOpts" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "name" );
    if( !rvp )
    {
        ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
        self->errorOccurred = true;
        return;
    }
    else
    {
        readOpts = SerializeReferenceValue_getValue( rvp );
        optsLen = SerializeReferenceValue_getValueLen( rvp );
        Any_strncpy( buffToStoreResult, readOpts, optsLen );
    }
}


char *Serialize_getHeaderOptsPtr( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *retVal = (char *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getHeaderOptsPtr" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    header = self->header;
    ANY_REQUIRE( header );

    rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "opts" );

    if( rvp == NULL)
    {
        rvp = SerializeReferenceValue_new();
        SerializeReferenceValue_init( rvp, "opts", NULL);
        /* SerializeReferenceValue_append( header->listHead, &header->listTail, rvp ); */
        SerializeReferenceValue_push( &header->listHead, rvp );
    }

    retVal = SerializeReferenceValue_getValue( rvp );

    return retVal;
}


bool Serialize_peekHeader( Serialize *self,
                           char *type,
                           char *name,
                           int *objSize,
                           char *format,
                           char *opts )
{
    long scanfReadBytes = 0;
    int maj = 0;
    int min = 0;
    char ungetBuffer[SERIALIZE_HEADER_MAXLEN + 9] = "";   /* 9 == "HRIS-2.0" */
    char buffer[SERIALIZE_HEADER_MAXLEN] = "";
    bool parseOpts = true;
    bool retVal = false;
    SerializeHeader *header = (SerializeHeader *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( type );
    ANY_REQUIRE( name );
    ANY_REQUIRE( objSize );
    ANY_REQUIRE( format );

    if( self->useHeader == false )
    {
        ANY_LOG( 0, "Serialize_peekHeader() does not allow the SERIALIZE_MODE_NOHEADER option.", ANY_LOG_ERROR );
        goto exitLabel;
    }

    if( opts == NULL)
    {
        ANY_LOG( 7, "`opts' NULL, will not parse options string.", ANY_LOG_INFO );
        parseOpts = false;
    }

    /* We need to read the header from the stream. The IOChannel#scanf
   * returns the number of input elements matched, but reads
   * effectively, in our case, SERIALIZE_HEADER_PREAMBLEMAXLEN bytes.
   * We'll have to consider this when ungetting it. */
    if( IOChannel_scanf( self->stream, &scanfReadBytes,
                         (char *)SERIALIZE_HEADER_PREAMBLE"%d.%d",
                         &maj, &min ) != 2 )
    {
        if( IOChannel_eof( self->stream ) == false )
        {
            ANY_LOG( 0, "Uncorrect header format!", ANY_LOG_ERROR );
        }
        self->errorOccurred = true;
        goto exitLabel;
    }

    header = self->header;

    header->majVersion = maj;
    header->minVersion = min;

    // Start detection of header version
    SERIALIZE_START_HEADER_SWITCH( header ) ;

            // Handle version 1.0
            SERIALIZE_START_HANDLE_VERSION( 1, 0 )
                {
                    int i = 0;
                    long int tmp = 0;

                    ANY_LOG( 7, "Reading the fields of the header from the stream.", ANY_LOG_INFO );

                    if( IOChannel_scanf( self->stream, &tmp, (char *)"%s %s %d %s ",
                                         type, name, objSize, format ) != 4 )
                    {
                        ANY_LOG( 0, "An error occured while reading header elements.", ANY_LOG_ERROR );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }

                    ANY_LOG( 7, "Header fields were correctly read from the stream.", ANY_LOG_INFO );

                    if( parseOpts == true )
                    {
                        for( i = 0; i < SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE; i++ )
                        {
                            if( IOChannel_read( self->stream, &opts[ i ], 1 ) != 1 )
                            {
                                ANY_LOG( 0, "No more chars to read the option string; allocate a larger buffer.",
                                         ANY_LOG_ERROR );
                                self->errorOccurred = true;
                                goto exitLabel;
                            }

                            if( opts[ i ] == '\n' )
                            {
                                /* Remove trailing newline */
                                ANY_LOG( 7, "Header terminator `\\n' was found, terminating parsing.", ANY_LOG_INFO );
                                opts[ i ] = '\0';
                                break;
                            }
                        }

                        if( self->errorOccurred == true )
                        {
                            ANY_LOG( 7, "Header parsing aborted.", ANY_LOG_INFO );
                            goto exitLabel;
                        }
                    }
                }
            SERIALIZE_STOP_HANDLE_VERSION();

                // Handle version 2.0
            SERIALIZE_START_HANDLE_VERSION( 2, 0 )
                {
                    char *readType;
                    char *readName;
                    char *readFormat;
                    char *readOpts;
                    char *readObjSize;
                    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
                    SerializeReferenceValue *list = (SerializeReferenceValue *)NULL;
                    SerializeReferenceValue *listTail = (SerializeReferenceValue *)NULL;

                    ANY_LOG( 7, "Reading the fields of the header from the stream.", ANY_LOG_INFO );

                    if( IOChannel_gets( self->stream, buffer, SERIALIZE_HEADER_MAXLEN) <= 0 )
                    {
                        ANY_LOG( 0, "Could not read header from stream.", ANY_LOG_ERROR );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }

                    /* Read from the string the various values */

                    // Parse string and make NVPs
                    rvp = SerializeReferenceValue_new();
                    SerializeReferenceValue_init( rvp, "", NULL);
                    list = rvp;
                    listTail = list;

                    SerializeReferenceValue_getRVP( &list, NULL, &listTail, buffer );

                    // Read 'type' value then store it in the header
                    readType = SerializeReferenceValue_findValue( list, "type" );
                    if( !readType )
                    {
                        ANY_LOG( 0, "Error: could not find reference %s", ANY_LOG_INFO, "type" );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }
                    else
                    {
                        Any_strncpy( type, readType, Any_strlen( readType ) + 1 );
                    }

                    // Read 'name' value then store it in the header
                    readName = SerializeReferenceValue_findValue( list, "name" );
                    if( !readName )
                    {
                        ANY_LOG( 0, "Error: could not find reference %s", ANY_LOG_INFO, "name" );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }
                    else
                    {
                        Any_strncpy( name, readName, Any_strlen( readName ) + 1 );
                    }

                    // Read 'objSize' value
                    readObjSize = SerializeReferenceValue_findValue( list, "objSize" );
                    if( !readObjSize )
                    {
                        ANY_LOG( 0, "Error: could not find reference %s", ANY_LOG_INFO, "objSize" );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }
                    else
                    {
                        *( objSize ) = atoi( readObjSize );
                    }

                    // Read 'format' value
                    readFormat = SerializeReferenceValue_findValue( list, "format" );
                    if( !readFormat )
                    {
                        ANY_LOG( 0, "Error: could not find reference %s", ANY_LOG_INFO, "format" );
                        self->errorOccurred = true;
                        goto exitLabel;
                    }
                    else
                    {
                        Any_strncpy( format, readFormat, Any_strlen( readFormat ) + 1 );
                    }

                    ANY_LOG( 7, "Header fields were correctly read from the stream.", ANY_LOG_INFO );

                    if( parseOpts == true )
                    {
                        // Read 'opts' value
                        readOpts = SerializeReferenceValue_findValue( list, "opts" );
                        if( !readOpts )
                        {
                            ANY_LOG( 0, "Error: could not find reference %s", ANY_LOG_INFO, "opts" );
                            self->errorOccurred = true;
                            goto exitLabel;
                        }
                        else
                        {
                            Any_strncpy( opts, readOpts, Any_strlen( readOpts ) + 1 );
                        }

                        /* Do not free rvp! It's currently pointing to the last element
       * in the list so freeing the list will be enough to free all the
       * allocated memory. */
                        SerializeReferenceValue_destroyList( list );
                    }

                    if( self->errorOccurred == true )
                    {
                        ANY_LOG( 7, "Header parsing aborted.", ANY_LOG_INFO );
                        goto exitLabel;
                    }

//     ANY_LOG( 7, "Getting header length.", ANY_LOG_INFO );
//     len = Any_strlen( buffer );
                }
            SERIALIZE_STOP_HANDLE_VERSION();

            SERIALIZE_STOP_HEADER_SWITCH();

    // Parsing finished, call IOChannel#unget to rewind the stream

    // Write to the buffer first the preamble that was read at the beginning
    Any_snprintf( ungetBuffer, scanfReadBytes + 1, SERIALIZE_HEADER_PREAMBLE
            "%d.%d", maj, min );
    // Write the rest of the header as it was reconstructed
    Any_strncat( ungetBuffer, buffer, SERIALIZE_HEADER_MAXLEN - 1 );
    // Then add last trailing newline
    Any_strncat( ungetBuffer, "\n", SERIALIZE_HEADER_MAXLEN - 1 );

    if( IOChannel_unget( self->stream, ungetBuffer, Any_strlen( ungetBuffer )) == -1 )
    {
        ANY_LOG( 0, "Unable to write header back to stream.", ANY_LOG_ERROR );
        goto exitLabel;
    }

    retVal = true;

    exitLabel:
    return retVal;
}


void Serialize_onBeginSerialize( Serialize *self, void (*function)( void * ), void *functionParam )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    /* TODO: Remember to enable it - 30-Jan-2012
     Serialize_addAnyEventInfo( self->onBeginSerialize, function, functionParam );
  */
}


void Serialize_onEndSerialize( Serialize *self, void (*function)( void * ), void *functionParam )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    /* TODO: Remember to enable it - 30-Jan-2012
     Serialize_addAnyEventInfo( self->onEndSerialize, function, functionParam );
  */
}

/*-------------------------------------------------------------------------*/
/*    Private functions                                                    */
/*-------------------------------------------------------------------------*/

static void Serialize_fireEventInfo( Serialize *self, AnyEventInfo *eventInfo )
{
    AnyEventInfo *info = eventInfo;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    if( eventInfo )
    {
        while( info )
        {
            if( info->function )
            {
                ( *info->function )( info->functionParam );
            }
            info = info->next;
        }
    }
}

static void Serialize_partialReset( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_partialReset" );

    ANY_REQUIRE( self );

    self->indentLevel = SERIALIZE_INDENTLEVEL;
    self->columnWrap = SERIALIZE_COLUMNWRAP;
    self->baseTypeEnable = false;
    self->forceBinaryDeploy = false;
    self->errorOccurred = false;
    self->roundOff = 0;
    self->backOff = 0;
    self->objInitialOffset = 0;
    self->numTypeCalls = 0;
    self->recoveryJmpSet = false;
}


static void Serialize_resetSerialize( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "Serialize_resetSerialize" );

    ANY_REQUIRE( self );
    /* Do Not Check Valid! */

    self->mode = SERIALIZE_MODE_NULL;
    self->indentLevel = SERIALIZE_INDENTLEVEL;
    self->columnWrap = SERIALIZE_COLUMNWRAP;
    self->streamMode = SERIALIZE_STREAMMODE_NORMAL;
    self->format = (SerializeFormatInfo *)NULL;
    self->header = (SerializeHeader *)NULL;
    self->formatList = (MTList *)NULL;
    self->calcSizeStream = (IOChannel *)NULL;
    self->stream = (IOChannel *)NULL;
    self->baseTypeEnable = false;
    self->isLittleEndian = false;
    self->forceBinaryDeploy = false;
    self->isInitMode = false;
    self->isAutoCalcSizeMode = false;
    self->isTranslateMode = false;
    self->useHeader = true;
    self->errorOccurred = false;
    self->roundOff = 0;
    self->backOff = 0;
    self->objInitialOffset = 0;
    self->numTypeCalls = 0;
    self->recoveryJmpSet = false;
    /* TODO: Remember to enable it - 30-Jan-2012
     self->onBeginSerialize   = NULL;
     self->onEndSerialize     = NULL;
  */
}


static bool Serialize_checkModes( Serialize *self )
{
    bool retVal = false;
    int modes = -1;

    SERIALIZE_TRACE_FUNCTION( "Serialize_checkModes" );

    ANY_REQUIRE( self );
    /* Prevent segmentation fault with uninitialized plugins */
    ANY_REQUIRE( self->format );

    modes = SERIALIZEFORMAT_GETALLOWEDMODES( self );

    if( self->isTranslateMode == true )
    {
        if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_TRANSLATE ) == false )
        {
            ANY_LOG( 0, "Mode TRANSLATE was set, but format does not allow it!",
                     ANY_LOG_ERROR );
            self->errorOccurred = true;
            goto exitLabel;
        }
    }

    if( self->mode == SERIALIZE_MODE_CALC )
    {
        if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_CALC ) == false )
        {
            ANY_LOG( 0, "Mode CALCSIZE was set, but format does not allow it!",
                     ANY_LOG_ERROR );
            self->errorOccurred = true;
            goto exitLabel;
        }
    }
    retVal = true;

    exitLabel:;
    return retVal;
}


static long long Serialize_getStreamPosition( Serialize *self )
{
    long long retVal = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getStreamPosition" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZE_VALID );

    if( self->isTranslateMode == false )
    {
        ANY_REQUIRE( self->stream );

        retVal = IOChannel_getStreamPosition( self->stream );

        if( retVal < 0 )
        {
            ANY_LOG( 0, "Unable to get valid stream position from CalcSize stream.",
                     ANY_LOG_ERROR );
            retVal = 0;
        }
    }

    return retVal;
}


static bool Serialize_setStreamModeFromModes( Serialize *self, int modes )
{
    bool retVal = true;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setStreamModeFromModes" );

    ANY_REQUIRE( self );

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_STREAMMODE_NORMAL ))
    {
        self->streamMode = SERIALIZE_STREAMMODE_NORMAL;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_STREAMMODE_FLUSH ))
    {
        self->streamMode = SERIALIZE_STREAMMODE_FLUSH;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_STREAMMODE_LOOP ))
    {
        self->streamMode = SERIALIZE_STREAMMODE_LOOP;
    }

    return retVal;
}


static bool Serialize_setDirectionFromModes( Serialize *self, int modes )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setDirectionFromModes" );

    ANY_REQUIRE( self );

    self->mode = -1;

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_WRITE ))
    {
        self->mode = SERIALIZE_MODE_WRITE;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_READ ))
    {
        self->mode = SERIALIZE_MODE_READ;
    }

    if( SERIALIZE_MODE_IS( modes, SERIALIZE_MODE_CALC ))
    {
        self->mode = SERIALIZE_MODE_CALC;
    }

    retVal = ((int)self->mode != -1 ? true : false );

    return retVal;
}


static bool Serialize_isTheFirstBeginTypeCall( Serialize *self )
{
    char *streamType = NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isTheFirstBeginTypeCall" );

    ANY_REQUIRE( self );

    streamType = IOChannel_getStreamType( self->stream );

    if( streamType != NULL && Any_strcmp( streamType, "Udp" ) == 0 )
    {
        ANY_LOG( 0, "Serialization over UDP is unreliable,", ANY_LOG_WARNING );
        ANY_LOG( 0, "consider using TCP instead!", ANY_LOG_WARNING );
    }

    return ( self->numTypeCalls == 0 ? true : false );
}


static void Serialize_doFirstBeginTypeCallOps( Serialize *self,
                                               const char *name,
                                               const char *type )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *optsString = (char *)NULL;
    long long headerIniOffset = 0;
    long long headerEndOffset = 0;
    int maj = 0;
    int min = 0;
    long scanfReadBytes = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_doFirstBeginTypeCallOps" );

    ANY_REQUIRE( self );

    SERIALIZE_REQUIRE_STRING( name );
    SERIALIZE_REQUIRE_STRING( type );

    header = self->header;
    ANY_REQUIRE( header );

    header->objSize = 0;
    header->headerSize = 0;

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            /* Move list to cache pool */
            SerializeReferenceValue_append( &self->header->poolHead, self->header->listHead );
            /* The lists were joined, so now we reset the listHead and listTail
       * pointers. */
            self->header->listHead = NULL;
            self->header->listTail = self->header->listHead;

            if( self->streamMode == SERIALIZE_STREAMMODE_LOOP )
            {
                ANY_REQUIRE( self->offsetForLoop >= 0 );

                if( IOChannel_seek( self->stream, self->offsetForLoop, IOCHANNELWHENCE_SET ) == -1 )
                {
                    ANY_LOG( 0, "Loop Mode is Set But Seek Returned -1", ANY_LOG_ERROR );
                    self->errorOccurred = true;
                }
            }

            /* Get initial header offset */
            headerIniOffset = Serialize_getStreamPosition( self );

            /* fire all the callbacks if any */
            /* TODO: Remember to enable it - 30-Jan-2012
         Serialize_fireEventInfo( self, self->onBeginSerialize );
      */

            /* fire the related IOChannel eventInfo */
            Serialize_fireEventInfo( self, (AnyEventInfo *)IOChannel_getProperty( self->stream, "onBeginSerialize" ));

            if( IOChannel_scanf( self->stream, &scanfReadBytes,
                                 (char *)SERIALIZE_HEADER_PREAMBLE"%d.%d ",
                                 &maj, &min ) != 2 )
            {
                if( IOChannel_eof( self->stream ) == false )
                {
                    ANY_LOG( 0, "Uncorrect header format!", ANY_LOG_ERROR );
                }
                self->errorOccurred = true;
                break;
            }

            header->majVersion = maj;
            header->minVersion = min;

            ANY_LOG( 7, "Header label and version correctly read.", ANY_LOG_INFO );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            Serialize_checkModes( self );

            header->majVersion = SERIALIZE_HEADER_MAJVERSIONDEFAULT;
            header->minVersion = SERIALIZE_HEADER_MINVERSIONDEFAULT;

            maj = SERIALIZE_HEADER_MAJVERSIONDEFAULT;
            min = SERIALIZE_HEADER_MINVERSIONDEFAULT;

            if( self->streamMode == SERIALIZE_STREAMMODE_LOOP )
            {
                ANY_REQUIRE( self->offsetForLoop >= 0 );

                if( IOChannel_seek( self->stream, self->offsetForLoop,
                                    IOCHANNELWHENCE_SET ) == -1 )
                {
                    ANY_LOG( 0, "Loop mode is set but seek returned -1.",
                             ANY_LOG_ERROR );
                    self->errorOccurred = true;
                }
            }

            /* Get initial header offset */
            headerIniOffset = Serialize_getStreamPosition( self );

            /* Before Print Data, Save BackOff */
            self->backOff = 0;
            self->backOff = Serialize_getStreamPosition( self );

            /* fire all the callbacks if any */
            /* TODO: Remember to enable it - 30-Jan-2012
         Serialize_fireEventInfo( self, self->onBeginSerialize );
      */

            /* fire the related IOChannel eventInfo */
            Serialize_fireEventInfo( self, (AnyEventInfo *)IOChannel_getProperty( self->stream, "onBeginSerialize" ));

            if( IOChannel_printf( self->stream,
                                  (char *)SERIALIZE_HEADER_PREAMBLE"%d.%d ",
                                  &maj, &min ) < 0 )
            {
                ANY_LOG( 0, "Unable to print header label.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            ANY_LOG( 7, "Header label and version correctly written.",
                     ANY_LOG_INFO );
        }
            break;

        default:
        {
            ANY_LOG( 0, "Bad serialize mode[%d].", ANY_LOG_ERROR, self->mode );
        }
            break;
    }

    /* Finally if all went Ok, use the right header parser  */
    if( self->errorOccurred == false )
    {
        SERIALIZE_START_HEADER_SWITCH( header ) ;

                SERIALIZE_START_HANDLE_VERSION( 1, 0 )
                    {
                        ANY_LOG( 7, "Using parser for header version 1.0", ANY_LOG_INFO );

                        SerializeHeader_useParserV10( self, name, type );

                        /* Set or reset internal option if any */
                        optsString = SerializeReferenceValue_findValue( self->header->listHead, "opts" );
                        if( rvp != NULL)
                        {
                            SERIALIZEFORMATOPTIONS_SET( self, optsString );
                        }

                        /* Set header size */
                        headerEndOffset = Serialize_getStreamPosition( self );
                        header->headerSize = ( headerEndOffset - headerIniOffset );
                    }
                SERIALIZE_STOP_HANDLE_VERSION();

                SERIALIZE_START_HANDLE_VERSION( 2, 0 )
                    {
                        ANY_LOG( 7, "Using parser for header version 2.0", ANY_LOG_INFO );

                        SerializeHeader_useParserV20( self, name, type );

                        /* Set or reset internal option if any */
                        optsString = SerializeReferenceValue_findValue( self->header->listHead, "opts" );
                        if( optsString != NULL)
                        {
                            SERIALIZEFORMATOPTIONS_SET( self, optsString );
                        }

                        /* Set header size */
                        headerEndOffset = Serialize_getStreamPosition( self );
                        header->headerSize = ( headerEndOffset - headerIniOffset );
                    }
                SERIALIZE_STOP_HANDLE_VERSION();

                SERIALIZE_STOP_HEADER_SWITCH();
    }
}


static bool Serialize_isTheLastEndTypeCall( Serialize *self )
{
    bool retVal = false;

    SERIALIZE_TRACE_FUNCTION( "Serialize_isTheLastEndTypeCall" );

    ANY_REQUIRE( self );

    retVal = ( self->numTypeCalls == 0 ? true : false );

    return retVal;
}


static void Serialize_doLastEndTypeCallOps( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;

    SERIALIZE_TRACE_FUNCTION( "Serialize_doLastEndTypeCallOps" );

    ANY_REQUIRE( self );

    header = self->header;
    ANY_REQUIRE( header );

    /* Notifies the channel that a new object is starting */
    IOChannel_setProperty( self->stream, "isBeginType", (void *)true );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            switch( self->streamMode )
            {
                case SERIALIZE_STREAMMODE_NORMAL:
                case SERIALIZE_STREAMMODE_LOOP:
                    break;

                case SERIALIZE_STREAMMODE_FLUSH:
                {
                    if( IOChannel_flush( self->stream ) == -1 )
                    {
                        ANY_LOG( 7, "SERIALIZE_STREAMMODE_FLUSH is set, but flush() returned -1!", ANY_LOG_ERROR );
                        self->errorOccurred = true;
                    }
                    /* Re-Calc BackOff */
                    self->backOff = Serialize_getStreamPosition( self );
                }
                    break;

                default:
                {
                    ANY_LOG( 0, "Bad SerializeStreamMode!", ANY_LOG_ERROR );
                    self->errorOccurred = true;
                    ANY_REQUIRE( NULL );
                }
                    break;
            }
        }
            break;

        default:
        {
            ANY_LOG( 0, "Bad Serialize Mode[%d]!", ANY_LOG_ERROR, self->mode );
            self->errorOccurred = true;
            ANY_REQUIRE( NULL );
        }
            break;
    }

    /* fire all the callbacks if any */
    /* TODO: Remember to enable it - 30-Jan-2012
     Serialize_fireEventInfo( self, self->onEndSerialize );
  */

    /* fire the related IOChannel eventInfo */
    Serialize_fireEventInfo( self, (AnyEventInfo *)IOChannel_getProperty( self->stream, "onEndSerialize" ));
}


static void Serialize_doAutoCalcSizeOps( Serialize *self )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    char *ptr = (char *)NULL;
    bool memBasedStream = false;

    ANY_REQUIRE( self );

    header = self->header;
    ANY_REQUIRE( header );

    /* FIXME: memory streams can have the buffered flag set */
    /* if a malicious programmator set it... */
    if( IOChannel_usesWriteBuffering( self->stream ) == true )
    {
        long totalSize = 0;
        long bufferPos = 0;

        ptr = (char *)IOChannel_getInternalWriteBufferPtr( self->stream );
        ANY_REQUIRE( ptr );

        totalSize = Serialize_getHeaderSize( self ) + Serialize_getPayloadSize( self );
        ANY_REQUIRE( totalSize >= 0 );

        bufferPos = IOChannel_getWriteBufferedBytes( self->stream );
        ANY_REQUIRE( bufferPos >= 0 );

        if( totalSize > bufferPos )
        {
            ANY_LOG( 0, "AutoCalcSize flag was used in a buffered stream, but "
                    "probably the data was flushed because the buffer was not "
                    "big enough", ANY_LOG_ERROR );

            ANY_LOG( 0, "unable to modify header size", ANY_LOG_ERROR );

            goto exitLabel;
        }

        ptr += bufferPos - totalSize;
        ANY_REQUIRE( ptr );
    }
    else
    {
        if( IOChannel_hasPointer( self->stream ) == false )
        {
            ANY_LOG( 0, "AutoCalcSize flag was used, but stream is neither buffered "
                    "nor is it a memory based stream", ANY_LOG_ERROR );

            ANY_LOG( 0, "unable to modify header size", ANY_LOG_ERROR );

            goto exitLabel;
        }

        ptr = (char *)IOChannel_getProperty( self->stream, (char *)"MemPointer" );
        ANY_REQUIRE_MSG( ptr, "Memory-stream pointer is NULL" );

        memBasedStream = true;

        ptr += self->backOff;
        ANY_REQUIRE( ptr );
    }

    ANY_REQUIRE( ptr );
    if( Any_strncmp( ptr, "HRIS-", 5 ) != 0 )
    {
        if( memBasedStream == true )
        {
            ANY_LOG( 0, "AutoCalcSize flag was used in a memory based stream, but "
                    "stream seems corrupted", ANY_LOG_ERROR );

            ANY_LOG( 0, "no serialization header found", ANY_LOG_ERROR );

            self->errorOccurred = true;
        }
        else
        {
            ANY_LOG( 0, "AutoCalcSize flag was used in a buffered stream, but "
                    "probably the data was flushed because the buffer was not "
                    "big enough", ANY_LOG_ERROR );

            ANY_LOG( 0, "unable to modify header size", ANY_LOG_ERROR );
        }
        goto exitLabel;
    }
    else
    {
        char *sizeCharPtr = (char *)NULL;
        int numChars = -1;
        int i = 0;

        ANY_LOG( 7, "AutoCalcSize is working on the buffer to the header size field",
                 ANY_LOG_INFO );

        sizeCharPtr = ptr;
        ANY_REQUIRE( sizeCharPtr );

        SERIALIZE_START_HEADER_SWITCH( header )

                SERIALIZE_START_HANDLE_VERSION( 1, 0 )
                    {
                        /* "HRIS-1.0 Point point 43 Binary options\n" */

                        for( i = 0; i < 3; i++ )
                        {
                            sizeCharPtr = Any_strchr( sizeCharPtr, ' ' );
                            ANY_REQUIRE( sizeCharPtr );

                            sizeCharPtr++;
                            ANY_REQUIRE( sizeCharPtr );
                        }
                    }
                SERIALIZE_STOP_HANDLE_VERSION();

                SERIALIZE_START_HANDLE_VERSION( 2, 0 )
                    {
                        /* "HRIS-2.0 type = 'Point' name = point objSize =          0 format = Binary opts = 'LITTLE_ENDIAN' */

                        for( i = 0; i < 9; i++ )
                        {
                            sizeCharPtr = Any_strchr( sizeCharPtr, ' ' );
                            ANY_REQUIRE( sizeCharPtr );

                            sizeCharPtr++;
                            ANY_REQUIRE( sizeCharPtr );
                        }
                    }
                SERIALIZE_STOP_HANDLE_VERSION();

                SERIALIZE_STOP_HEADER_SWITCH();

        numChars = Any_sprintf( sizeCharPtr, "%10ld", (long)self->header->objSize );
        ANY_REQUIRE( numChars > 0 );


        sizeCharPtr += numChars;
        ANY_REQUIRE( sizeCharPtr );

        *sizeCharPtr = ' ';

        ANY_LOG( 7, "header size field correctly written by AutoCalcSize",
                 ANY_LOG_INFO );
    }

    exitLabel:;
}


/*-------------------------------------------------------------------------*/
/* CalcSize Private Functions                                              */
/*-------------------------------------------------------------------------*/

static IOChannel *SerializeCalcStream_create( void )
{
    IOChannel *stream = (IOChannel *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeCalcStream_create" );

    stream = IOChannel_new();
    ANY_REQUIRE_MSG( stream, "Cannot Malloc' For The CalcSize stream" );

    if( IOChannel_init( stream ) == false )
    {
        ANY_LOG( 0, "IOChannel_init() for calc stream failed", ANY_LOG_ERROR );
        goto releaseFromDelete;
    }

    if( IOChannel_open( stream, (char *)"Calc://",
                        IOCHANNEL_MODE_W_ONLY,
                        IOCHANNEL_PERMISSIONS_ALL) == false )
    {
        ANY_LOG( 0, "IOChannel_open() for calc stream failed", ANY_LOG_ERROR );
        goto releaseFromCLear;
    }

    goto exitLabel;

    releaseFromCLear:
    IOChannel_clear( stream );

    releaseFromDelete:
    IOChannel_delete( stream );

    stream = (IOChannel *)NULL;

    exitLabel:
    return stream;
}


static void SerializeCalcStream_destroy( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "SerializeCalcStream_destroy" );

    ANY_REQUIRE( self );

    if( IOChannel_close( self->calcSizeStream ) == false )
    {
        ANY_LOG( 0, "IOChannel_close() for calc stream failed", ANY_LOG_ERROR );
    }
    else
    {
        ANY_LOG( 7, "CalcSize stream correctly closed", ANY_LOG_INFO );
    }

    IOChannel_clear( self->calcSizeStream );
    IOChannel_delete( self->calcSizeStream );
}

/*-------------------------------------------------------------------------*/
/* SerializeFormatList Private Functions                                   */
/*-------------------------------------------------------------------------*/

static MTList *SerializeFormatList_create( void )
{
    MTList *list = (MTList *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeFormatList_create" );

    list = MTList_new();
    ANY_REQUIRE_MSG( list, "Can't allocate memory for MTList" );

    if( MTList_init( list ) == false )
    {
        ANY_LOG( 0, "Unable to initialize the MTList For formats", ANY_LOG_ERROR );
    }
    else
    {
        MTList_setDeleteMode( list, MTLIST_DELETEMODE_MANUAL );
    }

    return list;
}


static bool SerializeFormatList_addFormat( Serialize *self,
                                           const char *format,
                                           const char *path )
{
#define SERIALIZE_PLUGINNAME_MAXLEN     512

    SerializeFormat *plugin = (SerializeFormat *)NULL;
    char pluginName[SERIALIZE_PLUGINNAME_MAXLEN];
    char libraryName[SERIALIZE_PLUGINNAME_MAXLEN];
    DynamicLoader *libraryHandle = (DynamicLoader *)NULL;
    bool retVal = false;
    int status = -1;

    SERIALIZE_TRACE_FUNCTION( "SerializeFormatList_addFormat" );

    ANY_REQUIRE( self );

    /* First try to find a static plugin */
    plugin = SerializeFormat_findStaticFormat( format );

    /* if the plugin has been found */
    if( plugin )
    {
        status = Any_snprintf( pluginName, SERIALIZE_PLUGINNAME_MAXLEN, "SerializeFormat%sOps", plugin->formatName );
        ANY_REQUIRE( status > 0 );

        goto _addPublicSymbol;
    }

    /* THEN TRY TO DETECT IF THE SYMBOL IS AVAILABLE IN THE PUBLIC SYMBOL'S SPACE */

    /* Build The Plugin Name */
    status = Any_snprintf( pluginName, SERIALIZE_PLUGINNAME_MAXLEN, "SerializeFormat%sOps", format );
    ANY_REQUIRE( status > 0 );

    ANY_LOG( 7, "Searching public symbol[%s]", ANY_LOG_INFO, pluginName );

    /* search the symbol in the global symbol space */
    plugin = (SerializeFormat *)DynamicLoader_getSymbolByName(NULL, pluginName );

    /* if the plugin has been found */
    if( plugin )
    {
        goto _addPublicSymbol;
    }

    /* Search the symbol in the libToolBOSCore library */
    libraryHandle = DynamicLoader_new();

    if( !libraryHandle )
    {
        ANY_LOG( 0, "Unable to allocate memory for a new DynamicLoader object", ANY_LOG_ERROR );
        goto exitLabel;
    }

    /* Build platform specific libToolBOSCore library name */
#if defined(__windows__)
    status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                           "libToolBOSCore.%d.%d.dll",
                           TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#else
    status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                           "libToolBOSCore.so.%d.%d",
                           TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#endif

    if( DynamicLoader_init( libraryHandle, libraryName ) != 0 )
    {
        ANY_LOG( 0, "Unable to initialize the DynamicLoader object. ( dlerror [%s] )",
                 ANY_LOG_ERROR, DynamicLoader_getError( libraryHandle ));
        DynamicLoader_delete( libraryHandle );
        libraryHandle = NULL;
        goto exitLabel;
    }

    ANY_LOG( 7, "libToolBOSCore.so library opened, searching for symbol [%s].",
             ANY_LOG_INFO, pluginName );

    plugin = (SerializeFormat *)DynamicLoader_getSymbolByName( libraryHandle, pluginName );

    if( plugin )
    {
        /* Found the symbol  */
        goto _addPublicSymbol;
    }
    else
    {
        /* Couldn't find the symbol in the libToolBOSCore library, clean
     * up after ourselves and keep looking */
        DynamicLoader_clear( libraryHandle );
        DynamicLoader_delete( libraryHandle );
        libraryHandle = NULL;
    }

    /* Build The Library Name */
    if( path != (char *)NULL)
    {
#if defined(__windows__)
        status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                               "%s/libSerializeFormat%s.%d.%d.dll",
                               path, format, TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#else
        status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                               "%s/libSerializeFormat%s.so.%d.%d",
                               path, format, TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#endif
    }
    else
    {
#if defined(__windows__)
        status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                               "libSerializeFormat%s.%d.%d.dll",
                               format, TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#else
        status = Any_snprintf( libraryName, SERIALIZE_PLUGINNAME_MAXLEN,
                               "libSerializeFormat%s.so.%d.%d",
                               format, TOOLBOS_MAJVERSION, TOOLBOS_MINVERSION );
#endif
    }
    ANY_REQUIRE( status > 0 );

    libraryHandle = DynamicLoader_new();

    if( !libraryHandle )
    {
        ANY_LOG( 0, "Unable to allocate memory for a new DynamicLoader object", ANY_LOG_ERROR );
        goto exitLabel;
    }

    if( DynamicLoader_init( libraryHandle, libraryName ) != 0 )
    {
        ANY_LOG( 0, "Unable to open plugin library[%s] for Format [%s]! ( dlerror [%s] )",
                 ANY_LOG_ERROR, libraryName, format, DynamicLoader_getError( libraryHandle ));

        DynamicLoader_delete( libraryHandle );
        libraryHandle = NULL;
        goto exitLabel;
    }

    ANY_LOG( 7, "[%s] Library opened, searching for symbol[%s]",
             ANY_LOG_INFO, libraryName, pluginName );

    plugin = (SerializeFormat *)DynamicLoader_getSymbolByName( libraryHandle, pluginName );

    if( plugin == (SerializeFormat *)NULL)
    {
        ANY_LOG( 0, "Unable To Find the requested Plugin[%s]! ( dlerror [%s] )",
                 ANY_LOG_ERROR, format, DynamicLoader_getError( libraryHandle ));

        DynamicLoader_clear( libraryHandle );
        DynamicLoader_delete( libraryHandle );
        libraryHandle = NULL;

        self->errorOccurred = true;

        goto exitLabel;
    }

    _addPublicSymbol:

    retVal = Serialize_addFormat( self, plugin );

    if( retVal == false )
    {
        if( libraryHandle )
        {
            DynamicLoader_clear( libraryHandle );
            DynamicLoader_delete( libraryHandle );
            libraryHandle = NULL;
        }

        self->errorOccurred = true;
    }
    else
    {
        SerializeFormatInfo *tmp = (SerializeFormatInfo *)NULL;

        tmp = SerializeFormatList_find( self, format );

        SerializeFormatList_setPluginLibHandle( self, tmp, libraryHandle );
    }

    exitLabel:;

    return retVal;
#undef SERIALIZE_PLUGINNAME_MAXLEN
}


static SerializeFormat *SerializeFormat_findStaticFormat( const char *formatName )
{
    int i;
    SerializeFormat *retVal = NULL;

    ANY_REQUIRE( formatName );

    for( i = 0; Serialize_internalFormats[ i ]; i++ )
    {
        if( Any_strcmp( Serialize_internalFormats[ i ]->formatName, formatName ) == 0 )
        {
            retVal = Serialize_internalFormats[ i ];
            break;
        }
    }

    return retVal;
}


static bool SerializeFormatList_setPluginLibHandle( Serialize *self,
                                                    SerializeFormatInfo *info,
                                                    DynamicLoader *libraryHandle )
{
    bool retVal = true;

    SERIALIZE_TRACE_FUNCTION( "SerializeFormatList_setPluginLibHandle" );

    ANY_REQUIRE( self );
    ANY_REQUIRE( info );
    /* ANY_REQUIRE( libraryHandle ); */

    info->libHandle = libraryHandle;

    return retVal;
}


static SerializeFormatInfo *SerializeFormatList_find( Serialize *self,
                                                      const char *format )
{
    SerializeFormatInfo *retVal = (SerializeFormatInfo *)NULL;
    SerializeFormatInfo *ptr = (SerializeFormatInfo *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeFormatList_find" );

    ANY_REQUIRE( self );

    SERIALIZE_REQUIRE_STRING( format );

    MTLIST_FOREACH_NOLOCK_BEGIN( self->formatList )
            {
                ptr = (SerializeFormatInfo *)MTLIST_FOREACH_ELEMENTPTR;

                if( Any_strcasecmp( format, ptr->ops->formatName ) == 0 )
                {
                    /* Found! Set The Return Value... */
                    retVal = ptr;
                    MTLIST_FOREACH_NOLOCK_BREAK;
                }
            }
    MTLIST_FOREACH_NOLOCK_END;

    return retVal;
}


static void SerializeFormatList_destroy( Serialize *self )
{
    SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
    SerializeFormat *formatPtr = (SerializeFormat *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeFormatList_destroy" );

    ANY_REQUIRE( self );

    ANY_LOG( 7, "Looping into the MTList to release formats", ANY_LOG_INFO );

    MTLIST_FOREACH_NOLOCK_BEGIN( self->formatList )
            {
                infoPtr = (SerializeFormatInfo *)MTLIST_FOREACH_ELEMENTPTR;
                ANY_REQUIRE( infoPtr );

                formatPtr = infoPtr->ops;

                ANY_LOG( 7, "Unloading [%s] format",
                         ANY_LOG_INFO, formatPtr->formatName );

                if( infoPtr->data )
                {
                    ANY_LOG( 7, "Clear And Delete options for format [%s]",
                             ANY_LOG_INFO, formatPtr->formatName );

                    self->format = infoPtr;

                    SERIALIZEFORMATOPTIONS_CLEAR( self );
                    SERIALIZEFORMATOPTIONS_DELETE( self );
                }

                if( infoPtr->libHandle )
                {
                    DynamicLoader_clear( infoPtr->libHandle );
                    DynamicLoader_delete( infoPtr->libHandle );
                    infoPtr->libHandle = NULL;
                }

                ANY_FREE( infoPtr );
            }
    MTLIST_FOREACH_NOLOCK_END;

    ANY_LOG( 7, "Freeing MTList", ANY_LOG_INFO );

    MTList_clear( self->formatList );
    MTList_delete( self->formatList );
}

/*-------------------------------------------------------------------------*/
/* SerializeHeader Private functions                                       */
/*-------------------------------------------------------------------------*/

static SerializeHeader *SerializeHeader_create( void )
{
    int i = 0;
    SerializeHeader *self = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_create" );

    self = ANY_TALLOC( SerializeHeader );
    ANY_REQUIRE( self );

    /* Create SerializeReferenceValue main list */
    rvp = SerializeReferenceValue_new();
    SerializeReferenceValue_init( rvp, "", NULL);
    self->listHead = rvp;

    /* Since we just created it, the list tail is the same as the list head */
    self->listTail = self->listHead;

    /* Create cache list */
    rvp = SerializeReferenceValue_new();
    SerializeReferenceValue_init( rvp, "", NULL);
    self->poolHead = rvp;

    self->poolTail = self->poolHead;

    /* Populate cache list with a fixed number of elements and
   * initialize them with empty values. */
    for( i = 0; i <= SERIALIZEREFERENCEVALUE_DEFAULT_LIST_SIZE; i++ )
    {
        rvp = SerializeReferenceValue_new();
        SerializeReferenceValue_init( rvp, "", NULL);
        SerializeReferenceValue_push( &self->poolHead, rvp );
    }

    self->majVersion = SERIALIZE_HEADER_MAJVERSIONDEFAULT;
    self->minVersion = SERIALIZE_HEADER_MINVERSIONDEFAULT;

    return self;
}


static SerializeReferenceValue *SerializeHeader_getReferenceValue( SerializeHeader *self, char *ref )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;
    long refLen = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( ref );

    refLen = Any_strlen( ref );

    current = self->listHead;

    while( current != NULL)
    {
        if( Any_strncmp( current->reference, ref, refLen ) == 0 )
        {
            return current;
        }
        current = current->next;
    }

    return current;
}


static void SerializeHeader_useParserV10( Serialize *self,
                                          const char *name,
                                          const char *type )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    int objSize = 0;
    int i = 0;

    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_useParserV10" );

    ANY_REQUIRE( self );

    SERIALIZE_REQUIRE_STRING( name );
    SERIALIZE_REQUIRE_STRING( type );

    header = self->header;
    ANY_REQUIRE( header );

    /* "HRIS-1.0 Point point 43 Binary options\n" */

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            char readType[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";
            char readName[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";
            char readFormat[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";
            char readOpts[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";

            ANY_LOG( 7, "Reading the fields of the header from the stream.",
                     ANY_LOG_INFO );

            if( IOChannel_scanf( self->stream, NULL, "%s %s %d %s ",
                                 readType, readName, &objSize, readFormat ) != 4 )
            {
                ANY_LOG( 0, "Unable to read header elements.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            ANY_LOG( 7, "Header fields were correctly read from the stream. "
                    "Going to read options string until \"\\n\" is found.",
                     ANY_LOG_INFO );

            /* Read the options */
            for( i = 0; i < header->optsSize; i++ )
            {
                if( IOChannel_read( self->stream, &readOpts[ i ], 1 ) != 1 )
                {
                    ANY_LOG( 0, "No more chars to read the option string.",
                             ANY_LOG_ERROR );
                    self->errorOccurred = true;
                    break;
                }

                if( readOpts[ i ] == '\n' )
                {
                    /* Remove trailing newline */
                    ANY_LOG( 7, "Header terminator \"\\n\" was found.", ANY_LOG_INFO );
                    readOpts[ i ] = '\0';
                    break;
                }
            }

            if( self->errorOccurred == true )
            {
                ANY_LOG( 7, "Header parsing aborted.", ANY_LOG_INFO );
                break;
            }

            /* Check type */
            ANY_LOG( 7, "Matching struct type.", ANY_LOG_INFO );
            if( Any_strncmp( readType, type, header->typeSize ) != 0 )
            {
                ANY_LOG( 0, "The struct type read from the header is different "
                        "from the expected one: read [%s], expected [%s]",
                         ANY_LOG_ERROR, readType, type );
                self->errorOccurred = true;
                break;
            }
            ANY_LOG( 7, "Struct type matches.", ANY_LOG_INFO );

            /* Check name */
            /*
       * This check is too strong
       *
       ANY_LOG( 7, "Matching Struct Name..", ANY_LOG_INFO );
       if( Any_strncmp( readName, name, header->nameSize ) != 0 )
       {
       ANY_LOG( 0, "The struct Name read from the header is different "
       "from the expected one! Read[%s], Expected [%s]",
       ANY_LOG_ERROR, header->name, name );
       self->errorOccurred = true;
       break;
       }
       ANY_LOG( 7, "Ok, Struct Name Matches..", ANY_LOG_INFO );
      */

            /* Check format */
            if( self->format != NULL)
            {
                SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
                SerializeFormat *formatPtr = (SerializeFormat *)NULL;

                infoPtr = self->format;

                formatPtr = infoPtr->ops;
                ANY_REQUIRE( formatPtr );

                ANY_LOG( 7, "Matching format header with the currently set.", ANY_LOG_INFO );
                if( Any_strncmp( readFormat, formatPtr->formatName, header->formatSize ) != 0 )
                {
                    ANY_LOG( 7,
                             "The format read from the header is different from the expected one. Read \"%s\", expected \"%s\". Switching to the read format.",
                             ANY_LOG_WARNING, readFormat, formatPtr->formatName );
                }
                else
                {
                    ANY_LOG( 7, "Format matches.", ANY_LOG_INFO );
                }

                /* Setting the format */
                ANY_LOG( 7, "Calling Serialize_setFormat() from header parser",
                         ANY_LOG_INFO );
                if( Serialize_setFormat( self, readFormat, readOpts ) == false )
                {
                    ANY_LOG( 0,
                             "Cannot set format \"%s\" read from the header. Trying to switch to the user-specified format (%s).",
                             ANY_LOG_WARNING, readFormat, formatPtr->formatName );

                    if( Serialize_setFormat( self, formatPtr->formatName, readOpts ) == false )
                    {
                        ANY_LOG( 0, "Cannot set format '%s'",
                                 ANY_LOG_ERROR, formatPtr->formatName );
                        self->errorOccurred = true;
                    }
                }
            }
            else
            {
                ANY_LOG( 7, "Setting format \"%s\"", ANY_LOG_INFO, readFormat );
                if( Serialize_setFormat( self, readFormat, readOpts ) == false )
                {
                    ANY_LOG( 0,
                             "Cannot set format \"%s\" read from the header, and the user did not specify any format. Setting error.",
                             ANY_LOG_ERROR, readFormat );
                    self->errorOccurred = true;
                }
            }

            if( self->errorOccurred != true )
            {
                SerializeHeader_setInfo( self, readType, readName, readOpts, readFormat, 0 );
            }

        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
            SerializeFormat *formatPtr = (SerializeFormat *)NULL;
            char *opts = (char *)NULL;
            char sizeAsString[32] = "";

            infoPtr = self->format;
            ANY_REQUIRE( infoPtr );

            formatPtr = infoPtr->ops;
            ANY_REQUIRE( formatPtr );

            /* Save the information we have at this point - all object except size*/
            /* type, name, objSize, maj, min  */
            ANY_LOG( 7, "Saving header info before write.", ANY_LOG_INFO );

            SerializeHeader_setInfo( self, type, name, NULL, formatPtr->formatName, 0 );

            ANY_LOG( 7, "Writing the fields of the header into the stream.",
                     ANY_LOG_INFO );

            ANY_REQUIRE( header->objSize >= 0 );

            if( self->isAutoCalcSizeMode == true )
            {
                Any_sprintf( sizeAsString, "%10ld", (long)0 );
            }
            else
            {
                /* Check Type And Format */
                Any_sprintf( sizeAsString, "%10ld", (long)header->objSize );
            }

            if( IOChannel_printf( self->stream, (char *)"%s %s %s %s ",
                                  type, name, sizeAsString, formatPtr->formatName ) <= 0 )
            {
                ANY_LOG( 0, "Unable to write header elements.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            /* Write the options */
            ANY_LOG( 7, "Writing the header options.", ANY_LOG_INFO );

            opts = SerializeReferenceValue_findValue( header->listHead, "opts" );
            ANY_REQUIRE( opts );

            if( Any_strlen( opts ) > 0 )
            {
                ANY_LOG( 7, "Writing the header options.", ANY_LOG_INFO );
                if( IOChannel_printf( self->stream, "%s", opts ) <= 0 )
                {
                    ANY_LOG( 0, "Unable to write the option string!", ANY_LOG_ERROR );
                    self->errorOccurred = true;
                    break;
                }
            }

            ANY_LOG( 7, "Writing the header terminator.", ANY_LOG_INFO );
            IOChannel_printf( self->stream, (char *)"\n" );
        }
            break;

        default:
        {
            ANY_LOG( 0, "Bad serialize mode: %d.", ANY_LOG_ERROR, self->mode );
            ANY_REQUIRE( NULL );
        }
            break;
    }
}


static void SerializeHeader_useParserV20( Serialize *self,
                                          const char *name,
                                          const char *type )
{
    SerializeHeader *header = (SerializeHeader *)NULL;

    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_useParserV20" );

    ANY_REQUIRE( self );

    SERIALIZE_REQUIRE_STRING( name );
    SERIALIZE_REQUIRE_STRING( type );

    header = self->header;
    ANY_REQUIRE( header );

    /* "HRIS-2.0 type = 'Point' name = point objSize = 43 format = Binary [opts = 'options']\n" */

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            char headerString[SERIALIZE_HEADER_MAXLEN] = "";
            char *readType = (char *)NULL;
            char *readName = (char *)NULL;
            char *readFormat = (char *)NULL;
            char *readOpts = (char *)NULL;
            long readFormatSize = 0;
            SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;

            ANY_LOG( 7, "Start reading serialization header", ANY_LOG_INFO );

            if( IOChannel_gets( self->stream, headerString, SERIALIZE_HEADER_MAXLEN) <= 0 )
            {
                ANY_LOG( 0, "Could not read header from stream.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            // Parse string and make reference-value pairs.
            SerializeReferenceValue_getRVP( &header->listHead, &header->poolHead, &header->listTail, headerString );

            /* Read from the string the various values */
            readType = SerializeReferenceValue_findValue( header->listHead, "type" );
            if( !readType )
            {
                ANY_LOG( 0, "Error: type could not be found.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            readName = SerializeReferenceValue_findValue( header->listHead, "name" );
            if( !readName )
            {
                ANY_LOG( 0, "Error: name could not be found.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "format" );
            if( !rvp )
            {
                ANY_LOG( 0, "Error: format could not be found.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }
            else
            {
                readFormat = SerializeReferenceValue_getValue( rvp );
                readFormatSize = SerializeReferenceValue_getValueLen( rvp );
            }

            /* Read 'opts' value but remember there's no guarantee we received
       *  options from the stream. */
            readOpts = SerializeReferenceValue_findValue( header->listHead, "opts" );

            ANY_LOG( 7, "Header fields were correctly read from the stream. ", ANY_LOG_INFO );

            /* Check type */
            ANY_LOG( 7, "Matching struct type.", ANY_LOG_INFO );
            if( Any_strncmp( readType, type, header->typeSize ) != 0 )
            {
                ANY_LOG( 0,
                         "The struct type read from the header is different from the expected one: read [%s], expected [%s]",
                         ANY_LOG_ERROR, readType, type );
                self->errorOccurred = true;
                break;
            }
            ANY_LOG( 7, "Struct type matches.", ANY_LOG_INFO );

            /* Check Format */
            if( self->format != NULL)
            {
                SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
                SerializeFormat *formatPtr = (SerializeFormat *)NULL;

                infoPtr = self->format;

                formatPtr = infoPtr->ops;
                ANY_REQUIRE( formatPtr );

                ANY_LOG( 7, "Matching format header with the currently set.", ANY_LOG_INFO );
                if( Any_strncmp( formatPtr->formatName, readFormat, readFormatSize ) != 0 )
                {
                    ANY_LOG( 7,
                             "The format read from the header is different from the expected one: read[%s], expected [%s]. Switching to the read format.",
                             ANY_LOG_WARNING, readFormat, formatPtr->formatName );
                }
                else
                {
                    ANY_LOG( 7, "Format matches.", ANY_LOG_INFO );
                }

                /* Setting the format */
                ANY_LOG( 7, "Calling Serialize_setFormat() from the header parser.", ANY_LOG_INFO );
                if( Serialize_setFormat( self, readFormat, readOpts ) == false )
                {
                    ANY_LOG( 0,
                             "Cannot set format[%s] read from the header. Trying to switch to the user-specified format[%s].",
                             ANY_LOG_WARNING, readFormat, formatPtr->formatName );

                    if( Serialize_setFormat( self, formatPtr->formatName, readOpts ) == false )
                    {
                        ANY_LOG( 0, "Cannot set format[%s]. Aborting.", ANY_LOG_ERROR, formatPtr->formatName );
                        self->errorOccurred = true;
                    }
                }
            }
            else
            {
                ANY_LOG( 7, "Setting format \"%s\"", ANY_LOG_INFO, readFormat );
                if( Serialize_setFormat( self, readFormat, readOpts ) == false )
                {
                    ANY_LOG( 0,
                             "Cannot set format \"%s\" read from the header, and the user did not specified any format. Setting error.",
                             ANY_LOG_ERROR, readFormat );
                    self->errorOccurred = true;
                }
            }

            if( self->errorOccurred != true )
            {
                SerializeHeader_updateHeaderSize( self );
            }
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SerializeFormatInfo *infoPtr = (SerializeFormatInfo *)NULL;
            SerializeFormat *formatPtr = (SerializeFormat *)NULL;
            SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
            char *typeToWrite = (char *)NULL;
            char *nameToWrite = (char *)NULL;
            char *optsToWrite = (char *)NULL;
            char sizeAsString[32];

            infoPtr = self->format;
            ANY_REQUIRE( infoPtr );

            formatPtr = infoPtr->ops;
            ANY_REQUIRE( formatPtr );

            /* Save the information we have at this point: all object except objSize ) */
            ANY_LOG( 7, "Saving header info.", ANY_LOG_INFO );

            SerializeHeader_setInfo( self, type, name, NULL, formatPtr->formatName, 0 );

            ANY_LOG( 7, "Writing the fields of the header into the stream.",
                     ANY_LOG_INFO );

            ANY_REQUIRE( header->objSize >= 0 );

            if( self->isAutoCalcSizeMode == true )
            {
                Any_sprintf( sizeAsString, "%10ld", (long)0 );
            }
            else
            {
                /* Check type and format */
                Any_sprintf( sizeAsString, "%10ld", (long)header->objSize );
            }

            rvp = SerializeHeader_getReferenceValue( header, "type" );
            ANY_REQUIRE_MSG( rvp, "Couldn't find a list for reference \"type\"" );
            typeToWrite = SerializeReferenceValue_getValue( rvp );

            rvp = SerializeHeader_getReferenceValue( header, "name" );
            ANY_REQUIRE_MSG( rvp, "Couldn't find a list for reference \"name\"" );
            nameToWrite = SerializeReferenceValue_getValue( rvp );

            rvp = SerializeHeader_getReferenceValue( header, "opts" );
            /* No ANY_REQUIRE because there's no guarantee we have options. */
            if( rvp != NULL)
            {
                optsToWrite = SerializeReferenceValue_getValue( rvp );
            }

            if( IOChannel_printf( self->stream, (char *)"type = '%s' name = %s objSize = %s format = %s ", typeToWrite,
                                  nameToWrite, sizeAsString, formatPtr->formatName ) <= 0 )
            {
                ANY_LOG( 0, "Unable to write header elements.", ANY_LOG_ERROR );
                self->errorOccurred = true;
                break;
            }

            /* Write the options */
            if( optsToWrite != NULL && Any_strlen( optsToWrite ) > 0 )
            {
                ANY_LOG( 7, "Writing the header options.", ANY_LOG_INFO );
                if( IOChannel_printf( self->stream, "opts = '%s'", optsToWrite ) <= 0 )
                {
                    ANY_LOG( 0, "Unable to write the option string!", ANY_LOG_ERROR );
                    self->errorOccurred = true;
                    break;
                }
            }

            ANY_LOG( 7, "Writing the header terminator.", ANY_LOG_INFO );
            IOChannel_printf( self->stream, (char *)"\n" );
        }
            break;

        default:
        {
            ANY_LOG( 0, "Bad serialize mode[%d].", ANY_LOG_ERROR, self->mode );
            ANY_REQUIRE( NULL );
        }
            break;

    }
}


void Serialize_setHeaderSizes( Serialize *self, const int typeSize, const int nameSize, const int optsSize,
                               const int formatSize )
{
    SerializeHeader *header;

    SERIALIZE_TRACE_FUNCTION( "Serialize_setHeaderSizes" );

    ANY_REQUIRE( self );

    header = self->header;

    // Check that the user has specified a length and it is bigger than what was previously allocated
    if( typeSize > 0 && typeSize > header->typeSize )
    {
        header->typeSize = typeSize;
    }
    else if( typeSize == 0 && header->typeSize == 0 )
    {
        header->typeSize = SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE;
    }

    // Check that the user has specified a length and it is bigger than what was previously allocated
    if( nameSize > 0 && nameSize > header->nameSize )
    {
        header->nameSize = nameSize;
    }
    else if( nameSize == 0 && header->nameSize == 0 )
    {
        header->nameSize = SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE;
    }

    // Check that the user has specified a length and it is bigger than what was previously allocated
    if( optsSize > 0 && optsSize > header->optsSize )
    {
        header->optsSize = optsSize;
    }
    else if( optsSize == 0 && header->optsSize == 0 )
    {
        header->optsSize = SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE;
    }

    // Check that the user has specified a length and it is bigger than what was previously allocated
    if( formatSize > 0 && formatSize > header->formatSize )
    {
        header->formatSize = formatSize;
    }
    else if( formatSize == 0 && header->formatSize == 0 )
    {
        header->formatSize = SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE;
    }
}


static void SerializeHeader_setInfo( Serialize *self, const char *type,
                                     const char *name,
                                     const char *opts,
                                     const char *format,
                                     const long objSize )
{
    SerializeHeader *header = (SerializeHeader *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char buffer[SERIALIZE_HEADER_MAXLEN];

    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_setInfo" );

    ANY_REQUIRE( self );

    header = self->header;
    ANY_REQUIRE( header );

    if( type != (char *)NULL)
    {
        rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "type" );
        if( !rvp )
        {
            rvp = SerializeReferenceValue_pop( &header->poolHead );
            if( !rvp )
            {
                ANY_LOG( 5, "Could not update type with value %s", ANY_LOG_WARNING, type );
            }
            else
            {
                SerializeReferenceValue_push( &header->listHead, rvp );
            }
        }
        SerializeReferenceValue_update( rvp, "type", (char *)type );
    }

    if( name != (char *)NULL)
    {
        rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "name" );
        if( !rvp )
        {
            rvp = SerializeReferenceValue_pop( &header->poolHead );
            if( !rvp )
            {
                ANY_LOG( 5, "Could not update name with value %s", ANY_LOG_WARNING, name );
            }
            else
            {
                SerializeReferenceValue_push( &header->listHead, rvp );
            }
        }
        SerializeReferenceValue_update( rvp, "name", (char *)name );
    }

    if( opts != (char *)NULL)
    {
        rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "opts" );
        if( !rvp )
        {
            rvp = SerializeReferenceValue_pop( &header->poolHead );
            if( !rvp )
            {
                ANY_LOG( 5, "Could not update opts with value %s", ANY_LOG_WARNING, opts );
            }
            else
            {
                SerializeReferenceValue_push( &header->listHead, rvp );
            }
        }
        SerializeReferenceValue_update( rvp, "opts", (char *)opts );
    }

    if( format != (char *)NULL)
    {
        rvp = SerializeReferenceValue_findReferenceValue( header->listHead, "format" );
        if( !rvp )
        {
            rvp = SerializeReferenceValue_pop( &header->poolHead );
            if( !rvp )
            {
                ANY_LOG( 5, "Could not update format with value %s", ANY_LOG_WARNING, format );
            }
            else
            {
                SerializeReferenceValue_push( &header->listHead, rvp );
            }
        }
        SerializeReferenceValue_update( rvp, "format", (char *)format );
    }

    if( objSize > 0 )
    {
        header->objSize = objSize;
    }

    SERIALIZE_START_HEADER_SWITCH( header ) ;

            SERIALIZE_START_HANDLE_VERSION( 1, 0 ) ;
                {
                    /* Every time header changes, the size also must be updated */
                    header->headerSize = Any_snprintf( buffer,
                                                       SERIALIZE_HEADER_MAXLEN - 1,
                                                       SERIALIZE_HEADER_PREAMBLE
                                                               "%d.%d %s %s %10ld %s %s\n",
                                                       header->majVersion, header->minVersion,
                                                       ( type ? type : "" ),
                                                       ( name ? name : "" ), objSize,
                                                       ( format ? format : "" ),
                                                       ( opts ? opts : "" ));
                }
            SERIALIZE_STOP_HANDLE_VERSION();

            SERIALIZE_START_HANDLE_VERSION( 2, 0 ) ;
                {
                    SerializeHeader_updateHeaderSize( self );
                }
            SERIALIZE_STOP_HANDLE_VERSION();

            SERIALIZE_STOP_HEADER_SWITCH();

}


static void SerializeHeader_destroy( Serialize *self )
{
    SERIALIZE_TRACE_FUNCTION( "SerializeHeader_destroy" );

    ANY_REQUIRE( self );

    SerializeReferenceValue_destroyList( self->header->listHead );
    self->header->listHead = NULL;
    SerializeReferenceValue_destroyList( self->header->poolHead );
    self->header->poolHead = NULL;

    ANY_FREE( self->header );
}


static long Serialize_getTypeMaxSizeAsAscii( SerializeType type )
{
    long retVal = 0;

    SERIALIZE_TRACE_FUNCTION( "Serialize_getTypeMaxSizeAsAscii" );

    /* Warning: for array types the retVal is equal to the single element max lenght*/

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_CHAR;
            break;

        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_CHAR;
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_CHAR;
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_SINT;
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_USINT;
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_INT;
            break;
        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_UINT;
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_LINT;
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_ULINT;
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_LL;
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_FLOAT;
            break;

        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_FLOAT;
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_DOUBLE;
            break;

        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
            retVal = SERIALIZE_TYPEMAXTEXTLEN_LDOUBLE;
            break;

        case SERIALIZE_TYPE_STRING :
            /* Size is not used on Strings.. */
            /* ANY_LOG( 0, "Usage of String field type detected, please convert it to a char array!", ANY_LOG_WARNING ); */
            retVal = SERIALIZE_TYPEMAXTEXTLEN_STRING;
            break;

        default:
            ANY_LOG( 0, "Serialize_getTypeMaxSizeAsAscii. Unknown SerializeType : %d",
                     ANY_LOG_FATAL, type );
            ANY_REQUIRE( NULL );
            break;
    }

    return retVal;
}


/*---------------------------------------------------------------------------*/
/* Serialization functions for low-level datatypes                           */
/*---------------------------------------------------------------------------*/


void Char_serialize( char *value, const char *name,
                     Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_CHAR,
                           name, value, sizeof( char ), 1 );
}


void SChar_serialize( signed char *value, const char *name,
                      Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_SCHAR,
                           name, value, sizeof( signed char ), 1 );
}


void UChar_serialize( unsigned char *value, const char *name,
                      Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_UCHAR,
                           name, value, sizeof( unsigned char ), 1 );
}


void SInt_serialize( short int *value, const char *name,
                     Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_SINT,
                           name, value, sizeof( short int ), 1 );
}


void USInt_serialize( unsigned short int *value, const char *name,
                      Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_USINT,
                           name, value, sizeof( unsigned short int ), 1 );
}


void Int_serialize( int *value, const char *name, Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_INT,
                           name, value, sizeof( int ), 1 );
}


void UInt_serialize( unsigned int *value, const char *name,
                     Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_UINT,
                           name, value, sizeof( unsigned int ), 1 );
}


void LInt_serialize( long int *value, const char *name,
                     Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LINT,
                           name, value, sizeof( long int ), 1 );
}


void ULInt_serialize( unsigned long int *value, const char *name,
                      Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_ULINT,
                           name, value, sizeof( unsigned long int ), 1 );
}


void LL_serialize( long long *value, const char *name,
                   Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LL,
                           name, value, sizeof( long long ), 1 );
}


void ULL_serialize( unsigned long long *value, const char *name,
                    Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_ULL,
                           name, value, sizeof( unsigned long long ), 1 );
}


void Float_serialize( float *value, const char *name,
                      Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_FLOAT,
                           name, value, sizeof( float ), 1 );
}


void Double_serialize( double *value, const char *name,
                       Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_DOUBLE,
                           name, value, sizeof( double ), 1 );
}


void LDouble_serialize( long double *value, const char *name,
                        Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LDOUBLE,
                           name, value, sizeof( long double ), 1 );
}


void String_serialize( char *value, const char *name, int stringLen,
                       Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_STRING,
                           name, value, sizeof( char ), stringLen );
}


void CharArray_serialize( char *value, const char *name, int arrayLen,
                          Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_CHARARRAY,
                           name, value, sizeof( char ), arrayLen );
}


void SCharArray_serialize( signed char *value, const char *name, int arrayLen,
                           Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_SCHARARRAY,
                           name, value, sizeof( unsigned char ), arrayLen );
}


void UCharArray_serialize( unsigned char *value, const char *name, int arrayLen,
                           Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_UCHARARRAY,
                           name, value, sizeof( unsigned char ), arrayLen );
}


void SIntArray_serialize( short int *value, const char *name, int arrayLen,
                          Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_SINTARRAY,
                           name, value, sizeof( short int ), arrayLen );
}


void USIntArray_serialize( unsigned short int *value, const char *name,
                           int arrayLen, Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_USINTARRAY, name,
                           value, sizeof( unsigned short int ), arrayLen );
}


void IntArray_serialize( int *value, const char *name, int arrayLen,
                         Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_INTARRAY,
                           name, value, sizeof( int ), arrayLen );
}


void UIntArray_serialize( unsigned int *value, const char *name, int arrayLen,
                          Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_UINTARRAY,
                           name, value, sizeof( unsigned int ), arrayLen );
}


void LIntArray_serialize( long int *value, const char *name, int arrayLen,
                          Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LINTARRAY,
                           name, value, sizeof( long int ), arrayLen );
}


void ULIntArray_serialize( unsigned long int *value, const char *name,
                           int arrayLen, Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_ULINTARRAY,
                           name, value, sizeof( unsigned long int ), arrayLen );
}


void LLArray_serialize( long long int *value, const char *name, int arrayLen,
                        Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LLARRAY,
                           name, value, sizeof( long long int ), arrayLen );
}


void ULLArray_serialize( unsigned long long int *value, const char *name, int arrayLen,
                         Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_ULLARRAY,
                           name, value, sizeof( unsigned long long int ), arrayLen );
}


void FloatArray_serialize( float *value, const char *name, int arrayLen,
                           Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_FLOATARRAY,
                           name, value, sizeof( float ), arrayLen );
}


void DoubleArray_serialize( double *value, const char *name, int arrayLen,
                            Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_DOUBLEARRAY,
                           name, value, sizeof( double ), arrayLen );
}


void LDoubleArray_serialize( long double *value, const char *name, int arrayLen,
                             Serialize *serialize )
{
    Serialize_doSerialize( serialize, SERIALIZE_TYPE_LDOUBLEARRAY,
                           name, value, sizeof( long double ), arrayLen );
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


/* EOF */
