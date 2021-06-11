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
 * \page Serialize_About Serialization
 *
 * The Serialize library (Serialize.h) allows transforming objects into
 * a persistent format for storage or transmitting over the network.
 *
 * The location where the data is serialized to (e.g. file, memory area,
 * TCP server,...) is decoupled and handled by the IOChannel library.
 *
 * \note Handling Serialize and IOChannel instances might be uncomfortable.
 *       Have a look to \ref QuickSerializers_About for convenience.
 *
 * The default format is JSON. But also Binary, Xml and some other formats
 * are available. From Wikipedia: "JSON [...] is a text-based open standard
 * designed for human-readable data interchange. It is derived from the
 * JavaScript scripting language for representing simple data structures
 * and associative arrays, called objects. Despite its relationship to
 * JavaScript, it is language-independent [...]"
 *
 * \li \subpage QuickSerializers_About
 * \li \subpage BaseSerialize_About
 * \li \subpage SerializeIntro
 * \li \subpage SerializeSpecialFunctionHowTo
 * \li \subpage SerializeCommonMistakes
 * \li \subpage SerializeTranslateModeInfo
 *
 * \see \ref ToolBOS_HowTo_SerializeToPython_viaJSON "HowTo: Deserialize JSON data in Python"
 * \see \ref ToolBOS_HowTo_SerializeToPython_ctypes "HowTo: Call deserialize functions from Python"
 *
 * <center>
 * \image html Serialization.png
 * </center>
 *
 * Unlike other implementations, the same serialization function is used for
 * both serialization and deserialization depending on the modality of the
 * passed Serialize object.
 *
 * In the example below, a structure of type Point is serialized trough its
 * specific serialization function Point_serialize().
 *
 * \code
 *
 *  Serialize *serializer = (Serialize*)NULL;
 *  IOChannel *stream = (IOChannel*)NULL;
 *  Point point = { POSX_VAL, POSY_VAL };
 *
 *  stream = IOChannel_new();
 *  IOChannel_init( stream );
 *  IOChannel_open( stream, "File://mySerialization.txt",
 *                  IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,
 *                  IOCHANNEL_PERMISSIONS_ALL );
 *
 *  serializer = Serialize_new();
 *  Serialize_init( serializer, NULL, SERIALIZE_STREAMMODE_NORMAL );
 *  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );
 *
 *  Serialize_setStream( serializer, stream );
 *  Serialize_setFormat( serializer, "Json", "" );
 *
 *  Point_serialize( &point, "point", serializer );
 *
 * \endcode
 *
 * results in a file \c mySerialization.txt with this content:
 * \verbatim
HRIS-1.0 BBDMBaseI64 myBBDMBaseI64          0 Json
,
{
  "Point": {
    "x": 123,
    "y": 456
  }
}
\endverbatim
 *
 * \note The first two lines (starting with HRIS and the comma) are optional
 *       headers added for polymorphism support of some ToolBOS utilities.
 *       Such header should be omitted when exchanging data with standard
 *       JSON-compatible software:
 *       \code
 *       Serialize_setMode( s, <usual_flags> | SERIALIZE_MODE_NOHEADER );
 *       \endcode
 */


#ifndef SERIALIZE_H
#define SERIALIZE_H


/*---------------------------------------------------------------------------*/
/* Include Files                                                             */
/*---------------------------------------------------------------------------*/

#if !defined(__windows__)

#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>

#endif

#include <setjmp.h>

#include <Any.h>
#include <MTList.h>
#include <IOChannel.h>
#include <DynamicLoader.h>
#include <SerializeReferenceValue.h>


/*---------------------------------------------------------------------------*/
/* Public defines                                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The first bytes of a package, usually called "label" or "preamble"
*/
#define SERIALIZE_HEADER_PREAMBLE                      "HRIS-"

/*!
  \brief Library Major version
*/
#define SERIALIZE_LIB_MAJVERSION                             3

/*!
  \brief Library Minor version
*/
#define SERIALIZE_LIB_MINVERSION                             0

/*!
  \brief Default header parser major version
*/
#define SERIALIZE_HEADER_MAJVERSIONDEFAULT                   2

/*!
  \brief Default header parser major version
*/
#define SERIALIZE_HEADER_MINVERSIONDEFAULT                   0

/*!
  \brief Define the maximum size of the string "PREAMBLE+VERSION"
*/
#define SERIALIZE_HEADER_PREAMBLEMAXLEN                      8

/*!
  \brief Define the maximum size of the header data type
*/
#define SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE 2048

/*!
  \brief  Define the maximum size of the header line in bytes.

  We expect to have maximum 5 elements, each one of the default size.
*/
#define SERIALIZE_HEADER_MAXLEN ( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE * 5 )

/*!
  \brief Define the default columnWrap
*/
#define SERIALIZE_COLUMNWRAP                                 3

/*!
  \brief Define the starting indentLevel for text formats
*/
#define SERIALIZE_INDENTLEVEL                                0


/*!
 * brief Define the default Serialize format
 */
#if !defined( SERIALIZE_DEFAULT_FORMAT )
#define SERIALIZE_DEFAULT_FORMAT "Json"
#endif

/*!
 * brief Define the default Serialize format options
 */
#if !defined( SERIALIZE_DEFAULT_FORMAT_OPTIONS )
#define SERIALIZE_DEFAULT_FORMAT_OPTIONS ""
#endif


/*!
  \page SerializeIntro Details

  This library provides an API for serialization of primitive and structured
  data in a simple way. Suppose you have the following structure:

  \code
  typedef struct Point
  {
    unsigned long int posX;
    unsigned long int posY;
  }
  Point;
  \endcode

  To serialize the Point structure above, a special function is needed:

  \code
  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    Serialize_beginType( s, name, "Point" );
      ULInt_serialize( &(self->posX), "posX", s );
      ULInt_serialize( &(self->posY), "posY", s );
    Serialize_endType( s );
  }
  \endcode

  Once you have defined this function you can use serialization to produce
  any kind of representation you want, setting the format you prefer.
  It is also possible for the user to create custom formats. Currently provided ones are:

    \a Binary, \a Ascii, \a Xml, \a Matlab, \a MxArray, \a Json

  The data can be serialized over an IOChannel instance: this means
  that you can make data persistent using for example a "File://"
  stream, or you can transmit remotely over Tcp using the "Tcp://" stream,
  or share data with other processes or threads using "Shm://" or
  custom streams.

  The Translate mode does not use streams as described in the paragraph \ref SerializeTranslateModeInfo.

  Here is an example on how the serialization function defined above (Point_serialize()) could be used:

  \code
  int main( int argc, char *argv[] )
  {
    Serialize *serializer = (Serialize*)NULL;
    IOChannel *streamer = (IOChannel*)NULL;
    Point point = { POSX_VAL, POSY_VAL };

    // Create and open the stream: we want to make data persistent
    // so we will use the "File://" stream...
    streamer = IOChannel_new();
    IOChannel_init( streamer );

    IOChannel_open( stream, "File://mySerialization.txt",
                    IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT, IOCHANNEL_MODE_TRUNC,
                    IOCHANNEL_PERMISSIOND_ALL );

    // Create the serializer and set it to the write mode...
    serializer = Serialize_new();
    Serialize_init( serializer, NULL, SERIALIZE_STREAMMODE_NORMAL );

    // Many modes and flags can be set...
    Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );

    // Set the stream, Serialize doesn't care about the kind of stream used.
    Serialize_setStream( serializer, stream );

    // Set the format: here we use Xml...
    Serialize_setFormat( serializer, "Xml", "" );

    // Start Serializing: the specific serialize function will write data to a file.
    Point_serialize( &point, "point", serializer );

    IOChannel_close( streamer );
    IOChannel_clear( streamer );
    IOChannel_delete( streamer );

    Serialize_clear( serializer );
    Serialize_delete( serializer );

    return 0;
  }
  \endcode

  The output of this program should be a file "mySerialization.txt", whose content is
  similar to the following:

  \code
  HRIS-1.0 Point point        0 Xml
  <struct type="Point" name="point">
      <field type="unsigned long int" name="posX">63</field>
      <field type="unsigned long int" name="posY">88</field>
  </struct>
  \endcode

  You can leave this code untouched and change the string "Xml" in the Serialize_setFormat()
  to have the same structure in the Ascii, Binary or Matlab formats.
 */


/*!
  \page SerializeSpecialFunctionHowTo Serialization functions

  In this section some particular implementations of the Serialize function are analyzed
  together with some common errors related to them.

  Sometimes your Serialize special functions must implement some logical
  checks for correct working. Suppose you have the following structures:

  \code
  typedef struct Point
  {
    int  posX;
    int  posY;
  }
  Point;

  typedef struct MatrixOfPoints
  {
    int    width;
    int    height;
    Point *matrix;
  }
  MatrixOfPoints;

  \endcode

  You are going to write a serialize function both for the Point and
  for MatrixOfPoints. Suppose you are working with Tcp:// stream, and you are sending
  a MatrixOfPoints 5x3 from an host to a remote one.
  The host which reads the data that is sent, will surely set the values of its Points instances
  deserializing the data, but how can it establish if the received matrix is a 5x3 if, when
  reading, their values are overwritten? The solution is shown in the following code.

  \code
  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char*)"Point" );
      Int_serialize( &(self->posX), (char*)"posX", s );
      Int_serialize( &(self->posY), (char*)"posY", s );
    Serialize_endType( s );
  }

  void MatrixOfPoints_serialize( MatrixOfPoints *self, char *name, Serialize *s )
  {
    int width = 0;
    int height = 0;
    int length = ((self->width)*(self->height));

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char*)"Matrix" );
    {
      if( Serialize_isReading( s ) == true )
      {
        Int_serialize( &(width), (char*)"width", s );
        ANY_REQUIRE_MSG( self->width == width,
                         "Reading a Matrix with a different width" );

        Int_serialize( &(height), (char*)"height", s );
        ANY_REQUIRE_MSG( self->height == height ,
                         "Reading a Matrix with a different height" );
      }

      if( Serialize_isWriting( s ) == true )
      {
        Int_serialize( &(self->width), (char*)"width", s );
        Int_serialize( &(self->height), (char*)"height", s );
      }

      STRUCT_ARRAY_SERIALIZE( self->matrix, (char*)"matrix", (char*)"Point",
                              Point_serialize, length, s );
    }
    Serialize_endType( s );
  }
  \endcode

  As you can see the 'width' and the 'height' are properties of this
  matrix, so when writing you will send their values through and when reading you will
  use an additional variable to check that the expected values are correct.
 */


