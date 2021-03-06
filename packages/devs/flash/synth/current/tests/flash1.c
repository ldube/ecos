/* Hay, the copyright is usefull for something! */

static char copyright[] = "
//==========================================================================
//
//      flash1.c
//
//      Test flash operations for the synth target synth flash driver
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the \"License\"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an \"AS IS\"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           andrew.lunn@ascom.ch
// Contributors:        andrew.lunn
// Date:                2000-10-31
// Purpose:             Test a flash driver
// Description:         Try out a number of flash operations and make sure
//                      what is in the flash is what we expeect.
//                      
//####DESCRIPTIONEND####
//
//==========================================================================
";

#include <cyg/io/flash.h>
#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

#include <string.h>

#ifndef CYGINT_ISO_STRING_STRFUNCS
# define NA_MSG "Need string functions for test"
#endif

#ifdef NA_MSG
void cyg_user_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
}
#else

//==========================================================================
// main

void cyg_user_start(void)
{
    int ret;
    char data[1024];
    void *flash_start, *flash_end;
    int block_size, blocks;
    char *prog_start;
    unsigned char * ptr;

    CYG_TEST_INIT();
  
    ret=flash_init(NULL,0,(_printf *)diag_printf);
  
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_init");

    flash_dev_query(data);
    CYG_TEST_PASS_FAIL(!strncmp(data,"Linux Synthetic Flash",sizeof(data)),
                       "flash_query"); 

    ret = flash_get_limits(NULL,&flash_start,&flash_end);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_get_limits");

    ret = flash_get_block_info(&block_size, &blocks);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_get_block_info");

    /* Erase the whole flash. Not recommended on real hardware since this
     will probably erase the bootloader etc!!! */
    ret=flash_erase(flash_start,block_size * blocks,NULL);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_erase1");

    /* check that its actually been erased, and test the mmap area */
    for (ptr=flash_start,ret=0; ptr < (unsigned char *)flash_end; ptr++) {
        if (*ptr != 0xff) {
            ret++;
        }
    }
  
    CYG_TEST_PASS_FAIL((ret == 0),"flash empty check");

    ret = flash_program(flash_start,&copyright,sizeof(copyright),NULL);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_program1");
  
    /* Check the contents made it into the flash */
    CYG_TEST_PASS_FAIL(!strncmp(flash_start,copyright,sizeof(copyright)),
                       "flash program contents");

    /* .. and check nothing else changed */
    for (ptr=(unsigned char *)flash_start+sizeof(copyright),ret=0; 
         ptr < (unsigned char *)flash_end; ptr++) {
        if (*ptr != 0xff) {
            ret++;
        }
    }
  
    CYG_TEST_PASS_FAIL((ret == 0),"flash program overrun check");

    /* Program over a block boundary */
    prog_start = (unsigned char *)flash_start + block_size - sizeof(copyright)/2;
    ret = flash_program(prog_start,&copyright,sizeof(copyright),NULL);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_program2");
  
    /* Check the first version is still OK */
    CYG_TEST_PASS_FAIL(!strncmp(flash_start,copyright,sizeof(copyright)),
                       "Original contents");
  
    CYG_TEST_PASS_FAIL(!strncmp(prog_start,copyright,sizeof(copyright)),
                       "New program contents");

    /* Check the bit in between is still erased */
    for (ptr=(unsigned char *)flash_start+sizeof(copyright),ret=0; 
         ptr < (unsigned char *)prog_start; ptr++) {
        if (*ptr != 0xff) {
            ret++;
        }
    }
    CYG_TEST_PASS_FAIL((ret == 0),"flash erase check1");
  
    /* Erase the second block and make sure the first is not erased */
    ret=flash_erase((void *)((unsigned)flash_start+block_size),
                    block_size,NULL);
    CYG_TEST_PASS_FAIL((ret == FLASH_ERR_OK),"flash_erase2");

    /* Check the erase worked */
    for (ptr=(unsigned char *)flash_start+block_size,ret=0; 
         ptr < (unsigned char *)flash_start+block_size*2; ptr++) {
        if (*ptr != 0xff) {
            ret++;
        }
    }

    CYG_TEST_PASS_FAIL((ret == 0), "flash erase check2");
  
    /* Lastly check the first half of the copyright message is still there */
    CYG_TEST_PASS_FAIL(!strncmp(prog_start,copyright,sizeof(copyright)/2),
                       "Block 1 OK");

#if 0
    /* This test it fatal! Its not run by default!
     Check the flash is read only, by trying to write to it. We expect
     to get an exception */

    *(char *)flash_start = 'a';
#endif

    CYG_TEST_PASS_FINISH("flash1");
}

#endif /* ifndef NA_MSG */

/* EOF flash1.c */
