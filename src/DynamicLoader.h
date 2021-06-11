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


/*!
 * \page DynamicLoader_About Dynamic loading of shared libraries
 *
 * The DynamicLoader.h contains:
 *
 * \li a platform independent wrapper for the dynamic linker
 * \li functions to load symbols by name from a specified library or
 *     from the global symbol scope
 *
 * <h3>Example:</h3>
 * \code
 * ...
 * function = DynamicLoader_getSymbolByName( NULL, FUNCTIONNAME );
 *
 * if ( function )
 * {
 *   void (*callFunc)( char* ) = (void(*)(char*))function;
 *
 *   ANY_LOG( 0, "The function '%s' is at address '%p'", ANY_LOG_INFO, FUNCTIONNAME, function );
 *   ANY_LOG( 0, "Calling the function '%s'", ANY_LOG_INFO, FUNCTIONNAME );
 *
 *   (*callFunc)( "It works!!!" );
 * }
 * else
 * {
 *   char *err = DynamicLoader_getError( dl );
 *   ANY_LOG( 0, "The function '%s' hasn't been found. The error is: %s", ANY_LOG_INFO, FUNCTIONNAME, err );
 * }
 * ...
 * \endcode
 */


#ifndef DYNAMICLOADER_H
#define DYNAMICLOADER_H


/*--------------------------------------------------------------------------*/
/* Includes                                                                 */
/*--------------------------------------------------------------------------*/


#include <Any.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*--------------------------------------------------------------------------*/
/* Public definitions and datatypes                                         */
/*--------------------------------------------------------------------------*/


typedef struct DynamicLoader
{
    unsigned long valid;
    /**< Valid flag */
    char *libraryName;
    /**< Library name */
    void *libraryHandle;                            /**< Library handler */
#if defined(__windows__)
#define DYNAMICLOADER_MAXERRORSIZE 256
    char errMsg[DYNAMICLOADER_MAXERRORSIZE];        /**< scratch area for windows messages */
#endif
}
        DynamicLoader;


typedef void (*DynamicLoaderFunction)( void );


/*--------------------------------------------------------------------------*/
/* Public functions                                                         */
/*--------------------------------------------------------------------------*/


/*!
 * \brief Create a new DynamicLoader
 *
 * This function creates a new DynamicLoader object
 *
 * \return Return a pointer to a new DynamicLoader object otherwise NULL
 */
DynamicLoader *DynamicLoader_new( void );


/*!
 * \brief Initialize and open a new dynamic library
 * \param self Pointer to a DynamicLoader object
 * \param libraryName Dynamic library path
 *
 * This function tries to load the dynamic library specified by the
 * libraryName parameter. If the specified parameter is NULL than the
 * DynamicLoader library tries to open a global pool of symbols.
 *
 * \return Return 0 on success, not 0 on failure
 */
int DynamicLoader_init( DynamicLoader *self, const char *libraryName );


/*!
 * \brief Find the address of symbol in the DynamicLoader library
 *
 * This function tries to find the address of the symbol specified by the
 * symbolName parameter.
 *
 * \note This function is discouraged. Please use
 *       \c DynamicLoader_getFunctionSymbol() or
 *       \c DynamicLoader_getDataSymbol() instead.
 *
 * \param self Pointer to a DynamicLoader object, or \c NULL to search in the
 *             global symbol scope
 * \param symbolName The searched symbol
 *
 * \return On success the address of the searched symbol is returned, otherwise
 *         \c NULL
 */
DynamicLoaderFunction DynamicLoader_getSymbolByName( DynamicLoader *self,
                                                     const char *symbolName );


/*!
 * \brief Find the address of symbol in the DynamicLoader library
 *
 * This function tries to find the address of the symbol specified by the
 * symbolName parameter.
 *
 * In contrast to \c DynamicLoader_getSymbolByName() it makes use of
 * the coding convention to have function names like "Project_function()",
 * or in a more OOP-like style "Class_method(). This way, you don't need to
 * precalculate the string for the symbol name yourself.
 *
 * \param self Pointer to a DynamicLoader object, or \c NULL to search in the
 *             global symbol scope
 *
 * \param className the project name ("class") part of the function,
 *                  e.g. "ArrayBlockF32"
 *
 * \param methodName the "method" part of the function name,
 *                   e.g. "indirectSerialize"
 *
 * \return On success the address of the searched symbol is returned, otherwise
 *         \c NULL
 */
DynamicLoaderFunction DynamicLoader_getSymbolByClassAndMethodName( DynamicLoader *self,
                                                                   const char *className,
                                                                   const char *methodName );


/*!
 * \brief Find the address of function symbol in the DynamicLoader library
 *
 * This function tries to find the address of the symbol specified by the
 * symbolName parameter.
 *
 * \param self Pointer to a DynamicLoader object, or \c NULL to search in the
 *             global symbol scope
 * \param symbolName The searched symbol
 *
 * \return On success the address of the searched symbol is returned, otherwise
 *         \c NULL
 */
DynamicLoaderFunction DynamicLoader_getFunctionSymbol( DynamicLoader *self,
                                                       const char *name );


/*!
 * \brief Find the address of data symbol in the DynamicLoader library
 *
 * This function tries to find the address of the symbol specified by the
 * symbolName parameter.
 *
 * \param self Pointer to a DynamicLoader object, or \c NULL to search in the
 *             global symbol scope
 * \param symbolName The searched symbol
 *
 * \return On success the address of the searched symbol is returned, otherwise
 *         \c NULL
 */
void *DynamicLoader_getDataSymbol( DynamicLoader *self, const char *name );


/*!
 * \brief Get the last error string reported by the DynamicLoader
 * \param self Pointer to a DynamicLoader object
 *
 * This function returns the last error string reported by the DynamicLoader.
 * The user should call this function after calling DynamicLoader_init() and
 * DynamicLoader_getSymbolByName(). This function could return NULL when everything
 * is ok.
 *
 * \return Returns not NULL when an error occurs
 *
 * \see DynamicLoader_init()
 * \see DynamicLoader_getSymbolByName()
 */
char *DynamicLoader_getError( DynamicLoader *self );


/*!
 * \brief Return the library name
 * \param self Pointer to a DynamicLoader object
 *
 * This function returns the library name
 *
 * \return The library name pointer, NULL for the global process or invalid library
 */
char *DynamicLoader_getLibraryName( DynamicLoader *self );


/*!
 * \brief Close and clear the DynamicLoader object
 * \param self Pointer to a DynamicLoader object
 *
 * Closes and clear the DynamicLoader library
 */
void DynamicLoader_clear( DynamicLoader *self );


/*!
 * \brief Release the memory allocated by the DynamicLoader object
 * \param self Pointer to a DynamicLoader object
 *
 * This function releases the memory allocated by the DynamicLoader object
 */
void DynamicLoader_delete( DynamicLoader *self );


#if defined(__cplusplus)
}
#endif


#endif  /* DYNAMICLOADER_H */


/* EOF */
