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


/*
 * The gcc implementation relies on the atomic builtins as described
 * at: http://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html
 *
 * The MSVC implementation relies on the synchronization function as described
 * at: http://msdn.microsoft.com/en-us/library/ms686360(v=VS.85).aspx
 */


#ifndef ATOMIC_H
#define ATOMIC_H

#include <Any.h>

#if defined(__windows__)
#include <winbase.h>
#include <intrin.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif

#if defined( __GNUC__)

typedef long AnyAtomic;

#if defined(__arm__)
#  if defined(__32BIT__)
typedef long AnyAtomic64;
#  endif
#else
typedef long long AnyAtomic64;
#endif

/* Atomic for 32bits (long/integer) */

#define Atomic_set( __self, __value )\
do {\
  *__self = __value;\
  __sync_synchronize();\
} while( 0 )

#define Atomic_get( __self ) ( __sync_synchronize(), *__self )

#define Atomic_add( __self, __value ) __sync_add_and_fetch( __self, __value )

#define Atomic_subtract( __self, __value ) __sync_sub_and_fetch( __self, __value )

#define Atomic_inc( __self ) __sync_add_and_fetch( __self, 1 )

#define Atomic_dec( __self ) __sync_sub_and_fetch( __self, 1 )

#define Atomic_and( __self, __value ) __sync_and_and_fetch( __self, __value )

#define Atomic_or( __self, __value ) __sync_or_and_fetch( __self, __value )

#define Atomic_xor( __self, __value ) __sync_xor_and_fetch( __self, __value )

#define Atomic_testAndSetValue( __self, __testValue, __newValue ) __sync_val_compare_and_swap( __self, __testValue, __newValue )

#define Atomic_testAndSetBool( __self, __testValue, __newValue ) __sync_bool_compare_and_swap( __self, __testValue, __newValue )

/* Atomic64 for 64bits (longlong) */

#define Atomic64_set( __self, __value ) Atomic_set( __self, __value )

#define Atomic64_get( __self ) Atomic_get( __self )

#define Atomic64_add( __self, __value ) Atomic_add( __self, __value )

#define Atomic64_subtract( __self, __value ) Atomic_subtract( __self, __value )

#define Atomic64_inc( __self ) Atomic_inc( __self )

#define Atomic64_dec( __self ) Atomic_dec( __self )

#define Atomic64_and( __self, __value ) Atomic_and( __self, __value )

#define Atomic64_or( __self, __value ) Atomic_or( __self, __value )

#define Atomic64_xor( __self, __value ) Atomic_xor( __self, __value )

#define Atomic64_testAndSetValue( __self, __testValue, __newValue ) Atomic_testAndSetValue( __self, __testValue, __newValue )

#define Atomic64_testAndSetBool( __self, __testValue, __newValue ) Atomic_testAndSetBool( __self, __testValue, __newValue )

/* AtomicPointer */

#define AtomicPointer_testAndSetValue( __self, __testValue, __newValue ) Atomic_testAndSetValue( __self, __testValue, __newValue )

#define AtomicPointer_testAndSetBool( __self, __testValue, __newValue ) Atomic_testAndSetBool( __self, __testValue, __newValue )

#define AtomicPointer_set( __self, __value ) Atomic_set( __self, __value )

#define AtomicPointer_get( __self ) Atomic_get( __self )


#endif /* __GNUC__ */


/* checks for some microsoft compiler */
#if defined( _MSC_VER )

typedef volatile long AnyAtomic;

#if defined(__64BIT__)
typedef volatile long long AnyAtomic64;
#else
typedef AnyAtomic AnyAtomic64;
#endif

/* Atomic 32bits (long/integer ) */

#define Atomic_set( __self, __value ) _InterlockedExchange( __self, __value )

#define Atomic_get( __self ) ( _ReadWriteBarrier(), *__self )

#define Atomic_add( __self, __value ) _InterlockedExchangeAdd( __self, __value )

#define Atomic_sub( __self, __value ) _InterlockedExchangeAdd( __self, -__value )

#define Atomic_inc( __self ) _InterlockedIncrement( __self )

#define Atomic_dec( __self ) _InterlockedDecrement( __self )

#define Atomic_and( __self, __value ) _InterlockedAnd( __self, __value )

#define Atomic_or( __self, __value ) _InterlockedOr( __self, __value )

#define Atomic_xor( __self, __value ) _InterlockedXor( __self, __value )

#define Atomic_testAndSetValue( __self, __testValue, __newValue ) _InterlockedCompareExchange( __self, __newValue, __testValue )

#define Atomic_testAndSetBool( __self, __testValue, __newValue ) ( _InterlockedCompareExchange( __self, __newValue, __testValue ) == __testValue )

