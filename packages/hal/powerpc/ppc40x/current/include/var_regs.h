#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC 40x variant CPU definitions
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
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov,gthomas
// Date:         2000-08-27
// Purpose:      Provide PPC40x register definitions
// Description:  Provide PPC40x register definitions
//               The short difinitions (sans CYGARC_REG_) are exported only
//               if CYGARC_HAL_COMMON_EXPORT_CPU_MACROS is defined.
// Usage:        Included via the acrhitecture register header:
//               #include <cyg/hal/ppc_regs.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
// Cache
#define CYGARC_REG_HID0   1008

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define HID0       CYGARC_REG_HID0
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// MMU control.
#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#define SPR_EVPR        982            // Exception vector prefix
#define SPR_PID         945            // Process ID

#define M_EPN_EPNMASK  0xfffff000      // effective page no mask
#define M_EPN_EV       0x00000040      // entry valid
#define M_EPN_SIZE(n)  (n<<7)          // entry size (0=1K, 1=4K, ... 7=16M)

#define M_RPN_RPNMASK  0xfffff000      // real page no mask
#define M_RPN_EX       0x00000200      // execute enable
#define M_RPN_WR       0x00000100      // write enable
#define M_RPN_W        0x00000008      // write-through (when cache enabled)
#define M_RPN_I        0x00000004      // cache inhibited
#define M_RPN_M        0x00000002      // memory coherent (not implemented)
#define M_RPN_G        0x00000001      // guarded

#define CYGARC_TLBWE(_id_, _hi_, _lo_) \
        asm volatile ("tlbwe %1,%0,0; tlbwe %2,%0,1" :: "r"(_id_), "r"(_hi_), "r"(_lo_));

#define CYGARC_TLBRE(_id_, _hi_, _lo_) \
        asm volatile ("tlbre %0,%2,0; tlbre %1,%2,1" : "=r"(_hi_), "=r"(_lo_) : "r"(_id_));

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// Device control register access macros.
#define CYGARC_MTDCR(_tbr_, _v_) \
    asm volatile ("mtdcr %0, %1;" :: "I" (_tbr_), "r" (_v_));
#define CYGARC_MFDCR(_tbr_, _v_) \
    asm volatile ("mfdcr %0, %1;" : "=r" (_v_) : "I" (_tbr_));

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

// Interrupt control (device) registers
#define DCR_EXIER  66
#define DCR_EXISR  64
#define DCR_IOCR   160

// Timer control (special) registers
#define SPR_PIT    987
#define SPR_TCR    986
#define SPR_TSR    984

// Interval and watchdog timer control
#define TCR_WP          0xC0000000  // Watchdog timer period
#define TCR_WP_17       0x00000000  //   2^17 clocks
#define TCR_WP_21       0x40000000  //   2^21 clocks
#define TCR_WP_25       0x80000000  //   2^25 clocks
#define TCR_WP_29       0xC0000000  //   2^29 clocks
#define TCR_WRC         0x30000000  // Reset control
#define TCR_WRC_None    0x00000000  //   No reset on timeout
#define TCR_WRC_Core    0x10000000  //   Reset core on timeout
#define TCR_WRC_Chip    0x20000000  //   Reset chip on timeout
#define TCR_WRC_System  0x30000000  //   Reset system on timeout
#define TCR_WIE         0x08000000  // Watchdog interrupt enable
#define TCR_PIE         0x04000000  // Programmable timer interrupt
#define TCR_FP          0x03000000  // Fixed timer interval
#define TCR_FP_9        0x00000000  //   2^9 clocks
#define TCR_FP_13       0x01000000  //   2^13 clocks
#define TCR_FP_17       0x02000000  //   2^17 clocks
#define TCR_FP_21       0x03000000  //   2^21 clocks
#define TCR_FIE         0x00800000  // Fixed timer interrupt
#define TCR_ARE         0x00400000  // Auto-reload enable

// Interval and watchdog status
#define TSR_ENW         0x80000000  // Enable next watchdog
#define TSR_WIS         0x40000000  // Watchdog interrupt pending
#define TSR_WRS         0x30000000  // Watchdog reset state
#define TSR_WRS_None    0x00000000  //   No watchdog reset
#define TSR_WRS_Core    0x10000000  //   Core reset by watchdog
#define TSR_WRS_Chip    0x20000000  //   Chip reset by watchdog
#define TSR_WRS_System  0x30000000  //   System reset by watchdog
#define TSR_PIS         0x08000000  // Programmable timer interrupt
#define TSR_FIS         0x04000000  // Fixed timer interrupt

// Debug registers
#define SPR_DBSR        1008
#define SPR_DBCR        1010

#define DBCR_IDM        0x40000000  // Internal debug enable
#define DBCR_IC         0x08000000  // Instruction completion

#endif //  CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
