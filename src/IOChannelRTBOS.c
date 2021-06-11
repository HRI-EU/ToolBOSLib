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


/* some API parameters unused but kept for polymorphism */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <IOChannelRTBOS.h>
#include <IOChannelReferenceValue.h>


IOCHANNELINTERFACE_CREATE_PLUGIN( RTBOS );


/*
 * This callback will read the last OK\n in a de/serialize VFS sequence
 *
 * -> serialize(....)
 * <- OK
 * <- [Serialize Block of Data]
 * <- OK (this is eaten by this function)
 *
 * -> deserialize
 * <- OK
 * -> [Serialize Block of Data]
 * <- OK (this is eaten by this function)
 *
 */
static void IOChannelRTBOS_onEndSerialize( IOChannel *self )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    char buffer[4];

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( BerkeleySocket_isReadDataAvailable( streamPtr->socket ))
    {
        if( BerkeleySocket_read( streamPtr->socket, (void *)buffer, 3 ) != 3 )
        {
            ANY_LOG( 0, "Error Retrieving last de/serialize OK - request acknowledgement failed.", ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        }
    }
    else
    {
        ANY_LOG( 0, "IOChannel_RTBOS: unable to read answer."
                "(BerkeleySocket_isReadDataAvailable() failed.)", ANY_LOG_ERROR );

        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
    }
}


static long IOChannelRTBOS_internalWrite( IOChannel *self, const void *buffer, long size )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE_MSG( buffer, "IOChannelRTBOS_internalWrite(). Buffer not valid." );
    ANY_REQUIRE_MSG( size > 0, "IOChannelRTBOS_internalWrite(). Size must be a positive number." );

    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( BerkeleySocket_isWritePossible( streamPtr->socket ))
    {
        retVal = BerkeleySocket_write( streamPtr->socket, (BaseUI8 *)buffer, size );

        if( retVal == -1 )
        {
            if(( errno != EALREADY ) && ( errno != ENOTCONN ))
            {
                ANY_LOG( 0, "Setting EOF", ANY_LOG_INFO );
                IOCHANNEL_SET_EOF( self );
            }
            else
            {
                ANY_LOG( 0, "Setting error", ANY_LOG_INFO );
                IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
            }
        }

        if( retVal < size )
        {
            ANY_LOG( 0, "Setting error", ANY_LOG_INFO );
            IOChannel_setError( self, IOCHANNELERROR_BLLW );
        }
    }
    else
    {
        ANY_LOG( 0, "Setting error", ANY_LOG_INFO );
        IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
        goto exitLabel;
    }

    exitLabel:;

    if( retVal == -1 && ( IOChannel_isErrorOccurred( self ) == false ))
    {
        ANY_LOG( 7,
                 "IOChannelRTBOS_internalWrite() is going to return -1 but there is no error set: setting it to avoid IOChannel crash.",
                 ANY_LOG_ERROR );
        ANY_LOG( 0, "Setting error", ANY_LOG_INFO );
        IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
    }

    return retVal;
}


void *IOChannelRTBOS_new()
{
    return ANY_TALLOC( IOChannelRTBOS );
}


bool IOChannelRTBOS_init( IOChannel *self )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Init IOChannelRTBOS  */

    /* This flag will be used by _read()/_write()
     *  to correctly decide when to send to the
     *  RTBOS instance the [de]serialization request.
     */
    streamPtr->isBeginType = true;
    Any_memset( streamPtr->format, 0, IOCHANNELRTBOS_FORMATLENGTH );

    /* Create and initialize BerkeleySocket */
    streamPtr->socket = NULL;

    streamPtr->socketClient = BerkeleySocketClient_new();
    ANY_REQUIRE( streamPtr->socketClient );

    streamPtr->onEndSerialize = NULL;

    /* let the BerkeleySocketClient to create its socket */
    if( BerkeleySocketClient_init( streamPtr->socketClient, NULL ) == false )
    {
        ANY_LOG( 0, "Unable to initialize BerkeleySocketClient for RTBOS", ANY_LOG_ERROR );
    }
    else
    {
        streamPtr->onEndSerialize = ANY_TALLOC( AnyEventInfo );
        ANY_REQUIRE( streamPtr->onEndSerialize );

        /* this will set the callback that removes the OKs */
        streamPtr->onEndSerialize->function = (void ( * )( void * ))IOChannelRTBOS_onEndSerialize;
        streamPtr->onEndSerialize->functionParam = self;
        streamPtr->onEndSerialize->next = NULL;

        retVal = true;
    }

    return retVal;
}


