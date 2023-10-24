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


#ifndef BBDM_C_H
#define BBDM_C_H


/*--------------------------------------------------------------------------*/
/* Include files                                                            */
/*--------------------------------------------------------------------------*/


#include <ctype.h>

#include <Any.h>
#include <Base.h>
#include <Serialize.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/


/*!
 * \brief maximum length of a BBDM instance name
 */
#define BBDM_MAXINSTANCENAMELEN ( 128 )


/*--------------------------------------------------------------------------*/
/* Datatypes                                                                */
/*--------------------------------------------------------------------------*/


/*!
 * \brief additional datatype-independent BBDM data
 *
 * Common further data along the payload, f.i. timestep used for
 * synchronization.
 */
typedef struct BBDMTag
{
    BaseI64 timestep;
    char instanceName[BBDM_MAXINSTANCENAMELEN];

} BBDMTag;


/*!
 * \brief BBDM Masterlist (for fast switch-statements in polymorph components)
 */
typedef enum
{
    BBDMTYPE_NONE,
    BBDMTYPE_ARRAY2DPOINT,
    BBDMTYPE_ARRAY2DRECT,
    BBDMTYPE_ARRAY2DSIZE,
    BBDMTYPE_ARRAYBLOCKF32,
    BBDMTYPE_ARRAYBLOCKI16,
    BBDMTYPE_ARRAYBLOCKUI16,
    BBDMTYPE_ARRAYBLOCKUI8,
    BBDMTYPE_ARRAYF32,
    BBDMTYPE_ARRAYF64,
    BBDMTYPE_ARRAYI32,
    BBDMTYPE_ARRAYI64,
    BBDMTYPE_ARRAYMEMI8,
    BBDMTYPE_ARRAYSPARSEBLOCKF32,
    BBDMTYPE_BASE2DF32,
    BBDMTYPE_BASE2DPOINT,
    BBDMTYPE_BASE2DRECT,
    BBDMTYPE_BASE2DSIZE,
    BBDMTYPE_BASEBOOL,
    BBDMTYPE_BASEF32,
    BBDMTYPE_BASEF64,
    BBDMTYPE_BASEI16,
    BBDMTYPE_BASEI32,
    BBDMTYPE_BASEI64,
    BBDMTYPE_BASEI8,
    BBDMTYPE_BLOCKF32,
    BBDMTYPE_BLOCKI16,
    BBDMTYPE_BLOCKI64,
    BBDMTYPE_BLOCKUI8,
    BBDMTYPE_MEMI8,
    BBDMTYPE_PQUEUEARRAY,
    BBDMTYPE_SPARSEBLOCKF32,
    BBDMTYPE_STRING,
    BBDMTYPE_TIMELABELEDBUFFER,
    BBDMTYPE_IPLIMAGE,
    BBDMTYPE_XIF
} BBDMType;


/*!
 * \brief Retrieve meta-info about payload content type
 *
 * A BBDMProperties struct contains various meta-info about the inner payload
 * data (not the BBDM itself), such as image width and height, number of array
 * dimensions or length of strings. The generic BBDM interface allows polymorph
 * / type-independent access to such data.
 *
 * \see \ref ToolBOS_HowTo_Polymorphism
 */
typedef struct BBDMProperties
{
    BaseI32 width;
    BaseI32 height;
    BaseI32 length;
    BaseI32 maxNoSparseEntries;
    BaseI32 size1;
    BaseI32 size2;
    BaseI32 size3;
    BaseI32 size4;
    BaseI32 totalSize;

    struct
    {
        /*! plain C or Base-types (int, BaseF32)             */
        unsigned int scalar   : 1;

        /*! struct types (Base2DPoint, BlockF32, MemI8)      */
        unsigned int compound : 1;

        /*! any datatype defined in BPL (ArrayF32, BlockF32) */
        unsigned int bplType  : 1;

        /*! BPL array types (ArrayF32, ArrayBlockF32)        */
        unsigned int bplArray : 1;

        /*! BPL block types (BlockUI8, SparseBlockF32)       */
        unsigned int bplBlock : 1;

        /*! Base.h mem-types (MemI8, ArrayMemI8)             */
        unsigned int memType  : 1;
    } type;

    BBDMType id;

} BBDMProperties;


