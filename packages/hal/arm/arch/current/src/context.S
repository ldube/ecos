// #===========================================================================
// #
// #    context.S
// #
// #    ARM context switch code
// #
// #===========================================================================
// ####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
// ####COPYRIGHTEND####
// #===========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):    nickg, gthomas
// # Contributors: nickg, gthomas
// # Date:         1998-09-15
// # Purpose:      ARM context switch code
// # Description:  This file contains implementations of the thread context 
// #               switch routines. It also contains the longjmp() and setjmp()
// #               routines.
// #
// #####DESCRIPTIONEND####
// #
// #===========================================================================

#include <pkgconf/hal.h>

#include "arm.inc"

        .text

// ----------------------------------------------------------------------------
//  function declaration macro (start body in ARM mode)
                
#ifdef __thumb__
#define FUNC_START_ARM(_name_, _r_)              \
        .code   16                              ;\
        .thumb_func                             ;\
        .globl _name_                           ;\
_name_:                                         ;\
        ldr     _r_,=_name_ ## _ARM             ;\
        bx      _r_                             ;\
        .code   32                              ;\
_name_ ## _ARM:

#else

#define FUNC_START_ARM(_name_, _r_) \
        .globl _name_; \
_name_: 
        
#endif
        
// ----------------------------------------------------------------------------
//  hal_thread_switch_context
//  Switch thread contexts
//  R0 = address of sp of next thread to execute
//  R1 = address of sp save location of current thread

// Need to save/restore R4..R12, R13 (sp), R14 (lr)

// Note: this is a little wasteful since r0..r3 don't need to be saved.
// They are saved here though so that the information can match the HAL_SavedRegisters
        
FUNC_START_ARM(hal_thread_switch_context, r2)
        mov     ip,sp                   // saved stack pointer
        stmfd   sp!,{fp,ip,lr,pc}       // filler
        stmfd   sp!,{r0-r10,fp,ip,lr,pc}
        mrs     r2,cpsr
        str     r2,[sp,#armreg_cpsr]
        str     sp,[r1]                 // return new stack pointer
#ifdef __thumb__
        b       hal_thread_load_context_ARM // skip mode switch stuff
#endif

//        # Now load the destination thread by dropping through
//        # to hal_thread_load_context
        
// ----------------------------------------------------------------------------
//  hal_thread_load_context
//  Load thread context
//  R0 = address of sp of next thread to execute
//  Note that this function is also the second half of
//  hal_thread_switch_context and is simply dropped into from it.
        
FUNC_START_ARM(hal_thread_load_context, r2)
        ldr     fp,[r0]                 // get context to restore
        mrs     r0,cpsr                 // disable IRQ's
        orr     r0,r0,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        msr     cpsr,r0
        ldr     r0,[fp,#armreg_cpsr]
        msr     spsr,r0
        ldmfd   fp,{r0-r10,fp,sp,lr}
#ifdef __thumb__
        mrs     r1,spsr                 // r1 is scratch 
                                        // [r0 holds initial thread arg]
        msr     cpsr,r1                 // hopefully no mode switch here!
        bx      lr
#else
        movs    pc,lr                   // also restores saved PSR
#endif

// ----------------------------------------------------------------------------
//  HAL longjmp, setjmp implementations
//  hal_setjmp saves only to callee save registers 4-14
//  and lr into buffer supplied in r0[arg0]

FUNC_START_ARM(hal_setjmp, r2)
        stmea   r0,{r4-r14}
        mov     r0,#0
#ifdef __thumb__
        bx      lr
#else
        mov     pc,lr;          # return
#endif

//  hal_longjmp loads state from r0[arg0] and returns
        
FUNC_START_ARM(hal_longjmp, r2)
        ldmfd   r0,{r4-r14}
        mov     r0,r1;          # return [arg1]
#ifdef __thumb__
        bx      lr
#else
        mov     pc,lr
#endif

// ----------------------------------------------------------------------------
//  end of context.S
