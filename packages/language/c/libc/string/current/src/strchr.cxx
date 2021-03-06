//===========================================================================
//
//      strchr.cxx
//
//      ANSI standard strchr() routine
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
// Author(s):    jlarmour
// Contributors:  
// Date:         2000-04-14
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_string.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <string.h>                // Header for this file
#include <stddef.h>         // Compiler definitions such as size_t, NULL etc.
#include <cyg/libc/string/stringsupp.hxx> // Useful string function support and
                                          // prototypes

// EXPORTED SYMBOLS

externC char *
strchr( const char *s, int c ) CYGBLD_ATTRIB_WEAK_ALIAS(__strchr);

// MACROS

// DETECTCHAR returns nonzero if X contains the byte used 
// to fill MASK.
#define DETECTCHAR(X,MASK) (CYG_LIBC_STR_DETECTNULL( (X) ^ (MASK) ))

// FUNCTIONS

char *
__strchr( const char *s, int c )
{
    CYG_REPORT_FUNCNAMETYPE( "__strchr", "returning %08x" );
    CYG_REPORT_FUNCARG2( "s=%08x, c=%d", s, c );

    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    char charc = c;
    
    while (*s && *s != charc)
    {
        s++;
    }
    
    if (*s != charc)
    {
        s = NULL;
    }
    
    CYG_REPORT_RETVAL( s );

    return (char *) s;
#else
    char charc = c;
    CYG_WORD mask;
    CYG_WORD j;
    CYG_WORD *aligned_addr;
    
    if (CYG_LIBC_STR_UNALIGNED (s))
    {
        while (*s && *s != charc)
            s++;
        if (*s != charc)
            s = NULL;

        CYG_REPORT_RETVAL( s );

        return (char *) s;
    }
    
    for (j = 0, mask = 0; j < sizeof(mask); j++)
        mask = (mask << 8) + charc;
    
    aligned_addr = (CYG_WORD *)s;
    while (!CYG_LIBC_STR_DETECTNULL (*aligned_addr) &&
           !DETECTCHAR (*aligned_addr, mask))
        aligned_addr++;
    
    // The block of bytes currently pointed to by aligned_addr
    // contains either a null or the target char, or both.  We
    // catch it using the bytewise search.
    
    s = (char *)aligned_addr;
    while (*s && *s != charc) 
        s++;
    if (*s != charc)
        s = NULL;

    CYG_REPORT_RETVAL( s );

    return (char *)s;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __strchr()

// EOF strchr.cxx
