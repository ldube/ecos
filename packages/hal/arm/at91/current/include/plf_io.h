#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov, gthomas
// Date:        2001-07-12
// Purpose:     AT91/EB40 platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// USART

#define AT91_USART0 0xFFFD0000
#define AT91_USART1 0xFFFCC000

#define AT91_US_CR  0x00  // Control register
#define AT91_US_CR_RxRESET (1<<2)
#define AT91_US_CR_TxRESET (1<<3)
#define AT91_US_CR_RxENAB  (1<<4)
#define AT91_US_CR_RxDISAB (1<<5)
#define AT91_US_CR_TxENAB  (1<<6)
#define AT91_US_CR_TxDISAB (1<<7)
#define AT91_US_CR_RSTATUS (1<<8)
#define AT91_US_MR  0x04  // Mode register
#define AT91_US_MR_CLOCK   4
#define AT91_US_MR_CLOCK_MCK  (0<<AT91_US_MR_CLOCK)
#define AT91_US_MR_CLOCK_MCK8 (1<<AT91_US_MR_CLOCK)
#define AT91_US_MR_CLOCK_SCK  (2<<AT91_US_MR_CLOCK)
#define AT91_US_MR_LENGTH  6
#define AT91_US_MR_LENGTH_5   (0<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_6   (1<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_7   (2<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_8   (3<<AT91_US_MR_LENGTH)
#define AT91_US_MR_SYNC    8
#define AT91_US_MR_SYNC_ASYNC (0<<AT91_US_MR_SYNC)
#define AT91_US_MR_SYNC_SYNC  (1<<AT91_US_MR_SYNC)
#define AT91_US_MR_PARITY  9
#define AT91_US_MR_PARITY_EVEN  (0<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_ODD   (1<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_SPACE (2<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_MARK  (3<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_NONE  (4<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_MULTI (6<<AT91_US_MR_PARITY)
#define AT91_US_MR_STOP   12
#define AT91_US_MR_STOP_1       (0<<AT91_US_MR_STOP)
#define AT91_US_MR_STOP_1_5     (1<<AT91_US_MR_STOP)
#define AT91_US_MR_STOP_2       (2<<AT91_US_MR_STOP)
#define AT91_US_MR_MODE   14
#define AT91_US_MR_MODE_NORMAL  (0<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_ECHO    (1<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_LOCAL   (2<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_REMOTE  (3<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE9  17
#define AT91_US_MR_CLKO   18
#define AT91_US_IER 0x08  // Interrupt enable register
#define AT91_US_IER_RxRDY   (1<<0)  // Receive data ready
#define AT91_US_IER_TxRDY   (1<<1)  // Transmitter ready
#define AT91_US_IER_RxBRK   (1<<2)  // Break received
#define AT91_US_IER_ENDRX   (1<<3)  // Rx end
#define AT91_US_IER_ENDTX   (1<<4)  // Tx end
#define AT91_US_IER_OVRE    (1<<5)  // Rx overflow
#define AT91_US_IER_FRAME   (1<<6)  // Rx framing error
#define AT91_US_IER_PARITY  (1<<7)  // Rx parity
#define AT91_US_IER_TIMEOUT (1<<8)  // Rx timeout
#define AT91_US_IER_TxEMPTY (1<<9)  // Tx empty
#define AT91_US_IDR 0x0C  // Interrupt disable register
#define AT91_US_IMR 0x10  // Interrupt mask register
#define AT91_US_CSR 0x14  // Channel status register
#define AT91_US_CSR_RxRDY 0x01 // Receive data ready
#define AT91_US_CSR_TxRDY 0x02 // Transmit ready
#define AT91_US_RHR 0x18  // Receive holding register
#define AT91_US_THR 0x1C  // Transmit holding register
#define AT91_US_BRG 0x20  // Baud rate generator
#define AT91_US_RTO 0x24  // Receive time out
#define AT91_US_TTG 0x28  // Transmit timer guard

#define AT91_US_BAUD(baud) (CYGNUM_HAL_ARM_AT91_CLOCK_SPEED/(16*(baud)))

// PIO

#define AT91_PIO      0xFFFF0000

#define AT91_PIO_PER  0x00  // PIO enable
#define AT91_PIO_PDR  0x04  // PIO disable
#define AT91_PIO_PSR  0x08  // PIO status
#define AT91_PIO_OER  0x10  // Output enable
#define AT91_PIO_SODR 0x30  // Set out bits
#define AT91_PIO_CODR 0x34  // Clear out bits

// Advanced Interrupt Controller (AIC)

#define AT91_AIC      0xFFFFF000

