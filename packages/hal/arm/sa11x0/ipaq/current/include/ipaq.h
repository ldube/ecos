#ifndef CYGONCE_IPAQ_H
#define CYGONCE_IPAQ_H

//=============================================================================
//
//      ipaq.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    gthomas
// Contributors: gthomas, richard.panton@3glab.com
// Date:         2001-02-24
// Purpose:      Intel SA1110/iPAQ platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/ipaq.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================


#ifndef __ASSEMBLER__
//
// Extended GPIO
// Note: This register is write-only.  Thus a shadow copy is provided so that
// it may be safely updated/shared by multiple threads.
//
extern unsigned long _ipaq_EGPIO;  // Shadow copy

extern unsigned short _ipaq_LCD_params[(4*2)+1];  // Various LCD parameters

extern void ipaq_EGPIO(unsigned long mask, unsigned long value);

extern int jacket_present(void); // return non-zero if jacket is present


typedef enum {
  MODEL_H3600   = 0x36, // '6'
  MODEL_H3800   = 0x38, // '8'
  MODEL_UNKNOWN = 0x3F, // '?'
} model_t;

extern model_t get_model(void);

#endif

// 
// Signal assertion levels
//
#define SA1110_LOGIC_ONE(m)  (m & 0xFFFF)
#define SA1110_LOGIC_ZERO(m) (m & 0x0000)

//
// iPAQ Extended GPIO definitions
//
#define SA1110_EGPIO			REG16_PTR(0x49000000)
#define SA1110_EIO_MIN			0x0080

#define SA1110_EIO_VPP			0x0001
# define SA1110_EIO_VPP_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_VPP)
# define SA1110_EIO_VPP_ON			SA1110_LOGIC_ONE(SA1110_EIO_VPP)
#define SA1110_EIO_CF_RESET		0x0002
# define SA1110_EIO_CF_RESET_DISABLE		SA1110_LOGIC_ZERO(SA1110_EIO_CF_RESET)
# define SA1110_EIO_CF_RESET_ENABLE		SA1110_LOGIC_ONE(SA1110_EIO_CF_RESET)
#define SA1110_EIO_OPT_RESET		0x0004
# define SA1110_EIO_OPT_RESET_DISABLE		SA1110_LOGIC_ZERO(SA1110_EIO_OPT_RESET)
# define SA1110_EIO_OPT_RESET_ENABLE		SA1110_LOGIC_ONE(SA1110_EIO_OPT_RESET)
#define SA1110_EIO_CODEC_RESET		0x0008	// Active LOW
# define SA1110_EIO_CODEC_RESET_DISABLE		SA1110_LOGIC_ONE(SA1110_EIO_CODEC_RESET)
# define SA1110_EIO_CODEC_RESET_ENABLE		SA1110_LOGIC_ZERO(SA1110_EIO_CODEC_RESET)
#define SA1110_EIO_OPT_PWR		0x0010
# define SA1110_EIO_OPT_PWR_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_OPT_PWR)
# define SA1110_EIO_OPT_PWR_ON			SA1110_LOGIC_ONE(SA1110_EIO_OPT_PWR)
#define SA1110_EIO_OPT			0x0020
# define SA1110_EIO_OPT_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_OPT)
# define SA1110_EIO_OPT_ON			SA1110_LOGIC_ONE(SA1110_EIO_OPT)
#define SA1110_EIO_LCD_3V3		0x0040
# define SA1110_EIO_LCD_3V3_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_LCD_3V3)
# define SA1110_EIO_LCD_3V3_ON			SA1110_LOGIC_ONE(SA1110_EIO_LCD_3V3)
#define SA1110_EIO_RS232		0x0080
# define SA1110_EIO_RS232_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_RS232)
# define SA1110_EIO_RS232_ON			SA1110_LOGIC_ONE(SA1110_EIO_RS232)
#define SA1110_EIO_LCD_CTRL		0x0100
# define SA1110_EIO_LCD_CTRL_OFF		SA1110_LOGIC_ZERO(SA1110_EIO_LCD_CTRL)
# define SA1110_EIO_LCD_CTRL_ON			SA1110_LOGIC_ONE(SA1110_EIO_LCD_CTRL)
#define SA1110_EIO_IR			0x0200
# define SA1110_EIO_IR_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_IR)
# define SA1110_EIO_IR_ON			SA1110_LOGIC_ONE(SA1110_EIO_IR)
#define SA1110_EIO_AMP			0x0400
# define SA1110_EIO_AMP_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_AMP)
# define SA1110_EIO_AMP_ON			SA1110_LOGIC_ONE(SA1110_EIO_AMP)
#define SA1110_EIO_AUDIO		0x0800
# define SA1110_EIO_AUDIO_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_AUDIO)
# define SA1110_EIO_AUDIO_ON			SA1110_LOGIC_ONE(SA1110_EIO_AUDIO)
#define SA1110_EIO_MUTE			0x1000
# define SA1110_EIO_MUTE_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_MUTE)
# define SA1110_EIO_MUTE_ON			SA1110_LOGIC_ONE(SA1110_EIO_MUTE)
#define SA1110_EIO_IR_FSEL		0x2000
# define SA1110_EIO_SIR				SA1110_LOGIC_ZERO(SA1110_EIO_IR_FSEL)
# define SA1110_EIO_FIR				SA1110_LOGIC_ONE(SA1110_EIO_IR_FSEL)
#define SA1110_EIO_LCD_5V		0x4000
# define SA1110_EIO_LCD_5V_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_LCD_5V)
# define SA1110_EIO_LCD_5V_ON			SA1110_LOGIC_ONE(SA1110_EIO_LCD_5V)
#define SA1110_EIO_LCD_VDD		0x8000
# define SA1110_EIO_LCD_VDD_OFF			SA1110_LOGIC_ZERO(SA1110_EIO_LCD_VDD)
# define SA1110_EIO_LCD_VDD_ON			SA1110_LOGIC_ONE(SA1110_EIO_LCD_VDD)