/*--------------------------------------------------------------------------*/
/* Public functions                                                         */
/*--------------------------------------------------------------------------*/


typedef void (*BBDMFunction)( void );

typedef void *(*BBDMNewFunc)( void );

typedef void (*BBDMInitFromStringFunc )( void *self, const char *initString );

typedef void (*BBDMClearFunc)( void *self );

typedef void (*BBDMDeleteFunc)( void *self );

typedef char *(*BBDMGetInstanceNameFunc)( void *self );

typedef void (*BBDMSetInstanceNameFunc)( void *self, const char *instanceName );

typedef BaseI64 (*BBDMGetTimestepFunc)( void *self );

typedef void (*BBDMSetTimestepFunc)( void *self, BaseI64 instanceName );

typedef void *(*BBDMGetDataFunc)( void *self );

typedef int (*BBDMCopyDataFunc)( void *self, const void *src );

typedef void *(*BBDMGetPropertiesFunc)( void *self, BBDMProperties *p );

typedef void (*BBDMRandFunc)( void *self,
                              BaseF64 valueMin,
                              BaseF64 valueMax,
                              unsigned int *randomSeedState );

BaseBool BBDMProperties_isEQ( const BBDMProperties *self,
                              const BBDMProperties *src );


/*--------------------------------------------------------------------------*/
/* Component definition                                                     */
/*--------------------------------------------------------------------------*/


/*!
 * \brief Initializes the information export mechanism.
 *
 * \param componentName name of component
 *
 * This macro has to be put into the
 * header file.
 */
#define BBDM_INFO_EXPORT( componentName ) \
\
extern char *componentName##_info[]


/*!
 * \brief Starts the data component information section in the
 * implementation file.
 *
 * \param componentName name of component
 *
 *
 * Example:
 * \code
 * BBDM_INFO_BEGIN( BBDMMemI8 )
 * BBDM_INFO_DESCRIPTION( "Wrapper for MemI8" )
 * BBDM_INFO_INITSTRING_BEGIN
 * BBDM_INFO_INITFIELD( "length", "BaseUI32", "%u", "10",
 *                      "buffer length", "[0..MAX_INT]" )
 * BBDM_INFO_INITSTRING_END
 * BBDM_INFO_END;
 * \endcode
 *
 * \see BBDM_INFO_END
 */
#define BBDM_INFO_BEGIN( componentName ) \
char *componentName##_info[] = \
{ (char*)"@BBDM-INFO-BEGIN", \
(char*)"@Name: " #componentName,


/*!
 * \brief Describes the data component itself in the implementation file.
 *
 * \param componentDescription a description of the component
 *
 * \see BBDM_INFO_BEGIN
 */
#define BBDM_INFO_DESCRIPTION( componentDescription ) \
\
(char*)"@Description: " componentDescription,


/*!
 * \brief Starts the init parameter information section in the
 * implementation file.
 *
 * \see BBDM_INFO_INITSTRING_END
 * \see BBDM_INFO_BEGIN
 */
#define BBDM_INFO_INITSTRING_BEGIN


/*!
 * \brief Describes a parameter of the _init() function that is
 * used to initialize a component instance.
 *
 * \param fieldName the name of the parameter
 * \param fieldType the type of the parameter
 * \param fieldPattern the scanf() conversion pattern
 * \param fieldDefaultValue the default value of the parameter
 * \param fieldDescription a description of the parameter
 * \param fieldRangeDescription a description of
 *        the admissible range of values for this parameter
 *
 * \see BBDM_INFO_INITSTRING_BEGIN
 * \see BBDM_INFO_INITSTRING_END
 * \see BBDM_INFO_BEGIN
 */
#define BBDM_INFO_INITFIELD( fieldName, fieldType, fieldPattern, fieldDefaultValue, fieldDescription, fieldRangeDescription ) \
(char*)"@InitStringField: " fieldName ", " fieldType ", " fieldPattern \
                ", " fieldDefaultValue ", " fieldDescription \
                ", " fieldRangeDescription,


/*!
 * \brief Finishes the init parameter information section in the
 * implementation file.
 *
 * \see BBDM_INFO_INITSTRING_BEGIN
 * \see BBDM_INFO_BEGIN
 */
#define BBDM_INFO_INITSTRING_END


/*!
 * \brief Finishes the component information section in the header file.
 *
 * \see BBDM_INFO_BEGIN
 */
#define BBDM_INFO_END \
\
(char*)"@BBDM-INFO-END", (char*)NULL }


