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
#include <MTList.h>
#include <WorkQueue.h>


typedef struct WorkQueueTaskPool WorkQueueTaskPool;

struct WorkQueueTask
{
    unsigned long         valid;
    WorkQueueTaskFn       taskFn;
    WorkQueueTaskCallback callback;
    void                  *instance;
    void                  *userData;
    Mutex                 *mutex;
    Cond                  *taskTerminatedCond;
    bool terminated;
};

struct WorkQueueWorker
{
    unsigned long valid;
    Threads       *workerThread;
    WorkQueue     *parent;
    AnyAtomic     exit;
    AnyAtomic     busy;
};

struct WorkQueue
{
    unsigned long     valid;
    MTQueue           *tasks;
    WorkQueueTaskPool *taskPool;
    MTList            *workers;
    int               minWorkers;
    int               maxWorkers;
    Mutex             *mutex;
    Barrier           *workerTerminationBarrier;
    bool clearing;
    AnyAtomic         maxWorkersReached;
    AnyAtomic         freeWorkers;
};

struct WorkQueueTaskPool
{
    unsigned long valid;
    AnyAtomic     taskBalance;
    MTQueue       *tasks;
    MTList        *nonTerminatedTasks;
    MTQueue       *tempQueue;
};

#define WORKQUEUE_VALID            0x3da80c98
#define WORKQUEUE_INVALID          0x3f9588ed
#define WORKQUEUEWORKER_VALID      0xdcb10c66
#define WORKQUEUEWORKER_INVALID    0x2e8a98ac
#define WORKQUEUETASK_VALID        0x7a62caff
#define WORKQUEUETASK_INVALID      0x4a9df323
#define WORKQUEUETASKPOOL_VALID    0x601a8acf
#define WORKQUEUETASKPOOL_INVALID  0x7a6b05fd

#define WORKQUEUETASK_POP_TIMEOUT  200000

#define WORKQUEUE_MTQUEUE_CLASS     1
#define WORKQUEUE_TASKPOOL_INITIAL_SIZE 10

static WorkQueueWorker *WorkQueueWorker_new();

static bool WorkQueueWorker_init( WorkQueueWorker *self, WorkQueue *parent );

static void WorkQueueWorker_clear( WorkQueueWorker *self );

static void WorkQueueWorker_delete( WorkQueueWorker *self );

static void WorkQueueTask_signal( WorkQueueTask *self );

static WorkQueueTask *WorkQueueTask_new();

static void WorkQueueTask_clear( WorkQueueTask *self );

static void WorkQueueTask_delete( WorkQueueTask *self );

static WorkQueueTaskPool *WorkQueueTaskPool_new();

static bool WorkQueueTaskPool_init( WorkQueueTaskPool *self, unsigned int initialSize );

static void WorkQueueTaskPool_clear( WorkQueueTaskPool *self );

static void WorkQueueTaskPool_delete( WorkQueueTaskPool *self );

static WorkQueueTask *WorkQueueTaskPool_getTask( WorkQueueTaskPool *self );

static void WorkQueueTaskPool_disposeTask( WorkQueueTaskPool *self, WorkQueueTask *task );

static void WorkQueueTaskPool_refreshTasks( WorkQueueTaskPool *self );


WorkQueue *WorkQueue_new()
{
    return ANY_TALLOC( WorkQueue );
}


bool WorkQueue_init( WorkQueue *self, unsigned int minWorkers, unsigned int maxWorkers )
{
    unsigned int i;
    int          iRetVal;
    bool         bRetVal;

    ANY_REQUIRE( self );
    ANY_REQUIRE( maxWorkers == 0 || maxWorkers >= minWorkers );

    self->valid             = WORKQUEUE_INVALID;
    self->clearing          = false;
    self->maxWorkersReached = ( minWorkers == maxWorkers );
    self->freeWorkers       = 0;

    /* Create and init task queue */

    self->tasks = MTQueue_new();
    ANY_REQUIRE( self->tasks != NULL );

    iRetVal = MTQueue_init( self->tasks, MTQUEUE_FIFO, true );
    ANY_REQUIRE( iRetVal == 0 );

    /* Create and init task pool */
    self->taskPool = WorkQueueTaskPool_new();
    ANY_REQUIRE( self->taskPool );
    bRetVal = WorkQueueTaskPool_init( self->taskPool, WORKQUEUE_TASKPOOL_INITIAL_SIZE );
    ANY_REQUIRE( bRetVal );

    /* Create and init workers  */
    self->workers = MTList_new();
    ANY_REQUIRE( self->workers );
    bRetVal = MTList_init( self->workers );
    ANY_REQUIRE( bRetVal );
    MTList_setDeleteMode( self->workers, MTLIST_DELETEMODE_MANUAL );

    /* Create and init mutex  */
    self->mutex = Mutex_new();
    ANY_REQUIRE( self->mutex );
    bRetVal = Mutex_init( self->mutex, MUTEX_PRIVATE );
    ANY_REQUIRE( bRetVal );

    self->minWorkers = minWorkers;
    self->maxWorkers = maxWorkers;

    /* Create and init workers */
    for( i = 0; i < minWorkers; i++ )
    {
        WorkQueueWorker *worker;

        worker = WorkQueueWorker_new();
        ANY_REQUIRE( worker );

        bRetVal = WorkQueueWorker_init( worker, self );
        ANY_REQUIRE( bRetVal );
        MTList_add( self->workers, worker );
    }

    self->valid = WORKQUEUE_VALID;
    return true;
}


