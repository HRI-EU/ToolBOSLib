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
 * \page AnyTracing_About Binary logging
 *
 * AnyTracing.h provides a set of functionalities to send and receive
 * generic messages in binary form. They are consisting of a header and an
 * associated payload.
 *
 * This library requires the user to write own tracing functions:
 *
 * \code
 void RTBOSLogMsg_trace( RTBOSLogMsg *eventToSend, AnyTracing *self, AnyTracingRefId event, AnyTracingRefId subEvent, AnyTracingRefId argument )
 {
   ANY_REQUIRE( self );
   ANY_REQUIRE( eventToSend );
   ANY_REQUIRE( event > 0 );

   eventToSend->head.sizeOfStruct = sizeof( RTBOSLogMsg );

   eventToSend->eventId = event;
   eventToSend->additionalParam = argument;

   if( AnyTracing_write( self, eventToSend, sizeof( RTBOSLogMsg ) ) == false )
   {
     ANY_LOG( 0, "An error occurred while writing to stream.", ANY_LOG_ERROR );
   }
 }
 * \endcode
 *
 * This is an implementation of a tracing function for a RTBOSLogMsg
 * data type, with a payload consisting of two AnyTracingRefId
 * variables. An implementation of such a data structure might be the
 * following:
 *
 * \code
 typedef struct RTBOSLogMsg
 {
  AnyTracingHeader head;
  AnyTracingRefId eventId;
  AnyTracingRefId additionalParam;
 } RTBOSLogMsg;
 * \endcode
 *
 * A data structure to be used with the AnyTracing library has some
 * requirements to satisfy: the first element must be the
 * AnyTracingHeader, and the payload can be arbitrarily long, as long
 * as it comes after the header.  When writing the tracing function,
 * the generic prototype is the following:
 *
 * \code
 void [datatype]_trace ( [datatype] *dataToSend, AnyTracing *self, ... );
 * \endcode
 *
 * where [datatype] is the user provided data structure, and the
 * variadic arguments at the end can be arbitrarily many,
 * corresponding to the payload of the tracing message.
 *
 * The function must check for the validy of the passed
 * arguments. Successively, it must set the variables: the field
 * sizeOfStruct of the AnyTracingHeader contained in *dataToSend->head
 * must be updated to reflect the size of the whole message,
 * comprising of header and associated payload; the fields containing
 * the payload must then be adjusted. The last step is a call to the
 * AnyTracing_write() function provided by the library, to write the
 * data structure to the stream pointed to by the AnyTracing instance.
 */


#ifndef ANYTRACING_H
#define ANYTRACING_H

#include <Any.h>
#include <Base.h>
#include <IOChannel.h>
#include <LogType.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*---------------------------------------------*/
/* Defines                                     */
/*---------------------------------------------*/

typedef enum
{
    NAME_TYPE_FILE,
    NAME_TYPE_MODULE,    // RTBOS module ID
    NAME_TYPE_PORT,      // for BBCM / BBDM ports
    NAME_TYPE_OPTIONAL,  // for BBCM / BBDM ports
    NAME_TYPE_Count
} NameType;

/*!
 * \brief Header validation flag
 */
#define ANYTRACING_HEADER_VALID      0x2202bd11
/*!
 * \brief Header invalidation flag
 */
#define ANYTRACING_HEADER_INVALID    0x6236d5bf

/*!
 * \brief Type for IDs.
 *
 */
typedef BaseUI64 AnyTracingRefId;

/*!
 * \brief Type for timestamps.
 *
 */
typedef BaseUI64 AnyTracingTimestamp;

/*!
 * \brief Type for message registration.
 *
 */
typedef BaseUI64 AnyTracingMsgCategory;

/*!
 * \brief Macro used to get current thread id.
 *
 * This macro must be redefined by the user to provide his own method to get the current thread id. If this is not done, the field threadId in the AnyTracingHeader will be set to 0.
 */
