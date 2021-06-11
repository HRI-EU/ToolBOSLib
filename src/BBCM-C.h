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
 * \page BBCM_About BBCM (Code components)
 *
 * BBCM stands for "Brain-Bytes Component Model" and defines a software
 * component model in C/C++ for encapsulating algorithms.
 *
 * With the help of that, different teams can create and ship software
 * components that can be used to assemble an application. As an example,
 * you can consider someone creating an application in Germany using some
 * piece of code created in Japan, without having to understand the way
 * it was implemented but just focussing on its interface.
 *
 * Similar to BBDM.h this headerfile defines macros commonly used in all
 * computing components.
 *
 * \li BBCM-C.h
 * \li BBCM-Cpp.h
 *
 */


#ifndef BBCM_C_H
#define BBCM_C_H


#include <ctype.h>

#include <Any.h>


/*!
 * \brief Struct used for output event information.
 *
 */
typedef struct BBCMEventInfo
{
    void (*function)( void * );

    void *functionParam;
    struct BBCMEventInfo *next;
}
        BBCMEventInfo;


/*!
 * \brief Define the type used to declare a BBCM Status change callback
 *
 * Callback parameters are: instance passed at registration, general component
 * status and size of the component status
 */
typedef void (BBCMStatusChange)( void *, char *, long );

/*!
 * \brief Initializes the information export mechanism.
 *
 * This macro has to be put into the
 * header file.
 *
 * \param componentName name of component
 *
 */
#define BBCM_INFO_EXPORT( componentName ) \
\
extern char *componentName##_info[]


/*!
 * \brief Starts the data component information section in the implementation file.
 *
 * \param componentName name of component
 *
 *
 * Example (being a usual "passive" component)
 * \code
BBCM_INFO_BEGIN( PasswordEncrypter )
BBCM_INFO_COMPUTINGMODE_PASSIVE
BBCM_INFO_DESCRIPTION( "A simple password encrypter" )
BBCM_INFO_INITSTRING_BEGIN
BBCM_INFO_INITFIELD( "key", "int", "%d", "13",
                     "Encryption key", "[0..255]" )
BBCM_INFO_INITSTRING_END
BBCM_INFO_INPUT( "UserPassword", "char*",
                 "User password to encrypt",  "MANDATORY" )
BBCM_INFO_OUTPUT( "Encrypted Password", "char*",
                  "Encrypted user password", "MANDATORY" )
BBCM_INFO_INPUTEVENT( "Encrypt", "Perform encryption of user password" )
BBCM_INFO_OUTPUTEVENT( "BadUserPassword",
                       "User password is bad an will not be encrypted" )
BBCM_INFO_END;
 * \endcode
 *
 * \see BBCM_INFO_END
 */