void WorkQueue_clear( WorkQueue *self )
{
    bool bRetVal;
    int  status;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUE_VALID );

    self->valid = WORKQUEUE_INVALID;

    self->clearing = true;

    /* Initialize barrier to wait for worker termination */

    self->workerTerminationBarrier = Barrier_new();
    ANY_REQUIRE( self->workerTerminationBarrier );
    bRetVal = Barrier_init( self->workerTerminationBarrier,
                            BARRIER_PRIVATE,
                            MTList_numElements( self->workers ) + 1,
                            NULL, NULL );
    ANY_REQUIRE( bRetVal );

    /* set workers->exit */
    MTLIST_FOREACH_BEGIN ( self->workers, MTLIST_ITERATE_FOR_READ )
                {
                    WorkQueueWorker *worker;
                    worker = (WorkQueueWorker *)MTLIST_FOREACH_ELEMENTPTR;
                    Atomic_set( &worker->exit, true );
                }
    MTLIST_FOREACH_END;

    /* Wake up all the workers */
    MTQueue_setQuit( self->tasks, true );
    MTQueue_wakeUpAll( self->tasks );

    /* Wait for all workers to terminate */
    Barrier_wait( self->workerTerminationBarrier );

    /* Wait for Barrier to be empty before clearing */
    while( !Barrier_isEmpty( self->workerTerminationBarrier ) )
    {
        Any_sleepMilliSeconds( 500 );
    }

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    MTLIST_FOREACH_BEGIN ( self->workers, MTLIST_ITERATE_FOR_READ )
                {
                    WorkQueueWorker *worker;
                    worker = (WorkQueueWorker *)MTLIST_FOREACH_ELEMENTPTR;
                    WorkQueueWorker_clear( worker );
                    WorkQueueWorker_delete( worker );
                }
    MTLIST_FOREACH_END;

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );

    /* cleanup */
    Barrier_clear( self->workerTerminationBarrier );
    Barrier_delete( self->workerTerminationBarrier );

    MTList_clear( self->workers );
    MTList_delete( self->workers );

    MTQueue_clear( self->tasks );
    MTQueue_delete( self->tasks );

    Mutex_clear( self->mutex );
    Mutex_delete( self->mutex );

    WorkQueueTaskPool_clear( self->taskPool );
    WorkQueueTaskPool_delete( self->taskPool );
    self->tasks                    = NULL;
    self->workers                  = NULL;
    self->workerTerminationBarrier = NULL;
    self->taskPool                 = NULL;
    self->clearing                 = false;

}


