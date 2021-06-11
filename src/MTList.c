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


#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <MTList.h>


MTListElement *MTList_searchListElement( MTList *self, int (*cmpFunc)( void *, void * ), void *searched );

static MTListElement *MTListElement_new( void );

static bool MTListElement_init( MTListElement *self, void *data );

static void MTListElement_clear( MTListElement *self );

static void MTListElement_delete( MTListElement *self );


/* MTList */

MTList *MTList_new( void )
{
    MTList *self = NULL;

    self = ANY_TALLOC( MTList );
    ANY_REQUIRE( self );

    return self;
}


bool MTList_init( MTList *self )
{
    bool retVal = true;
    ANY_REQUIRE( self );
    self->valid = MTLIST_INVALID;

    /* unreachable code: RWLock_init() always returns true
     *
    if ( RWLock_init ( self->rwlock, RWLOCK_PRIVATE ) == false )
    {
      retVal = false;
      goto out;
    }
     * changed to:
     */
    self->rwlock = RWLock_new();
    ANY_REQUIRE_MSG( self->rwlock, "unable to allocate self->rwlock" );
    RWLock_init( self->rwlock, RWLOCK_PRIVATE );

    /* initialize some list's fields */
    self->first = NULL;
    self->last = NULL;
    self->numElement = 0L;
    self->deleteMode = MTLIST_DELETEMODE_AUTOMATIC;

    self->valid = MTLIST_VALID;

//  out:

    return retVal;
}


void MTList_setDeleteMode( MTList *self, int deleteMode )
{
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    status = RWLock_writeLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    self->deleteMode = deleteMode;

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );
}


bool MTList_add( MTList *self, void *data )
{
    int status = 0;
    bool ret = false;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* create a new element */
    ptr = MTListElement_new();

    /* check for the creation */
    if( ptr )
    {
        /* init the element */
        if( MTListElement_init( ptr, data ) == true )
        {
            status = RWLock_writeLock( self->rwlock );
            ANY_REQUIRE( status == 0 );

            /* we have some element ? */
            if( self->numElement )
            {
                /* add a new element in tail */
                self->last->next = ptr;
            }
            else
            {
                self->first = ptr;
            }

            self->last = ptr;

            /* increase the element counter */
            self->numElement++;

            /* unlock the object */
            status = RWLock_unlock( self->rwlock );
            ANY_REQUIRE( status == 0 );

            ret = true;
        }
        else
        {
            ANY_LOG( 0, "Error in initialize new element", ANY_LOG_ERROR );
        }
    }
    else
    {
        ANY_LOG( 0, "Error in allocating new element", ANY_LOG_ERROR );
    }

    return ( ret );
}


bool MTList_insert( MTList *self, void *data )
{
    int status = 0;
    bool retVal = false;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* create a new element */
    ptr = MTListElement_new();

    /* check for the creation */
    if( ptr )
    {
        /* init the element */

        /* unreachable code: MTListElement_init() always returns true
         */
//     if ( MTListElement_init( ptr, data ) == true )
//     {
//       status = RWLock_writeLock( self->rwlock );
//       ANY_REQUIRE( status == 0 );
//
//       /* the brand new element refers to the current first */
//       ptr->next = self->first;
//
//       /* now the new first is the brand new element */
//       self->first = ptr;
//
//       if ( self->last == (MTListElement*)NULL )
//       {
//         self->last = ptr;
//       }
//
//       /* increase the element counter */
//       self->numElement++;
//
//       /* unlock the object */
//       status = RWLock_unlock( self->rwlock );
//       ANY_REQUIRE( status == 0 );
//
//       retVal = true;
//     }
//     else
//     {
//       ANY_LOG( 0, "Error in initialize new element", ANY_LOG_ERROR );
//     }
        /* changed to: */
        MTListElement_init( ptr, data );

        status = RWLock_writeLock( self->rwlock );
        ANY_REQUIRE( status == 0 );

        /* the brand new element refers to the current first */
        ptr->next = self->first;

        /* now the new first is the brand new element */
        self->first = ptr;

        if( self->last == (MTListElement *)NULL)
        {
            self->last = ptr;
        }

        /* increase the element counter */
        self->numElement++;

        /* unlock the object */
        status = RWLock_unlock( self->rwlock );
        ANY_REQUIRE( status == 0 );

        retVal = true;
    }
    else
    {
        ANY_LOG( 0, "Error in allocating new element", ANY_LOG_ERROR );
    }

    /*
     * effectively always returns 'true' because the ANY_REQUIRE in
     * MTListElement_new() leads to that always a valid pointer is
     * returned
     *
     * thus our retVal will never be 'false'
     */
    return retVal;
}


