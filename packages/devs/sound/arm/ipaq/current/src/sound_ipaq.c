//==========================================================================
//
//      sound_ipaq.c
//
//      Sound driver for the Compaq Ipaq
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
// Copyright (C) 2001 3G Lab Ltd.
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    dominic.ostrowski@3glab.com
// Contributors: dominic.ostrowski@3glab.com
// Date:         2001-04-11
// Purpose:      
// Description:  Sound driver for the Compaq Ipaq
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_sound_arm_ipaq.h>
#include <cyg/kernel/kapi.h> // builds without?
#include <cyg/hal/hal_io.h> // builds without?
#include <cyg/hal/hal_arch.h> // builds without?
#include <cyg/hal/drv_api.h> // builds without?
#include <cyg/hal/hal_intr.h> // builds without?
#include "cyg/hal/hal_cache.h" // needed for HAL_VIRT_TO_PHYS_ADDRESS
#include <cyg/hal/hal_sa11x0.h> // builds without (now) but prob reqd
#include <cyg/hal/ipaq.h> // builds without (now) but prob reqd
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>

#include <cyg/io/devtab.h>
#include <cyg/io/sound.h>
#include "sound_ipaq.h" //may be required for signalling events

// This driver allows up to three priorities of sound driver user.
// Any user opening a channel to the sound driver, must set a priority
// for itself. Writes with no priority assigned are ignored.
// Higher priority users cause lower priority users to suspend until the
// DMA hardware becomes free. It is therefore important that higher priority
// users maintain full buffers.
// No checks are made for a priority already being occupied, the newcomer
// always displaces the current holder. All sound driver users must ensure
// that there are no priority contentions.
// Similarly all audio settings are user relative, priority must be set before
// audio settings are changed.

static cyg_interrupt    ssp_interrupt;
static cyg_handle_t     ssp_int_handle;

// An array of buffers and associated variables are used to
// handle the users of this driver - each functions completely
// independently - retaining their own settings

static struct user_channel sound_channel_array[SOUND_DRIVER_CHANNELS];

////////////////////////////////////////////////////////////////////////////
//
// CODE starts here
//	These are support functions for the UCB server
//
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// L3 interface
//
///////////////////////////////////////////////////////////////////////////////

static inline void L3_acquirepins(void)
{
    *SA11X0_GPIO_PIN_OUTPUT_SET = 
        (SA1110_GPIO_L3_MODE | SA1110_GPIO_L3_CLOCK | SA1110_GPIO_L3_DATA);
    *SA11X0_GPIO_PIN_DIRECTION |=  
        (SA1110_GPIO_L3_MODE | SA1110_GPIO_L3_CLOCK|SA1110_GPIO_L3_DATA);
}

static inline void L3_releasepins(void)
{
    *SA11X0_GPIO_PIN_DIRECTION &= 
        ~(SA1110_GPIO_L3_MODE | SA1110_GPIO_L3_CLOCK|SA1110_GPIO_L3_DATA);
}

static void L3_init(void)
{
    *SA11X0_GPIO_ALTERNATE_FUNCTION &= 
        ~(SA1110_GPIO_L3_DATA | SA1110_GPIO_L3_CLOCK | SA1110_GPIO_L3_MODE);
    L3_releasepins();
}

static inline int L3_getbit(void)
{
    int data;

    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_CLOCK;
    L3_ClockLowTime;

    data = (*SA11X0_GPIO_PIN_LEVEL & SA1110_GPIO_L3_DATA) ? 1 : 0;

    *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_CLOCK;
    L3_ClockHighTime;

    return data;
}

static inline void L3_sendbit(int bit)
{
    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_CLOCK;

    if (bit & 1)
        *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_DATA;
    else
        *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_DATA;

    L3_ClockLowTime;

    *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_CLOCK;
    L3_ClockHighTime;
}

static void L3_sendbyte(char data, int mode)
{
    int i;

    switch(mode) {
        case 0:
	    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_MODE;
	    break;
	case 1:
	    break;
	default:
	    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_MODE;
	    L3_HaltTime;
	    *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_MODE;
	    break;
    }

    L3_ModeSetupTime;

    for (i = 0; i < 8; i++)
        L3_sendbit(data >> i);

    if (mode == 0)
        *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_MODE;

    L3_ModeHoldTime;
}

static char L3_getbyte(int mode)
{
    char data = 0;
    int i;

    switch(mode) {
	case 0:
	    break;
	case 1:
	    break;
	default:
	    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_L3_MODE;
	    L3_HaltTime;
	    *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_L3_MODE;
	    break;
    }

    L3_ModeSetupTime;

    for (i = 0; i < 8; i++)
        data |= (L3_getbit() << i);

    L3_ModeHoldTime;

    return data;
}

static int L3_write(char addr, char *data, int len)
{
    int mode = 0;
    int bytes = len;

    L3_acquirepins();
    L3_sendbyte(addr, mode++);
    while(len--)
	L3_sendbyte(*data++, mode++);
    L3_releasepins();

    return bytes;
}