/*!
  \page SerializeCommonMistakes Common mistakes

  A common mistake done while implementing a serialize special
  function is that of passing arguments by value and not by reference.
  Look at this example:

  \code
  void Base2DPoint_serialize( Base2DPoint *self, char *name, Serialize *serializer )
  {
    int i1 = 0;
    int i2 = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, (char*)"Base2DPoint" );

      i1 = Base2DPoint_getX( self );
      BaseI32_serialize( &i1, (char*)"x", serializer );

      i2 = Base2DPoint_getY( self );
      BaseI32_serialize( &i2, (char*)"y", serializer );

    Serialize_endType( serializer );
  }
  \endcode

  You'll probably notice that this code works in the right way in
  writing mode, but when reading, the values of the "self" do
  not change. Be careful using copies and local variables.
  If you really want use them, a solution may be the following:

  \code
  void Base2DPoint_serialize( Base2DPoint *self, char *name, Serialize *serializer )
  {
    int i1 = 0;
    int i2 = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( serializer );

    Serialize_beginType( serializer, name, (char*)"Base2DPoint" );

      if( Serialize_isWriting( serializer ) == true )
      {
        i1 = BaseI32_getX( self );
        BaseI32_serialize( &i1, (char*)"x", serializer );

        i2 = BaseI32_getY( self );
        BaseI32_serialize( &i2, (char*)"y", serializer );
      }

      if( Serialize_isReading( serializer ) == true )
      {
        BaseI32_serialize( &i1, (char*)"x", serializer );
        BaseI32_serialize( &i2, (char*)"y", serializer );

        // Set Read Values!
        Base2DPoint_set( self, i1, i2 );
      }

    Serialize_endType( serializer );
  }
  \endcode
 */


/*!
  \page SerializeTranslateModeInfo Translation mode

  The Translate mode is a special feature that was added to the
  Serialize lib to translate data for which you provides a
  special Serialize function, to some particular structured format.
  Actually the only format which allows translate mode is MxArray.

  - MxArray

  The MxArray format allow to CREATE a Matlab mxArray representation
  of any data for which you provides a Serialize special function.
  This is very useful because the mxArray 's objects can be
  transmitted from/to Matlab Engines thanks to the Matlab API's.
  In this way you can create an mxArray representation of your data, send
  it to Matlab in the format of mxArray for any computational operations
  you want, and then retrive the modified mxArray: at this point
  you can UPDATE your original instance ( not in the format of mxArray )
  from the received mxArray.
  The Format in fact allow these three operations:

  CREATE an mxArray. <br>
  UPDATE The values of an mxArray from a standard instance ( not in the mxArray format ). <br>
  UPDATE The values of a standard instance ( not in the mxArray format ) from an mxArray. <br>
*/


#if defined(__cplusplus)
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Types                                                                     */
/*---------------------------------------------------------------------------*/
/*!
  \brief Forward Declaration
*/
struct Serialize;

/*!
  \brief Forward Declaration
*/
struct SerializeHeader;

/*!
  \brief Map The Enumerator on the specified bit
*/
#define SERIALIZE_MODE_MAP( __enumType, __bit ) __enumType =  1 << __bit

/*!
  \brief Tells is a specific bit is set
*/
#define SERIALIZE_MODE_IS( __modes, __toCheck ) ((__modes|__toCheck) == __modes)

typedef enum SerializeDeployDataMode
{
    SERIALIZE_MODE_MAP( SERIALIZE_DEPLOYDATAMODE_BINARY, 0 ), /**< Deploy Mode is Binary  */
    SERIALIZE_MODE_MAP( SERIALIZE_DEPLOYDATAMODE_ASCII, 1 )  /**< Deploy Mode is Ascii */
}
        SerializeDeployDataMode;

typedef enum SerializeStreamMode
{
    SERIALIZE_MODE_MAP( SERIALIZE_STREAMMODE_NORMAL, 2 ),  /**< Streaming depends only from the used IOChannel  */
    SERIALIZE_MODE_MAP( SERIALIZE_STREAMMODE_FLUSH, 3 ),  /**< Streaming is forced to flush for each object   */
    SERIALIZE_MODE_MAP( SERIALIZE_STREAMMODE_LOOP, 4 )   /**< After object serialization, rewind the stream  */
}
        SerializeStreamMode;

typedef enum SerializeMode
{
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_WRITE, 5 ),  /**< Write Data Into The Stream     */
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_READ, 6 ),  /**< Read Data From The Stream      */
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_CALC, 7 ),  /**< Calculates Size                */
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_NULL, 8 )   /**< Internal use: no direction set */
}
        SerializeMode;

typedef enum SerializeFlags
{
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_AUTOCALC, 9 ), /**< Auto set header size param */
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_NOHEADER, 10 ), /**< Do not use header          */
    SERIALIZE_MODE_MAP( SERIALIZE_MODE_TRANSLATE, 11 )  /**< Translate mode             */
}
        SerializeFlags;
/*! @} */

/*!
  \brief Supported datatypes
 */
typedef enum SerializeType
{
    SERIALIZE_TYPE_CHAR = 0, /**< char                        */
            SERIALIZE_TYPE_SCHAR, /**< signed char                 */
            SERIALIZE_TYPE_UCHAR, /**< unsigned char               */
            SERIALIZE_TYPE_SINT, /**< short int                   */
            SERIALIZE_TYPE_USINT, /**< unsigned short int          */
            SERIALIZE_TYPE_INT, /**< int                         */
            SERIALIZE_TYPE_UINT, /**< unsigned int                */
            SERIALIZE_TYPE_LINT, /**< long int                    */
            SERIALIZE_TYPE_ULINT, /**< unsigned long int           */
            SERIALIZE_TYPE_LL, /**< long long                   */
            SERIALIZE_TYPE_ULL, /**< unsigned long long          */
            SERIALIZE_TYPE_FLOAT, /**< float                       */
            SERIALIZE_TYPE_DOUBLE, /**< double                      */
            SERIALIZE_TYPE_LDOUBLE, /**< long double                 */
            SERIALIZE_TYPE_CHARARRAY, /**< array of char               */
            SERIALIZE_TYPE_SCHARARRAY, /**< array of signed char        */
            SERIALIZE_TYPE_UCHARARRAY, /**< array of unsigned char      */
            SERIALIZE_TYPE_SINTARRAY, /**< array of short int          */
            SERIALIZE_TYPE_USINTARRAY, /**< array of unsigned short int */
            SERIALIZE_TYPE_INTARRAY, /**< array of int                */
            SERIALIZE_TYPE_UINTARRAY, /**< array of unsigned int       */
            SERIALIZE_TYPE_LINTARRAY, /**< array of long int           */
            SERIALIZE_TYPE_ULINTARRAY, /**< array of unsigned long long */
            SERIALIZE_TYPE_LLARRAY, /**< array of long int           */
            SERIALIZE_TYPE_ULLARRAY, /**< array of unsigned long long */
            SERIALIZE_TYPE_FLOATARRAY, /**< array of float              */
            SERIALIZE_TYPE_DOUBLEARRAY, /**< array of double             */
            SERIALIZE_TYPE_LDOUBLEARRAY, /**< array of double             */
            SERIALIZE_TYPE_STRING             /**< quoted string               */
}
        SerializeType;

/*!
  \brief Max ascii size of the type char
*/
#define SERIALIZE_TYPEMAXTEXTLEN_CHAR        6

/*!
  \brief Max ascii size of the type unsigned char
*/
#define SERIALIZE_TYPEMAXTEXTLEN_UCHAR       6

/*!
  \brief Max ascii size of the type short int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_SINT        6

/*!
  \brief Max ascii size of the type unsigned short int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_USINT       5

/*!
  \brief Max ascii size of the type int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_INT        11

/*!
  \brief Max ascii size of the type unsigned int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_UINT       10

/*!
  \brief Max ascii size of the type long int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_LINT       11

/*!
  \brief Max ascii size of the type unsigned long int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_ULINT      10

/*!
  \brief Max ascii size of the type long long int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_LL         20

/*!
  \brief Max ascii size of the type unsigned long long int
*/
#define SERIALIZE_TYPEMAXTEXTLEN_ULL        20

/*!
  \brief Max ascii size of the type float
*/
#define SERIALIZE_TYPEMAXTEXTLEN_FLOAT      14

/*!
  \brief Max ascii size of the type double
*/
#define SERIALIZE_TYPEMAXTEXTLEN_DOUBLE     23

/*!
  \brief Max ascii size of the type long double
*/
#define SERIALIZE_TYPEMAXTEXTLEN_LDOUBLE    25

/*!
  \brief Max ascii size of the type long double
*/
#define SERIALIZE_TYPEMAXTEXTLEN_STRING   1024

/*!
  \brief Log Level Verbosity
*/
#define SERIALIZE_LOGLEVEL_TRACESETTINGS        5
#define SERIALIZE_LOGLEVEL_TRACEWARNINGS        7
#define SERIALIZE_LOGLEVEL_TRACEFUNCTION       10
#define SERIALIZE_LOGLEVEL_TRACEAPICALLS       12


