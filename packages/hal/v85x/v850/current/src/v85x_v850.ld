//=============================================================================
//
// MLT linker script for NEC
// adapted from packages/hal/arm/pid/v1_1/src/pid.ld
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
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//=============================================================================

#include <pkgconf/system.h>

STARTUP(vectors.o)
// Enter at reset_vector in non-RAM startups so they are more representative
// of actual execution when loaded over ICE
#ifdef CYG_HAL_STARTUP_RAM
ENTRY(start)
#else
ENTRY(reset_vector)
#endif
#ifdef EXTRAS
INPUT(extras.o)
#endif
GROUP(libtarget.a libgcc.a)

#define ALIGN_LMA 4
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

#define SECTIONS_BEGIN                          \
  /* Debug information */                       \
  .debug_aranges  0 : { *(.debug_aranges) }     \
  .debug_pubnames 0 : { *(.debug_pubnames) }    \
  .debug_info     0 : { *(.debug_info) }        \
  .debug_abbrev   0 : { *(.debug_abbrev) }      \
  .debug_line     0 : { *(.debug_line) }        \
  .debug_frame    0 : { *(.debug_frame) }       \
  .debug_str      0 : { *(.debug_str) }         \
  .debug_loc      0 : { *(.debug_loc) }         \
  .debug_macinfo  0 : { *(.debug_macinfo) }

#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { FORCE_OUTPUT; KEEP (*(.vectors)) } \
    > _region_

#define SECTION_ram_vectors(_region_, _vma_, _lma_) \
    .ram_vectors _vma_ : _lma_ \
    { ___ram_vectors_start = ABSOLUTE(.); \
      FORCE_OUTPUT; KEEP (*(.ram_vectors)) \
      ___ram_vectors_end = ABSOLUTE(.); \
    } > _region_ \
    ___ram_vectors_loadaddr = LOADADDR(.ram_vectors);

#define SECTION_text(_region_, _vma_, _lma_) \
    .text _vma_ : _lma_ \
    { stext = ABSOLUTE(.); \
    PROVIDE (__stext = ABSOLUTE(.)); \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) \
    *(.glue_7) *(.glue_7t)  \
    } > _region_ \
    etext = .; PROVIDE (__etext = .);

#define SECTION_fini(_region_, _vma_, _lma_) \
    .fini _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fini) } \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_) \
    .rodata _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata*) } \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_) \
    .rodata1 _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata1) } \
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_) \
    .fixup _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fixup) } \
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_) \
    .gcc_except_table _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.gcc_except_table) } \
    > _region_

#define SECTION_data(_region_,  _vma_, _lma_) \
    .data _vma_ : _lma_ \
    { ___ram_data_start = ABSOLUTE (.); *(.data*) *(.data1); \
    _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.); \
    _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.); \
    . = ALIGN (4); \
    KEEP(*( SORT (.ecos.table.*))) ; \
    ___CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) ___CTOR_END__ = ABSOLUTE (.); \
    ___DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) ___DTOR_END__ = ABSOLUTE (.); \
    *(.dynamic) *(.sdata*) *(.sbss*) } \
    > _region_ \
    ___rom_data_start = LOADADDR (.data); \
    ___ram_data_end = .; PROVIDE (___ram_data_end = .); _edata = .; PROVIDE (edata = .); \
    PROVIDE (___rom_data_end = LOADADDR (.data) + SIZEOF(.data));

#define SECTION_bss(_region_,  _vma_, _lma_) \
    .bss _vma_ : _lma_ \
    { ___bss_start = ABSOLUTE (.); \
    *(.scommon) *(.dynbss) *(.bss*) *(COMMON) \
    ___bss_end = ABSOLUTE (.); } \
    > _region_

#include <cyg/hal/plf_sections.h>

#include <pkgconf/hal_v85x.h>
#include <pkgconf/hal_v85x_v850.h>
#include CYGHWR_MEMORY_LAYOUT_LDI

// EOF v85x_v850.ld
