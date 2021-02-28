/* 
 * Written 1999-03-19 by Jonathan Larmour, Cygnus Solutions
 * Modifed for audio testing by Dominic Ostrowski 11-04-01
 * This file is in the public domain and may be used for any purpose
 */

/* CONFIGURATION CHECKS */

#include <pkgconf/system.h>     /* which packages are enabled/disabled */
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif
#ifdef CYGPKG_LIBC
# include <pkgconf/libc.h>
#endif
#ifdef CYGPKG_IO_SERIAL
# include <pkgconf/io_serial.h>
#endif

#ifndef CYGFUN_KERNEL_API_C
# error Kernel API must be enabled to build this example
#endif

#ifndef CYGPKG_LIBC_STDIO
# error C library standard I/O must be enabled to build this example
#endif

#ifndef CYGPKG_IO_SERIAL_HALDIAG
# error I/O HALDIAG pseudo-device driver must be enabled to build this example
#endif

/* INCLUDES */

#include <stdio.h>                      /* printf */
#include <stdlib.h>                      /* printf */
#include <string.h>                     /* strlen */
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#include <cyg/io/io.h>                  /* I/O functions */
#include <cyg/io/sound.h>               /* Sound definitions */
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <cyg/hal/hal_cache.h>          /* HAL_VIRT_TO_PHYS_ADDRESS */

#include "sound_sample.h"

/* DEFINES */

#define NTHREADS 2
#define STACKSIZE ( CYGNUM_HAL_STACK_SIZE_TYPICAL + 4096 )

/* STATICS */

static cyg_handle_t thread[NTHREADS];
static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

/* FUNCTIONS */

static void simple_prog(CYG_ADDRESS data)
{
  cyg_io_handle_t handle;

  // DMA must use physical addresses (found from HAL_VIRT_TO_PHYS_ADDRESS).
  // In here we must use virtual addresses, but this would result in data
  // being trapped in the cache (we could use physical addresses
  // right now - but this won't work with memory protection).
  // When memory protection is available, it will be possible to map in
  // a suitably sized area of virtual memory as non-cacheable, but right
  // now this could only be done at the 1MB large page size.

  // A temporary solution has been used where we create buffers just by a
  // malloc here, and they are copied into the real DMA buffers inside
  // the driver.
  // Although inefficient, this ensures the same code will work with and
  // without memory protection.

  // These buffers may be of any size (that is a multiple of a pair of 16
  // bit values), but a multiple of the DMA transfer
  // size is more efficient.

  // START_TEMPORARY
  // THIS BIT WILL CHANGE WITH MEMORY PROTECTION

  cyg_uint16* buffer_0 = (cyg_uint16*)malloc(SOUND_BUFFER_SIZE);
  cyg_uint16* buffer_1 = (cyg_uint16*)malloc(SOUND_BUFFER_SIZE);

  // END_TEMPORARY

  Cyg_ErrNo err = cyg_io_lookup("/dev/sound", &handle);
  if ( err != ENOERR ) {
    printf( "OPEN failed %d\n", err );
    return;
  }

  cyg_uint32 priority = 1;
  cyg_uint32 parameter = 0;
  cyg_uint32 len = sizeof(cyg_uint32);

  err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_PRIORITY,
                           (void*)&priority, &len );
  if ( err != ENOERR ) {
    printf( "Cannot set required mode: %d\n", err );
  }

  parameter = CYG_IO_SET_SPEED_44100;
  err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_SPEED,
                           (void*)&parameter, &len );

  cyg_uint32 sound_sample_len = (sizeof(sound_sample) / 2); // length in half words
  cyg_uint32 sound_buffer_len = (SOUND_BUFFER_SIZE / 2); // length in half words

  // Fill buffers 16 bit L + R interleaved little endian
  for (int r = 0; r < 10; r++) {
    cyg_uint32 k = 0;
    cyg_uint32 j = 0;

    // Fill buffer 0, start playing by calling cyg_io_write.
    // Fill buffer 1, queue this by calling cyg_io_write again.
    // This call to cyg_io_write will then block.
    // Once buffer 0 has been emptied,
    // this call to cyg_io_write will unblock,
    // allowing the buffer 0 to be refilled.

    while (j < sound_sample_len) {
      cyg_uint32 i = 0;
      while (i<sound_buffer_len && j<sound_sample_len) {
          // fill up the buffer
          buffer_0[i++] = sound_sample[j++];
      }

      // Start playing buffer_a
      cyg_uint32 buffer_0_len = i * 2; // transfer size in bytes
      err = cyg_io_write( handle, (void*)buffer_0, &buffer_0_len );

      parameter = 26;
      // Volume is specified as 0 (lowest) - 99 (highest)
      err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_GAIN,
                               (void*)&parameter, &len );
      if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
      }

      // Half way through the sound clip, time to test something!
      if ((j > (sound_sample_len / 2)) && k == 0) {
        k = 1;

    // Test speed
                /*parameter = CYG_IO_SET_SPEED_44100;
    err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_SPEED,
        (void*)&parameter, &len );
    if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
    }*/

    // Test flushing
          /*err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_FLUSH,
                    (void*)&parameter, &len );
          if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
          }
    break;*/

    // Test pause and resume
          /*err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_PAUSE,
                    (void*)&parameter, &len );
          if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
          }
    for (i=0; i<0x1000000; i++) {
                    // spin for a few seconds
    }
          err = cyg_io_set_config( handle, CYG_IO_SET_CONFIG_RESUME,
                    (void*)&parameter, &len );
          if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
          }*/

        // Test higher priority sound
        cyg_thread_resume(thread[1]);
      }

      i = 0;
      while (i<sound_buffer_len && j<sound_sample_len) {
        // fill up the buffer
        buffer_1[i++] = sound_sample[j++];
      }

      // Start playing buffer_b
      cyg_uint32 buffer_1_len = i * 2; // transfer size in bytes
      err = cyg_io_write( handle, (void*)buffer_1, &buffer_1_len );
    }

    printf( "Repeating sample \n");
  }
  cyg_test_exit();
}

