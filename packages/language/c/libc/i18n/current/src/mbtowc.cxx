//===========================================================================
//
//      mbtowc.cxx
//
//      ISO standard mbtowc() routine 
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
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.          
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jjohnstn
// Contributors:  jjohnstn
// Date:          2000-11-01
// Purpose:       Provide ISO C mbtowc() routine
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code was based upon newlib/libc/stdlib/mbtowc.c
// The following is modified from the original newlib description:
//
/*
FUNCTION
<<mbtowc>>---multibyte to wide char converter

INDEX
	mbtowc

ANSI_SYNOPSIS
	#include <stdlib.h>
	int mbtowc(wchar_t *<[pwc]>, const char *<[s]>, size_t <[n]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int mbtowc(<[pwc]>, <[s]>, <[n]>)
	wchar_t *<[pwc]>;
	const char *<[s]>;
	size_t <[n]>;

DESCRIPTION
When CYGINT_LIBC_I18N_MB_REQUIRED is not defined, this is a minimal ANSI-conforming 
implementation of <<mbtowc>>.  In this case,
only ``multi-byte character sequences'' recognized are single bytes,
and they are ``converted'' to themselves.
Each call to <<mbtowc>> copies one character from <<*<[s]>>> to
<<*<[pwc]>>>, unless <[s]> is a null pointer.  The argument n
is ignored.

When CYGINT_LIBC_I18N_MB_REQUIRED is defined, this routine calls the LC_CTYPE locale mbtowc_fn to perform
the conversion, passing a state variable to allow state dependent
decoding.  The result is based on the locale setting which may
be restricted to a defined set of locales.

RETURNS
This implementation of <<mbtowc>> returns <<0>> if
<[s]> is <<NULL>> or is the empty string; 
it returns <<1>> if not CYGINT_LIBC_I18N_MB_REQUIRED or
the character is a single-byte character; it returns <<-1>>
if n is <<0>> or the multi-byte character is invalid; 
otherwise it returns the number of bytes in the multibyte character.
If the return value is -1, no changes are made to the <<pwc>>
output string.  If the input is the empty string, a wchar_t nul
is placed in the output string and 0 is returned.  If the input
has a length of 0, no changes are made to the <<pwc>> output string.

PORTABILITY
<<mbtowc>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<mbtowc>> requires no supporting OS subroutines.
*/


// CONFIGURATION

#include <pkgconf/libc_i18n.h>     // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <locale.h>
#include <stdlib.h>                // Header for this file
#include <string.h>                // strcmp definition
#include <stddef.h>                // size_t definition
#include "internal.h"              // __current_ctype_locale

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif /* CYGSEM_LIBC_I18N_PER_THREAD_MB */

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_I18N_MBTOWC_TRACE_LEVEL)
static int mbtowc_trace = CYGNUM_LIBC_I18N_MBTOWC_TRACE_LEVEL;
# define TL1 (0 < mbtowc_trace)
#else
# define TL1 (0)
#endif

// STATICS

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
# ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
static volatile cyg_ucount32 mbtowc_data_index=CYGNUM_KERNEL_THREADS_DATA_MAX;
static Cyg_Mutex mbtowc_data_mutex CYG_INIT_PRIORITY(LIBC);
# else
static int cyg_libc_mbtowc_last;
# endif
#endif

// FUNCTIONS

int
mbtowc ( wchar_t *pwc, const char *s, size_t n ) 
{
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
  int  *state;
#endif
  int   retval;
  
  CYG_REPORT_FUNCNAMETYPE( "mbtowc", "returning %d" );
  CYG_REPORT_FUNCARG3( "pwc=%08x, s=%08x, n=%ud", pwc, s, n );
  
  if (pwc != NULL)
    CYG_CHECK_DATA_PTR( pwc, "pwc is not a valid pointer!" );
  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
  Cyg_Thread *self = Cyg_Thread::self();
  
  // Get a per-thread data slot if we haven't got one already
  // Do a simple test before locking and retrying test, as this is a
  // rare situation
  if (CYGNUM_KERNEL_THREADS_DATA_MAX==mbtowc_data_index) {
    mbtowc_data_mutex.lock();
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==mbtowc_data_index) {
      
      // the kernel just throws an assert if this doesn't work
      // FIXME: Should use real CDL to pre-allocate a slot at compile
      // time to ensure there are enough slots
      mbtowc_data_index = self->new_data_index();
      
    }
    mbtowc_data_mutex.unlock();
  } // if
  
  // we have a valid index now
  
  state = (int *)self->get_data_ptr(mbtowc_data_index);
#else
  state = &cyg_libc_mbtowc_last;
#endif
  
  CYG_TRACE2( TL1, "Retrieved mbtowc_last address %08x containing %d",
	      state, *state );

  if (__current_ctype_locale->mbtowc_fn)
    {
      retval = __current_ctype_locale->mbtowc_fn (pwc, s, n, state);
      CYG_REPORT_RETVAL( retval );
      return retval;
    }
#endif /* CYGINT_LIBC_I18N_MB_REQUIRED */

  if (s == NULL)
    retval = 0;
  else if (n == 0)
    retval = -1;
  else
    {
      if (pwc)
	*pwc = (wchar_t) *s;
      retval =  (*s != '\0');
    }
  
  CYG_REPORT_RETVAL( retval );
  return retval;
} // mbtowc()

// EOF mbtowc.cxx
