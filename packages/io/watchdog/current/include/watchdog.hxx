#ifndef CYGONCE_DEVS_WATCHDOG_HXX
#define CYGONCE_DEVS_WATCHDOG_HXX

//==========================================================================
//
//      watchdog.hxx
//
//      Watchdog interface declaration
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-07-14
// Purpose:     Watchdog declarations
// Description: This file defines the interface to the watchdog device
//              that provides timer based recovery from software and
//              hardware faults.
// Usage:       #include <cyg/devs/watchdog.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros

class Cyg_Watchdog_Action;

// -------------------------------------------------------------------------
// Watchdog class

class Cyg_Watchdog
{

    Cyg_Watchdog_Action         *action_list;

    cyg_uint64                  resolution;
    
public:

    Cyg_Watchdog();

    // Return time interval allowed between resets before watchdog
    // triggers, in nanoseconds.
    cyg_uint64 get_resolution( void );
    
    // Start the watchdog running.
    void start( void );

    // Reset watchdog timer. This needs to be called regularly to prevent
    // the watchdog firing.
    void reset( void );

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT    
    // Trigger the watchdog as if the timer had expired.
    void trigger( void );

    // Register an action routine that will be called when the timer
    // triggers.
    void install_action( Cyg_Watchdog_Action *wdaction );

    // Deregister a previously registered action routine.
    void uninstall_action( Cyg_Watchdog_Action *wdaction );
#endif
    
    // A static instance of the single system defined watchdog device.
    static Cyg_Watchdog watchdog;

private:
    void init_hw( void );
};

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT    
// -------------------------------------------------------------------------
// Watchdog action class

class Cyg_Watchdog_Action
{
    friend class Cyg_Watchdog;

    Cyg_Watchdog_Action         *next;          // link in chain
    
    void (*action)( CYG_ADDRWORD data );        // action function

    CYG_ADDRWORD data;                          // data argument
    
public:

    Cyg_Watchdog_Action(
        void (*action)( CYG_ADDRWORD data ),
        CYG_ADDRWORD data
        );

    ~Cyg_Watchdog_Action();

    void install();
    
    void uninstall();
};

// -------------------------------------------------------------------------
// Cyg_Watchdog_Action inlines

inline Cyg_Watchdog_Action::Cyg_Watchdog_Action(
    void (*action_arg)( CYG_ADDRWORD data ),
    CYG_ADDRWORD data_arg
    )
{
    next        = NULL;
    action      = action_arg;
    data        = data_arg;
}

inline Cyg_Watchdog_Action::~Cyg_Watchdog_Action()
{
    Cyg_Watchdog::watchdog.uninstall_action( this );
}

inline void Cyg_Watchdog_Action::install()
{
    Cyg_Watchdog::watchdog.install_action( this );    
}

inline void Cyg_Watchdog_Action::uninstall()
{
    Cyg_Watchdog::watchdog.uninstall_action( this );    
}

#endif // CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_DEVS_WATCHDOG_HXX
// EOF watchdog.hxx
