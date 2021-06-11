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


#ifndef ANYMEM_H
#define ANYMEM_H

#include <errno.h>
#include <stdlib.h>

#include <AnyDef.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__windows__)
#include <windows.h>
#include <windowsx.h>
#else

#include <alloca.h>

#endif

#ifndef ANY_MALLOC
/*!
 * \brief Allocate "nMemb" times a block of memory of size "size"
 * \param nMemb number of bloks
 * \param size block size
 *
 * This macro should be used to allocate "nMemb" times a block
 * of memory of size "size".
 *
 * Example:
 * \code
 *   myIntVector = (int*)ANY_MALLOC( 10, sizeof( int ) );
 *
 *   myImageVector = (Image*)ANY_MALLOC( 5, sizeof( Image ) );
 * \endcode
 */
#define ANY_MALLOC( nMemb, size ) \
  calloc((nMemb),(size))

#if defined(__windows__)
#undef ANY_MALLOC
#define ANY_MALLOC( nMemb, size ) \
  (void*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ( nMemb ) * ( size ) )
#endif

#endif


#ifndef ANY_BALLOC
/*!
 * \brief Allocate "nMemb" times a block of memory of size 8 bit
 * \param nMemb number of byte (8 bit) to be allocated
 *
 * This macro should be used to allocate "nMemb" times a block
 * of memory of size 8 bit.
 *
 * Example:
 * \code
 *   destStr = (char*)ANY_BALLOC( strlen( srcStr) );
 *
 *   myBuffer = (void*)ANY_BALLOC( filesize );
 * \endcode
 */
#define ANY_BALLOC( nMemb ) \
  calloc((nMemb),sizeof(char))

#if defined(__windows__)
#undef ANY_BALLOC
#define ANY_BALLOC( nMemb ) \
  (void*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ( nMemb ) * sizeof( char ) )
#endif

#endif


#ifndef ANY_TALLOC
/*!
 * \brief Allocate the memory for the type "type" and return a typed pointer
 * \param type type to be allocated
 *
 * This macro should be used to allocate the memory for the type
 * "type" and return a typed pointer.
 *
 * Example:
 * \code
 *   myImagePtr = ANY_TALLOC( Image )
 *
 *   myPtrToMyStruct = ANY_TALLOC( MyStruct );
 * \endcode
 */
#define ANY_TALLOC( type ) \
  (type *)calloc(1,sizeof(type))

#if defined(__windows__)
#undef ANY_TALLOC
#define ANY_TALLOC( type ) \
  (type*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( type ) )
#endif

#endif


#ifndef ANY_TNALLOC
/*!
 * \deprecated Please use \c ANY_NTALLOC instead
 */
#define ANY_TNALLOC( nMemb, type ) \
  (type *)calloc((nMemb),sizeof(type))

#if defined(__windows__)
#undef ANY_TNALLOC
#define ANY_TNALLOC( nMemb, type ) \
  (type*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ( nMemb ) * sizeof( type ))
#endif
#endif


#ifndef ANY_NTALLOC
/*!
 * \brief Allocate "nMemb" times a block of memory for the type "type"
 *        and return a typed pointer
 * \param nMemb number of block type
 * \param type type to be allocated
 *
 * This macro should be used to allocate "nMemb" times a block of
 * memory for the type "type" and return a typed pointer.
 *
 * Example:
 * \code
 *   myImageVector = ANY_NTALLOC( 10, Image )
 *
 *   myMyStructVector = ANY_NTALLOC( 2, MyStruct );
 * \endcode
 */
#define ANY_NTALLOC( nMemb, type ) \
  (type *)calloc((nMemb),sizeof(type))

#if defined(__windows__)
#undef ANY_NTALLOC
#define ANY_NTALLOC( nMemb, type ) \
  (type*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ( nMemb ) * sizeof( type ))
#endif

#endif


#if !defined(__windows__)

void *Any_memAllocAlign( size_t size, size_t alignment );
/*!
 * \deprecated Please use \c ANY_NTALLOC_ALIGN instead
 */
#define ANY_TNALLOC_ALIGN( __num, __type, __alignment )                    \
  (__type*)Any_memAllocAlign( (__num) * sizeof(__type), __alignment )

#endif  /* __windows__ */


#if !defined(__windows__)

void *Any_memAllocAlign( size_t size, size_t alignment );

#define ANY_NTALLOC_ALIGN( __num, __type, __alignment )                    \
  (__type*)Any_memAllocAlign( (__num) * sizeof(__type), __alignment )

#endif  /* __windows__ */


#ifndef ANY_FREE
/*!
 * \brief Release a memory pointer
 * \param ptr pointer to be released
 *
 * This function is used to release the memory allocated with the
 * ANY_*ALLOC macros.
 *
 * Example:
 * \code
 *   ANY_FREE( myImageVector );
 * \endcode
 */
