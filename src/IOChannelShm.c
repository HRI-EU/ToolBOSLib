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


#include <IOChannelGenericMem.h>
#include <IOChannelReferenceValue.h>

#if !defined(__windows__)

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/mman.h>

#endif


IOCHANNELINTERFACE_CREATE_PLUGIN( Shm );


#if !defined(__windows__)

static bool IOChannelShm_openByKey( IOChannel *self, IOChannelMode mode,
                                    IOChannelPermissions permissions,
                                    key_t key, long size );

static bool IOChannelShm_openByName( IOChannel *self, char *name,
                                     IOChannelMode mode,
                                     IOChannelPermissions permissions,
                                     long size );

#endif


static void *IOChannelShm_new( void )
{
    return IOChannelGenericMem_new();
}


static bool IOChannelShm_init( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannel_valid( self );

    return IOChannelGenericMem_init( self );
}


static bool IOChannelShm_open( IOChannel *self, char *infoString,
                               IOChannelMode mode,
                               IOChannelPermissions permissions, va_list varArg )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The shm_xxx() are not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    IOChannelReferenceValue **vect = (IOChannelReferenceValue **)NULL;
    key_t key;
    long size = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( infoString );

    IOChannel_valid( self );

    IOCHANNELREFERENCEVALUE_BEGINSET( &vect )

    if( *infoString == '\0' )
    {
        /* If no options are defined -> open a shared using key -> getting key */
        IOCHANNEL_GET_ARGUMENT( key, key_t, varArg );
        IOCHANNELREFERENCEVALUE_ADDSET( key, "%ld", (long)key );
    }
    else
    {
        /* If option was defined -> it is a named shared */
        IOCHANNELREFERENCEVALUE_ADDSET( name, "%s", infoString );
    }
    /* getting size */
    IOCHANNEL_GET_ARGUMENT( size, long, varArg );
    IOCHANNELREFERENCEVALUE_ADDSET( size, "%ld", size );

    IOCHANNELREFERENCEVALUE_ENDSET( &vect );

    retVal = IOChannelShm_openFromString( self, vect );

    IOCHANNELREFERENCEVALUE_FREESET( &vect );

#endif

    return retVal;
}


static bool IOChannelShm_openFromString( IOChannel *self,
                                         IOChannelReferenceValue **referenceVector )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The shm_xxx() are not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    char *name = (char *)NULL;
    char patchBuffer[IOCHANNEL_INFOSTRING_MAXLEN];
    key_t key;
    long size = 0;
    char *infoString = (char *)NULL;
    IOChannelMode mode = self->mode;
    IOChannelPermissions permissions = 0;
    char *value = (char *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( referenceVector );

    IOChannel_valid( self );

    infoString = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_NAME );

    if( IOCHANNEL_MODEIS_DEFINED( mode ) == false )
    {
        mode = IOCHANNEL_MODE_RW;
        self->mode = mode;
    }

    value = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_PERM );
    if( value )
    {
        permissions = IOChannelReferenceValue_getAccessPermissions( value );
        value = (char *)NULL;
    }
    else
    {
        permissions = IOCHANNEL_PERMISSIONS_ALL;
    }

    if( !infoString )
    {
        /* No name is defined: open a shared using key */
        key = (key_t)IOChannelReferenceValue_getLong( referenceVector, IOCHANNELREFERENCEVALUE_KEY );
        size = IOChannelReferenceValue_getULong( referenceVector, IOCHANNELREFERENCEVALUE_SIZE );

        retVal = IOChannelShm_openByKey( self, mode, permissions, key, size );
    }
    else
    {
        /* If name was found in referenceVector, it is a named shared */
        name = infoString;
        size = IOChannelReferenceValue_getULong( referenceVector, IOCHANNELREFERENCEVALUE_SIZE );

        /* If a name was defined and a key is found, maybe an error has occurred */
        value = IOChannelReferenceValue_getString( referenceVector, IOCHANNELREFERENCEVALUE_KEY );
        if( value )
        {
            IOChannel_setError( self, IOCHANNELERROR_BMMFL );
            ANY_LOG( 5, "Warning, found a name and a key. Only one of these references is allowed.", ANY_LOG_WARNING );
            goto outLabel;
        }

        if( *name == '/' )
        {
            retVal = IOChannelShm_openByName( self, name, mode, permissions, size );
        }
        else
        {
            snprintf( patchBuffer, IOCHANNEL_INFOSTRING_MAXLEN, "/%s", infoString );

            retVal = IOChannelShm_openByName( self, patchBuffer, mode, permissions, size );
        }
    }

    outLabel:;

