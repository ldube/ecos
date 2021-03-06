//==========================================================================
//
//      pthread.cxx
//
//      POSIX pthreads implementation
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
// Copyright (C) 2000, 2001 Red Hat, Inc.
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg, jlarmour
// Date:                2000-03-27
// Purpose:             POSIX pthread implementation
// Description:         This file contains the implementation of the POSIX pthread
//                      functions.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/posix.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include "pprivate.h"                  // POSIX private header

#include <cyg/kernel/thread.hxx>       // thread definitions
#include <cyg/kernel/mutex.hxx>        // mutex definitions
#include <cyg/kernel/clock.hxx>        // clock definitions
#include <pthread.h>

#include <cyg/kernel/thread.inl>       // thread inlines

//-----------------------------------------------------------------------------
// new operator to allow us to construct mutex objects

inline void *operator new(size_t size,  cyg_uint8 *ptr) { return (void *)ptr; };

//=============================================================================
// Mutexes

//-----------------------------------------------------------------------------
// Mutex attributes manipulation functions

//-----------------------------------------------------------------------------
// Initialize attribute object

externC int pthread_mutexattr_init ( pthread_mutexattr_t *attr)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    attr->protocol      = PTHREAD_PRIO_NONE;
#ifdef _POSIX_THREAD_PRIO_PROTECT    
    attr->prioceiling   = 0;
#endif
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Destroy attribute object

externC int pthread_mutexattr_destroy ( pthread_mutexattr_t *attr)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    // Nothing to do here...
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Optional functions depending on priority inversion protection options.

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)

// Set priority inversion protection protocol
externC int pthread_mutexattr_setprotocol ( pthread_mutexattr_t *attr,
                                            int protocol)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    switch( protocol )
    {
    case PTHREAD_PRIO_NONE:
#if defined(_POSIX_THREAD_PRIO_INHERIT)        
    case PTHREAD_PRIO_INHERIT:
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT)
    case PTHREAD_PRIO_PROTECT:
#endif        
        attr->protocol = protocol;
        PTHREAD_RETURN(0);

    default:
        PTHREAD_RETURN(EINVAL);
    }
    
    PTHREAD_RETURN(0);
}

// Get priority inversion protection protocol
externC int pthread_mutexattr_getprotocol ( pthread_mutexattr_t *attr,
                                            int *protocol)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    if( protocol != NULL )
        *protocol = attr->protocol;
    
    PTHREAD_RETURN(0);
}

#if defined(_POSIX_THREAD_PRIO_PROTECT)

// Set priority for priority ceiling protocol
externC int pthread_mutexattr_setprioceiling ( pthread_mutexattr_t *attr,
                                               int prioceiling)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    
    attr->prioceiling = prioceiling;
    
    PTHREAD_RETURN(0);
}

// Get priority for priority ceiling protocol
externC int pthread_mutexattr_getprioceiling ( pthread_mutexattr_t *attr,
                                               int *prioceiling)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    if( prioceiling != NULL )
        *prioceiling = attr->prioceiling;
    
    PTHREAD_RETURN(0);
}

// Set priority ceiling of given mutex, returning old ceiling.
externC int pthread_mutex_setprioceiling( pthread_mutex_t *mutex,
                                          int prioceiling,
                                          int *old_ceiling)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(mutex);

    pthread_mutex_lock( mutex );

    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;

    if( old_ceiling != NULL )
        *old_ceiling = mx->get_ceiling();
    
    mx->set_ceiling( prioceiling );
    
    pthread_mutex_unlock( mutex );

    PTHREAD_RETURN(0);
}

// Get priority ceiling of given mutex
externC int pthread_mutex_getprioceiling( pthread_mutex_t *mutex,
                                          int *prioceiling)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(mutex);

    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;

    if( prioceiling != NULL )
        *prioceiling = mx->get_ceiling();
        
    PTHREAD_RETURN(0);
}

#endif // defined(_POSIX_THREAD_PRIO_PROTECT)

#endif // defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)

//-----------------------------------------------------------------------------
// Mutex functions

//-----------------------------------------------------------------------------
// Initialize mutex. If mutex_attr is NULL, use default attributes.

externC int pthread_mutex_init (pthread_mutex_t *mutex,
                                const pthread_mutexattr_t *mutex_attr)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( mutex );
    
    pthread_mutexattr_t use_attr;

    // Set up the attributes we are going to use
    if( mutex_attr == NULL )
        pthread_mutexattr_init( &use_attr );
    else use_attr = *mutex_attr;

    // Now translate the POSIX protocol identifier into the eCos one.
    Cyg_Mutex::cyg_protcol protocol;
    
    switch( use_attr.protocol )
    {
#if defined(_POSIX_THREAD_PRIO_PROTECT)
    case PTHREAD_PRIO_PROTECT:
        protocol = Cyg_Mutex::CEILING;
        break;
#endif
#if defined(_POSIX_THREAD_PRIO_INHERIT)
    case PTHREAD_PRIO_INHERIT:
        protocol = Cyg_Mutex::INHERIT;
        break;
#endif        
    case PTHREAD_PRIO_NONE:
        protocol = Cyg_Mutex::NONE;
        break;

    default:
        PTHREAD_RETURN(EINVAL);
    }

    Cyg_Mutex *mx = new((cyg_uint8 *)mutex) Cyg_Mutex(  protocol );

    mx = mx; // silence compiler warning
