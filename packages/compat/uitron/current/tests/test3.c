//===========================================================================
//
//      test3.c
//
//      uITRON "C" test program three
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-03-13
// Purpose:     uITRON API testing
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al
#include <cyg/infra/testcase.h>         // testing infrastructure

#ifdef CYGPKG_UITRON                    // we DO want the uITRON package

#ifdef CYGSEM_KERNEL_SCHED_MLQUEUE      // we DO want prioritized threads

#ifdef CYGFUN_KERNEL_THREADS_TIMER      // we DO want timout-able calls

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK     // we DO want the realtime clock

// we're OK if it's C++ or neither of those two is defined:
#if defined( __cplusplus ) || \
    (!defined( CYGIMP_UITRON_INLINE_FUNCS ) && \
     !defined( CYGIMP_UITRON_CPP_OUTLINE_FUNCS) )

// =================== TEST CONFIGURATION ===================
#if \
    /* test configuration for enough tasks */                      \
    (CYGNUM_UITRON_TASKS >= 4)                                  && \
    (CYGNUM_UITRON_TASKS < 90)                                  && \
    (CYGNUM_UITRON_START_TASKS == 1)                            && \
    ( !defined(CYGPKG_UITRON_TASKS_CREATE_DELETE) ||               \
      CYGNUM_UITRON_TASKS_INITIALLY >= 4             )          && \
                                                                   \
    /* test configuration for enough semaphores */                 \
    defined( CYGPKG_UITRON_SEMAS )                              && \
    (CYGNUM_UITRON_SEMAS >= 3)                                  && \
    (CYGNUM_UITRON_SEMAS < 90)                                  && \
    ( !defined(CYGPKG_UITRON_SEMAS_CREATE_DELETE) ||               \
      CYGNUM_UITRON_SEMAS_INITIALLY >= 3             )          && \
                                                                   \
    /* the end of the large #if statement */                       \
    1 

// ============================ END ============================



#include <cyg/compat/uitron/uit_func.h> // uITRON

int t2done = 0;
int t3done = 0;
int t4done = 0;

externC void
cyg_package_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO( "Calling cyg_uitron_start()" );
    cyg_uitron_start();
}

void task1( unsigned int arg )
{
    ER ercd;
    INT scratch;

    CYG_TEST_INFO( "Task 1 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 1 == scratch, "tid not 1" );
    
    // start lower prio tasks to interact with
    ercd = sta_tsk( 2, 222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = sta_tsk( 3, 333 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = sta_tsk( 4, 444 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );

    // now start the test
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    CYG_TEST_INFO( "T1 awoken" );
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    CYG_TEST_INFO( "T1 signalled" );
    
    // let the others complete, so we get the status back
    ercd = dly_tsk( 50 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    
    CYG_TEST_CHECK( t2done, "t2 not done" );
    CYG_TEST_CHECK( t3done, "t3 not done" );
    CYG_TEST_CHECK( t4done, "t4 not done" );

    CYG_TEST_PASS( "Immediate-dispatch producer/consumer test OK" );

    // all done
    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}



void task2( unsigned int arg )
{
    ER ercd;
    INT scratch;

    CYG_TEST_INFO( "Task 2 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 2 == scratch, "tid not 2" );
    if ( 222 != arg )
        CYG_TEST_FAIL( "Task 2 arg not 222" );

    // now start the test
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    CYG_TEST_INFO( "T2 awoken" );
    ercd = sig_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = wup_tsk( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );

    CYG_TEST_INFO( "T2 completing" );
    t2done++;

    ercd = slp_tsk();
    CYG_TEST_FAIL( "Task 2 sleep came back" );
}

void task3( unsigned int arg )
{
    ER ercd;
    INT scratch;

    CYG_TEST_INFO( "Task 3 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 3 == scratch, "tid not 3" );
    if ( 333 != arg )
        CYG_TEST_FAIL( "Task 3 arg not 333" );

    // now start the test
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    CYG_TEST_INFO( "T3 awoken" );
    ercd = sig_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    
    CYG_TEST_INFO( "T3 completing" );
    t3done++;

    ercd = slp_tsk();
    CYG_TEST_FAIL( "Task 3 sleep came back" );
}

void task4( unsigned int arg )
{
    ER ercd;
    INT scratch;

    CYG_TEST_INFO( "Task 4 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 4 == scratch, "tid not 4" );
    if ( 444 != arg )
        CYG_TEST_FAIL( "Task 4 arg not 444" );

    // now start the test
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );

    CYG_TEST_INFO( "T4 completing" );
    t4done++;

    ercd = slp_tsk();
    CYG_TEST_FAIL( "Task 4 sleep came back" );
}

#else // not enough (or too many) uITRON objects configured in
#define N_A_MSG "not enough uITRON objects to run test"
#endif // not enough (or too many) uITRON objects configured in
#else  // not C++ and some C++ specific options enabled
#define N_A_MSG "C++ specific options selected but this is C"
#endif  // not C++ and some C++ specific options enabled
#else // ! CYGVAR_KERNEL_COUNTERS_CLOCK   - can't test without it
#define N_A_MSG "no CYGVAR_KERNEL_COUNTERS_CLOCK"
#endif // ! CYGVAR_KERNEL_COUNTERS_CLOCK  - can't test without it
#else  // ! CYGFUN_KERNEL_THREADS_TIMER   - can't test without it
#define N_A_MSG "no CYGFUN_KERNEL_THREADS_TIMER"
#endif // ! CYGFUN_KERNEL_THREADS_TIMER   - can't test without it
#else  // ! CYGIMP_THREAD_PRIORITY        - can't test without it
#define N_A_MSG "no CYGSEM_KERNEL_SCHED_MLQUEUE"
#endif // ! CYGSEM_KERNEL_SCHED_MLQUEUE   - can't test without it
#else  // ! CYGPKG_UITRON
#define N_A_MSG "uITRON Compatibility layer disabled"
#endif // CYGPKG_UITRON

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG defined ie. we are N/A.

// EOF test3.c
