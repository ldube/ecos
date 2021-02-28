//==========================================================================
//
//      sound_ipaq.h
//
//      Device definitions for the sound driver on the Compaq iPAQ
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
// The Initial Developer of the Original Code is 3G Lab Ltd.
// Portions created by 3G Lab are                                          
// Copyright (C) 2000 3G Lab Ltd.
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    dominic.ostrowski@3glab.com
// Contributors: dominic.ostrowski@3glab.com
// Date:         2000-04-11
// Purpose:      
// Description:  Sound device driver definitions for the Compaq iPAQ
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_DEVS_SOUND_ARM_IPAQ_H
#define CYGONCE_DEVS_SOUND_ARM_IPAQ_H

// Following should probably be defined in /hal/arm/sa11x0/var/current/include/hal_sa11x0.h)

// SA11X0 DMA Registers

// DMA channels 4 & 5 may be used for USB (though not on iPAQ)
// DMA channels 2 & 3 will be used here.

#define CYGNUM_HAL_INTERRUPT_DMA_2            22
#define CYGNUM_HAL_INTERRUPT_DMA_3            23

#define SA11X0_DMA_2_DDAR                     SA11X0_REGISTER(0x30000040)
#define SA11X0_DMA_2_DCSR_SET                 SA11X0_REGISTER(0x30000044)
#define SA11X0_DMA_2_DCSR_CLEAR               SA11X0_REGISTER(0x30000048)
#define SA11X0_DMA_2_DCSR_READ                SA11X0_REGISTER(0x3000004C)
#define SA11X0_DMA_2_DBSA                     SA11X0_REGISTER(0x30000050)
#define SA11X0_DMA_2_DBTA                     SA11X0_REGISTER(0x30000054)
#define SA11X0_DMA_2_DBSB                     SA11X0_REGISTER(0x30000058)
#define SA11X0_DMA_2_DBTB                     SA11X0_REGISTER(0x3000005C)

#define SA11X0_DMA_3_DDAR                     SA11X0_REGISTER(0x30000060)
#define SA11X0_DMA_3_DCSR_SET                 SA11X0_REGISTER(0x30000064)
#define SA11X0_DMA_3_DCSR_CLEAR               SA11X0_REGISTER(0x30000068)
#define SA11X0_DMA_3_DCSR_READ                SA11X0_REGISTER(0x3000006C)
#define SA11X0_DMA_3_DBSA                     SA11X0_REGISTER(0x30000070)
#define SA11X0_DMA_3_DBTA                     SA11X0_REGISTER(0x30000074)
#define SA11X0_DMA_3_DBSB                     SA11X0_REGISTER(0x30000078)
#define SA11X0_DMA_3_DBTB                     SA11X0_REGISTER(0x3000007C)

#define CYGNUM_HAL_INTERRUPT_DMA_PLAY         CYGNUM_HAL_INTERRUPT_DMA_2
#define CYGNUM_HAL_INTERRUPT_DMA_RECORD       CYGNUM_HAL_INTERRUPT_DMA_3

#define SA11X0_DMA_PLAY_DDAR                  SA11X0_DMA_2_DDAR 
#define SA11X0_DMA_PLAY_DCSR_SET              SA11X0_DMA_2_DCSR_SET
#define SA11X0_DMA_PLAY_DCSR_CLEAR            SA11X0_DMA_2_DCSR_CLEAR  
#define SA11X0_DMA_PLAY_DCSR_READ             SA11X0_DMA_2_DCSR_READ
#define SA11X0_DMA_PLAY_DBSA                  SA11X0_DMA_2_DBSA 
#define SA11X0_DMA_PLAY_DBTA                  SA11X0_DMA_2_DBTA
#define SA11X0_DMA_PLAY_DBSB                  SA11X0_DMA_2_DBSB 
#define SA11X0_DMA_PLAY_DBTB                  SA11X0_DMA_2_DBTB