static int L3_read(char addr, char * data, int len)
{
    int mode = 0;
    int bytes = len;

    L3_acquirepins();
    L3_sendbyte(addr, mode++);
    while(len--)
        *data++ = L3_getbyte(mode++);
    L3_releasepins();

    return bytes;
}

///////////////////////////////////////////////////////////////////////////////
//
// UDA1341 interface
//
///////////////////////////////////////////////////////////////////////////////

static inline void audio_ipaq_init(void)
{
    // General DMA initialisation
    *SA11X0_DMA_PLAY_DDAR = SA11X0_DMA_DDAR_DATUMS_SIZE_16 
        | SA11X0_DMA_DDAR_SSP | SA11X0_DMA_DDAR_SSP_TX;
    *SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_IE;

    // Setup the uarts
    *SA11X0_GPIO_ALTERNATE_FUNCTION &= ~(SA11X0_GPIO_PIN_10 | SA11X0_GPIO_PIN_11 | SA11X0_GPIO_PIN_12 | SA11X0_GPIO_PIN_13);
    *SA11X0_GPIO_ALTERNATE_FUNCTION |= (SA1110_GPIO_SYS_CLOCK);
    *SA11X0_GPIO_PIN_DIRECTION &= ~(SA1110_GPIO_SYS_CLOCK);

    *SA11X0_SSP_CONTROL0 = 0;
    *SA11X0_SSP_CONTROL0 = 
        SA11X0_SSP_DATA_BITS_16 + SA11X0_SSP_DATA_FRAME_FORMAT_TEXAS;
    *SA11X0_SSP_CONTROL0 |= SA11X0_SSP_DIVIDE_BY_8;
    *SA11X0_SSP_CONTROL1 = SA11X0_SSP_EXTERNAL_CLOCK_SELECT;
    *SA11X0_SSP_CONTROL0 |= SA11X0_SSP_ENABLE;

    // Enable the audio power
    ipaq_EGPIO( SA1110_EIO_CODEC_RESET, SA1110_EIO_CODEC_RESET_DISABLE);
    ipaq_EGPIO( SA1110_EIO_AMP, SA1110_EIO_AMP_ON);
    ipaq_EGPIO( SA1110_EIO_AUDIO, SA1110_EIO_AUDIO_ON);
    ipaq_EGPIO( SA1110_EIO_MUTE, SA1110_EIO_MUTE_OFF);
	
    // external clock configured for 44100 samples/sec
    *SA11X0_GPIO_PIN_DIRECTION |= 
        (SA1110_GPIO_CLK_SET0 | SA1110_GPIO_CLK_SET1);
    *SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_CLK_SET0;
    *SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_CLK_SET1;

    // Wait for the UDA1341 to wake up
    hal_delay_us(100000);
}

static inline void audio_power_off(void)
{
  // Disable the audio power
    ipaq_EGPIO( SA1110_EIO_MUTE, SA1110_EIO_MUTE_ON);
    ipaq_EGPIO( SA1110_EIO_AUDIO, SA1110_EIO_AUDIO_OFF);
    ipaq_EGPIO( SA1110_EIO_AMP, SA1110_EIO_AMP_OFF);
    ipaq_EGPIO( SA1110_EIO_CODEC_RESET, SA1110_EIO_CODEC_RESET_ENABLE);
}

static void audio_uda1341_reset(void)
{
    // Reset the chip
    char reset;
    reset = 1;
    L3_write((UDA1341_L3Addr << 2) | UDA1341_STATUS, &reset, 1);

    ipaq_EGPIO( SA1110_EIO_CODEC_RESET, SA1110_EIO_CODEC_RESET_ENABLE);
    ipaq_EGPIO( SA1110_EIO_CODEC_RESET, SA1110_EIO_CODEC_RESET_DISABLE);

    reset = 0;
    L3_write((UDA1341_L3Addr << 2) | UDA1341_STATUS, &reset, 1);
}

