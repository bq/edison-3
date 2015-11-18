/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 *
 * This is a wrapper to include standard header files in device drivers.
 *
 */

#ifndef __DRSTD_H__
#define __DRSTD_H__

//==============================================================================
// Standard include files
//==============================================================================

#if defined(__cplusplus)

    #include <cstdint>
    #include <cstdlib>
    #include <cstdio>
    #include <cstring>
    #include <string>
    #include <cstdarg>
    #include <exception>
    using namespace std;

    extern "C" {
        // nothing here
    }

#else

    #include <stdint.h>
    #include <stdbool.h>
    #include <stddef.h>
    //#include <stdlib.h>
    //#include <stdio.h>
    //#include <string.h>
    #include <stdarg.h>

#endif // defined(__cplusplus)

//==============================================================================
// Compiler specific definitions
//==============================================================================

#if defined(__ARMCC__)

    #define _INLINE             __inline__
    #define _SECTION(x)         __attribute__((section(x)))
    #define _UNUSED             __attribute__((unused))
    #define _USED               __attribute__((used))
    #define _NORETURN           __attribute__((noreturn))
    #define _WEAK               __attribute__((weak))

    // The section attribute specifies that a variable must be placed in a
    // particular data section. The zero_init attribute specifies that a
    // variable with no initializer is placed in a ZI data section. If an
    // initializer is specified, an error is reported.
    #define _ZEROINIT           __attribute__((zero_init))
#elif defined(__GNUC__)
    #define _INLINE             __inline__
    #define _SECTION(x)         __attribute__((section(x)))
    #define _UNUSED             __attribute__((unused))
    #define _USED               __attribute__((used))
    #define _NORETURN           __attribute__((noreturn))
    #define _WEAK               __attribute__((weak))
    #define _ZEROINIT
#elif defined(_lint)
    #define _INLINE
    #define _SECTION(x)
    #define _UNUSED
    #define _USED
    #define _NORETURN
    #define _WEAK
    #define _ZEROINIT
#else
    #error "unknown compiler"
#endif

#if defined(__ARMCC__) && defined(DRIVER)
    #pragma import(__use_two_region_memory)
    #pragma import(__use_no_semihosting)
#endif

//==============================================================================
// C/C++ compatibility
//==============================================================================

#if defined(__cplusplus)

    #define _EXTERN_C                extern "C"
    #define _BEGIN_EXTERN_C          extern "C" {
    #define _END_EXTERN_C            }

#else

    #define _EXTERN_C
    #define _BEGIN_EXTERN_C
    #define _END_EXTERN_C

#endif // defined(__cplusplus)


// Thread entry function
#define _THREAD_ENTRY       _EXTERN_C _NORETURN


//==============================================================================
// DRAPI settings
//==============================================================================
#define _DRAPI_EXTERN_C     _EXTERN_C
#define _DRAPI_NORETURN     _NORETURN

#define _TLAPI_EXTERN_C     _DRAPI_EXTERN_C
#define _TLAPI_NORETURN     _DRAPI_NORETURN

//==============================================================================
// Basic types
//==============================================================================

#if !defined(TRUE)
    #define TRUE    (1==1)
#endif // !defined(TRUE)

#if !defined(FALSE)
    #define FALSE   (1!=1)
#endif // !defined(TRUE)


#define DUMMY_FUNCTION()       do{}while(FALSE)


typedef void        *void_ptr;  /**< a pointer to anything. */
typedef void_ptr    addr_t;     /**< an address, can be physical or virtual */
typedef uint32_t    bool_t;     /**< boolean data type. */

//==============================================================================
// Driver entry function
//==============================================================================

#define _DRAPI_ENTRY        _DRAPI_EXTERN_C _DRAPI_NORETURN

_DRAPI_ENTRY void drMain(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
);

//==============================================================================
// Stack utilities
//==============================================================================

typedef uint32_t        stackEntry_t, *stackEntry_ptr;

// calculate the size in byte for the given number of stack entries
#define STACK_ENTRY_COUNT_TO_BYTE SIZE(e)    (e * sizeof(stackEntry_t))
// calculate the number of stack entries require to hold given number of bytes
#define BYTE_SIZE_TO_STACK_ENTRY_COUNT(b)    ((b + sizeof(stackEntry_t) - 1) / sizeof(stackEntry_t))