#define BBCM_INFO_BEGIN( componentName ) \
char *componentName##_info[] = \
{ (char*)"@BBCM-INFO-BEGIN", \
(char*)"@Name: " #componentName, \
(char*)"#BBCM_INFO_BEGIN( " #componentName " )",


/*!
 * \brief Describes the component itself in the implementation file.
 *
 * \param componentCategory the category where the component will be installed
 *
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_CATEGORY( componentCategory ) \
\
(char*)"@Category: " componentCategory, \
(char*)"#BBCM_INFO_CATEGORY( " #componentCategory " )",


/*!
 * \brief Describes the component itself in the implementation file.
 *
 * \param componentDescription a description of the component
 *
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_DESCRIPTION( componentDescription ) \
\
(char*)"@Description: " componentDescription, \
(char*)"#BBCM_INFO_DESCRIPTION( " #componentDescription " )",

/*!
 * \brief Specifies if the component is a system module or not. A system module
 * receives a pointer to RTBOX at runtime.
 *
 * \param Value true if the component is a system module, false otherwise
 *
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_SYSTEM_MODULE( value )      \
                                              \
(char*)"@SystemModule: " value,               \
(char*)"#BBCM_INFO_SYSTEM_MODULE( " #value ")",

/*!
 * \brief Specifies that the default working/execution modality is "active"
 */
#define BBCM_INFO_COMPUTINGMODE_ACTIVE \
\
(char*)"@ComputingMode: active", \
(char*)"#BBCM_INFO_COMPUTINGMODE_ACTIVE",


/*!
 * \brief Specifies that the default working/execution modality is "passive"
 */
#define BBCM_INFO_COMPUTINGMODE_PASSIVE \
\
(char*)"@ComputingMode: passive", \
(char*)"#BBCM_INFO_COMPUTINGMODE_PASSIVE",


/*!
 * \brief Starts the reference information section in the
 * implementation file.
 *
 * \see BBCM_INFO_INITSTRING_END
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_INITSTRING_BEGIN \
\
(char*)"#BBCM_INFO_INITSTRING_BEGIN",


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
 * \param isComplex either true or false.
 *        If false, the BBCM template creates code for this reference.
 *
 * \see BBCM_INFO_INITSTRING_BEGIN
 * \see BBCM_INFO_INITSTRING_END
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_INITFIELD( fieldName, fieldType, fieldPattern, fieldDefaultValue, fieldDescription, fieldRangeDescription, isComplex ) \
(char*)"@InitStringField: " fieldName ", " fieldType ", " fieldPattern \
                ", " fieldDefaultValue ", " fieldDescription \
                ", " fieldRangeDescription, \
(char*)"#BBCM_INFO_INITFIELD( " #fieldName ", " #fieldType ", " #fieldPattern ", " #fieldDefaultValue ", " #fieldDescription ", " #fieldRangeDescription ", " #isComplex " )",


/*!
 * \brief Finishes the reference information section in the
 * implementation file.
 *
 * \see BBCM_INFO_INITSTRING_BEGIN
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_INITSTRING_END \
\
(char*)"#BBCM_INFO_INITSTRING_END",


/*!
 * \brief Starts the system reference information section in the
 * implementation file.
 *
 * \see BBCM_INFO_INITSTRING_BEGIN
 * \see BBCM_INFO_INITSTRING_END
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_SYSTEMSTRING_BEGIN \
\
(char*)"#BBCM_INFO_SYSTEMSTRING_BEGIN",


/*!
 * \brief Describes a system parameter  used to setup a component instance.
 *
 * \param fieldName the name of the parameter
 * \param fieldType the type of the parameter
 * \param fieldPattern the scanf() conversion pattern
 * \param fieldDefaultValue the default value of the parameter
 * \param fieldDescription a description of the parameter
 * \param fieldRangeDescription a description of
 *        the admissible range of values for this parameter
 * \param isComplex either true or false.
 *        If false, the BBCM template creates code for this system reference.
 *
 * \see BBCM_INFO_SYSTEMSTRING_BEGIN
 * \see BBCM_INFO_SYSTEMSTRING_END
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_SYSTEMFIELD( fieldName, fieldType, fieldPattern, fieldDefaultValue, fieldDescription, fieldRangeDescription, isComplex ) \
(char*)"@SystemStringField: " fieldName ", " fieldType ", " fieldPattern \
                ", " fieldDefaultValue ", " fieldDescription \
                ", " fieldRangeDescription, \
(char*)"#BBCM_INFO_SYSTEMFIELD( " #fieldName ", " #fieldType ", " #fieldPattern ", " #fieldDefaultValue ", " #fieldDescription ", " #fieldRangeDescription ", " #isComplex " )",


/*!
 * \brief Finishes the system reference information section in the
 * implementation file.
 *
 * \see BBCM_INFO_INITSTRING_BEGIN
 * \see BBCM_INFO_INITSTRING_END
 * \see BBCM_INFO_SYSTEMSTRING_BEGIN
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_SYSTEMSTRING_END \
\
(char*)"#BBCM_INFO_SYSTEMSTRING_END",


/*!
 * \brief Describes an input.
 *
 * \param inputName the name of the input
 * \param inputType the type of the input
 * \param inputDescription a description of the input
 * \param inputBinding one of OPTIONAL or MANDATORY
 *
 * \see BBCM_INFO_OUTPUT
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_INPUT( inputName, inputType, inputDescription, inputBinding ) \
\
(char*)"@Input: " inputName ", " inputType ", " inputDescription ", " inputBinding, \
(char*)"#BBCM_INFO_INPUT( " #inputName ", " #inputType ", " #inputDescription ", " #inputBinding " )", \
BBCM_INFO_INPUTEVENT( inputName, "Notify a change in the input" )


/*!
 * \brief Describes an output.
 *
 * \param outputName the name of the output
 * \param outputType the type of the output
 * \param outputDescription a description of the output
 * \param outputBinding one of OPTIONAL or MANDATORY
 *
 * \see BBCM_INFO_INPUT
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_OUTPUT( outputName, outputType, outputDescription, outputBinding ) \
\
(char*)"@Output: " outputName ", " outputType ", " outputDescription ", " outputBinding, \
(char*)"#BBCM_INFO_OUTPUT( " #outputName ", " #outputType ", " #outputDescription ", " #outputBinding " )", \
BBCM_INFO_OUTPUTEVENT( outputName, "Notify a change in the output" )


/*!
 * \brief Describes an input event.
 *
 * \param inputEventName the name of the input event
 * \param inputEventDescription a description of the input event
 *
 * \see BBCM_INFO_OUTPUTEVENT
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_INPUTEVENT( inputEventName, inputEventDescription ) \
\
(char*)"@InputEvent: " inputEventName ", " inputEventDescription, \
(char*)"#BBCM_INFO_INPUTEVENT( " #inputEventName ", " #inputEventDescription " )",


/*!
 * \brief Describes an output event.
 *
 * \param outputEventName the name of the output event
 * \param outputEventDescription a description of the output event
 *
 * \see BBCM_INFO_INPUTEVENT
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_OUTPUTEVENT( outputEventName, outputEventDescription ) \
\
(char*)"@OutputEvent: " outputEventName ", " outputEventDescription, \
(char*)"#BBCM_INFO_OUTPUTEVENT( " #outputEventName ", " #outputEventDescription " )",


/*!
 * \brief Sets a default value for a reference.
 * This is interpreted by DTBOS as a pre-defined ReferenceSetting.
 *
 * \param referenceName the name of the reference
 * \param referenceValue the pre-defined value
 */
#define BBCM_DEFAULT_REFERENCESETTING( referenceName, referenceValue ) \
\
(char*)"#BBCM_DEFAULT_REFERENCESETTING( " #referenceName ", " #referenceValue " )",


/*!
 * \brief Sets a default value for a system reference.
 * This is interpreted by DTBOS as a pre-defined System Reference Setting.
 *
 * \param referenceName the name of the reference
 * \param referenceValue the pre-defined value
 */
#define BBCM_DEFAULT_SYSTEMREFERENCESETTING( referenceName, referenceValue ) \
\
(char*)"#BBCM_DEFAULT_SYSTEMREFERENCESETTING( " #referenceName ", " #referenceValue " )",


/*!
 * \brief Finishes the component information section in the header file.
 *
 * \see BBCM_INFO_BEGIN
 */
#define BBCM_INFO_END \
\
(char*)"@BBCM-INFO-END",(char*)"#BBCM_INFO_END", (char*)NULL }