#define ANY_FREE( ptr ) \
  free((void*)(ptr))

#if defined(__windows__)
#undef ANY_FREE
#define ANY_FREE( ptr ) HeapFree( GetProcessHeap(), 0, ptr )
#endif

#endif

#ifndef ANY_FREE_SET
/*!
 * \brief Release a memory pointer and set it to NULL
 * \param ptr pointer to be released
 *
 * This function is used to release the memory allocated with the
 * ANY_*ALLOC macros and set the pointer to NULL.
 *
 * If the pointer is already NULL this macro does nothing.
 *
 * Example:
 * \code
 *   ANY_FREE_SET( myImageVector );
 * \endcode
 */

#if defined(__windows__)

#define ANY_FREE_SET(ptr) \
do {\
   HeapFree( GetProcessHeap(), 0, ptr );         \
   (ptr)=(NULL);\
} while (0)

#else

#define ANY_FREE_SET( ptr ) \
do {\
   if( (ptr) ) \
   { \
     free((void*)(ptr));\
     (ptr)=NULL;\
   } \
} while (0)

#endif

#endif


#ifndef ANY_STACK_MALLOC
/*!
 * \brief Allocate in the stack "nMemb" times a block of memory of size "size"
 * \param nMemb number of bloks
 * \param size block size
 *
 * This macro should be used to allocate "nMemb" times a block
 * of memory of size "size" in the stack.
 *
 * Example:
 * \code
 *   myIntVector = (int*)ANY_STACK_MALLOC( 10, sizeof( int ) );
 *
 *   myImageVector = (Image*)ANY_STACK_MALLOC( 5, sizeof( Image ) );
 * \endcode
 *
 * \note The memory doesn't need to be freed since it's allocated in the stack
 */
#define ANY_STACK_MALLOC( nMemb, size ) \
  alloca((nMemb*size))

#if defined(__windows__)
#undef ANY_STACK_MALLOC
#define ANY_STACK_MALLOC( nMemb, size ) \
  _alloca((nMemb*size))
#endif

#endif


#ifndef ANY_STACK_BALLOC
/*!
 * \brief Allocate "nMemb" times a block of memory of size 8 bit in the stack
 * \param nMemb number of byte (8 bit) to be allocated
 *
 * This macro should be used to allocate "nMemb" times a block
 * of memory of size 8 bit in the stack.
 *
 * Example:
 * \code
 *   destStr = (char*)ANY_BALLOC( strlen( srcStr) );
 *
 *   myBuffer = (void*)ANY_BALLOC( filesize );
 * \endcode
 *
 * \note The memory doesn't need to be freed since it's allocated in the stack
 */
#define ANY_STACK_BALLOC( nMemb ) \
  alloca((nMemb*sizeof(char)))

#if defined(__windows__)
#undef ANY_STACK_BALLOC
#define ANY_STACK_BALLOC( nMemb ) \
  _alloca((nMemb*sizeof(char)))
#endif

#endif


#ifndef ANY_STACK_TALLOC
/*!
 * \brief Allocate in the stack the memory for the type "type" and return a typed pointer
 * \param type type to be allocated
 *
 * This macro should be used to allocate the memory for the type
 * "type" and return a typed pointer. The memory is allocated in the stack
 *
 * Example:
 * \code
 *   myImagePtr = ANY_STACK_TALLOC( Image )
 *
 *   myPtrToMyStruct = ANY_STACK_TALLOC( MyStruct );
 * \endcode
 *
 * \note The memory doesn't need to be freed since it's allocated in the stack
 */
#define ANY_STACK_TALLOC( type ) \
  (type *)alloca(sizeof(type))

#if defined(__windows__)
#undef ANY_STACK_TALLOC
#define ANY_STACK_TALLOC( type ) \
  (type *)_alloca(sizeof(type))
#endif

#endif


#ifndef ANY_STACK_TNALLOC
/*!
 * \brief Allocate "nMemb" times a block of memory in the stack for the type "type"
 *        and return a typed pointer
 * \param nMemb number of block type
 * \param type type to be allocated
 *
 * This macro should be used to allocate "nMemb" times a block of
 * memory in the stack for the type "type" and return a typed pointer.
 *
 * Example:
 * \code
 *   myImageVector = ANY_STACK_TNALLOC( 10, Image )
 *
 *   myMyStructVector = ANY_STACK_TNALLOC( 2, MyStruct );
 * \endcode
 *
 * \note The memory doesn't need to be freed since it's allocated in the stack
 */
#define ANY_STACK_TNALLOC( nMemb, type ) \
  (type *)alloca((nMemb*sizeof(type)))

#if defined(__windows__)
#undef ANY_STACK_TNALLOC
#define ANY_STACK_TNALLOC( nMemb, type ) \
  (type *)_alloca((nMemb*sizeof(type)))
#endif

#endif

#if defined(__cplusplus)
}
#endif

#endif
