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


#ifndef SERIALIZEUTILITY_H
#define SERIALIZEUTILITY_H


#include <Any.h>
#include <Base.h>
#include <BBDM-C.h>
#include <IOChannel.h>
#include <Serialize.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * \page SerializeUtility_About Serialize Utility
 *
 * The <b>Serialize Utility</b> is a set of functions (and CLI-interfaces)
 * to generate, convert or print serialized data.
 *
 * \li it currently supports all Base- and BPL-types, and their BBDMs
 * \li supports creation of randomized data
 *
 * <h3>Example:</h3>
 *
 * \verbatim
   # create binary-serialized file with ten random float values
   CreateSerializedData -t BaseF32 -c 10 -f Binary -r -o myRandomData.bin


   # verbose creation of a BBDMBlockF32 with certain geometry (write to console)
   CreateSerializedData -t BBDMBlockF32 -i "width=640 height=480" -v


   # interactively print the first 5 elements from a serialized file onto console
   PrintSerializedData -c 5 -i -f ./example.bin
   \endverbatim
 */


/*! \brief max. length of a dataname parameter for a serialize function */
#define SERIALIZEUTILITY_DATANAME_MAXLEN   ( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE )

/*! \brief max. length of a datatype parameter for a serialize function */
#define SERIALIZEUTILITY_DATATYPE_MAXLEN   ( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE )

/*! \brief max. length of a path to a file */
#define SERIALIZEUTILITY_FILENAME_MAXLEN   ( 1024 )

/*! \brief max. length of the format-parameter to Serialize_setFormat() */
#define SERIALIZEUTILITY_FORMAT_MAXLEN     ( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE )

/*! \brief max. length of an initString passed to a BBDM's _initFromString()*/
#define SERIALIZEUTILITY_INITSTRING_MAXLEN ( 1024 )

/*! \brief max. length of a symbol (e.g. functionname) in a shared library */
#define SERIALIZEUTILITY_SYMBOLNAME_MAXLEN ( 256 )


typedef struct SerializeUtility
{
    unsigned long valid;

    char bbdmType[SERIALIZEUTILITY_DATATYPE_MAXLEN];
    char payloadType[SERIALIZEUTILITY_DATATYPE_MAXLEN];
    char dataName[SERIALIZEUTILITY_DATANAME_MAXLEN];
    char inputDataFormat[SERIALIZEUTILITY_FORMAT_MAXLEN];
    char outputDataFormat[SERIALIZEUTILITY_FORMAT_MAXLEN];
    char inputFile[SERIALIZEUTILITY_FILENAME_MAXLEN];
    char outputFile[SERIALIZEUTILITY_FILENAME_MAXLEN];
    char initString[SERIALIZEUTILITY_INITSTRING_MAXLEN];

    BaseUI32 maxElements;
    BaseUI32 elementsDone;
    BaseI32 fileSize;

    IOChannel *nullChannel;  // used temp. while the (de-)serializers don't have a valid stream
    IOChannel *inputChannel;
    IOChannel *outputChannel;
    Serialize *serializer;
    Serialize *deserializer;
    DynamicLoader *dynamicLoader;

    BBDMNewFunc bbdmFunc_new;
    BBDMInitFromStringFunc bbdmFunc_initFromString;
    BBDMClearFunc bbdmFunc_clear;
    BBDMDeleteFunc bbdmFunc_delete;
    BBDMGetDataFunc bbdmFunc_getData;
    BBDMRandFunc bbdmFunc_indirectRand;
    SerializeFunction bbdmFunc_indirectSerialize;
    SerializeFunction payloadFunc_serialize;

    BaseBool inputIsBBDM;
    BaseBool outputIsBBDM;
    void *tmpObject;

    BaseBool (*onDeserialize)( struct SerializeUtility *self );

    BaseBool useRandomization;

    BaseUI32 delay;
    BaseBool interactive;

    BaseF64 valueMin;
    BaseF64 valueMax;
    BaseUI32 randomSeedState;
}
        SerializeUtility;


