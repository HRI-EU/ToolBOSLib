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


/*--------------------------------------------------------------------------*/
/* Include files                                                            */
/*--------------------------------------------------------------------------*/


#include <SerializeUtility.h>

#include <FileSystem.h>


/*--------------------------------------------------------------------------*/
/* Private constants                                                        */
/*--------------------------------------------------------------------------*/


#define SERIALIZEUTILITY_VALID              ( 0x998877 )
#define SERIALIZEUTILITY_INVALID            ( 0x112233 )

#define SERIALIZEUTILITY_LOGLEVEL_CRITICAL  ( 0 )
#define SERIALIZEUTILITY_LOGLEVEL_DEFAULT   ( 3 )
#define SERIALIZEUTILITY_LOGLEVEL_DEBUG     ( 5 )

#define SERIALIZEUTILITY_DATAFORMAT_DEFAULT ( "Ascii" )
#define SERIALIZEUTILITY_DATANAME_DEFAULT   ( "data" )


/*--------------------------------------------------------------------------*/
/* Private functions prototypes                                             */
/*--------------------------------------------------------------------------*/


void SerializeUtility_setupDeserializer( SerializeUtility *self );

BaseBool SerializeUtility_setupSerializer( SerializeUtility *self,
                                           const char *outputUrl );

void SerializeUtility_closeSerializer( SerializeUtility *self );

void SerializeUtility_closeDeserializer( SerializeUtility *self );

void SerializeUtility_checkInputFile( SerializeUtility *self );

void SerializeUtility_setOnDeserialize( SerializeUtility *self,
                                        BaseBool (*onDeserialize)( struct SerializeUtility *self ));

DynamicLoaderFunction SerializeUtility_getSymbolByName( SerializeUtility *self,
                                                        const char *prefix,
                                                        const char *suffix );

BaseBool SerializeUtility_detectDataType( SerializeUtility *self );

BaseBool SerializeUtility_processFile( SerializeUtility *self );


/*--------------------------------------------------------------------------*/
/* Public function implementations                                          */
/*--------------------------------------------------------------------------*/


SerializeUtility *SerializeUtility_new( void )
{
    return ANY_TALLOC( SerializeUtility );
}


void SerializeUtility_init( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid != SERIALIZEUTILITY_VALID );

    self->nullChannel = IOChannel_new();
    IOChannel_init( self->nullChannel );

    self->inputChannel = IOChannel_new();
    IOChannel_init( self->inputChannel );

    self->outputChannel = IOChannel_new();
    IOChannel_init( self->outputChannel );

    self->deserializer = Serialize_new();
    Serialize_init( self->deserializer,
                    NULL,
                    SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_READ );

    self->serializer = Serialize_new();
    Serialize_init( self->serializer,
                    NULL,
                    SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );


    Any_memset( self->bbdmType, 0x00, SERIALIZEUTILITY_DATATYPE_MAXLEN );
    Any_memset( self->payloadType, 0x00, SERIALIZEUTILITY_DATATYPE_MAXLEN );
    Any_memset( self->dataName, 0x00, SERIALIZEUTILITY_DATANAME_MAXLEN );
    Any_memset( self->inputDataFormat, 0x00, SERIALIZEUTILITY_FORMAT_MAXLEN );
    Any_memset( self->inputFile, 0x00, SERIALIZEUTILITY_FILENAME_MAXLEN );
    Any_memset( self->initString, 0x00, SERIALIZEUTILITY_INITSTRING_MAXLEN );
    Any_memset( self->outputDataFormat, 0x00, SERIALIZEUTILITY_FORMAT_MAXLEN );
    Any_memset( self->outputFile, 0x00, SERIALIZEUTILITY_FILENAME_MAXLEN );

    Any_strncpy( self->outputDataFormat,
                 SERIALIZEUTILITY_DATAFORMAT_DEFAULT,
                 SERIALIZEUTILITY_FORMAT_MAXLEN - 1 );

    Any_strncpy( self->dataName,
                 SERIALIZEUTILITY_DATANAME_DEFAULT,
                 SERIALIZEUTILITY_DATANAME_MAXLEN - 1 );

    IOChannel_openFromString( self->nullChannel, "stream=Null://" );
    Serialize_setStream( self->serializer, self->nullChannel );
    Serialize_setStream( self->deserializer, self->nullChannel );

    self->dynamicLoader = (DynamicLoader *)NULL;
    self->bbdmFunc_new = NULL;
    self->bbdmFunc_initFromString = NULL;
    self->bbdmFunc_indirectRand = NULL;
    self->bbdmFunc_indirectSerialize = NULL;
    self->bbdmFunc_clear = NULL;
    self->bbdmFunc_delete = NULL;
    self->bbdmFunc_getData = NULL;
    self->elementsDone = 0;
    self->fileSize = 0;
    self->delay = 0;
    self->inputIsBBDM = false;
    self->interactive = false;
    self->maxElements = BASEUI32_MAX;
    self->onDeserialize = NULL;
    self->outputIsBBDM = false;
    self->payloadFunc_serialize = NULL;
    self->randomSeedState = (BaseUI32)getpid();
    self->tmpObject = NULL;
    self->useRandomization = false;
    self->valueMin = 0;
    self->valueMax = 1000;
    self->valid = SERIALIZEUTILITY_VALID;
}


