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


#include <AnyTracing.h>
#include <BerkeleySocket.h>


#define ANYTRACING_VALID      0x81f3a464
#define ANYTRACING_INVALID    0xfede4cb0
#define ANYTRACING_HASHSTRING_FACTOR 5381


AnyTracing *AnyTracing_new( void )
{
    return (ANY_TALLOC( AnyTracing ));
}


bool AnyTracing_init( AnyTracing *self )
{
    bool outVal = false;

    ANY_REQUIRE( self );

    self->valid = ANYTRACING_INVALID;

    /* IOChannel stream */

    self->stream = IOChannel_new();
    ANY_REQUIRE( self->stream );


    /* unreachable code: IOChannel_init() always returns true
     *
    if( IOChannel_init( self->stream ) == false )
    {
      ANY_LOG( 0, "Could not initialize IOChannel.", ANY_LOG_ERROR );
      goto outLabel;
    }
     * changed to:
     */
    IOChannel_init( self->stream );

    self->valid = ANYTRACING_VALID;

    outVal = true;

    /* unreachable code: labels are never used
    outLabel:
     */


    /* always returns 'true' */
    return ( outVal );
}


IOChannel *AnyTracing_getStream( AnyTracing *self )
{
    ANY_REQUIRE( self );
    return self->stream;
}


void AnyTracing_setDefaultHostId( AnyTracing *self, AnyTracingRefId id )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( id > 0 );

    self->defaultHostId = id;
}


AnyTracingRefId AnyTracing_getDefaultHostId( AnyTracing *self )
{
    ANY_REQUIRE( self );

    return ( self->defaultHostId );
}


bool AnyTracing_connect( AnyTracing *self, char *dest )
{
    bool outVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( dest );

    if( IOChannel_openFromString( self->stream, dest ) == false )
    {
        ANY_LOG( 0, "Could not open IOChannel.", ANY_LOG_ERROR );
        goto outLabel;
    }
    if( IOChannel_hasBerkeleySocket( self->stream ))
    {
        BerkeleySocket *socket = (BerkeleySocket *)IOChannel_getProperty( self->stream, "Socket" );
        BerkeleySocket_setDefaultTimeout( socket, 0 );
        BerkeleySocket_setTcpNoDelay( socket, true );
    }

    outVal = true;

    outLabel:
    return ( outVal );
}


bool AnyTracing_write( AnyTracing *self, void *buf, long size )
{
    bool outVal = false;
    char *ptr = NULL;
    long writtenBytes = 0;
    long sizeToWrite = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( buf );
    ANY_REQUIRE( size );

    sizeToWrite = size;

    /* Use a char* pointer because GCC complains about void pointers
     * when used in pointer arithmetics. */
    ptr = (char *)buf;

    do
    {
        if( IOChannel_isWritePossible( self->stream ))
        {
            writtenBytes = IOChannel_write( self->stream, ptr, sizeToWrite );

            if( writtenBytes < 0 || IOChannel_isErrorOccurred( self->stream ))
            {
                ANY_LOG( 0, "An error occurred while writing to stream: %s",
                         ANY_LOG_ERROR, IOChannel_getErrorDescription( self->stream ));
                goto outLabel;
            }

            if( writtenBytes == sizeToWrite )
            {
                /* ANY_LOG( 5, "Wrote %ld bytes successfully.", ANY_LOG_INFO, size ); */
                break;
            }
            else
            {
                /* calculate how many bytes we still have to write */
                sizeToWrite -= writtenBytes;
                /* Shift pointer of how many bytes we already wrote */
                ptr += writtenBytes;
                ANY_LOG( 5, "Still something to write, staying in the loop: writtenBytes [%ld], sizeToWrite [%ld]",
                         ANY_LOG_INFO, writtenBytes, sizeToWrite );
            }
        }
        else
        {
            ANY_LOG( 0, "Could not get write access to the stream. ", ANY_LOG_ERROR );
            goto outLabel;
        }
    } while( true );

    outVal = true;

    outLabel:
    return outVal;
}