int audio_uda1341_init(int channel)
{
    sound_channel_array[channel].audio_rate = AUDIO_RATE_DEFAULT;
    sound_channel_array[channel].chan_STATUS_0.DC_filter = 0;
    sound_channel_array[channel].chan_STATUS_0.input_fmt = 
        UDA_STATUS0_IF_LSB16;
    sound_channel_array[channel].chan_STATUS_0.system_clk =  
        UDA_STATUS0_SC_256FS;
    sound_channel_array[channel].chan_STATUS_0.reset = 0;
    sound_channel_array[channel].chan_STATUS_0.select = 0;

    sound_channel_array[channel].chan_STATUS_1.DAC_on = 1;
    sound_channel_array[channel].chan_STATUS_1.ADC_on = 1;
    sound_channel_array[channel].chan_STATUS_1.double_speed = 0;
    sound_channel_array[channel].chan_STATUS_1.DAC_pol = 0;
    sound_channel_array[channel].chan_STATUS_1.ADC_pol = 0;
    sound_channel_array[channel].chan_STATUS_1.DAC_gain = 0;
    sound_channel_array[channel].chan_STATUS_1.ADC_gain = 0;
    sound_channel_array[channel].chan_STATUS_1.select = 1;

    sound_channel_array[channel].chan_DATA0_0.volume = 55;
    sound_channel_array[channel].chan_DATA0_0.select = 0;

    sound_channel_array[channel].chan_DATA0_1.treble = 0;
    sound_channel_array[channel].chan_DATA0_1.bass = 0;
    sound_channel_array[channel].chan_DATA0_1.select = 1;

    sound_channel_array[channel].chan_DATA0_2.mode = 0;
    sound_channel_array[channel].chan_DATA0_2.mute = 0;
    sound_channel_array[channel].chan_DATA0_2.deemphasis = 0;
    sound_channel_array[channel].chan_DATA0_2.peak_detect = 1;
    sound_channel_array[channel].chan_DATA0_2.select = 2;

    sound_channel_array[channel].chan_DATA0_ext0.ext_addr = 0;
    sound_channel_array[channel].chan_DATA0_ext0.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext0.ch1_gain = 4;
    sound_channel_array[channel].chan_DATA0_ext0.select2 = 7;

    sound_channel_array[channel].chan_DATA0_ext1.ext_addr = 1;
    sound_channel_array[channel].chan_DATA0_ext1.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext1.ch2_gain = 4;
    sound_channel_array[channel].chan_DATA0_ext1.select2 = 7;

    sound_channel_array[channel].chan_DATA0_ext2.ext_addr = 2;
    sound_channel_array[channel].chan_DATA0_ext2.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext2.mixer_mode = 0;
    sound_channel_array[channel].chan_DATA0_ext2.mic_level = 1;
    sound_channel_array[channel].chan_DATA0_ext2.select2 = 7;

    sound_channel_array[channel].chan_DATA0_ext4.ext_addr = 4;
    sound_channel_array[channel].chan_DATA0_ext4.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext4.ch2_igain_l = 0 & 3;
    sound_channel_array[channel].chan_DATA0_ext4.reserved = 0;
    sound_channel_array[channel].chan_DATA0_ext4.AGC_ctrl = 0;
    sound_channel_array[channel].chan_DATA0_ext4.select2 = 7;

    sound_channel_array[channel].chan_DATA0_ext5.ext_addr = 5;
    sound_channel_array[channel].chan_DATA0_ext5.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext5.ch2_igain_h = 0 >> 2;
    sound_channel_array[channel].chan_DATA0_ext5.select2 = 7;

    sound_channel_array[channel].chan_DATA0_ext6.ext_addr = 6;
    sound_channel_array[channel].chan_DATA0_ext6.select1 = 24;
    sound_channel_array[channel].chan_DATA0_ext6.AGC_level = 0;
    sound_channel_array[channel].chan_DATA0_ext6.AGC_const = 0;
    sound_channel_array[channel].chan_DATA0_ext6.select2 = 7;

    sound_channel_array[channel].chan_DATA1.peak = 0;

    return 0;
}

static void audio_set_dsp_speed(int val, int channel) {

    switch (val) {
    case CYG_IO_SET_SPEED_8000:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_512FS;
	    break;
    case CYG_IO_SET_SPEED_11025:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_512FS;
	    break;
    case CYG_IO_SET_SPEED_16000:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_256FS;
	    break;
    case CYG_IO_SET_SPEED_22050:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_256FS;
	    break;
    case CYG_IO_SET_SPEED_32000:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_384FS;
	    break;
    case CYG_IO_SET_SPEED_44100:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_256FS;
	    break;
    case CYG_IO_SET_SPEED_48000:
	    sound_channel_array[channel].chan_STATUS_0.system_clk = 
                UDA_STATUS0_SC_256FS;
	    break;
    }
}

static void set_1341_dsp(int val) {
    // set the external clock generator
    switch (val) {
    case CYG_IO_SET_SPEED_32000:
    case CYG_IO_SET_SPEED_48000:
        // 00:
	*SA11X0_GPIO_PIN_OUTPUT_CLEAR = 
            SA1110_GPIO_CLK_SET0 | SA1110_GPIO_CLK_SET1;
	break;
    case CYG_IO_SET_SPEED_44100:
        // 01: 11.2896 MHz
	*SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_CLK_SET0;
	*SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_CLK_SET1;
	break;
    case CYG_IO_SET_SPEED_8000:
    case CYG_IO_SET_SPEED_16000:
        // 10: 4.096 MHz
	*SA11X0_GPIO_PIN_OUTPUT_CLEAR = SA1110_GPIO_CLK_SET0;
	*SA11X0_GPIO_PIN_OUTPUT_SET = SA1110_GPIO_CLK_SET1;
	break;
    case CYG_IO_SET_SPEED_11025:
    case CYG_IO_SET_SPEED_22050:
        // 11: 5.6245 MHz */
	*SA11X0_GPIO_PIN_OUTPUT_SET = 
            SA1110_GPIO_CLK_SET0 | SA1110_GPIO_CLK_SET1;
	break;
    }
}