static void second_sound(CYG_ADDRESS data)
{
  cyg_io_handle_t handle2;
  cyg_uint16* buffer_0 = (cyg_uint16*)malloc(SOUND_BUFFER_SIZE);
  cyg_uint16* buffer_1 = (cyg_uint16*)malloc(SOUND_BUFFER_SIZE);

  Cyg_ErrNo err = cyg_io_lookup("/dev/sound",&handle2);
  if ( err != ENOERR ) {
    printf( "OPEN failed %d\n", err );
    return;
  }

  cyg_uint32 priority = 0;
  cyg_uint32 parameter = 0;
  cyg_uint32 len = sizeof(cyg_uint32);

  cyg_uint32 sound_sample_len = (sizeof(sound_sample_tone) / 2); // length in half words
  cyg_uint32 sound_buffer_len = (SOUND_BUFFER_SIZE / 2); // length in half words

  // Fill buffers 16 bit L + R interleaved little endian

  for(;;) {
    cyg_uint32 l = 0;

    err = cyg_io_set_config( handle2, CYG_IO_SET_CONFIG_PRIORITY,
        (void*)&priority, &len );
    if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
    }

    for (cyg_uint32 k=0; k<100; k++) {
      cyg_uint32 j = 0;

      while (j<sound_sample_len) {

        cyg_uint32 i;
        for (i = 0; i < sound_buffer_len && j<sound_sample_len;) {
           // fill up the buffer
          buffer_0[i++] = sound_sample_tone[j++];
        }

        cyg_uint32 buffer_0_len = i * 2; // transfer size in bytes
        err = cyg_io_write( handle2, (void*)buffer_0, &buffer_0_len );

        // Half way through the sound clip, time to test something!
        if ((j > (sound_sample_len / 2)) && l == 0) {
          l = 1;
          // Test speed
          parameter = CYG_IO_SET_SPEED_8000;
          err = cyg_io_set_config( handle2, CYG_IO_SET_CONFIG_SPEED,
                                   (void*)&parameter, &len );

          if ( err != ENOERR ) {
            printf( "Cannot set required mode: %d\n", err );
          }

       // Test pause and resume
       /*err = cyg_io_set_config(handle2, CYG_IO_SET_CONFIG_PAUSE,
     (void*)&parameter, &len );
       if ( err != ENOERR ) {
     printf( "Cannot set required mode: %d\n", err );
       }
       for (i=0; i<0x1000000; i++) {
     // spin for a few seconds
       }
       err = cyg_io_set_config( handle2, CYG_IO_SET_CONFIG_RESUME,
     (void*)&parameter, &len );
       if ( err != ENOERR ) {
     printf( "Cannot set required mode: %d\n", err );
       }*/
        }

        for (i = 0; i<sound_buffer_len && j<sound_sample_len; ) {
          // fill up the buffer
          buffer_1[i++] = sound_sample_tone[j++];
        }

        cyg_uint32 buffer_1_len = i * 2; // transfer size in bytes
        err = cyg_io_write( handle2, (void*)buffer_1, &buffer_1_len );
      }
    }
    err = cyg_io_set_config( handle2, CYG_IO_SET_CONFIG_CLOSE,
                             (void*)&parameter, &len );
    if ( err != ENOERR ) {
        printf( "Cannot set required mode: %d\n", err );
    }

    cyg_thread_suspend(thread[1]);
  }
}

void cyg_user_start(void)
{
  if (1) cyg_thread_create(4, simple_prog, (cyg_addrword_t) 0, "SoundTest",
                    (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
  if (0) cyg_thread_create(4, second_sound, (cyg_addrword_t) 0, "Sound2Test",
                    (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
  cyg_thread_resume(thread[0]);
}