bool AnyTracing_read( AnyTracing *self, void *buf, long size )
{
    bool outVal = false;
    char *ptr = NULL;
    long readBytes;
    long sizeToRead;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( buf );
    ANY_REQUIRE( size );

    sizeToRead = size;

    /* Use a char* pointer because GCC complains about void pointers
     * when used in pointer arithmetics. */
    ptr = (char *)buf;

    do
    {
        if( IOChannel_isReadDataAvailable( self->stream ))
        {
            readBytes = IOChannel_read( self->stream, ptr, sizeToRead );

            if( IOChannel_eof( self->stream ) == true )
            {
                ANY_LOG( 5, "Found EOF, quitting.", ANY_LOG_INFO );
                goto outLabel;
            }

            if( readBytes < 0 || IOChannel_isErrorOccurred( self->stream ))
            {
                ANY_LOG( 0, "An error occurred while reading from stream: %s", ANY_LOG_ERROR,
                         IOChannel_getErrorDescription( self->stream ));
                goto outLabel;
            }

            if( sizeToRead == 0 || readBytes == sizeToRead )
            {
                ANY_LOG( 5, "Read whole header, quitting loop.", ANY_LOG_INFO );
                break;
            }
            else
            {
                /* We enter this branch when we read less than what we
                 * expected to read from the stream; this could happen
                 * especially when dealing with socket based streams. How do
                 * we handle this situation: we calculate how many bytes we
                 * still have to read, then we move the pointer of the buffer
                 * of [number of bytes read and already stored], so that the
                 * next read will "append" the chunk in the correct
                 * order/position. Were we not to do this we'd overwrite what
                 * we just read! */
                ANY_LOG( 5, "Still something to read, staying in the loop: readBytes [%ld], sizeToRead [%ld]",
                         ANY_LOG_INFO, readBytes, sizeToRead );
                sizeToRead -= readBytes;
                ptr += readBytes;
            }
        }
    } while( true );

    outVal = true;

    outLabel:
    return outVal;
}


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
                             unsigned int line )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( header );
    ANY_REQUIRE( categoryId );

    header->valid = ANYTRACING_HEADER_VALID;
    header->timestamp = timestamp;
    header->msgCategory = categoryId;
    header->logLevel = logLevel;
    header->hostId = ( hostId == ANYTRACING_DEFAULTHOSTID ? self->defaultHostId : hostId );
    header->pid = pid;
    header->threadId = threadId;
    header->moduleId = componentId;
    header->fileNameId = fileNameId;
    header->codeLine = line;
}


AnyTracingHeader *AnyTracing_readMsg( AnyTracing *self, void *buf, long size )
{
    char *ptr = NULL;
    AnyTracingHeader *header = NULL;
    long sizeToRead = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( buf );
    ANY_REQUIRE( (unsigned long)size > sizeof( AnyTracingHeader ));

    /*  unreachable code:
     *
    if( size < sizeof( AnyTracingHeader ) )
    {
      ANY_LOG( 0, "Buffer allocated is too small to contain header, quitting.", ANY_LOG_ERROR );
      goto outLabel;
    }
    */

    /* We need to read the header first */
    sizeToRead = sizeof( AnyTracingHeader );

    if( AnyTracing_read( self, buf, sizeToRead ) == false )
    {
        ANY_LOG( 0, "An error occurred while reading from stream or EOF was found.", ANY_LOG_INFO );
        goto outLabel;
    }

    /* Do this now because we need to access the sizeOfStruct field */
    header = (AnyTracingHeader *)buf;

    sizeToRead = ( header->sizeOfStruct - sizeof( AnyTracingHeader ));

    /* Read next block of data from stream and store it in ptr. Note:
     * since we already copied the header "on top" of the destination
     * buffer, we use as destination the offset from the beginning of
     * buffer shifted sizeof(AnyTracingHeader) bytes, so that it aligns
     * correctly. */
    ptr = (char *)header + sizeof( AnyTracingHeader );

    if( AnyTracing_read( self, ptr, sizeToRead ) == false )
    {
        ANY_LOG( 0, "An error occurred while reading from stream or EOF was found", ANY_LOG_INFO );
        header = NULL;
        goto outLabel;
    }

    outLabel:
    return ( header );
}


AnyTracingRefId AnyTracing_computeId( const char *str )
{
    /*
     * This string hashing algorithm is the djb2. This algorithm (k=33)
     * was first reported by Dan Bernstein many years ago in
     * comp.lang.c. Another version of this algorithm (now favored by
     * Bernstein) uses xor:
     *
     * hash(i) = hash(i - 1) * 33 ^ str[i];
     *
     * the magic of number 33 (why it works better than many other constants,
     * prime or not) has never been adequately explained.
     */

    AnyTracingRefId hash = ANYTRACING_HASHSTRING_FACTOR;
    int c;

    while(( c = *str++ ))
    {
        hash = (( hash << 5 ) + hash ) + c;    /* hash * 33 + c */
    }

    return hash;
}