static void set_1341(int channel)
{
    // Set the UDA1341 hardware to reflect the current channel settings

    set_1341_dsp(sound_channel_array[channel].audio_rate);

    // Load the setting associated with this channel
    L3_write((UDA1341_L3Addr << 2) | UDA1341_STATUS, 
        (char *) &sound_channel_array[channel].chan_STATUS_0, 1);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_STATUS, 
        (char *) &sound_channel_array[channel].chan_STATUS_1, 1);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_0, 1);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_1, 1);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_2, 1);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext0, 2);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext1, 2);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext2, 2);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext4, 2);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext5, 2);
    L3_write((UDA1341_L3Addr << 2) | UDA1341_DATA0, 
        (char *) &sound_channel_array[channel].chan_DATA0_ext6, 2);
}

static void audio_power_on(void)
{
    L3_init();

    audio_ipaq_init();

    audio_uda1341_reset();
}

///////////////////////////////////////////////////////////////////////////////
//
// Channel interface
//
///////////////////////////////////////////////////////////////////////////////

static void channel_init(int channel) {

    // START_TEMPORARY

    HAL_VIRT_TO_PHYS_ADDRESS(sound_channel_array[channel].log_buffer_0, 
        sound_channel_array[channel].phys_buffer_0);
    HAL_VIRT_TO_PHYS_ADDRESS(sound_channel_array[channel].log_buffer_1, 
        sound_channel_array[channel].phys_buffer_1);

    // END_TEMPORARY

    cyg_flag_init( &sound_channel_array[channel].block_flag );

    // Both buffers start off marked free, 
    // they are marked as in use when sound_write is called.
    // (sound_write will block until a buffer is free)
    // They are marked free again in the isr,
    // which will call the dsr to unblock sound_write.
    sound_channel_array[channel].phys_buffer_0_free = true;
    sound_channel_array[channel].phys_buffer_1_free = true;

    sound_channel_array[channel].current_phys_buffer_is_0 = true;

    sound_channel_array[channel].pause = false;
    sound_channel_array[channel].state = CHANNEL_STATE_IDLE;
    sound_channel_array[channel].dsr_pending = false;
    sound_channel_array[channel].channel_open = false;
    sound_channel_array[channel].user_handle = (cyg_handle_t)NULL;

    // UDA1341 settings are held seperately for each channel
    audio_uda1341_init(channel);
}

///////////////////////////////////////////////////////////////////////////////
//
// SSP interface
//
///////////////////////////////////////////////////////////////////////////////

static void 
play_buffer_0(int channel) {

    if (sound_channel_array[channel].pause == false) {
	// The DMA hardware requires that we alternate between DMA buffer A
	// and B.
	if (*SA11X0_DMA_PLAY_DCSR_READ & SA11X0_DMA_DCSR_BIU) { 
	    *SA11X0_DMA_PLAY_DBSB = 
		sound_channel_array[channel].phys_transfer_0_start_address;
	    if (sound_channel_array[channel].phys_buffer_0_len > 
		SA11X0_DMA_TRANSFER_SIZE) {
		*SA11X0_DMA_PLAY_DBTB = SA11X0_DMA_TRANSFER_SIZE;
		sound_channel_array[channel].phys_buffer_0_len -= 
		    SA11X0_DMA_TRANSFER_SIZE;
	    } else {
		*SA11X0_DMA_PLAY_DBTB = 
		    sound_channel_array[channel].phys_buffer_0_len;
		sound_channel_array[channel].phys_buffer_0_len = 0;
	    }
	    *SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_IE;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_RUN 
		| SA11X0_DMA_DCSR_STRTB;
	} else {
	    *SA11X0_DMA_PLAY_DBSA = 
		sound_channel_array[channel].phys_transfer_0_start_address;
	    if (sound_channel_array[channel].phys_buffer_0_len > 
		SA11X0_DMA_TRANSFER_SIZE) {
		*SA11X0_DMA_PLAY_DBTA = SA11X0_DMA_TRANSFER_SIZE;
		sound_channel_array[channel].phys_buffer_0_len -= 
		    SA11X0_DMA_TRANSFER_SIZE;
	    }
	    else {
		*SA11X0_DMA_PLAY_DBTA = 
		    sound_channel_array[channel].phys_buffer_0_len;
		sound_channel_array[channel].phys_buffer_0_len = 0;
	    }
	    *SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_IE;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_RUN 
		| SA11X0_DMA_DCSR_STRTA;
	}
    }
}

