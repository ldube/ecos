//=================================================================
//
//        intr0.cxx
//
//        Interrupt test 0
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     dsm
// Contributors:  dsm, jlarmour
// Date:          1999-02-16
// Description:   Very basic test of interrupt objects
// Options:
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE_SIZE
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/intr.hxx>
#include <cyg/hal/hal_intr.h>

#include <cyg/infra/testcase.h>

#include "testaux.hxx"

static cyg_ISR isr0, isr1;
static cyg_DSR dsr0, dsr1;

static char intr0_obj[sizeof(Cyg_Interrupt)];
static char intr1_obj[sizeof(Cyg_Interrupt)];

static cyg_uint32 isr0(cyg_vector vector, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);

    Cyg_Interrupt::acknowledge_interrupt(vector);
    return 0;
}

static void dsr0(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(cyg_vector, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
}

static cyg_uint32 isr1(cyg_vector vector, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(cyg_vector, vector);
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
    return 0;
}

static void dsr1(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(cyg_vector, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
}

static bool flash( void )
{
    Cyg_Interrupt intr0 = Cyg_Interrupt(CYGNUM_HAL_ISR_MIN, 0, (CYG_ADDRWORD)333, isr0, dsr0 );

    return true;
}

/* IMPORTANT: The calling convention for VSRs is target dependent.  It is
 * unlikely that a plain C or C++ routine would function correctly on any
 * particular platform, even if it could correctly access the system
 * resources necessary to handle the event that caused it to be called.
 * VSRs usually must be written in assembly language.
 * 
 * This is just a test program.  The routine vsr0() below is defined simply
 * to define an address that will be in executable memory.  If an event
 * causes this VSR to be called, all bets are off.  If it is accidentally
 * installed in the vector for the realtime clock, the system will likely
 * freeze.
 */

static cyg_VSR vsr0;

static void vsr0()
{
}

void intr0_main( void )
{
    CYG_TEST_INIT();

    CHECK(flash());
    CHECK(flash());

    // Make sure the chosen levels are not already in use.
    int in_use;
    cyg_vector lvl1 = CYGNUM_HAL_ISR_MIN + (1 % CYGNUM_HAL_ISR_COUNT);
    HAL_INTERRUPT_IN_USE( lvl1, in_use );
    Cyg_Interrupt* intr0 = NULL;
    if (!in_use)
        intr0 = new((void *)&intr0_obj[0]) Cyg_Interrupt( lvl1, 1, (CYG_ADDRWORD)777, isr0, dsr0 );
     
    cyg_vector lvl2 = CYGNUM_HAL_ISR_MIN + ( 15 % CYGNUM_HAL_ISR_COUNT);
    HAL_INTERRUPT_IN_USE( lvl2, in_use );
    Cyg_Interrupt* intr1 = NULL;
    if (!in_use && lvl1 != lvl2)
        intr1 = new((void *)&intr1_obj[0]) Cyg_Interrupt( lvl2, 1, 888, isr1, dsr1 );

    // Check these functions at least exist
    Cyg_Interrupt::disable_interrupts();
    Cyg_Interrupt::enable_interrupts();

    if (intr0)
        intr0->attach();
    if (intr1)
        intr1->attach();
    if (intr0)
        intr0->detach();
    if (intr1)
        intr1->detach();

    // If this attaching interrupt replaces the previous interrupt
    // instead of adding to it we could be in a big mess if the
    // vector is being used by something important.
        
    cyg_vector v = (CYGNUM_HAL_VSR_MIN + 11) % CYGNUM_HAL_VSR_COUNT;
    cyg_VSR *old_vsr, *new_vsr;
    Cyg_Interrupt::set_vsr( v, vsr0, &old_vsr );
    Cyg_Interrupt::get_vsr( v, &new_vsr );
    CHECK( vsr0 == new_vsr );

    new_vsr = NULL;
    Cyg_Interrupt::set_vsr( v, old_vsr, &new_vsr );
    CHECK( new_vsr == vsr0 );

    Cyg_Interrupt::set_vsr( v, new_vsr );
    new_vsr = NULL;
    Cyg_Interrupt::get_vsr( v, &new_vsr );      
    CHECK( vsr0 == new_vsr );

    Cyg_Interrupt::set_vsr( v, old_vsr );
    CHECK( vsr0 == new_vsr );
    new_vsr = NULL;
    Cyg_Interrupt::get_vsr( v, &new_vsr );
    CHECK( old_vsr == new_vsr );
        
    CHECK( NULL != vsr0 );

    cyg_vector v1;
#ifdef CYGPKG_HAL_MIPS_TX39    
    // This can be removed when PR 17831 is fixed
    if ( cyg_test_is_simulator )
        v1 = 12 % CYGNUM_HAL_ISR_COUNT;
    else /* NOTE TRAILING ELSE... */
#endif
    v1 = CYGNUM_HAL_ISR_MIN + (6 % CYGNUM_HAL_ISR_COUNT);

    Cyg_Interrupt::mask_interrupt(v1);
    Cyg_Interrupt::unmask_interrupt(v1);

    Cyg_Interrupt::configure_interrupt(v1, true, true);

    CYG_TEST_PASS_FINISH("Intr 0 OK");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    intr0_main();
}
// EOF intr0.cxx