/*!
  \brief Logs the Serialize Functions calls

  Internal Usage
*/
#ifdef SERIALIZE_DEBUG_MODE

#define SERIALIZE_TRACE_FUNCTION( __constString )\
  ANY_LOG( SERIALIZE_LOGLEVEL_TRACEFUNCTION,\
           "SERIALIZE_TRACE_FUNCTION-Entering in "__constString"()",\
            ANY_LOG_INFO )

/*!
  \brief Logs the Serialize Format Api function calls

  Plugins Developing Only
*/
#define SERIALIZE_TRACE_APICALLS( __constString )\
  ANY_LOG( SERIALIZE_LOGLEVEL_TRACEAPICALLS,\
           "SERIALIZE_TRACE_APICALLS-Entering in "__constString"()",\
           ANY_LOG_INFO )

#else

#define SERIALIZE_TRACE_FUNCTION( __unused )
#define SERIALIZE_TRACE_APICALLS( __unused )

#endif


/*!
  \brief Defines a SerializeFormatBeginType function
 */
typedef void (*SerializeFormatBeginType)( struct Serialize *,
                                          const char *, const char * );

/*!
  \brief Defines a SerializeFormatBeginBaseType function
*/
typedef void (*SerializeFormatBeginBaseType)( struct Serialize *,
                                              const char *, const char * );

/*!
  \brief Defines a SerializeFormatBeginArray function
*/
typedef void (*SerializeFormatBeginArray)( struct Serialize *, SerializeType,
                                           const char *, const int );

/*!
  \brief Defines a SerializeFormatBeginStructArray function
*/
typedef void (*SerializeFormatBeginStructArray)( struct Serialize *,
                                                 const char *,
                                                 const char *, const int );

/*!
  \brief Defines a SerializeFormatDoSerialize function
*/
typedef void (*SerializeFormatDoSerialize)( struct Serialize *, SerializeType,
                                            const char *, void *, const int, const int );

/*!
  \brief Defines a SerializeFormatStructArraySeparator function
*/
typedef void (*SerializeFormatStructArraySeparator)( struct Serialize *,
                                                     const char *,
                                                     const int, const int );

/*!
  \brief Defines a SerializeFormatEndStructArray function
*/
typedef void (*SerializeFormatEndStructArray)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatEndArray function
*/
typedef void (*SerializeFormatEndArray)( struct Serialize *, SerializeType,
                                         const char *, const int );

/*!
  \brief Defines a SerializeFormatEndBaseType function
*/
typedef void (*SerializeFormatEndBaseType)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatEndType function
*/
typedef void (*SerializeFormatEndType)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatGetAllowedModes function
*/
typedef int (*SerializeFormatGetAllowedModes)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatOptionsNew function
*/
typedef void *(*SerializeFormatOptionsNew)( void );

/*!
  \brief Defines a SerializeFormatOptionsInit function
*/
typedef void (*SerializeFormatOptionsInit)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatOptionsSet function
*/
typedef void (*SerializeFormatOptionsSet)( struct Serialize *, const char * );

/*!
  \brief Defines a SerializeFormatOptionsSetProperty function
*/
typedef bool (*SerializeFormatOptionsSetProperty)( struct Serialize *,
                                                   const char *, void * );

/*!
  \brief Defines a SerializeFormatOptionsGetProperty function
*/
typedef void *(*SerializeFormatOptionsGetProperty)( struct Serialize *,
                                                    const char * );

/*!
  \brief Defines a SerializeFormatOptionsClear function
*/
typedef void (*SerializeFormatOptionsClear)( struct Serialize * );

/*!
  \brief Defines a SerializeFormatOptionsDelete function
*/
typedef void (*SerializeFormatOptionsDelete)( struct Serialize * );


/*!
  \brief Call the respectively format declared beginType
*/
#define SERIALIZEFORMAT_BEGINTYPE( __self, __name, __type )\
  do\
  {\
    /* Increase Number Of calls */\
    __self->numTypeCalls++;\
  \
    if( (__self->numTypeCalls > 1) && (__self->baseTypeEnable == true) )\
    {\
      (*__self->format->ops->indirectBeginBaseType)( __self, __name, __type );\
    }\
    else\
    {\
      (*__self->format->ops->indirectBeginType)( __self, __name, __type );\
    }\
  }\
  while ( 0 )

/*!
  \brief Call the respectively format declared beginArray
*/
#define SERIALIZEFORMAT_BEGINARRAY( __self, __type, __name, __len )\
  (*__self->format->ops->indirectBeginArray)( __self, __type, __name, __len )

/*!
  \brief Call the respectively format declared beginStructArray
*/
#define SERIALIZEFORMAT_BEGINSTRUCTARRAY( __self, __arrayName, __elementType, __arrayLen )\
  (*__self->format->ops->indirectBeginStructArray)( __self, __arrayName, __elementType, __arrayLen )

/*!
  \brief Call the respectively format options declared beginStructArraySeparator
*/
#define SERIALIZEFORMAT_BEGINSTRUCTARRAYSEPARATOR( __self, __arrayName, __arrayPosition, __arrayLen )\
  (*__self->format->ops->indirectBeginStructArraySeparator)( __self, __arrayName, __arrayPosition, __arrayLen )

/*!
  \brief Call the respectively format declared doSerialize
*/
#define SERIALIZEFORMAT_DOSERIALIZE( __self, __type, __name, __value, __size, __len )\
  (*__self->format->ops->indirectDoSerialize)( __self, __type, __name, __value, __size, __len )

/*!
  \brief Call the respectively format options declared endStructArraySeparator
*/
#define SERIALIZEFORMAT_ENDSTRUCTARRAYSEPARATOR( __self, __arrayName, __arrayPosition, __arrayLen )\
  (*__self->format->ops->indirectEndStructArraySeparator)( __self, __arrayName, __arrayPosition, __arrayLen )

/*!
  \brief Call the respectively format declared endStructArray
*/
#define SERIALIZEFORMAT_ENDSTRUCTARRAY( __self )\
  (*__self->format->ops->indirectEndStructArray)( __self )

/*!
  \brief Call the respectively format declared endArray
*/
#define SERIALIZEFORMAT_ENDARRAY( __self, __type, __name, __len )\
  (*__self->format->ops->indirectEndArray)( __self, __type, __name, __len )

/*!
  \brief Call the respectively format declared endType
*/
#define SERIALIZEFORMAT_ENDTYPE( __self )\
do\
{\
  ANY_REQUIRE( __self->numTypeCalls >= 0 );\
\
  if( (__self->numTypeCalls > 1) && (__self->baseTypeEnable == true) )\
  {\
    (*__self->format->ops->indirectEndBaseType)( __self );\
  }\
  else\
  {\
    (*__self->format->ops->indirectEndType)( __self );\
  }\
\
  /* Decrease Number Of calls */\
  __self->numTypeCalls--;\
  ANY_REQUIRE( __self->numTypeCalls >= 0 );\
}\
while ( 0 )

/*!
  \brief Call the respectively format declared getAllowedModes
*/
#define SERIALIZEFORMAT_GETALLOWEDMODES( __self )\
  (*__self->format->ops->indirectGetAllowedModes)( __self )

/*!
  \brief Call the respectively format options declared new
*/
#define SERIALIZEFORMATOPTIONS_NEW( __self )\
  (*__self->format->ops->indirectFormatOptionsNew)()

/*!
  \brief Call the respectively format options declared init
*/
#define SERIALIZEFORMATOPTIONS_INIT( __self )\
  (*__self->format->ops->indirectFormatOptionsInit)( __self )

/*!
  \brief Call the respectively format options declared set
*/
#define SERIALIZEFORMATOPTIONS_SET( __self, __optionsString )\
  (*__self->format->ops->indirectFormatOptionsSet)( __self, __optionsString )

/*!
  \brief Call the respectively format options set Parameter
*/
#define SERIALIZEFORMATOPTIONS_SETPROPERTY( __self, __optName, __optValue )\
  (*__self->format->ops->indirectFormatOptionsSetProperty)( __self, __optName, __optValue )\

/*!
  \brief Call the respectively format options get Parameter
*/
#define SERIALIZEFORMATOPTIONS_GETPROPERTY( __self, __optName )\
  (*__self->format->ops->indirectFormatOptionsGetProperty)( __self, __optName )\

/*!
  \brief Call the respectively format options declared clear
*/
#define SERIALIZEFORMATOPTIONS_CLEAR( __self )\
  (*__self->format->ops->indirectFormatOptionsClear)( __self )

/*!
  \brief Call the respectively format options declared delete
*/
#define SERIALIZEFORMATOPTIONS_DELETE( __self )\
  (*__self->format->ops->indirectFormatOptionsDelete)( __self )


/* Macros For Plugins */


/*!
  \brief Used by formats to print indent spaces
*/
#define SERIALIZE_INDENT( __self ) Serialize_indent( __self )

/*!
  \brief Defines Default Indentation Size For Text Based Formats
*/
#define SERIALIZE_INDENT_SIZE     ( 2 )

/*!
  \brief Used To Develop Formats, increase current indentation
*/
#define SERIALIZE_INDENT_INCR( __self ) ((__self->indentLevel)+=SERIALIZE_INDENT_SIZE)

/*!
  \brief Used To Develop Formats, decrease current indentation
*/
#define SERIALIZE_INDENT_DECR( __self ) ((__self->indentLevel)-=SERIALIZE_INDENT_SIZE)

