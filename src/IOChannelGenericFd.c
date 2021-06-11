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


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(__windows__)

#include <unistd.h>

#endif

#if defined(__windows__)

#if !defined(__mingw__)
#pragma warning( push )
#pragma warning( disable : 4005 )
#endif

#include <windows.h>

#if !defined(__mingw__)
#pragma warning( pop )
#endif

#include <time.h>
#include <io.h>
#include <direct.h>
static int ftruncate( int fd, off_t where );
static int internal_fstat( int fd, struct stat *buf );
#endif

#include <IOChannelGenericFd.h>


static long long IOChannelGenericFd_seekBack( IOChannel *self, long long offset );

static long long IOChannelGenericFd_seekForward( IOChannel *self, long long offset );


void *IOChannelGenericFd_new( void )
{
    IOChannelGenericFd *self = (IOChannelGenericFd *)NULL;

    self = ANY_TALLOC( IOChannelGenericFd );

    ANY_REQUIRE( self );

    return self;
}


bool IOChannelGenericFd_init( IOChannel *self )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Don' use, IOChannelGenericFd_setFd, which checks fd validity! */
    streamPtr->fd = -1;
    streamPtr->isRegularFile = false;

    return true;
}


bool IOChannelGenericFd_setFd( IOChannel *self, int fd )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    bool retVal = false;
    struct stat st;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    if( fd < 0 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    else
    {
#if !defined(__msvc__)
        if( fstat( fd, &st ) != 0 )
#else
            if( internal_fstat( fd, &st ) != 0 )
#endif
        {
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
        }
        else
        {
            IOChannel_setType( self, IOCHANNELTYPE_FD );
            streamPtr->fd = fd;
            /* Get info about fd */
#if !defined(__windows__)
            streamPtr->isRegularFile = S_ISREG( st.st_mode );
#else
            streamPtr->isRegularFile = true;
#endif
        }
        retVal = true;
    }
    return retVal;
}


int IOChannelGenericFd_getFd( IOChannel *self )
{
    int retVal = -1;
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = streamPtr->fd;

    return retVal;
}


int *IOChannelGenericFd_getFdPtr( IOChannel *self )
{
    int *retVal = (int *)NULL;
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    retVal = &( streamPtr->fd );

    return retVal;
}


bool IOChannelGenericFd_unSet( IOChannel *self )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    bool retVal = true;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->fd = -1;

    return retVal;
}


long IOChannelGenericFd_read( IOChannel *self, void *buffer, long size )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size >= 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

#if !defined(__windows__)
    retVal = read( streamPtr->fd, buffer, size );
#else
    retVal = _read( streamPtr->fd, buffer, size );
#endif

    if( retVal == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }

    if( retVal < size )
    {
        IOCHANNEL_SET_EOF( self );
    }
    return retVal;
}


long IOChannelGenericFd_write( IOChannel *self, const void *buffer, long size )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    long retVal = -1;

    ANY_REQUIRE( self );
    ANY_REQUIRE( buffer );
    ANY_REQUIRE( size >= 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

#if !defined(__windows__)
    retVal = write( streamPtr->fd, buffer, size );
#else
    retVal = _write( streamPtr->fd, buffer, size );
#endif

    if( retVal == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }

    if( retVal < size )
    {
        IOChannel_setError( self, IOCHANNELERROR_BLLW );
    }

    return retVal;
}


/*
 * Each stream can implement its proper seek: high level seek returns exactly
 * the return value of low level seek. This means that low level seek must
 * manage:
 *
 * - currentIndexPosition repositioning
 * - seek return value
 * - internalUngetBufferIndex resositioning.
 *
 * Remember that IOChannel_read first reads from ungetBuffer if there are
 * ungetted bytes, and then from real stream.
 */
long long IOChannelGenericFd_seek( IOChannel *self,
                                   long long offset,
                                   IOChannelWhence whence )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long retVal = -1;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    /* Not all fd's refers to a regular file */
    if( streamPtr->isRegularFile == false )
    {
        ANY_LOG( 5, "Seek has not effect if fd isn't a regular file...",
                 ANY_LOG_WARNING );
        retVal = 0;
        goto outLabel;
    }

    switch( whence )
    {
        case IOCHANNELWHENCE_SET:
        case IOCHANNELWHENCE_END:
        {
            streamPtr = IOChannel_getStreamPtr( self );
            ANY_REQUIRE( streamPtr );

            ungetBuffer = self->ungetBuffer;
            ANY_REQUIRE( ungetBuffer );

#if !defined(__windows__)
            retVal = lseek( streamPtr->fd, offset, whence );
#else
            retVal = _lseeki64( streamPtr->fd, offset, whence );
#endif

            if( retVal != -1 )
            {
                ungetBuffer->index = 0;
                self->currentIndexPosition = retVal;
            }
            else
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
            }
            break;
        }

        case IOCHANNELWHENCE_CUR:
        {
            if( offset == 0 )
            {
                retVal = self->currentIndexPosition;
            }
            else if( offset < 0 )
            {
                retVal = IOChannelGenericFd_seekBack( self, offset );
            }
            else
            {
                retVal = IOChannelGenericFd_seekForward( self, offset );
            }
            break;
        }

        default:
            IOChannel_setError( self, IOCHANNELERROR_BWHESEK );
            retVal = -1;
            break;
    }

    outLabel:
    return retVal;
}


