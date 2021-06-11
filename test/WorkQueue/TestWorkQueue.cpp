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


#define BUFLEN          ( 256 )
#define MSG_COUNT       ( 100 )
#define MSG_LEN         ( 32 )
#define NUM_MSG         ( 10 )
#define BUFFSIZE   255
#define MAX_SIZE  100

#if !defined(__windows__)

#include <unistd.h>

#endif


#include <Any.h>
#include <WorkQueue.h>
#include <MThreads.h>
#include <CuTest.h>

struct MutexAndCondition {
    Mutex *mutex;
    Cond *cond;
    int flag;
};

// Global "error occured" flag. Set to true when an error occured within a thread.
bool errorOccured = false;


void Test_WorkQueue_NewInitClearDelete_01( CuTest *tc )
{
    ANY_LOG( 0, "WaitOneTask Start", ANY_LOG_INFO );
    WorkQueue *queue;

    queue = WorkQueue_new();
    CuAssertPtrNotNull(tc, queue);

    CuAssertTrue( tc, WorkQueue_init( queue, 5, 10 ) );

    WorkQueue_clear( queue );
    WorkQueue_delete( queue );

    ANY_LOG( 0, "WaitOneTask End", ANY_LOG_INFO );
}

WorkQueueTaskStatus WaitOneTask_01_taskFn( void *instance, void*userData )
{
    bool *data = (bool *)instance;
    *data = true;
    return WORKQUEUE_TASK_SUCCESS;
}

void Test_WorkQueue_WaitOneTask_01( CuTest *tc )
{
    ANY_LOG( 0, "WaitOneTask Start", ANY_LOG_INFO );
    bool taskExecuted = false;
    WorkQueue *queue;

    errorOccured = false;

    queue = WorkQueue_new();
    CuAssertPtrNotNull(tc, queue);

    CuAssertTrue( tc, WorkQueue_init( queue, 5, 10 ) );

    WorkQueueTask *task = WorkQueue_getTask( queue );
    CuAssertPtrNotNull(tc, task);
    ANY_LOG( 0, "WaitOneTask initializing task", ANY_LOG_INFO );
    CuAssertTrue( tc, WorkQueueTask_init( task, WaitOneTask_01_taskFn, &taskExecuted,
                                          NULL, NULL ));

    ANY_LOG( 0, "WaitOneTask enqueuing task", ANY_LOG_INFO );
    WorkQueue_enqueue( queue, task);

    ANY_LOG( 0, "WaitOneTask sleep", ANY_LOG_INFO );
    Any_sleepSeconds( 1 );

    ANY_LOG( 0, "WaitOneTask wait", ANY_LOG_INFO );
    WorkQueueTask_wait( task );

    CuAssertTrue( tc, taskExecuted );

    CuAssertTrue( tc, !errorOccured );

    ANY_LOG( 0, "WaitOneTask clearing", ANY_LOG_INFO );

    WorkQueue_disposeTask( queue, task );
    WorkQueue_clear( queue );
    WorkQueue_delete( queue );

    ANY_LOG( 0, "WaitOneTask End", ANY_LOG_INFO );
}

void Test_WorkQueue_WaitOneTask_02( CuTest *tc )
{
    ANY_LOG( 0, "WaitOneTask Start", ANY_LOG_INFO );
    bool taskExecuted = false;
    WorkQueue *queue;

    errorOccured = false;

    queue = WorkQueue_new();
    CuAssertPtrNotNull(tc, queue);

    CuAssertTrue( tc, WorkQueue_init( queue, 5, 10 ) );

    WorkQueueTask *task = WorkQueue_getTask( queue );
    CuAssertPtrNotNull(tc, task);
    ANY_LOG( 0, "WaitOneTask initializing task", ANY_LOG_INFO );
    CuAssertTrue( tc, WorkQueueTask_init( task, WaitOneTask_01_taskFn, &taskExecuted,
                                          NULL, NULL ));

    ANY_LOG( 0, "WaitOneTask enqueuing task", ANY_LOG_INFO );
    WorkQueue_enqueue( queue, task);

    ANY_LOG( 0, "WaitOneTask wait", ANY_LOG_INFO );
    WorkQueueTask_wait( task );

    CuAssertTrue( tc, taskExecuted );

    CuAssertTrue( tc, !errorOccured );

    ANY_LOG( 0, "WaitOneTask clearing", ANY_LOG_INFO );
    WorkQueue_disposeTask( queue, task );
    WorkQueue_clear( queue );
    WorkQueue_delete( queue );


    ANY_LOG( 0, "WaitOneTask End", ANY_LOG_INFO );
}

void WorkQueue_OneTaskWithCallback_callback( WorkQueueTaskStatus status, WorkQueueTask *task )
{
    int istatus;

    struct MutexAndCondition *data =(struct MutexAndCondition *)WorkQueueTask_getInstance( task );
    data->flag = 1;

    istatus = Mutex_lock( data->mutex );
    ANY_REQUIRE( istatus == 0 );

    Cond_signal( data->cond );

    istatus = Mutex_unlock( data->mutex );
    ANY_REQUIRE( istatus );
}

WorkQueueTaskStatus WorkQueue_OneTaskWithCallback_taskFn( void *instance, void *userData )
{
    return WORKQUEUE_TASK_SUCCESS;
}