// internal macros to get variable names from given name. Don't try using the
// variable names directly.
#define _STACK_ARRAY_VAR(_name_)    _stack_##_name_##_array
#define _STACK_ENTRIES_VAR(_name_)  _stack_##_name_##_entries
#define _SECTION_STACK              _SECTION("stack")

// declare a stack. Basically, this is an array in RAM. However, no assumptions
// must be make about how a stack is defined internally. Access functions and
// macros must be used. The actual implementation may change anytime
// Note about the "extern const" declaration, that in C++ variables declared
// "const" have internal linkage. To enforce external linkage, "extern" must be
// given, too. Actually, it is given by _EXTERN_C, but we keep the additional
// "extern" here to remember why we need it.
// _STACK_ARRAY_VAR() has "_USED", because compiler/liker shall never remove
// it. If a stack is declared, it must exist, even if no code references it. It
// may still be needed by a wrapper that set up our binary externally.
// _STACK_ENTRIES_VAR() has "_UNSED" because it my not be referenced at all. It
// is just an internal helper variable.
#if defined(__ARMCC__)
#define DECLARE_STACK(_name_,_size_) \
            _BEGIN_EXTERN_C \
                _USED stackEntry_t             __attribute__((aligned(8))) _STACK_ARRAY_VAR(_name_)[BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_)] _SECTION_STACK _ZEROINIT; \
                _UNUSED extern const uint32_t  _STACK_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_); \
            _END_EXTERN_C
#elif defined(__GNUC__)
#define DECLARE_STACK(_name_,_size_) \
            _BEGIN_EXTERN_C \
                _USED stackEntry_t             __attribute__((aligned(8))) _STACK_ARRAY_VAR(_name_)[BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_)]; \
                _UNUSED const uint32_t  _STACK_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_STACK_ENTRY_COUNT(_size_); \
            _END_EXTERN_C
#else
    #error "unknown compiler"
#endif


// import the references from a stack declares somewhere else
#define EXTERNAL_STACK(_name_) \
            _BEGIN_EXTERN_C \
                extern stackEntry_t    _STACK_ARRAY_VAR(_name_)[]; \
                extern const uint32_t  _STACK_ENTRIES_VAR(_name_); \
            _END_EXTERN_C

#define DR_MAIN_STACK_NAME                            drMain
#define DECLARE_DRIVER_MAIN_STACK(_mainStackSize_)  DECLARE_STACK(DR_MAIN_STACK_NAME, _mainStackSize_)


// get a pointer to the top of a given stack.
#define GET_STACK_TOP(_name_) \
            ( &_STACK_ARRAY_VAR(_name_)[_STACK_ENTRIES_VAR(_name_)] )
#define getStackTop(_name_) GET_STACK_TOP(_name_)

// get a pointer to the bottom of a given stack.
#define GET_STACK_BOTTOM(_name_) \
            ( &_STACK_ARRAY_VAR(_name_)[0] )
#define getStackBottom(_name_) GET_STACK_BOTTOM(_name_)

// get the size of a given stack. cannot use sizeof() here, as only the symbol
// name are know for external stacks.
#define GET_STACK_SIZE(_name_) \
            ( _STACK_ENTRIES_VAR(_name_) * sizeof(stackEntry_t) )
#define getStackSize(_name_) GET_STACK_SIZE(_name_)

//==============================================================================
// Heap utilities
//==============================================================================
#if TBASE_API_LEVEL >= 3

typedef uint32_t        heapEntry_t, *heapEntry_ptr;

// calculate the size in byte for the given number of heap entries
#define HEAP_ENTRY_COUNT_TO_BYTE_SIZE(e)    (e * sizeof(heapEntry_t))
// calculate the number of heap entries require to hold given number of bytes
#define BYTE_SIZE_TO_HEAP_ENTRY_COUNT(b)    ((b + sizeof(heapEntry_t) - 1) / sizeof(heapEntry_t))

// internal macros to get variable names from given name. Don't try using the
// variable names directly.
#define _HEAP_ARRAY_VAR(_name_)    _heap_##_name_##_array
#define _HEAP_ENTRIES_VAR(_name_)  _heap_##_name_##_entries
#define _SECTION_HEAP              _SECTION("heap")

