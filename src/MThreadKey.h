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


#if !defined( MTHREADKEY_H )
#define MTHREADKEY_H

#include <Any.h>
#include <pthread.h>


#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief MThreadKey definition
 */
typedef struct MThreadKey
{
    unsigned long valid;
    /**< valid flag */
    pthread_key_t key;    /**< Multithreaded key */
}
        MThreadKey;

/*!
 * \brief Define the function callback for the user's data destructor
 */
typedef void ( MThreadKeyDestructor )( void * );

/*!
 * \brief Allocate a new MThreadKey instance
 *
 * This function return a new allocated MThreadKey instance
 *
 * \return A pointer to a new allocated MThreadKey instance
 *
 * \see MThreadKey_init()
 * \see MThreadKey_clear()
 * \see MThreadKey_delete()
 */
MThreadKey *MThreadKey_new( void );

/*!
 * \brief Initialize a MThreadKey instance
 * \param self MThreadKey instance pointer
 * \param destructor Function pointer for the user's data destructor
 *
 * This function initialize a new MThreadKey and set the optional destructor function
 * for deallocating the user's data. The destructor can be NULL if any destructor is
 * provided
 *
 * \return Return true on success, false otherwise
 *
 * \see MThreadKey_new()
 * \see MThreadKey_clear()
 * \see MThreadKey_delete()
 */
bool MThreadKey_init( MThreadKey *self, MThreadKeyDestructor *destructor );

/*!
 * \brief Set a per thread user's instance value for the given MThreadKey
 * \param self MThreadKey instance pointer
 * \param value Pointer to the user's date
 *
 * This function set a per thread user's instance value for the given MThreadKey
 *
 * \return Return true on success, false otherwise
 *
 * \see MThreadKey_get()
 */
bool MThreadKey_set( MThreadKey *self, void *value );

/*!
 * \brief Return a per thread user's instance value for the given MThreadKey
 * \param self MThreadKey instance pointer
 *
 * This function return a per thread user's instance value for the given MThreadKey.
 *
 * \return Return a valid pointer on success, NULL otherwise
 *
 * \see MThreadKey_set()
 */
void *MThreadKey_get( MThreadKey *self );

/*!
 * \brief Clear the given MThreadKey instance
 * \param self MThreadKey instance pointer
 *
 * This function clear the given MThreadKey instance. Beware, this function must be called only once
 * for destroying the given key and not it's per thread user's date
 *
 * \return Return a valid pointer on success, NULL otherwise
 *
 * \see MThreadKey_new()
 * \see MThreadKey_init()
 * \see MThreadKey_delete()
 */
void MThreadKey_clear( MThreadKey *self );

/*!
 * \brief Clear the given MThreadKey instance
 * \param self MThreadKey instance pointer
 *
 * This function releases the memory allocated for the given MThreadKey instance.
 *
 * \return Return Nothing
 *
 * \see MThreadKey_new()
 * \see MThreadKey_init()
 * \see MThreadKey_clear()
 */
void MThreadKey_delete( MThreadKey *self );

#if defined(__cplusplus)
}
#endif

#endif  /* MTHREADKEY_H */