/*!
  \brief Used by the Format to get/set the properties

  Start get/set property section
*/
#define SERIALIZEPROPERTY_START( __optName )\
  do\
  {\
    const char *__prvOptName = __optName;

/*!
  \brief Used by the Format to start the parsing of the property to get/set

  Begin the parsing of a property to get/set
*/
#define SERIALIZEPROPERTY_PARSE_BEGIN( __optName )\
    if( Any_strcasecmp( __prvOptName, #__optName ) == 0)\
    {

/*!
  \brief Used by the Format to end the parsing of the property to get/set

  End the parsing of a property to get/set
*/
#define SERIALIZEPROPERTY_PARSE_END( __optName )\
      break;\
    }

/*!
  \brief Used by the Format to get/set the properties

  End get/set property section
*/
#define SERIALIZEPROPERTY_END \
  }\
  while ( 0 );

/*!
  \brief Serialize Format format options

  \param __formatName The name of the Plugin options
*/
#define SERIALIZEFORMAT_OPTIONS( __formatName )\
  SerializeFormat##__formatName##Ops

/*!
  \brief Serialize Format declare format options

  \param __formatName The name of the Plugin of which you want to declare the options
*/
#define SERIALIZEFORMAT_DECLARE_OPTIONS( __formatName )\
  SerializeFormat SERIALIZEFORMAT_OPTIONS( __formatName )

/*!
  \brief Serialize Format Plugin Creation

  \param __formatName The name of the Plugin you want to Create
*/
#define SERIALIZEFORMAT_CREATE_PLUGIN( __formatName )\
  SERIALIZEFORMAT_DECLARE( __formatName );\
  SERIALIZEFORMAT_DECLARE_OPTIONS( __formatName ) =\
  SERIALIZEFORMAT_CREATE( __formatName )

/*!
  \brief Declares the prototypes for the format

  \param __formatName The name of the format for which prototypes are created
*/
#define SERIALIZEFORMAT_DECLARE( __formatName )\
\
  static void SerializeFormat##__formatName##_beginType( Serialize *self, const char* name , const char* type );\
  static void SerializeFormat##__formatName##_beginBaseType( Serialize *self, const char* name , const char* type );\
  static void SerializeFormat##__formatName##_beginArray( Serialize  *self,\
                                                          SerializeType   type,\
                                                          const char       *arrayName,\
                                                          const int         arrayLen     );\
  static void SerializeFormat##__formatName##_beginStructArray( Serialize  *self,\
                                                                const char       *arrayName,\
                                                                const char       *elementType,\
                                                                const int         arrayLen     );\
  static void  SerializeFormat##__formatName##_beginStructArraySeparator( Serialize  *self,\
                                                                          const char *arrayName,\
                                                                          const int arrayPosition,\
                                                                          const int arrayLen );\
  static void SerializeFormat##__formatName##_doSerialize( Serialize *self,\
                                                          SerializeType   type,\
                                                          const char           *name,\
                                                          void           *value,\
                                                          const  int    size,\
                                                          const  int    len );\
  static void SerializeFormat##__formatName##_endStructArraySeparator( Serialize  *self,\
                                                                      const char *arrayName,\
                                                                      const int arrayPosition,\
                                                                      const int arrayLen );\
  static void  SerializeFormat##__formatName##_endStructArray( Serialize *self );\
  static void SerializeFormat##__formatName##_endArray( Serialize     *self,\
                                                        SerializeType  type,\
                                                        const char    *name,\
                                                        const int len );\
  static void SerializeFormat##__formatName##_endBaseType( Serialize *self );\
  static void SerializeFormat##__formatName##_endType( Serialize *self );\
  static int  SerializeFormat##__formatName##_getAllowedModes( Serialize *self );\
\
  static void* SerializeFormat##__formatName##Options_new( void );\
  static void  SerializeFormat##__formatName##Options_init( Serialize *self );\
  static void  SerializeFormat##__formatName##Options_set( Serialize *self, const char *optionsString );\
  static bool  SerializeFormat##__formatName##Options_setProperty( Serialize *self, const char *optName, void *optValue );\
  static void* SerializeFormat##__formatName##Options_getProperty( Serialize *self, const char *optName );\
  static void  SerializeFormat##__formatName##Options_clear( Serialize *self );\
  static void  SerializeFormat##__formatName##Options_delete( Serialize *self )

/*!
  \brief Fill The SerializeFormat structure with the respectives function operations

  \param __formatName The name of the specific format functions to be filled
*/
#define SERIALIZEFORMAT_CREATE( __formatName )\
  {\
    (char*)#__formatName,\
    SerializeFormat##__formatName##_beginType,\
    SerializeFormat##__formatName##_beginBaseType,\
    SerializeFormat##__formatName##_beginArray,\
    SerializeFormat##__formatName##_beginStructArray,\
    SerializeFormat##__formatName##_beginStructArraySeparator,\
    SerializeFormat##__formatName##_doSerialize,\
    SerializeFormat##__formatName##_endStructArraySeparator,\
    SerializeFormat##__formatName##_endStructArray,\
    SerializeFormat##__formatName##_endArray,\
    SerializeFormat##__formatName##_endBaseType,\
    SerializeFormat##__formatName##_endType,\
    SerializeFormat##__formatName##_getAllowedModes,\
    SerializeFormat##__formatName##Options_new,\
    SerializeFormat##__formatName##Options_init,\
    SerializeFormat##__formatName##Options_set,\
    SerializeFormat##__formatName##Options_setProperty,\
    SerializeFormat##__formatName##Options_getProperty,\
    SerializeFormat##__formatName##Options_clear,\
    SerializeFormat##__formatName##Options_delete\
  }

/*!
  \brief Build an one integer-made version from the two integers
*/
#define SERIALIZE_BUILDVERSION( __maj, __min ) (( __maj << 16 ) | __min)

/*!
  \brief Start detecting which header version we are using

  \param __header SerializeHeader instance
 */
#define SERIALIZE_START_HEADER_SWITCH( __header )  \
do                                                                  \
{                                                                   \
int __majVersion = __header->majVersion;                            \
int __minVersion = __header->minVersion;                            \
switch( SERIALIZE_BUILDVERSION( __majVersion, __minVersion ) )      \
{

/*!
  \brief Recognize given version and allow the user to handle the particular header version

  \param __externalMajVersion Major header version
  \param __externalMinVersion Minor header version
*/
#define SERIALIZE_START_HANDLE_VERSION( __externalMajVersion, __externalMinVersion ) \
case SERIALIZE_BUILDVERSION( __externalMajVersion, __externalMinVersion ): \
{

/*!
  \brief Stop recognition of version
*/
#define SERIALIZE_STOP_HANDLE_VERSION()        \
}                                               \
    break;

/*!
  \brief Stop detection of header version
*/
#define SERIALIZE_STOP_HEADER_SWITCH()                                 \
default:                                                                \
{                                                                       \
  ANY_LOG( 0, "Header version not supported: %d.%d ",                   \
           ANY_LOG_ERROR, __majVersion, __minVersion );                 \
  self->errorOccurred = true;                                           \
}                                                                       \
break;                                                                  \
}                                                                       \
} while( 0 );

/*!
 * \brief Check if the element type is an array element
 * \param __elemType Element type
 */
#define SERIALIZE_IS_ARRAY_ELEMENT( __elemType ) \
  (( __elemType == SERIALIZE_TYPE_CHARARRAY || \
     __elemType == SERIALIZE_TYPE_SCHARARRAY || \
     __elemType == SERIALIZE_TYPE_UCHARARRAY || \
     __elemType == SERIALIZE_TYPE_SINTARRAY || \
     __elemType == SERIALIZE_TYPE_USINTARRAY || \
     __elemType == SERIALIZE_TYPE_INTARRAY || \
     __elemType == SERIALIZE_TYPE_UINTARRAY || \
     __elemType == SERIALIZE_TYPE_LINTARRAY || \
     __elemType == SERIALIZE_TYPE_ULINTARRAY || \
     __elemType == SERIALIZE_TYPE_LLARRAY || \
     __elemType == SERIALIZE_TYPE_ULLARRAY || \
     __elemType == SERIALIZE_TYPE_FLOATARRAY || \
     __elemType == SERIALIZE_TYPE_DOUBLEARRAY || \
     __elemType == SERIALIZE_TYPE_LDOUBLEARRAY ) ? true : false )

/*!
  \brief Serialize Format Operations
*/
typedef struct SerializeFormat
{
    char *formatName;
    /**< Specific Format name */
    SerializeFormatBeginType indirectBeginType;
    /**< Specific format BeginType */
    SerializeFormatBeginBaseType indirectBeginBaseType;
    /**< Specific format BeginBaseType */
    SerializeFormatBeginArray indirectBeginArray;
    /**< Specific format BeginArray */
    SerializeFormatBeginStructArray indirectBeginStructArray;
    /**< Specific format BeginStructArray */
    SerializeFormatStructArraySeparator indirectBeginStructArraySeparator;
    /**< Specific format BeginStructArray Separator */
    SerializeFormatDoSerialize indirectDoSerialize;
    /**< Specific format DoSerialize */
    SerializeFormatStructArraySeparator indirectEndStructArraySeparator;
    /**< Specific format EndStructArray Separator */
    SerializeFormatEndStructArray indirectEndStructArray;
    /**< Specific format EndStructArray */
    SerializeFormatEndArray indirectEndArray;
    /**< Specific format EndArray */
    SerializeFormatEndBaseType indirectEndBaseType;
    /**< Specific format EndBaseType */
    SerializeFormatEndType indirectEndType;
    /**< Specific format EndType */
    SerializeFormatGetAllowedModes indirectGetAllowedModes;
    /**< Specific format GetAllowedModes */
    SerializeFormatOptionsNew indirectFormatOptionsNew;
    /**< Specific format option New */
    SerializeFormatOptionsInit indirectFormatOptionsInit;
    /**< Specific format option Init */
    SerializeFormatOptionsSet indirectFormatOptionsSet;
    /**< Specific format option Set/Reset */
    SerializeFormatOptionsSetProperty indirectFormatOptionsSetProperty;
    /**< Specific format option SetProperty */
    SerializeFormatOptionsGetProperty indirectFormatOptionsGetProperty;
    /**< Specific format option GetProperty */
    SerializeFormatOptionsClear indirectFormatOptionsClear;
    /**< Specific format option Clear */
    SerializeFormatOptionsDelete indirectFormatOptionsDelete;       /**< Specific format option Delete */
}
        SerializeFormat;