#define AT91_AIC_SMR0   ((0*4)+0x000)
#define AT91_AIC_SMR1   ((1*4)+0x000)
#define AT91_AIC_SMR2   ((2*4)+0x000)
#define AT91_AIC_SMR3   ((3*4)+0x000)
#define AT91_AIC_SMR4   ((4*4)+0x000)
#define AT91_AIC_SMR5   ((5*4)+0x000)
#define AT91_AIC_SMR6   ((6*4)+0x000)
#define AT91_AIC_SMR7   ((7*4)+0x000)
#define AT91_AIC_SMR8   ((8*4)+0x000)
#define AT91_AIC_SMR9   ((9*4)+0x000)
#define AT91_AIC_SMR10  ((10*4)+0x000)
#define AT91_AIC_SMR11  ((11*4)+0x000)
#define AT91_AIC_SMR12  ((12*4)+0x000)
#define AT91_AIC_SMR13  ((13*4)+0x000)
#define AT91_AIC_SMR14  ((14*4)+0x000)
#define AT91_AIC_SMR15  ((15*4)+0x000)
#define AT91_AIC_SMR16  ((16*4)+0x000)
#define AT91_AIC_SMR17  ((17*4)+0x000)
#define AT91_AIC_SMR18  ((18*4)+0x000)
#define AT91_AIC_SMR19  ((19*4)+0x000)
#define AT91_AIC_SMR20  ((20*4)+0x000)
#define AT91_AIC_SMR21  ((21*4)+0x000)
#define AT91_AIC_SMR22  ((22*4)+0x000)
#define AT91_AIC_SMR23  ((23*4)+0x000)
#define AT91_AIC_SMR24  ((24*4)+0x000)
#define AT91_AIC_SMR25  ((25*4)+0x000)
#define AT91_AIC_SMR26  ((26*4)+0x000)
#define AT91_AIC_SMR27  ((27*4)+0x000)
#define AT91_AIC_SMR28  ((28*4)+0x000)
#define AT91_AIC_SMR29  ((29*4)+0x000)
#define AT91_AIC_SMR30  ((30*4)+0x000)
#define AT91_AIC_SMR31  ((31*4)+0x000)
#define AT91_AIC_SMR_LEVEL_LOW  (0<<5)
#define AT91_AIC_SMR_LEVEL_HI   (2<<5)
#define AT91_AIC_SMR_EDGE_NEG   (1<<5)
#define AT91_AIC_SMR_EDGE_POS   (3<<5)
#define AT91_AIC_SMR_PRIORITY   0x07
#define AT91_AIC_SVR0   ((0*4)+0x080)
#define AT91_AIC_SVR1   ((1*4)+0x080)
#define AT91_AIC_SVR2   ((2*4)+0x080)
#define AT91_AIC_SVR3   ((3*4)+0x080)
#define AT91_AIC_SVR4   ((4*4)+0x080)
#define AT91_AIC_SVR5   ((5*4)+0x080)
#define AT91_AIC_SVR6   ((6*4)+0x080)
#define AT91_AIC_SVR7   ((7*4)+0x080)
#define AT91_AIC_SVR8   ((8*4)+0x080)
#define AT91_AIC_SVR9   ((9*4)+0x080)
#define AT91_AIC_SVR10  ((10*4)+0x080)
#define AT91_AIC_SVR11  ((11*4)+0x080)
#define AT91_AIC_SVR12  ((12*4)+0x080)
#define AT91_AIC_SVR13  ((13*4)+0x080)
#define AT91_AIC_SVR14  ((14*4)+0x080)
#define AT91_AIC_SVR15  ((15*4)+0x080)
#define AT91_AIC_SVR16  ((16*4)+0x080)
#define AT91_AIC_SVR17  ((17*4)+0x080)
#define AT91_AIC_SVR18  ((18*4)+0x080)
#define AT91_AIC_SVR19  ((19*4)+0x080)
#define AT91_AIC_SVR20  ((20*4)+0x080)
#define AT91_AIC_SVR21  ((21*4)+0x080)
#define AT91_AIC_SVR22  ((22*4)+0x080)
#define AT91_AIC_SVR23  ((23*4)+0x080)
#define AT91_AIC_SVR24  ((24*4)+0x080)
#define AT91_AIC_SVR25  ((25*4)+0x080)
#define AT91_AIC_SVR26  ((26*4)+0x080)
#define AT91_AIC_SVR27  ((27*4)+0x080)
#define AT91_AIC_SVR28  ((28*4)+0x080)
#define AT91_AIC_SVR29  ((29*4)+0x080)
#define AT91_AIC_SVR30  ((30*4)+0x080)
#define AT91_AIC_SVR31  ((31*4)+0x080)
#define AT91_AIC_IVR    0x100
#define AT91_AIC_FVR    0x104
#define AT91_AIC_ISR    0x108
#define AT91_AIC_IPR    0x10C
#define AT91_AIC_IMR    0x110
#define AT91_AIC_CISR   0x114
#define AT91_AIC_IECR   0x120
#define AT91_AIC_IDCR   0x124
#define AT91_AIC_ICCR   0x128
#define AT91_AIC_ISCR   0x12C
#define AT91_AIC_EOI    0x130
#define AT91_AIC_SVR    0x134

// Timer / counter