//
// Special purpose GPIO interrupt mappings
//
#define SA1110_CF_IRQ               CYGNUM_HAL_INTERRUPT_GPIO21
#define SA1110_CF_DETECT            CYGNUM_HAL_INTERRUPT_GPIO17

//
// GPIO layout
//
#define SA1110_GPIO_CF_DETECT       0x00020000     // 0 = Compact Flash detect
#define SA1110_GPIO_CF_PRESENT      SA1110_LOGIC_ZERO(SA1110_GPIO_CF_DETECT)
#define SA1110_GPIO_CF_ABSENT       SA1110_LOGIC_ONE(SA1110_GPIO_CF_DETECT)

#define JACKET_DETECT               SA11X0_GPIO_PIN_27

//
// LCD Controller
//
#define SA1110_LCCR0                SA11X0_REGISTER(0x30100000)
#define SA1110_LCSR                 SA11X0_REGISTER(0x30100004)
#define SA1110_DBAR1                SA11X0_REGISTER(0x30100010)
#define SA1110_DCAR1                SA11X0_REGISTER(0x30100014)
#define SA1110_DBAR2                SA11X0_REGISTER(0x30100018)
#define SA1110_DCAR2                SA11X0_REGISTER(0x3010001C)
#define SA1110_LCCR1                SA11X0_REGISTER(0x30100020)
#define SA1110_LCCR2                SA11X0_REGISTER(0x30100024)
#define SA1110_LCCR3                SA11X0_REGISTER(0x30100028)

// PC Card/Compact Flash Interface for SA-1100 / SA-1110
#define LINKUP_SLOT0 *(volatile unsigned short*)(0x1a000000)
#define LINKUP_SLOT1 *(volatile unsigned short*)(0x19000000)

// STATUS REGISTER
// ===============
//3:0 S4:S1 These 4 bits reflect the current status of the Voltage Control Pins S4 down to S1.
#define STATUS_BVD1 (1 << 4)  // Battery Voltage Detect 1 status. This bit 
                              // reflects the status of P_BVD[1] pin.
#define STATUS_BVD2 (1 << 5)  // Battery Voltage Detect 2 status. This bit reflects
                              // the status of P_BVD[2] pin.
#define STATUS_VS1  (1 << 6)  // Voltage Sense 1 status. This bit reflects the status 
                              // of P_VS[1] pin.
#define STATUS_VS2  (1 << 7)  // Voltage Sense 2 status. This bit reflects the status 
                              // of P_VS[2] pin.
