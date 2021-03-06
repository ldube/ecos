//=============================================================================
//
//      hal_priv.c
//
//      SPARClite Architecture sim-specific private variables
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt
// Date:        1998-12-10
// Purpose:     private vars for SPARClite sim.
//              
//####DESCRIPTIONEND####
//
//=============================================================================


#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arch.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h> // CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
#endif

#include <cyg/hal/hal_clock.h>

// ------------------------------------------------------------------------
// Clock static to keep period recorded.
cyg_int32 cyg_hal_sparclite_clock_period = 0;

/* Address of clock switch */
#define CLKSW_ADDR  0x01000003

#define HAL_SPARC_86940_TIMER1_CONTROL_INIT ( \
                0 |                           \
                HAL_SPARC_86940_TCR_CLKPRS |  \
                HAL_SPARC_86940_TCR_OUTLOW   )
                // Disable, CLKSEL=prescale(2), periodic interrupts, gate 0
                // and lower the output: OUTCTL = 2

#define HAL_SPARC_86940_TIMER1_CONTROL_ENABLE ( \
                HAL_SPARC_86940_TCR_CE |        \
                HAL_SPARC_86940_TCR_CLKPRS |    \
                HAL_SPARC_86940_TCR_OUTSAME   )
                // Enable + .... and no change to the output: OUTCTL = 0

void hal_clock_initialize( cyg_uint32 p )
{
    cyg_uint32 ints;
    cyg_uint32 clk;
    HAL_DISABLE_INTERRUPTS( ints );

    HAL_SPARC_86940_TIMER1_CONTROL_WRITE(
       HAL_SPARC_86940_TIMER1_CONTROL_INIT ); // Make sure it's disabled

    // Clear any pending interrupts:
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_RTC );

    clk = *(unsigned char *)CLKSW_ADDR;
    if (clk & 0x80)
        clk = 10;

    /* could be 10 - 62 MHz */
    clk = (clk & 0x3f);  /* in MHz */
    // BUT the 86940 is internally clocked at half that speed (page 20 RHS)
    clk >>= 1;
    // so the clock is now the number of system ticks we want per counter
    // tick...
    HAL_SPARC_86940_TIMER1_PRESCALER_WRITE(
        0 /* internal clock */    | 
        HAL_SPARC_86940_PRS_ODIV1 |
        clk );
    // should give 1MHz on the externally visible counter...
    HAL_SPARC_86940_TIMER1_CONTROL_WRITE(
                             HAL_SPARC_86940_TIMER1_CONTROL_INIT );
    HAL_SPARC_86940_TIMER1_RELOAD_WRITE( p );
    HAL_SPARC_86940_TIMER1_CONTROL_WRITE(
                             HAL_SPARC_86940_TIMER1_CONTROL_ENABLE );
    HAL_SPARC_86940_TIMER1_RELOAD_WRITE( p );
    cyg_hal_sparclite_clock_period = p;
    HAL_RESTORE_INTERRUPTS( ints );
}

// ------------------------------------------------------------------------

#ifdef CYG_HAL_USE_ROM_MONITOR_CYGMON
#include <cyg/hal/hal_cygm.h>           // CygMon vector table & layout

static struct { int eCosV, CygMonV; } setup[] = {
    { CYGNUM_HAL_VECTOR_OTHERS                , BSP_EXC_TRAP               },
    { CYGNUM_HAL_VECTOR_FETCH_ABORT           , BSP_EXC_IACCESS            },
    { CYGNUM_HAL_VECTOR_ILLEGAL_OP            , BSP_EXC_ILL                },
    { CYGNUM_HAL_VECTOR_PRIV_OP               , BSP_EXC_IPRIV              },
    { CYGNUM_HAL_VECTOR_UNALIGNED             , BSP_EXC_ALIGN              },
    { CYGNUM_HAL_VECTOR_DATA_ABORT            , BSP_EXC_DACCESS            },
};

#endif // CYG_HAL_USE_ROM_MONITOR_CYGMON

// ------------------------------------------------------------------------
// Board specific startups.

extern void hal_board_prestart( void );
extern void hal_board_poststart( void );

#define SLEB_LED (*(volatile char *)(0x02000003))

#define LED( _x_ ) SLEB_LED = (char)(0xff & ~(_x_))

void hal_board_prestart( void )
{

    LED( 0xc0 );

    // Disable default clocks &c
    HAL_SPARC_86940_TIMER1_PRESCALER_WRITE( 1 ); // as if at reset
    HAL_SPARC_86940_TIMER1_CONTROL_WRITE( 0 );   // as if at reset

#ifdef CYG_HAL_USE_ROM_MONITOR_CYGMON
    // then initialize our vectors to point to (or bounce to)
    // CygMon's equivalent functionality.
    {
        extern void hal_user_trap_to_cygmon_vsr( void );
        extern void hal_nofpcp_trap_to_cygmon_vsr( void );
        extern void hal_nmi_handler( void );
        int i;

        HAL_VSR_SET( CYGNUM_HAL_VECTOR_USER_TRAP,
                     (CYG_ADDRESS)hal_user_trap_to_cygmon_vsr, NULL );

        HAL_VSR_SET( CYGNUM_HAL_VECTOR_NOFPCP,
                     (CYG_ADDRESS)hal_nofpcp_trap_to_cygmon_vsr, NULL );

        for ( i = 0; i < sizeof(setup)/sizeof(setup[0]); i++ )
            HAL_VSR_SET( setup[i].eCosV,
                         CYGMON_VECTOR_TABLE[ setup[i].CygMonV ], NULL );

        // Just point this one straight though, and unmask it.
        // That way CygMon looks after ^Cs itself.
        HAL_VSR_SET( CYGNUM_HAL_VECTOR_INTERRUPT_10,
                     CYGMON_VECTOR_TABLE[ BSP_EXC_INT10 ], NULL );
        HAL_INTERRUPT_UNMASK( CYGNUM_HAL_VECTOR_INTERRUPT_10 );

        // Just point this one straight though, and unmask it.
        // That way CygMon looks after the Ethernet itself.
        HAL_VSR_SET( CYGNUM_HAL_VECTOR_INTERRUPT_14,
                     CYGMON_VECTOR_TABLE[ BSP_EXC_INT14 ], NULL );
        HAL_INTERRUPT_UNMASK( CYGNUM_HAL_VECTOR_INTERRUPT_14 );

        // Install handler for the NMI button on the board
        HAL_VSR_SET( CYGNUM_HAL_VECTOR_INTERRUPT_15,
                     (CYG_ADDRESS)hal_nmi_handler, NULL );
        HAL_INTERRUPT_UNMASK( CYGNUM_HAL_VECTOR_INTERRUPT_15 );

        LED( 0xc4 );

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
        {
            extern void patch_dbg_syscalls(void * vector);
            patch_dbg_syscalls( (void *)CYGMON_VECTOR_TABLE );
        }
#endif // CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

    }
#endif // CYG_HAL_USE_ROM_MONITOR_CYGMON

    LED( 0xc8 );
}

void hal_board_poststart( void )
{
    LED( 0xe0 );
    HAL_ENABLE_INTERRUPTS();
    LED( 0xf0 );
}


// EOF hal_priv.c
