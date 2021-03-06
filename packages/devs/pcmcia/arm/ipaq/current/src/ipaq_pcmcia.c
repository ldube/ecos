//==========================================================================
//
//      devs/pcmcia/ipaq/ipaq_pcmcia.c
//
//      PCMCIA support (Card Services)
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
// Contributors: gthomas
//               richard.panton@3glab.com
// Date:         2001-02-24
// Purpose:      PCMCIA support
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io_pcmcia.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_intr.h>           // HAL interrupt macros
#include <cyg/hal/drv_api.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>   // Configuration header
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/hal/hal_if.h>

#include <cyg/io/pcmcia.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_sa11x0.h>  // Board definitions
#include <cyg/hal/ipaq.h>

#ifdef CYGPKG_KERNEL
static cyg_interrupt cf_detect_interrupt;
static cyg_handle_t  cf_detect_interrupt_handle;
static cyg_interrupt cf_irq_interrupt;
static cyg_handle_t  cf_irq_interrupt_handle;

// This ISR is called when a CompactFlash board is inserted
static int
cf_detect_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(SA1110_CF_DETECT);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR handles the board insertion
static void
cf_detect_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;
    struct cf_slot *slot = (struct cf_slot *)data;
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        slot->state = CF_SLOT_STATE_Inserted;
    } else {
        slot->state = CF_SLOT_STATE_Removed;  // Implies powered up, etc.
    }
    cyg_drv_interrupt_acknowledge(SA1110_CF_DETECT);
    cyg_drv_interrupt_unmask(SA1110_CF_DETECT);
}

// This ISR is called when the card interrupt occurs
static int
cf_irq_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(SA1110_CF_IRQ);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR handles the card interrupt
static void
cf_irq_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct cf_slot *slot = (struct cf_slot *)data;
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    cyg_bool was_ctrlc_int;
#endif

    // Clear interrupt [edge indication]
    cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    was_ctrlc_int = HAL_CTRLC_CHECK(vector, data);
    if (!was_ctrlc_int) // Fall through and run normal code
#endif
    // Process interrupt
    (slot->irq_handler.handler)(vector, count, slot->irq_handler.param);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(SA1110_CF_IRQ);
}
#endif

static void
do_delay(int ticks)
{
#ifdef CYGPKG_KERNEL
    cyg_thread_delay(ticks);
#else
    CYGACC_CALL_IF_DELAY_US(10000*ticks);
#endif
}

//
// Fill in the details of a PCMCIA slot and initialize the device
//
bool
cf_hwr_init(struct cf_slot *slot)
{
    static int int_init = 0;
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;

    if (!jacket_present()) {
      return false;
    }

    if (!int_init) {
        int_init = 1;
#ifdef CYGPKG_KERNEL
        // Set up interrupts
        cyg_drv_interrupt_create(SA1110_CF_DETECT,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t) slot,  //  Data item passed to interrupt handler
                                 (cyg_ISR_t *)cf_detect_isr,
                                 (cyg_DSR_t *)cf_detect_dsr,
                                 &cf_detect_interrupt_handle,
                                 &cf_detect_interrupt);
        cyg_drv_interrupt_attach(cf_detect_interrupt_handle);
        cyg_drv_interrupt_configure(SA1110_CF_DETECT, true, true);  // Detect either edge
        cyg_drv_interrupt_acknowledge(SA1110_CF_DETECT);
        cyg_drv_interrupt_unmask(SA1110_CF_DETECT);

        cyg_drv_interrupt_create(SA1110_CF_IRQ,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t) slot,  //  Data item passed to interrupt handler
                                 (cyg_ISR_t *)cf_irq_isr,
                                 (cyg_DSR_t *)cf_irq_dsr,
                                 &cf_irq_interrupt_handle,
                                 &cf_irq_interrupt);
        cyg_drv_interrupt_attach(cf_irq_interrupt_handle);
        cyg_drv_interrupt_unmask(SA1110_CF_IRQ);
        cyg_drv_interrupt_configure(SA1110_CF_IRQ, false, false);  // Falling edge
        cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