#define STATUS_RDY  (1 << 8)  // P_RDY status. This bit reflects the status of P_RDY pin.
#define STATUS_CD1  (1 << 9)  // Card Detect 1: P_nCD[1] status. This bit reflects 
                              // the status of P_nCD[1] pin.
#define STATUS_CD2  (1 << 10) // Card Detect 2: P_nCD[2] status. This bit reflects 
                              // the status of P_nCD[2] pin.
//15:12 ID Chip ID: This ID is for LinkUp Systems Corporation internal use only.

// COMMAND REGISTER
// ================

#define CMD_S1    (1 << 0) // S1 Voltage Control 1 (5v active low?)
#define CMD_S2    (1 << 1) // S2 Voltage Control 2 (3v active low?)
#define CMD_S3    (1 << 2) // S3 Voltage Control 3 (vcc)
#define CMD_S4    (1 << 3) // S4 Voltage Control 4

#define CMD_RESET (1 << 4) // RESET Software Reset, this bit drives the P_RESET output. 
                           // If set to high the P_RESET output is high.
#define CMD_APOE  (1 << 5) // APOE Automatic power off enable: When this bit is set, 
                           // S[4:1] will be cleared when P_nCD1 or P_nCD2 is high 
                           // effectively turning off the power to the slot. S[4:1] will not
                           // regain the state if P_nCD1 and P_nCD2 should become active but
                           // have to be set under software control.
#define CMD_CFE   (1 << 6) // CFE Compact Flash enable: If this bit is set the device is in
                           // Compact Flash mode. Addressing is limited to the space A[10:0] 
                           // as defined by Compact Flash standard. A[25:11] are driven high.
#define CMD_SOE   (1 << 7) // SOE PC Card socket signal output driver enable. The socket 
                           // signals cannot be driven active unless this bit is set.
#define CMD_SSP   (1 << 8) // SSP Socket Select Polarity: This bit has to be programmed in
                           // a two-slot system.
                           // If this bit is "0" the device will respond when PSKTSEL is "0"
                           // If this bit is "1" the device will respond when PSKTSEL is "1"
#define CMD_TST   (1 << 15)// TST Test bit: This bit should be set to 0 at all times


// H3800
#define A2_BASE          0x49000000
#define PIOA_BASE        (A2_BASE)
#define PIOB_BASE        (A2_BASE + 0x0200)
#define ADC_BASE         (A2_BASE + 0x1200)

#define GLOBAL_IEN       *(volatile cyg_uint32*)(A2_BASE + 0x1600 + 0x00)

#define PIOC_DATA        *(volatile cyg_uint32*)(A2_BASE + 0x1e00 + 0x68)
#define  H3800_SD_PWR    (0x01 << 1)
#define  H3800_LCD_ON    (0x1F << 5 )
#define  H3800_BLPWR_ON  (0x01 << 10)
#define  H3800_SPKR_ON   (0x01 << 12)
#define  H3800_EAR_OFF   (0x01 << 13)
#define  H3800_AUD_PWRON (0x01 << 14)
#define  H36_AUDIO       (SA1110_EIO_AUDIO | SA1110_EIO_MUTE | SA1110_EIO_AMP)
#define  H38_A1_MASK     (SA1110_EIO_LCD_3V3 | H36_AUDIO)

#define PIOA_DIR         *(volatile cyg_uint32*)(PIOA_BASE + 0x00) // High for input
#define PIOA_EDGE        *(volatile cyg_uint32*)(PIOA_BASE + 0x04) // High for edge
#define PIOA_RISE        *(volatile cyg_uint32*)(PIOA_BASE + 0x08) // High for rising
#define PIOA_LEVEL       *(volatile cyg_uint32*)(PIOA_BASE + 0x0C) // High for high
#define PIOA_ISTAT       *(volatile cyg_uint32*)(PIOA_BASE + 0x10) // Write to clear
#define PIOA_DATA        *(volatile cyg_uint32*)(PIOA_BASE + 0x14)
#define PIOA_IEN         *(volatile cyg_uint32*)(PIOA_BASE + 0x1C)
#define PIOA_ALT         *(volatile cyg_uint32*)(PIOA_BASE + 0x3C)
#define  PIOA_Y1         (1 << 0)
#define  PIOA_X0         (1 << 1)
#define  PIOA_Y0         (1 << 2)
#define  PIOA_X1         (1 << 3)
#define  PIOA_PEN        (1 << 5)
#define  PIOA_SD         (1 << 6)
#define  PIOA_SLT        (1 << 11)

