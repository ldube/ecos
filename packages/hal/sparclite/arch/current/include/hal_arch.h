#ifndef CYGONCE_HAL_ARCH_H
#define CYGONCE_HAL_ARCH_H

//==========================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions
//
//==========================================================================
//####COPYRIGHTBEGIN####
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
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg, gthomas, hmt
// Contributors: nickg, gthomas, hmt
// Date:         1999-02-20
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>
// 
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_intr.h>           // HAL_DISABLE_INTERRUPTS

//--------------------------------------------------------------------------
// Processor saved states:
//
// All these structures must be doubleword (64 bit) aligned.
// The code that creates them on the stack will ensure this is so.

#define HAL_THREAD_CONTEXT_GLOBAL_BASE 0
#define HAL_THREAD_CONTEXT_OUT_BASE    8
#define HAL_THREAD_CONTEXT_LOCAL_BASE 16
#define HAL_THREAD_CONTEXT_IN_BASE    24

typedef struct 
{
    // this is the save structure found at *(stack_ptr) always, note that
    // i[6] is the frame pointer is the previous stack pointer, and
    // o[6] is the stack pointer is the next frame pointer,
    // so they form a linked list back up the call stack.
    cyg_uint32  l[8];                                   /* Locals r16-r23 */
    cyg_uint32  i[8];                                   /* Ins    r24-r31 */
} HAL_SavedWindow;

typedef struct 
{
    // Window save at stack pointer
    HAL_SavedWindow li;
//16
    // This is the rest of the save state:
    //   NOTE: g[0] is used for the CWP, for %g0 == 0.  Also note that the
    //   assembler routines must load/store it in the right order.
    cyg_uint32  g[8] ;                                  /* Globals r0- r7 */
    cyg_uint32  o[8] ;                                  /* Outs    r8-r15 */
//32 words in size

// There is no need to save any other state; for example, condition codes,
// the PC and NextPC, and Y, are preserved in local registers in the trap
// handling window and so preserved in the caller stack frame as viewed
// from an ISR.  Note that the VSR is jumped to with those locals being set
// up (and Y in situ), and it must preserve them itself before calling any
// subsequent handlers (ISRs).

} HAL_SavedRegisters;


typedef struct 
{
    // Window save at stack pointer
    HAL_SavedWindow li;
    cyg_uint32      composite_return_ptr;          /* structure returns */
    cyg_uint32      spill_args[6];                 /* for callee to store */
    cyg_uint32      spare;                         /* keep this 64-bits   */
} HAL_FrameStructure;


//--------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//--------------------------------------------------------------------------
// Bit manipulation macros

#define HAL_LSBIT_INDEX(index, mask)            \
    CYG_MACRO_START                             \
    asm volatile (                              \
        "scan   %1, 0, %%l7;"                   \
        "mov    31, %0;"                        \
        "sub    %0, %%l7, %0"                   \
        : "=r"(index)                           \
        : "r"(mask & ~(mask-1))                 \
        : "l7"                                  \
        );                                      \
CYG_MACRO_END

#define HAL_MSBIT_INDEX(index, mask)            \
    CYG_MACRO_START                             \
    asm volatile (                              \
        "scan   %1, 0, %%l7;"                   \
        "mov    31, %0;"                        \
        "sub    %0, %%l7, %0"                   \
        : "=r"(index)                           \
        : "r"(mask)                             \
        : "l7"                                  \
        );                                      \
CYG_MACRO_END


//--------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

externC CYG_ADDRESS
hal_thread_init_context(  CYG_WORD sparg,
                          CYG_WORD thread,
                          CYG_WORD entry,
                          CYG_WORD id ); 

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )         \
CYG_MACRO_START                                                             \
    _sparg_ = hal_thread_init_context( (CYG_WORD)(_sparg_),                 \
                                       (CYG_WORD)(_thread_),                \
                                       (CYG_WORD)(_entry_),                 \
                                       (CYG_WORD)(_id_) );                  \
CYG_MACRO_END

