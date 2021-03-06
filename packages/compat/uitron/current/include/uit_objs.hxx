#ifndef CYGONCE_COMPAT_UITRON_UIT_OBJS_HXX
#define CYGONCE_COMPAT_UITRON_UIT_OBJS_HXX
//===========================================================================
//
//      uit_objs.hxx
//
//      uITRON static objects
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
// Purpose:     uITRON static system objects
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

#include <cyg/infra/cyg_type.h>         // types; cyg_int32, CYG_ADDRWORD

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>

#include <cyg/kernel/thread.hxx>        // Cyg_Thread
#include <cyg/kernel/mbox.hxx>          // Cyg_Mbox
#include <cyg/kernel/flag.hxx>          // Cyg_Flag
#include <cyg/kernel/sema2.hxx>         // Cyg_Counting_Semaphore2
#include <cyg/memalloc/memfixed.hxx>    // Cyg_Mempool_Fixed
#include <cyg/memalloc/memvar.hxx>      // Cyg_Mempool_Variable
#include <cyg/kernel/timer.hxx>         // Cyg_Timer

// ------------------------------------------------------------------------
// Some pasting macros to create names of the config macro and the
// static data resulting:

#define CYG_UITRON_NUM( _which_ )  (CYGNUM_UITRON_ ## _which_)
#define CYG_UITRON_OBJS( _which_ )  cyg_uitron_ ## _which_
#define CYG_UITRON_PTRS( _which_ )  cyg_uitron_ ## _which_ ## _ptrs
// ------------------------------------------------------------------------
// CYG_UITRON_DECL
// 
// Macro to declare static uitron static objects; uses the appropriate
// config define for the number of them to have.

#define CYG_UITRON_OBJS_INIT_PRIORITY CYG_INIT_PRIORITY( COMPAT )

#define CYG_UITRON_DECL( _which_ ) \
    CYG_UITRON_OBJS( _which_ ) [ CYG_UITRON_NUM( _which_ ) ] \
    CYG_UITRON_OBJS_INIT_PRIORITY

// and the array of pointers to them for those with dynamic existence:
#define CYG_UITRON_DECL_PTRS( _which_ ) \
    CYG_UITRON_PTRS( _which_ ) [ CYG_UITRON_NUM( _which_ ) ]


// ------------------------------------------------------------------------
// The external system objects themselves.

#ifdef CYGPKG_UITRON_SEMAS
extern
Cyg_Counting_Semaphore2 CYG_UITRON_OBJS( SEMAS )        [];
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
extern
Cyg_Counting_Semaphore2 *CYG_UITRON_PTRS( SEMAS )       [];
#endif
#endif
#ifdef CYGPKG_UITRON_MBOXES
extern
Cyg_Mbox                CYG_UITRON_OBJS( MBOXES )       [];
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
extern
Cyg_Mbox                *CYG_UITRON_PTRS( MBOXES )      [];
#endif
#endif
#ifdef CYGPKG_UITRON_FLAGS
extern
Cyg_Flag                CYG_UITRON_OBJS( FLAGS )        [];
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
extern
Cyg_Flag                *CYG_UITRON_PTRS( FLAGS )       [];
#endif
#endif
// there must always be tasks
extern
Cyg_Thread              CYG_UITRON_OBJS( TASKS )        [];
#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
extern
Cyg_Thread              *CYG_UITRON_PTRS( TASKS )       [];
#endif
// no endif
#ifdef CYGPKG_UITRON_MEMPOOLFIXED
extern
Cyg_Mempool_Fixed       CYG_UITRON_OBJS( MEMPOOLFIXED ) [];
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
extern
Cyg_Mempool_Fixed       *CYG_UITRON_PTRS( MEMPOOLFIXED )[];
#endif
#endif
#ifdef CYGPKG_UITRON_MEMPOOLVAR
extern
Cyg_Mempool_Variable    CYG_UITRON_OBJS( MEMPOOLVAR )   [];
#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
extern
Cyg_Mempool_Variable    *CYG_UITRON_PTRS( MEMPOOLVAR )  [];
#endif
#endif
#ifdef CYGPKG_UITRON_CYCLICS
extern
Cyg_Timer               CYG_UITRON_OBJS( CYCLICS )      [];
#endif
#ifdef CYGPKG_UITRON_ALARMS
extern
Cyg_Timer               CYG_UITRON_OBJS( ALARMS )       [];
#endif

// ------------------------------------------------------------------------
// Ancillary system objects - cleaner than extending the basic class

#ifdef CYGIMP_THREAD_PRIORITY
// An array of priorities, for resetting back to the "created" prio when a
// task cycles though exit, dormancy, restart.
extern cyg_priority
cyg_uitron_task_initial_priorities[ CYG_UITRON_NUM( TASKS ) ];
// and an accessor macro, for the addressing of this is naturally
// from 1..N also:
#define CYG_UITRON_TASK_INITIAL_PRIORITY( _tskid_ ) \
    (cyg_uitron_task_initial_priorities[ (_tskid_) - 1 ])
#endif // CYGIMP_THREAD_PRIORITY

// ------------------------------------------------------------------------

#endif // CYGPKG_UITRON

#endif // CYGONCE_COMPAT_UITRON_UIT_OBJS_HXX
// EOF uit_objs.hxx