void WorkQueue_delete( WorkQueue *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static void WorkQueue_addWorker( WorkQueue *self )
{
    WorkQueueWorker *worker;
    bool            bRetVal;

    worker = WorkQueueWorker_new();

    bRetVal = WorkQueueWorker_init( worker, self );
    ANY_REQUIRE( bRetVal );
    MTList_add( self->workers, worker );
}


WorkQueueTask *WorkQueue_getTask( WorkQueue *self )
{
    return WorkQueueTaskPool_getTask( self->taskPool );
}


void WorkQueue_disposeTask( WorkQueue *self, WorkQueueTask *task )
{
    WorkQueueTaskPool_disposeTask( self->taskPool, task );
}


void WorkQueue_enqueue( WorkQueue *self, WorkQueueTask *task )
{
    int status;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUE_VALID );
    ANY_REQUIRE( task );
    ANY_REQUIRE( task->valid == WORKQUEUETASK_VALID );

    ANY_LOG( 10, "Enqueued task %p, queue len %ld", ANY_LOG_INFO,
             (void *)task, MTQueue_numElements( self->tasks ) );
    /* Checking workercount to avoid locking when the maximum
       worker count has been reached */
    if( ( !Atomic_get( &self->maxWorkersReached ) ) &&
        ( !Atomic_get( &self->freeWorkers ) ) > 0 )
    {
        if( self->maxWorkers )
        {
            status = Mutex_lock( self->mutex );
            ANY_REQUIRE( status == 0 );
        }
        /* Checking the condition again after locking */
        if( !Atomic_get( &self->maxWorkersReached ) )
        {
            WorkQueue_addWorker( self );
            Atomic_set( &self->maxWorkersReached,
                        ( self->maxWorkers != 0 && MTList_numElements( self->workers ) >= self->maxWorkers ) );
            ANY_LOG( 5, "Worker added to work queue, current number of workers: %ld",
                     ANY_LOG_INFO, MTList_numElements( self->workers ) );
        }

        if( self->maxWorkers )
        {
            status = Mutex_unlock( self->mutex );
            ANY_REQUIRE( status == 0 );
        }
    }
    MTQueue_push( self->tasks, task, WORKQUEUE_MTQUEUE_CLASS );
}


/* this is the main worker thread */
static void *WorkQueueWorker_main( void *worker )
{
    WorkQueueWorker     *self = (WorkQueueWorker *)worker;
    WorkQueueTaskStatus status;
    WorkQueueTask       *task = NULL;

    Atomic_set( &self->busy, false );
    Atomic_inc( &self->parent->freeWorkers );

    while( !Atomic_get( &self->exit ) )
    {
        ANY_LOG( 10, "Popping( %p ) task, queue len %ld", ANY_LOG_INFO,
                 (void *)self->parent, MTQueue_numElements( self->parent->tasks ) );
        task = (WorkQueueTask *)MTQueue_popWait( self->parent->tasks, NULL, 0 );
        ANY_LOG( 10, "Popped task %p, queue len %ld", ANY_LOG_INFO,
                 (void *)task, MTQueue_numElements( self->parent->tasks ) );

        if( task )
        {
            Atomic_dec( &self->parent->freeWorkers );
            Atomic_set( &self->busy, true );

            status = task->taskFn( task->instance, task->userData );

            if( task->callback )
            { task->callback( status, task ); }

            /* Signal task termination to callers of Task_wait() */
            WorkQueueTask_signal( task );

            Atomic_set( &self->busy, false );
            Atomic_inc( &self->parent->freeWorkers );
        }
    }

    Barrier_wait( self->parent->workerTerminationBarrier );

    return NULL;
}


WorkQueueWorker *WorkQueueWorker_new()
{
    return ANY_TALLOC( WorkQueueWorker );
}


bool WorkQueueWorker_init( WorkQueueWorker *self, WorkQueue *parent )
{
    bool retVal = true;

    ANY_REQUIRE( self );

    self->valid = WORKQUEUEWORKER_INVALID;

    Atomic_set( &self->exit, false );

    self->parent = parent;

    self->workerThread = Threads_new();
    if( !self->workerThread )
    { goto exit_error; }

    retVal = Threads_init( self->workerThread, false );

    if( !retVal )
    { goto dealloc_thread; }

    /* Start the worker thread */
    if( Threads_start( self->workerThread, WorkQueueWorker_main, self ) != 0 )
    {
        goto clear_thread;
    }

    self->valid = WORKQUEUEWORKER_VALID;
    return true;

    clear_thread:
    Threads_clear( self->workerThread );
    dealloc_thread:
    Threads_delete( self->workerThread );
    exit_error:
    return false;
}


void WorkQueueWorker_clear( WorkQueueWorker *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUEWORKER_VALID );

    self->valid = WORKQUEUEWORKER_INVALID;

    Threads_clear( self->workerThread );
    Threads_delete( self->workerThread );

    self->workerThread = NULL;
    self->parent       = NULL;
}