/*!
 * \brief resets the SerializeUtility object and frees memory of members
 *
 * \param self pointer to a valid SerializeUtility instance
 */
void SerializeUtility_clear( SerializeUtility *self );


/*!
 * \brief main entry function to convert serialized data
 *
 * Use this as master entry function once you've set up all parameters
 * with the corresponding set-functions. This function will, depending
 * on the member states, read serialized data from file, convert it
 * depending on the member states, and write to the desired output
 * channel (file or console).
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_convert( SerializeUtility *self );


/*!
 * \brief main entry function to generate serialized data
 *
 * Use this as master entry function once you've set up all parameters
 * with the corresponding set-functions. This function will, depending
 * on the member states, generate data and write to the desired output
 * channel (file or console).
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_create( SerializeUtility *self );


/*!
 * \brief destroys the object and frees the memory used by the object
 *
 * \param self pointer to a SerializeUtility instance
 *
 * \attention Please do not forget to call \c SerializeUtility_clear() before.
 */
void SerializeUtility_delete( SerializeUtility *self );


/*!
 * \brief reads the data from the given file (if the file exists and the
 *        file content is correct) and stores the result into the
 *        self->tmpObject variable
 *
 * \param self pointer to a SerializeUtility instance
 *
 * \return true on success, false otherwise
 */
BaseBool SerializeUtility_deserializeFromFile( SerializeUtility *self );


/*!
 * \brief initializes member attributes of a SerializeUtility object
 *
 * \param self pointer to an allocated, but not initialized SerializeUtility instance
 */
void SerializeUtility_init( SerializeUtility *self );


/*!
 * \brief creates a new SerializeUtility instance
 *
 * \returns pointer to a new SerializeUtility instance
 */
SerializeUtility *SerializeUtility_new( void );


/*!
 * \brief main entry function to print serialized data onto console
 *
 * Use this as master entry function once you've set up all parameters
 * with the corresponding set-functions. This function will, depending
 * on the member states, read serialized data from file, convert it
 * to ASCII format and print them to the console. It has pretty much
 * in common with SerializeUtility_convert().
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_print( SerializeUtility *self );


/*!
 * \brief toggle blocking/non-blocking network sends
 *
 * \param self pointer to a SerializeUtility instance
 * \param blocking true=blocking, false=non-blocking
 *
 * \see SerializeUtility_waitForUI()
 */
void SerializeUtility_setBlockingMode( SerializeUtility *self,
                                       BaseBool blocking );


/*!
 * \brief set descriptive name for serialization data
 *
 * The name will appear in the "dataname" filed of the serialization
 * header. When deserializing the data, the same name will be
 * expected.
 *
 * \param self pointer to a SerializeUtility instance
 * \param name dataname for serialization function
 */
void SerializeUtility_setDataName( SerializeUtility *self,
                                   const char *name );


/*!
 * \brief sets optional delay between processing elements from file
 *
 * In certain cases you may want to wait some time between deserializing
 * the elements from file. For example, to be able to read printed values
 * on the screen you could set some delay.
 * If untouched (zero delay), the file will be deserialized as fast as
 * possible and the data will be printed (or whatever) with maximum
 * speed.
 *
 * \param self pointer to a SerializeUtility instance
 * \param milliSeconds time to wait (in ms) between deserializing elements
 */
void SerializeUtility_setDelay( SerializeUtility *self,
                                BaseUI32 milliSeconds );


/*!
 * \brief optional initString for creating data
 *
 * For structured datatypes (e.g. BlockF32 or BBDMArray2DPoint) you can
 * override the default sizes etc. when creating the data. Internally,
 * even if you specify a Non-BBDM datatype (such as BlockF32), the
 * SerializeUtility always operates on BBDMs (in this case BBDMBlockF32).
 * Those BBDMs can be initialized with a particular initString.
 * Later, we transparently operate on the data-pointer inside the BBDM
 * in case you did not request a BBDM.
 *
 * As a summary, you can create BBDMs or Non-BBDMs, both with the
 * references of the BBDM.
 *
 * \param self pointer to a SerializeUtility instance
 * \param initString initialization string to pass to the BBDM
 *
 * \see documentation of the BBDM for your datatype, if existing it is
 *      usually located in ${SIT}/Modules/BBDM
 */