static bool IOChannelRTBOS_openFromString( IOChannel *self, IOChannelReferenceValue **referenceVector )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    char *ptr = (char *)NULL;
    bool retVal = false;
    int i = 0;
    long size = 0;
    char *hostName = (char *)NULL;
    int port = 0;
    char *format = (char *)NULL;
    char ipAddress[128];
    BerkeleySocketType protocol = BERKELEYSOCKET_TCP;
    char buffer[IOCHANNELRTBOS_PATHSIZE_MAXLEN];
    char *postName = (char *)NULL;
    char instanceName[IOCHANNELRTBOS_PATHSIZE_MAXLEN];
    int loops = 0;
    int retry;
    BaseUI64 retryTimeout;
    int isBlocking = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    /* Open IOChannelRTBOS: mode must be changed. */
    self->mode = IOCHANNEL_MODE_RW;

    hostName = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_HOST );

    if( !hostName )
    {
        ANY_LOG( 0, "Error. Host name not found or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    port = IOChannelReferenceValue_getInt( referenceVector, IOCHANNELREFERENCEVALUE_PORT );

    if( port <= 0 )
    {
        ANY_LOG( 0, "Error. Port not found or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    postName = IOChannelReferenceValue_getString( referenceVector, "data" );
    if( !postName )
    {
        ANY_LOG( 0, "Error. Name of data structure not found or error occurred.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    format = IOChannelReferenceValue_getString( referenceVector, "format" );
    if( !format )
    {
        ANY_LOG( 5, "Warning. Format not found - defaulting to 'Binary'.", ANY_LOG_WARNING );
        format = "Binary";
    }

    retry = IOChannelReferenceValue_getInt( referenceVector, "retry" );

    if( retry == 0 )
    {
        ANY_LOG( 5, "Warning. retry not found - defaulting to '1'.", ANY_LOG_WARNING );
        retry = 1;
    }

    retryTimeout = IOChannelReferenceValue_getLong( referenceVector, "retrytimeout" );

    if( retryTimeout == 0 )
    {
        ANY_LOG( 5, "Warning. retryTimeout not found - defaulting to '1 second'.", ANY_LOG_WARNING );
        retryTimeout = 1000L;
    }

    isBlocking = IOChannelReferenceValue_getLong( referenceVector, "blocking" );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /*
     * Must be reset to true for each _open()
     *  in order to ensure we mantain a consistent
     *  state.
     */
    streamPtr->isBeginType = true;
    streamPtr->isBlocking = isBlocking != 0;

    Any_strncpy( streamPtr->format, format, IOCHANNELRTBOS_FORMATLENGTH - 1 );

    /* Resolving host and connecting */
    if( BerkeleySocket_host2Addr( hostName, ipAddress, 128 ) == NULL )
    {
        ANY_LOG( 1, "Unable to resolve the hostname: %s", ANY_LOG_WARNING, hostName );
        goto exitLabel;
    }

    while( loops < retry )
    {
        ANY_LOG( 5, "Connecting to RTBOS '%s:%d' try #%d/%d",
                 ANY_LOG_INFO, ipAddress, port, loops, retry );

        streamPtr->socket = BerkeleySocketClient_connect( streamPtr->socketClient, protocol, ipAddress, port );

        if( streamPtr->socket == NULL )
        {
            ANY_LOG( 0, "Connection try #%d/%d to RTBOS '%s:%d' has failed",
                     ANY_LOG_WARNING, loops, retry, ipAddress, port );

            loops++;

            if( loops < retry )
            {
                ANY_LOG( 0, "Connection try #%d/%d waiting RTBOS '%s:%d' becoming available in %"
                BASEUI64_PRINT
                "ms",
                        ANY_LOG_WARNING, loops, retry, ipAddress, port, retryTimeout );

                Any_sleepMilliSeconds( retryTimeout );
            }
            else
            {
                ANY_LOG( 0, "Connection try #%d/%d to RTBOS '%s:%d' has failed. ABORTING!!!",
                         ANY_LOG_ERROR, loops, retry, ipAddress, port );

                IOChannel_setError( self, IOCHANNELERROR_UCONCL );
                goto exitLabel;
            }
        }
        else
        {
            break;
        }
    }

    BerkeleySocket_setDefaultTimeout( streamPtr->socket,
                                      BERKELEYSOCKET_TIMEOUT_SECONDS( IOCHANNELRTBOS_SOCKET_TIMEOUT ));
    /* set blocking mode if requested */
    if( streamPtr->isBlocking )
    {
        ANY_LOG( 1, "Setting Blocking mode for RTBOS channel %s:%d", ANY_LOG_WARNING, ipAddress, port );
        BerkeleySocket_setBlocking( streamPtr->socket, true );
    }

    IOChannel_setType( self, IOCHANNELTYPE_SOCKET );

    ANY_LOG( 5, "Connection established: receiving header.", ANY_LOG_INFO );

    /* Reading header */

    if( BerkeleySocket_isReadDataAvailable( streamPtr->socket ))
    {
        char *aux = buffer;
        for( i = 0; i < IOCHANNELRTBOS_PATHSIZE_MAXLEN; i++ )
        {
            if( BerkeleySocket_read( streamPtr->socket, (void *)aux, 1 ) == 1 )
            {
                if( *aux == '\n' )
                {
                    break;
                }
                aux++;
            }
        }
        ptr = aux;
    }
    else
    {
        ANY_LOG( 0, "Unable to receive header - no data incoming. ", ANY_LOG_ERROR );
        ANY_LOG( 7, "BerkeleySocket_isReadDataAvailable() failed.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    if( *ptr == '\n' )
    {
        *ptr = '\0';
        ANY_LOG( 5, "Received Header: [%s]", ANY_LOG_INFO, buffer );
    }
    else
    {
        *ptr = '\0';
        ANY_LOG( 5, "Incorrect header syntax: [%s]", ANY_LOG_ERROR, buffer );
        ANY_LOG( 7, "Could not find newline.", ANY_LOG_INFO );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    /* handle the RTBOS connection header */

    /* Remove the "Ready" */
    ptr = Any_rindex( buffer, ' ' );
    ANY_REQUIRE( ptr );
    *ptr = '\0';

    /* removed the architecture info */
    ptr = Any_rindex( buffer, ' ' );
    ANY_REQUIRE( ptr );
    *ptr = '\0';

    /* get the RTBOS instance name */
    ptr = Any_rindex( buffer, ' ' );
    ANY_REQUIRE( ptr );
    ptr++;
    ANY_REQUIRE( ptr );

    Any_strncpy( instanceName, ptr, IOCHANNELRTBOS_PATHSIZE_MAXLEN - 1 );
    ANY_LOG( 7, "instanceName[%s]", ANY_LOG_INFO, instanceName );

    /* Sending request to change path */
    ANY_LOG( 5, "Sending request to change path: ["
    IOCHANNELRTBOS_REPOSITORYPATH_PREFIX
    "%s_%s]",
            ANY_LOG_INFO, instanceName, postName );

    size = Any_snprintf( buffer, IOCHANNELRTBOS_PATHSIZE_MAXLEN,
                         "cd "
    IOCHANNELRTBOS_REPOSITORYPATH_PREFIX
    "%s_%s\n",
            instanceName, postName );

    if( BerkeleySocket_isWritePossible( streamPtr->socket ))
    {
        if( BerkeleySocket_write( streamPtr->socket, (BaseUI8 *)buffer, size ) != size )
        {
            ANY_LOG( 0, "IOChannel_RTBOS: Unable to send Request to change path in '%s'!",
                     ANY_LOG_ERROR, buffer );

            IOChannel_setError( self, IOCHANNELERROR_UCONCL );
            goto exitLabel;
        }
    }
    else
    {
        ANY_LOG( 0, "IOChannel_RTBOS: unable to send request to change path in '%s'."
                "(BerkeleySocket_isWritePossible() failed.)", ANY_LOG_ERROR, buffer );

        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    /* Reading ack */
    if( BerkeleySocket_isReadDataAvailable( streamPtr->socket ))
    {
        if( BerkeleySocket_read( streamPtr->socket, (BaseUI8 *)buffer, 3 ) != 3 )
        {
            ANY_LOG( 0, "%s: No such BBDM (please check spelling)",
                     ANY_LOG_ERROR, postName );
            IOChannel_setError( self, IOCHANNELERROR_UCONCL );
            goto exitLabel;
        }
        buffer[ 3 ] = '\0';
    }
    else
    {
        ANY_LOG( 0, "IOChannel_RTBOS: unable to read answer."
                "(BerkeleySocket_isReadDataAvailable() failed.)", ANY_LOG_ERROR );

        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }

    if( Any_strcmp( buffer, "OK\n" ) != 0 )
    {
        ANY_LOG( 0, "Request to change path has failed.", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_UCONCL );
        goto exitLabel;
    }
    else
    {
        ANY_LOG( 7, "Request to change path accepted.", ANY_LOG_INFO );
    }

    retVal = true;

    exitLabel:
    return retVal;
}


/* Example of openString:
 *  RTBOS://localhost:2000/blockF32[@Binary] <- Format is optional
 *  host = localhost port = 2000 data = blockF32 format = Binary <- format is optional
 */
bool IOChannelRTBOS_open( IOChannel *self, char *infoString,
                          IOChannelMode mode,
                          IOChannelPermissions permissions,
                          va_list varArg )
{
    bool retVal = false;
    int i = 0;
    int j = 0;
    char *ptr;
    char hostName[256];
    char port[256];
    char data[256];
    char format[256];
    int status = 0;
    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    if( *infoString == IOCHANNELREFERENCEVALUE_EOF )
    {
        ANY_LOG( 0, "IOChannelRTBOS_open(). Not valid info string to open server connection. ", ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BIST );
        goto outLabel;
    }

    Any_memset( hostName, 0, 256 );
    Any_memset( port, 0, 256 );
    Any_memset( data, 0, 256 );
    Any_memset( format, 0, 256 );

    /*
     * Status:
     *  0 - parsing hostname
     *  1 - parsing port
     *  2 - parsing data
     *  3 - parsing format
     */

    ptr = infoString;
    for( i = 0; *ptr; i++, ptr++ )
    {
        switch( status )
        {
            case 0:
                if( *ptr == ':' )
                {
                    hostName[ j ] = '\0';
                    status++;
                    j = 0;
                    break;
                }
                hostName[ j ] = *ptr;
                j++;
                break;
            case 1:
                if( *ptr == '/' )
                {
                    port[ j ] = '\0';
                    status++;
                    j = 0;
                    break;
                }
                port[ j ] = *ptr;
                j++;
                break;
            case 2:
                if( *ptr == '@' )
                {
                    data[ j ] = '\0';
                    status++;
                    j = 0;
                    break;
                }
                data[ j ] = *ptr;
                j++;
                break;
            case 3:
                format[ j ] = *ptr;
                j++;
                break;
        }
    }

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    IOCHANNELREFERENCEVALUE_ADDSET( host, "%s", hostName );

    IOCHANNELREFERENCEVALUE_ADDSET( port, "%s", port );

    IOCHANNELREFERENCEVALUE_ADDSET( data, "%s", data );

    if( Any_strlen( format ) > 0 )
    {
        IOCHANNELREFERENCEVALUE_ADDSET( format, "%s", format );
    }

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelRTBOS_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

    outLabel:;
    return retVal;
}


long IOChannelRTBOS_read( IOChannel *self, void *buffer, long size )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    long retVal = -1;
    char *ptr = NULL;
    int i = 0;
    int status = -1;
    char buff[512];

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE_MSG( buffer, "IOChannelRTBOS_read(). Not valid buffer" );
    ANY_REQUIRE_MSG( size > 0, "IOChannelRTBOS_read(). Size must be a positive number" );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Read from IOChannelRTBOS  */
    if( streamPtr->isBeginType )
    {
        char ackBuffer[5];
        int len = 0;

        Any_memset( ackBuffer, 0, 5 );

        ANY_LOG( 7, "IOChannel_read(): Sending serializeCmdRead request...",
                 ANY_LOG_INFO );

        streamPtr->isBeginType = false;

        len = Any_strlen( streamPtr->format );
        ANY_REQUIRE( len > 0 );

        len = Any_snprintf( buff, 512,
                            ( streamPtr->isBlocking ? IOCHANNELRTBOS_CMDBLOCKINGREAD : IOCHANNELRTBOS_CMDREAD ),
                            streamPtr->format );

        if( BerkeleySocket_write( streamPtr->socket, (void *)buff, len ) != len )
        {
            ANY_LOG( 0, "IOChannelRTBOS_read(). Unable to send serialize command.",
                     ANY_LOG_ERROR );
            goto exitLabel;
        }

        ptr = ackBuffer;
        if( BerkeleySocket_isReadDataAvailable( streamPtr->socket ))
        {
            i = 0;

            while( i < 3 )
            {
                status = BerkeleySocket_read( streamPtr->socket, (void *)ptr, 1 );
                ANY_LOG( 5, "STATUS[%d][%c]", ANY_LOG_INFO, status, *ptr );

                if( status == 1 )
                {
                    ptr++;
                    i++;
                }
                else if( status == -1 )
                {
                    ANY_LOG( 0, "IOChannelRTBOS_read(). Unable to receive ack after serialize request.",
                             ANY_LOG_ERROR );
                    goto exitLabel;
                }
            }
        }
        else
        {
            ANY_LOG( 0, "Data not available, unable to read the Ok.", ANY_LOG_ERROR );
            goto exitLabel;
        }

        if( Any_strcmp( ackBuffer, "OK\n" ) != 0 )
        {
            ANY_LOG( 0, "IOChannelRTBOS_read(). Serialize request refused ([%s] was received instead of OK).",
                     ANY_LOG_ERROR, ackBuffer );
            goto exitLabel;
        }

        ANY_LOG( 5, "Received Ack for the request[serialize( %s, \"\" )]",
                 ANY_LOG_INFO, streamPtr->format );
    }

    if( streamPtr->isBlocking || BerkeleySocket_isReadDataAvailable( streamPtr->socket ))
    {
        retVal = BerkeleySocket_read( streamPtr->socket, buffer, size );

        if( retVal == -1 )
        {
            if(( errno != EALREADY ) && ( errno != ENOTCONN ))
            {
                IOCHANNEL_SET_EOF( self );
            }
            else
            {
                IOChannel_setError( self, IOCHANNELERROR_BSOCKR );
            }
        }

        if( retVal == 0 )
        {
            /*IOCHANNEL_SET_EOF( self );*/
        }
    }
    else
    {
        ANY_LOG( 5, "BerkeleySocket_isReadDataAvailable() returned false", ANY_LOG_INFO );
        goto exitLabel;
    }

    exitLabel:;

    if( retVal == -1 && ( IOChannel_isErrorOccurred( self ) == false ))
    {
        ANY_LOG( 5,
                 "IOChannelRTBOS_read() is going to return -1 but there is no error set: setting it to avoid IOChannel crash.",
                 ANY_LOG_ERROR );
        IOChannel_setError( self, IOCHANNELERROR_BSOCKR );
    }

    return retVal;
}


long IOChannelRTBOS_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE_MSG( buffer, "IOChannelRTBOS_write(). Buffer not valid." );
    ANY_REQUIRE_MSG( size > 0, "IOChannelRTBOS_write(). Size must be a positive number." );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( streamPtr->isBeginType )
    {
        char ackBuffer[5];
        int mySize = 0;

        ANY_LOG( 5, "isBeginType == true", ANY_LOG_INFO );

        streamPtr->isBeginType = false;

        mySize = Any_strlen( IOCHANNELRTBOS_CMDWRITE );

        if( BerkeleySocket_write( streamPtr->socket,
                                  (void *)IOCHANNELRTBOS_CMDWRITE, mySize ) != mySize )
        {
            ANY_LOG( 0, "IOChannelRTBOS_write(). Unable to send RTBOS deserialize command [%s].",
                     ANY_LOG_ERROR, IOCHANNELRTBOS_CMDWRITE );
            IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
            goto exitLabel;
        }

        if( BerkeleySocket_read( streamPtr->socket, (void *)ackBuffer, 3 ) != 3 )
        {
            ANY_LOG( 0, "IOChannelRTBOS_write(). Unable to get ack after the request to deserialize.", ANY_LOG_ERROR );
            IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
            goto exitLabel;
        }
    }

    if( IOChannel_usesWriteBuffering( self ))
    {
        retVal = IOChannel_addToWriteBuffer( self, buffer, size );
    }
    else
    {
        retVal = IOChannelRTBOS_internalWrite( self, buffer, size );
    }

    exitLabel:;

    if( retVal == -1 && ( IOChannel_isErrorOccurred( self ) == false ))
    {
        ANY_LOG( 7,
                 "IOChannelRTBOS_write() is going to return -1 but there is no error set: setting it to avoid IOChannel crash.",
                 ANY_LOG_ERROR );
        ANY_LOG( 0, "Setting Error", ANY_LOG_INFO );
        IOChannel_setError( self, IOCHANNELERROR_BSOCKW );
    }

    return retVal;
}


long IOChannelRTBOS_flush( IOChannel *self )
{
    long retVal = -1;
    void *buffer;
    long size;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    if( IOChannel_usesWriteBuffering( self ))
    {
        buffer = IOChannel_getInternalWriteBufferPtr( self );
        size = IOChannel_getWriteBufferedBytes( self );

        ANY_REQUIRE_MSG( buffer, "IOChannelRTBOS_flush(). Buffer not valid." );
        ANY_REQUIRE_MSG( size > 0, "IOChannelRTBOS_flush(). Size must be a positive number." );

        retVal = IOChannelRTBOS_internalWrite( self, buffer, size );
    }
    else
    {
        retVal = 0;
    }

    return retVal;
}


long long IOChannelRTBOS_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    long long retVal = 0;

    ANY_REQUIRE( self );

    return retVal;
}


bool IOChannelRTBOS_close( IOChannel *self )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    ANY_LOG( 7, "Closing RTBOS connection..", ANY_LOG_INFO );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Close IOChannelRTBOS */
    ANY_REQUIRE( streamPtr->socketClient );

    BerkeleySocketClient_disconnect( streamPtr->socketClient );
    streamPtr->socket = NULL;

    retVal = true;

    return retVal;
}


void *IOChannelRTBOS_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Get propertyName */
    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( Socket )
        {
            retVal = BerkeleySocketClient_getSocket( streamPtr->socketClient );
        }
        IOCHANNELPROPERTY_PARSE_END( Socket )

        IOCHANNELPROPERTY_PARSE_BEGIN( SocketClient )
        {
            retVal = streamPtr->socketClient;
        }
        IOCHANNELPROPERTY_PARSE_END( SocketClient )

        IOCHANNELPROPERTY_PARSE_BEGIN( Fd )
        {
            streamPtr->socketFd = BerkeleySocket_getFd(
                    BerkeleySocketClient_getSocket( streamPtr->socketClient ));
            retVal = &streamPtr->socketFd;
        }
        IOCHANNELPROPERTY_PARSE_END( Fd )

        IOCHANNELPROPERTY_PARSE_BEGIN( isBeginType )
        {
            retVal = (void *)streamPtr->isBeginType;
        }
        IOCHANNELPROPERTY_PARSE_END( isBeginType )

        IOCHANNELPROPERTY_PARSE_BEGIN( onEndSerialize )
        {
            retVal = (void *)streamPtr->onEndSerialize;
        }
        IOCHANNELPROPERTY_PARSE_END( onEndSerialize )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


bool IOChannelRTBOS_setProperty( IOChannel *self, const char *propertyName, void *property )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Set propertyName */
    if( Any_strcmp( propertyName, "isBeginType" ) == 0 )
    {
        streamPtr->isBeginType = (bool)property;
        retVal = true;
    }
    else if( Any_strcmp( propertyName, "onEndSerialize" ) == 0 )
    {
        streamPtr->onEndSerialize = (AnyEventInfo *)property;
        retVal = true;
    }
    else
    {
        ANY_LOG( 5, "Unknown property. Only valid property name is 'isBeginType'.", ANY_LOG_WARNING );
    }

    return retVal;
}


void IOChannelRTBOS_clear( IOChannel *self )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr->onEndSerialize );
    streamPtr->onEndSerialize = NULL;

    /* Clear IOChannelRTBOS */
    BerkeleySocketClient_clear( streamPtr->socketClient );
    BerkeleySocketClient_delete( streamPtr->socketClient );
}


void IOChannelRTBOS_delete( IOChannel *self )
{
    IOChannelRTBOS *streamPtr = (IOChannelRTBOS *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


/* EOF */