void SerializeUtility_clear( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->deserializer != NULL );
    ANY_REQUIRE( self->serializer != NULL );
    ANY_REQUIRE( self->inputChannel != NULL );
    ANY_REQUIRE( self->outputChannel != NULL );

    Serialize_clear( self->deserializer );
    Serialize_delete( self->deserializer );

    Serialize_clear( self->serializer );
    Serialize_delete( self->serializer );

    IOChannel_clear( self->inputChannel );
    IOChannel_delete( self->inputChannel );

    IOChannel_clear( self->outputChannel );
    IOChannel_delete( self->outputChannel );

    IOChannel_close( self->nullChannel );
    IOChannel_clear( self->nullChannel );
    IOChannel_delete( self->nullChannel );

    // if the SerializeUtil is not initialized with any content
    // the dynamicLoader is not initialized since it depends
    // on the data type of the content
    if( self->dynamicLoader != (DynamicLoader *)NULL)
    {
        DynamicLoader_clear( self->dynamicLoader );
        DynamicLoader_delete( self->dynamicLoader );
    }

    self->valid = SERIALIZEUTILITY_INVALID;
}


void SerializeUtility_delete( SerializeUtility *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseBool SerializeUtility_detectDataType( SerializeUtility *self )
{
    BaseI32 size = -1;
    bool status = false;
    BaseBool retVal = true;
    Serialize *deserializer = (Serialize *)NULL;
    IOChannel *channel = (IOChannel *)NULL;
    char *openString = (char *)NULL;
    char *readType = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    openString = (char *)ANY_BALLOC( SERIALIZEUTILITY_FILENAME_MAXLEN + 7 );  /* 7 = "File://" */
    readType = (char *)ANY_BALLOC( SERIALIZEUTILITY_DATATYPE_MAXLEN );

    ANY_REQUIRE( openString );
    ANY_REQUIRE( readType );

    // read the header from the buffer to determine the dataType

    channel = IOChannel_new();
    ANY_REQUIRE_MSG( channel, "Could not instantiate IOChannel instance" );

    IOChannel_init( channel );

    Any_snprintf( openString, SERIALIZEUTILITY_FILENAME_MAXLEN + 7 - 1, "File://%s", self->inputFile );
    status = IOChannel_open( channel, openString, IOCHANNEL_MODE_R_ONLY, IOCHANNEL_PERMISSIONS_ALL);
    ANY_REQUIRE_MSG( status == true, "Unable to open the IOChannel." );

    deserializer = Serialize_new();
    Serialize_init( deserializer, channel, SERIALIZE_STREAMMODE_NORMAL );
    Serialize_setMode( deserializer, SERIALIZE_MODE_READ );

    if( Serialize_peekHeader( deserializer, readType, self->dataName, &size, self->inputDataFormat, NULL) == false )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL, "An error occurred while calling Serialize#peekHeader.",
                 ANY_LOG_ERROR );
        retVal = false;
        goto out;
    }

    if( Any_strncmp( readType, "", SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 ) == 0 )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect dataType inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }

    if( self->dataName == (const char *)NULL)
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect dataName inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }


    if( self->inputDataFormat == (const char *)NULL)
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect inputDataFormat inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }

    if( Any_strncmp( readType, "", SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 ) == 0 )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect dataType inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }

    if( Any_strncmp( self->dataName, "", SERIALIZEUTILITY_DATANAME_MAXLEN - 1 ) == 0 )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect dataName inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }

    if( Any_strncmp( self->inputDataFormat, "", SERIALIZEUTILITY_FORMAT_MAXLEN - 1 ) == 0 )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Unable to detect inputDataFormat inside %s",
                 ANY_LOG_ERROR, self->inputFile );
        retVal = false;
        goto out;
    }

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", readType );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->inputFile );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->dataName );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->inputDataFormat );

    retVal = SerializeUtility_setInputDataType( self, readType );

    out:
    IOChannel_close( channel );
    IOChannel_clear( channel );
    IOChannel_delete( channel );

    Serialize_clear( deserializer );
    Serialize_delete( deserializer );

    ANY_FREE( openString );
    ANY_FREE( readType );

    return retVal;
}


