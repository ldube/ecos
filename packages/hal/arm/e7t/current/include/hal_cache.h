#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
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
// Author(s):   nickg, gthomas
// Contributors:        nickg, gthomas
// Date:        1998-09-28
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/hal_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/plf_io.h>

//-----------------------------------------------------------------------------
// Cache dimensions - one unified cache

#define HAL_CACHE_UNIFIED

#define HAL_UCACHE_SIZE                 0x2000   // Size of cache in bytes
#define HAL_UCACHE_LINE_SIZE            16       // Size of a cache line
#define HAL_UCACHE_WAYS                 2        // Associativity of the cache

#define HAL_UCACHE_SETS (HAL_UCACHE_SIZE/(HAL_UCACHE_LINE_SIZE*HAL_UCACHE_WAYS))

//-----------------------------------------------------------------------------
// Global control of cache

// Enable the cache
#define HAL_UCACHE_ENABLE()                     \
    CYG_MACRO_START                             \
    cyg_uint32 syscfg;                          \
    HAL_READ_UINT32(E7T_SYSCFG, syscfg);        \
    syscfg |= E7T_SYSCFG_CE;                    \
    HAL_WRITE_UINT32(E7T_SYSCFG, syscfg);       \
    CYG_MACRO_END

// Disable the cache
#define HAL_UCACHE_DISABLE()                    \
    CYG_MACRO_START                             \
    cyg_uint32 syscfg;                          \
    HAL_READ_UINT32(E7T_SYSCFG, syscfg);        \
    syscfg &= ~E7T_SYSCFG_CE;                   \
    HAL_WRITE_UINT32(E7T_SYSCFG, syscfg);       \
    CYG_MACRO_END

// Invalidate the entire cache
#define HAL_UCACHE_INVALIDATE_ALL()                             \
    CYG_MACRO_START                                             \
    register cyg_uint32* tag = (cyg_uint32*)E7T_CACHE_TAG_ADDR; \
    register int i;                                             \
    for (i = 0; i < HAL_UCACHE_SETS/4; i++) {                   \
        *tag++ = 0;                                             \
        *tag++ = 0;                                             \
        *tag++ = 0;                                             \
        *tag++ = 0;                                             \
    }                                                           \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
// No action necessary. Cache is write-through.
#define HAL_UCACHE_SYNC()

// Query the state of the cache
#define HAL_UCACHE_IS_ENABLED(_state_)                  \
    CYG_MACRO_START                                     \
    cyg_uint32 syscfg;                                  \
    HAL_READ_UINT32(E7T_SYSCFG, syscfg);                \
    (_state_) = (syscfg & E7T_SYSCFG_CE) ? 1 : 0;       \
    CYG_MACRO_END

// Purge contents of cache
#define HAL_UCACHE_PURGE_ALL()  HAL_UCACHE_INVALIDATE_ALL()

// Set the cache refill burst size
//#define HAL_UCACHE_BURST_SIZE(_size_)

// Set the cache write mode
//#define HAL_UCACHE_WRITE_MODE( _mode_ )

//#define HAL_UCACHE_WRITETHRU_MODE       0
//#define HAL_UCACHE_WRITEBACK_MODE       1

// Load the contents of the given address range into the cache
// and then lock the cache so that it stays there.
//#define HAL_UCACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_UCACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_UCACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_UCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
//#define HAL_UCACHE_FLUSH( _base_ , _size_ )

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_UCACHE_INVALIDATE( _base_ , _size_ )

// Write dirty cache lines to memory for the given address range.
//#define HAL_UCACHE_STORE( _base_ , _size_ )

// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_UCACHE_READ_HINT( _base_ , _size_ )

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_UCACHE_WRITE_HINT( _base_ , _size_ )

// Allocate and zero the cache lines associated with the given range.
//#define HAL_UCACHE_ZERO( _base_ , _size_ )

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Data and instruction cache macros map onto the both-cache macros

//-----------------------------------------------------------------------------
// Global control of data cache

#define HAL_DCACHE_SIZE                 HAL_UCACHE_SIZE
#define HAL_DCACHE_LINE_SIZE            HAL_UCACHE_LINE_SIZE
#define HAL_DCACHE_WAYS                 HAL_UCACHE_WAYS
#define HAL_DCACHE_SETS                 HAL_UCACHE_SETS

// Enable the data cache
#define HAL_DCACHE_ENABLE()             HAL_UCACHE_ENABLE()

// Disable the data cache
#define HAL_DCACHE_DISABLE()            HAL_UCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()     HAL_UCACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()               HAL_UCACHE_SYNC()

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)  HAL_UCACHE_IS_ENABLED(_state_)

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

//#define HAL_DCACHE_WRITETHRU_MODE       0
//#define HAL_DCACHE_WRITEBACK_MODE       1

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
//#define HAL_DCACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_DCACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_DCACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
//#define HAL_DCACHE_FLUSH( _base_ , _size_ )

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )

// Write dirty cache lines to memory for the given address range.
//#define HAL_DCACHE_STORE( _base_ , _size_ )

// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_DCACHE_READ_HINT( _base_ , _size_ )

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_DCACHE_WRITE_HINT( _base_ , _size_ )

// Allocate and zero the cache lines associated with the given range.
//#define HAL_DCACHE_ZERO( _base_ , _size_ )

//-----------------------------------------------------------------------------
// Global control of Instruction cache

#define HAL_ICACHE_SIZE                 HAL_UCACHE_SIZE
#define HAL_ICACHE_LINE_SIZE            HAL_UCACHE_LINE_SIZE
#define HAL_ICACHE_WAYS                 HAL_UCACHE_WAYS
#define HAL_ICACHE_SETS                 HAL_UCACHE_SETS

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()             HAL_UCACHE_ENABLE()

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()            HAL_UCACHE_DISABLE()

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()     HAL_UCACHE_INVALIDATE_ALL()


// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()               HAL_UCACHE_SYNC()

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)  HAL_UCACHE_IS_ENABLED(_state_)

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_size_)

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.

//#define HAL_ICACHE_LOCK(_base_, _size_)

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