/*!
  \brief Serialize Plugin Info
*/
typedef struct SerializeFormatInfo
{
    SerializeFormat *ops;
    /**< The SerializeFormat functions  */
    void *data;
    /**< Pointer to the format options  */
    DynamicLoader *libHandle; /**< Handles plugin library pointer */
}
        SerializeFormatInfo;

/*!
  \brief Fields of HRI Serialization header, Version 2.0
*/
typedef struct SerializeHeader
{
    int majVersion;
    /**< header major version */
    int minVersion;
    /**< header minor version */
    long objSize;
    /**< Size of the object for the current format */

    SerializeReferenceValue *listHead;
    SerializeReferenceValue *listTail;
    SerializeReferenceValue *poolHead;
    SerializeReferenceValue *poolTail;

    int typeSize;
    int nameSize;
    int optsSize;
    int formatSize;
    long headerSize;              /**< Size of the header */
    bool dumpable;                /**< User choice: ignores unknown headers  */
}
        SerializeHeader;

/*!
  \brief Serialize definition
*/
typedef struct Serialize
{
    unsigned long valid;
    /**< Validation Tag                       */
    SerializeFormatInfo *format;
    /**< Current serialize format             */
    SerializeStreamMode streamMode;
    /**< Normal, Loop or Flush Mode           */
    SerializeHeader *header;
    /**< Package header, current version 2.0  */
    SerializeMode mode;
    /**< Read, Write or CalcSize              */
    MTList *formatList;          /**< Store the added formats              */
    bool isLittleEndian;
    /**< True if Host is little endian        */
    int numTypeCalls;
    /**< Counts beginType and endType calls   */
    IOChannel *calcSizeStream;
    /**< Reserved for calcsize                */
    IOChannel *stream;             /**< Data Stream                          */
    bool baseTypeEnable;      /**< Flag For Plain Data Fields           */
    bool forceBinaryDeploy;   /**< Force Deploy To be always Binary     */
    bool isInitMode;          /**< If true Serialize init data          */
    bool isAutoCalcSizeMode;  /**< If true, Bufferize for CalcSize      */
    bool isTranslateMode;     /**< Tells format to do translations      */
    bool useHeader;
    /**< If False Headers are not used        */
    int indentLevel;
    /**< Current indentation                  */
    int columnWrap;
    /**< Num of elements per row into array   */
    long offsetForLoop;
    /**< Loop rewinds always at this point    */
    long roundOff;
    /**< Max from real calcsize Roundoff      */
    long long backOff;
    /**< Num BackOff Bytes For AutoCalcSize   */
    long long objInitialOffset;    /**< Offset into stream where obj starts  */
    bool errorOccurred;
    /**< Error condition                      */
    jmp_buf recoveryJmp;         /**< Store the first Serialize status in case of any error to recover */
    bool recoveryJmpSet;      /**< if true than the recoveryJmp has been setted */
    /* TODO: Remember to enable it - 30-Jan-2012 */
    /* AnyEventInfo           *onBeginSerialize; */   /**< triggered on begin serialize */
    /* AnyEventInfo           *onEndSerialize;   */  /**< triggered on end serialize */
    /**/
}
        Serialize;

/*!
 * \brief Checks if an error or EOF is occurred
 * \param __self Serialize instance pointer
 */
#define SERIALIZE_IS_ERROR_OR_EOF( __self ) \
  ( Serialize_isEof( __self ) == true || Serialize_isErrorOccurred( __self ) == true )

/*---------------------------------------------------------------------------*/
/* Public prototypes                                                         */
/*---------------------------------------------------------------------------*/

/*!
  \brief Create a new Serialize-instance

  Create a new Serialize-instance.

  \return The pointer to the new created instance
*/
Serialize *Serialize_new( void );

/*!
  \brief Initialize a Serialize

  Must be called to initialize a Serialize

  \param self Pointer to a Serialize
  \param stream Pointer the IOChannel to use for streaming
  \param modes  Modes and flags for serialize setting

  The Serialize_init() must be called in order to initialize the
  Serialize object.
  The modes parameter is equal to those that are passed to
  Serialize_setMode(), so refer to this function for more details
  about modes.
  The Stream parameter can be a valid IOChannel instance or also NULL pointer:
  in the latter case you can set the stream at later time using the
  Serialize_setStream().

  This means that doing:

  \code
  serializer = Serialize_new();
  Serialize_init( serializer, streamer, SERIALIZE_STREAMMODE_NORMAL |
                                        SERIALIZE_MODE_WRITE|
                                        SERIALIZE_MODE_AUTOCALC );
  \endcode

  is equal to:

  \code
  serializer = Serialize_new();
  Serialize_init( serializer, NULL, SERIALIZE_STREAMMODE_NORMAL );

  Serialize_setStream( serializer, streamer );
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE|SERIALIZE_MODE_AUTOCALC );
  \endcode

  Example:

  \code
  int main( int argc, char *argv[] )
  {
    Serialize *serializer = (Serialize*)NULL;
    IOChannel *streamer = (IOChannel*)NULL;
    Point point = { POSX_VAL, POSY_VAL };

    // Create and open the stream: we wanna make data persistent
    // so we will use the "File://" stream...

    streamer = IOChannel_new();
    IOChannel_init( streamer );

    IOChannel_open( stream, "File://mySerialization.txt",
                  IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT, IOCHANNEL_MODE_TRUNC,
                  IOCHANNEL_PERMISSIOND_ALL );

    // Create the serializer and set it for write mode...
    serializer = Serialize_new();
    Serialize_init( serializer, streamer, SERIALIZE_STREAMMODE_NORMAL |
                                          SERIALIZE_MODE_WRITE );

    // Only set the format...
    Serialize_setFormat( serializer, "Xml", "" );

    // Serialize!
    Point_serialize( &point, "point", serializer );

    IOChannel_close( streamer );
    IOChannel_clear( streamer );
    IOChannel_delete( streamer );

    Serialize_clear( serializer );
    Serialize_delete( serializer );

    return 0;
  }
  \endcode

  \see Serialize_setStream
  \see Serialize_setMode

  \return True on success, false otherwise
*/
bool Serialize_init( Serialize *self, IOChannel *stream, int modes );

/*!
  \brief Check if a format is defined

  Ask Serialize if a format was already loaded
  by Serialize_setFormat()

  \param self Pointer to a Serialize
  \param format Character String Indicatin the format

  \return True if format is into the internal list, false otherwise

  \see Serialize_setFormat
  \see Serialize_addFormat
*/
bool Serialize_isFormatDefined( Serialize *self, char *format );

/*!
  \brief Add a format to Serialize

  Add a format into the list of avaiable ones: if a
  format with the same name was already defined, a call
  to Serialize_setFormat() will set the last added one.

  \param self Pointer to a Serialize
  \param format Point to a SerializeFormat operations

  \return True if format is into the internal list, false otherwise

  \see Serialize_setFormat
*/
bool Serialize_addFormat( Serialize *self, SerializeFormat *format );

/*!
  \brief Set the format

  \param self Pointer to a Serialize
  \param format Character string of format
  \param options Character string of format options

  This Function allows to set the format that is going
  to be used for serialize data. The string of the format
  is case senitive, so calling:

  \code
  status = Serialize_setFormat( serializer, "binary", NULL );
  \endcode

  is not equal to:

  \code
  status = Serialize_setFormat( serializer, "Binary", NULL );
  \endcode

  The 3rd parameter, depends from the format, so values like emty
  strings or NULL pointers may be allowed ( generally this set the default
  options ).
  Currently all format accepts NULL or emty strings ( "" ): the
  Ascii format in addition accepts as options:

  "WITH_TYPE=TRUE" or "WITH_TYPE=FALSE"

  passing a NULL pointer is equal to pass "WITH_TYPE=FALSE" ( which is the
  default behaviour ).

  If the function return false, maybe the Plugin of the specified
  format is not in your shared library path ( LD_LIBRARY_PATH on Linux )

  \return True on success, false otherwise

  \see Serialize_setFormatProperty
  \see Serialize_getFormatProperty

  \see Serialize_addFormat
*/
bool Serialize_setFormat( Serialize *self,
                          const char *format,
                          const char *options );

/*!
  \brief Set a Format Property

  If the currently set format allow it, this function can be used
  to change/set some format specific properties.

  \param self Pointer to a Serialize
  \param optName Character string of the name of the property
  \param opt Pointer to the value to assign to the property

  \return True on success, false otherwise

  \see Serialize_setFormat
  \see Serialize_getFormatProperty
*/
bool Serialize_setFormatProperty( Serialize *self, char *optName, void *opt );

/*!
  \brief Get a Format Property

  If the currently set format allow it, this function can be used
  to get some format specific properties.

  \param self Pointer to a Serialize
  \param optName Character string of the name of the property

  \return The pointer to the value of the property, NULL otherwise

  \see Serialize_setFormat

  \see Serialize_setFormat
  \see Serialize_setFormatProperty
*/
void *Serialize_getFormatProperty( Serialize *self, char *optName );

