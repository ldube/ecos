//===========================================================================
//
//      vsnprintf.cxx
//
//      ANSI Stdio vsnprintf() function
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-02-13
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for this file
#include <errno.h>                  // error codes
#include <cyg/io/devtab.h>          // Device table
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

#include <cyg/libc/stdio/io.inl>     // I/O system inlines

#ifndef CYGPKG_LIBC_STDIO_FILEIO

// FUNCTIONS

static Cyg_ErrNo
str_write(cyg_stdio_handle_t handle, const void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *dev = (cyg_devtab_entry_t *)handle;
    cyg_uint8 **str_p = (cyg_uint8 **)dev->priv;
    cyg_ucount32 i;

    // I suspect most strings passed to vsnprintf will be relatively short,
    // so we just take the simple approach rather than have the overhead
    // of calling memcpy etc.

    // simply copy string until we run out of user space

    for (i = 0; i < *len; i++, (*str_p)++ )
    {
        **str_p = *((cyg_uint8 *)buf + i);
    } // for

    *len = i;

    return ENOERR;
    
} // str_write()

static DEVIO_TABLE(devio_table,
                   str_write,       // write
                   NULL,            // read
                   NULL,            // select
                   NULL,            // get_config
                   NULL);           // set_config

externC int
vsnprintf( char *s, size_t size, const char *format, va_list arg )
{
    int rc;
    // construct a fake device with the address of the string we've
    // been passed as its private data. This way we can use the data
    // directly
    DEVTAB_ENTRY_NO_INIT(strdev,
                         "strdev",       // Name
                         NULL,           // Dependent name (layered device)
                         &devio_table,   // I/O function table
                         NULL,           // Init
                         NULL,           // Lookup
                         &s);            // private
    Cyg_StdioStream my_stream( &strdev, Cyg_StdioStream::CYG_STREAM_WRITE,
                               false, false, _IONBF, 0, NULL );
    
    rc = vfnprintf( (FILE *)&my_stream, size, format, arg );

    // Null-terminate it, but note that s has been changed by str_write(), so
    // that it now points to the end of the string
    s[0] = '\0';

    return rc;

} // vsnprintf()

#else

externC int
vsnprintf( char *s, size_t size, const char *format, va_list arg )
{
    int rc;

    Cyg_StdioStream my_stream( Cyg_StdioStream::CYG_STREAM_WRITE,
                               size, (cyg_uint8 *)s );
    
    rc = vfnprintf( (FILE *)&my_stream, size, format, arg );

    if( rc > 0 )
        s[rc] = '\0';

    return rc;

} // vsnprintf()

#endif

// EOF vsnprintf.cxx