bool MTList_remove( MTList *self, int (*cmpFunc)( void *, void * ), void *searched )
{
    int status = 0;
    int retCmpFunc = 0;
    bool result = false;
    MTListElement *ptr = NULL;
    MTListElement **ptrNext = NULL;
    MTListElement *oldPtr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );
    ANY_REQUIRE( cmpFunc );
    ANY_OPTIONAL( searched );

    /* lock the list for write */
    status = RWLock_writeLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* start from the first element */
    ptrNext = &( self->first );
    oldPtr = NULL;
    ptr = self->first;

    while( ptr )
    {
        /* lock the element for read before to call the cmpFunc() */
        status = RWLock_readLock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /*
         * Call the comparison function. The cmpFunc should
         * return -1, 0 or greater than 0 if the comparison it's less, equal or greater
         * of the searched element
         */
        retCmpFunc = ( *cmpFunc )( ptr->data, searched );

        /* unlock the element */
        status = RWLock_unlock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /* we have found the element ? */
        if( retCmpFunc == 0 )
        {
            *ptrNext = ptr->next;

            if( ptr == self->last )
            {
                self->last = oldPtr;

                if( oldPtr )
                {
                    oldPtr->next = NULL;
                }
            }

            if( ptr->data && self->deleteMode == MTLIST_DELETEMODE_AUTOMATIC )
            {
                ANY_FREE( ptr->data );
            }

            MTListElement_clear( ptr );
            MTListElement_delete( ptr );

            self->numElement--;
            result = true;

            break;
        }

        /* next element */
        ptrNext = &( ptr->next );
        oldPtr = ptr;
        ptr = ptr->next;
    }

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /*
     * if we arrive at the and of the list we always have ptr == NULL
     * than we'll return NULL as expected for a not found element
     */
    return ( result );
}


bool MTList_set( MTList *self, void *searched, void *newData )
{
    int status = 0;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* start from the first element */
    ptr = self->first;

    while( ptr )
    {
        /* lock the element for read before to call the cmpFunc() */
        status = RWLock_readLock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /*
         * Call the comparison function. The cmpFunc should
         * return -1, 0 or greater than 0 if the comparison it's less, equal or greater
         * of the searched element
         */
        if( ptr->data == searched )
        {
            ptr->data = newData;

            /* unlock the element */
            status = RWLock_unlock( ptr->rwlock );
            ANY_REQUIRE( status == 0 );
            break;
        }

        /* unlock the element */
        status = RWLock_unlock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /* next element */
        ptr = ptr->next;
    }

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /*
     * if we arrive at the and of the list we always have ptr == NULL
     * than we'll return NULL as expected for a not found element
     */
    return ( ptr ? true : false );
}


void *MTList_iterate( MTList *self, int (*func)( void * ), int flags )
{
    int status = 0;
    int retFunc = 0;
    MTListElement *ptr = NULL;
    int (*lockFunc)( RWLock * ) = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );
    ANY_REQUIRE( func );

    /*
     * setup the locking function. In this way we'll save some
     * cpu cicle for check and speed up the iteration
     */
    if( flags & MTLIST_ITERATE_FOR_READ )
    {
        lockFunc = RWLock_readLock;
    }

    if( flags & MTLIST_ITERATE_FOR_WRITE )
    {
        lockFunc = RWLock_writeLock;
    }

    /* the lockFunc definition is required otherwise we fail */
    ANY_REQUIRE( lockFunc );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* start from the first element */
    ptr = self->first;

    while( ptr )
    {
        /* lock as flags request the element before to call the func() */
        status = ( *lockFunc )( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /*
          * Call the generic function. The func should return 0 to stop the
          * iteration or not zero to continue
          */
        retFunc = ( *func )( ptr->data );

        /* unlock the element */
        status = RWLock_unlock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /* we had to stop ? */
        if( retFunc == 0 )
        {
            break;
        }

        /* next element */
        ptr = ptr->next;
    }

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    return ( ptr ? ptr->data : NULL);
}


void *MTList_search( MTList *self, int (*cmpFunc)( void *, void * ), void *searched )
{
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );
    ANY_REQUIRE( cmpFunc );

    ptr = MTList_searchListElement( self, cmpFunc, searched );

    /*
     * if we arrive at the and of the list we always have ptr == NULL
     * than we'll return NULL as expected for a not found element
     */
    return ( ptr ? ptr->data : NULL);
}


bool MTList_isPresent( MTList *self, void *searched )
{
    bool result = false;
    int status = 0;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* we start from then first element */
    ptr = self->first;

    /* and we loop until we have element in list */
    while( ptr )
    {
        ANY_REQUIRE( ptr->valid == MTLISTELEMENT_VALID );

        if( ptr->data == searched )
        {
            result = true;
            break;
        }

        ptr = ptr->next;
    }

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    return ( result );
}


long MTList_numElements( MTList *self )
{
    int status = 0;
    long num = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    num = self->numElement;

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    return ( num );
}