#if defined(_POSIX_THREAD_PRIO_PROTECT)
    if ( protocol == Cyg_Mutex::CEILING )
        mx->set_ceiling( use_attr.prioceiling );
#endif
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Destroy mutex.

externC int pthread_mutex_destroy (pthread_mutex_t *mutex)
{
    PTHREAD_ENTRY();

    int err = ENOERR;
    
    PTHREAD_CHECK( mutex );

    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;
    
    if( mx->get_owner() != NULL )
        err = EBUSY;
    else mx->~Cyg_Mutex();
    
    PTHREAD_RETURN(err);
}

//-----------------------------------------------------------------------------
// Lock mutex, waiting for it if necessary.

externC int pthread_mutex_lock (pthread_mutex_t *mutex)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( mutex );

    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;

    if( mx->get_owner() == Cyg_Thread::self() )
        PTHREAD_RETURN(EDEADLK);
    
    // Loop here until we acquire the mutex. Even if we are kicked out
    // of the wait by a signal or release we must retry.
    while( !mx->lock() )
        continue;
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Try to lock mutex.

externC int pthread_mutex_trylock (pthread_mutex_t *mutex)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( mutex );
    
    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;

    if( mx->get_owner() == Cyg_Thread::self() )
        PTHREAD_RETURN(EDEADLK);
    
    if( mx->trylock() )
        PTHREAD_RETURN(0);

    PTHREAD_RETURN(EBUSY);
}


//-----------------------------------------------------------------------------
// Unlock mutex.

externC int pthread_mutex_unlock (pthread_mutex_t *mutex)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( mutex );
    
    Cyg_Mutex *mx = (Cyg_Mutex *)mutex;

    mx->unlock();
    
    PTHREAD_RETURN(0);
}


//=============================================================================
// Condition Variables

//-----------------------------------------------------------------------------
// Attribute manipulation functions
// We do not actually support any attributes at present, so these do nothing.

//-----------------------------------------------------------------------------
// Initialize condition variable attributes

externC int pthread_condattr_init (pthread_condattr_t *attr)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    // There are no condition variable attributes at present
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Destroy condition variable attributes

externC int pthread_condattr_destroy (pthread_condattr_t *attr)
{
    PTHREAD_ENTRY();
    
    PTHREAD_CHECK(attr);

    // nothing to do here...
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Condition variable functions

//-----------------------------------------------------------------------------
// Initialize condition variable.

externC int pthread_cond_init (pthread_cond_t *cond,
                               const pthread_condattr_t *attr)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( cond );

    Cyg_Condition_Variable *cv =
        new((cyg_uint8 *)cond) Cyg_Condition_Variable();

    cv = cv;
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Destroy condition variable.

externC int pthread_cond_destroy (pthread_cond_t *cond)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( cond );

    ((Cyg_Condition_Variable *)cond)->~Cyg_Condition_Variable();
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Wake up one thread waiting for condition variable

externC int pthread_cond_signal (pthread_cond_t *cond)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( cond );

    ((Cyg_Condition_Variable *)cond)->signal();
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Wake up all threads waiting for condition variable

externC int pthread_cond_broadcast (pthread_cond_t *cond)
{
    PTHREAD_ENTRY();

    PTHREAD_CHECK( cond );

    ((Cyg_Condition_Variable *)cond)->broadcast();
    
    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Block on condition variable until signalled. The mutex is
// assumed to be locked before this call, will be unlocked
// during the wait, and will be re-locked on wakeup.

externC int pthread_cond_wait (pthread_cond_t *cond,
                               pthread_mutex_t *mutex)
{
    PTHREAD_ENTRY();

    // check for cancellation first.
    PTHREAD_TESTCANCEL();

    PTHREAD_CHECK( cond );
    PTHREAD_CHECK( mutex );    

    ((Cyg_Condition_Variable *)cond)->wait( *(Cyg_Mutex *)mutex );
    
    // check if we were woken because we were being cancelled
    PTHREAD_TESTCANCEL();

    PTHREAD_RETURN(0);
}

//-----------------------------------------------------------------------------
// Block on condition variable until signalled, or the timeout expires.

externC int pthread_cond_timedwait (pthread_cond_t *cond,
                                    pthread_mutex_t *mutex,
                                    const struct timespec *abstime)
{
    PTHREAD_ENTRY();

    // check for cancellation first.
    PTHREAD_TESTCANCEL();

    PTHREAD_CHECK( cond );
    PTHREAD_CHECK( mutex );    
    PTHREAD_CHECK( abstime );    

    cyg_tick_count ticks;
    struct Cyg_Clock::converter ns_converter, sec_converter;
    
    Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1, &ns_converter );
    Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1000000000, &sec_converter );

    ticks = Cyg_Clock::convert( abstime->tv_sec, &sec_converter );
    ticks += Cyg_Clock::convert( abstime->tv_nsec, &ns_converter );
    
    ((Cyg_Condition_Variable *)cond)->wait( *(Cyg_Mutex *)mutex, ticks );
    
    // check if we were woken because we were being cancelled
    PTHREAD_TESTCANCEL();

    if ( Cyg_Thread::self()->get_wake_reason() == Cyg_Thread::TIMEOUT )
        PTHREAD_RETURN(ETIMEDOUT);
    else
        PTHREAD_RETURN(0);
}

// -------------------------------------------------------------------------
// EOF mutex.cxx