#define BBDM_INFO_FIELD( fieldName, fieldType, fieldValue )                  \
(char*)"@BBDM-INFO-FIELD: " fieldName ", " fieldType ", " fieldValue,


/*--------------------------------------------------------------------------*/
/* Reference parsing                                                        */
/*--------------------------------------------------------------------------*/


/*!
 * \brief Starts the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * \param initString the initialization string
 *
 * This macro is always followed by the macro BBDM_INITFIELD_BEGIN.
 *
 * Example:
 * \code
 * int BBDMMemI8_initFromString( BBDMMemI8 *self, const char *initString )
 * {
 *   BaseUI32 length = BBDMMEMI8_DEFAULT_LENGTH;
 *
 *   ANY_REQUIRE( self );
 *
 *   if ( initString != (char*)NULL )
 *   {
 *
 *     BBDM_INITSTRING_PARSE_BEGIN( initString )
 *     {
 *       BBDM_INITFIELD_BEGIN
 *       BBDM_INITFIELD_GET( length, "%u", &length )
 *       {
 *       }
 *       BBDM_INITFIELD_END
 *     }
 *     BBDM_INITSTRING_PARSE_END;
 *
 *  }
 *
 * return BBDMMemI8_init( self, length );
 * }
 * \endcode
 *
 *
 * \see BBDM_INITSTRING_PARSE_END
 * \see BBDM_INITFIELD_BEGIN
 */
#define BBDM_INITSTRING_PARSE_BEGIN( initString ) \
do\
{\
  const char *__initString = initString;\
  int __currPos = 0;\
  int __tokenPos = 0;\
  int __tokenLen = 0;\
  bool __equalFound = false;\
  int __initStringLen = 0;\
  char __ch = '\0';\
\
  if ( __initString != (char*)NULL )\
  {\
    __initStringLen = Any_strlen( __initString );\
\
    while ( __currPos < __initStringLen )\
    {\
      __ch = __initString[ __currPos ];\
\
      while ( ( __currPos < __initStringLen ) && ( isspace( (int)__ch ) ) )\
      {\
        ++__currPos;\
        __ch = __initString[ __currPos ];\
      }\
\
      __tokenPos = __currPos;\
\
      if ( isalpha( (int)__ch ) || ( __ch == '_' ) )\
      {\
        while ( ( __currPos < __initStringLen ) && ( isalnum( (int)__ch ) || ( __ch == '_' ) ) )\
        {\
          ++__currPos;\
          __ch = __initString[ __currPos ];\
        }\
      }\
\
      __tokenLen = __currPos-__tokenPos;\
\
      while ( ( __currPos < __initStringLen ) && ( isspace( (int)__ch ) ) )\
      {\
        ++__currPos;\
        __ch = __initString[ __currPos ];\
      }\
\
      if ( ( __tokenLen > 0 ) && ( __ch == '=' ) )\
      {\
        ++__currPos;\
        __ch = __initString[ __currPos ];\
        __equalFound = true;\
      }\
      else\
        __equalFound = false;\
\
      while ( ( __currPos < __initStringLen ) && ( isspace( (int)__ch ) ) )\
      {\
        ++__currPos;\
        __ch = __initString[ __currPos ];\
      }\
\
      if ( ( __tokenLen > 0 ) && ( __equalFound == true ) )\
      {


/*!
 *
 * \brief Finishes the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * This macro is always preceded by the macro BBDM_INITFIELD_END.
 *
 * \see BBDM_INITSTRING_PARSE_BEGIN
 * \see BBDM_INITFIELD_END
 */
#define BBDM_INITSTRING_PARSE_END \
      }\
\
      if ( ( __tokenLen == 0 ) || ( __equalFound == true ) )\
      {\
        if ( __ch == '"' )\
        {\
          ++__currPos;\
          __ch = __initString[ __currPos ];\
\
          while ( ( __currPos < __initStringLen ) && ( __ch != '"' ) )\
          {\
            ++__currPos;\
            __ch = __initString[ __currPos ];\
          }\
\
          ++__currPos;\
        }\
        else\
        {\
          while ( ( __currPos < __initStringLen ) && ( !isspace( (int)__ch ) ) )\
          {\
            ++__currPos;\
            __ch = __initString[ __currPos ];\
          }\
        }\
      }\
    }\
  }\
}\
while( 0 )