/*!
  \brief Set Modes and Flags for Serialize

  \param self Pointer to a Serialize
  \param modes Modes and Flags of Serialize

  This function allow to set the modes and the flags for serialization. There
  are some modes that MUST be set before serialize any data, and flags
  that can be optionally used but are NOT REQUIRED.

  The modes syntax is of this type:

  \ref SerializeMode | \ref SerializeStreamMode | \ref SerializeFlags

  The DIRECTION IS REQUIRED TO BE SET before serialization. You can choose
  between one \ref SerializeMode of these:

  (Mutually-exlusives)

  - SERIALIZE_MODE_WRITE

  - SERIALIZE_MODE_READ

  - SERIALIZE_MODE_CALC

  You can ADDITIONALLY unary-OR ONE of the previous modes with ZERO or ONE of
  the following \ref SerializeStreamMode FLAGS:

  (Mutually-exlusives)

  - SERIALIZE_STREAMMODE_NORMAL

  - SERIALIZE_STREAMMODE_FLUSH

  - SERIALIZE_STREAMMODE_LOOP

  So far, YOU CAN specify ONE of these \ref SerializeFlags FLAGS:

  (Mutually-exlusives)

  - SERIALIZE_MODE_AUTOCALC

  - SERIALIZE_MODE_TRANSLATE

  - SERIALIZE_MODE_NOHEADER


  - Detailed description

  The first three modes allows to set the direction of the serialization:
  the SERIALIZE_MODE_WRITE set the serialization to write data into the stream;
  this means that when you will set the stream using the Serialize_setStream()
  ( if you haven't set it yet ) you should use a stream which does not conflicts
  with the direction that has been chosen; this means that you cannot set a stream
  that is open in READ-only mode; if so, no error is generated until serialization
  starts. This is also true if you set the SERIALIZE_MODE_READ: in this case do not set
  a WRITE-only stream!
  If you use SERIALIZE_MODE_CALC, you do not have to provide any stream, and
  if a previous one is set, then Serialize automatically will unset it.
  The direction mode remains set until a new one is explicitly set: this
  means that you can set the direction once a time, and then change
  flags at your pleasure.

  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );


  // we are always in write mode after this call...
  Serialize_setMode( serializer, SERIALIZE_STREAMMODE_LOOP );


  // we are always in write mode after this call...
  Serialize_setMode( serializer, SERIALIZE_STREAMMODE_NORMAL |
                                 SERIALIZE_MODE_NOHEADER );

  \endcode

  The formats, which works in translate mode ( actually only MxArray )
  do not use streams, so the direction has a little less different
  meaning. This special cases are discussed in a dedicated paragraph.
  -- \ref SerializeTranslateModeInfo --

  The second group of modes influences the way in which data is
  streamed; suppose you wanna serialize in write mode some data:

  \code
  int main( int argc, char **argv )
  {
    Serialize *serializer = (Serialize*)NULL;
    Rectangle *rectangles[10];
    int retVal = 0;

    ...
    // Wanna serialize only some elements of the array ( e.g. 1, 4, 7 )
    Rectangle_serialize( rectangles[1], "rectangle", serializer );
    Rectangle_serialize( rectangles[4], "rectangle", serializer );
    Rectangle_serialize( rectangles[7], "rectangle", serializer );

    ...

    return retVal;
  }
  \endcode

  The SERIALIZE_STREAMMODE_NORMAL threats the stream as a continuous
  stream; this means that the three rectangle structures are written
  appending each one to the stream.
  If you are using e.g. a File:// stream at the end of the
  serialization you will find a file containing three
  rectangle structures.

  If you use SERIALIZE_STREAMMODE_LOOP, the next serialization will
  overwrite the previous one( flushing the last serialized data if
  stream uses bufferization ).
  If you are using a e.g. a File:// stream at the end of the
  serialization you will find a file containing one rectangle
  structurer, which correspond to the last serialized rectangle
  ( which has overwritten the previous onde ).

  The SERIALIZE_STREAMMODE_FLUSH is similar to SERIALIZE_STREAMMODE_NORMAL,
  but has some effects on  stream bufferization: if a bufferized
  stream if passed, before next serialization, Serialize will flush
  automatically the stream.

  By default if no one of these flags is specified, Serialize works
  using the SERIALIZE_STREAMMODE_NORMAL settings. A flag remains set
  until another one is explicitly set calling Serialize_setMode().

  \warning
  It is better to do not do explicit operations on the stream
  ( like calling IOChannel_seek()  IOChannel_tell() or other.. ) if you
  expect that these modes work correctly.

  The last three flags can be OR'ed to obtain the following results:

  SERIALIZE_MODE_AUTOCALC : if the stream is a bufferized stream or a memory
  based stream, then the Serialize will set automatically the object size
  parameter in the header.
  By default any time you call Serialize_setMode(), if the flag is not
  explicitly specified, this option is disabled.

  SERIALIZE_MODE_TRANSLATE: this flag tell the format to work in translate
  mode; not all the stream allow this mode, so if translate mode is set
  for a format that does not allow it, no error will arise until
  serialization starts.
  By default any time you call Serialize_setMode(), if the flag is not
  explicitly specified, this option is disabled.

  SERIALIZE_MODE_NOHEADER: when in write mode, this flags tells Serialize
  to do not print the header; in read mode, the flag tells Serialize to do
  not expect any header: be careful because this means that before
  de-serialize any data the format must be already set, because Serialize
  can't automatically set it ( differently from that happens when heder is used ).
  By default any time you call Serialize_setMode(), if the flag is not
  explicitly specified, this option is disabled.


  Some examples:

  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );
  \endcode

  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_READ| SERIALIZE_STREAMMODE_LOOP );
  \endcode

  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE |
                                 SERIALIZE_STREAMMODE_NORMAL|
                                 SERIALIZE_MODE_NOHEADER );
  \endcode

  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE | SERIALIZE_MODE_NOHEADER );
  \endcode

  The following:
  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );
  Serialize_setMode( serializer, SERIALIZE_MODE_AUTOCALC );
  \endcode

  is equal to:
  \code
  Serialize_setMode( serializer, SERIALIZE_MODE_WRITE| SERIALIZE_MODE_AUTOCALC );
  \endcode
*/
void Serialize_setMode( Serialize *self, int modes );

/*!
  \brief Set The Stream for Serialize

  The function allow to set the stream that will be used for serialize
  data. Serialize does not cares of what type of stream is passed, so
  can be used file streams, memory or Socket based stream. For more infos
  about streams, refer to the IOChannel documentation.

  No any check is done on stream( like if it is open or if modes that was opened
  conflicts with the Serialize modes ) until Serialization begins.

  \param self Pointer to a Serialize
  \param stream Pointer to a valid IOChannel instance

  \see Serialize_init
  \see Serialize_getStream
*/
void Serialize_setStream( Serialize *self, IOChannel *stream );

/*!
  \brief Get The Stream currently used

  The function allow to get the IOChannel instance pointer that
  is currently set ( and that has been previously set by the
  Serialize_init() or the Serialize_setStream() );

  \param self Pointer to a Serialize

  Example:
  \code
  Serialize *serializer = (Serialize*)NULL;
  IOChannel *stream = (IOChannel*)NULL;

  ...

  stream = Serialize_getStream( serializer );
  \endcode

  \return Pointer to the stream ( if set ), NULL otherwise

  \see Serialize_init
  \see Serialize_setStream
*/
IOChannel *Serialize_getStream( Serialize *self );

/*!
  \brief Retrieve the data pointer of the format

  \param self Pointer to a Serialize

  A format can sometimes use a private data to perform
  its tasks. You can access to the content of this data
  using the Serialize_setFormatProperty() and
  Serialize_getFormatProperty(); This function allow to get
  direcly the pointer of the internal structure which handles
  this data.
  This function was mainly provided for developing formats, so
  its better to access to this data using the set/get property
  functions

  \return Pointer to the specific format data, NULL otherwise.

  \see SerializeFormat

  \warning
    No All the formats use additional data, so a NULL
    return value does not means error.
*/
void *Serialize_getFormatDataPtr( Serialize *self );

/*!
  \brief Ask if Serialize is in Read mode

  \param self Pointer to a Serialize

  \return True if self is a Read mode, false otherwise

  \see Serialize_isWriting

  \ref SerializeSpecialFunctionHowTo
*/
bool Serialize_isReading( Serialize *self );

/*!
  \brief Ask if Serialize is in Write mode

  \param self Pointer to a Serialize

  \return True if self is a Write mode, false otherwise

  \see Serialize_isReading

  \ref SerializeSpecialFunctionHowTo
*/
bool Serialize_isWriting( Serialize *self );

/*!
  \brief Ask if internal error is set

  \param self Pointer to a Serialize

  \return True in case of error, false otherwise

  \see Serialize_cleanError
*/
bool Serialize_isErrorOccurred( Serialize *self );

/*!
  \brief Ask if end of file reached

  \param self Pointer to a Serialize

  \return True in case end of file has been reached, false otherwise

  \see Serialize_isErrorOccurred
*/
bool Serialize_isEof( Serialize *self );

/*!
  \brief Clean internal error if set

  \param self Pointer to a Serialize

  \see Serialize_isErrorOccurred
*/
void Serialize_cleanError( Serialize *self );

/*!
  \brief Get The value of current columnWrap

  \param self Pointer to a Serialize

  \return The current columnWrap

  \see Serialize_setColumnWrap
*/
int Serialize_getColumnWrap( Serialize *self );

/*!
  \brief Set the number of columns in which array are printed

  Set the number of columns in which array are printed when
  a text based format is used: this means that has not sense
  set columnWrap when using Binary Format.

  \param self Pointer to a Serialize
  \param columnWrap Number of columns

  \see Serialize_getColumnWrap
*/
void Serialize_setColumnWrap( Serialize *self, unsigned int columnWrap );

/*!
  \brief Returns the length of the serialization header

  \param self Pointer to a Serialize object

  After a serialization, this function returns the number of bytes
  needed by the header to serialize data in the current format.
  This function can be called after a serialization was done
  in write, read or calcsize mode.

  \return Total length of the header, included the trailing '\\n'

  \see Serialize_getPayloadSize
  \see Serialize_getMaxSerializeSize
*/
long Serialize_getHeaderSize( Serialize *self );

