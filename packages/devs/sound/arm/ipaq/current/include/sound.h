//==========================================================================
//
//      sound.h
//
//      User input driver definitions.
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
// Date:         2001-04-11
// Purpose:      Sound device definitions
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_IO_SOUND_H
#define CYGONCE_IO_SOUND_H

// A "feature" of the SA1110 DMA hardware is an odd maximum transfer size
// of 8191 bytes, the maximum "useful" size in terms of 16 bit L & R pairs
// is 8188 bytes
#define SA11X0_DMA_TRANSFER_SIZE         0x1FFC // 8188 Bytes 

// Should be a multiple of SA11X0_DMA_TRANSFER_SIZE
// 16K corresponds to ~0.1s at 44.1KHz 16 Bit Stereo
#define SOUND_BUFFER_SIZE     SA11X0_DMA_TRANSFER_SIZE * 2 // 8 bytes under 16K

// The allowed cyg_io_set_config() keys
#define CYG_IO_SET_CONFIG_PRIORITY	 0x900
#define CYG_IO_SET_CONFIG_PRECISION	 0x901
#define CYG_IO_SET_CONFIG_SPEED	         0x902
#define CYG_IO_SET_CONFIG_CHANNELS	 0x903
#define CYG_IO_SET_CONFIG_ENCODING	 0x904
#define CYG_IO_SET_CONFIG_GAIN	         0x905
#define CYG_IO_SET_CONFIG_FLUSH  	 0x906
#define CYG_IO_SET_CONFIG_CLOSE  	 0x907
#define CYG_IO_SET_CONFIG_PAUSE  	 0x908
#define CYG_IO_SET_CONFIG_RESUME  	 0x909

#define CYG_IO_PRIORITY_0                0
#define CYG_IO_PRIORITY_1                1
#define CYG_IO_PRIORITY_2                2

#define SOUND_DRIVER_CHANNELS            3

#define CYG_IO_SET_PRECISION_I2S	 0x0
#define CYG_IO_SET_PRECISION_LSB_16	 0x1
#define CYG_IO_SET_PRECISION_LSB_18	 0x2
#define CYG_IO_SET_PRECISION_LSB_20	 0x3
#define CYG_IO_SET_PRECISION_MSB	 0x4
#define CYG_IO_SET_PRECISION_LSB_16_MSB	 0x5
#define CYG_IO_SET_PRECISION_LSB_18_MSB	 0x6
#define CYG_IO_SET_PRECISION_LSB_20_MSB	 0x7
#define CYG_IO_SET_PRECISION_MASK	 0x7

#define CYG_IO_SET_SPEED_8000	         0x0
#define CYG_IO_SET_SPEED_11025	         0x1
#define CYG_IO_SET_SPEED_16000	         0x2
#define CYG_IO_SET_SPEED_22050	         0x3
#define CYG_IO_SET_SPEED_32000	         0x4
#define CYG_IO_SET_SPEED_44100	         0x5
#define CYG_IO_SET_SPEED_48000	         0x6

#endif