BaseI32 SerializeUtility_detectDatatypeInFile( SerializeUtility *self )
{
    BaseI32 returnValue = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE_MSG( self->inputFile != (const char *)NULL,
                     "Please specify an input file!" );
    ANY_REQUIRE_MSG( Any_strlen( self->inputFile ) > 0,
                     "Please specify an input file!" );

    // verify if the file exists at all
    if( FileSystem_isRegularFile( self->inputFile ) == true )
    {
        // read the header from the serialized stream to determine the dataType
        if( SerializeUtility_detectDataType( self ) == false )
        {
            returnValue = -2;
        }
        else
        {
            returnValue = 0;
        }
    }

    return returnValue;
}


BaseBool SerializeUtility_deserializeFromFile( SerializeUtility *self )
{
    BaseBool returnValue = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    if( self->inputIsBBDM == true )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                 "start deserializing element #%d into BBDM",
                 ANY_LOG_INFO,
                 self->elementsDone );

        self->bbdmFunc_indirectSerialize( self->tmpObject,
                                          self->dataName,
                                          self->deserializer );

        if( IOChannel_eof( Serialize_getStream( self->deserializer )) == true )
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "EOF found", ANY_LOG_INFO );
            returnValue = true;
        }
        else if( Serialize_isErrorOccurred( self->deserializer ) == false )
        {
            returnValue = true;
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                     "done with deserializing into BBDM",
                     ANY_LOG_INFO );
        }
        else
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                     "error while deserializing into BBDM",
                     ANY_LOG_ERROR );
        }
    }
    else
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                 "start deserializing element #%d into raw data struct",
                 ANY_LOG_INFO,
                 self->elementsDone );

        self->payloadFunc_serialize( self->tmpObject,
                                     self->dataName,
                                     self->deserializer );

        if( IOChannel_eof( Serialize_getStream( self->deserializer )) == true )
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "EOF found", ANY_LOG_INFO );
            returnValue = true;
        }
        else if( Serialize_isErrorOccurred( self->deserializer ) == false )
        {
            returnValue = true;
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                     "done with serializing into raw data struct",
                     ANY_LOG_INFO );
        }
        else
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                     "error while serializing into raw data struct",
                     ANY_LOG_ERROR );
        }
    }
    return returnValue;
}


BaseBool SerializeUtility_processFile( SerializeUtility *self )
{
    BaseBool serResult = false;
    BaseBool returnValue = false;
    char key = '\0';

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->bbdmFunc_getData != NULL );
    ANY_REQUIRE( self->bbdmFunc_indirectSerialize != NULL );
    ANY_REQUIRE( self->payloadFunc_serialize != NULL );
    ANY_REQUIRE( self->onDeserialize != NULL );
    ANY_REQUIRE( Any_strlen( self->dataName ) > 0 );
    ANY_REQUIRE( Any_strlen( self->inputFile ) > 0 );
    ANY_REQUIRE( self->fileSize > 0 );
    ANY_REQUIRE( self->deserializer != (Serialize *)NULL );
    ANY_REQUIRE( self->serializer != (Serialize *)NULL );
    ANY_REQUIRE( self->inputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->outputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->tmpObject != (void *)NULL );

    while(( Serialize_isErrorOccurred( self->deserializer ) == false ) &&
          ( key != 'q' ) &&
          ( self->elementsDone < self->maxElements ))
    {
        serResult = SerializeUtility_deserializeFromFile( self );

        ANY_REQUIRE_MSG( serResult == true, "error while serializing" );

        if( IOChannel_eof( Serialize_getStream( self->deserializer )) == true )
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "EOF found", ANY_LOG_INFO );
            returnValue = true;
            break;
        }

        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                 "calling onDeserialize callback",
                 ANY_LOG_INFO );

        self->onDeserialize( self );

        if( self->interactive == true )
        {
            printf( "\nPress any key to continue ('q' to quit)... " );
            key = (char)getc( stdin );
        }

        ( self->elementsDone )++;
    }


    SerializeUtility_closeDeserializer( self );
    SerializeUtility_closeSerializer( self );

    return returnValue;
}