void SerializeUtility_setInitString( SerializeUtility *self,
                                     const char *initString );


/*!
 * \brief toggle interactive/non-interactive network sends
 *
 * If true, between each e.g. deserialization the user gets prompted
 * (message and wait for keypress) before continuing. When pressing
 * 'q' the loop gets exited.
 *
 * \param self pointer to a SerializeUtility instance
 * \param interactive true=interactive, false=continuous
 */
void SerializeUtility_setInteractiveMode( SerializeUtility *self,
                                          BaseBool interactive );


/*!
 * \brief sets internal name for data format of the source data
 *
 * This function modifies the self->inputDataFormat field and
 * might be used to override the auto-detected input file
 * serialization format. In general it is not recommended to
 * change this value.
 *
 * \param self pointer to a SerializeUtility instance
 * \param format one of the serialization formats supported by
 *               the Serialize 3.0 library (e.g. "Ascii")
 */
void SerializeUtility_setInputDataFormat( SerializeUtility *self,
                                          const char *format );


/*!
 * \brief sets internal name for data type of the source data
 *
 * This function modifies the self->inputDataType field and
 * might be used to override the auto-detected input data
 * type. In general it is not recommended to change this value.
 *
 * \param self pointer to a SerializeUtility instance
 * \param datatype datatype name (e.g. "BaseF32" or "BBDMBlockF32")
 * \return true if the given dataType was correct
 */
BaseBool SerializeUtility_setInputDataType( SerializeUtility *self,
                                            const char *datatype );

/*!
 * \brief sets the path to the input file
 *
 * You need to call this function whenever attempting to read
 * serialized data from a file. The function checks with
 * ANY_REQUIRE if the filename points to a regular file and
 * permissions are granted.
 *
 * \param self pointer to a SerializeUtility instance
 * \param filename path from where to read the input data
 */
void SerializeUtility_setInputFile( SerializeUtility *self,
                                    const char *filename );


/*!
 * \brief optional number or upper limit of elements to process
 *
 * If a value is set, at maximum the specified number of elements
 * will be processed. For example, only max. this number of
 * elements will be read from input file (even if it contains
 * more), or precisely that amount of elements will be generated
 * when creating serialized data.
 *
 * \note When deserializing from a file that contains less elements
 *       than the number you specified, the number of elements in
 *       the file precedes. So consider this value as upper limit.
 *
 * \param self pointer to a SerializeUtility instance
 * \param count number of elements to generate, or upper limit of
 *              elements to deserialize from file (if file contains
 *              more)
 */
void SerializeUtility_setMaxElements( SerializeUtility *self,
                                      unsigned int count );


/*!
 * \brief sets internal name for data format of the output data
 *
 * This function modifies the self->outputDataFormat field and
 * can be used to set the format of the serialized output data
 * (if appropriate). This is particularly useful for converting
 * serialized data from one format to another.
 *
 * \param self pointer to a SerializeUtility instance
 * \param format one of the serialization formats supported by
 *               the Serialize 3.0 library (e.g. "Xml")
 */
void SerializeUtility_setOutputDataFormat( SerializeUtility *self,
                                           const char *format );


/*!
 * \brief sets internal name for data type of the result data
 *
 * This function modifies the self->outputDataType field and
 * can be used to force the datatype-name field of the serialized
 * output data. This is particularly useful e.g. when extracting
 * the payload data out of a BBDM.
 *
 * \param self pointer to a SerializeUtility instance
 * \param datatype datatype name (e.g. "BaseF32")
 */
void SerializeUtility_setOutputDataType( SerializeUtility *self,
                                         const char *datatype );


