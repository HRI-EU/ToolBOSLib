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


#include <Any.h>
#include <MTQueue.h>

#define MTQUEUE_VALID      0x016e134c
#define MTQUEUE_INVALID    0x49d1602c


typedef struct MTQueueElement
{
    void *data;
    MTQueueUserClass userClass;
    struct MTQueueElement *next;
}
MTQueueElement;

struct MTQueue
{
    unsigned long valid;
    MTQueueType type;
    Mutex *lock;
    Cond *pushCond;
    unsigned long numElements;
    void *head;
    void *tail;
    bool quit;
};

static void MTQueue_lock( MTQueue *self );

static void MTQueue_unlock( MTQueue *self );

static void *MTQueue_unlockedPop( MTQueue *self, MTQueueUserClass *userClass );

static void MTQueue_signalPush( MTQueue *self );

static void MTQueue_addTail( MTQueue *self, void *data, MTQueueUserClass userClass );

static void MTQueue_addHead( MTQueue *self, void *data, MTQueueUserClass userClass );


MTQueue *MTQueue_new( void )
{
    MTQueue *self = ANY_TALLOC( MTQueue );

    return self;
}


int MTQueue_init( MTQueue *self, MTQueueType type, bool multiThread )
{
    ANY_REQUIRE( self );

    self->valid = MTQUEUE_INVALID;

    if( multiThread )
    {
        self->lock = Mutex_new();
        ANY_REQUIRE_MSG( self->lock, "Unable to allocate memory for a new Mutex" );
        Mutex_init( self->lock, MUTEX_PRIVATE );

        self->pushCond = Cond_new();
        ANY_REQUIRE_MSG( self->pushCond, "Unable to allocate memory for a new Cond" );
        Cond_init( self->pushCond, COND_PRIVATE );

        Cond_setMutex( self->pushCond, self->lock );
    }

    self->type = type;
    self->numElements = 0UL;
    self->head = NULL;
    self->tail = NULL;
    self->quit = false;

    self->valid = MTQUEUE_VALID;

    return 0;
}


void MTQueue_push( MTQueue *self, void *data, MTQueueUserClass userClass )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );
    ANY_REQUIRE( data );

    MTQueue_lock( self );

    switch( self->type )
    {
        /* FIFO queue must add new elements on tail */
        case MTQUEUE_FIFO:
            MTQueue_addTail( self, data, userClass );
            MTQueue_signalPush( self );
            break;

            /* FIFO queue must add new elements on head */
        case MTQUEUE_LIFO:
            MTQueue_addHead( self, data, userClass );
            MTQueue_signalPush( self );
            break;

        default:
            ANY_LOG( 0, "Invalid queue type '0x%08x'", ANY_LOG_ERROR, self->type );
            break;
    }

    MTQueue_unlock( self );
}


void *MTQueue_pop( MTQueue *self, MTQueueUserClass *userClass )
{
    void *retVal = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    MTQueue_lock( self );

    retVal = MTQueue_unlockedPop( self, userClass );

    MTQueue_unlock( self );

    return retVal;
}


void *MTQueue_popWait( MTQueue *self, MTQueueUserClass *userClass, const long microsecs )
{
    void *retVal = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    MTQueue_lock( self );

    if ( self->quit )
    {
        goto _quit;
    }

    /* wait until got an element */
    if( self->numElements == 0 )
    {
        ANY_LOG( 5, "No element found in the MTQueue, sleeping...", ANY_LOG_INFO );

        Cond_wait( self->pushCond, microsecs );

        ANY_LOG( 5, "Wakeup because a new element has been pushed in the MTQueue, checking ...", ANY_LOG_INFO );
    }

    retVal = MTQueue_unlockedPop( self, userClass );

 _quit:

    MTQueue_unlock( self );

    return retVal;
}


unsigned long MTQueue_numElements( MTQueue *self )
{
    unsigned long retVal = 0UL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    MTQueue_lock( self );

    retVal = self->numElements;

    MTQueue_unlock( self );

    return retVal;
}


void MTQueue_setQuit( MTQueue * self, bool status )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    MTQueue_lock( self );

    self->quit = status;

    MTQueue_unlock( self );
}


void MTQueue_wakeUpAll( MTQueue * self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    MTQueue_lock( self );

    Cond_broadcast( self->pushCond );

    MTQueue_unlock( self );
}


void MTQueue_clear( MTQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    /* empty the queue */
    while( MTQueue_pop( self, MTQUEUE_NOCLASS ));

    self->valid = MTQUEUE_INVALID;

    ANY_REQUIRE( self->numElements == 0 );

    if( self->lock )
    {
        Mutex_clear( self->lock );
        Mutex_delete( self->lock );
        self->lock = NULL;
    }

    if( self->pushCond )
    {
        Cond_clear( self->pushCond );
        Cond_delete( self->pushCond );
        self->pushCond = NULL;
    }
}


void MTQueue_delete( MTQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_INVALID );

    ANY_FREE( self );
}


static void *MTQueue_unlockedPop( MTQueue *self, MTQueueUserClass *userClass )
{
    MTQueueElement *e = NULL;
    void *retVal = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    if( self->numElements > 0 )
    {
        e = (MTQueueElement *)self->head;

        self->head = (void *)e->next;
        retVal = e->data;

        if( userClass )
        {
            *userClass = e->userClass;
        }

        ANY_FREE( e );

        self->numElements--;

        if( self->numElements == 0 )
        {
            ANY_REQUIRE( self->head == NULL );
            self->head = self->tail = NULL;
        }
    }

    return retVal;
}


static void MTQueue_lock( MTQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    if( self->lock )
    {
        int status = Mutex_lock( self->lock );
        ANY_REQUIRE( status == 0 );
    }
}


static void MTQueue_unlock( MTQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    if( self->lock )
    {
        int status = Mutex_unlock( self->lock );
        ANY_REQUIRE( status == 0 );
    }
}


static void MTQueue_signalPush( MTQueue *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );

    if( self->pushCond )
    {
        Cond_signal( self->pushCond );
    }
}


static void MTQueue_addHead( MTQueue *self, void *data, MTQueueUserClass userClass )
{
    MTQueueElement *e = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );
    ANY_REQUIRE( data );

    e = ANY_TALLOC( MTQueueElement );
    ANY_REQUIRE_MSG( e, "Unable to allocate memory for a new MTQueueElement" );

    e->data = data;
    e->userClass = userClass;

    if( self->head )
    {
        e->next = (MTQueueElement *)self->head;
        self->head = (void *)e;
    }
    else
    {
        e->next = NULL;
        self->head = (void *)e;
        self->tail = (void *)e;
    }

    self->numElements++;
}


static void MTQueue_addTail( MTQueue *self, void *data, MTQueueUserClass userClass )
{
    MTQueueElement *e = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == MTQUEUE_VALID );
    ANY_REQUIRE( data );

    e = ANY_TALLOC( MTQueueElement );
    ANY_REQUIRE_MSG( e, "Unable to allocate memory for a new MTQueueElement" );

    e->data = data;
    e->userClass = userClass;

    if( self->tail )
    {
        ((MTQueueElement *)self->tail )->next = e;
        self->tail = (void *)e;
    }
    else
    {
        e->next = NULL;
        self->head = (void *)e;
        self->tail = (void *)e;
    }

    self->numElements++;
}

/* EOF */