#endif

    return retVal;
}


static long IOChannelShm_read( IOChannel *self, void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE_MSG( buffer, "IOChannelShm_read(). Not valid buffer" );
    ANY_REQUIRE_MSG( size > 0, "IOChannelShm_read(). Size must be a positive number" );

    return IOChannelGenericMem_read( self, buffer, size );
}


static long IOChannelShm_write( IOChannel *self, const void *buffer, long size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE_MSG( buffer, "IOChannelShm_write(). Not valid buffer" );
    ANY_REQUIRE_MSG( size > 0, "IOChannelShm_write(). Size must be a positive number" );

    return IOChannelGenericMem_write( self, buffer, size );
}


static long IOChannelShm_flush( IOChannel *self )
{
    ANY_REQUIRE( self );

    return IOChannelGenericMem_flush( self );
}


static long long IOChannelShm_seek( IOChannel *self, long long offset, IOChannelWhence whence )
{
    ANY_REQUIRE( self );

    return IOChannelGenericMem_seek( self, offset, whence );
}


static bool IOChannelShm_close( IOChannel *self )
{
    bool retVal = false;

#if defined(__windows__)

    ANY_LOG( 1, "The shm_xxx() are not available on windows at moment", ANY_LOG_WARNING );
    IOChannel_setError( self, IOCHANNELERROR_ENOTSUP );

#else

    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    int status = 0;

    ANY_REQUIRE( self );

    streamPtr = IOChannel_getStreamPtr( self );

    if( !IOCHANNEL_MODEIS_NOTCLOSE( self->mode ))
    {
        if( streamPtr->isMapped )
        {
            /* This is the converse operation of shm_open-openByName  */
            retVal = IOChannelGenericMem_unmapFd( self );

            if( retVal )
            {
                status = close( streamPtr->fd );
                if( status == -1 )
                {
                    retVal = false;
                    IOCHANNEL_SETSYSERRORFROMERRNO( self );
                }
            }
        }
        else
        {
            /* This is the converse operation of shmget-openByKey */
            status = shmdt( streamPtr->ptr );
            if( status == -1 )
            {
                IOCHANNEL_SETSYSERRORFROMERRNO( self );
                retVal = false;
            }
            else
            {
                retVal = true;
            }
        }
    }

#endif

    return retVal;
}


static void *IOChannelShm_getProperty( IOChannel *self, const char *propertyName )
{
    IOChannelGenericMem *streamPtr = (IOChannelGenericMem *)NULL;
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( propertyName );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    streamPtr = IOChannel_getStreamPtr( self );
    ANY_REQUIRE( streamPtr );

    IOCHANNELPROPERTY_START
    {
        IOCHANNELPROPERTY_PARSE_BEGIN( MemPointer )
        {
            retVal = streamPtr->ptr;
        }
        IOCHANNELPROPERTY_PARSE_END( MemPointer )
    }
    IOCHANNELPROPERTY_END;


    if( !retVal )
    {
        ANY_LOG( 7, "Property '%s' not set or not defined for this stream",
                 ANY_LOG_WARNING, propertyName );
    }

    return retVal;
}


static bool IOChannelShm_setProperty( IOChannel *self, const char *propertyName,
                                      void *property )
{
    return false;
}