#define SA11X0_DMA_RECORD_DDAR                SA11X0_DMA_3_DDAR 
#define SA11X0_DMA_RECORD_DCSR_SET            SA11X0_DMA_3_DCSR_SET
#define SA11X0_DMA_RECORD_DCSR_CLEAR          SA11X0_DMA_3_DCSR_CLEAR  
#define SA11X0_DMA_RECORD_DCSR_READ           SA11X0_DMA_3_DCSR_READ
#define SA11X0_DMA_RECORD_DBSA                SA11X0_DMA_3_DBSA 
#define SA11X0_DMA_RECORD_DBTA                SA11X0_DMA_3_DBTA
#define SA11X0_DMA_RECORD_DBSB                SA11X0_DMA_3_DBSB 
#define SA11X0_DMA_RECORD_DBTB                SA11X0_DMA_3_DBTB

#define SA11X0_DMA_DDAR_READ                  0x1 
#define SA11X0_DMA_DDAR_BIG_ENDIAN            0x2 
#define SA11X0_DMA_DDAR_DATUMS_8              0x4 
#define SA11X0_DMA_DDAR_DATUMS_SIZE_16        0x8 
#define SA11X0_DMA_DDAR_SSP                   0x81C01B00
#define SA11X0_DMA_DDAR_SSP_TX                0xE0 

#define SA11X0_DMA_DCSR_RUN                   0x1 
#define SA11X0_DMA_DCSR_IE                    0x2 
#define SA11X0_DMA_DCSR_ERROR                 0x4 
#define SA11X0_DMA_DCSR_DONEA                 0x8 
#define SA11X0_DMA_DCSR_STRTA                 0x10
#define SA11X0_DMA_DCSR_DONEB                 0x20 
#define SA11X0_DMA_DCSR_STRTB                 0x40
#define SA11X0_DMA_DCSR_BIU                   0x80 

// SA11X0 SSP Registers

#define SA11X0_SSP_CONTROL0                   SA11X0_REGISTER(0x70060)
#define SA11X0_SSP_CONTROL1                   SA11X0_REGISTER(0x70064)
#define SA11X0_SSP_DATA                       SA11X0_REGISTER(0x7006C)
#define SA11X0_SSP_STATUS                     SA11X0_REGISTER(0x70074)

// SA11X0 SSP Control Register 0 Bit Fields.

#define SA11X0_SSP_DATA_BITS_4                0x03
#define SA11X0_SSP_DATA_BITS_5                0x04
#define SA11X0_SSP_DATA_BITS_6                0x05
#define SA11X0_SSP_DATA_BITS_7                0x06
#define SA11X0_SSP_DATA_BITS_8                0x07
#define SA11X0_SSP_DATA_BITS_9                0x08
#define SA11X0_SSP_DATA_BITS_10               0x09
#define SA11X0_SSP_DATA_BITS_11               0x0A
#define SA11X0_SSP_DATA_BITS_12               0x0B
#define SA11X0_SSP_DATA_BITS_13               0x0C
#define SA11X0_SSP_DATA_BITS_14               0x0D
#define SA11X0_SSP_DATA_BITS_15               0x0E
#define SA11X0_SSP_DATA_BITS_16               0x0F

#define SA11X0_SSP_DATA_FRAME_FORMAT_MOTOROLA 0x00
#define SA11X0_SSP_DATA_FRAME_FORMAT_TEXAS    0x10
#define SA11X0_SSP_DATA_FRAME_FORMAT_NATIONAL 0x20

#define SA11X0_SSP_ENABLE                     0x80

#define SA11X0_SSP_DIVIDE_BY_8                0x300

#define SA11X0_SSP_EXTERNAL_CLOCK_POLARITY    0x08
#define SA11X0_SSP_EXTERNAL_CLOCK_PHASE       0x10
#define SA11X0_SSP_EXTERNAL_CLOCK_SELECT      0x20

// Sound stuff starts here

// IPAQ implements L3 bus through GPIO

#define SA1110_GPIO_CLK_SET0                  SA11X0_GPIO_PIN_12
#define SA1110_GPIO_CLK_SET1                  SA11X0_GPIO_PIN_13
#define SA1110_GPIO_L3_DATA                   SA11X0_GPIO_PIN_14
#define SA1110_GPIO_L3_MODE                   SA11X0_GPIO_PIN_15
#define SA1110_GPIO_L3_CLOCK                  SA11X0_GPIO_PIN_16
#define SA1110_GPIO_SYS_CLOCK                 SA11X0_GPIO_PIN_19