#endif
    }
    slot->attr = (unsigned char *)0x28000000;
    slot->attr_length = 0x200;
    slot->io = (unsigned char *)0x20000000;
    slot->io_length = 0x04000000;
    slot->mem = (unsigned char *)0x2C000000;
    slot->mem_length = 0x04000000;
    slot->int_num = SA1110_CF_IRQ;
#ifdef CYG_HAL_STARTUP_ROM
    // Disable CF bus & power (idle/off)
    ipaq_EGPIO( SA1110_EIO_OPT_PWR | SA1110_EIO_OPT | SA1110_EIO_CF_RESET,
	        SA1110_EIO_OPT_PWR_ON | SA1110_EIO_OPT_ON | SA1110_EIO_CF_RESET_ENABLE );
#endif
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        if ((_ipaq_EGPIO & SA1110_EIO_OPT_PWR) == SA1110_EIO_OPT_PWR_ON) {
            // Assume that the ROM environment has turned the bus on
            slot->state = CF_SLOT_STATE_Ready;
        } else {
            slot->state = CF_SLOT_STATE_Inserted;
        }
    } else {
        slot->state = CF_SLOT_STATE_Empty;
    }

    return true;
}

void
cf_hwr_poll(struct cf_slot *slot)
{
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        slot->state = CF_SLOT_STATE_Inserted;
    } else {
        slot->state = CF_SLOT_STATE_Empty;
    }
}

//
// Transition the card/slot to a new state
// note: currently only supports transitions to Ready, Empty
//
void
cf_hwr_change_state(struct cf_slot *slot, int new_state)
{    
    int i, ptr, len;
    unsigned char buf[256];
    unsigned short voltset = 0;
    bool linkup = (LINKUP_SLOT0 & 0x1800) == 0x1800 ? true : false;

    if (new_state == CF_SLOT_STATE_Ready) {
        if (slot->state == CF_SLOT_STATE_Inserted) {
            if (linkup) {
              if ((LINKUP_SLOT0 & (STATUS_VS1 | STATUS_VS2)) == (STATUS_VS1 | STATUS_VS2)) {
                voltset = CMD_S2 | CMD_S3; // enable 5v and disable 3v
              } else { // VS1=0 and VS2=1 => 3.3V
                voltset = CMD_S1 | CMD_S3; // enable 3v and disable 5v
              }
            }

            ipaq_EGPIO( SA1110_EIO_OPT_PWR | SA1110_EIO_OPT | SA1110_EIO_CF_RESET,
                        SA1110_EIO_OPT_PWR_ON | SA1110_EIO_OPT_ON | SA1110_EIO_CF_RESET_DISABLE);
            do_delay(30);  // At least 300 ms
            slot->state = CF_SLOT_STATE_Powered;
            ipaq_EGPIO( SA1110_EIO_CF_RESET, SA1110_EIO_CF_RESET_ENABLE);
            if (linkup) {
                LINKUP_SLOT0 = ( CMD_RESET | CMD_APOE | CMD_SOE | CMD_S1 | CMD_S2); //Reset
            }
            do_delay(1);  // At least 10 us
            slot->state = CF_SLOT_STATE_Reset;
            ipaq_EGPIO( SA1110_EIO_CF_RESET, SA1110_EIO_CF_RESET_DISABLE);
            do_delay(5); // At least 20 ms
            if (linkup) {
                LINKUP_SLOT0 = ( CMD_APOE | CMD_SOE | voltset);
            }
            do_delay(5); // At least 20 ms
            // Wait until the card is ready to talk
            for (i = 0;  i < 10;  i++) {
                ptr = 0;
                if (cf_get_CIS(slot, CF_CISTPL_VERS_1, buf, &len, &ptr)) {
                    slot->state = CF_SLOT_STATE_Ready;
                    break;
                }
                do_delay(10); 
            }
        }
    }
}

//
// Acknowledge interrupt (used in non-kernel environments)
//
void
cf_hwr_clear_interrupt(struct cf_slot *slot)
{
    // Clear interrupt [edge indication]
    cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
}