#ifndef ANYTRACING_TID
#define ANYTRACING_TID 0
#endif

/*!
 * \brief Default host id
 */
#ifndef ANYTRACING_DEFAULTHOSTID
#define ANYTRACING_DEFAULTHOSTID 0
#endif

/*!
 * \brief Registration message category
 */
#define ANYTRACING_REGISTRATION 1
#define ANYTRACING_TRACEMSG     2
#define ANYTRACING_LOGASCIIMSG  6
/*!
 * \brief Category for user defined messages
 */
#define ANYTRACING_USERDEFINED 200


/*!
 * \brief Header structure
 *
 */
typedef struct AnyTracingHeader
{
    unsigned long valid;
    /*!< Validation tag. */
    unsigned int sizeOfStruct;
    /*!< Size of the entire
                                 * structure. Calculated adding
                                 * to the size of the header the
                                 * size of the payload.*/
    AnyTracingTimestamp timestamp;
    /*!< Timestamp of the message. */
    AnyTracingMsgCategory msgCategory;
    /*!< Category of the message */
    unsigned int logLevel;
    /*!< Log level of the message. */
    AnyTracingRefId hostId;
    /*!< Id of the sending host */
    unsigned int pid;
    /*!< PID of the sending process. */
    unsigned int threadId;
    /*!< TID of the sending thread. */
    AnyTracingRefId moduleId;
    /*!< ID of the sending module. */
    AnyTracingRefId fileNameId;
    /*!< ID of the filename where the tracing was asked. */
    unsigned int codeLine;             /*!< Codeline where the tracing was asked. */
} AnyTracingHeader;

/*!
 * \brief Main structure
 *
 */
typedef struct AnyTracing
{
    unsigned long valid;
    /*!< Validation tag. */
    IOChannel *stream;
    /*!< IOChannel instance */
    AnyTracingRefId defaultHostId;     /*!< Default host id reference */
} AnyTracing;

/*!
 * \brief String registration structure.
 *
 */

typedef struct AnyTracingMsgRegistration
{
    AnyTracingHeader head;
    /*!< Header containing all the metadata. */
    BaseUI16 nameType;
    /*!< Type of Msg to register */
    AnyTracingRefId msgId;
    /*!< ID of the string to register. */
    unsigned int msgLen;
    /*!< Length of string to register. */
    union
    {
        char *msg;
        char msgVect[sizeof( char * )]; /*!< String to register. */
    } m;
} AnyTracingMsgRegistration;

#define ANY_TRACING_MAX_TEXT_LENGTH  128
typedef struct AnyTracingMsg
{
    AnyTracingHeader head;
    /*!< Header containing all the metadata. */
    unsigned short logType;
    /*!< Info, Error, Wrning ... */
    unsigned int msgLen;
    /*!< Length of string to register. */
    char msg[ANY_TRACING_MAX_TEXT_LENGTH];
} AnyTracingMsg;

#if defined(__GNUC__)
#define ANYTRACING_UNUSED( __var ) __var __attribute__((unused))
#endif

#if defined(__MSVC__)
#define ANYTRACING_UNUSED(__var) __var
#endif

/*!
 * \brief Declare one static filenameId in the including file, this variable is singleton
 *        holding the give filenameId
 */
static AnyTracingRefId ANYTRACING_UNUSED( __AnyTracing_filenameId );