static void IOChannelShm_clear( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericMem_clear( self );
}


static void IOChannelShm_delete( IOChannel *self )
{
    ANY_REQUIRE( self );

    IOChannelGenericMem_delete( self );
}


#if !defined(__windows__)

static bool IOChannelShm_openByKey( IOChannel *self,
                                    IOChannelMode mode,
                                    IOChannelPermissions permissions,
                                    key_t key, long size )
{
    int shmAtFlags = 0;
    int shmGetFlags = 0;
    int shmId = 0;
    bool retVal = false;
    void *shmPtr = (void *)NULL;

    ANY_REQUIRE( self );
    IOChannel_valid( self );

    if( size <= 0 )
    {
        IOChannel_setError( self, IOCHANNELERROR_BSIZE );
        retVal = false;
        goto outLabel;
    }

    ANY_TRACE( 4, "%d", key );
    ANY_TRACE( 4, "%ld", size );
    IOChannel_logMode( 4, mode );
    IOChannel_logPermission( 4, permissions );

    if( IOCHANNEL_MODEIS_CREAT( mode ) || IOCHANNEL_MODEIS_TRUNC( mode ))
    {
        shmGetFlags = IPC_CREAT;
    }
    else
    {
        shmGetFlags = 0;
    }

    shmGetFlags = shmGetFlags | permissions;
    shmId = shmget( key, size, shmGetFlags );
    if( shmId == -1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
        retVal = false;
        goto outLabel;
    }


    switch( IOCHANNEL_GET_ACCESS_MODE( self->mode ))
    {
        case IOCHANNEL_MODE_R_ONLY:
            shmAtFlags = SHM_RDONLY;
            break;
            /* Write Only is emulated */
        case IOCHANNEL_MODE_W_ONLY:
        case IOCHANNEL_MODE_RW:
            shmAtFlags = 0;
            break;
        default :
            IOChannel_setError( self, IOCHANNELERROR_BMODE );
            retVal = false;
            goto outLabel;
    }

    shmPtr = shmat( shmId, NULL, shmAtFlags );
    if( shmPtr == (void *)-1 )
    {
        IOCHANNEL_SETSYSERRORFROMERRNO( self );
        retVal = false;
    }
    else
    {
        IOChannelGenericMem_setPtr( self, shmPtr, shmId, size, false );
        retVal = true;
    }

    outLabel:
    return retVal;
}


static bool IOChannelShm_openByName( IOChannel *self, char *name,
                                     IOChannelMode mode,
                                     IOChannelPermissions permissions,
                                     long size )
{
    bool retVal = false;
    int shmId = 0;

    ANY_REQUIRE( self );
    IOChannel_valid( self );
    ANY_REQUIRE_MSG( name, "IOChannelShm_openByName(). Not valid name" );

    if( size <= 0 )
    {
        ANY_LOG( 0, "You are trying to open a Named Shm, but the size parameter "
                         "is Null, zero or a negative value. Maybe you forget to "
                         "specify the shm size parameter in the IOChannel_open()",
                 ANY_LOG_WARNING );
    }
    else
    {
        ANY_TRACE( 4, "%s", name );
        ANY_TRACE( 4, "%ld", size );
        IOChannel_logMode( 4, mode );
        IOChannel_logPermission( 4, permissions );

        shmId = shm_open( name, mode, permissions );
        if( shmId == -1 )
        {
            ANY_LOG( 0, "Error while trying to open a Named Shm. "
                             "Shared memory object '%s' does not exists "
                             "or bad access flags were set for mode and permissions.",
                     ANY_LOG_WARNING, name );
            IOCHANNEL_SETSYSERRORFROMERRNO( self );
            retVal = false;
        }
        else
        {
            retVal = IOChannelGenericMem_mapFd( self, shmId, size );
        }
    }

    return retVal;
}

#endif


/* EOF */