// declare a heap. Basically, this is an array in RAM. However, no assumptions
// must be make about how a heap is defined internally. Access functions and
// macros must be used. The actual implementation may change anytime
// Note about the "extern const" declaration, that in C++ variables declared
// "const" have internal linkage. To enforce external linkage, "extern" must be
// given, too. Actually, it is given by _EXTERN_C, but we keep the additional
// "extern" here to remember why we need it.
// _HEAP_ARRAY_VAR() has "_USED", because compiler/liker shall never remove
// it. If a heap is declared, it must exist, even if no code references it. It
// may still be needed by a wrapper that set up our binary externally.
// _HEAP_ENTRIES_VAR() has "_UNSED" because it my not be referenced at all. It
// is just an internal helper variable.
#if defined(__ARMCC__)
#define DECLARE_HEAP(_name_,_size_) \
            _BEGIN_EXTERN_C \
                _USED heapEntry_t             __attribute__((aligned(8))) _HEAP_ARRAY_VAR(_name_)[BYTE_SIZE_TO_HEAP_ENTRY_COUNT(_size_)] _SECTION_HEAP _ZEROINIT; \
                _USED extern const uint32_t  _HEAP_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_HEAP_ENTRY_COUNT(_size_); \
            _END_EXTERN_C
#elif defined(__GNUC__)
#define DECLARE_HEAP(_name_,_size_) \
            _BEGIN_EXTERN_C \
                _USED heapEntry_t             __attribute__((aligned(8))) _HEAP_ARRAY_VAR(_name_)[BYTE_SIZE_TO_HEAP_ENTRY_COUNT(_size_)]; \
                _USED const uint32_t  _HEAP_ENTRIES_VAR(_name_) = BYTE_SIZE_TO_HEAP_ENTRY_COUNT(_size_); \
            _END_EXTERN_C
#else
    #error "unknown compiler"
#endif


// import the references from a heap declares somewhere else
#define EXTERNAL_HEAP(_name_) \
            _BEGIN_EXTERN_C \
                extern heapEntry_t    _HEAP_ARRAY_VAR(_name_)[]; \
                extern const uint32_t  _HEAP_ENTRIES_VAR(_name_); \
            _END_EXTERN_C

#define DR_MAIN_HEAP_NAME                            drMain
#define DECLARE_DRIVER_MAIN_HEAP(_mainHeapSize_)  DECLARE_HEAP(DR_MAIN_HEAP_NAME, _mainHeapSize_)


// get a pointer to the top of a given heap.
#define GET_HEAP_TOP(_name_) \
            ( &_HEAP_ARRAY_VAR(_name_)[_HEAP_ENTRIES_VAR(_name_)] )
#define getHeapTop(_name_) GET_HEAP_TOP(_name_)

// get a pointer to the bottom of a given heap.
#define GET_HEAP_BOTTOM(_name_) \
            ( &_HEAP_ARRAY_VAR(_name_)[0] )
#define getHeapBottom(_name_) GET_HEAP_BOTTOM(_name_)

// get the size of a given heap. cannot use sizeof() here, as only the symbol
// name are know for external heaps.
#define GET_HEAP_SIZE(_name_) \
            ( _HEAP_ENTRIES_VAR(_name_) * sizeof(heapEntry_t) )
#define getHeapSize(_name_) GET_HEAP_SIZE(_name_)

#endif /* TBASE_API_LEVEL */

//==============================================================================
// Debugging output
//==============================================================================


// This enables debugging output if DEBUG is set. 
// Macros __INCLUDE_DEBUG so avoid having debug output directly connectoed to DEBUG. 
// This allows to have debug output for release builds as well.
#if defined(DEBUG)

    #define __INCLUDE_DEBUG

#endif

#include "DrApi/DrApiLogging.h"

#if defined(__INCLUDE_DEBUG)

    //------------------------------------------------------------------------
    // debug a da data blob
    _EXTERN_C void __dbgBlob(
                        const char    *szDescriptor,
                        const void    *blob,
                        uint32_t      sizeOfBlob);

    //------------------------------------------------------------------------
    // assert message function.
    _EXTERN_C void __dbgAssertMsg(
        const char      *exp,
        const char      *file,
        const uint32_t  line);

    #define dbgC(...)           drDbgPrintf("%c", __VA_ARGS__)
    #define dbgS(...)           drDbgPrintf("%s", __VA_ARGS__)
    #define dbgD(...)           drDbgPrintf("%d", __VA_ARGS__)
    #define dbgB(...)           drDbgPrintf("%x", __VA_ARGS__)
    #define dbgH(...)           drDbgPrintf("%x", __VA_ARGS__)

    #define dbgBlob(...)        __dbgBlob(__VA_ARGS__)
    #define dbgAssertMsg(...)   __dbgAssertMsg(__VA_ARGS__)