/*!
 * \brief Main tracing macro
 *
 * This macro is the interface to use to send a tracing message. The
 * parameters passed will be utilized to automatically detect the
 * correct low level tracing function to use. For this reason, the
 * argument __type must correspond to the type of the message to
 * send. Look at this example
 *
 * \code
 *
 *   ANYTRACING_TRACE( tracer, RTBOSLogMsg, 0, hostId, moduleId, ANYTRACING_CADAPTOREVENT, eventId, (AnyTracingRefId)NULL );
 *
 * \endcode
 *
 * This macro will look for a RTBOSLogMsg_trace function, since __type
 * was 'RTBOSLogMsg', and it will build a tracing message containg the
 * rest of the data passed as argument. The macro accepts a variable
 * number of arguments after those that are required, and will pass
 * them verbatim to the tracing function. It is pressing to note that
 * while the macro will not, by itself, do any kind of sanity check on
 * the parameters, the underlying function will, so it is important to
 * have consistency between tracing function arguments and how those
 * are passed to the macro. The order is also meaningful.
 *
 * \param __self AnyTracing instance.
 * \param __type Type of event.
 * \param __logLevel Log level.
 * \param __hostId ID of the local host.
 * \param __componentId Module Id.
 * \param __eventCategory Category of the event being traced.
 * \param ... Parameters to pass topic the tracing function. This is the payload of the tracing message, it will be passed to the low level function as-is.
 *
 */
#define ANYTRACING_TRACE( __self, __type, __logLevel, __hostId, __componentId, __eventCategory, ... )                   \
do                                                                   \
{                                                                   \
  __type __typePtr;                                                           \
  AnyTracingTimestamp __timestamp = Any_getTime();                                           \
                                                                   \
  if( ANY_UNLIKELY( __AnyTracing_filenameId == 0 ) )                                           \
  {                                                                   \
    AnyTracingMsgRegistration __msgToSend;                                               \
    __AnyTracing_filenameId = AnyTracing_computeId( (void*)__BASENAME_FILE__ );                               \
    AnyTracing_buildHeader( __self,                                                   \
                            &__msgToSend.head,                                               \
                            __timestamp,                                               \
                            ANYTRACING_REGISTRATION,                                           \
                            0,                                                       \
                            __hostId,                                                   \
                            ANY_LOG_GETPID,                                               \
                            ANYTRACING_TID,                                               \
                            0,                                                       \
                            __AnyTracing_filenameId,                                           \
                            __LINE__);                                                   \
    AnyTracingMsgRegistration_trace( &__msgToSend, __self, NAME_TYPE_FILE, __AnyTracing_filenameId, (char*)__BASENAME_FILE__ );\
  }                                                                   \
                                                                   \
  AnyTracing_buildHeader( __self,                                                   \
                          &__typePtr.head,                                               \
                          __timestamp,                                                   \
                          __eventCategory,                                               \
                          __logLevel,                                                   \
                          __hostId,                                                   \
                          ANY_LOG_GETPID,                                               \
                          ANYTRACING_TID,                                               \
                          __componentId,                                               \
                          __AnyTracing_filenameId,                                           \
                          __LINE__);                                                   \
                                                                   \
  __type##_trace( &__typePtr,                                                       \
                  __self,                                                       \
                  __VA_ARGS__ );                                                   \
} while( 0 )

/*!
 * \brief Register macro
 *
 * This macro is the main interface to use when registering the
 * association between a string and its id.
 *
 * \param __self AnyTracing instance.
 * \param __nameType Type of Msg to register
 * \param __id ID of the string.
 * \param __str String to register.
 * \param __hostId AnyTracingRefId of the host system.
 *
 */
#define ANYTRACING_REGISTER( __self, __nameType, __id, __str, __hostId )            \
do                                                                      \
{                                                                       \
  AnyTracingMsgRegistration __msgToSend;                                \
                                                                        \
  AnyTracing_buildHeader( __self,                                       \
                          &__msgToSend.head,                            \
                          Any_getTime(),                                \
                          ANYTRACING_REGISTRATION,                      \
                          0,                                            \
                          __hostId,                                     \
                          ANY_LOG_GETPID,                               \
                          ANYTRACING_TID,                               \
                          0,                                            \
                          0,                                            \
                          0);                                           \
                                                                        \
  AnyTracingMsgRegistration_trace( &__msgToSend,                        \
                                   __self,                              \
                                   __nameType,                          \
                                   __id,                                \
                                   __str );                             \
} while( 0 )                                                            \