void WorkQueueWorker_delete( WorkQueueWorker *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


static WorkQueueTask *WorkQueueTask_new()
{
    return ANY_TALLOC( WorkQueueTask );
}


bool WorkQueueTask_init( WorkQueueTask *self, WorkQueueTaskFn taskFn, void *instance,
                         void *userData, WorkQueueTaskCallback callback )
{
    bool retVal = true;

    ANY_REQUIRE( self );
    ANY_LOG( 10, "WorkQueueTask_init(%p, %p)", ANY_LOG_INFO, (void *)self, (void *)taskFn );

    self->valid = WORKQUEUETASK_INVALID;

    self->terminated = false;
    self->instance   = instance;
    self->userData   = userData;

    self->taskFn   = taskFn;
    self->callback = callback;

    self->taskTerminatedCond = Cond_new();
    if( !self->taskTerminatedCond )
    { goto exit_error; }

    retVal = Cond_init( self->taskTerminatedCond, COND_PRIVATE );
    if( !retVal )
    { goto dealloc_cond; }

    self->mutex = Mutex_new();
    if( !self->mutex )
    { goto clear_cond; }

    retVal = Mutex_init( self->mutex, MUTEX_PRIVATE );
    if( !retVal )
    { goto dealloc_mutex; }

    Cond_setMutex( self->taskTerminatedCond, self->mutex );

    self->valid = WORKQUEUETASK_VALID;
    return true;

    dealloc_mutex:
    Mutex_delete( self->mutex );
    clear_cond:
    Cond_clear( self->taskTerminatedCond );
    dealloc_cond:
    Cond_delete( self->taskTerminatedCond );
    exit_error:
    return false;
}


void *WorkQueueTask_getInstance( WorkQueueTask *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASK_VALID );

    return self->instance;
}


void *WorkQueueTask_getUserData( WorkQueueTask *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASK_VALID );

    return self->userData;
}


void WorkQueueTask_signal( WorkQueueTask *self )
{
    int status;
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASK_VALID );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    self->terminated = true;
    status = Cond_signal( self->taskTerminatedCond );
    ANY_REQUIRE( status == 0 );

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );
}


void WorkQueueTask_wait( WorkQueueTask *self )
{
    int status;
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASK_VALID );

    status = Mutex_lock( self->mutex );
    ANY_REQUIRE( status == 0 );

    if( !self->terminated )
    {
        status = Cond_wait( self->taskTerminatedCond, 0 );
        ANY_REQUIRE( status == 0 );
    }

    status = Mutex_unlock( self->mutex );
    ANY_REQUIRE( status == 0 );
}


static void WorkQueueTask_clear( WorkQueueTask *self )
{
    ANY_REQUIRE( self );
    ANY_LOG( 10, "WorkQueueTask_clear(%p), %lx", ANY_LOG_INFO, (void *)self, Threads_id() );
    ANY_REQUIRE( self->valid == WORKQUEUETASK_VALID );

    self->valid = WORKQUEUETASK_INVALID;

    Cond_clear( self->taskTerminatedCond );
    Mutex_clear( self->mutex );
    Cond_delete( self->taskTerminatedCond );
    Mutex_delete( self->mutex );

    self->instance           = NULL;
    self->userData           = NULL;
    self->taskFn             = NULL;
    self->callback           = NULL;
    self->taskTerminatedCond = NULL;
    self->mutex              = NULL;
}


static void WorkQueueTask_delete( WorkQueueTask *self )
{
    ANY_REQUIRE( self );
    ANY_FREE( self );
}


/* Task Pool */

static WorkQueueTaskPool *WorkQueueTaskPool_new()
{
    return ANY_TALLOC( WorkQueueTaskPool );
}


static bool WorkQueueTaskPool_init( WorkQueueTaskPool *self, unsigned int initialSize )
{
    unsigned int  i;
    int           iRetVal;
    bool          bRetVal;
    WorkQueueTask *task;

    ANY_REQUIRE( self );

    self->valid = WORKQUEUETASKPOOL_VALID;

    self->taskBalance = 0;

    self->tasks = MTQueue_new();
    ANY_REQUIRE( self->tasks );
    iRetVal = MTQueue_init( self->tasks, MTQUEUE_FIFO, true );
    ANY_REQUIRE( iRetVal == 0 );

    self->nonTerminatedTasks = MTList_new();
    ANY_REQUIRE( self->nonTerminatedTasks );
    bRetVal = MTList_init( self->nonTerminatedTasks );
    ANY_REQUIRE( bRetVal );
    MTList_setDeleteMode( self->nonTerminatedTasks, MTLIST_DELETEMODE_MANUAL );

    self->tempQueue = MTQueue_new();
    ANY_REQUIRE( self->tempQueue );
    iRetVal = MTQueue_init( self->tempQueue, MTQUEUE_FIFO, true );
    ANY_REQUIRE( iRetVal == 0 );

    for( i = 0; i < initialSize; i++ )
    {
        task = WorkQueueTask_new();
        ANY_REQUIRE( task );
        MTQueue_push( self->tasks, task, WORKQUEUE_MTQUEUE_CLASS );
    }
    return true;
}