static void 
play_buffer_1(int channel) {

    if (sound_channel_array[channel].pause == false) {
	if (*SA11X0_DMA_PLAY_DCSR_READ & SA11X0_DMA_DCSR_BIU) { 
	    *SA11X0_DMA_PLAY_DBSB = 
		sound_channel_array[channel].phys_transfer_1_start_address;
	    if (sound_channel_array[channel].phys_buffer_1_len > 
		SA11X0_DMA_TRANSFER_SIZE) {
		*SA11X0_DMA_PLAY_DBTB = SA11X0_DMA_TRANSFER_SIZE;
		sound_channel_array[channel].phys_buffer_1_len -= 
		    SA11X0_DMA_TRANSFER_SIZE;
	    }
	    else {
		*SA11X0_DMA_PLAY_DBTB = 
		    sound_channel_array[channel].phys_buffer_1_len;
		sound_channel_array[channel].phys_buffer_1_len = 0;
	    }
	    *SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_IE;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_RUN 
		| SA11X0_DMA_DCSR_STRTB;
	} else {
	    *SA11X0_DMA_PLAY_DBSA = 
		sound_channel_array[channel].phys_transfer_1_start_address;
	    if (sound_channel_array[channel].phys_buffer_1_len > 
		SA11X0_DMA_TRANSFER_SIZE) {
		*SA11X0_DMA_PLAY_DBTA = SA11X0_DMA_TRANSFER_SIZE;
		sound_channel_array[channel].phys_buffer_1_len -= 
		    SA11X0_DMA_TRANSFER_SIZE;
	    }
	    else {
		*SA11X0_DMA_PLAY_DBTA = 
		    sound_channel_array[channel].phys_buffer_1_len;
		sound_channel_array[channel].phys_buffer_1_len = 0;
	    }
	    *SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_IE;
	    *SA11X0_DMA_PLAY_DCSR_SET = SA11X0_DMA_DCSR_RUN 
		| SA11X0_DMA_DCSR_STRTA;
	}
    }
}

static int 
suspend(int channel) {
    int i;
    // Check if a higher priority channel is suspended,
    // if it is, mark this channel as suspended and 
    // mark the new channel as playing. 
    for (i=0; i<channel; i++) {
	if (sound_channel_array[i].state == CHANNEL_STATE_SUSPENDED) {
	    sound_channel_array[channel].state = CHANNEL_STATE_SUSPENDED;
	    // We got here through the isr, so this buffer is partially played
	    // and we must move on the buffer as part of the suspend
	    if(sound_channel_array[channel].current_phys_buffer_is_0) {
		sound_channel_array[channel].phys_transfer_0_start_address += 
		    SA11X0_DMA_TRANSFER_SIZE;
	    } else {
		sound_channel_array[channel].phys_transfer_1_start_address += 
		    SA11X0_DMA_TRANSFER_SIZE;
	    }
            set_1341(i);
	    sound_channel_array[i].state = CHANNEL_STATE_PLAYING;
	    return i;
	}
    }
    return channel;
}

static int 
resume(int channel) {
    int i;
    // Mark this channel as idle.
    // Check if another channel is suspended,
    // if it is, mark this other channel as playing.
    sound_channel_array[channel].state = CHANNEL_STATE_IDLE;
    // If this channel is also closed i.e. this isn't a brief
    // hold up from the user side!
//if (sound_channel_array[channel].channel_open ==false) {
    for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	if (sound_channel_array[i].state == CHANNEL_STATE_SUSPENDED) {
	    sound_channel_array[i].state = CHANNEL_STATE_PLAYING;
            set_1341(i);
	    // We are either resuming an unstarted channel or the
	    // buffer was moved on when the channel was suspended
	    if(sound_channel_array[i].current_phys_buffer_is_0) {
		play_buffer_0(i);
	    } else {
		play_buffer_1(i);
	    }
	    return i;
	}
    }
//}
    return channel;
}

static int
ssp_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs) {

    int channel,i;
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_DMA_PLAY);

    // The physical buffer may be of any size, so the ISR repeat until the 
    // buffer is empty. At this point it will switch to the other phys
    // buffer (if full). All this is done in the ISR to ensure there
    // are no audio drop-outs.

    // First find which channel is playing
    channel = -1;
    for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	if (sound_channel_array[i].state == CHANNEL_STATE_PLAYING) {
	    channel = i;
	}
    }

    // Check if a higher priority channel is suspended and resume it
    channel = suspend(channel);

    // Check if the current user buffer for this channel is  unfinished
    if ((sound_channel_array[channel].current_phys_buffer_is_0 && 
        (sound_channel_array[channel].phys_buffer_0_len > 0)) 
        || ((!sound_channel_array[channel].current_phys_buffer_is_0) && 
        (sound_channel_array[channel].phys_buffer_1_len > 0))) {

	if(sound_channel_array[channel].current_phys_buffer_is_0) {
	    sound_channel_array[channel].phys_transfer_0_start_address += 
		SA11X0_DMA_TRANSFER_SIZE;
	    play_buffer_0(channel);
	} else {
	    sound_channel_array[channel].phys_transfer_1_start_address += 
		SA11X0_DMA_TRANSFER_SIZE;
	    play_buffer_1(channel);
	}
	return CYG_ISR_HANDLED;

    } else {
    // Check if this is the end of a buffer, mark the channel for the dsr
    // and play the other buffer or see if there is another channel to resume 
	*SA11X0_DMA_PLAY_DCSR_CLEAR = 0x7F;
	sound_channel_array[channel].dsr_pending = true;
        if (sound_channel_array[channel].current_phys_buffer_is_0) {
            sound_channel_array[channel].current_phys_buffer_is_0 = false;
            sound_channel_array[channel].phys_buffer_0_free = true;
	    if(!sound_channel_array[channel].phys_buffer_1_free) {
                play_buffer_1(channel);
	    } else {
		// Mark this channel as idle and resume any other 
	        // suspended channel
	        channel = resume(channel);
	    }
        } else {
            sound_channel_array[channel].current_phys_buffer_is_0 = true;
	    sound_channel_array[channel].phys_buffer_1_free = true;
	    if(!sound_channel_array[channel].phys_buffer_0_free) {
                play_buffer_0(channel);
	    } else {
	        channel = resume(channel);
	    }
        }
        return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
    }
}