/*!
 * \brief Logging macro
 *
 * This macro prints the contents of a message (represented as
 * returned by the AnyTracing_readMsg()) using internally an user
 * defined function.
 *
 * \param __msg
 * \param __type
 *
 */
#define ANYTRACING_LOG( __msg, __type )         \
do                                              \
{                                               \
  __type##_log( (__type*)__msg );               \
} while( 0 )

/*---------------------------------------------*/
/* UI macros                                   */
/*---------------------------------------------*/
/*!
 * \brief Begin a block for the deconding of a message
 *
 * After reading a message with the function AnyTracing_readMsg(),
 * pass it to this macro to begin the decoding.
 *
 * \param __header Pointer to a header as returned by AnyTracing_readMsg()
 *
 * \see AnyTracing_readMsg()
 */
#define ANYTRACING_DECODEMSG_BEGIN( __header )      \
do                                                  \
{                                                   \
  void *__ptr = header;                             \
  switch( __header->msgCategory )                   \
  {

/*!
 * \brief Start decoding a message.
 *
 * This macro helps in the decodification of the message read. There
 * should be one block of ANYTRACING_DECODEMSG_BEGIN and
 * ANYTRACING_DECODEMSG_END for every expected type of message. The
 * handling of the payload of the message can be done between the
 * macro calls.
 *
 * \param __id Message category id.
 * \param __type Type relative to the message category.
 * \param __data Variable to access the message with.
 *
 * \see AnyTracing_readMsg() for an example of usage.
 */
#define ANYTRACING_DECODEMSG_DETECT_BEGIN( __id, __type, __data )   \
case __id:                                                          \
{                                                                   \
  __data = (__type*)__ptr;

/*!
 * \brief End the decoding of a message.
 *
 */
#define ANYTRACING_DECODEMSG_DETECT_END()         \
break;                                             \
}

/*!
 * \brief End the decoding block.
 *
 */
#define ANYTRACING_DECODEMSG_END()                                     \
default:                                                                \
  {                                                                     \
    ANY_LOG( 0, "Could not detect message category. This probably means we received corrupted data from the stream.", ANY_LOG_ERROR );                                     \
    break;                                                              \
  }                                                                     \
} /* Close switch */                                                    \
} while( 0 );

/*---------------------------------------------*/
/* Public prototypes                           */
/*---------------------------------------------*/
/*!
 * \brief Allocate a new AnyTracing instance.
 *
 * This method is used to allocate the memory for a new AnyTracing
 * object. For every object created with this function
 * AnyTracing_delete() must be called to free the memory again.
 * Before you can use the object it must be initialized by calling
 * AnyTracing_init() and possibly AnyTracing_connect().
 *
 * \return Pointer to newly allocated AnyTracing.
 */
AnyTracing *AnyTracing_new( void );

/*!
 * \brief Initialize a new AnyTracing instance.
 *
 * This function is used to start the initialization of a newly
 * created AnyTracing object. In order to finish the initialization
 * phase AnyTracing_connect() might need to be called.  When the
 * object is not needed anymore AnyTracing_clear() must be called
 * before the object can be freed via AnyTracing_delete():
 * AnyTracing_clear() can handle the disconnection from the stream
 * internally.
 *
 * \param self AnyTracing instance.
 *
 * \return True on success, false otherwise.
 */
bool AnyTracing_init( AnyTracing *self );


/*!
 * \brief Store the default hostname reference ID in the AnyTracing instance.
 *
 * This function is used to save in the AnyTracing instance a
 * previously calculated AnyTracingRefId representing the hostname of
 * the machine running the module.
 *
 * \param self AnyTracing instance.
 * \param id AnyTracinfRefId of the hostname.
 *
 * \see AnyTracing_getDefaultHostId()
 * \see AnyTracing_computeId()
 */
void AnyTracing_setDefaultHostId( AnyTracing *self, AnyTracingRefId id );


IOChannel *AnyTracing_getStream( AnyTracing *self );