MTListElement *MTList_searchListElement( MTList *self, int (*cmpFunc)( void *, void * ), void *searched )
{
    int status = 0;
    int retCmpFunc = 0;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );
    ANY_REQUIRE( cmpFunc );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* start from the first element */
    ptr = self->first;

    while( ptr )
    {
        /* lock the element for read before to call the cmpFunc() */
        status = RWLock_readLock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /*
         * Call the comparison function. The cmpFunc should
         * return -1, 0 or greater than 0 if the comparison it's less, equal or greater
         * of the searched element
         */
        retCmpFunc = ( *cmpFunc )( ptr->data, searched );

        /* unlock the element */
        status = RWLock_unlock( ptr->rwlock );
        ANY_REQUIRE( status == 0 );

        /* we have found the element ? */
        if( retCmpFunc == 0 )
        {
            break;
        }

        /* next element */
        ptr = ptr->next;
    }

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /*
     * if we arrive at the and of the list we always have ptr == NULL
     * than we'll return NULL as expected for a not found element
     */
    return ( ptr );
}


MTListElement *MTList_getNextElement( MTList *self, MTListElement *current )
{
    MTListElement *next = NULL;
    int status = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    /* lock the list for read */
    status = RWLock_readLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    if( !self->numElement )
    {
        goto out;
    }

    if( current )
    {
        ANY_REQUIRE( current->valid == MTLISTELEMENT_VALID );

        /* lock the element for read before to call the cmpFunc() */
        status = RWLock_readLock( current->rwlock );
        ANY_REQUIRE( status == 0 );

        next = current->next;

        status = RWLock_unlock( current->rwlock );
        ANY_REQUIRE( status == 0 );
    }
    else
    {
        next = self->first;
    }

    out:

    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    return ( next );
}


MTListElement *MTList_getNextElementNoLock( MTList *self, MTListElement *current )
{
    MTListElement *next = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    if( !self->numElement )
    {
        goto out;
    }

    if( current )
    {
        ANY_REQUIRE( current->valid == MTLISTELEMENT_VALID );
        next = current->next;
    }
    else
    {
        next = self->first;
    }

    out:

    return ( next );
}


void *MTList_getElementData( MTList *self, MTListElement *element )
{
    void *retVal = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    if( element )
    {
        ANY_REQUIRE( element->valid == MTLISTELEMENT_VALID );
        retVal = element->data;
    }

    return retVal;
}


void MTList_clear( MTList *self )
{
    int status = 0;
    MTListElement *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLIST_VALID );

    status = RWLock_writeLock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* check if we must clear some list element */
    if( self->numElement )
    {
        /* we start from then first element */
        ptr = self->first;

        /* and we loop until we have element in list */
        while( ptr )
        {
            ANY_REQUIRE ( ptr->valid == MTLISTELEMENT_VALID );

            /* the first in list is the next one */
            self->first = ptr->next;

            if( ptr->data && self->deleteMode == MTLIST_DELETEMODE_AUTOMATIC )
            {
                ANY_FREE ( ptr->data );
            }

            ptr->data = NULL;

            /* clear & delete the element */
            MTListElement_clear( ptr );
            MTListElement_delete( ptr );

            self->numElement--;

            ptr = self->first;
        }
    }

    /* sanity check */
    ANY_REQUIRE( self->numElement == 0 && self->first == (MTListElement *)NULL );

    /* only for security */
    self->last = self->first;

    /* unlock the object */
    status = RWLock_unlock( self->rwlock );
    ANY_REQUIRE( status == 0 );

    /* destroy the rwlock */
    RWLock_clear( self->rwlock );
    RWLock_delete( self->rwlock );

    /* now the object is invalid */
    self->valid = MTLIST_INVALID;
}


void MTList_delete( MTList *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* MTListElement implementation */

static MTListElement *MTListElement_new( void )
{
    MTListElement *self = NULL;

    self = ANY_TALLOC( MTListElement );
    ANY_REQUIRE( self );

    return self;
}


static bool MTListElement_init( MTListElement *self, void *data )
{
    bool retVal = true;

    ANY_REQUIRE( self );
    ANY_REQUIRE( data );

    self->valid = MTLISTELEMENT_INVALID;

    /* unreachable code: RWLock_init() always returns true
     *
    if ( RWLock_init ( &(self->rwlock), RWLOCK_PRIVATE ) == false )
    {
      retVal = false;
      goto out;
    }
     * changed to:
     */
    self->rwlock = RWLock_new();
    RWLock_init( self->rwlock, RWLOCK_PRIVATE );

    /* initialize some fields */
    self->flags = 0L;
    self->next = NULL;
    self->data = data;

    self->valid = MTLISTELEMENT_VALID;

    /* unreachable code: labels are never used
     *
   out:
     */

    return retVal;
}


static void MTListElement_clear( MTListElement *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTLISTELEMENT_VALID );

    RWLock_clear( self->rwlock );
    RWLock_delete( self->rwlock );

    self->flags = 0L;
    self->next = NULL;

    self->valid = MTLISTELEMENT_INVALID;
}


static void MTListElement_delete( MTListElement *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* EOF */
