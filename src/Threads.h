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


#ifndef THREADS_H
#define THREADS_H


#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#if !defined(__msvc__)
#include <sys/time.h>
#endif

#include <Any.h>


#define THREADS_EINVAL      EINVAL
#define THREADS_EAGAIN      EAGAIN
#define THREADS_ESRCH       ESRCH
#define THREADS_ENOSYS      ENOSYS
#define THREADS_ENOMEM      ENOMEM
#define THREADS_EBUSY       EBUSY
#define THREADS_EPERM       EPERM
#define THREADS_ETIMEDOUT   ETIMEDOUT
#define THREADS_ENOTSUP     ENOTSUP
#define THREADS_EINTR       EINTR
#define THREADS_EDEADLK     EDEADLK

#define THREADS_SCHED_RR    SCHED_RR
#define THREADS_SCHED_FIFO  SCHED_FIFO
#define THREADS_SCHED_OTHER SCHED_OTHER


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct Threads
{
    unsigned long valid;
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param schedulerParams;
} Threads;


Threads *Threads_new( void );

/*!
 * \brief Initialize a Threads
 * \param self Pointer to a Thread
 * \param joinable If true, the thread is joinable, otherwise is created detached
 *
 * \return Return \a true if initialized correctly, \a false otherwise
 */
bool Threads_init( Threads *self, bool joinable );

/*!
 * \brief Start the thread's execution
 * \param self Pointer to a Thread
 * \param start_routine Pointer to function of the thread's entry point
 * \param arg Thread's argument
 *
 * Start the thread's execution on the \a start_routine entry point. The
 * \a start_routine must be a pointer of function that return a void's pointer
 * and take a void's pointer as argument.
 */
int Threads_start( Threads *self,
                   void *(*start_routine)( void * ),
                   void *arg );

/*!
 * \brief Join the thread's execution
 *
 * \return Return \a 0 on success, \a EAGAIN on error
 */
int Threads_join( Threads *self, void **retValue );

/*!
 * \brief Set the thread's execution as cancellable
 *
 * Must be called in order to permit the thread's cancellation after its
 * execution. It's used inside the thread's routine and delimit some critical
 * code execution with its counterpart \a Threads_setUncancellable
 */
void Threads_setCancellable( Threads *self );

/*!
 * \brief Unset the thread's execution as cancellable
 *
 * Must be called in order to don't permit the thread's cancellation after its
 * execution. It's used inside the thread's routine and delimit some critical
 * code execution with its counterpart \a Threads_setCancellable
 */
void Threads_setUncancellable( Threads *self );

/*!
 * \brief Thread safe termination
 *
 * Must be called in order to terminate safetly the thread's
 * execution. It this function is called inside of some \a Threads_cleanupPush
 * call, all the remaining cleanup handler will be called in order
 * to shutdown the thread cleaning up all the resource
 */
void Threads_exit( Threads *self, void *retval );

/*!
 * \brief Cancel a thread in safe mode
 *
 * Cancel the specified Threads. This function call the safe mode
 * thread's cancellation in order to terminate it outside the
 * thread's process. The thread will be cancelled only if it's outside
 * if its critical code section if defined
 */
int Threads_stop( Threads *self );

void Threads_kill( Threads *self );

/*!
 * \brief Setup a cancellation handler
 * \param __routine Pointer to a cleanup function
 * \param __arg Pointer to a void argument of the cleanup function
 *
 * Install a cleanup handler for the thread. The clean up handle is called
 * with the \a __arg argument if the thread is cancelled or if it have called
 * the \a Threads_exit. This macro must be in the same scope of the its counterpart
 * \a Threads_cleanupPop and cannot be declare in a separate function. All the
 * cleanup handler are defined as stack so the last defined is the first called
 */
#define Threads_cleanupPush( __routine, __arg )\
        pthread_cleanup_push(__routine, __arg)

