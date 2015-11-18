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

#ifndef __DRDRMCOMMON_H__
#define __DRDRMCOMMON_H__


#include "DrApi/DrApiCommon.h"
#include "TlApi/TlApiError.h"


/* Priority definitions */
#define EXCH_PRIORITY       MAX_PRIORITY
#define IPCH_PRIORITY       (MAX_PRIORITY-1)
#define DCIH_PRIORITY       (MAX_PRIORITY-1)


/* Kernel exceptions */
#define TRAP_UNKNOWN        ( 0)    /* unknown exception. */
#define TRAP_SYSCALL        ( 1)    /* invalid syscall number. */
#define TRAP_SEGMENTATION   ( 2)    /* illegal memory access. */
#define TRAP_ALIGNMENT      ( 3)    /* misaligned memory access. */
#define TRAP_UNDEF_INSTR    ( 4)    /* undefined instruction. */

/* System root and SP ids*/
#define ROOTID_SYSTEM       0xFFFFFFFE
#define SPID_SYSTEM         0xFFFFFFFE

/* Driver version (used by the IPC handler) */
#define DRIVER_VERSION       1

/* Thread numbers */
#define DRIVER_THREAD_NO_EXCH      1
#define DRIVER_THREAD_NO_IPCH      2
#define DRIVER_THREAD_NO_DCIH      3



#define THREADID_THREADNO_SHIFT     16
#define THREADID_TASKID_MASK        ((1 << THREADID_THREADNO_SHIFT) - 1)
#define THREADID_THREADNO_MASK      (~THREADID_TASKID_MASK)
#define GET_THREADNO(threadid)      (threadid >> THREADID_THREADNO_SHIFT)


/* For address operations */
#define PTR_OFFS(ptr,offset)   ( (addr_t)( (uint32_t)(ptr) + (offset) ) )

/**
 * Maximum allowed length is 512KB. Some of the available pages
 * are used for mapping various physical memory locations, etc..
 */
#define DR_MAX_INPUT_LENGTH         0x80000


/* Macro and relevat definitions for stack */
#define STACK_DBG_INIT_VALUE        0xCC
#define fillStack(_name_,_val_) \
    memset(_STACK_ARRAY_VAR(_name_), _val_, getStackSize(_name_))
#define clearStack(_name_)      fillStack(_name_,STACK_DBG_INIT_VALUE)
#define TASKID_GET_THREADID(taskid,threadno) ((threadid_t)((taskid) | ((threadno) << THREADID_THREADNO_SHIFT)))
#define FUNC_PTR(func)              VAL2PTR( PTR2VAL( func ) )
#define getStackTop(_name_)         GET_STACK_TOP(_name_)

/* Thread specific definitions */
#define threadid_is_null(threadid)  (NILTHREAD == threadid)  /* returns true if thread id is NILTHREAD */


#endif // __DRDRMCOMMON_H__

