/*==========================================================================
//
//        kmbox1.cxx
//
//        Kernel Mbox test 1
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
// Author:        dsm
// Contributors:    dsm
// Date:          1998-06-02
// Description:   Tests basic mbox functionality.
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#define NTHREADS 2
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

static cyg_handle_t m0, m1, m2;
static cyg_mbox mbox0, mbox1, mbox2;

static cyg_atomic q = 0;

#ifndef CYGMTH_MBOX_PUT_CAN_WAIT
#define cyg_mbox_PUT cyg_mbox_tryput
#endif

static void entry0( cyg_addrword_t data )
{
    cyg_count8 u,i,j;

    CYG_TEST_INFO("Testing put() and tryput() without wakeup");
    CYG_TEST_CHECK(!cyg_mbox_waiting_to_get(m0), "mbox not initialized properly");
    CYG_TEST_CHECK(0==cyg_mbox_peek(m0), "mbox not initialized properly");
    CYG_TEST_CHECK(NULL==cyg_mbox_peek_item(m0), "mbox not initialized properly");
    cyg_mbox_PUT(m0, (void *)55);
    CYG_TEST_CHECK(1==cyg_mbox_peek(m0), "peek() wrong");
    CYG_TEST_CHECK(55==(cyg_count8)cyg_mbox_peek_item(m0), "peek_item() wrong");
    for(u=1; cyg_mbox_tryput(m0, (void*)u); u++) {
        CYG_TEST_CHECK(55==(cyg_count8)cyg_mbox_peek_item(m0), "peek_item() wrong");
        CYG_TEST_CHECK(u+1==cyg_mbox_peek(m0), "peek() wrong");
    }
    CYG_TEST_CHECK(u == CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE, "mbox not configured size");

    // m0 now contains ( 55 1 2 .. u-1 )
    CYG_TEST_CHECK(u==cyg_mbox_peek(m0), "peek() wrong");
    CYG_TEST_CHECK(55==(cyg_count8)cyg_mbox_peek_item(m0), "peek_item() wrong");

    CYG_TEST_INFO("Testing get(), tryget()");
    
    i = (cyg_count8)cyg_mbox_tryget(m0);
    CYG_TEST_CHECK( 55 == i, "Got wrong message" );
    for(j=1; j<u;j++) {
        CYG_TEST_CHECK( j == (cyg_count8)cyg_mbox_peek_item(m0), "peek_item()" );
        CYG_TEST_CHECK( cyg_mbox_peek(m0) == u - j, "peek() wrong" );
        i = (cyg_count8)cyg_mbox_get(m0);
        CYG_TEST_CHECK( j == i, "Got wrong message" );
    }
    
    CYG_TEST_CHECK( NULL == cyg_mbox_peek_item(m0), "peek_item()" );
    CYG_TEST_CHECK( 0 == cyg_mbox_peek(m0), "peek()");
    
    // m0 now empty

    CYG_TEST_CHECK(!cyg_mbox_waiting_to_put(m0), "waiting_to_put()");
    CYG_TEST_CHECK(!cyg_mbox_waiting_to_get(m0), "waiting_to_get()");

    CYG_TEST_INFO("Testing get(), blocking");
    
    CYG_TEST_CHECK(0==q++, "bad synchronization");
    cyg_mbox_PUT(m1, (void*)99);                  // wakes t1
    i = (cyg_count8)cyg_mbox_get(m0);          // sent by t1
    CYG_TEST_CHECK(3==i, "Recieved wrong message");
    CYG_TEST_CHECK(2==q++, "bad synchronization");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    CYG_TEST_CHECK(NULL==cyg_mbox_timed_get(m0, cyg_current_time()+10),
                   "unexpectedly found message");
    CYG_TEST_CHECK(3==q++, "bad synchronization");
    // Allow t1 to run as this get times out
    // t1 must not be waiting...
    CYG_TEST_CHECK(cyg_mbox_waiting_to_get(m0), "waiting_to_get()");

    cyg_mbox_PUT(m0, (void*)7);                   // wake t1 from timed get
#ifdef CYGMTH_MBOX_PUT_CAN_WAIT
    q=10;
    while(cyg_mbox_tryput(m0, (void*)6))          // fill m0's queue
        ;
    // m0 now contains ( 6 ... 6 )
    CYG_TEST_CHECK(10==q++, "bad synchronization");
    cyg_mbox_put(m1, (void*)4);                   // wake t1
    CYG_TEST_CHECK(!cyg_mbox_timed_put(m0, (void*)8, cyg_current_time()+10),
                   "timed put() unexpectedly worked");
    CYG_TEST_CHECK(12==q++, "bad synchronization");
    // m0 still contains ( 6 ... 6 )
    cyg_mbox_put(m0, (void*)9);
    CYG_TEST_CHECK(13==q++, "bad synchronization");
#endif
#endif
    i=(cyg_count8)cyg_mbox_get(m2);
    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry1( cyg_addrword_t data )
{
    cyg_count8 i;
    i = (cyg_count8)cyg_mbox_get(m1);
    CYG_TEST_CHECK(1==q++, "bad synchronization");
    cyg_mbox_PUT(m0, (void *)3);                  // wake t0

#if defined(CYGFUN_KERNEL_THREADS_TIMER)
    CYG_TEST_INFO("Testing timed functions");
    CYG_TEST_CHECK(7==(cyg_count8)cyg_mbox_timed_get(m0,cyg_current_time()+20),
                   "timed get()");
    CYG_TEST_CHECK(4==q++, "bad synchronization");
#ifdef CYGMTH_MBOX_PUT_CAN_WAIT
    CYG_TEST_CHECK(4==(cyg_count8)cyg_mbox_get(m1));

    CYG_TEST_CHECK(11==q++, "bad synchronization");
    thread[0]->delay(20);    // allow t0 to reach put on m1
    CYG_TEST_CHECK(14==q++, "bad synchronization");
    CYG_TEST_CHECK(cyg_mbox_waiting_to_put(m0), "waiting_to_put()");
    do {
        // after first get m0 contains ( 6 .. 6 9 )
        i=(cyg_count8)cyg_mbox_tryget(m0);
    } while(6==i);
    CYG_TEST_CHECK(9==i,"put gone awry");
#endif
#endif
    CYG_TEST_PASS_FINISH("Kernel C API Mbox 1 OK");
}

void kmbox1_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kmbox1-0",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_thread_create(4, entry1 , (cyg_addrword_t)1, "kmbox1-1",
        (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
    cyg_thread_resume(thread[1]);

    cyg_mbox_create( &m0, &mbox0 );
    cyg_mbox_create( &m1, &mbox1 );
    cyg_mbox_create( &m2, &mbox2 );

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
    kmbox1_main();
}
#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF kmbox1.c */