#define PIOB_DIR         *(volatile cyg_uint32*)(PIOB_BASE + 0x00) // High for input
#define PIOB_EDGE        *(volatile cyg_uint32*)(PIOB_BASE + 0x04) // High for edge
#define PIOB_RISE        *(volatile cyg_uint32*)(PIOB_BASE + 0x08) // High for rising
#define PIOB_LEVEL       *(volatile cyg_uint32*)(PIOB_BASE + 0x0C) // High for high
#define PIOB_ISTAT       *(volatile cyg_uint32*)(PIOB_BASE + 0x10) // Write to clear
#define PIOB_DATA        *(volatile cyg_uint32*)(PIOB_BASE + 0x14)
#define PIOB_IEN         *(volatile cyg_uint32*)(PIOB_BASE + 0x1C)
#define  PIOB_ADC        (1 << 18)
#define  PIOB_TMR0       (1 << 21)
#define PIOB_ALT         *(volatile cyg_uint32*)(PIOB_BASE + 0x3C)


#define  H3800_OPT_PWR   (1 << 13)
#define  H3800_OPT       (1 << 12)
#define  H3800_CF_RESET  (1 << 8)
#define  H3800_EAR_OUT   (1 << 7)
#define  H3800_SD_EMPTY  (1 << 6)


#define A2_CLOCK_EN      *(volatile cyg_uint32*)(A2_BASE + 0x1000 + 0x00)
#define A2_CLK_EX2       ( 1 << 15 )
#define A2_CLK_EX1       ( 1 << 14 )
#define A2_CLK_EX0       ( 1 << 13 )
#define A2_CLK_SD(x)     ( (x) << 10 )
#define A2_CLK_PWM       ( 1 << 7  )
#define A2_CLK_ADC       ( 1 << 4 )
#define A2_CLK_AUD_0     ( 1 << 0 ) // For 8kHz
#define A2_CLK_AUD_1     ( 1 << 1 ) // For 48kHz
#define A2_CLK_AUD_2     ( 1 << 2 ) // For 11kHz
#define A2_CLK_AUD_3     ( 1 << 3 ) // For 44 or 22 kHz
#define   SD3_HZ 33868800

#define A2_PWM0_TBASE      *(volatile cyg_uint8  *)(A2_BASE + 0x0600 + 0x00)
#define  A2_PWM_TBASE_EN   (1 << 4)
#define  A2_PWM_TBASE_MAX  8
#define A2_PWM0_PERIOD     *(volatile cyg_uint16 *)(A2_BASE + 0x0600 + 0x04)
#define A2_PWM0_DUTY       *(volatile cyg_uint16 *)(A2_BASE + 0x0600 + 0x08)
#define  DUTY_MAX 48
#define  DUTY_MIN 14

#define LED_BASE(n)        (A2_BASE + 0x0800 + ((n)*0x80))
#define LED_TBASE(n)       *(volatile cyg_uint32 *)(LED_BASE(n) + 0x00)
#define  LED_TBASE_MAX     0xD
#define  LED_TBASE_EN      (1 << 4)
#define  LED_TBASE_AS      (1 << 5)
#define  LED_TBASE_AL      (1 << 6)
#define LED_PERIOD(n)      *(volatile cyg_uint32  *)(LED_BASE(n) + 0x04)
#define LED_DUTY(n)        *(volatile cyg_uint32 *)(LED_BASE(n) + 0x08)
#define LED_ASCLK(n)       *(volatile cyg_uint32 *)(LED_BASE(n) + 0x0C)

#define A2_FLASH_CTRL      *(volatile cyg_uint8*)(A2_BASE + 0x1F00)

// - ADC - 10 bit data