/*!
 * \brief sets the path to the output file
 *
 * If not called, functions that produce output data will print
 * onto the console. You may want to call this function to redirect
 * output data into a file. The file will be created with default
 * umask settings of the user.
 *
 * \attention Existing files will be overwritten without further
 *            notification!
 *
 * \param self pointer to a SerializeUtility instance
 * \param filename path where to write the output data
 */
void SerializeUtility_setOutputFile( SerializeUtility *self,
                                     const char *filename );


/*!
 * \brief toggle whether or not to create randomized data
 *
 * If not set, newly created serialized data will have default
 * values (mostly zeroes, empty or so). If useRandomization is true,
 * the randomizer for the appropriate type will be called before
 * writing the data. The default is to not perform randomization.
 *
 * This works by calling the BBDM..._indirectRandomize() function.
 * It is implemented for most of the datatypes, but sometimes not
 * for all fields (if would not make sense). However it's a fairly
 * simple way to get some trivial data for testing ;-)
 *
 * \param self pointer to a SerializeUtility instance
 * \param useRandomization true=randomize, false=normal (default)
 */
void SerializeUtility_setRandomization( SerializeUtility *self,
                                        BaseBool useRandomization );

/*!
 * \brief detects the data type contained in the file set with
 *        setInputFile()
 *
 * \param self pointer to a SerializeUtility instance
 * \return 0 ok
 *         -1 file does not exist
 *         -2 wrong content
 */
BaseI32 SerializeUtility_detectDatatypeInFile( SerializeUtility *self );

/*!
 * \brief writes the data (currently contained in self->tmpObject) to
 *        the output. Call setupSerializer() to initialize the output.
 *
 * \param self pointer to a SerializeUtility instance
 * \return true on success
 */
BaseBool SerializeUtility_serializeElementToOutput( SerializeUtility *self );

/*!
 * \brief sets up the output serializer
 *        the output. Call setupSerializer() to initialize the output.
 *
 * \param self pointer to a SerializeUtility instance
 * \param outputUrl the url passed to the inner IOChannel object
 * \return true on success
 */
BaseBool SerializeUtility_setupSerializer( SerializeUtility *self,
                                           const char *outputUrl );

/*!
 * \brief closes the output serializer. Requieres a successfull call
 *        to setupSerializer()
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_closeSerializer( SerializeUtility *self );

/*!
 * \brief sets up the input serializer. Requieres a successfull call
 *        to setInputFile()
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_setupDeserializer( SerializeUtility *self );

/*!
 * \brief closes the input serializer. Requieres a successfull call
 *        to setupDeserializer()
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_closeDeserializer( SerializeUtility *self );

/*!
 * \brief detectes the functions to the data / BBDM. This method
 *        requires a successfull call to setInputDataType() which also
 *        will be called when serializing data from file
 *        The DynamicLoader is used to find the
 *        new(),
 *        initFromString(),
 *        clear(),
 *        delete(),
 *        getData(),
 *        indirectRand(),
 *        indirectSerialize()
 *        functions from the data / BBDM.
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_detectFunctions( SerializeUtility *self );

/*!
 * \brief wraps the call to Type_new() to create a new
 *        internal data / BBDM
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_constructObject( SerializeUtility *self );

/*!
 * \brief wraps the call to Type_initFromString() to create a new
 *        internal data / BBDM
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_initializeObject( SerializeUtility *self );

/*!
 * \brief destroys the internal data / BBDM
 *
 * \param self pointer to a SerializeUtility instance
 */
void SerializeUtility_destroyObject( SerializeUtility *self );

/*!
 * \brief gives access to the internal data (self->tmpObject). The access is
 *        independend of the correct initialisation of the internal data / BBDM
 *
 * \param self pointer to a SerializeUtility instance
 * \return pointer to the internal data
 */
void *SerializeUtility_getBBDM( SerializeUtility *self );

#if defined(__cplusplus)
}
#endif


#endif

/* EOF */
