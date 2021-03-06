//==========================================================================
//
//      quicc_eth.h
//
//      PowerPC QUICC (MPC8xx) ethernet 
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
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PowerPC QUICC (MPC8xx) Ethernet

#include <cyg/hal/quicc/ppc8xx.h>                  // QUICC structure definitions

struct quicc_eth_info {
    volatile struct ethernet_pram  *pram;            // Parameter RAM pointer
    volatile struct scc_regs       *ctl;             // SCC control registers
    volatile struct cp_bufdesc     *txbd, *rxbd;     // Next Tx,Rx descriptor to use
    struct cp_bufdesc              *tbase, *rbase;   // First Tx,Rx descriptor
    struct cp_bufdesc              *tnext, *rnext;   // Next descriptor to check for interrupt
    int                             txsize, rxsize;  // Length of individual buffers
    unsigned long                   txkey[CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM];
};

// SCC registers - ethernet mode

// General SCC mode register
#define QUICC_SCC_GSMH_IRP          0x00040000  // Infared polarity
#define QUICC_SCC_GSMH_GDE          0x00010000  // Glitch detect enable
#define QUICC_SCC_GSMH_TCRC         0x00008000  // Transparent CRC
#define QUICC_SCC_GSMH_REVD         0x00004000  // Reverse data (transparent)
#define QUICC_SCC_GSMH_TRX          0x00002000  // Transparent Rx
#define QUICC_SCC_GSMH_TTX          0x00001000  // Transparent Tx

#define QUICC_SCC_GSML_TCI          0x10000000  // Transmit clock invert
#define QUICC_SCC_GSML_TPL          0x00E00000  // Tx preamble bits
#define QUICC_SCC_GSML_TPL_8        0x00200000  //    8 bits
#define QUICC_SCC_GSML_TPL_16       0x00400000  //   16 bits
#define QUICC_SCC_GSML_TPL_32       0x00600000  //   32 bits
#define QUICC_SCC_GSML_TPL_48       0x00800000  //   48 bits (used for ethernet)
#define QUICC_SCC_GSML_TPL_64       0x00A00000  //   64 bits
#define QUICC_SCC_GSML_TPL_128      0x00C00000  //  128 bits
#define QUICC_SCC_GSML_TPP          0x00180000  // Tx preamble pattern
#define QUICC_SCC_GSML_TPP_00       0x00000000  //   all zeroes
#define QUICC_SCC_GSML_TPP_01       0x00080000  //   10 repeats (ethernet)
#define QUICC_SCC_GSML_TPP_10       0x00100000  //   01 repeats
#define QUICC_SCC_GSML_TPP_11       0x00180000  //   all ones (localtalk)
#define QUICC_SCC_GSML_ENR          0x00000020  // Enable receiver
#define QUICC_SCC_GSML_ENT          0x00000010  // Enable transmitter
#define QUICC_SCC_GSML_MODE         0x0000000F  // Operating mode
#define QUICC_SCC_GSML_MODE_HDLC    0x00000000
#define QUICC_SCC_GSML_MODE_ATALK   0x00000002
#define QUICC_SCC_GSML_MODE_ENET    0x0000000C

// Function code
#define QUICC_SCC_FCR_BE            0x0010  // Big Endian operation

// Event register
#define QUICC_SCCE_GRC              0x0080  // Gracefull stop complete
#define QUICC_SCCE_TXE              0x0010  // Transmit error
#define QUICC_SCCE_RXF              0x0008  // Received full frame
#define QUICC_SCCE_BSY              0x0004  // No free receive buffers
#define QUICC_SCCE_TX               0x0002  // Buffer transmit complete
#define QUICC_SCCE_RX               0x0001  // Buffer received
#define QUICC_SCCE_INTS (QUICC_SCCE_TXE | QUICC_SCCE_RXF | QUICC_SCCE_TX)