#define ADC_MUX            *(volatile cyg_uint32*)(ADC_BASE + 0x00)
#define  MUX_LIGHT         0
#define  MUX_TOUCHX        3
#define  MUX_TOUCHY        4
#define  MUX_CLK_EN        8
#define ADC_CTRL           *(volatile cyg_uint32*)(ADC_BASE + 0x04)
#define  ADC_PRSCL(x)      (0xF & (x))
#define  ADC_FREE_RUN      (1 << 4)
#define  ADC_INT_EN        (1 << 5)
#define  ADC_START         (1 << 6) //Auto clear when done.
#define  ADC_PWR           (1 << 7)
#define ADC_DATA           *(volatile cyg_uint32*)(ADC_BASE + 0x08)

// - MMC

#define MMC_BASE          (A2_BASE + 0x1c00)
#define SDREG_CLKCTRL     *(volatile cyg_uint8 *)(MMC_BASE + 0x00)
#define  CLKCTRL_STOP     1
#define  CLKCTRL_START    2
#define SDREG_STAT        *(volatile cyg_uint16*)(MMC_BASE + 0x04)

#define  STAT_TO_READ     (1 << 0)
#define  STAT_TO_RESP     (1 << 1)
#define  STAT_CRC_WR      (1 << 2)
#define  STAT_CRC_READ    (1 << 3)
#define  STAT_SPI_READ    (1 << 4)
#define  STAT_CRC_RESP    (1 << 5)
#define  STAT_FIFOE       (1 << 6)
#define  STAT_FIFOF       (1 << 7)
#define  STAT_CLOCK       (1 << 8)
#define  STAT_CRC         (3 << 9)
#define  STAT_XFRDONE     (1 << 11)
#define  STAT_WRDONE      (1 << 12)
#define  STAT_RSPDONE     (1 << 13)

#define  STAT_CRC_ERR     (STAT_CRC_WR | STAT_CRC_READ | STAT_CRC_RESP)
#define  STAT_TO_ERR      (STAT_TO_READ | STAT_TO_RESP)
#define  STAT_ERRORS      (STAT_TO_ERR | STAT_CRC_ERR | STAT_SPI_READ)

#define SDREG_CLKFREQ     *(volatile cyg_uint8 *)(MMC_BASE + 0x08)

#define SDREG_RESETREV    *(volatile cyg_uint16*)(MMC_BASE + 0x0C)
#define SDREG_CTRL        *(volatile cyg_uint16*)(MMC_BASE + 0x14)
#define  CTRL_RESP_NONE   0x00
#define  CTRL_RESP_R1     0x01
#define  CTRL_RESP_R2     0x02
#define  CTRL_RESP_R3     0x03
#define  CTRL_DE          (1 << 2)
#define  CTRL_WRITE       (1 << 3)
#define  CTRL_INIT        (1 << 6)
#define  CTRL_BUSY        (1 << 7)

#define SDREG_RESP_TO     *(volatile cyg_uint8 *)(MMC_BASE + 0x18)
#define SDREG_READ_TO     *(volatile cyg_uint16*)(MMC_BASE + 0x1C)

#define SDREG_BLEN        *(volatile cyg_uint16*)(MMC_BASE + 0x20)
#define   DBS_BLOCKSIZE_MASK    0x3ff
#define   DBS_BLOCKSIZE(s)    ((s) & DBS_BLOCKSIZE_MASK)
#define   DBS_BLOCKSIZE_MAX   0x800
#define SDREG_NBLK        *(volatile cyg_uint16*)(MMC_BASE + 0x24)

#define SDREG_INTMASK     *(volatile cyg_uint8*)(MMC_BASE + 0x34)
#define  INT_TX_DONE       (1 << 0)
#define  INT_PRG_DONE      (1 << 1)
#define  INT_CMD_RESP      (1 << 2)
#define  INT_BUF_RDY       (1 << 3)
#define  INT_ALL           (INT_TX_DONE | INT_PRG_DONE | INT_CMD_RESP | INT_BUF_RDY)

#define SDREG_CMD         *(volatile cyg_uint8 *)(MMC_BASE + 0x38)
#define SDREG_ARGH        *(volatile cyg_uint16*)(MMC_BASE + 0x3C)
#define SDREG_ARGL        *(volatile cyg_uint16*)(MMC_BASE + 0x40)
#define SDREG_RSP         *(volatile cyg_uint16*)(MMC_BASE + 0x44)
#define SDREG_DATA        *(volatile cyg_uint16*)(MMC_BASE + 0x4C)

/* end of ipaq.h                                                          */
#endif /* CYGONCE_IPAQ_H */