static void
ssp_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data )
{
    int i;
    // Find which channel to unblock. This will be the highest 
    // priority channel with the dsr pending member set, and may not 
    // correspond to the order in which the isr's occured.
    // However all the channels with dsr's pending will get unblocked,
    // once the dsr's catch up.
    for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	if (sound_channel_array[i].dsr_pending == true) {
	    sound_channel_array[i].dsr_pending = false;
            // Just signal sound_write to unblock
            cyg_flag_setbits( &sound_channel_array[i].block_flag, 
                DSR_UNBLOCK );
	    break;
	}
    }
}

////////////////////////////////////////////////////////////////////////////
// Open the sound device driver
////////////////////////////////////////////////////////////////////////////

static Cyg_ErrNo
sound_lookup(struct cyg_devtab_entry **tab, struct cyg_devtab_entry *st, const char *name) {
    
    return ENOERR;
}

////////////////////////////////////////////////////////////////////////////
//
//
// These are the device io routines
//
//
////////////////////////////////////////////////////////////////////////////

static Cyg_ErrNo
sound_read(cyg_io_handle_t handle, void *buffer, cyg_uint32 *len) {

    return ENOERR;
}

static Cyg_ErrNo
sound_write(cyg_io_handle_t handle, const void *buffer, cyg_uint32 *len) {

    // DMA must use physical addresses (found from HAL_VIRT_TO_PHYS_ADDRESS).
    // If virtual addresses were used by the user to fill the buffers, 
    // this would result in data being trapped in the cache (we could use 
    // physical addresses right now - but this won't work with memory 
    // protection).
    // When memory protection is available, it will be possible to map in
    // a suitably sized area of virtual memory as non-cacheable, but right
    // now this could only be done at the 1MB large page size.

    // A temporary solution has been used where we create buffers just by a 
    // malloc user side, and they are copied 1:1 into the physicallly mapped 
    // buffers inside here.
    // Although inefficient, this ensures the same code will work with 
    // and without memory protection.

    // These user & physical buffers are labelled 0 and 1 to avoid confusion 
    // with DMA buffers A and B (in fact what the Intel manual refer to as 
    // buffers are simply registers holding the transfer address and size.
    // The DMA hardware requires that we alternate between A and B, so this 
    // may happen continuously to exhaust a single physical buffer). 
    // User buffers may be of any size, smaller or 
    // larger than a DMA transfer, though it is preferable that they are a 
    // multiple of it. 

    cyg_handle_t user_handle;
    cyg_flag_value_t stat;
    int channel;
    cyg_bool playing;
    cyg_uint32 i,j;
    cyg_uint16* log_buffer = (cyg_uint16*)buffer;
    j = (*len)/2;

    // Who's calling?
    user_handle = cyg_thread_self();

    // Test without DMA
    /*for(i=0; i<j; i++) {
	while(!(*SA11X0_SSP_STATUS & 0x2)) {}
	*SA11X0_SSP_DATA = log_buffer[i];
	//printf("16 bit l or r %x\n", log_buffer[i]);
	}*/

    // Is this handle assigned a channel?
    channel = -1;
    for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	if (sound_channel_array[i].user_handle == user_handle) {
	    channel = i;
	}
    }

    // No channel assigned to this handle
    if(channel == -1) {
        return ENOERR;
    }

    if (sound_channel_array[channel].phys_buffer_0_free) {
    // START_TEMPORARY
        for(i=0; i<j; i++) {
	    sound_channel_array[channel].phys_buffer_0[i] = log_buffer[i];
	}
        sound_channel_array[channel].phys_transfer_0_start_address = 
            (cyg_uint32)sound_channel_array[channel].phys_buffer_0;
    // END_TEMPORARY
        sound_channel_array[channel].phys_buffer_0_len = *len; // DMA in bytes!
        sound_channel_array[channel].phys_buffer_0_free = false;
	// We should only call play_buffer if the sound driver hardware
	// is not in use and the dma inactive. Otherwise the subsequent 
	// isr's will ensure that all channels marked as supended or playing
	// are played out.
	// We must also ensure that if another thread prempts with a call to
	// sound_write part way through this one, that play_buffer 
	// cannot be called twice.
	// So first we check if any channel is playing, if false we call 
	// play_buffer, else we mark this channel as suspended.
	// Therefore the first channel into the cyg_scheduler_lock will 
	// seize the opportunity to progress to playing.
	// Flush or suspend commands are also locked out.
	// We also need to mask out isr's in case the last channel 
	// attempted to transit from playing to idle just after
	// the check for any playing channel.
	cyg_scheduler_lock();
        cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	playing = false;
	for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	    if (sound_channel_array[i].state == CHANNEL_STATE_PLAYING) {
	        playing = true;
	    }
	}
    
	if (playing == false) {
	    // If no channel is currently playing set this channel to play
            sound_channel_array[channel].state = CHANNEL_STATE_PLAYING;
	    set_1341(channel);
            play_buffer_0(channel);
	} else if (sound_channel_array[channel].state ==
            CHANNEL_STATE_PLAYING) {
	    // If this channel is currently playing, leave it alone
	} else {
	    // This channel is either transiting form idle to suspended,
	    // or is currently suspended
            sound_channel_array[channel].state = CHANNEL_STATE_SUSPENDED;
	}
        cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	cyg_scheduler_unlock();

    } else if (sound_channel_array[channel].phys_buffer_1_free) {
    // START_TEMPORARY
        for(i=0; i<j; i++) {
	    sound_channel_array[channel].phys_buffer_1[i] = log_buffer[i];
	}
        sound_channel_array[channel].phys_transfer_1_start_address = 
            (cyg_uint32)sound_channel_array[channel].phys_buffer_1;
    // END_TEMPORARY
        sound_channel_array[channel].phys_buffer_1_len = *len; // DMA in bytes!
        sound_channel_array[channel].phys_buffer_1_free = false;
	cyg_scheduler_lock();
        cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	playing = false;
	for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	    if (sound_channel_array[i].state == CHANNEL_STATE_PLAYING) {
	        playing = true;
	    }
	}
    
	if (playing == false) {
	    // If no channel is currently playing set this channel to play
            sound_channel_array[channel].state = CHANNEL_STATE_PLAYING;
            set_1341(channel);
            play_buffer_1(channel);
	} else if (sound_channel_array[channel].state ==
            CHANNEL_STATE_PLAYING) {
	    // If this channel is currently playing, leave it alone
	} else {
	    // This channel is either transiting form idle to suspended,
	    // or is currently suspended
            sound_channel_array[channel].state = CHANNEL_STATE_SUSPENDED;
	}
        cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	cyg_scheduler_unlock();
    }

    if (!sound_channel_array[channel].phys_buffer_0_free && 
        !sound_channel_array[channel].phys_buffer_1_free) {
        // BLOCK here if both buffers are full
        stat = cyg_flag_wait(&sound_channel_array[channel].block_flag, 
            BLOCK_SOUND_WRITE, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
    }

    return ENOERR;
}