/* Atomic64 64bits (long long) */
#if defined(__64BIT__)

#define Atomic64_set( __self, __value ) _InterlockedExchange64( __self, __value )

#define Atomic64_get( __self ) ( _ReadWriteBarrier(), *__self )

#define Atomic64_add( __self, __value ) _InterlockedExchangeAdd64( __self, __value )

#define Atomic64_sub( __self, __value ) _InterlockedExchangeAdd64( __self, -__value )

#define Atomic64_inc( __self ) _InterlockedIncrement64( __self )

#define Atomic64_dec( __self ) _InterlockedDecrement64( __self )

#define Atomic64_and( __self, __value ) _InterlockedAnd64( __self, __value )

#define Atomic64_or( __self, __value ) _InterlockedOr64( __self, __value )

#define Atomic64_xor( __self, __value ) _InterlockedXor64( __self, __value )

#define Atomic64_testAndSetValue( __self, __testValue, __newValue ) _InterlockedCompareExchange64( __self, __newValue, __testValue )

#define Atomic64_testAndSetBool( __self, __testValue, __newValue ) ( _InterlockedCompareExchange64( __self, __newValue, __testValue ) == __testValue )

#define AtomicPointer_set( __self, __value ) _InterlockedExchangePointer( __self, __value )

#define AtomicPointer_testAndSetValue( __self, __testValue, __newValue ) _InterlockedCompareExchangePointer( __self, __newValue, __testValue )

#define AtomicPointer_testAndSetBool( __self, __testValue, __newValue ) ( _InterlockedCompareExchangePointer( __self, __newValue, __testValue ) == __testValue )

#else /* __64BIT___ */

#define Atomic64_set( __self, __value ) _InterlockedExchange( __self, __value )

#define Atomic64_get( __self ) ( _ReadWriteBarrier(), *__self )

#define Atomic64_add( __self, __value ) _InterlockedExchangeAdd( __self, __value )

#define Atomic64_sub( __self, __value ) _InterlockedExchangeAdd( __self, -__value )

#define Atomic64_inc( __self ) _InterlockedIncrement( __self )

#define Atomic64_dec( __self ) _InterlockedDecrement( __self )

#define Atomic64_and( __self, __value ) _InterlockedAnd( __self, __value )

#define Atomic64_or( __self, __value ) _InterlockedOr( __self, __value )

#define Atomic64_xor( __self, __value ) _InterlockedXor( __self, __value )

#define Atomic64_testAndSetValue( __self, __testValue, __newValue ) _InterlockedCompareExchange( __self, __newValue, __testValue )

#define Atomic64_testAndSetBool( __self, __testValue, __newValue ) ( _InterlockedCompareExchange( __self, __newValue, __testValue ) == __testValue )

#define AtomicPointer_set( __self, __value ) _InterlockedExchange( (AnyAtomic*)__self, (AnyAtomic)__value )

#define AtomicPointer_testAndSetValue( __self, __testValue, __newValue ) _InterlockedCompareExchange( (AnyAtomic*)__self, (AnyAtomic)__newValue, __testValue )

#define AtomicPointer_testAndSetBool( __self, __testValue, __newValue ) ( _InterlockedCompareExchange( (AnyAtomic*)__self, (AnyAtomic)__newValue, __testValue ) == __testValue )

#endif /* __64BIT__ */

/* AtomicPointer */

#define AtomicPointer_get( __self ) ( _ReadWriteBarrier(), *__self )

#endif  /* _MSC_VER */

/* 32bit ARM doesn't supports 64bit memory barriers */
#if defined(__arm__) && defined(__32BIT__)

#undef Atomic64_set
#define Atomic64_set Atomic_set

#undef Atomic64_get
#define Atomic64_get Atomic_get

#undef Atomic64_add
#define Atomic64_add Atomic_add

#undef Atomic64_subtract
#define Atomic64_subtract Atomic_subtract

#undef Atomic64_inc
#define Atomic64_inc Atomic_inc

#undef Atomic64_dec
#define Atomic64_dec Atomic_dec

#undef Atomic64_and
#define Atomic64_and Atomic_and

#undef Atomic64_or
#define Atomic64_or Atomic_or

#undef Atomic64_xor
#define Atomic64_xor Atomic_xor

#undef Atomic64_testAndSetValue
#define Atomic64_testAndSetValue Atomic_testAndSetValue

#undef Atomic64_testAndSetBool
#define Atomic64_testAndSetBool Atomic_testAndSetBool

#endif

#if defined(__cplusplus)
}
#endif


#endif  /* ATOMIC_H */


/* EOF */


