//==========================================================================
//
//      dev/if_sc_lpe.c
//
//      Ethernet device driver for Socket Communications Compact Flash card
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2000-07-07
// Purpose:      
// Description:  hardware driver for LPCF+ ethernet
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/pcmcia.h>
#include <eth_drv.h>
#include <netdev.h>
#include <string.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#else
#include <cyg/hal/hal_if.h>
#endif

#include <cyg/io/dp83902a.h>

#define DP_DATA         0x10
#define DP_CARD_RESET   0x1f

#define SC_LPE_MANUF 0x0104
#define FA411_MANUF  0x0149



#ifdef CYGPKG_NET
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
static char sc_lpe_card_handler_stack[STACK_SIZE];
static cyg_thread sc_lpe_card_handler_thread_data;
static cyg_handle_t sc_lpe_card_handler_thread_handle;
#endif  // CYGPKG_NET

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
// This runs as a separate thread to handle the card.  In particular, insertions
// and deletions need to be handled and they take time/coordination, thus the
// separate thread.
//
#ifdef CYGPKG_NET
static void
#else
static int
#endif
sc_lpe_card_handler(cyg_addrword_t param)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)param;
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t*)sc->driver_private;
    struct cf_slot *slot;
    struct cf_cftable cftable;
    struct cf_config config;
    int i, len, ptr, cor = 0;
    unsigned char buf[256], *cp;
    cyg_uint8* base;
    unsigned char *vers_product, *vers_manuf, *vers_revision, *vers_date;
    cyg_uint16 manuf_id = 0;
#ifndef CYGPKG_NET
    int tries = 0;
