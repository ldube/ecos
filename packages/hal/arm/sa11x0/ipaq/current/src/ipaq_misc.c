//==========================================================================
//
//      ipaq_misc.c
//
//      HAL misc board support code for StrongARM SA1110/iPAQ
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
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.
// All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: hmt
//               Travis C. Furrer <furrer@mit.edu>
//               Richard Panton <richard.panton@3glab.com>
// Date:         2001-02-24
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // Virtual vector interfaces
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_sa11x0.h>         // Hardware definitions
#include <cyg/hal/ipaq.h>               // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/lcd_support.h>
#include <cyg/hal/atmel_support.h>

// All the MM table layout is here:
#include <cyg/hal/hal_mm.h>

void
hal_mmu_init(void)
{
    unsigned long ttb_base = SA11X0_RAM_BANK0_BASE + 0x4000;
    unsigned long i;

    /*
     * Set the TTB register
     */
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_DACR_DEFAULT;
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*               Actual  Virtual  Size   Attributes                                                    Function  */
    /*		     Base     Base     MB      cached?           buffered?        access permissions                 */
    /*             xxx00000  xxx00000                                                                                */
    X_ARM_MMU_SECTION(0x000,  0x500,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Boot flash ROMspace */
    X_ARM_MMU_SECTION(0x080,  0x080,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Application flash ROM */
    X_ARM_MMU_SECTION(0x100,  0x100,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SA-1101 Development Board Registers */
    X_ARM_MMU_SECTION(0x180,  0x180,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Ethernet Adaptor */
    X_ARM_MMU_SECTION(0x184,  0x184,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* XBusReg    */
    X_ARM_MMU_SECTION(0x188,  0x188,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SysRegA    */
    X_ARM_MMU_SECTION(0x18C,  0x18C,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SysRegB    */
    X_ARM_MMU_SECTION(0x190,  0x190,     4,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CPLD A     */
    X_ARM_MMU_SECTION(0x194,  0x194,     4,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CPLD B     */
    X_ARM_MMU_SECTION(0x200,  0x200,   512,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA Socket A */
    X_ARM_MMU_SECTION(0x300,  0x300,   512,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA Sockets B */
    X_ARM_MMU_SECTION(0x400,  0x400,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SA1111 Daughter card */
    X_ARM_MMU_SECTION(0x480,  0x480,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Video Controller Daughter card */
    X_ARM_MMU_SECTION(0x800,  0x800, 0x400,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* StrongARM(R) Registers */
    X_ARM_MMU_SECTION(0xC00,  0x000,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xC00,  0xC00,    32,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xE00,  0xE00,   128,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Zeros (Cache Clean) Bank */

}


static void h3800_ctrl(cyg_uint32 mask, cyg_uint32 value)
{
  cyg_uint32 new_value = PIOC_DATA;
  cyg_uint32 new_mask = 0;
  if (mask & SA1110_EIO_LCD_3V3)
  {
    new_mask |= H3800_LCD_ON;
  }
  if (mask & SA1110_EIO_MUTE)
  {
    new_mask |= (H3800_SPKR_ON | H3800_EAR_OFF);
  }
  if (mask & SA1110_EIO_AUDIO)
  {
    new_mask |= H3800_AUD_PWRON;
  }
  new_value &= ~new_mask;
  value &= mask;

  if (value & SA1110_EIO_LCD_3V3)
  {
    new_value |= H3800_LCD_ON;
  }
  if (value & SA1110_EIO_MUTE_ON)
  {
    new_value |= H3800_EAR_OFF; // SPKR_ON is already clear
  }
  else
  {
#if 1
    new_value &= ~(H3800_SPKR_ON | H3800_EAR_OFF); // ear on
#else
    new_value |= (H3800_SPKR_ON | H3800_EAR_OFF); // spk on
#endif
  }
  if (value & SA1110_EIO_AUDIO_ON)
  {
    A2_CLOCK_EN &= ~0xF;
    A2_CLOCK_EN |= (A2_CLK_EX1 | A2_CLK_AUD_3);
    new_value |= H3800_AUD_PWRON;
  }
  PIOC_DATA = new_value;
}

//
// Board control register support
//   Update the board control register (write only).  Only the bits
// specified by 'mask' are changed to 'value'.
//

void
ipaq_EGPIO(unsigned long mask, unsigned long value)
{
  model_t model = get_model();
  if (model == MODEL_H3600)
  {
    _ipaq_EGPIO = (_ipaq_EGPIO & ~mask) | (mask & value);
    *SA1110_EGPIO = _ipaq_EGPIO;
  }
  else if (model == MODEL_H3800)
  {
    cyg_uint32 new_value = PIOA_DATA;
    cyg_uint32 new_mask = 0;
    if (mask & H38_A1_MASK)
    {
      h3800_ctrl(mask, value);
      return;
    }

    if (mask & SA1110_EIO_OPT_PWR)
    {
      new_mask |= H3800_OPT_PWR;
    }
    if (mask & SA1110_EIO_OPT)
    {
      new_mask |= H3800_OPT;
    }
    if (mask & SA1110_EIO_CF_RESET)
    {
      new_mask |= H3800_CF_RESET;
    }
    if (mask & SA1110_EIO_VPP)
    {
      A2_FLASH_CTRL = (value & SA1110_EIO_VPP_ON) ? 1 : 0;
      return;
    }

    new_value &= ~new_mask;
    value &= mask;

    if (value & SA1110_EIO_CF_RESET_ENABLE)
    {
      new_value |= H3800_CF_RESET;
    }
    if (value & SA1110_EIO_OPT_PWR_ON)
    {
      new_value |= H3800_OPT_PWR;
    }
    if (value & SA1110_EIO_OPT_ON)
    {
      new_value |= H3800_OPT;
    }
    PIOA_DATA = new_value;
  }
}

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
  // Force "alternate" use of GPIO pins used for LCD screen
  *SA11X0_GPIO_ALTERNATE_FUNCTION |= 0x000003FC;      // Bits 2..9
  *SA11X0_GPIO_PIN_DIRECTION |= 0x000003FC;           // Bits 2..9
  *SA11X0_GPIO_PIN_OUTPUT_CLEAR = 0x000003FC;         // Bits 2..9

  *SA1110_LCCR0 &= ~(1);

  if (get_model() == MODEL_H3600)
  {
    // Pins used for buttons, communications with Atmel
    //*SA11X0_GPIO_PIN_DIRECTION &= 0x03FE0C003;
    *SA11X0_GPIO_RISING_EDGE_DETECT |= 0x00000002;
    *SA11X0_GPIO_FALLING_EDGE_DETECT |= 0x00000002;

    // Setup communication with Atmel micro-controller
    *SA1110_GPCLK_CONTROL_0 = SA1110_GPCLK_SUS_UART;
    *SA11X0_PPC_PIN_ASSIGNMENT &= ~SA11X0_PPC_UART_PIN_REASSIGNED;
    atmel_init();
    ipaq_EGPIO(SA1110_EIO_LCD_3V3|SA1110_EIO_LCD_CTRL|SA1110_EIO_LCD_5V|SA1110_EIO_LCD_VDD,
               SA1110_EIO_LCD_3V3_ON|SA1110_EIO_LCD_CTRL_ON|SA1110_EIO_LCD_5V_ON|SA1110_EIO_LCD_VDD_ON);
  }
  else if(get_model() == MODEL_H3800)
  {
    PIOA_DIR |= (PIOA_PEN);
    PIOB_DIR = 0xFFF;
    PIOB_ALT = 0x003e;
    PIOB_EDGE &= ~0xFFF;

    PIOB_LEVEL = ~PIOB_DATA & 0xFFF;
    PIOA_IEN = 0;
    PIOB_IEN = 0;
    GLOBAL_IEN |= 1;
    PIOB_IEN = 0xFFF | PIOB_ADC;


    PIOA_EDGE |= PIOA_PEN;
    PIOA_LEVEL &= ~PIOA_PEN;
    PIOA_IEN &= ~PIOA_PEN;
    PIOA_ISTAT |= PIOA_PEN;
    PIOA_IEN |= PIOA_PEN;

    cyg_uint16 v = PIOA_DATA & ~(PIOA_X0 | PIOA_Y0 | PIOA_Y1 | PIOA_X1);
    PIOA_DATA = v | (PIOA_X0 | PIOA_Y1 | PIOA_X1);  // Pen mode

    A2_CLOCK_EN |= (A2_CLK_ADC | A2_CLK_EX1);
  }
  lcd_brightness(0);

  // Configure the jacket detection pin
  *SA11X0_GPIO_PIN_DIRECTION      &= ~JACKET_DETECT; // input
  *SA11X0_GPIO_ALTERNATE_FUNCTION &= ~JACKET_DETECT; // normal gpio
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void ipaq_program_new_stack(void *func)
{
    register CYG_ADDRESS stack_ptr asm("sp");
    register CYG_ADDRESS old_stack asm("r4");
    register code_fun *new_func asm("r0");
    old_stack = stack_ptr;
    stack_ptr = CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - 0x100000;
    new_func = (code_fun*)func;
    new_func();
    HAL_ICACHE_INVALIDATE_ALL();
    stack_ptr = old_stack;
    return;
}

//
// Support for platform specific I/O channels
//

void
plf_if_init(void) 
{
#ifdef CYGSEM_IPAQ_LCD_COMM
    // Initialize screen
    lcd_init(16);
    // Initialize I/O channel
    lcd_comm_init();
#endif
}

int jacket_present(void) // return non-zero if jacket is present
{
  return JACKET_DETECT & *SA11X0_GPIO_PIN_LEVEL ? 0 : 1;
}

// This works because my devices have different cpu id's.
//
model_t get_model(void)
{
  unsigned int cpuid;
  __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r" (cpuid));

  if (cpuid == 0x6901b119)
  {
    return MODEL_H3800;
  }
  else if (cpuid == 0x6901b118)
  {
    return MODEL_H3600;
  }
  return MODEL_UNKNOWN;
}