/*!
 * \brief Remove a cancellation handler
 * \param __execute Execute the pushed handler
 *
 * Remove the last installed cleanup handler. If \a __execute is non zero the
 * last installed cleanup handler is executed as described in \a Threads_cleanupPush
 * and finally removed, otherwise if \a __execute is zero the last installed
 * cleanup handler is removed but not execute. As described on \a Threads_cleanupPush
 * this macro must called on the same scope the its counterpart
 */
#define Threads_cleanupPop( __execute )\
        pthread_cleanup_pop(__execute)

/*!
 * \brief Yield the processor to another threads or process
 *
 * Must be called in order to optimize the processor/s utilization.
 * If the Threads had nothing to do should call this in order to yield
 * the processor to another threads or other process
 *
 * \return Return \a 0 on success, non zero otherwise
 */
int Threads_yield( void );

unsigned long Threads_id( void );

bool Threads_isEqualId( Threads *self, unsigned long id );

/*!
 * \brief Function to be called on thread fault
 *
 * This function shuold be called when a fault append.
 * This function just call Threads_exit.
 */
void Threads_faultRecovery( void );

/*!
 * \brief Sets the Threads's priority
 *
 * This function sets the Thread's priority. If priority is
 * equal to zero then no priority is set.
 */
void Threads_setPriority( Threads *self, int priority );

int Threads_getPriority( Threads *self );

void Threads_clear( Threads *self );

void Threads_delete( Threads *self );