#else

    #define dbgC(...)           DUMMY_FUNCTION()
    #define dbgS(...)           DUMMY_FUNCTION()
    #define dbgD(...)           DUMMY_FUNCTION()
    #define dbgB(...)           DUMMY_FUNCTION()
    #define dbgH(...)           DUMMY_FUNCTION()

    #define dbgBlob(...)        DUMMY_FUNCTION()
    #define dbgAssertMsg(...)   DUMMY_FUNCTION()


#endif // __INCLUDE_DEBUG


// C=character, S=String, N=newLine, F=file, L=line,
// D=decimal value, H=hex value, P=pointer
#define dbgN()                  drDbgPrintf("\n")
#define dbgP(p)                 drDbgPrintf("%p", p)
#define dbgDC(d,c)              do{drDbgPrintf("%d%s", d, c);}while(FALSE)
#define dbgHC(h,c)              do{drDbgPrintf("%x%s", h, c);}while(FALSE)
#define dbgSN(s)                do{drDbgPrintf("%s\n", s);}while(FALSE)
#define dbgSD(s,d)              do{drDbgPrintf("%s%d", s, d);}while(FALSE)
#define dbgSDN(s,d)             do{drDbgPrintf("%s%d\n", s, d);}while(FALSE)
#define dbgSH(s,h)              do{drDbgPrintf("%s%x", s, h);}while(FALSE)
#define dbgSHN(s,h)             do{drDbgPrintf("%s%x\n", s, h);}while(FALSE)
#define dbgSHS(s1,h1,s2)        do{drDbgPrintf("%s%x%s", s1, h1, s2);}while(FALSE)
#define dbgSHSHN(s1,h1,s2,h2)   do{drDbgPrintf("%s%x%s%x", s1, h1, s2, h2);}while(FALSE)
#define dbgSP(s,p)              do{drDbgPrintf("%s%p", s, p);}while(FALSE)
#define dbgSPN(s,p)             do{drDbgPrintf("%s%p\n", s, p);}while(FALSE)
#define dbgSPSN(s1,p,s2)        do{drDbgPrintf("%s%p%s\n", s1, p, s2);}while(FALSE)
#define dbgFL(f,l)              do{drDbgPrintf("%s:%d", f,l);}while(FALSE)
#define dbgFLN(f,l)             do{drDbgPrintf("%s:%d\n", f,l);}while(FALSE)



//==============================================================================
// Assert
//==============================================================================

#if defined(_lint)
    #define assert(e) __assert(e)
    //lint -esym(534, __assert) Ignore return value of function.
    static int __assert(int e) { return e ? : 1 : 0; }

#elif defined(DEBUG)

    #define __INCLUDE_ASSERT

#elif defined(NDEBUG) && defined(NDEBUG_WITH_ASSERT)

    #define __INCLUDE_ASSERT

#endif


#ifndef _lint
#if defined(__INCLUDE_ASSERT)

    _NORETURN void _doAssert(
        const char      *expr,
        const char      *file,
        const uint32_t  line
    );


    // we don't use assert() from RVCT, because to take too much space
    // when all assert strings are used.
    // #define ASSERT_FROM_RVCT

    #if defined(ASSERT_FROM_RVCT)

        // if ARM Library's assert.h is used, "__ASSERT_MSG" must be defined
        // that __aeabi_assert() gets called. Our implementation of this
        // function will call doAssert() then. See ARM library documentation
        // for further information.
        #define __ASSERT_MSG
        #include <assert.h>

    #else // use our assert implementation

        #define assert(cond) \
            do{if (!(cond)){_doAssert(NULL,__FILE__, __LINE__);}} while(FALSE)

    #endif // defined(ASSERT_FROM_RVCT)

#else // not __INCLUDE_ASSERT

    #define assert(cond)                DUMMY_FUNCTION()

#endif // [not] __INCLUDE_ASSERT
#endif // lint


#endif //__DRSTD_H__