// Protocol specific mode register
#define QUICC_PMSR_HEARTBEAT        0x8000  // Enable heartbeat
#define QUICC_PMSR_FORCE_COLLISION  0x4000  // Force a collision
#define QUICC_PMSR_RCV_SHORT_FRAMES 0x2000  // Accept short frames
#define QUICC_PMSR_INDIV_ADDR_MODE  0x1000  // Check individual address (hash)
#define QUICC_PMSR_ENET_CRC         0x0800  // Enable ethernet CRC mode
#define QUICC_PMSR_PROMISCUOUS      0x0200  // Enable promiscuous mode
#define QUICC_PMSR_BROADCAST        0x0100  // Accept broadcast packets
#define QUICC_PMSR_SPECIAL_BACKOFF  0x0080  // Enable special backoff timer
#define QUICC_PMSR_LOOPBACK         0x0040  // Enable loopback mode
#define QUICC_PMSR_SAMPLE_INPUTS    0x0020  // Discretely look at input pins
#define QUICC_PMSR_LATE_COLLISION   0x0010  // Enable late collision window
#define QUICC_PMSR_SEARCH_AFTER_22  0x000A  // Start frame search after 22 bits
#define QUICC_PMSR_FULL_DUPLEX      0x0001  // Full duplex mode

// Receive buffer status
#define QUICC_BD_RX_LAST            0x0800  // Last buffer in chain
#define QUICC_BD_RX_FIRST           0x0400  // First buffer in chain
#define QUICC_BD_RX_MISS            0x0100  // Missed data
#define QUICC_BD_RX_LG              0x0020  // Rx frame too long
#define QUICC_BD_RX_NO              0x0010  // Rx frame not properly aligned
#define QUICC_BD_RX_SH              0x0008  // Rx frame too short
#define QUICC_BD_RX_CR              0x0004  // Bad CRC
#define QUICC_BD_RX_OV              0x0002  // Rx overrun
#define QUICC_BD_RX_CL              0x0001  // Collision during frame  

// Transmit buffer status
#define QUICC_BD_TX_PAD             0x4000  // Pad short packets
#define QUICC_BD_TX_LAST            0x0800  // Last buffer in chain
#define QUICC_BD_TX_TC              0x0400  // Transmit CRC after buffer
#define QUICC_BD_TX_DEF             0x0200  // Transmission was deferred
#define QUICC_BD_TX_HB              0x0100  // Heartbeat detected
#define QUICC_BD_TX_LC              0x0080  // Late collision
#define QUICC_BD_TX_RL              0x0040  // Retransmit limit exceeded
#define QUICC_BD_TX_RC              0x003C  // Retry count
#define QUICC_BD_TX_UN              0x0002  // Tx underrun
#define QUICC_BD_TX_CSL             0x0001  // Carrier lost

// MBX specific "wiring" - aux connections to MX68160 Ethernet support chip
#define QUICC_MBX_PA_RXD            0x0001  // Rx Data on Port A
#define QUICC_MBX_PA_TXD            0x0002  // Tx Data on Port A
#define QUICC_MBX_PA_Tx_CLOCK       0x0200  // Tx Clock = CLK2
#define QUICC_MBX_PA_Rx_CLOCK       0x0800  // Rx Clock = CLK4
#define QUICC_MBX_PC_Tx_ENABLE      0x0001  // Tx Enable (TENA)
#define QUICC_MBX_PC_COLLISION      0x0010  // Collision detect
#define QUICC_MBX_PC_Rx_ENABLE      0x0020  // Rx Enable (RENA)
#define QUICC_MBX_SICR_MASK         0x00FF  // SI Clock Route - important bits
#define QUICC_MBX_SICR_ENET  (7<<3)|(5<<0)  //   Rx=CLK4, Tx=CLK2
#define QUICC_MBX_SICR_SCC1_ENABLE  0x0040  // Enable SCC1 to use NMSI

#define MBX_CTL1   (cyg_uint8 *)0xFA100000  // System control register
#define MBX_CTL1_ETEN                 0x80  // 1 = Enable ethernet tranceiver
#define MBX_CTL1_ELEN                 0x40  // 1 = Enable ethernet loopback
#define MBX_CTL1_EAEN                 0x20  // 1 = Auto select ethernet interface
#define MBX_CTL1_TPEN                 0x10  // 0 = AUI, 1 = TPI
#define MBX_CTL1_FDDIS                0x08  // 1 = Disable full duplex (if TP mode)

#define IEEE_8023_MAX_FRAME         1518    // Largest possible ethernet frame
#define IEEE_8023_MIN_FRAME           64    // Smallest possible ethernet frame

