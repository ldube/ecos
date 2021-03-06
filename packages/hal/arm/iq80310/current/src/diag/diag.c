//==========================================================================
//
//      diag.c
//
//      Additional RedBoot commands to run board diags.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2000-10-10
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define  DEFINE_VARS
#include <redboot.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/hal_tables.h>

#include "iq80310.h"

int pci_config_cycle = 0;	/* skip exception handling when performing pci config cycle */

static void do_hdwr_diag(int argc, char *argv[]);

RedBoot_cmd("diag", 
            "Run board diagnostics", 
            "",
            do_hdwr_diag
    );


void hdwr_diag (void);

void do_hdwr_diag(int arg, char *argv[])
{
    hal_virtual_comm_table_t* __chan;

    // Turn off interrupts on debug channel.
    // All others should already be disabled.
    __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    if (__chan)
	CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);

    // Reset secondary PCI bus
    *(volatile cyg_uint16 *)BCR_ADDR |= 0x40;  // reset secondary bus
    *(volatile cyg_uint16 *)BCR_ADDR &= ~0x40;  // release reset
    
    hdwr_diag();
}

void __disableDCache(void)
{
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
}

void __enableDCache()
{
    HAL_DCACHE_ENABLE();
}


void _flushICache()
{
    HAL_ICACHE_INVALIDATE_ALL();
}

void __enableICache()
{
    HAL_ICACHE_ENABLE();
}

void __disableICache()
{
    HAL_ICACHE_DISABLE();
}

void _enableFiqIrq()
{
    asm ("mrc p15, 0, r0, c13, c0, 1;"
	 "orr r0, r0, #0x2000;"
	 "mrc p15, 0, r0, c13, c0, 1;"
	 "mrc p13, 0, r0, c0, c0, 0;"
	 "orr		r0, r0, #3;"
	 "mcr	p13, 0, r0, c0, c0, 0;"
	 : : );
}


void _enable_timer()
{
    asm("ldr r1, =0x00000005;"
	"mcr p14, 0, r1, c0, c0, 0 ;"
	: : : "r1" );
}

void _disable_timer()
{
    asm("ldr r1, =0x00000000;"
	"mcr p14, 0, r1, c0, c0, 0 ;"
	: : : "r1" );
}

void _usec_delay()
{
    asm ("ldr	r2, =0x258;"		/* 1 microsec = 600 clocks (600 MHz CPU core) */
	 "0: mrc p14, 0, r0, c1, c0, 0;"	/*read CCNT into r0 */
	 "cmp r2, r0;"	/* compare the current count */
	 "bpl	0b;" /* stay in loop until count is greater */
	 "mrc p14, 0, r1, c0, c0, 0;"
	 "orr	r1, r1, #4;"	/* clear the timer */
	 "mcr p14, 0, r1, c0, c0, 0 ;"
	 : : : "r0","r1","r2");
}

void _msec_delay()
{
    asm ("ldr	r2, =0x927c0;"  /* 1 millisec = 600,000 clocks (600 MHz CPU core) */
	 "0: mrc p14, 0, r0, c1, c0, 0;"	/*read CCNT into r0 */
	 "cmp r2, r0;"	/* compare the current count */
	 "bpl	0b;" /* stay in loop until count is greater */
	 "mrc p14, 0, r1, c0, c0, 0;"
	 "orr	r1, r1, #4;"	/* clear the timer */
	 "mcr p14, 0, r1, c0, c0, 0 ;"
	 : : : "r0","r1","r2");
}

unsigned int _read_timer()
{
    unsigned x;
    asm("mrc p14, 0, %0, c1, c0, 0;" : "=r"(x) : );
    return x;
}

#if 0
FUNC_START _read_intstr
	mrc		p13, 0, r0, c4, c0, 0

	mov     pc, lr
FUNC_END _read_intstr


FUNC_START _read_cpsr
	mrs		r0, cpsr

	mov     pc, lr
FUNC_END _read_cpsr


FUNC_START _cspr_enable_fiq_int
	mrs		r0, cpsr
	bic		r0, r0, #0x40
	msr		cpsr, r0

	mov     pc, lr
FUNC_END _cspr_enable_fiq_int


FUNC_START _cspr_enable_irq_int
	mrs		r0, cpsr
	bic		r0, r0, #0x80
	msr		cpsr, r0

	mov     pc, lr
FUNC_END _cspr_enable_irq_int

#endif
