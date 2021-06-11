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


#ifndef TRAPS_H
#define TRAPS_H


#include <Any.h>

#if defined(__windows__)
#include <windows.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief This struct define the generic fields for delivering trap's information
 */
typedef struct TrapsException
{
    int exception;
    /**< Store the signal */
    void *address;
    /**< Store the address where the exception happens */
    void *specific1;
    /**< Store the specific arch depend information such us siginfo_t or EXCEPTION_POINTERS */
    void *specific2;       /**< Store the specific arch depend information such us ctx */
}
        TrapsException;


/* call trace */
/*!
 * \brief Print to the standard output the current call trace log
 *
 * The function print out to the standard output the current call trace log
 */
void Traps_callTrace( void );

/*!
 * \brief Force a coredump
 *
 * The function force the generation of a core dump
 */
void Traps_coreDump( void );

/*!
 * \brief Setup the fault handler for catching signals
 * \param faultHandler Pointer to a function accepting a user's parameter
 * \param faultHandlerParam User's define parameter to pass to the function handler
 *
 * The function setup the fault handler for catching signals in the application
 */
void Traps_faultSetup( void (*faultHandler)( void * ), void *faultHandlerParam );

/*!
 * \brief Setup the fault handler for catching signals
 * \param faultExtendedHandler Pointer to a function accepting a user's parameter
 * \param faultExtendedHandlerParam User's define parameter to pass to the function handler
 *
 * The function setup the fault handler for catching signals in the application. The trap handler
 * needs an addition parameter which isn't required by the Traps_faultSetup()
 */
void Traps_faultSetupExtended( void (*faultExtendedHandler)( void *, TrapsException * ),
                               void *faultExtendedHandlerParam );

/*!
 * \brief Block/Unblock a given signal
 * \param block Can be SIG_BLOCK or SIG_UNBLOCK
 * \param signum The requested signal
 *
 * The function block/unblock the given signal
 */
void Traps_blockSignals( int block, int signum );

/*!
 * \brief The function sets a trap handler for catching a signal
 * \param signum The requested signal
 * \param handler The function handler for the given signal
 *
 * The function setup a trap handler for a given signal
 */
void Traps_catchSignal( int signum, void (*handler)( int ));

/*!
 * \brief Traps all the synchronous signals
 *
 * The function traps all the synchronous signals
 */
void Traps_trapSynchronousSignal( void );

/*!
 * \brief Untraps all the synchronous signals
 *
 * The function untraps all the synchronous signals
 */
void Traps_untrapSynchronousSignal( void );

/*!
 * \brief Setup the limits for generating core dumps
 *
 * The function setup the limits for generating core dumps
 */
void Traps_coredumpSetup( void );

#define Traps_callTraceOnExit( __notUsed ) \
do\
{\
  atexit( Traps_callTrace );\
}\
while( 0 )

#if defined(__cplusplus)
}
#endif

#endif  /* TRAPS_H */


/* EOF */