void SerializeUtility_create( SerializeUtility *self )
{
    BaseUI32 i = 0;
    char outputUrl[SERIALIZEUTILITY_FILENAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( Any_strlen( self->bbdmType ) > 0 );
    ANY_REQUIRE( Any_strlen( self->dataName ) > 0 );
    ANY_OPTIONAL( Any_strlen( self->initString ) > 0 );

    /* do not create "infinite" numbers by default, but just one */
    if( self->maxElements == BASEUI32_MAX)
    {
        self->maxElements = 1;
    }


    /* discover function pointers, create and initialize a new BBDM */
    SerializeUtility_detectFunctions( self );
    SerializeUtility_constructObject( self );
    SerializeUtility_initializeObject( self );


    /* setup the serializer */
    if( Any_strlen( self->outputFile ) > 0 )
    {
        Any_snprintf( outputUrl,
                      SERIALIZEUTILITY_FILENAME_MAXLEN - 1,
                      "File://%s",
                      self->inputFile );
    }
    else
    {
        Any_strncpy( outputUrl,
                     "StdOut://",
                     SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );
    }
    SerializeUtility_setupSerializer( self, outputUrl );


    /* generate elements */
    for( i = 0; i < self->maxElements; i++ )
    {

        /* if requested by user, call the randomizer on the initialized object */
        if( self->useRandomization == true )
        {
            self->bbdmFunc_indirectRand( self->tmpObject,
                                         self->valueMin,
                                         self->valueMax,
                                         &( self->randomSeedState ));
        }


        if( self->outputIsBBDM == true )
        {
            /* serialize the entire BBDM */
            self->bbdmFunc_indirectSerialize( self->tmpObject,
                                              self->dataName,
                                              self->serializer );
        }
        else
        {
            /* serialize only the the inner payload if requested */
            self->payloadFunc_serialize( self->bbdmFunc_getData( self->tmpObject ),
                                         self->dataName,
                                         self->serializer );
        }
    }

    /* destroy the BBDM */
    SerializeUtility_destroyObject( self );
}


void SerializeUtility_print( SerializeUtility *self )
{
    BaseI32 status = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    SerializeUtility_checkInputFile( self );
    status = SerializeUtility_detectDatatypeInFile( self );

    if( status == 0 )
    {
        SerializeUtility_detectFunctions( self );
        SerializeUtility_constructObject( self );

        SerializeUtility_setOnDeserialize( self, &SerializeUtility_serializeElementToOutput );
        SerializeUtility_setupDeserializer( self );
        SerializeUtility_setupSerializer( self, "StdOut://" );

        SerializeUtility_processFile( self );
    }
    else if( status == -1 )
    {
        ANY_LOG( 0, "The input file '%s' does not exist or is not readable",
                 ANY_LOG_ERROR, self->inputFile );
    }
    else if( status == -2 )
    {
        /* problem loading dynamic libraries, message was already printed by
         * SerializeUtility_setInputDataType() invoked from
         * SerializeUtility_detectDatatypeInFile()
         */
        /* ERROR CASE */
    }
    else
    {
        ANY_LOG( 0, "The input file '%s' does not contain valid data",
                 ANY_LOG_ERROR, self->inputFile );
    }
}


void SerializeUtility_convert( SerializeUtility *self )
{
    BaseI32 fileStatus = -1;
    char outputUrl[SERIALIZEUTILITY_FILENAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->inputFile != (const char *)NULL );
    ANY_REQUIRE( self->outputFile != (const char *)NULL );

    ANY_REQUIRE_MSG( Any_strlen( self->inputFile ) > 0, "No input file specified" );
    //ANY_REQUIRE_MSG( Any_strlen( self->outputFile ) > 0, "No output file specified" );

    ANY_REQUIRE( Any_strlen( self->inputFile ) < SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );
    //ANY_REQUIRE( Any_strlen( self->outputFile ) < SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );

    ANY_REQUIRE_MSG( Any_strncmp( self->inputFile,
                                  self->outputFile,
                                  SERIALIZEUTILITY_FILENAME_MAXLEN ) != 0,
                     "Input and output file must not be the same." );

    SerializeUtility_checkInputFile( self );
    fileStatus = SerializeUtility_detectDatatypeInFile( self );

    if( fileStatus == 0 )
    {
        SerializeUtility_detectFunctions( self );
        SerializeUtility_constructObject( self );

        SerializeUtility_setOnDeserialize( self, &SerializeUtility_serializeElementToOutput );
        SerializeUtility_setupDeserializer( self );

        if( Any_strlen( self->outputFile ) > 0 )
        {
            Any_snprintf( outputUrl,
                          SERIALIZEUTILITY_FILENAME_MAXLEN - 1,
                          "File://%s",
                          self->inputFile );
        }
        else
        {
            Any_strncpy( outputUrl,
                         "StdOut://",
                         SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );
        }
        SerializeUtility_setupSerializer( self, outputUrl );

        SerializeUtility_processFile( self );
    }
    else if( fileStatus == -1 )
    {
        ANY_LOG( 0, "The input file '%s' does not exist or is not readable",
                 ANY_LOG_ERROR, self->inputFile );
    }
    else
    {
        ANY_LOG( 0, "The input file '%s' does not contain valid data",
                 ANY_LOG_ERROR, self->inputFile );
    }
}


void SerializeUtility_setInputFile( SerializeUtility *self,
                                    const char *filename )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( filename != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( filename ) < SERIALIZEUTILITY_FILENAME_MAXLEN );
    ANY_REQUIRE( Any_strlen( filename ) > 0 );

    Any_strncpy( self->inputFile, filename, SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->inputFile );
}