#endif
    bool first = true;

    memset(&config, 0, sizeof(config));
    slot = (struct cf_slot*)dp->plf_priv;
    cyg_drv_dsr_lock();
    while (true) {
        cyg_drv_dsr_unlock();   // Give DSRs a chance to run (card insertion)
        cyg_drv_dsr_lock();
        if ((slot->state == CF_SLOT_STATE_Inserted) ||
            ((slot->state == CF_SLOT_STATE_Ready) && first)) {
            first = false;
            if (slot->state != CF_SLOT_STATE_Ready) {
                cf_change_state(slot, CF_SLOT_STATE_Ready);
            }
            if (slot->state != CF_SLOT_STATE_Ready) {
                diag_printf("CF card won't go ready!\n");
#ifndef CYGPKG_NET
                return false;
#else
                continue;
#endif
            }
            len = sizeof(buf);
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_MANFID, buf, &len, &ptr)) {
                manuf_id = *(cyg_uint16 *)&buf[2];
                //diag_printf("MANFID = 0x%x\n", *(cyg_uint16 *)&buf[2]);
                if (manuf_id == 0xd601 || manuf_id == 0x274 || manuf_id == 0x45) {
                  diag_printf("Unsupported PC Card found.\n");
#ifndef CYGPKG_NET
                  return false;
#else
                  continue;
#endif
                }
            } 
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_VERS_1, buf, &len, &ptr)) {
                // Find individual strings
                cp = &buf[4];
                vers_product = cp;
                while (*cp++) ;  // Skip to nul
                vers_manuf = cp;
                while (*cp++) ;  // Skip to nul
                vers_revision = cp;
                while (*cp++) ;  // Skip to nul
                vers_date = cp;
#ifndef CYGPKG_NET
                if (tries != 0) diag_printf("\n");
                diag_printf("%s: %s %s %s\n", vers_manuf, vers_product, vers_revision, vers_date);
#endif
            }
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_CONFIG, buf, &len, &ptr)) {
                if (cf_parse_config(buf, len, &config)) {
                    cor = config.base;
                }
            }
            if (!cor) {
//                diag_printf("Couldn't find COR pointer!\n");
                continue;
            }

            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_CFTABLE_ENTRY, buf, &len, &ptr)) {
                if (cf_parse_cftable(buf, len, &cftable)) {
                    cyg_uint8 tmp;
                    // Initialize dp83902a IO details
                    if (cftable.io_space.num_addrs) {
                      base = (cyg_uint8*)&slot->io[cftable.io_space.base[0]];
                    } else if (manuf_id == FA411_MANUF && (config.mask[0] & 0x60) == 0x60) {
                      volatile unsigned char *cfg = slot->attr;
                      unsigned short io_offset = cfg[config.reg[6]];
                      io_offset = (io_offset << 8) | cfg[config.reg[5]];
                      base = (cyg_uint8*)&slot->io[io_offset];
                    } else {
                      base = (cyg_uint8*)&slot->io[0];
                    }
                    dp->base = base;
                    dp->data = base + DP_DATA;
                    dp->interrupt = slot->int_num;
                    cf_set_COR(slot, cor, cftable.cor);
                    // Reset card  (read issues RESET, write clears it)
                    HAL_READ_UINT8(base+DP_CARD_RESET, tmp);
                    HAL_WRITE_UINT8(base+DP_CARD_RESET, tmp);
                    // Wait for card
                    do {
                        DP_IN(base, DP_ISR, tmp);
                    } while (0 == (tmp & DP_ISR_RESET));

                    // Fetch hardware address from card - terrible, but not well defined
                    // Patterned after what Linux drivers do
                    if (!dp->hardwired_esa) {
                        static unsigned char sc_lpe_addr[] = { 0x00, 0xC0, 0x1B, 0x00, 0x99, 0x9E};
                        if ((slot->attr[0x1C0] == sc_lpe_addr[0]) &&
                            (slot->attr[0x1C2] == sc_lpe_addr[1]) &&
                            (slot->attr[0x1C4] == sc_lpe_addr[2])) {
                            sc_lpe_addr[3] = slot->attr[0x1C6];
                            sc_lpe_addr[4] = slot->attr[0x1C8];
                            sc_lpe_addr[5] = slot->attr[0x1CA];
                        } else {
                            // Coudn't find it in the CIS (attribute) data
                            unsigned char prom[32];

                            // Tell device to give up ESA
                            DP_OUT(base, DP_DCR, DP_DCR_LS | DP_DCR_FIFO_4); // Bytewide access
                            DP_OUT(base, DP_RBCH, 0);    // Remote byte count
                            DP_OUT(base, DP_RBCL, 0);
                            DP_OUT(base, DP_ISR, 0xFF);  // Clear any pending interrupts
                            DP_OUT(base, DP_IMR, 0x00);  // Mask all interrupts 
                            DP_OUT(base, DP_RCR, 0x20);  // Monitor
                            DP_OUT(base, DP_TCR, 0x02);  // loopback
                            DP_OUT(base, DP_RBCH, 32);   // Remote byte count
                            DP_OUT(base, DP_RBCL, 0);
                            DP_OUT(base, DP_RSAL, 0);    // Remote address
                            DP_OUT(base, DP_RSAH, 0);
                            DP_OUT(base, DP_CR, DP_CR_START|DP_CR_RDMA);  // Read data
                            for (i = 0;  i < 32;  i++) {
                                HAL_READ_UINT8(base+DP_DATAPORT, prom[i]);
                            }
                            if (manuf_id == FA411_MANUF && (config.mask[0] & 0x60) == 0x60) {
// Set to 1 to get the hardware mac address from card.
#if 0
                              for (i = 0;  i < 6;  i++) {
                                sc_lpe_addr[i] = prom[i*2];
                              }
#endif
                            } else if ((prom[0] == sc_lpe_addr[0]) &&
                                (prom[2] == sc_lpe_addr[1]) &&
                                (prom[4] == sc_lpe_addr[2])) {
                                diag_printf("Getting address from port\n");
                                sc_lpe_addr[3] = prom[6];
                                sc_lpe_addr[4] = prom[8];
                                sc_lpe_addr[5] = prom[10];
                            } else {
                                diag_printf("No valid ESA found in CIS! Hardwiring to 00:C0:1B:00:99:9E\n");
                            }
                        }
                        for (i = 0;  i < 6;  i++) {
                            dp->esa[i] = sc_lpe_addr[i];
                        }
                    }

                    // Initialize upper level driver
                    (sc->funs->eth_drv->init)(sc, dp->esa);
                    // Tell system card is ready to talk
                    dp->tab->status = CYG_NETDEVTAB_STATUS_AVAIL;
#ifndef CYGPKG_NET
                    cyg_drv_dsr_unlock();
                    return true;
#endif
                } else {
                    diag_printf("Can't parse CIS\n");
                    continue;
                }
            } else {
                diag_printf("Can't fetch config info\n");
                continue;
            }
        } else if (slot->state == CF_SLOT_STATE_Removed) {
            diag_printf("Compact Flash card removed!\n");
        } else {
            cyg_drv_dsr_unlock();
            do_delay(50);  // FIXME!
#ifndef CYGPKG_NET
            if (tries == 0) diag_printf("... Waiting for network card: ");
            diag_printf(".");
            if (++tries == 10) {
                // 5 seconds have elapsed - give up
                return false;
            }
            cf_hwr_poll(slot);  // Check to see if card has been inserted
#endif
            cyg_drv_dsr_lock();
        }
    }
}

bool 
cyg_sc_lpe_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *)sc->driver_private;
    struct cf_slot* slot;

    // Make sure Compact Flash subsystem is initialized
    if (!cf_init()) {
      return false;
    }
    slot = dp->plf_priv = (void*)cf_get_slot(0);
    dp->tab = tab;

#ifdef CYGPKG_NET
    // Create card handling [background] thread
    cyg_thread_create(CYGPKG_NET_THREAD_PRIORITY-1,          // Priority
                      sc_lpe_card_handler,                   // entry
                      (cyg_addrword_t)sc,                    // entry parameter
                      "SC LP-E card support",                // Name
                      &sc_lpe_card_handler_stack[0],         // Stack
                      STACK_SIZE,                            // Size
                      &sc_lpe_card_handler_thread_handle,    // Handle
                      &sc_lpe_card_handler_thread_data       // Thread data structure
            );
    cyg_thread_resume(sc_lpe_card_handler_thread_handle);    // Start it

    // Initialize environment, setup interrupt handler
    // eth_drv_dsr is used to tell the fast net thread to run the deliver funcion.
    cf_register_handler(slot, eth_drv_dsr, sc);

    return false;  // Device is not ready until inserted, powered up, etc.
#else
    // Initialize card
    return sc_lpe_card_handler(sc);
#endif
}

int
cyg_sc_lpe_int_vector(struct eth_drv_sc *sc)
{
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *)sc->driver_private;
    struct cf_slot* slot = (struct cf_slot*)dp->plf_priv;

    return slot->int_num;
}