/*!
  \brief Returns the total length of the serialized object including header + payload

  \param self Pointer to a Serialize object

  After a serialization, this function returns the number of bytes
  needed by the header + payload to serialize data in the current format.
  This function can be called after a serialization was done
  in write, read or calcsize mode.

  \return Total length of the header + payload, included the trailing '\\n'

  \see Serialize_getPayloadSize
  \see Serialize_getHeaderSize
  \see Serialize_getMaxSerializeSize
*/
long Serialize_getTotalSize( Serialize *self );

/*!
  \brief Get the size of Serialized data

  \param self Pointer to a Serialize object

  After a serialization, this function returns the number of bytes
  needed by the object to serialize data in the current format.
  This function can be called after a serialization was done
  in write, read or calcsize mode.

  \return Total size of the object in the current set format

  \see Serialize_getHeaderSize
  \see Serialize_getTotalSize
  \see Serialize_getMaxSerializeSize
*/
long Serialize_getPayloadSize( Serialize *self );

/*!
  \brief Get the maximum serialization size of specific text based format

  \param self Pointer to a Serialize object

  Called after a Serialization in write ot calsize mode, this function allows to
  get the maximum number of bytes which are needed to serialize an object
  in the specific text based format( Ascii, Xml.. ). An object in fact can
  have different sizes depending from the values of its fields.
  Look at this:

  \code
  typedef struct Point
  {
    int posX;
    int posY;
  }
  Point;
  \endcode

  If you create an instace of this structure with posX = 5 and posY = 12, and
  then serialize it on Ascii format you have that REAL serialize size that is
  calculated as the size needed by the following text:

  \code
  struct Point
  {
    int posX = 5;
    int posY = 12;
  };
  \endcode

  But the MAXIMUM serialize size is calculated as the size of this:

  \code
  struct Point
  {
    int posX = +0000000005;
    int posY = +0000000012;
  };
  \endcode


  This means that two instances of the same object which have different fields values
  can have different serialize size  ( the value returned by Serialize_getPayloadSize() )
  but the value returned by Serialize_getMaxSerializeSize() is always equal.

  As you can undestand, for Binary format calling Serialize_getPayloadSize()
  and Serialize_getMaxSerializeSize() will return the same value.

  \return The maximum size of the object

  \see Serialize_getHeaderSize
  \see Serialize_getPayloadSize
  \see Serialize_getTotalSize
*/
long Serialize_getMaxSerializeSize( Serialize *self );

/*!
 * Default behaviour for quitting from the Serialize_beginType() macro
 * The user might redefine it before including the Serialize.h for handling
 * different quitting strategy within the Serialize function
 */
#if !defined(SERIALIZE_EXITFROMBEGINTYPE)
#define SERIALIZE_EXITFROMBEGINTYPE    return
#endif

/*!
  \brief Start a new type

  \param __self Pointer to a Serialize
  \param __name the name of the type-instance (must NOT be empty or NULL!)
  \param __type the name of the object

  When implementing a serialize function, all the Serialize API must be
  enclosed between Serialize_beginType() and Serialize_endType(), as
  shown in the implementation of the following serialize functions:

  \code

  typedef struct Point
  {
    int  posX;
    int  posY;
  }
  Point;

  typedef struct Rectangle
  {
    Point         leftUpperCorner;
    unsigned int  height;
    unsigned int  width;
  }
  Rectangle;

  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    Serialize_beginType( s, name, (char*)"Point" );
      Int_serialize( &(self->posX), (char*)"posX", s );
      Int_serialize( &(self->posY), (char*)"posY", s );
    Serialize_endType( s );
  }

  void Rectangle_serialize( Rectangle *self, char *name, Serialize *s )
  {
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char*)"Rectangle" );
      Point_serialize( &(self->leftUpperCorner), (char*)"leftUpperCorner", s );
      UInt_serialize( &(self->width), (char*)"width", s );
      UInt_serialize( &(self->height), (char*)"height", s );
    Serialize_endType( s );
  }
  \endcode

  \note Please note that any code written after the Serialize_endType() will
        not be executed in case of error

  \see Serialize_endType
*/
#define Serialize_beginType( __self, __name, __type ) \
do { \
  Serialize*  __mySelf = __self;\
  const char* __myName = __name;\
  const char* __myType = __type;\
\
  if ( __self->recoveryJmpSet == false )\
  {\
    __self->recoveryJmpSet = true;\
\
    if ( setjmp( __self->recoveryJmp ) != 0 )\
    {\
      ANY_LOG( 0, "Aborting serialization function", ANY_LOG_ERROR );\
      SERIALIZE_EXITFROMBEGINTYPE;\
    }\
  }\
\
  Serialize_internalBeginType( __mySelf, __myName, __myType );\
} while( 0 )

/*!
  \brief Start a new internal type
  \param self Pointer to a Serialize
  \param name the name of the type-instance (must NOT be empty or NULL!)
  \param type the name of the object

  When implementing a serialize function, all the Serialize API must be
  enclosed between \a Serialize_beginType() and \a Serialize_endType(). The process
  of defining a new function passes through this internal function, which performs
  some internal preparation

  \see Serialize_beginType
  \see Serialize_endType
*/
void Serialize_internalBeginType( Serialize *self, const char *name, const char *type );

/*!
  \brief Start a new Base type

  It can be used instead of Serialize_beginType().
  A block that is opened with Serialize_beginBaseType() must be
  closed using Serialize_endBaseType().

  When this function is used, and the nesting level is greater than
  one, no substructure is created, but its fields are direcly
  serialized.

  \code
  typedef struct Point
  {
    int  posX;
    int  posY;
  }
  Point;

  typedef struct Rectangle
  {
    Point         leftUpperCorner;
    unsigned int  height;
    unsigned int  width;
  }
  Rectangle;

  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    Serialize_beginType( s, name, (char*)"Point" );
      Int_serialize( &(self->posX), (char*)"posX", s );
      Int_serialize( &(self->posY), (char*)"posY", s );
    Serialize_endType( s );
  }

  void Rectangle_serialize( Rectangle *self, char *name, Serialize *s )
  {
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char*)"Rectangle" );
      Point_serialize( &(self->leftUpperCorner), (char*)"leftUpperCorner", s );
      UInt_serialize( &(self->width), (char*)"width", s );
      UInt_serialize( &(self->height), (char*)"height", s );
    Serialize_endType( s );
  }
  \endcode

  The standard usage, will produce the following output ( using for
  example the Matlab format )

  Rectangle.Point.leftUpperCorner.posX = 12;
  Rectangle.Point.leftUpperCorner.posY = 25;
  Rectangle.height = 77;
  Rectangle.width = 1;


  But If the Point_serialize() would be defined in this way:

  \code
  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    Serialize_beginBaseType( s, name, (char*)"Point" );
      Int_serialize( &(self->posX), (char*)"posX", s );
      Int_serialize( &(self->posY), (char*)"posY", s );
    Serialize_endBaseType( s );
  }
  \endcode

  The output would be this:

  Rectangle.leftUpperCorner.posX = 12;
  Rectangle.leftUpperCorner.posY = 25;
  Rectangle.height = 77;
  Rectangle.width = 1;

  This behaviour is valid for all the formats, expect for
  the Binary one.
  Note: also the MxArray format ( which works in translate mode only )
  allow this.

  \param self Pointer to a Serialize
  \param name the name of the type-instance (must NOT be empty or NULL!)
  \param type the name of the object

  \see Serialize_endBaseType
*/
void Serialize_beginBaseType( Serialize *self, const char *name, const char *type );

/*!
  \brief Serialize the type

  The function must never be used by the user, it is the gataway for serialize
  any data type.

  \param self  Pointer to a Serialize
  \param type  the type of the  element(s) to serialize
  \param name  the name of the element(s) to serialize (must not be empty)
  \param value Pointer to the value of the element(s) to serialize
  \param size  size of single element to serialize ( expressed in bytes )
  \param len   the number length of element to serialize

  \see SerializeType
*/
void Serialize_doSerialize( Serialize *self,
                            SerializeType type,
                            const char *name,
                            void *value,
                            size_t size,
                            const int len );

/*!
  \brief End a new Base type

  This function closes a block opened by the Serialize_beginBaseType().
  FOR EXPLANATIONS on its USAGE see Serialize_beginType();

  \param self Pointer to a Serialize

  \see Serialize_beginBaseType
*/
void Serialize_endBaseType( Serialize *self );

/*!
  \brief End a new type

  \param __self Pointer to a Serialize instance

  When implementing a serialize function, all the Serialize API must be
  enclosed between Serialize_beginType() and Serialize_endType(), as
  shown in the implementation of the following serialize functions:

  \code

  typedef struct Point
  {
    int  posX;
    int  posY;
  }
  Point;

  typedef struct Rectangle
  {
    Point         leftUpperCorner;
    unsigned int  height;
    unsigned int  width;
  }
  Rectangle;

  void Point_serialize( Point *self, char *name, Serialize *s )
  {
    Serialize_beginType( s, name, (char*)"Point" );
      Int_serialize( &(self->posX), (char*)"posX", s );
      Int_serialize( &(self->posY), (char*)"posY", s );
    Serialize_endType( s );
  }

  void Rectangle_serialize( Rectangle *self, char *name, Serialize *s )
  {
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name, (char*)"Rectangle" );
      Point_serialize( &(self->leftUpperCorner), (char*)"leftUpperCorner", s );
      UInt_serialize( &(self->width), (char*)"width", s );
      UInt_serialize( &(self->height), (char*)"height", s );
    Serialize_endType( s );
  }
  \endcode

  \note Please note that any code written after the Serialize_endType() will
        not be executed in case of error

  \see Serialize_beginType
*/