////////////////////////////////////////////////////////////////////////////
// The sound device select() interface
////////////////////////////////////////////////////////////////////////////
static cyg_bool
sound_select(cyg_io_handle_t handle, cyg_uint32 which, cyg_addrword_t info) {
    return true;
}

////////////////////////////////////////////////////////////////////////////
// The sound device cyg_io_set_config() interface
////////////////////////////////////////////////////////////////////////////
static Cyg_ErrNo
sound_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buffer, cyg_uint32 *len) {

    cyg_handle_t user_handle;
    int i = 0;
    int j;
    cyg_bool open;

    // Who's calling?
    user_handle = cyg_thread_self();

    CYG_CHECK_DATA_PTRC( buffer );
    switch ( key ) {
	case CYG_IO_SET_CONFIG_PRIORITY :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
	    if (0 <= *(cyg_uint32*)buffer < SOUND_DRIVER_CHANNELS) {
                channel_init(*(cyg_uint32*)buffer);
                sound_channel_array[*(cyg_uint32*)buffer].user_handle = 
                    user_handle;
		// If this is the first channel opening power on
		cyg_scheduler_lock();
		cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		open = false;
		for (j=0; j<SOUND_DRIVER_CHANNELS; j++) {
		    if (sound_channel_array[j].channel_open == true) {
			open = true;
		    }
		}
		if (open == false) {
		    audio_power_on();
		}
		// Channel isn't really open until the handle has been set and
		// all channel info reset (we may need to clean up from the
		// previous user).
                sound_channel_array[*(cyg_uint32*)buffer].channel_open = true;
		cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		cyg_scheduler_unlock();
	    } else {
		return -EINVAL;
	    }
	    break;
	case CYG_IO_SET_CONFIG_PRECISION :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
                    sound_channel_array[i].chan_STATUS_0.input_fmt = 
                        *(cyg_uint32*)buffer & CYG_IO_SET_PRECISION_MASK;
		    // Must prevent another user or the isr from interfering
	            cyg_scheduler_lock();
                    cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    if(sound_channel_array[i].state == CHANNEL_STATE_PLAYING){
                        set_1341(i);
		    }
                    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	            cyg_scheduler_unlock();
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_SPEED :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
                    sound_channel_array[i].audio_rate = 
                        *(cyg_uint32*)buffer;
                    audio_set_dsp_speed(*(cyg_uint32*)buffer, i);
		    // Must prevent another user or the isr from interfering
	            cyg_scheduler_lock();
                    cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    if(sound_channel_array[i].state == CHANNEL_STATE_PLAYING){
         	        set_1341(i);
		    }
                    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	            cyg_scheduler_unlock();
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_CHANNELS :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
	    break;
	case CYG_IO_SET_CONFIG_ENCODING :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
	    break;
	case CYG_IO_SET_CONFIG_GAIN :
	    if ( *len != sizeof(cyg_uint32) )
		return -EINVAL;
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
		    sound_channel_array[i].chan_DATA0_0.volume = 
		        63 - (((*(cyg_uint32*)buffer & 0xff) + 1) * 63) / 100;
		    // Must prevent another user or the isr from interfering
	            cyg_scheduler_lock();
                    cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    if(sound_channel_array[i].state == CHANNEL_STATE_PLAYING){
		        set_1341(i);
		    }
                    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	            cyg_scheduler_unlock();
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_FLUSH :
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
		    // Forcing the buffers to zero length will cause
		    // play to end cleanly via the next isr
	            sound_channel_array[i].phys_buffer_0_len = 0;
	            sound_channel_array[i].phys_buffer_1_len = 0;
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_CLOSE :
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
		    // If this is the last channel closing power off
		    cyg_scheduler_lock();
		    cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    sound_channel_array[i].channel_open = false;
		    sound_channel_array[i].user_handle = (cyg_handle_t)NULL;
		    open = false;
		    for (j=0; j<SOUND_DRIVER_CHANNELS; j++) {
			if (sound_channel_array[j].channel_open == true) {
			    open = true;
			}
		    }
		    if (open == false) {
			audio_power_off();
		    }
		    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    cyg_scheduler_unlock();
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_PAUSE :
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
                    sound_channel_array[i].pause = true;
		}
	    }
	    break;
	case CYG_IO_SET_CONFIG_RESUME :
            for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
	        if (sound_channel_array[i].user_handle == user_handle) {
                    sound_channel_array[i].pause = false;
		    // Must prevent another user or the isr from interfering
	            cyg_scheduler_lock();
                    cyg_drv_interrupt_mask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
		    if(sound_channel_array[i].state == CHANNEL_STATE_PLAYING){
	                if (sound_channel_array[i].current_phys_buffer_is_0) {
			    play_buffer_0(i);
		        } else {
			    play_buffer_1(i);
		        }
		    }
                    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);
	            cyg_scheduler_unlock();
		}
	    }
	    break;
	default :
	    return -EDEVNOSUPP;
    }

    return ENOERR;
}

