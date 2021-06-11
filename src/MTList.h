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
 * \page MTList_About Linked lists
 *
 * MTList.h contains an implementation of a thread-safe linked list
 * which allows secure list operations such as add/insert/delete/remove
 * even if invoked within a concurrent environment i.e. when the list is
 * shared among more than one thread.
 *
 * Use MTList_insert() to add a new element on top of the list and
 * MTList_add() to add it to the bottom of the list. Looping macros
 * like MTLIST_FOREACH_BEGIN allow to iterate on the list. These macros
 * differ in the direction they follow (normal or reverse) or in the
 * way they access elements (locked or unlocked).
 *
 * <h3>Example:</h3>
 * \code
 *    ...
 *    aList = MTList_new();

 *    if( MTList_init( aList ) != true )
 *    {
 *      MTList_delete( aList );
 *      return -1;
 *    }
 *
 *    MTList_setDeleteMode( aList, MTLIST_DELETEMODE_MANUAL );
 *    MTList_insert( aList, (char*)"2^ element" );
 *    MTList_insert( aList, (char*)"1^ element" );
 *    MTList_remove( aList, (int (*)(void*, void*))strcmp, (void*)"2^ element" );
 *    MTList_add( aList, (char*)"3^ element" );
 *
 *    // Print all the elements.
 *    MTLIST_FOREACH_BEGIN (aList, MTLIST_ITERATE_FOR_READ )
 *    {
 *      printf( "Element: %s\n", (char*)MTLIST_FOREACH_ELEMENTPTR );
 *    }
 *    MTLIST_FOREACH_END;
 *
 *    MTList_clear( aList );
 *    MTList_delete( aList );
 *    ...
 * \endcode
 */


#ifndef MTLIST_H
#define MTLIST_H


#include <Any.h>
#include <RWLock.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct MTListElement
{
    unsigned long valid;
    /**< Element's validation */
    RWLock *rwlock;
    /**< Element's Read/Write lock */
    long flags;
    /**< Element's flags */
    struct MTListElement *next;
    /**< Next element in list */
    void *data;   /**< Element's data */
}
        MTListElement;

/**!
 *   \brief MTList definition
 *
 */
typedef struct MTList
{
    unsigned long valid;
    /**< List's validation */
    RWLock *rwlock;
    /**< List's Read/Write lock */
    MTListElement *first;
    /**< First element in list */
    MTListElement *last;
    /**< Last element in list */
    long numElement;
    /**< Number of elements in list */
    int deleteMode;    /**< Element's delete mode */
}
        MTList;

/* object structure validity */

/**!
 *   \brief Magic number for the MTList validity
 *
 * It's used to validate a MTList.
 *
 */
#define MTLIST_VALID    0xa951bd65

/**!
 *   \brief Magic number for the MTList invalidity
 *
 * It's used to invalidate an MTList.
 *
 */
#define MTLIST_INVALID    0x523c6bbf

/**!
 *   \brief Magic number for the MTListElement validity
 *
 * It's used to validate a MTListElement.
 *
 */
#define MTLISTELEMENT_VALID    0xa673edee

/**!
 *   \brief Magic number for the MTListElement invalidity
 *
 * It's used to invalidate an MTListElement.
 *
 */
#define MTLISTELEMENT_INVALID    0x08039bc4

/**!
 *   \brief Flag for write on list iteration.
 *
 * It's used on \a MTList_iterate().
 *
 */
#define MTLIST_ITERATE_FOR_WRITE    0x00000001

/**!
 *   \brief Flag for read on list iteration.
 *
 * It's used on \a MTList_iterate().
 *
 */
#define MTLIST_ITERATE_FOR_READ    0x00000002

/*!
 *   \brief Delete all the data element's on MTList_clear()
 */
#define MTLIST_DELETEMODE_AUTOMATIC    0x00000001

/*!
 *   \brief Not delete all the data element's on List_clear()
 */
#define MTLIST_DELETEMODE_MANUAL    0x00000002

/*!
 *  \brief Create a new MTList object.
 *        Should be used to create dinamically a new MTList object.
 *
 * This method is used to create dinamically a new MTList object. For every created
 * object with this function, should be called the MTList_delete() to destroy
 * the object and deallocate all the resource.
 *
 *  \return Return a new MTList pointer. Null if can't create.
 *  \see MTList_init()
 *  \see MTList_clear()
 *  \see MTList_delete()
 */
MTList *MTList_new( void );

/*!
 *  \brief Initialize a MTList object
 *  \param self Pointer to a MTList object
 *
 * This method is used to initialize an empty object MTList. The object must be
 * initialize before used. When the object isn't needed any more
 * should be cleaned by the MTList_clear().
 *
 *  \return Return true if the object is initialized correctly, false if not.
 *  \see MTList_new()
 *  \see MTList_clear()
 *  \see MTList_delete()
 */
bool MTList_init( MTList *self );

/*!
 *  \brief Add a new element on the MTList
 *  \param self Pointer to a MTList object
 *  \param data Data to add on the MTList
 *
 * This method is used to add a new element on the MTList.
 * The element is added at the bottom of the list.
 *
 *  \return Return true if the object is added correctly, false if not.
 *
 *  \see MTList_insert()
 *  \see MTList_remove()
 */