/*!
 * \brief Get the hostname reference ID from the AnyTracing instance.
 *
 * This function is used to get the stored default hostname AnyTracingRefId from the AnyTracing instance.
 *
 * \param self AnyTracing instance.
 *
 * \return AnyTracingRefId representing the default hostname.
 *
 * \see AnyTracing_setDefaultHostId()
 */
AnyTracingRefId AnyTracing_getDefaultHostId( AnyTracing *self );


/*!
 * \brief Connect an AnyTracing instance with a given stream.
 *
 * This function allows to open the stream specified by the character
 * string dest. This will be the channel used by the library to send
 * and read messages.
 *
 * \param self AnyTracing instance.
 * \param dest String, conforming to the IOChannel openString format, describing the stream to connect to.
 *
 * \return True on success, false otherwise.
 */
bool AnyTracing_connect( AnyTracing *self, char *dest );

/*!
 * \brief Write data to the stream.
 *
 * Generic write function, that sends a generic buffer to the
 * stream. This should not be used directly by the user, who should
 * use the macros ANYTRACING_TRACE and ANYTRACING_REGISTER instead.
 *
 * \param self AnyTracing instance.
 * \param buf Pointer to the area of memory to write to stream.
 * \param size Number of bytes to write.
 *
 * \see ANYTRACING_TRACE
 * \see ANYTRACING_REGISTER
 *
 * \return True on success, False otherwise.
 *
 */
bool AnyTracing_write( AnyTracing *self, void *buf, long size );

/*!
 * \brief Read data from the stream.
 *
 * Generic read function, that writes into the passed buf pointer the
 * data read from the stream.
 *
 * \param self AnyTracing instance.
 * \param buf Pointer to the area of memory to write data read from the stream.
 * \param size Number of bytes to read.
 *
 * \see AnyTracing_readMsg
 *
 * \return True on success, False otherwise.
 */
bool AnyTracing_read( AnyTracing *self, void *buf, long size );

/*!
 * \brief Build AnyTracingHeader from given data.
 *
 * This function builds a new AnyTracingHeader with all the
 * information passed as argument. Only self, header and categoryId
 * are required. The information will be stored in the header pointed
 * to by the AnyTracingHeader pointer passed as parameter, thus
 * avoiding the need of a new allocation every time the function is
 * called. This function has to be used when writing a tracing
 * function.
 *
 * \param self AnyTracing instance.
 * \param header Target AnyTracingHeader.
 * \param timestamp Timestamp of the message sent.
 * \param categoryId Category of the tracing message.
 * \param logLevel Message log level.
 * \param hostId Sending host id.
 * \param pid Process ID of sender.
 * \param threadId Thread ID of sender.
 * \param componentId Sending module id.
 * \param fileNameId Filename of the module requiring the tracing.
 * \param line Codeline where the tracing was requested.
 */
void AnyTracing_buildHeader( AnyTracing *self,
                             AnyTracingHeader *header,
                             AnyTracingTimestamp timestamp,
                             AnyTracingRefId categoryId,
                             unsigned int logLevel,
                             AnyTracingRefId hostId,
                             unsigned int pid,
                             unsigned int threadId,
                             AnyTracingRefId componentId,
                             AnyTracingRefId fileNameId,
                             unsigned int line );

