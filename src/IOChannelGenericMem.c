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


#include <IOChannelGenericMem.h>


#if !defined(__windows__)

static bool IOChannel_setProtection( IOChannel *self, int *protection );

#endif

static long long IOChannelGenericMem_seekBack( IOChannel *self, long long offset );

static long long IOChannelGenericMem_seekForward( IOChannel *self, long long offset );


void *IOChannelGenericMem_new( void )
{
    IOChannelGenericMem *self;

    self = ANY_TALLOC( IOChannelGenericMem );

    ANY_REQUIRE( self );

    return self;
}


bool IOChannelGenericMem_init( IOChannel *self )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->ptr = (void *)NULL;
    streamPtr->fd = -1;
    streamPtr->size = 0;
    streamPtr->isMapped = false;

    return true;
}


void IOChannelGenericMem_setPtr( IOChannel *self,
                                 void *ptr,
                                 int fd,
                                 long size,
                                 bool isMapped )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE( ptr );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->ptr = ptr;
    streamPtr->fd = fd;
    streamPtr->size = size;
    streamPtr->isMapped = isMapped;
    IOChannel_setType( self, IOCHANNELTYPE_MEMPTR );
}


long IOChannelGenericMem_read( IOChannel *self, void *buffer, long size )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    long nBytes = 0;
    char *ptr = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ptr = (char *)streamPtr->ptr;
    ptr += self->currentIndexPosition;

    if( self->currentIndexPosition < streamPtr->size )
    {
        if(( self->currentIndexPosition + size ) > streamPtr->size )
        {
            nBytes = (long)( streamPtr->size - self->currentIndexPosition );
            IOCHANNEL_SET_EOF( self );
        }
        else
        {
            nBytes = size;
        }

        /* When Buffer is full, nBytes == 0 */
        ANY_REQUIRE( nBytes >= 0 );
        Any_memcpy( buffer, ptr, nBytes );
    }
    else
    {
        nBytes = -1;
        IOCHANNEL_SET_EOF( self );
    }

    return (long)nBytes;
}


long IOChannelGenericMem_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    long nBytes = 0;
    char *ptr = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size > 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ptr = (char *)streamPtr->ptr;
    ptr += self->currentIndexPosition;

    if( self->currentIndexPosition < streamPtr->size )
    {
        if(( self->currentIndexPosition + size ) > streamPtr->size )
        {
            nBytes = (long)( streamPtr->size - self->currentIndexPosition );
            ANY_LOG( 0, "Writing more bytes (%ld) then remaining (%ld) into IOChannel Mem stream", ANY_LOG_WARNING,
                     size, nBytes );
            IOCHANNEL_SET_EOF( self );
        }
        else
        {
            nBytes = size;
        }

        /* When Buffer is full, nBytes == 0 */
        ANY_REQUIRE( nBytes >= 0 );
        Any_memcpy( ptr, buffer, nBytes );
    }
    else
    {
        nBytes = -1;
        IOCHANNEL_SET_EOF( self );
    }

    return (long)nBytes;
}


long IOChannelGenericMem_flush( IOChannel *self )
{
    long retVal = 0;

    ANY_REQUIRE( self );
    /* Where not allowed, flush returns always 0 */
    ANY_LOG( 5, "Cannot do flush on Generics memory streams", ANY_LOG_WARNING );

    return retVal;
}


long long IOChannelGenericMem_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    /* IOChannel_seek sets self->currentIndexPosition with the  */
    /* low level return value unless it is a negative number  */
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    long long retVal = -1;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    switch( whence )
    {
        case IOCHANNELWHENCE_SET:
        {
            if(( offset > streamPtr->size ) || ( offset < 0 ))
            {
                IOChannel_setError( self, IOCHANNELERROR_EOVERFLOW );
                retVal = -1;
            }
            else
            {
                /* Change retVal and currentIndexPosition */
                self->currentIndexPosition = offset;
                retVal = offset;
            }
            break;
        }

        case IOCHANNELWHENCE_CUR:
        {
            if( offset == 0 )
            {
                /* Seek was called to get current position... */
                retVal = self->currentIndexPosition;
            }
            else if( offset < 0 )
            {
                /* Seek was called to go back... */
                retVal = IOChannelGenericMem_seekBack( self, offset );
            }
            else
            {
                /* Seek was called to go forward... */
                retVal = IOChannelGenericMem_seekForward( self, offset );
            }
            break;
        }

        case IOCHANNELWHENCE_END:
        {
            IOChannel_setError( self, IOCHANNELERROR_BNDSEK );
            retVal = -1;
            break;
        }

        default:
            IOChannel_setError( self, IOCHANNELERROR_BWHESEK );
            retVal = -1;
            break;
    }

    return retVal;
}