bool MTList_add( MTList *self, void *data );

/*!
 *  \brief Insert a new element on the top of the MTList
 *  \param self Pointer to a MTList object
 *  \param data Data to add on the MTList
 *
 * This method is used to a new element on the MTList.
 * The element is added at the top of the list.
 *
 *  \return Return true if the object is added correctly, false if not.
 *
 *  \see MTList_add()
 *  \see MTList_remove()
 */
bool MTList_insert( MTList *self, void *data );

/*!
 *  \brief Remove a element from the MTList
 *  \param self Pointer to a MTList object
 *  \param cmpFunc compare function called for each element on the
 *                 list in order to search the element
 *  \param searched information of the element to be deleted from the MTList
 *
 * This method is used to remove an element from the MTList.
 *
 *  \return Return true if the object is removed correctly, false if not.
 *
 *  \see MTList_add()
 *  \see MTList_insert()
 */
bool MTList_remove( MTList *self, int (*cmpFunc)( void *, void * ), void *searched );

/*!
 *  \brief Set a new pointer value for an existing element in the list
 *  \param self Pointer to a MTList object
 *  \param searched information of the element to be deleted from the MTList
 *  \param newData New pointer that replace the old pointer
 *
 * This method is used to modify the data pointer of an element from the MTList.
 *
 *  \return Return true if the new pointer is set correctly, false if not.
 */
bool MTList_set( MTList *self, void *searched, void *newData );

/*!
 *  \brief Search an element on the MTList
 *  \param self Pointer to a MTList object
 *  \param cmpFunc Pointer of the comparison element's function
 *  \param searched Pointer to the searched value on the MTList
 *
 * This method is used to search an element on the MTList. The \a cmpFunc will
 * be called for each element on the list as \a cmpFunc (on_list_element, searched_element)
 * until it return 0 to stop the comparison.
 * The cmpFunc must return less than zero, zero or greater than zero
 * if the element is less, equal or greather of the searched one.
 *
 *  \return Return the searched element if found, NULL otherwise.
 *  \see MTList_iterate()
 *  \see MTList_isPresent()
 */
void *MTList_search( MTList *self, int (*cmpFunc)( void *, void * ), void *searched );

/*!
 *  \brief Verify if the pointer to element is present on the MTList
 *  \param self Pointer to a MTList object
 *  \param searched Pointer to the searched element on the MTList
 *
 * This method is used to verify if an element is present on the MTList.
 *
 *  \return Return true if the element is present, false otherwise.
 *  \see MTList_search()
 *  \see MTList_iterate()
 */
bool MTList_isPresent( MTList *self, void *searched );

/*!
 *  \brief Iterate for all the element on the MTList
 *  \param self Pointer to a MTList object
 *  \param func Function called for each element on the list
 *  \param flags Flags for lockup each element
 *
 * This method is used to iterate into all the element on the list. The \a func will
 * be called for each element on the list until it return 0 to stop the iteration.
 *
 * The \a flags param must be MTList_ITERATE_FOR_WRITE or MTList_ITERATE_FOR_READ to
 * lockup each element for read or write.
 *
 *  \return Return the last element passed to \a func, NULL otherwise.
 *  \see MTList_search()
 */
void *MTList_iterate( MTList *self, int (*func)( void * ), int flags );

/*!
 * \brief Fast list walker
 * \param __listHead Pointer to the list
 * \param __lockMode List lock mode
 *
 * This macro walk all the list's elements executing the code betwheen
 * the macros MTLIST_FOREACH_BEGIN/MTLIST_FOREACH_END. The code inside
 * should use the macro \a MTList_FOREACH_ELEMENTPTR in order to access
 * each element's holded data.
 *
 * \see MTLIST_FOREACH_END
 * \see MTLIST_FOREACH_ELEMENTPTR
 *
 */
#define MTLIST_FOREACH_BEGIN( __listHead, __lockMode ) \
{\
  int (*__lockFunc)(RWLock*) = (int (*)(RWLock*))NULL;\
  int __status = 0;\
  MTList *__list = __listHead;\
  MTListElement *__ptr = (MTListElement*)NULL;\
\
  ANY_REQUIRE (__listHead);\
\
  switch (__lockMode)\
  {\
    case MTLIST_ITERATE_FOR_WRITE: __lockFunc = RWLock_writeLock;\
          break;\
\
    case MTLIST_ITERATE_FOR_READ: __lockFunc = RWLock_readLock;\
          break;\
\
    default: ANY_LOG (0, "Invalid __lockMode %d", ANY_LOG_ERROR, __lockMode);\
              ANY_REQUIRE (0);\
          break;\
  }\
\
  RWLock_readLock (__list->rwlock);\
\
  __ptr = __list->first;\
\
  for (__ptr = __list->first; __ptr; __ptr = __ptr->next)\
  {\
    /* lock as flags request the element before to call the func() */\
    __status = (*__lockFunc) (__ptr->rwlock);\
    ANY_REQUIRE (__status == 0);\
\
    {

/*!
 * \brief This macro close the MTLIST_FOREACH_BEGIN
 *
 * \see MTLIST_FOREACH_BEGIN
 */
#define MTLIST_FOREACH_END \
    }\
\
    RWLock_unlock (__ptr->rwlock);\
  }\
  RWLock_unlock (__list->rwlock);\
}