/*!
 * \brief Starts the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * This macro is always preceded by the
 * macro BBDM_INITSTRING_PARSE_BEGIN.
 *
 * \see BBDM_INITSTRING_PARSE_BEGIN
 * \see BBDM_INITFIELD_END
 */
#define BBDM_INITFIELD_BEGIN \
 if ( 0 )\
 {\
   {


#if defined(__GNUC__) && ( __GNUC__ < 3 ) && defined(__cplusplus)

#define BBDM_INITFIELD_GET( fieldName, fieldPattern, VaArgs... ) \
   }\
 }\
 else if ( Any_strncmp( #fieldName, &__initString[__tokenPos], __tokenLen ) == 0 ) \
 {\
   ANY_LOG( 5, "Parsing field: " #fieldName, ANY_LOG_INFO );\
   Any_sscanf( &__initString[__currPos], fieldPattern, VaArgs );\
   {

#else

/*!
 *
 * \brief Gets the initial value of parameter(s) of _init().
 *
 * \param fieldName the name of the parameter(s)
 * \param fieldPattern the scanf() conversion
 *        specifier(s) of the parameter(s)
 * \param ... variable argument list
 *
 * This macro extracts the value of the parameter from the initialization
 * string inside the initialization string parsing section of the
 * _initFromString() function.
 *
 *
 * \see BBDM_INITSTRING_PARSE_BEGIN
 */
#define BBDM_INITFIELD_GET( fieldName, fieldPattern, ... ) \
   }\
 }\
 else if ( Any_strncmp( #fieldName, &__initString[__tokenPos], __tokenLen ) == 0 ) \
 {\
   ANY_LOG( 5, "Parsing field: " #fieldName, ANY_LOG_INFO );\
   Any_sscanf( &__initString[__currPos], fieldPattern, __VA_ARGS__ );\
   {

#endif


/*!
 * \brief Finishes the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * This macro is always followed by the
 * macro BBDM_INITSTRING_PARSE_END.
 *
 * \see BBDM_INITSTRING_PARSE_END
 * \see BBDM_INITFIELD_BEGIN
 * \see BBDM_INITSTRING_PARSE_BEGIN
 */
#define BBDM_INITFIELD_END \
   }\
 }

/* XML initialization support */

#define BBDM_INITXML_PARSE_BEGIN( __moduleName, __xmlString )           \
    do {                                                                \
        int __len;                                                      \
        xmlDocPtr __doc;                                                \
        xmlNodePtr __node;                                              \
        const xmlChar *__parameterName;                                 \
        xmlChar *__parameterValueUtf8Str;                               \
        xmlAttrPtr __attr;                                              \
                                                                        \
        LIBXML_TEST_VERSION;                                            \
                                                                        \
        ANY_REQUIRE( __xmlString )                                      \
                                                                        \
        __len = strlen( __xmlString );                                  \
        __doc = xmlReadMemory( __xmlString, __len,                      \
                               "__placeholder__.xml", NULL, 0 );        \
        if ( __doc == NULL )                                            \
        {                                                               \
            ANY_LOG( 0, "Malformed XML initialization string for %s",   \
                     ANY_LOG_ERROR, __moduleName);                      \
            return -1;                                                  \
        }                                                               \
        __node = xmlDocGetRootElement( __doc );                         \
                                                                        \
        ANY_REQUIRE( __node );                                          \
        ANY_REQUIRE( __node->name );                                    \
                                                                        \
        if ( !( xmlStrEqual( __node->name, BAD_CAST( __moduleName ) ) ) ) \
        {                                                               \
            xmlFreeDoc( __doc );                                        \
            ANY_LOG( 0, "Bad XML initialization string for %s, root element should be %s", \
                     ANY_LOG_ERROR, __moduleName, __moduleName);        \
            return -1;                                                  \
        }                                                               \
                                                                        \
        for( __attr = __node->properties; __attr != NULL; __attr = __attr->next ) \
        {                                                               \
            char *__parameterValueAsciiStr;                             \
                                                                        \
            __parameterName = __attr->name;                             \
                                                                        \
            __parameterValueUtf8Str = xmlNodeListGetString( __doc, __attr->children, 1 ); \
                                                                        \
                                                                        \
            ANY_REQUIRE( __parameterValueUtf8Str );                     \
                                                                        \
            /* FIXME: Should convert this using iconv */                \
            __parameterValueAsciiStr = (char *)__parameterValueUtf8Str; \
            {


/*!
 *
 * \brief Finishes the parsing section of the initialization string
 * inside the _initFromXML() function.
 *
 * \see BBDM_INITXML_PARSE_BEGIN
 */
#define BBDM_INITXML_PARSE_END()                \
    }                                           \
        }                                       \
        xmlFreeDoc( __doc );                    \
        } while ( 0 )                           \


/*!
 * \brief Handle for the parsed flag of a given parameter.
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBDM_DECL_PARAMETER_PARSED_FLAG
 * \see BBDM_PARAMETER_CHECK_PARSED
 */
#define BBDM_PARAMETER_PARSED( __parameter )    \
    __##__parameter##_parsed

/*!
 * \brief Declares the parsed flag for a given parameter inside the _initFromXML()
 *        function..
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBDM_PARAMETER_PARSED
 */
#define BBDM_DECL_PARAMETER_PARSED_FLAG( __parameter )  \
    bool BBDM_PARAMETER_PARSED( __parameter ) = false

/*!
 * \brief Emits a warning if the parameter has not been initialized at the end of
 *        the _initFromXML() function.
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBDM_PARAMETER_PARSED
 */
#define BBDM_PARAMETER_CHECK_PARSED( __parameter )                      \
    do {                                                                \
        if ( !( BBDM_PARAMETER_PARSED( __parameter ) ) )                \
        {                                                               \
            ANY_LOG( 0, "The parameter %s has not been initialized", ANY_LOG_WARNING, #__parameter ); \
        }                                                               \
    } while ( 0 )


/*!
 *
 * \brief Gets the initial value of parameter(s) of _init().
 *
 * \param __parameter the name of the parameter
 * \param __format the scanf() conversion specifier of the parameter.
 *
 * This macro extracts the value of the parameter from the initialization XML string
 * inside the parsing section of the _initFromXML() function.
 *
 * \see BBDM_INITXML_PARSE_BEGIN
 */
#define BBDM_INITXML_INIT_PARAMETER( __parameter, __format, __ptr)      \
    if ( xmlStrEqual( __parameterName, BAD_CAST( #__parameter ) ) )     \
    {                                                                   \
        Any_sscanf( (char*)__parameterValueAsciiStr, __format, __ptr ); \
        xmlFree( __parameterValueUtf8Str );                             \
        BBDM_PARAMETER_PARSED( __parameter ) = true;                    \
        continue;                                                       \
    }


/*!
 * \brief Logs when an unknown parameter name is encountered when parsing the
 *        xml string in the _initFromXML() function.
 */
#define BBDM_LOG_UNKNOWN_PARAMETER_NAME()                               \
    do {                                                                \
        xmlFree( __parameterValueUtf8Str );                             \
        ANY_LOG( 0, "Unknown XML child name %s", ANY_LOG_WARNING, __parameterName ); \
    } while ( 0 )



/*--------------------------------------------------------------------------*/
/* Data access macros                                                       */
/*--------------------------------------------------------------------------*/


/*!
 * \brief Returns a pointer to the type name of an instance.
 *
 * \param self an instance
 *
 * \see BBDM_GET_TYPENAME_FIELD
 */
#define BBDM_GET_TYPENAME( self ) \
  (\
   (*(char**)(self ))\
  )


/*!
 * \brief Starts the declaration of a BBDM type.
 *
 * \param __bbdmTypeName the type name of the BBDM
 *
 * \see BBDM_DECLARE_STRUCT_END
 */
#define BBDM_DECLARE_STRUCT_BEGIN( __bbdmTypeName ) \
typedef struct __bbdmTypeName \
{\
  const char *typeName;


/*!
 * \brief Finishes the declaration of a BBDM type.
 *
 * \param __bbdmTypeName the type name of the BBDM
 *
 * \see BBDM_DECLARE_STRUCT_BEGIN
 */
#define BBDM_DECLARE_STRUCT_END( __bbdmTypeName ) \
} \
__bbdmTypeName


/*!
 *
 * \brief Exports some component internal data to public access.
 *
 * \param __bbdmTypeName the type name of the BBDM
 *
 *
 * This macro exports data such as the component
 * name.
 *
 * \see BBDM_CREATE_DATA
 */
#define BBDM_EXPORT_DATA( __bbdmTypeName ) \
\
extern const char* ANY_MACRO_CONCAT( __bbdmTypeName, _typeName )


/*!
 * \brief Create some component internal data.
 *
 * \param __bbdmTypeName the type name of the BBDM
 *
 * This macro creates data such as the pointer to the component name.
 *
 *
 * \see BBDM_EXPORT_DATA
 */
#define BBDM_CREATE_DATA( __bbdmTypeName )\
\
const char* ANY_MACRO_CONCAT ( __bbdmTypeName, _typeName ) = ANY_MACRO_STRING( __bbdmTypeName )


/*!
 * \brief Creates a string literal representing the full
 * type name of the component.
 *
 * \param __bbdmTypeName the type name of the BBDM
 *
 * \see BBDM_GET_TYPENAME_FIELD
 */
#define BBDM_GET_TYPENAME_CONST( __bbdmTypeName ) \
\
ANY_MACRO_CONCAT( __bbdmTypeName, _typeName )


/*!
 * \brief Returns a pointer to the type name of an instance.
 *
 * \param __self an instance
 *
 * \see BBDM_GET_TYPENAME_CONST
 */
#define BBDM_GET_TYPENAME_FIELD( __self ) \
\
((char*)(((BBDM*) (__self))->typeName))


/*!
 * \brief Returns a field of an instance.
 *
 * \param __self an instance
 * \param __fieldName a field
 *
 * \see BBDM_GET_TYPENAME_CONST
 * \see BBDM_GET_TYPENAME_FIELD
 */
#define BBDM_GET_FIELD( __self, __fieldName ) \
\
((__self)->__fieldName)


/*!
 *
 * \brief initializes an instance of type TypeName in the _init()
 * function.
 *
 * \param __self an instance
 * \param __bbdmTypeName the type name of the BBDM
 *
 * \see BBDM_CLEAR_STRUCT
 */
#define BBDM_INIT_STRUCT( __self, __bbdmTypeName ) \
do\
{\
  Any_memset( (void*)__self, 0, sizeof(__bbdmTypeName) ); \
  __self->typeName = BBDM_GET_TYPENAME_CONST( __bbdmTypeName ); \
} while( 0 )


/*!
 *
 * \brief Clears an instance of type TypeName in the _clear() function.
 *
 * \param __self an instance
 * \param __bbdmTypeName the type name of the BBDM
 *
 * \see BBDM_INIT_STRUCT
 */
#define BBDM_CLEAR_STRUCT( __self, __bbdmTypeName ) \
do\
{\
  Any_memset( (void*)__self, 0, sizeof(__bbdmTypeName) );\
} while( 0 )


/*!
 *
 * \brief Declares the base class for all BBDMs.
 *
 * \see BBDM_DECLARE_STRUCT_BEGIN
 * \see BBDM_DECLARE_STRUCT_END
 */
BBDM_DECLARE_STRUCT_BEGIN( BBDM )
BBDM_DECLARE_STRUCT_END( BBDM );


/*!
 * \brief Returns a pointer to the tag-field of any given BBDM
 *
 * \param __self a BBDM instance (any BBDM type)
 * \returns pointer to the BBDM's tag-field
 */
#define BBDM_GETTAG( __self ) (__self)->tag


/*!
 * \brief Returns a pointer to the data-field of any given BBDM
 *
 * \param __self a BBDM instance (any BBDM type)
 * \returns pointer to the BBDM's data-field
 */
#define BBDM_GETDATA( __self ) (__self)->data


/*!
 * \brief the default name if no one was passed to Serialize_beginType()
 */
#define BBDM_DEFAULT_BEGINTYPE_NAME ((char*)"data")


#define BBDMTag_copy( __self, __src )                             \
do                                                                \
{                                                                 \
  ANY_REQUIRE( __self );                                          \
  ANY_REQUIRE( __src );                                           \
  __self = __src;                                                 \
}                                                                 \
while ( 0 )


/*--------------------------------------------------------------------------*/
/* Data access functions                                                    */
/*--------------------------------------------------------------------------*/


/*!
 * \brief returns the BBDM type name
 *
 * NOTE: this is just the function version of the BBDM_GET_TYPENAME macro
 *
 * \see BBDM_GET_TYPENAME
 */
const char *BBDM_getTypeName( const void *self );


/*!
 * \brief returns the typename of the inner payload data
 */
const char *BBDM_getDataTypeName( const void *self );


/*!
 * \brief returns the BBDM instance name
 */
const char *BBDM_getInstanceName( const void *self );


/*!
 * \brief set BBDM instance name
 */
void BBDM_setInstanceName( void *self, const char *instanceName );


/*!
 * \brief get BBDM time step
 */
BaseI64 BBDM_getTimestep( const void *self );


/*!
 * \brief set BBDM time step
 */
void BBDM_setTimestep( void *self, BaseI64 timestep );


/*!
 * \brief get the inner data of a BBDM
 */
void *BBDM_getData( void *self );


/*!
 * \brief perform a deep copy of data from 'src' into 'self'
 *
 * This copies the data incl. nested substructures (if any) from 'src'
 * into the self-instance ('self' is the destination / copy target).
 * The BBDMTag (incl. timestep) is not affected by this function.
 *
 * \param self copy destination
 * \param src  copy source
 *
 * \returns in case of success returns zero, status code of the internally
 *          used copy function (e.g. BPLStatus for BBDMBlockF32) otherwise
 */
int BBDM_copyData( void *self, const void *src );


/*!
 * \brief get the payload properties of the inner data of a BBDM
 */
void BBDM_getProperties( const void *self, BBDMProperties *p );


/*!
 * \brief serialize the BBDM. Serialize type omitted for library independence
 */
void BBDM_serialize( void *self, const char *name, void *stream );


/*!
 * \brief returns the pointer of the BBDM's serialization function
 */
SerializeFunction BBDM_getSerializeFunctionPtr( void *self );


/*!
 * \brief returns the pointer of the BBDM's inner data serialization function
 */
SerializeFunction BBDM_getDataSerializeFunctionPtr( void *self );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