void SerializeUtility_setOutputFile( SerializeUtility *self,
                                     const char *filename )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( filename != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( filename ) < SERIALIZEUTILITY_FILENAME_MAXLEN );
    ANY_REQUIRE( Any_strlen( filename ) > 0 );

    Any_strncpy( self->outputFile,
                 filename,
                 SERIALIZEUTILITY_FILENAME_MAXLEN - 1 );

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->outputFile );
}


void SerializeUtility_setMaxElements( SerializeUtility *self,
                                      unsigned int count )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( count > 0 );
    ANY_REQUIRE( count < 1000000 );

    self->maxElements = count;
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d", self->maxElements );
}


void SerializeUtility_setInteractiveMode( SerializeUtility *self,
                                          BaseBool interactive )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    self->interactive = interactive;
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d", self->interactive );
}


void SerializeUtility_setDataName( SerializeUtility *self,
                                   const char *name )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( name != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( name ) < SERIALIZEUTILITY_DATANAME_MAXLEN - 1 );

    Any_strncpy( self->dataName,
                 name,
                 SERIALIZEUTILITY_DATANAME_MAXLEN - 1 );

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->dataName );
}


void SerializeUtility_setDelay( SerializeUtility *self,
                                BaseUI32 milliSeconds )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( milliSeconds < 60000 );  // max. 1 minute delay

    self->delay = milliSeconds;
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d", self->delay );
}


BaseBool SerializeUtility_setInputDataType( SerializeUtility *self,
                                            const char *datatype )
{
    char libName[SERIALIZEUTILITY_FILENAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( datatype != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( datatype ) < SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );
    ANY_REQUIRE( Any_strlen( datatype ) > 0 );
    ANY_REQUIRE( self->dynamicLoader == (DynamicLoader *)NULL );


    /*
     * if requested datatype is not a BBDM, internally use a BBDM to
     * construct the data but then only serialize the inner payload of
     * it to output file or so
     *
     */
    if( Any_strstr( datatype, "BBDM" ) == NULL)
    {
        self->inputIsBBDM = false;

        Any_snprintf( self->bbdmType,
                      SERIALIZEUTILITY_DATATYPE_MAXLEN - 1,
                      "BBDM%s",
                      datatype );

        Any_strncpy( self->payloadType,
                     datatype,
                     SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );
    }
    else
    {
        self->inputIsBBDM = true;

        Any_strncpy( self->bbdmType,
                     datatype,
                     SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );

        /* SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE - 4 = 2043 */
        Any_sscanf( datatype, "BBDM%2043s", self->payloadType );
    }

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d", self->inputIsBBDM );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->bbdmType );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->payloadType );

    Any_snprintf( libName, SERIALIZEUTILITY_FILENAME_MAXLEN,
                  "lib%s.so", self->bbdmType );

    self->dynamicLoader = DynamicLoader_new();

    if( DynamicLoader_init( self->dynamicLoader, libName ))
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Could not load data library '%s' (%s): Reason '%s'",
                 ANY_LOG_ERROR,
                 self->bbdmType,
                 libName,
                 DynamicLoader_getError( self->dynamicLoader ));

        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                 "Please make sure that the library is found in LD_LIBRARY_PATH",
                 ANY_LOG_ERROR );

        DynamicLoader_delete( self->dynamicLoader );
        self->dynamicLoader = (DynamicLoader *)NULL;

        return false;
    }
    else
    {
        return true;
    }
}