void AnyTracingMsgRegistration_trace( AnyTracingMsgRegistration *msgToSend,
                                      AnyTracing *self,
                                      NameType nameType,
                                      AnyTracingRefId id,
                                      const char *str )
{
    long size = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );
    ANY_REQUIRE( msgToSend );
    ANY_REQUIRE( msgToSend->head.valid == ANYTRACING_HEADER_VALID );
    ANY_REQUIRE( id > 0 );
    ANY_REQUIRE( str );

    //ANY_LOG (0, "Sender: NameRegistration of type %d : %lld = '%s'", ANY_LOG_INFO, nameType, id, str);
    msgToSend->msgLen = Any_strlen( str );
    msgToSend->nameType = nameType;
    msgToSend->msgId = id;

    /* The sizeOfStruct field stores the size of the whole structure,
     * without the size of the pointer to the string but considering the
     * size of the whole string, complete with /0 */
    msgToSend->head.sizeOfStruct =
            sizeof( AnyTracingMsgRegistration ) - sizeof( msgToSend->m.msg ) + ( msgToSend->msgLen + 1 );

    //AnyTracingMsgRegistration_log (msgToSend);

    /* We need to send the header, the ID and the message length as
     * 'meta information' that the other side will use to read the NEXT
     * IOChannel_write() that'll come, containing the string to
     * associate to this ID. */
    size = sizeof( AnyTracingMsgRegistration ) - sizeof( msgToSend->m.msg );

    //ANY_LOG (0, "write %ld bytes of %d'", ANY_LOG_INFO, size , sizeof( AnyTracingMsgRegistration ));

    if( AnyTracing_write( self, msgToSend, size ) == false )
    {
        ANY_LOG( 0, "An error occurred while writing to stream.", ANY_LOG_ERROR );
    }

    /* Calculate size of the payload. */
    size = msgToSend->msgLen + 1;

    //ANY_LOG (0, "write %ld extended bytes total %d'", ANY_LOG_INFO, size , msgToSend->head.sizeOfStruct);

    if( AnyTracing_write( self, (void *)str, size ) == false )
    {
        ANY_LOG( 0, "An error occurred while writing to stream.", ANY_LOG_ERROR );
    }
}


void AnyTracingHeader_log( AnyTracingHeader *self )
{
    ANY_REQUIRE( self );

    ANY_LOG( 0, "Valid: %lx", ANY_LOG_INFO, self->valid );
    ANY_LOG( 0, "size: %u", ANY_LOG_INFO, self->sizeOfStruct );
    ANY_LOG( 0, "ts: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->timestamp );
    ANY_LOG( 0, "category: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->msgCategory );
    ANY_LOG( 0, "loglevel: %d", ANY_LOG_INFO, self->logLevel );
    ANY_LOG( 0, "hostid: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->hostId );
    ANY_LOG( 0, "pid: %u", ANY_LOG_INFO, self->pid );
    ANY_LOG( 0, "threadid: %u", ANY_LOG_INFO, self->threadId );
    ANY_LOG( 0, "moduleId: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->moduleId );
    ANY_LOG( 0, "fileNameId: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->fileNameId );
    ANY_LOG( 0, "codeline: %u", ANY_LOG_INFO, self->codeLine );
}


void AnyTracingMsgRegistration_log( AnyTracingMsgRegistration *self )
{
    ANY_REQUIRE( self );

    AnyTracingHeader_log( &self->head );
    ANY_LOG( 0, "String: %s", ANY_LOG_INFO, self->m.msgVect );
    ANY_LOG( 0, "ID: %"
            BASEUI64_PRINT, ANY_LOG_INFO, self->msgId );
}


bool AnyTracing_disconnect( AnyTracing *self )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );

    if( IOChannel_isOpen( self->stream ))
    {
        IOChannel_close( self->stream );
    }

    retVal = true;

    return retVal;
}


void AnyTracing_clear( AnyTracing *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ANYTRACING_VALID );

    AnyTracing_disconnect( self );

    IOChannel_clear( self->stream );
    IOChannel_delete( self->stream );

    self->valid = ANYTRACING_INVALID;
}


void AnyTracing_delete( AnyTracing *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* EOF */