#define Serialize_endType( __self ) \
do {\
  Serialize_internalEndType( __self );\
  __self->recoveryJmpSet = false;\
} while( 0 )

void Serialize_internalEndType( Serialize *self );

/*!
  \brief Set header elements sizes.

  The header structure internally contains several arrays that will
  contain the values of the elements of the header, like "name" and
  "type". The size of these arrays can be set using this function. If
  0 is passed as value, that specific array will be initialized using
  the default size.
  Call this function after having initialized the Serialize instance.

  \see SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE
 */
void Serialize_setHeaderSizes( Serialize *self, const int typeSize, const int nameSize, const int optsSize,
                               const int formatSize );

/*!
  \brief Retrieves the full package header as string into result buffer

  Any serialized data is generally precedeed by a line, called header, wich contains
  informations about the object that follows. The header has this format:

  \code
    HRIS-<majVersion>.<minVersion> type = '<dataType>' name =  <dataName> objSize = <size> format = <formatMode>[ opts = 'formatOptions']\n
  \endcode

  majVersion and minVersion are two integer which tells what parser version was
  used to generate the header (when writing) and what version has to be used for
  reading. The other fields follow the syntax of the Parser version 2.0 (this
  means that you have an header which starts with "HRIS-2.0"):

  The 1st field is the Type of data which is serialized \see Serialize_getHeaderType

  The 2nd field is the Name of the data which is serialized \see Serialize_getHeaderName

  The 3rd field is the Size of the object that follows in the specified format

  The 4th field tells in what Format the object that follows was serialized

  The 5th ( optional) can specify some special options of the format \see Serialize_getHeaderOpts

  The header is ALWAYS terminated by a new line.


  Please ensure that the result buffer is at least \a SERIALIZE_HEADER_MAXLEN Bytes
  large, else some information may be lost.

  \param self Pointer to a Serialize object
  \param buffToStoreResult buffer where the information should be stored
  \param bufSize buffer size
*/
bool Serialize_getHeader( Serialize *self, char *buffToStoreResult, int bufSize );

/*!
  \brief Retrieves all the information written in the header

  Reads from the stream the header of the Serialization, without having side effects on the IOChannel, and saves the information in the pointers passed as arguments.

  \param self Pointer to a Serialize object
  \param type Type of the serialized object
  \param name Name of the serialized object
  \param objSize Size of the serialized object
  \param format Serialization format
  \param opts Serialization format options
*/
bool Serialize_peekHeader( Serialize *self,
                           char *type,
                           char *name,
                           int *objSize,
                           char *format,
                           char *opts );

/*!
  \brief Retrieves the data type information from header

  After a Serialization, this allows tho get the character string
  corresponding to the data type which was serialized. The max
  size of the data type is SERIALIZE_HEADER_DATATYPEMAXLEN.

  \code
  MyStruct_serialize( MyStruct *self, char *name, Serialize *s )
  {
   Serialize_beginType( s, name, "MyStruct" )
   ...

    Serialize_endType( s );
  }

  \endcode
  The data type is represented by the character string "MyStruct"
  found in the begin type.

  \param self Pointer to a Serialize object
  \param buffToStoreResult buffer where the information should be stored

  \see Serialize_getHeaderTypePtr
*/
void Serialize_getHeaderType( Serialize *self, char *buffToStoreResult );

/*!
  \brief Get the data type pointer

  This function allows to get the internal pointer which points to
  the character string TYPE of the the last serialized data.

  Example:

  \code
  ANY_LOG( 5, "Serialized data type is %s", ANY_LOG_INFO,
              Serialize_getHeaderTypePtr( serializer ) );
  \endcode

  \param self Pointer to a Serialize object

  \return The pointer to the internal character string

  More details on Serialize_getHeaderType()

  \see Serialize_getHeaderType
*/
char *Serialize_getHeaderTypePtr( Serialize *self );

/*!
  \brief Retrieves the data name information from header

  After a Serialization, this allows tho get the character string
  corresponding to the data name which was serialized. The max
  size of the data type is SERIALIZE_HEADER_DATANAMEMAXLEN.

  \code
  MyStruct_serialize( MyStruct *self, char *name, Serialize *s )
  {
   Serialize_beginType( s, name, "MyStruct" )
   ...

  }
  ...

  int main( int argc, char **argv )
  {
    Serialize *serializer = (Serialize*)NULL;
    MyStruct *myStruct = (MyStruct*)NULL;

    ...

    MyStruct_serialize( myStruct, "myStructName", serializer );

    ...
  }
  \endcode

  The data name is represented by the character string "myStructName".

  \param self Pointer to a Serialize object
  \param buffToStoreResult buffer where the information should be stored

  \see Serialize_getHeaderNamePtr
*/
void Serialize_getHeaderName( Serialize *self, char *buffToStoreResult );

/*!
  \brief Get the data name pointer

  This function allows to get the internal pointer which points to
  the character string NAME of the the last serialized data.

  Example:

  \code
  ANY_LOG( 5, "Serialized data name is %s", ANY_LOG_INFO,
              Serialize_getHeaderNamePtr( serializer ) );
  \endcode

  \param self Pointer to a Serialize object

  \return The pointer to the internal character string

  More details on Serialize_getHeaderName()

  \see Serialize_getHeaderName
*/
char *Serialize_getHeaderNamePtr( Serialize *self );

/*!
  \brief Retrieves the options information from header

  \param self Pointer to a Serialize object
  \param buffToStoreResult buffer where the information should be stored

  After a Serialization, this allows tho get the character string
  corresponding to the format options

  For correct working, you must pass a buffer at least of
  SERIALIZE_HEADER_DATAOPTSMAXLEN.

  Look at this:

  \code

  int main( iont argc, char **argv )
  {
    char buffer[SERIALIZE_HEADER_DATAOPTSMAXLEN];
    Serialize *serializer = (Serialize*)NULL;
    Rectangle *rectangle = (Rectangle*)NULL;

    Serialize_setFormat( s, "Ascii", "WITH_TYPE=TRUE" );

    Rectangle_serialize( rectangle, "myREctangle", serializer );

    Serialize_getHeaderOpts( s, buffer );
  }
  \endcode

  The buffer will contain the string "WITH_TYPE=TRUE"

  \see Serialize_getHeaderOptsPtr

 */
void Serialize_getHeaderOpts( Serialize *self, char *buffToStoreResult );

/*!
  \brief Get the pointer to the format options

  \param self Pointer to a Serialize object

  Get the pointer to the internal string which stores the options passed
  by the Serialize_setFormat() when writing, or found into the header when
  reading.

  \code
    ANY_LOG( 5, "Options of the used format are %s", ANY_LOG_INFO,
                Serialize_getHeaderOptsPtr( serializer ) );
  \endcode

  \return The pointer to the internal character string

  More details on Serialize_getHeaderOpts()

  \see Serialize_getHeaderOptsPtr

*/
char *Serialize_getHeaderOptsPtr( Serialize *self );

/*!
  \brief Set Serialize init mode flag

  \param self Pointer to a Serialize object
  \param status True of false
*/
void Serialize_setInitMode( Serialize *self, bool status );

/*!
  \brief Ask if init mode is set

  \param self Pointer to a Serialize object

  \return True if init mode is set, false otherwise
*/
bool Serialize_isInitMode( Serialize *self );

/*!
  \brief Check if machine byte order is little endian.

  \return True, if byte order is little endian, false otherwise.
*/
bool Serialize_isLittleEndian( void );

/*!
  \brief Clear a Serialize

  \param self Pointer to a Serialize object

   Clear an allocated Serialize.
*/
void Serialize_clear( Serialize *self );

/*!
  \brief Delete a Serialize

  \param self Pointer to a Serialize object

   Delete completely a Serialize
*/
void Serialize_delete( Serialize *self );

/*!
  \brief Retrieve the Serialize_beginType() nesting level

  \param self Pointer to a Serialize object

  This function is generally used for developing formats.
  It returns the number of Serialize_beginType() level of nesting.
  Every time a Serialize_beginType() is encountered, this number
  is increased, vice-versa when Serialize_endType() is encountered
  the value is decreased.
  The nesting level cannot be a negative number.

  \return Current nesting level
*/
int Serialize_getBeginTypeNestingLevel( Serialize *self );

bool Serialize_getInsideBasicArray( Serialize *self );

long Serialize_printf( Serialize *self, const char *fmt, ... );

long Serialize_indent( Serialize *self );

long Serialize_scanf( Serialize *self, const char *fmt, ... );

/** Internal Use Only **/

void Serialize_beginArray( Serialize *self,
                           SerializeType type,
                           const char *name, const int len );

void Serialize_beginStructArray( Serialize *self,
                                 const char *arrayName,
                                 const char *elementType,
                                 const int arrayLen );


void Serialize_beginStructArraySeparator( Serialize *self,
                                          const char *name,
                                          const int pos,
                                          const int len );

bool Serialize_deployDataType( Serialize *self,
                               SerializeType type,
                               SerializeDeployDataMode,
                               const char *spec,
                               int notYet,
                               long len,
                               void *data );

void Serialize_endStructArraySeparator( Serialize *self,
                                        const char *name,
                                        const int pos,
                                        const int len );

void Serialize_endStructArray( Serialize *self );

void Serialize_endArray( Serialize *self,
                         SerializeType type,
                         const char *name,
                         const int len );


void Serialize_onBeginSerialize( Serialize *self, void (*function)( void * ), void *functionParam );

void Serialize_onEndSerialize( Serialize *self, void (*function)( void * ), void *functionParam );

#if defined(__cplusplus)
}
#endif

#include <SerializeTypes.h>
#include <SerializeStructTypes.h>

#endif /* SERIALIZE_H */