static void WorkQueueTaskPool_clear( WorkQueueTaskPool *self )
{
    WorkQueueTask *task;
    int           balance;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASKPOOL_VALID );

    self->valid = WORKQUEUETASKPOOL_INVALID;

    task = (WorkQueueTask *)MTQueue_pop( self->tasks, NULL );
    while( task != NULL )
    {
        WorkQueueTask_delete( task );
        task = (WorkQueueTask *)MTQueue_pop( self->tasks, NULL );
    }
    MTQueue_clear( self->tasks );
    MTQueue_delete( self->tasks );

    MTQueue_clear( self->tempQueue );
    MTQueue_delete( self->tempQueue );

    MTLIST_FOREACH_BEGIN ( self->nonTerminatedTasks, MTLIST_ITERATE_FOR_READ )
                {
                    task = (WorkQueueTask *)MTLIST_FOREACH_ELEMENTPTR;
                    WorkQueueTask_wait( task );
                    WorkQueueTask_clear( task );
                    WorkQueueTask_delete( task );
                }
    MTLIST_FOREACH_END;

    MTList_clear( self->nonTerminatedTasks );
    MTList_delete( self->nonTerminatedTasks );

    balance = Atomic_get( &self->taskBalance );
    if( balance != 0 )
    {
        ANY_LOG( 0, "%d tasks have not been disposed correctly! ", ANY_LOG_WARNING,
                 balance );
    }
}


static void WorkQueueTaskPool_delete( WorkQueueTaskPool *self )
{
    ANY_FREE( self );
}


static WorkQueueTask *WorkQueueTaskPool_getTask( WorkQueueTaskPool *self )
{
    WorkQueueTask *task;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASKPOOL_VALID );

    WorkQueueTaskPool_refreshTasks( self );

    task = (WorkQueueTask *)MTQueue_pop( self->tasks, NULL );
    if( task == NULL )
    {
        task = WorkQueueTask_new();
    }

    Atomic_inc( &self->taskBalance );
    return task;
}


static int WorkQueueTaskPool_taskCompare( void *a, void *b )
{
    return a == b ? 0 : 1;
}


static void WorkQueueTaskPool_refreshTasks( WorkQueueTaskPool *self )
{
    WorkQueueTask *task;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASKPOOL_VALID );
    ANY_LOG( 10, "Refresh task begin %lx", ANY_LOG_INFO, Threads_id() );

    MTLIST_FOREACH_BEGIN ( self->nonTerminatedTasks, MTLIST_ITERATE_FOR_READ )
                {
                    task = (WorkQueueTask *)MTLIST_FOREACH_ELEMENTPTR;
                    if( task->terminated )
                    {
                        MTQueue_push( self->tempQueue, task, WORKQUEUE_MTQUEUE_CLASS );
                    }
                }
    MTLIST_FOREACH_END;

    task = MTQueue_pop( self->tempQueue, NULL );
    while( task != NULL )
    {
        MTList_remove( self->nonTerminatedTasks, WorkQueueTaskPool_taskCompare, task );
        MTQueue_push( self->tasks, task, WORKQUEUE_MTQUEUE_CLASS );
        ANY_LOG( 10, "Clearing task %p", ANY_LOG_INFO, (void *)task );
        WorkQueueTask_clear( task );
        task = MTQueue_pop( self->tempQueue, NULL );
    }
    ANY_LOG( 10, "Refresh task end %lx", ANY_LOG_INFO, Threads_id() );
}


static void WorkQueueTaskPool_disposeTask( WorkQueueTaskPool *self, WorkQueueTask *task )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == WORKQUEUETASKPOOL_VALID );

    ANY_LOG( 10, "Disposing task %p", ANY_LOG_INFO, (void *)task );
    Atomic_dec( &self->taskBalance );
    if( task->terminated )
    {
        MTQueue_push( self->tasks, task, WORKQUEUE_MTQUEUE_CLASS );
        WorkQueueTask_clear( task );
    }
    else
    {
        MTList_add( self->nonTerminatedTasks, task );
    }
}


/* EOF */