void SerializeUtility_setOutputDataType( SerializeUtility *self,
                                         const char *datatype )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( datatype != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( datatype ) < SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );
    ANY_REQUIRE( Any_strlen( datatype ) > 0 );

    /*
     * if requested datatype is not a BBDM, internally use a BBDM to
     * construct the data but then only serialize the inner payload of
     * it to output file or so
     *
     */
    if( Any_strstr( datatype, "BBDM" ) == NULL)
    {
        self->outputIsBBDM = false;

        Any_snprintf( self->bbdmType,
                      SERIALIZEUTILITY_DATATYPE_MAXLEN - 1,
                      "BBDM%s",
                      datatype );

        Any_strncpy( self->payloadType,
                     datatype,
                     SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );
    }
    else
    {
        self->outputIsBBDM = true;

        Any_strncpy( self->bbdmType,
                     datatype,
                     SERIALIZEUTILITY_DATATYPE_MAXLEN - 1 );

        /* SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE - 4 = 2043 */
        Any_sscanf( datatype, "BBDM%2043s", self->payloadType );
    }

    if( Any_strlen( self->dataName ) == 0 )
    {
        Any_strncpy( self->dataName, datatype, SERIALIZEUTILITY_DATANAME_MAXLEN );
    }

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d", self->outputIsBBDM );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->bbdmType );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->payloadType );
}


void SerializeUtility_setInputDataFormat( SerializeUtility *self,
                                          const char *format )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( format != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( format ) < SERIALIZEUTILITY_FORMAT_MAXLEN );
    ANY_REQUIRE( Any_strlen( format ) > 0 );

    Any_strncpy( self->inputDataFormat,
                 format,
                 SERIALIZEUTILITY_FORMAT_MAXLEN - 1 );

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%s", self->inputDataFormat );
}


void SerializeUtility_setOutputDataFormat( SerializeUtility *self,
                                           const char *format )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( format != (const char *)NULL );
    ANY_REQUIRE( Any_strlen( format ) < SERIALIZEUTILITY_FORMAT_MAXLEN );
    ANY_REQUIRE( Any_strlen( format ) > 0 );

    Any_strncpy( self->outputDataFormat,
                 format,
                 SERIALIZEUTILITY_FORMAT_MAXLEN - 1 );
}


void SerializeUtility_setInitString( SerializeUtility *self,
                                     const char *initString )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( initString != (const char *)NULL );

    Any_strncpy( self->initString,
                 initString,
                 SERIALIZEUTILITY_INITSTRING_MAXLEN - 1 );
}


void SerializeUtility_setOnDeserialize( SerializeUtility *self,
                                        BaseBool (*onDeserialize)( struct SerializeUtility *self ))
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( onDeserialize != NULL );

    self->onDeserialize = onDeserialize;
}


void SerializeUtility_setRandomization( SerializeUtility *self,
                                        BaseBool useRandomization )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    self->useRandomization = useRandomization;
}


/*--------------------------------------------------------------------------*/
/* Private function implementations                                         */
/*--------------------------------------------------------------------------*/


BaseBool SerializeUtility_serializeElementToOutput( SerializeUtility *self )
{
    BaseBool returnValue = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->dataName != (const char *)NULL );
    ANY_REQUIRE( self->bbdmFunc_getData != NULL );
    ANY_REQUIRE( self->bbdmFunc_indirectSerialize != NULL );
    ANY_REQUIRE( self->payloadFunc_serialize != NULL );
    ANY_REQUIRE( self->serializer != (Serialize *)NULL );
    ANY_REQUIRE( self->tmpObject != (void *)NULL );

    if( self->inputIsBBDM == true )
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                 "serializing BBDM (#%d)",
                 ANY_LOG_INFO,
                 self->elementsDone );

        self->bbdmFunc_indirectSerialize( self->tmpObject,
                                          self->dataName,
                                          self->serializer );

        if( Serialize_isErrorOccurred( self->serializer ) == false )
        {
            returnValue = true;
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                     "done with serializing",
                     ANY_LOG_INFO );
        }
        else
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                     "error while serializing",
                     ANY_LOG_ERROR );
            SerializeUtility_closeSerializer( self );
        }
    }
    else
    {
        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                 "serializing raw struct (#%d)",
                 ANY_LOG_INFO,
                 self->elementsDone );

        self->payloadFunc_serialize( self->tmpObject,
                                     self->dataName,
                                     self->serializer );

        if( Serialize_isErrorOccurred( self->serializer ) == false )
        {
            returnValue = true;
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEBUG,
                     "done with serializing",
                     ANY_LOG_INFO );
        }
        else
        {
            ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_CRITICAL,
                     "error while serializing",
                     ANY_LOG_ERROR );
            SerializeUtility_closeSerializer( self );
        }
    }

    return returnValue;
}