bool IOChannelGenericFd_truncate( IOChannel *self, long size )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    bool retVal = false;
    int status = -1;

    ANY_REQUIRE( size >= 0 );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    status = ftruncate( streamPtr->fd, size );

    /*
     * windows has _chsize_s() as ftruncate() replacement
     * but mingw doesn't have it
     *
     * status = _chsize_s( streamPtr->fd, (__int64)size );
     */

    if( status == -1 )
    {
        retVal = false;
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


bool IOChannelGenericFd_close( IOChannel *self )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    bool retVal = false;
    int status = -1;


    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

#if !defined(__windows__)
    status = close( streamPtr->fd );
#else
    status = _close( streamPtr->fd );
#endif

    if( status == -1 )
    {
        retVal = false;
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


void IOChannelGenericFd_clear( IOChannel *self )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;


    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr->fd = -1;
}


void IOChannelGenericFd_delete( IOChannel *self )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ANY_FREE( streamPtr );
}


static long long IOChannelGenericFd_seekBack( IOChannel *self, long long offset )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long newOffset = 0;
    long long retVal = -1;
    /* Note: offset has a negative value */

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    newOffset = offset - ungetBuffer->index;

#if !defined(__windows__)
    retVal = lseek( streamPtr->fd, newOffset, SEEK_CUR );
#else
    retVal = _lseeki64( streamPtr->fd, newOffset, SEEK_CUR );
#endif

    if( retVal == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
    }
    ungetBuffer->index = 0;

    return retVal;
}


static long long IOChannelGenericFd_seekForward( IOChannel *self, long long offset )
{
    IOChannelGenericFd *streamPtr = (IOChannelGenericFd *)NULL;
    IOChannelBuffer *ungetBuffer = (IOChannelBuffer *)NULL;
    long long newOffset = 0;
    long long retVal = -1;
    /* Note: offset has a positive value */

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    ungetBuffer = self->ungetBuffer;
    ANY_REQUIRE( ungetBuffer );

    if( offset < ungetBuffer->index )
    {
        /* seek into the buffer */
        ungetBuffer->index -= offset;
        /* must re-align currentIndexPosition with fd offset */
        retVal = self->currentIndexPosition - ungetBuffer->index;
    }
    else
    {
        newOffset = offset - ungetBuffer->index;
        if( newOffset == 0 )
        {
            retVal = self->currentIndexPosition;
        }
        else
        {

#if !defined(__windows__)
            retVal = lseek( streamPtr->fd, newOffset, SEEK_CUR );
#else
            retVal = _lseeki64( streamPtr->fd, newOffset, SEEK_CUR );
#endif

            if( retVal == -1 )
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
            }
        }
    }

    return retVal;
}


#if defined(__windows__)
static int ftruncate( int fd, off_t where )
{
  if ( _lseeki64( fd, where, SEEK_SET ) < 0 )
  {
    return( -1 );
  }

  if ( !SetEndOfFile( (HANDLE)_get_osfhandle( fd ) ) )
  {
    return( -1 );
  }

  return( 0 );
}

static time_t filetime_to_time_t( const FILETIME *ft )
{
  long long winTime = ((long long)ft->dwHighDateTime << 32) + ft->dwLowDateTime;

  winTime -= 116444736000000000LL; /* Windows to Unix Epoch conversion */
  winTime /= 10000000;       /* Nano to seconds resolution */

  return (time_t)winTime;
}

static int internal_fstat( int fd, struct stat *buf )
{
  HANDLE fh;
  BY_HANDLE_FILE_INFORMATION fdata;
  DWORD fileType = FILE_TYPE_UNKNOWN;

  fh = (HANDLE)_get_osfhandle( fd );

  if ( fh == INVALID_HANDLE_VALUE )
  {
    errno = EBADF;
    return -1;
  }

  /* direct non-file handles to MS's fstat() */
  fileType = GetFileType( fh );

  if ( fileType != FILE_TYPE_DISK && fileType != FILE_TYPE_CHAR )
  {
    return -1;
  }

  if ( fileType == FILE_TYPE_DISK && GetFileInformationByHandle( fh, &fdata ) )
  {
    int fMode = S_IREAD;

    if ( fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
      fMode |= S_IFDIR;
    }
    else
    {
      fMode |= S_IFREG;
    }

    if ( !( fdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) )
    {
      fMode |= S_IWRITE;
    }

    buf->st_ino = 0;
    buf->st_gid = 0;
    buf->st_uid = 0;
    buf->st_nlink = 1;
    buf->st_mode = fMode;
    buf->st_size = fdata.nFileSizeLow; /* Can't use nFileSizeHigh, since it's not a stat64 */
    buf->st_dev = buf->st_rdev = (_getdrive() - 1);
    buf->st_atime = filetime_to_time_t(&(fdata.ftLastAccessTime));
    buf->st_mtime = filetime_to_time_t(&(fdata.ftLastWriteTime));
    buf->st_ctime = filetime_to_time_t(&(fdata.ftCreationTime));

    return 0;
  }

  if ( fileType == FILE_TYPE_CHAR )
  {
    return 0;
  }


  errno = EBADF;

  return -1;
}


#endif


/* EOF */