bool IOChannelGenericMem_mapFd( IOChannel *self, int fd, long size )
{
    bool retVal = false;

#if defined(__windows__)
    ANY_LOG( 1, "The mmap() is not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else /* !__windows__ */

    void *startAddress = (void *)NULL;
    void *ptr = (void *)NULL;
    int protection = 0;
    int offset = 0;
    long length = 0;
    int status = 0;
    int flags = 0;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    ANY_REQUIRE_MSG( fd >= 0, "IOChannelGenericMem_mapFd(). Not valid fd to map. It is negative!" );
    ANY_REQUIRE_MSG( size > 0, "IOChannelGenericMem_mapFd(). Size must be a positive number" );

    /* Set Here Flags */
    length = size;
    flags = MAP_SHARED;

    if( IOChannel_setProtection( self, &protection ) == false )
    {
        retVal = false;
        goto outLabel;
    }

    if( IOCHANNEL_MODEIS_CREAT( self->mode ) || IOCHANNEL_MODEIS_TRUNC( self->mode ))
    {
        if( IOCHANNEL_MODEIS_R_ONLY( self->mode ))
        {
            IOChannel_setError( self, IOCHANNELERROR_BMMFL );
            retVal = false;
            goto outLabel;
        }

        /* Truncate File */
        if( ftruncate( fd, 0 ) == -1 )
        {
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            retVal = false;
        }
        else
        {
            status = lseek( fd, length - 1, SEEK_SET );
            if( status == -1 )
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
                retVal = false;
                goto outLabel;
            }

            status = write( fd, "", 1 );
            if( status == -1 )
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
                retVal = false;
                goto outLabel;
            }
        }
    }

    /* ...Mapping Fd... */
    ptr = mmap( startAddress, length, protection, flags, fd, offset );
    if( ptr == (void *)-1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
        retVal = false;
        goto outLabel;
    }
    else
    {
        IOChannelGenericMem_setPtr( self, ptr, fd, length, true );
        retVal = true;
    }

    outLabel:
#endif

    return retVal;
}


bool IOChannelGenericMem_unmapFd( IOChannel *self )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The munmap() is not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    int status = -1;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( !IOCHANNEL_MODEIS_R_ONLY( self->mode ))
    {
        ANY_REQUIRE( streamPtr->fd != -1 );

        status = ftruncate( streamPtr->fd, self->currentIndexPosition );
        if( status == -1 )
        {
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            retVal = false;
            goto outLabel;
        }
    }

    status = munmap( streamPtr->ptr, streamPtr->size );

    if( status == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
        retVal = false;
    }
    else
    {
        retVal = true;
    }

    outLabel:
#endif
    return retVal;
}


void IOChannelGenericMem_clear( IOChannel *self )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->ptr = (void *)NULL;
}


void IOChannelGenericMem_delete( IOChannel *self )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


#if !defined(__windows__)


static bool IOChannel_setProtection( IOChannel *self, int *protection )
{
    bool retVal = true;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid );

    /* Modes must be compatible with prewious fd mode definition */
    switch( IOCHANNEL_GET_ACCESS_MODE( self->mode ))
    {
        case IOCHANNEL_MODE_R_ONLY:
            *protection = PROT_READ;
            break;

        case IOCHANNEL_MODE_W_ONLY:
            *protection = PROT_WRITE;
            break;

        case IOCHANNEL_MODE_RW:
            *protection = ( PROT_READ | PROT_WRITE );
            break;

        default :
            IOChannel_setError( self, IOCHANNELERROR_BMODE );
            retVal = false;
            break;
    }

    return retVal;
}


#endif


static long long IOChannelGenericMem_seekBack( IOChannel *self, long long offset )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long retVal = -1;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    if( self->currentIndexPosition + offset < 0 )
    {
        IOChannel_setError( self, IOCHANNELERROR_EOVERFLOW );
        retVal = -1;
    }
    else
    {
        if( ungetBuffer->index != 0 )
        {
            offset -= ungetBuffer->index;
        }
        ungetBuffer->index = 0;
        self->currentIndexPosition += offset;
        retVal = self->currentIndexPosition;
    }

    return retVal;
}


static long long IOChannelGenericMem_seekForward( IOChannel *self, long long offset )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long retVal = -1;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    if(( self->currentIndexPosition + offset ) > streamPtr->size )
    {
        IOChannel_setError( self, IOCHANNELERROR_EOVERFLOW );
        retVal = -1;
    }
    else
    {
        if( ungetBuffer->index != 0 )
        {
            offset -= ungetBuffer->index;
        }
        ungetBuffer->index = 0;
        self->currentIndexPosition += offset;
        retVal = self->currentIndexPosition;
    }

    return retVal;
}


/* EOF */