void Test_WorkQueue_OneTaskWithCallback_01( CuTest *tc )
{
    ANY_LOG( 0, "OneTaskWithCallback Start", ANY_LOG_INFO );
    struct MutexAndCondition data;
    WorkQueue *queue;

    data.mutex = Mutex_new();
    CuAssertPtrNotNull( tc, data.mutex );
    CuAssertTrue( tc, Mutex_init( data.mutex, MUTEX_PRIVATE ) );

    data.flag = 0;
    data.cond = Cond_new();
    CuAssertPtrNotNull( tc, data.cond );
    CuAssertTrue( tc, Cond_init( data.cond, COND_PRIVATE ) );
    Cond_setMutex( data.cond, data.mutex );

    CuAssertIntEquals( tc, Mutex_lock( data.mutex ), 0 );

    queue = WorkQueue_new();
    CuAssertPtrNotNull(tc, queue);

    CuAssertIntEquals( tc, WorkQueue_init( queue, 5, 10 ), 1 );

    WorkQueueTask *task = WorkQueue_getTask( queue );
    CuAssertPtrNotNull(tc, task);
    CuAssertTrue( tc, WorkQueueTask_init( task, WorkQueue_OneTaskWithCallback_taskFn, &data,
                                          NULL, WorkQueue_OneTaskWithCallback_callback ) );

    WorkQueue_enqueue( queue, task);

    CuAssertIntEquals( tc, Cond_wait( data.cond, 0 ), 0 );

    CuAssertTrue( tc, data.flag );
    CuAssertIntEquals( tc, Mutex_unlock( data.mutex ), 0 );

    WorkQueue_disposeTask( queue, task );
    WorkQueue_clear( queue );
    WorkQueue_delete( queue );
    Mutex_clear( data.mutex );
    Mutex_delete( data.mutex );
    Cond_clear( data.cond );
    Cond_delete( data.cond );
    ANY_LOG( 0, "OneTaskWithCallback End", ANY_LOG_INFO );
}

WorkQueueTaskStatus SomeTasks_TaskFn( void *instance, void *userData )
{
    bool *taskExecuted;
    long long i;

    taskExecuted = (bool *)instance;
    i = (long long) userData;

    taskExecuted[ i ] = true;
    return WORKQUEUE_TASK_SUCCESS;
}

void Test_WorkQueue_SomeTasks( CuTest *tc )
{
    long long i;
    int nTasks = 20;
    bool *taskExecuted;
    WorkQueue *queue;
    WorkQueueTask **tasks;


    errorOccured = false;

    queue = WorkQueue_new();
    CuAssertPtrNotNull(tc, queue);

    CuAssertTrue( tc, WorkQueue_init( queue, 0, 10 ) );


    tasks = ANY_NTALLOC( nTasks, WorkQueueTask* );
    ANY_REQUIRE( tasks );

    taskExecuted = ANY_NTALLOC( nTasks, bool );
    ANY_REQUIRE( taskExecuted );

    for ( i = 0; i < nTasks; i++ )
    {
        taskExecuted[ i ] = false;
        tasks[ i ] = WorkQueue_getTask( queue );
        CuAssertPtrNotNull( tc, tasks[ i ] );
        CuAssertTrue( tc, WorkQueueTask_init( tasks[ i ], SomeTasks_TaskFn, taskExecuted,
                                              (void *)i, NULL ));
    }

    for ( i = 0; i < nTasks; i ++ )
    {
        WorkQueue_enqueue( queue, tasks[ i ] );
    }

    for ( i = 0; i < nTasks; i ++ )
    {
        WorkQueueTask_wait( tasks[ i ] );
    }

    for ( i = 0; i < nTasks; i ++ )
    {
        CuAssertTrue( tc, taskExecuted[ i ] );
    }

    CuAssertTrue( tc, !errorOccured );

    for ( i = 0; i < nTasks; i ++ )
    {
        WorkQueue_disposeTask( queue, tasks[i] );
    }
    WorkQueue_clear( queue );
    WorkQueue_delete( queue );

    ANY_LOG( 0, "WaitOneTask clearing", ANY_LOG_INFO );


    ANY_FREE( tasks );
    ANY_FREE( taskExecuted );
    ANY_LOG( 0, "WaitOneTask End", ANY_LOG_INFO );
}

void dump( void *arg )
{
    Traps_callTrace();
}
/*---------------------------------------------------------------------------*/
/* Main program                                                              */
/*---------------------------------------------------------------------------*/
int main( void )
{
    CuSuite  *suite   = CuSuiteNew();
    CuString *output  = CuStringNew();
    char     *verbose = (char *)NULL;

    Any_onRequire( dump, NULL );

    ANY_REQUIRE( suite );
    ANY_REQUIRE( output );

    verbose = getenv( (char *)"VERBOSE" );
    if( verbose != NULL && Any_strcmp( verbose, (char *)"TRUE" ) == 0 )
    {
        Any_setDebugLevel( 10 );
    }
    else
    {
        Any_setDebugLevel( 0 );
    }

    SUITE_ADD_TEST( suite, Test_WorkQueue_WaitOneTask_01 );
    SUITE_ADD_TEST( suite, Test_WorkQueue_WaitOneTask_02 );
    SUITE_ADD_TEST( suite, Test_WorkQueue_OneTaskWithCallback_01 );
    SUITE_ADD_TEST( suite, Test_WorkQueue_SomeTasks );
    SUITE_ADD_TEST( suite, Test_WorkQueue_NewInitClearDelete_01 );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    CuSuiteDetails( suite, output );

    Any_fprintf( stderr, "%s\n", output->buffer );

    CuSuiteDelete( suite );
    CuStringDelete( output );


    return suite->failCount;
}