// L3 setup and hold times (expressed in us)

#define L3_DataSetupTime                      hal_delay_us(1) // 190 ns
#define L3_DataHoldTime                       hal_delay_us(1) //  30 ns
#define L3_ModeSetupTime                      hal_delay_us(1) // 190 ns
#define L3_ModeHoldTime                       hal_delay_us(1) // 190 ns
#define L3_ClockHighTime                      hal_delay_us(1) // 250 ns
#define L3_ClockLowTime                       hal_delay_us(1) // 250 ns
#define L3_HaltTime                           hal_delay_us(1) // 190 ns

// UDA1341 L3 address and command types

#define UDA1341_L3Addr	5
#define UDA1341_DATA0	0
#define UDA1341_DATA1	1
#define UDA1341_STATUS	2

// Definitions

#define AUDIO_FMT_MASK		              (AFMT_S16_LE)
#define AUDIO_FMT_DEFAULT	              (AFMT_S16_LE)
#define AUDIO_CHANNELS_DEFAULT	              2
#define AUDIO_RATE_DEFAULT	              CYG_IO_SET_SPEED_22050
//#define AUDIO_RATE_DEFAULT	              CYG_IO_SET_SPEED_44100
 
// UDA1341 internal state variables.  
// Those are initialized according to what it should be after a reset.

// UDA1341 status settings

#define UDA_STATUS0_IF_I2S                    0
#define UDA_STATUS0_IF_LSB16                  1
#define UDA_STATUS0_IF_LSB18                  2
#define UDA_STATUS0_IF_LSB20                  3
#define UDA_STATUS0_IF_MSB                    4
#define UDA_STATUS0_IF_MSB16                  5
#define UDA_STATUS0_IF_MSB18                  6
#define UDA_STATUS0_IF_MSB20                  7

#define UDA_STATUS0_SC_512FS                  0
#define UDA_STATUS0_SC_384FS                  1
#define UDA_STATUS0_SC_256FS                  2

struct STATUS_0 {
    unsigned    DC_filter	: 1;    // DC filter
    unsigned	input_fmt	: 3;    // data input format
    unsigned	system_clk	: 2;    // system clock frequency
    unsigned	reset		: 1;    // reset
    unsigned	select	        : 1;    // must be set to 0
};

struct STATUS_1 {
    unsigned	DAC_on		: 1;	// DAC powered
    unsigned	ADC_on		: 1;	// DC powered
    unsigned	double_speed	: 1;	// double speed playback
    unsigned	DAC_pol		: 1;	// polarity of DAC
    unsigned	ADC_pol		: 1;	// polarity of ADC
    unsigned	ADC_gain	: 1;	// gain of ADC
    unsigned	DAC_gain	: 1;	// gain of DAC
    unsigned	select	        : 1;	// must be set to 1
};

// UDA1341 direct control settings

struct DATA0_0 {
    unsigned	volume		: 6;	// volume control
    unsigned	select	        : 2;	// must be set to 0
};

struct DATA0_1{
    unsigned	treble		: 2;
    unsigned	bass		: 4;
    unsigned	select	        : 2;	// must be set to 1
};

struct DATA0_2 {
    unsigned	mode		: 2;	// mode switch
    unsigned	mute		: 1;
    unsigned	deemphasis	: 2;
    unsigned	peak_detect	: 1;
    unsigned	select          : 2;	// must be set to 2
};

// DATA0 extended programming registers

struct DATA0_ext0 {
    unsigned	ext_addr        : 3;	// must be set to 0
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	ch1_gain	: 5;	// mixer gain channel 1
    unsigned	select2	        : 3;	// must be set to 7
};

struct DATA0_ext1{
    unsigned	ext_addr        : 3;	// must be set to 1
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	ch2_gain	: 5;	// mixer gain channel 2
    unsigned select2	        : 3;	// must be set to 7
};