void SerializeUtility_detectFunctions( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( Any_strlen( self->bbdmType ) > 0 );
    ANY_REQUIRE( Any_strlen( self->payloadType ) > 0 );

    ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEFAULT,
             "detecting BBDM functions",
             ANY_LOG_INFO );

    self->bbdmFunc_new = (BBDMNewFunc)SerializeUtility_getSymbolByName( self, self->bbdmType, "new" );
    self->bbdmFunc_initFromString = (BBDMInitFromStringFunc)SerializeUtility_getSymbolByName( self, self->bbdmType,
                                                                                              "initFromString" );
    self->bbdmFunc_clear = (BBDMClearFunc)SerializeUtility_getSymbolByName( self, self->bbdmType, "clear" );
    self->bbdmFunc_delete = (BBDMDeleteFunc)SerializeUtility_getSymbolByName( self, self->bbdmType, "delete" );
    self->bbdmFunc_getData = (BBDMGetDataFunc)SerializeUtility_getSymbolByName( self, self->bbdmType, "getData" );
    self->bbdmFunc_indirectRand = (BBDMRandFunc)SerializeUtility_getSymbolByName( self, self->bbdmType,
                                                                                  "indirectRand" );
    self->bbdmFunc_indirectSerialize = (SerializeFunction)SerializeUtility_getSymbolByName( self, self->bbdmType,
                                                                                            "indirectSerialize" );

    self->payloadFunc_serialize = (SerializeFunction)SerializeUtility_getSymbolByName( self, self->payloadType,
                                                                                       "indirectSerialize" );
}


DynamicLoaderFunction SerializeUtility_getSymbolByName( SerializeUtility *self,
                                                        const char *prefix,
                                                        const char *suffix )
{
    DynamicLoaderFunction ptr = (DynamicLoaderFunction)NULL;
    const char *dataType = (const char *)NULL;
    char symbolName[SERIALIZEUTILITY_SYMBOLNAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( Any_strlen( self->payloadType ) > 0 );
    ANY_REQUIRE( Any_strlen( self->bbdmType ) > 0 );
    ANY_REQUIRE( prefix != (const char *)NULL );
    ANY_REQUIRE( suffix != (const char *)NULL );

    Any_memset( symbolName, 0x00, SERIALIZEUTILITY_SYMBOLNAME_MAXLEN );
    dataType = self->outputIsBBDM ? self->bbdmType : self->payloadType;

    Any_snprintf( symbolName,
                  SERIALIZEUTILITY_SYMBOLNAME_MAXLEN - 1,
                  "%s_%s",
                  prefix,
                  suffix );

    ptr = DynamicLoader_getSymbolByName(NULL, symbolName );

    ANY_REQUIRE_VMSG( ptr, "%s: unsupported datatype (%s() not found)",
                      dataType, symbolName );

    return ptr;
}


// create a temp. object in memory
void SerializeUtility_constructObject( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->bbdmFunc_new != NULL );

    ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEFAULT,
             "constructing BBDM in memory",
             ANY_LOG_INFO );

    self->tmpObject = self->bbdmFunc_new();

    ANY_REQUIRE( self->tmpObject );
    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%p", self->tmpObject );
}


void SerializeUtility_initializeObject( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->bbdmFunc_initFromString != NULL );
    ANY_REQUIRE( self->tmpObject != (void *)NULL );
    ANY_REQUIRE( self->initString != (const char *)NULL );

    ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEFAULT,
             "initializing object",
             ANY_LOG_INFO );

    self->bbdmFunc_initFromString( self->tmpObject, self->initString );
}


void SerializeUtility_destroyObject( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->bbdmFunc_clear != NULL );
    ANY_REQUIRE( self->bbdmFunc_delete != NULL );
    ANY_REQUIRE( self->tmpObject != (void *)NULL );

    ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEFAULT,
             "destroying object",
             ANY_LOG_INFO );

    self->bbdmFunc_clear( self->tmpObject );
    self->bbdmFunc_delete( self->tmpObject );
}