////////////////////////////////////////////////////////////////////////////
// The sound device cyg_io_get_config() interface
////////////////////////////////////////////////////////////////////////////

static Cyg_ErrNo
sound_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buffer, cyg_uint32 *len) {
    CYG_CHECK_DATA_PTRC( buffer );
    switch ( key ) {

	default :
	    return -EDEVNOSUPP;
    }
    return ENOERR;
}

////////////////////////////////////////////////////////////////////////////
// Initialization for the sound device driver
////////////////////////////////////////////////////////////////////////////

static bool
sound_init(struct cyg_devtab_entry *tab) {

    int i;
    static bool _initialized = false;

    if ( _initialized ) return true;

    cyg_drv_interrupt_create( CYGNUM_HAL_INTERRUPT_DMA_PLAY,
                          69,
                          0,

                          (cyg_ISR_t*)ssp_isr,
                          (cyg_DSR_t*)ssp_dsr,
                          &ssp_int_handle,
                          &ssp_interrupt );

    cyg_drv_interrupt_attach( ssp_int_handle );
    cyg_drv_interrupt_unmask( CYGNUM_HAL_INTERRUPT_DMA_PLAY);

    for (i=0; i<SOUND_DRIVER_CHANNELS; i++) {
        channel_init(i);
    }

    _initialized = true;
    return true;
}

////////////////////////////////////////////////////////////////////////////
// The required device table structures
////////////////////////////////////////////////////////////////////////////

CHAR_DEVIO_TABLE(ipaq_sound_handlers,
		sound_write,
		sound_read,
		sound_select,
		sound_get_config,
		sound_set_config);

CHAR_DEVTAB_ENTRY(ipaq_sound_device,
		CYGDAT_DEVS_SOUND_ARM_IPAQ_NAME,
		NULL,					// Base device name
		&ipaq_sound_handlers,
		sound_init,
		sound_lookup,
		NULL);					// Private data pointer