/*!
 * \brief Starts the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * \param initString the initialization string
 *
 * This macro is always followed by the macro BBCM_INITFIELD_BEGIN.
 *
 * Example:
 * \code
    int PasswordEncrypter_initFromString( PasswordEncrypter *self,
                                          char *initString )
    {
        int key = PASSWORDENCRYPTER_DEFAULT_KEY;

        BBCM_INITSTRING_PARSE_BEGIN( initString )
        {
            BBCM_INITFIELD_BEGIN
            BBCM_INITFIELD_GET( key, "%d", &key )
            {
            }
            BBCM_INITFIELD_END;
        }
        BBCM_INITSTRING_PARSE_END;

        return PasswordEncrypter_init( self, key);
    }
 \endcode
 *
 *
 * \see BBCM_INITSTRING_PARSE_END
 * \see BBCM_INITFIELD_BEGIN
 */
#define BBCM_INITSTRING_PARSE_BEGIN( initString ) \
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
 * This macro is always preceded by the macro BBCM_INITFIELD_END.
 *
 * \see BBCM_INITSTRING_PARSE_BEGIN
 * \see BBCM_INITFIELD_END
 */
#define BBCM_INITSTRING_PARSE_END \
      }\
\
      if ( ( __tokenLen == 0 ) || ( __equalFound == true ) )\
      {\
        if ( __ch == '"' || __ch == '\'' )\
        {\
          char __strDelimiter = __ch;\
          char __formerCh = __ch;\
          ++__currPos;\
          __ch = __initString[ __currPos ];\
\
          while ( ( __currPos < __initStringLen ) && ( __ch != __strDelimiter ) && ( __formerCh != '\\' ) )\
          {\
            ++__currPos;\
            if( __formerCh == '\\' && __ch == '\\' )\
            {\
              __formerCh = ' ';\
            }\
            else\
            {\
              __formerCh = __ch;\
            }\
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
while (0)


/*!
 * \brief Starts the parsing section of the initialization string
 * inside the _initFromString() function.
 *
 * This macro is always preceded by the
 * macro BBCM_INITSTRING_PARSE_BEGIN.
 *
 * \see BBCM_INITSTRING_PARSE_BEGIN
 * \see BBCM_INITFIELD_END
 */
#define BBCM_INITFIELD_BEGIN \
 if ( 0 )\
 {\
   {

#if defined(__GNUC__) && ( __GNUC__ < 3 ) && defined(__cplusplus)

#define BBCM_INITFIELD_GET( fieldName, fieldPattern, VaArgs... ) \
   }\
 }\
 else if ( ( Any_strlen( #fieldName ) == __tokenLen ) && ( Any_strncmp( #fieldName, &__initString[__tokenPos], __tokenLen ) == 0 ) )\
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
 * \see BBCM_INITSTRING_PARSE_BEGIN
 */
#define BBCM_INITFIELD_GET( fieldName, fieldPattern, ... ) \
   }\
 }\
 else if ( ( Any_strlen( #fieldName ) == __tokenLen ) && ( Any_strncmp( #fieldName, &__initString[__tokenPos], __tokenLen ) == 0 ) ) \
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
 * macro BBCM_INITSTRING_PARSE_END.
 *
 * \see BBCM_INITSTRING_PARSE_END
 * \see BBCM_INITFIELD_BEGIN
 * \see BBCM_INITSTRING_PARSE_BEGIN
 */
#define BBCM_INITFIELD_END \
   }\
 }



/*!
 * \brief Starts the parsing section of the initialization XML string
 * inside the _initFromXML() function.
 *
 * \param __moduleName the name of the module
 * \param __xmlString the initialization string
 *
 * Example:
 * \code
 int PasswordEncrypter_initFromXML( PasswordEncrypter *self,
 char *xmlString )
 {
 int key = PASSWORDENCRYPTER_DEFAULT_KEY;

 BBCM_INITXML_PARSE_BEGIN( "PasswordEncrypter", initString )
 {
    BBCM_INITXML_INIT_PARAMETER( key, "%d", &key );
 }
 BBCM_INITXML_PARSE_END();

 return PasswordEncrypter_init( self, key);
 }
 \endcode
 *
 *
 * \see BBCM_INITXML_PARSE_END
 */
#define BBCM_INITXML_PARSE_BEGIN( __moduleName, __xmlString )           \
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
 * \see BBCM_INITXML_PARSE_BEGIN
 */
#define BBCM_INITXML_PARSE_END()                \
            }                                   \
        }                                       \
        xmlFreeDoc( __doc );                    \
    } while ( 0 )                               \


/*!
 * \brief Handle for the parsed flag of a given parameter.
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBCM_DECL_PARAMETER_PARSED_FLAG
 * \see BBCM_PARAMETER_CHECK_PARSED
 */
#define BBCM_PARAMETER_PARSED( __parameter )            \
    __##__parameter##_parsed


/*!
 * \brief Declares the parsed flag for a given parameter inside the _initFromXML()
 *        function..
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBCM_PARAMETER_PARSED
 */
#define BBCM_DECL_PARAMETER_PARSED_FLAG( __parameter )  \
    bool BBCM_PARAMETER_PARSED( __parameter ) = false


/*!
 * \brief Emits a warning if the parameter has not been initialized at the end of
 *        the _initFromXML() function.
 *
 * \param __parameter the parameter symbol.
 *
 * \see BBCM_PARAMETER_PARSED
 */
#define BBCM_PARAMETER_CHECK_PARSED( __parameter )                      \
    do {                                                                \
        if ( !( BBCM_PARAMETER_PARSED( __parameter ) ) )                \
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
 * \see BBCM_INITXML_PARSE_BEGIN
 */
#define BBCM_INITXML_INIT_PARAMETER( __parameter, __format, ... )      \
    if ( xmlStrEqual( __parameterName, BAD_CAST( #__parameter ) ) )     \
    {                                                                   \
        ANY_LOG( 7, "parsing reference '%s': %s", ANY_LOG_INFO, __parameterName, __parameterValueAsciiStr); \
        Any_sscanf( (char*)__parameterValueAsciiStr, __format, ##__VA_ARGS__ ); \
        xmlFree( __parameterValueUtf8Str );                             \
        BBCM_PARAMETER_PARSED( __parameter ) = true;                    \
        continue;                                                       \
    }


/*!
 * \brief Logs when an unknown parameter name is encountered when parsing the
 *        xml string in the _initFromXML() function.
 */
#define BBCM_LOG_UNKNOWN_PARAMETER_NAME()                               \
    do {                                                                \
        xmlFree( __parameterValueUtf8Str );                             \
        if ( !xmlStrcmp( __parameterName, BAD_CAST( "tagInfoDisabled" ) ) || \
             !xmlStrcmp( __parameterName, BAD_CAST( "tagInfoSourceInput" ) ) ) \
            continue;                                                   \
                                                                        \
        ANY_LOG( 0, "Unknown XML child name %s", ANY_LOG_WARNING, __parameterName ); \
    } while ( 0 )


/*!
 * \brief Registers a handler function for an output event.
 *
 * \param self an instance
 * \param eventName the name of the event to be registered
 * \param eventVarFunction the function to handle the event
 * \param eventVarFunctionParam a parameter for the handler function
 *
 * The memory must be released with the BBCM_RELEASE_OUTEVENT macro.
 *
 * \see BBCM_FIRE_OUTEVENT
 */
#define BBCM_REGISTER_OUTEVENT( self, eventName, eventVarFunction, eventVarFunctionParam ) \
do\
{\
  if ( self->eventName##Info.function == (void (*)(void*))NULL )\
  {\
    self->eventName##Info.function = eventVarFunction;\
    self->eventName##Info.functionParam = eventVarFunctionParam;\
    self->eventName##Info.next = (BBCMEventInfo*)NULL;\
  }\
  else\
  {\
    BBCMEventInfo *info = &(self->eventName##Info);\
\
    while ( info->next != (BBCMEventInfo*)NULL )\
    {\
      info = info->next;\
    }\
\
    info->next = ANY_TALLOC( BBCMEventInfo );\
    ANY_REQUIRE( info->next );\
    info->next->function = eventVarFunction;\
    info->next->functionParam = eventVarFunctionParam;\
    info->next->next = (BBCMEventInfo*)NULL;\
  }\
}\
while (0)


/*!
 * \brief Fires an output event.
 *
 * \param self an instance
 * \param eventName the name of the event to be fired
 *
 *
 * Example:
 * \code
 * // Fire the output event "Value"
 * BBCM_FIRE_OUTEVENT( self, value );
 * // Fire the output event "ZeroDetected"
 * BBCM_FIRE_OUTEVENT( self, zeroDetected );
 * \endcode
 *
 * \see BBCM_REGISTER_OUTEVENT
 */
#define BBCM_FIRE_OUTEVENT( self, eventName ) \
do\
{\
  BBCMEventInfo *info = &(self->eventName##Info);\
\
  while ( info != (BBCMEventInfo*)NULL )\
  {\
    if ( info->function != (void (*)(void*))NULL )\
      (*(info->function))( info->functionParam );\
    info = info->next;\
  }\
}\
while (0)


/*!
 * \brief Fires an output event according to the eventInfoPtr pointer.
 *
 * \param self an instance
 * \param eventInfoPtr the eventInfoPtr pointer
 *
 * Example:
 * \code
 * // Fire the output event "Value"
 * BBCM_FIRE_OUTEVENT_PTR( self, &(self->valueInfo) );
 * // Fire the output event "ZeroDetected"
 * BBCM_FIRE_OUTEVENT_PTR( self, &(self->zeroDetectedInfo) );
 * \endcode
 *
 * \see BBCM_REGISTER_OUTEVENT
 */
#define BBCM_FIRE_OUTEVENT_PTR( self, eventInfoPtr ) \
do\
{\
  BBCMEventInfo *info = eventInfoPtr;\
\
  while ( info != (BBCMEventInfo*)NULL )\
  {\
    if ( info->function != (void (*)(void*))NULL )\
      (*(info->function))( info->functionParam );\
    info = info->next;\
  }\
}\
while (0)


/*!
 * \brief Releases a registered output event.
 *
 * This macro releases all memory allocated to manage the event handling
 * functions for this output event.
 *
 * \param self an instance
 * \param eventName the name of the event
 *
 */
#define BBCM_RELEASE_OUTEVENT( self, eventName ) \
do\
{\
  BBCMEventInfo *ptrNext;\
  BBCMEventInfo *info = self->eventName##Info.next;\
\
  while ( info != (BBCMEventInfo*)NULL )\
  {\
    ptrNext = info->next;\
    ANY_FREE( info );\
    info = ptrNext;\
  }\
}\
while (0)


/*!
 * \brief Registers a handler function for the fatal error event.
 *
 * \param self an instance
 * \param eventVarFunction the function to handle the event
 * \param eventVarFunctionParam a parameter for the handler function
 *
 * \see BBCM_FIRE_FATALERROR
 */
#define BBCM_REGISTER_FATALERROR( self, eventVarFunction, eventVarFunctionParam ) \
do\
{\
  self->fatalErrorInfo.function = eventVarFunction;\
  self->fatalErrorInfo.functionParam = eventVarFunctionParam;\
  self->fatalErrorInfo.next = (BBCMEventInfo*)NULL;\
}\
while (0)


/*!
 * \brief Fires the fatal error event.
 *
 * \param self an instance
 *
 * \see BBCM_REGISTER_FATALERROR
 */
#define BBCM_FIRE_FATALERROR( self ) \
do\
{\
  if ( ( self ) && ( self->fatalErrorInfo.function != (void (*)(void*))NULL ) )\
    (*(self->fatalErrorInfo.function))( self->fatalErrorInfo.functionParam );\
  else\
  {\
    ANY_LOG( 0, "fatal error occured", ANY_LOG_FATAL );\
    exit( EXIT_FAILURE );\
  }\
}\
while (0)


/*!
 * \brief Clears an instance of type ComponentType
 * in the _clear() function.
 *
 * \param ComponentType the type name of the BBCM

 */
#define BBCM_CLEAR_STRUCT( ComponentType ) \
\
  Any_memset( (void*)self, 0, sizeof( ComponentType ) );


/*!
 * \brief maximum length of a BBCM instance name
 */
#define BBCM_MAXINSTANCENAMELEN ( 128 )


/*!
 * \brief Error message type
 */
#define BBCM_LOG_ERROR      ANY_LOG_ERROR


/*!
 * \brief (obsolete) Fatal error message type for ANY_LOG
 */
#define BBCM_LOG_FATAL      ANY_LOG_FATAL

/*!
 * \brief Warning message type for ANY_LOG
 */
#define BBCM_LOG_WARNING    ANY_LOG_WARNING


/*!
 * \brief Data message type for ANY_LOG
 */
#define BBCM_LOG_DATA       ANY_LOG_DATA


/*!
 * \brief Data message type for ANY_LOG
 */
#define BBCM_LOG_DATA_CHECK ANY_LOG_DATA_CHECK


/*!
 * \brief Info message type for ANY_LOG
 */
#define BBCM_LOG_INFO       ANY_LOG_INFO


#if defined(__GNUC__) && ( __GNUC__ < 3 ) && defined(__cplusplus)

/*!
 * \brief wrapper for ANY_LOG that also prints the BBCM's instance name
 *
 * If you do not provide variadic arguments, the compiler may issue a
 * warning such as "ISO C99 requires rest arguments to be used".
 * Please use \ref BBCM_MSG in such cases.
 *
 * \param __self the BBCM instance pointer
 * \param __debugLevel the higher the value, the less important is the message
 *                     (0 == print always)
 * \param __format format string for the message, same specifiers like printf()
 * \param __msgType one of ANY_LOG_INFO, ANY_LOG_WARNING etc.
 * \param __varArgs values for the __format placeholder
 */
#define BBCM_LOG( __self, __debugLevel, __format, __msgType, __varArgs... )      \
{                                                                                \
  ANY_REQUIRE( (__self)->instanceName );                                         \
  ANY_LOG( (__debugLevel), "%s: " __format, __msgType,                           \
           (__self)->instanceName, ##__varArgs );                                \
}


/*!
 * \brief wrapper for ANY_LOG that also prints the BBCM's instance name
 *
 * In contrast to BBCM_LOG, this macro does not take variadic arguments.
 *
 *
 * \param __self the BBCM instance pointer
 * \param __debugLevel the higher the value, the less important is the message
 *                     (0 == print always)
 * \param __format format string for the message, same specifiers like printf()
 * \param __msgType one of ANY_LOG_INFO, ANY_LOG_WARNING etc.
 */
#define BBCM_MSG( __self, __debugLevel, __format, __msgType )                    \
{                                                                                \
  ANY_REQUIRE( (__self)->instanceName );                                         \
  ANY_LOG( (__debugLevel), "%s: " __format, __msgType, (__self)->instanceName ); \
}

#else

/*!
 * \brief wrapper for ANY_LOG that also prints the BBCM's instance name
 *
 * If you do not provide variadic arguments, the compiler may issue a
 * warning such as "ISO C99 requires rest arguments to be used".
 * Please use \ref BBCM_MSG in such cases.
 *
 * \param __self the BBCM instance pointer
 * \param __debugLevel the higher the value, the less important is the message
 *                     (0 == print always)
 * \param __format format string for the message, same specifiers like printf()
 * \param __msgType one of ANY_LOG_INFO, ANY_LOG_WARNING etc.
 * \param ... values for the __format placeholder
 */
#define BBCM_LOG( __self, __debugLevel, __format, __msgType, ... )               \
{                                                                                \
  if( ( (__self)->instanceName == NULL ) ||                                      \
      ( Any_strcmp( ( (__self)->instanceName ), "" ) == 0 ) )                    \
  {                                                                              \
    ANY_LOG( __debugLevel, __format, __msgType, ##__VA_ARGS__ );                 \
  }                                                                              \
  else                                                                           \
  {                                                                              \
    ANY_LOG( (__debugLevel), "%s: " __format, __msgType, (__self)->instanceName, \
             ##__VA_ARGS__ );                                                    \
  }                                                                              \
}


/*!
 * \brief wrapper for ANY_LOG that also prints the BBCM's instance name
 *
 * In contrast to BBCM_LOG, this macro does not take variadic arguments.
 *
 * \param __self the BBCM instance pointer
 * \param __debugLevel the higher the value, the less important is the message
 *                     (0 == print always)
 * \param __format format string for the message, same specifiers like printf()
 * \param __msgType one of ANY_LOG_INFO, ANY_LOG_WARNING etc.
 */
#define BBCM_MSG( __self, __debugLevel, __format, __msgType )                    \
{                                                                                \
  if( ( (__self)->instanceName == NULL ) ||                                      \
      ( Any_strcmp( ( (__self)->instanceName ), "" ) == 0 ) )                    \
  {                                                                              \
    ANY_LOG( __debugLevel, __format, __msgType );                                \
  }                                                                              \
  else                                                                           \
  {                                                                              \
    ANY_LOG( (__debugLevel), "%s: " __format, __msgType,                         \
             (__self)->instanceName );                                           \
  }                                                                              \
}


#define SYSREF_DEFAULT( refName, refValue )

#endif

#endif


/* EOF */
