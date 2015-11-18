@ ------------------------------------------------------------------------
@ Copyright (c) 2013 TRUSTONIC LIMITED
@ All rights reserved
@
@ The present software is the confidential and proprietary information of
@ TRUSTONIC LIMITED. You shall not disclose the present software and shall
@ use it only in accordance with the terms of the license agreement you
@ entered into with TRUSTONIC LIMITED. This software may be subject to
@ export or import laws in certain countries.
@ ------------------------------------------------------------------------
	.eabi_attribute Tag_ABI_align8_needed, 1   
	.eabi_attribute Tag_ABI_align8_preserved, 1  
	.eabi_attribute Tag_CPU_unaligned_access, 1  

 
    .equ TIMER_DRV_IRQ,                0x23
    .equ TIMER_LOAD,                   0x20     @ ACVR rw
    .equ TIMER_VALUE,                  0x24     @ ACVR ro 
    .equ TIMER_CTRL,                   0x28     @ ACVR rw 
    .equ TIMER_INTCLR,                 0x2c     @ ACVR wo 

    .equ TIMER_CTRL_ONESHOT,           0x01     @1 << 0   CVR
    .equ TIMER_CTRL_32BIT,             0x02     @1 << 1   CVR
    .equ TIMER_CTRL_DIV1,              0x00     @0 << 2  ACVR
    .equ TIMER_CTRL_DIV16,             0x04     @1 << 2  ACVR
    .equ TIMER_CTRL_DIV256,            0x08     @2 << 2  ACVR
    .equ TIMER_CTRL_IE,                0x20     @1 << 5    VR
    .equ TIMER_CTRL_PERIODIC,          0x40     @1 << 6  ACVR
    .equ TIMER_CTRL_ENABLE,            0x80     @1 << 7  ACVR

    .equ TIMER_CTRL_INIT_VALUE,                TIMER_CTRL_32BIT \
                                             |    TIMER_CTRL_ENABLE \
                                             |    TIMER_CTRL_PERIODIC \
                                             |    TIMER_CTRL_IE \
                                             |    TIMER_CTRL_ONESHOT

    .arm


    .section text

	.global	plat_getTimerBase
	.type	plat_getTimerBase, %function
plat_getTimerBase:
        LDR      r0,TIMER_BASE_PHYS
        BX       lr

    .global plat_getIrqNumber   
	.type	plat_getIrqNumber, %function
plat_getIrqNumber:
        MOV      r0,#TIMER_DRV_IRQ
        BX       lr

    .global plat_timerInit 
	.type	plat_timerInit, %function
plat_timerInit:
        LDR      r1,DATA_AREA
        STR      r0,[r1,#0]  @ virt_timer_base
        MOV      r1,#0
        STR      r1,[r0,#TIMER_CTRL]
        MVN      r1,#0
        STR      r1,[r0,#TIMER_LOAD]
        STR      r1,[r0,#TIMER_VALUE]
        MOV      r1,#TIMER_CTRL_INIT_VALUE
        STR      r1,[r0,#TIMER_CTRL]
        LDR      r1,LOAD_VALUE
        STR      r1,[r0,#TIMER_LOAD]
        BX       lr

    .global plat_timerShutdown 
	.type	plat_timerShutdown, %function
plat_timerShutdown:
        LDR      r0,DATA_AREA
        MOV      r1,#0x20
        LDR      r0,[r0,#0]  @ virt_timer_base
        STR      r1,[r0,#TIMER_CTRL]
        MVN      r1,#0
        STR      r1,[r0,#TIMER_LOAD]
        STR      r1,[r0,#TIMER_VALUE]
        BX       lr
       
 
    .global plat_clearInterrupt    
	.type	plat_clearInterrupt, %function
plat_clearInterrupt:
        LDR      r0,DATA_AREA
        MOV      r1,#1
        LDR      r0,[r0,#0]  @ virt_timer_base
        STR      r1,[r0,#TIMER_INTCLR]
        LDR      r1,LOAD_VALUE
        STR      r1,[r0,#TIMER_LOAD]
        BX       lr

TIMER_BASE_PHYS:
    .word      0x10012000
LOAD_VALUE:
    .word      TIMER_LOAD_VALUE       @Timer counter load value
DATA_AREA:
    .word      virt_timer_base
    .comm	virt_timer_base,4,4


    .end