#define AT91_TC         0xFFFE0000
#define AT91_TC_TC0     0x00
#define AT91_TC_CCR     0x00
#define AT91_TC_CCR_CLKEN  0x01
#define AT91_TC_CCR_CLKDIS 0x02
#define AT91_TC_CCR_TRIG   0x04
#define AT91_TC_CMR     0x04
// Capture mode definitions
#define AT91_TC_CMR_CLKS   0
#define AT91_TC_CMR_CLKS_MCK2      (0<<0)
#define AT91_TC_CMR_CLKS_MCK8      (1<<0)
#define AT91_TC_CMR_CLKS_MCK32     (2<<0)
#define AT91_TC_CMR_CLKS_MCK128    (3<<0)
#define AT91_TC_CMR_CLKS_MCK1024   (4<<0)
#define AT91_TC_CMR_CLKS_XC0       (5<<0)
#define AT91_TC_CMR_CLKS_XC1       (6<<0)
#define AT91_TC_CMR_CLKS_XC2       (7<<0)
#define AT91_TC_CMR_CLKI           (1<<3)
#define AT91_TC_CMR_BURST_NONE     (0<<4)
#define AT91_TC_CMR_BURST_XC0      (1<<4)
#define AT91_TC_CMR_BURST_XC1      (2<<4)
#define AT91_TC_CMR_BURST_XC2      (3<<4)
#define AT91_TC_CMR_LDBSTOP        (1<<6)
#define AT91_TC_CMR_LDBDIS         (1<<7)
#define AT91_TC_CMR_TRIG_NONE      (0<<8)
#define AT91_TC_CMR_TRIG_NEG       (1<<8)
#define AT91_TC_CMR_TRIG_POS       (2<<8)
#define AT91_TC_CMR_TRIG_BOTH      (3<<8)
#define AT91_TC_CMR_EXT_TRIG_TIOB  (0<<10)
#define AT91_TC_CMR_EXT_TRIG_TIOA  (1<<10)
#define AT91_TC_CMR_CPCTRG         (1<<14)
#define AT91_TC_CMR_LDRA_NONE      (0<<16)
#define AT91_TC_CMR_LDRA_TIOA_NEG  (1<<16)
#define AT91_TC_CMR_LDRA_TIOA_POS  (2<<16)
#define AT91_TC_CMR_LDRA_TIOA_BOTH (3<<16)
#define AT91_TC_CMR_LDRB_NONE      (0<<16)
#define AT91_TC_CMR_LDRB_TIOA_NEG  (1<<16)
#define AT91_TC_CMR_LDRB_TIOA_POS  (2<<16)
#define AT91_TC_CMR_LDRB_TIOA_BOTH (3<<16)
// Waveform mode definitions [missing]
#define AT91_TC_CV      0x10
#define AT91_TC_RA      0x14
#define AT91_TC_RB      0x18
#define AT91_TC_RC      0x1C
#define AT91_TC_SR      0x20
#define AT91_TC_SR_COVF    (1<<0)  // Counter overrun
#define AT91_TC_SR_LOVR    (1<<1)  // Load overrun
#define AT91_TC_SR_CPA     (1<<2)  // RA compare
#define AT91_TC_SR_CPB     (1<<3)  // RB compare
#define AT91_TC_SR_CPC     (1<<4)  // RC compare
#define AT91_TC_SR_LDRA    (1<<5)  // Load A status
#define AT91_TC_SR_LDRB    (1<<6)  // Load B status
#define AT91_TC_SR_EXT     (1<<7)  // External trigger
#define AT91_TC_SR_CLKSTA  (1<<16) // Clock enable/disable status
#define AT91_TC_SR_MTIOA   (1<<17) // TIOA mirror
#define AT91_TC_SR_MTIOB   (1<<18) // TIOB mirror
#define AT91_TC_IER     0x24
#define AT91_TC_IER_COVF   (1<<0)  // Counter overrun
#define AT91_TC_IER_LOVR   (1<<1)  // Load overrun
#define AT91_TC_IER_CPA    (1<<2)  // RA compare
#define AT91_TC_IER_CPB    (1<<3)  // RB compare
#define AT91_TC_IER_CPC    (1<<4)  // RC compare
#define AT91_TC_IER_LDRA   (1<<5)  // Load A status
#define AT91_TC_IER_LDRB   (1<<6)  // Load B status
#define AT91_TC_IER_EXT    (1<<7)  // External trigger
#define AT91_TC_IDR     0x28
#define AT91_TC_IMR     0x2C
#define AT91_TC_TC1     0x40
#define AT91_TC_TC2     0x80
#define AT91_TC_BCR     0xC0
#define AT91_TC_BCR_SYNC   0x01
#define AT91_TC_BMR     0xC4

// External Bus Interface

#define AT91_EBI        0xFFE00000
#define AT91_EBI_RCR    0x20       // Reset control

// Power Saveings control

#define AT91_PS         0xFFFF4000
#define AT91_PS_CR        0x000    // Control
#define AT91_PS_PCER      0x004    // Peripheral clock enable
#define AT91_PS_PCDR      0x004    // Peripheral clock disable
#define AT91_PS_PCSR      0x004    // Peripheral clock status

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