// open the file for deserialization
void SerializeUtility_setupDeserializer( SerializeUtility *self )
{
    char url[SERIALIZEUTILITY_FILENAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->inputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->deserializer != (Serialize *)NULL );
    ANY_REQUIRE( Any_strlen( self->inputDataFormat ) > 0 );
    ANY_REQUIRE( Any_strlen( self->inputFile ) > 0 );

    Any_memset( url, 0x00, SERIALIZEUTILITY_FILENAME_MAXLEN );

    Any_snprintf( url,
                  SERIALIZEUTILITY_FILENAME_MAXLEN - 1,
                  "File://%s",
                  self->inputFile );

    IOChannel_open( self->inputChannel,
                    url,
                    IOCHANNEL_MODE_R_ONLY,
                    IOCHANNEL_PERMISSIONS_ALL);

    Serialize_setStream( self->deserializer, self->inputChannel );
    Serialize_setFormat( self->deserializer, self->inputDataFormat, "" );
    Serialize_setInitMode( self->deserializer, true );
}


BaseBool SerializeUtility_setupSerializer( SerializeUtility *self, const char *outputUrl )
{
    BaseBool returnValue = false;
    char filename[SERIALIZEUTILITY_FILENAME_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->outputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->serializer != (Serialize *)NULL );
    ANY_REQUIRE( Any_strlen( self->outputDataFormat ) > 0 );
    ANY_REQUIRE( outputUrl != (const char *)NULL );

    Any_memset( filename, 0x00, SERIALIZEUTILITY_FILENAME_MAXLEN );

    if( Any_strcmp( self->outputFile, "" ) != 0 )
    {
        Any_snprintf( filename,
                      SERIALIZEUTILITY_FILENAME_MAXLEN - 1,
                      "File://%s",
                      self->outputFile );

        ANY_LOG( SERIALIZEUTILITY_LOGLEVEL_DEFAULT,
                 "writing to %s",
                 ANY_LOG_DATA,
                 self->outputFile );

        returnValue = IOChannel_open( self->outputChannel,
                                      filename,
                                      IOCHANNEL_MODE_W_ONLY
                                      | IOCHANNEL_MODE_CREAT
                                      | IOCHANNEL_MODE_TRUNC,
                                      IOCHANNEL_PERMISSIONS_R_U
                                      | IOCHANNEL_PERMISSIONS_R_G
                                      | IOCHANNEL_PERMISSIONS_R_O
                                      | IOCHANNEL_PERMISSIONS_W_U );
    }
    else
    {
        returnValue = IOChannel_open( self->outputChannel,
                                      outputUrl, // something like "StdErr://",
                                      IOCHANNEL_MODE_W_ONLY,
                                      IOCHANNEL_PERMISSIONS_ALL);
    }

    if( returnValue == true )
    {
        Serialize_setStream( self->serializer, self->outputChannel );
        Serialize_setFormat( self->serializer, self->outputDataFormat, "" );
    }

    return returnValue;
}


// detach the file from the deserializer, and close the file
void SerializeUtility_closeDeserializer( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->inputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->deserializer != (Serialize *)NULL );
    ANY_REQUIRE( self->nullChannel != (IOChannel *)NULL );

    Serialize_setStream( self->deserializer, self->nullChannel );
    IOChannel_close( self->inputChannel );
}


void SerializeUtility_closeSerializer( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( self->outputChannel != (IOChannel *)NULL );
    ANY_REQUIRE( self->serializer != (Serialize *)NULL );
    ANY_REQUIRE( self->nullChannel != (IOChannel *)NULL );

    Serialize_setStream( self->serializer, self->nullChannel );
    IOChannel_close( self->outputChannel );

    Serialize_init( self->serializer,
                    NULL,
                    SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );
    Serialize_cleanError( self->serializer );
}


// compute the filesize to prevent reading behind EOF
void SerializeUtility_checkInputFile( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );
    ANY_REQUIRE( Any_strlen( self->inputFile ) > 0 );

    self->fileSize = FileSystem_getSize( self->inputFile );

    ANY_TRACE( SERIALIZEUTILITY_LOGLEVEL_DEBUG, "%d Bytes", self->fileSize );
}


void *SerializeUtility_getBBDM( SerializeUtility *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == SERIALIZEUTILITY_VALID );

    return self->tmpObject;
}


/* EOF */