/*!
 * \brief Sets the scheduler policy and priority for a given thread
 * \param self Instance pointer
 * \param policy Schedule policy (THREADS_SCHED_FIFO, THREADS_SCHED_RR or THREADS_SCHED_OTHER)
 * \param priority Priority for the thread
 * \return nothing

 * This function sets the scheduler policy and priority for a given thread.
 * The function should be called _BEFORE_ to start the thread my the
 * \a Threads_start() otherwise it doesn't have any effect on the running
 * thread.
 *
 * \section Threads_SchedulingPolicies Scheduling Policies
 *
 * The scheduler is the kernel part that decides which runnable process
 * will be executed by the CPU next. Generally a Unix like O.S. scheduler
 * offers three different scheduling policies, one for normal processes and
 * two for real-time applications. A static priority value sched_priority is
 * assigned to each process and this value can be changed only via system
 * calls. Conceptually, the scheduler maintains a list of runnable processes
 * for each possible sched_priority value, and sched_priority can have a
 * value in the range 0 to 99. In order to determine the process that runs
 * next, the kernel scheduler looks for the non-empty list with the highest
 * static priority and takes the process at the head of this list. The
 * scheduling policy determines for each process, where it will be inserted
 * into the list of processes with equal static priority and how it will
 * move inside this list.
 *
 * \a THREADS_SCHED_OTHER is the default universal time-sharing scheduler
 * policy used by most processes, \a THREADS_SCHED_FIFO and
 * \a THREADS_SCHED_RR are intended for special time-critical applications
 * that need precise control over the way in which runnable processes are
 * selected for execution. Processes scheduled with
 * \a THREADS_SCHED_OTHER must be assigned the static priority 0, processes
 * scheduled under \a THREADS_SCHED_FIFO or \a THREADS_SCHED_RR can have a
 * static priority in the range 1 to 99. Only processes with superuser
 * privileges can get a static priority higher than 0 and can therefore be
 * scheduled under \a THREADS_SCHED_FIFO or \a THREADS_SCHED_RR.
 *
 * All scheduling is preemptive: If a process/threads with a higher static
 * priority gets ready to run, the current process will be preempted and
 * returned into its wait list. The scheduling policy only determines the
 * ordering within the list of runnable processes with equal static priority.
 *
 * \subsection THREADS_SCHED_FIFO First In-First Out scheduling
 *
 * \a THREADS_SCHED_FIFO can only be used with static priorities higher
 * than 0, which means that when a \a THREADS_SCHED_FIFO processes becomes
 * runnable, it will always preempt immediately any currently running normal
 * \a THREADS_OTHER process. \a THREADS_SCHED_FIFO is a simple scheduling
 * algorithm without time slicing. For processes scheduled under the
 * \a THREADS_SCHED_FIFO policy, the following rules are applied:
 * A \a THREADS_SCHED_FIFO process that has been preempted by another process
 * of higher priority will stay at the head of the list for its priority
 * and will resume execution as soon as all processes of higher priority
 * are blocked again. When a \a THREADS_SCHED_FIFO process becomes runnable,
 * it will be inserted at the end of the list for its priority.
 * (POSIX 1003.1 specifies that the process should go to the end of the
 * list.) A process calling \a Thread_yield() will be put at the end of the
 * list. No other events will move a process scheduled under the \a
 * THREADS_SCHED_FIFO policy in the wait list of runnable processes with
 * equal static priority. A \a THREADS_SCHED_FIFO process runs until either
 * it is blocked by an I/O request, it is preempted by a higher priority
 * process, or it calls \a Thread_yield().
 *
 * \subsection THREADS_SCHED_RR Round Robin scheduling
 *
 * \a THREADS_SCHED_RR is a simple enhancement of \a THREADS_SCHED_FIFO.
 * Everything described above for \a THREADS_SCHED_FIFO also applies to
 * \a THREADS_SCHED_RR, except that each process is only allowed to run for
 * a maximum time quantum. If a \a THREADS_SCHED_RR process has been running
 * for a time period equal to or longer than the time quantum, it will be
 * put at the end of the list for its priority. A \a THREADS_SCHED_RR process
 * that has been preempted by a higher priority process and subsequently
 * resumes execution as a running process will complete the unexpired
 * portion of its round robin time quantum.
 *
 * \subsection THREADS_OTHER Default time-sharing scheduling
 *
 * \a THREADS_SCHED_OTHER can only be used at static priority 0.
 * \a THREADS_SCHED_OTHER is the standard time-sharing scheduler that is
 * intended for all processes that do not require special static priority
 * real-time mechanisms. The process to run is chosen from the static
 * priority 0 list based on a dynamic priority that is determined only
 * inside this list. The dynamic priority is based on the nice level (set
 * by the nice or setpriority system call) and increased for each time
 * quantum the process is ready to run, but denied to run by the scheduler.
 * This ensures fair progress among all \a THREADS_SCHED_OTHER processes.
 *
 * As a non-blocking end-less loop in a process scheduled under
 * \a THREADS_SCHED_FIFO or \a THREADS_SCHED_RR will block all processes
 * with lower priority forever, a software developer should always keep
 * available on the console a shell scheduled under a higher static priority
 * than the tested application. This will allow an emergency kill of tested
 * real-time applications that do not block or terminate as expected.
 * As \a THREADS_SCHED_FIFO and \a THREADS_SCHED_RR processes can preempt
 * other processes forever, only root processes are allowed to activate
 * these policies under Linux.
 *
 * The \a priority may have different values depending by the scheduler
 * policy. Generally start from 0 to 99 under real-time policy like
 * \a THREADS_SCHED_FIFO or \a THREADS_SCHED_RR.
 *
 * \code
 * Threads *t = (Threads*)NULL;
 *
 * t = Threads_new();
 * ANY_REQUIRE_MSG( t, "Unable to allocate memory for a Thread" );
 *
 * if ( Threads_init( t ) == false )
 * {
 *   ANY_LOG( 0, "Unable to initialize the Thread", ANY_LOG_FATAL );
 *   exit( -1 );
 * }
 *
 * // we want real-time RR scheduling policy with priority 50 for the thread 't'
 * Threads_setSchedPolicy( t, THREADS_SCHED_RR, 50 );
 *
 * // finally start the thread 't' for real-time purpose
 * if( Threads_start( t, myThreadMain, myInstance ) != 0 )
 * {
 *   ANY_LOG( 0, "Unable to start the Thread", ANY_LOG_FATAL );
 *   Threads_clear( t );
 *   Threads_delete( t );
 *   exit( -1 );
 * }
 * \endcode
 *
 * \see Threads_start()
 */
void Threads_setSchedPolicy( Threads *self, int policy, int priority );


#if defined(__cplusplus)
}
#endif

#endif


/* EOF */