/*!
 * \brief Data reference to a list member. Must be used only inside
 * the \a MTLIST_FOREACH macro list walking
 *
 * \see MTLIST_FOREACH
 *
 */
#define MTLIST_FOREACH_ELEMENTPTR \
        (__ptr->data)

/*!
 * \brief Break the FOREACH loop. Must be used only inside
 * the \a MTLIST_FOREACH macro list walking
 *
 * \see MTLIST_FOREACH
 *
 */
#define MTLIST_FOREACH_BREAK \
    { \
        RWLock_unlock (__ptr->rwlock); \
        break; \
    }

/*!
 * \brief Fast list walker
 * \param __listHead Pointer to the list
 *
 * This macro walk all the list's elements executing the code betwheen
 * the MTLIST_FOREACH_NOLOCK_BEGIN/END. The code inside should use the
 * macro \a MTList_FOREACH_ELEMENTPTR in order to access each element's
 * holded data.
 *
 * \see MTLIST_FOREACH_BEGIN
 *
 */
#define MTLIST_FOREACH_NOLOCK_BEGIN( __listHead ) \
{\
  MTList *__list = __listHead;\
  MTListElement *__ptr = NULL;\
\
  ANY_REQUIRE (__listHead);\
\
  __ptr = __list->first;\
\
  for ( __ptr = __list->first; __ptr; __ptr = __ptr->next )\
  {

/*!
 * \brief Break the FOREACH_NOLOCK loop. Must be used only inside
 * the \a MTLIST_FOREACH_NOLOCK macro list walking
 *
 * \see MTLIST_FOREACH
 *
 */
#define MTLIST_FOREACH_NOLOCK_BREAK \
    { \
        break; \
    }

/*!
 * \brief This macro close the MTLIST_FOREACH_NOLOCK_BEGIN
 *
 * \see MTLIST_FOREACH_NOLOCK_BEGIN
 */
#define MTLIST_FOREACH_NOLOCK_END \
  }\
}

/*!
 *  \brief Return the number of elements on the MTList
 *  \param self Pointer to a MTList object
 *
 * This method return the number of elements on the MTList.
 *
 *  \return Return the number of elements on the MTList.
 *
 */
long MTList_numElements( MTList *self );

/*!
 *  \brief Setup the list's elements deletetion mode
 *  \param self Pointer to a MTList object
 *  \param deleteMode Specify the deletetion mode
 *
 * This method is used to setup the list's element deletetion
 * mode.
 *
 *  \return Nothing
 */
void MTList_setDeleteMode( MTList *self, int deleteMode );

/*!
 * \brief Iterate in each element in the specified MTList
 * \param self Pointer to a MTList object
 * \param current Current element
 *
 * This function iterate in the specified MTList instance, the user can specify
 * the \a current from where to continue the iteration, if NULL than the first
 * element is returned. The operation will be done in readlock mode
 *
 * \return Return the next MTListElement instance or NULL if the MTList is terminated or empty
 *
 * \see MTList_getElementData()
 */
MTListElement *MTList_getNextElement( MTList *self, MTListElement *current );

/*!
 * \brief Iterate in each element in the specified MTList
 * \param self Pointer to a MTList object
 * \param current Current element
 *
 * This function iterate in the specified MTList instance, the user can specify
 * the \a current from where to continue the iteration, if NULL than the first
 * element is returned. The operation will be done without any locking mechanism
 *
 * \return Return the next MTListElement instance or NULL if the MTList is terminated or empty
 *
 * \see MTList_getElementData()
 */
MTListElement *MTList_getNextElementNoLock( MTList *self, MTListElement *current );

/*!
 * \brief Returns the element's stored data
 * \param self Pointer to a MTList object
 * \param element Current element
 *
 * Returns the element's stored data
 *
 * \return  Returns the element's stored data
 *
 * \see MTList_getNextElement()
 */
void *MTList_getElementData( MTList *self, MTListElement *element );

/*!
 *  \brief Clear the MTList object
 *  \param self Pointer to a MTList object
 *
 * This method is used to clear and release all the resource allocated by
 * the MTList_init(). Should be called if the object isn't needed anymore
 * and must called before the MTList_delete(). This method is applicable
 * only to all the objects created by the MTList_new().
 *
 *  \return Nothing
 *
 *  \see MTList_new()
 *  \see MTList_init()
 *  \see MTList_delete()
 */
void MTList_clear( MTList *self );

/*!
 *  \brief Release and destroy all the resource allocated.
 *  \param self Pointer to a MTList object
 *
 * This method is used to release and destroy all the resource allocated by
 * the MTList_new(). Should be called if the object isn't needed anymore.
 *
 *  \return Nothing
 *  \see MTList_new()
 */
void MTList_delete( MTList *self );


#if defined(__cplusplus)
}
#endif

#endif  /* MTLIST_H */


/* EOF */
