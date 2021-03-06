//==========================================================================
//
//      mips-mon.c
//
//      Support code to extend the generic monitor code to support
//      MIPS processors.
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
// Author(s):    dmoseley
// Contributors: dmoseley
// Date:         2000-06-07
// Purpose:      Support code to extend the generic monitor code to support
//               MIPS(R) processors.
// Description:  Further board specific support is in other files.
//               This file contains:
//                 register names lookup table
//
//               Empty Stubs:
//                 Interval timer - This should really belong to the application
//                 operating system.
//
//               Should not contain:
//                 low level uart getchar and putchar functions
//                 delay function to support uart
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include "monitor.h"

struct regstruct regtab[] =
{
    { "zero",      REG_ZERO },
    { "at",        REG_AT },
    { "v0",        REG_V0 },
    { "v1",        REG_V1 },
    { "a0",        REG_A0 },
    { "a1",        REG_A1 },
    { "a2",        REG_A2 },
    { "a3",        REG_A3 },
    { "t0",        REG_T0 },
    { "t1",        REG_T1 },
    { "t2",        REG_T2 },
    { "t3",        REG_T3 },
    { "t4",        REG_T4 },
    { "t5",        REG_T5 },
    { "t6",        REG_T6 },
    { "t7",        REG_T7 },
    { "s0",        REG_S0 },
    { "s1",        REG_S1 },
    { "s2",        REG_S2 },
    { "s3",        REG_S3 },
    { "s4",        REG_S4 },
    { "s5",        REG_S5 },
    { "s6",        REG_S6 },
    { "s7",        REG_S7 },
    { "t8",        REG_T8 },
    { "t9",        REG_T9 },
    { "k0",        REG_K0 },
    { "k1",        REG_K1 },
    { "gp",        REG_GP },
    { "sp",        REG_SP },
    { "s8",        REG_S8 },
    { "ra",        REG_RA },

    { "sr",        REG_SR },
    { "lo",        REG_LO },
    { "hi",        REG_HI },
    { "badvr",     REG_BAD },
    { "cause",     REG_CAUSE },
    { "pc",        REG_PC },
#ifdef CYGHWR_HAL_MIPS_FPU
    { "f0",        REG_F0 },
    { "f1",        REG_F1 },
    { "f2",        REG_F2 },
    { "f3",        REG_F3 },
    { "f4",        REG_F4 },
    { "f5",        REG_F5 },
    { "f6",        REG_F6 },
    { "f7",        REG_F7 },
    { "f8",        REG_F8 },
    { "f9",        REG_F9 },
    { "f10",       REG_F10 },
    { "f11",       REG_F11 },
    { "f12",       REG_F12 },
    { "f13",       REG_F13 },
    { "f14",       REG_F14 },
    { "f15",       REG_F15 },
    { "f16",       REG_F16 },
    { "f17",       REG_F17 },
    { "f18",       REG_F18 },
    { "f19",       REG_F19 },
    { "f20",       REG_F20 },
    { "f21",       REG_F21 },
    { "f22",       REG_F22 },
    { "f23",       REG_F23 },
    { "f24",       REG_F24 },
    { "f25",       REG_F25 },
    { "f26",       REG_F26 },
    { "f27",       REG_F27 },
    { "f28",       REG_F28 },
    { "f29",       REG_F29 },
    { "f30",       REG_F30 },
    { "f31",       REG_F31 },
    { "fcr31",     REG_FCR31 },
#endif /* CYGHWR_HAL_MIPS_FPU */
    { 0, 0 }, /* Terminating element must be last */
} ;

void
initialize_mon(void)
{
} /* initialize_mon */


#include <cyg/hal/hal_arch.h>
#include <bsp/common/bsp_if.h>
int
machine_syscall(HAL_SavedRegisters *regs)
{
    int res, err;
    target_register_t a0, a1, a2, a3;

    a0 = get_register(REG_A0);
    a1 = get_register(REG_A1);
    a2 = get_register(REG_A2);
    a3 = get_register(REG_A3);

    err = _bsp_do_syscall(a0, // Function
                          a1, a2, a3, 0, // arguments,
                          &res);
    if (err)
    {
        // This was a syscall.  It has now been handled, so update the registers appropriately
        put_register(REG_V0, res);
        bsp_skip_instruction(regs);
    }

    return err;
}


// Utility function for printing breakpoints
void bp_print(target_register_t bp_val)
{
    bsp_printf("0x%08lx\n", (unsigned long)bp_val);
}