/*!
 * \brief Read a message from the stream.
 *
 * This function reads a generic message from the stream and saves it
 * to the area pointed to by buf and also returns it to the
 * caller. The read message is returned as an AnyTracingHeader with
 * the corresponded payload available by interpretating the message
 * correctly. We provide a set of macros to simplify this
 * interpretation but the user is free to write his own reading
 * routine. This function requires the area of memory where the data
 * should be written to be correctly allocated beforehand; failure to
 * do so will cause the function to return to the caller with an error
 * status.
 *
 * What follows is an example of a read loop using the macros provided.
 *
 * \code
 *
  AnyTracingHeader *header = NULL;
  long bufferSize = 1024;

  [...]

  header = ANY_BALLOC( bufferSize );

  [...]

  while( ( header = AnyTracing_readMsg( tracer, header, bufferSize ) ) )
  {
    ANYTRACING_DECODEMSG_BEGIN( header );

      ANYTRACING_DECODEMSG_DETECT_BEGIN( ANYTRACING_REGISTRATION, AnyTracingMsgRegistration, reg );
        [ Do something with the registration message here ]
      ANYTRACING_DECODEMSG_DETECT_END();

      ANYTRACING_DECODEMSG_DETECT_BEGIN( ANYTRACING_DADAPTOREVENT, RTBOSLogMsg, event );
        [ Do something with the tracing message here ]
      ANYTRACING_DECODEMSG_DETECT_END();

    ANYTRACING_DECODEMSG_END();
  }
 *
 * \endcode
 *
 * This code reads from the stream until an EOF is found and stores
 * the data read in header, which was previously allocated. With the
 * help of the macros provided by the library this loop acts as a sort
 * of dispatcher of events, in the sense that it interprets the
 * message by reading the header and leaves the user freedom to handle
 * the message in the appropriate way, dispensing him of the
 * interpretation phase.
 *
 * \param self AnyTracing instance
 * \param buf  Pointer to the area of memory to write data read from the stream. This area of memory be big enough to contain an AnyTracingHeader.
 * \param size Size of the allocated area of memory.
 *
 * \return Pointer to the read block of data, interpretated as
 * AnyTracingHeader. NULL if an error occurred or the area of memory
 * to be used as destination was not usable (not big enough or not
 * previously allocated).
 */
AnyTracingHeader *AnyTracing_readMsg( AnyTracing *self, void *buf, long size );

/*!
 * \brief Compute the djb2 hash for the given string.
 *
 * This function calculates a numeric hash of a string. It is used to
 * get an AnyTracingRefId which is guaranteed to have a really low
 * chance of collisions; this means that the ID calculated can be
 * considered unique.
 *
 * \param ptr Pointer to string.
 *
 * \return The ID for the given string.
 */
AnyTracingRefId AnyTracing_computeId( const char *ptr );

/*!
 * \brief Tracing function for message registration.
 *
 * This function is used by the macro ANYTRACING_REGISTER() to send
 * the registration message. It should not be used directly, since the
 * macro will take care of collecting all the metadata and building
 * the header, freeing the user of the hassle of doing it manually.
 *
 * \param msgToSend message
 * \param self AnyTracing instance.
 * \param nameType Type of Msg to register
 * \param id id of string to register.
 * \param str String to register.
 */
void AnyTracingMsgRegistration_trace( AnyTracingMsgRegistration *msgToSend,
                                      AnyTracing *self,
                                      NameType nameType,
                                      AnyTracingRefId id,
                                      const char *str );


/*!
 * \brief Logging function for message registration
 *
 * This function can be used to print the contents of an
 * AnyTracingMsgRegistration structure.
 *
 * \param self AnyTracingMsgRegistration instance.
 */
void AnyTracingMsgRegistration_log( AnyTracingMsgRegistration *self );

void AnyTracingHeader_log( AnyTracingHeader *self );

/*!
 * \brief Disconnect an AnyTracing instance from the stream.
 *
 * \param self AnyTracing instance.
 *
 * \return True on success, false otherwise.
 */
bool AnyTracing_disconnect( AnyTracing *self );

/*!
 * \brief Clear the instance of the AnyTracing.
 *
 * \param self Instance of the AnyTracing.
 */
void AnyTracing_clear( AnyTracing *self );

/*!
 * \brief Free all data allocated for the instace of the AnyTracing.
 *
 * \param self Instance of the AnyTracing.
 */
void AnyTracing_delete( AnyTracing *self );

#if defined(__cplusplus)
}
#endif

#endif /* ANYTRACING_H */