struct DATA0_ext2 {
    unsigned	ext_addr        : 3;	// must be set to 2
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	mixer_mode      : 2;
    unsigned	mic_level	: 3;	// MIC sensitivity level
    unsigned	select2	        : 3;	// must be set to 7
};

struct DATA0_ext4 {
    unsigned	ext_addr        : 3;	// must be set to 4
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	ch2_igain_l	: 2;	// input amplifier gain channel 2 
                                        // (bits 1-0)
    unsigned	reserved        : 2;	// must be set to 0
    unsigned	AGC_ctrl	: 1;	// AGC control
    unsigned	select2         : 3;	// must be set to 7
};

struct DATA0_ext5 {
    unsigned	ext_addr        : 3;	// must be set to 5
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	ch2_igain_h	: 5;	// input amplifier gain channel 2 
                                        //(bits 6-2)
    unsigned	select2	        : 3;	// must be set to 7
};

struct DATA0_ext6 {
    unsigned	ext_addr        : 3;	// must be set to 6
    unsigned	select1	        : 5;	// must be set to 24
    unsigned	AGC_level	: 2;	// AGC output level
    unsigned	AGC_const	: 3;	// AGC time constant
    unsigned	select2	        : 3;	// must be set to 7
};

struct DATA1 {
    unsigned peak              : 6;    // peak level value
};

// The sound_write() function is sent messages by setting bits in a cyg_flag. 
#define DSR_UNBLOCK		((cyg_flag_value_t)0x1)
#define BLOCK_SOUND_WRITE	((cyg_flag_value_t)0xF)

struct user_channel {

    // START_TEMPORARY

    // The real driver side buffers - these are a temporary fix to 
    // provide a sound driver that will work with and without memory 
    // protection.
    cyg_uint16       log_buffer_0[SOUND_BUFFER_SIZE];
    cyg_uint16       log_buffer_1[SOUND_BUFFER_SIZE];

    // The driver side pointers to the buffers associated with the channel
    cyg_uint16*      phys_buffer_0;
    cyg_uint16*      phys_buffer_1;

    // END_TEMPORARY

    // The association betweeen the user and driver side resources
    cyg_handle_t     user_handle;

    // Indicates whether lower priority channels may resume
    cyg_bool channel_open;

    // Used by the user, and functions indpendently of state
    cyg_bool pause;

    // Blocks sound_write for this channel user until at least one 
    // buffer is free
    cyg_flag_t	block_flag;

    // State bits, any channel may be in one only of three states,
    // idle, suspended or playing - but only one channel may be playing
    // at any time.
    cyg_uint32 state;

    // Indicates this channel is free to unblock - as the dsr may not be
    // called before another isr occurs, and the dsr must be associated
    // with the same channel the isr acted upon.
    cyg_bool dsr_pending;

    // These indicate a buffer has been filled with data but has either
    // not started or not finished playing
    cyg_bool         phys_buffer_0_free;
    cyg_bool         phys_buffer_1_free;

    // Indicates which buffer is being played
    cyg_bool         current_phys_buffer_is_0; 

    // Holds values used by DMA hardware
    cyg_uint32       phys_buffer_0_len;
    cyg_uint32       phys_transfer_0_start_address;
    cyg_uint32       phys_buffer_1_len;
    cyg_uint32       phys_transfer_1_start_address;

    // UDA1341 Settings
    cyg_uint32       audio_rate;

    struct STATUS_0         chan_STATUS_0;
    struct STATUS_1         chan_STATUS_1;
    struct DATA0_0          chan_DATA0_0;
    struct DATA0_1          chan_DATA0_1;
    struct DATA0_2          chan_DATA0_2;
    struct DATA0_ext0       chan_DATA0_ext0;
    struct DATA0_ext1       chan_DATA0_ext1;
    struct DATA0_ext2       chan_DATA0_ext2;
    struct DATA0_ext4       chan_DATA0_ext4;
    struct DATA0_ext5       chan_DATA0_ext5;
    struct DATA0_ext6       chan_DATA0_ext6;
    struct DATA1            chan_DATA1;
};

#define CHANNEL_STATE_IDLE               0
#define CHANNEL_STATE_SUSPENDED          1
#define CHANNEL_STATE_PLAYING            2

#endif