//---------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
externC void hal_thread_load_context( CYG_ADDRESS to )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context((CYG_ADDRESS)_tspptr_,                \
                                  (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );


//---------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//---------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to happen
// if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and 
// HAL_BREAKINST_SIZE is its size in bytes.

#define HAL_BREAKPOINT(_label_)                \
asm volatile (" .globl  " #_label_ ";"         \
              #_label_":"                      \
              "ta 1"                           \
    );

#define HAL_BREAKINST           {0x91,0xd0,0x20,0x01}
#define HAL_BREAKINST_SIZE      4

//---------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Routines in icontext.c used here because they're quite large for
// the SPARClite (note param order):
externC void
cyg_hal_sparc_get_gdb_regs( void *gdb_regset,
                            HAL_SavedRegisters *eCos_regset );

externC void
cyg_hal_sparc_set_gdb_regs( HAL_SavedRegisters *eCos_regset,
                            void *gdb_regset );


// Copy a set of registers from a HAL_SavedRegisters structure into a GDB
// ordered array.
#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )              \
    CYG_MACRO_START                                             \
        cyg_hal_sparc_get_gdb_regs( (_aregval_), (_regs_) );    \
CYG_MACRO_END

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
    CYG_MACRO_START                                             \
        cyg_hal_sparc_set_gdb_regs( (_regs_), (_aregval_) );    \
CYG_MACRO_END

//---------------------------------------------------------------------------
// HAL setjmp

#define CYGARC_JMP_BUF_SIZE 32 // (words)

// this too must be doubleword aligned (64 bit)

typedef cyg_uint64 hal_jmp_buf[ CYGARC_JMP_BUF_SIZE / 2 ];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//---------------------------------------------------------------------------
// Flush Register Windows
//
// This is implemented as trap 3 in some SPARC systems.
// This macro is only for use from normal, foreground code.
// (including exception handlers and the like)

#define HAL_FLUSH_REGISTERS_TO_STACK()                                      \
    CYG_MACRO_START                                                         \
    cyg_uint32 _saveintr_;                                                  \
    HAL_DISABLE_INTERRUPTS( _saveintr_ ); /* leave traps on */              \
    asm volatile (                                                          \
        /* force out all our callers register sets onto the stack        */ \
        /* if necessary: the system will handily take care of this for   */ \
        /* us as follows:                                                */ \
        "save   %%sp, -16 * 4, %%sp;"   /* need all these to preserve    */ \
        "save   %%sp, -16 * 4, %%sp;"   /* the linked list property...   */ \
        "save   %%sp, -16 * 4, %%sp;"                                       \
        "save   %%sp, -16 * 4, %%sp;"                                       \
        "save   %%sp, -16 * 4, %%sp;"                                       \
        "save   %%sp, -16 * 4, %%sp;"                                       \
        "restore;"                                                          \
        "restore;"                                                          \
        "restore;"                                                          \
        "restore;"                                                          \
        "restore;"                                                          \
        "restore"                                                           \
        /* six of these is correct; a seventh would force out the        */ \
        /* current set that we are using right now.  Note that minimal   */ \
        /* space is allowed on stack for locals and ins in case this     */ \
        /* sequence itself gets interrupted and recurses too deep.       */ \
        :                                                                   \
        :                                                                   \
        : "memory"                                                          \
        );                                                                  \
    HAL_RESTORE_INTERRUPTS( _saveintr_ );                                   \
CYG_MACRO_END

//---------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

//externC void hal_idle_thread_action(cyg_uint32 loop_count);

#ifndef HAL_IDLE_THREAD_ACTION
#define HAL_IDLE_THREAD_ACTION(_count_) \
      /* Cyg_Clock::real_time_clock->tick() */
#endif

//---------------------------------------------------------------------------

// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// We define quite large stack needs for SPARClite, for it requires 576
// bytes (144 words) to process an interrupt and thread-switch, and
// momentarily, but needed in case of recursive interrupts, it needs 208
// words - if a sequence of saves to push out other regsets is interrupted.

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// A minimal, optimized stack frame is 24 words, but even -O2 code seems to
// place a few locals in the locals area: round this up to provide a
// sensible overestimate:
#define CYGNUM_HAL_STACK_FRAME_SIZE (4 * 32)

// Stack needed for a context switch: this is implicit in the estimate for
// interrupts so not explicitly used below:
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * 32)

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE \
    ((208 * 4) + 2 * CYGNUM_HAL_STACK_FRAME_SIZE)

// And we have lots of registers so no particular amount is added in for
// typical local variable usage.

// Typically we have 4 nestable interrupt sources, clock, serialin,
// serialout, (and NMI button, but you want it to not destroy context):

#define CYGNUM_HAL_STACK_SIZE_MINIMUM \
        (4 * CYGNUM_HAL_STACK_INTERRUPT_SIZE + 2 * CYGNUM_HAL_STACK_FRAME_SIZE)

#define CYGNUM_HAL_STACK_SIZE_TYPICAL \
        (CYGNUM_HAL_STACK_SIZE_MINIMUM + 8 * CYGNUM_HAL_STACK_FRAME_SIZE)

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//-----------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARCH_H
// End of hal_arch.h
