//==========================================================================
//
//      tests/server_test.c
//
//      Simple TCP 'server' test
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
// Network server test code
#include <stdio.h>
#include <stdlib.h>
#include <network.h>

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

extern void
cyg_test_exit(void);

void
pexit(char *s)
{
    perror(s);
    cyg_test_exit();
}

static void
server_test(struct bootp *bp)
{
    int s, client, client_len;
    struct sockaddr_in client_addr, local;
    char buf[256];
    int one = 1;
    fd_set in_fds;
    int num, len;
    struct timeval tv;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEADDR");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEPORT");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(7734);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind error");
    }
    listen(s, SOMAXCONN);
    while (true) {
        client_len = sizeof(client_addr);
        if ((client = accept(s, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            pexit("accept");
        }
        client_len = sizeof(client_addr);
        getpeername(client, (struct sockaddr *)&client_addr, &client_len);
        diag_printf("connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

#ifdef CYGPKG_LIBC_STDIO        
        sprintf(buf, "Hello %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
#else        
        strcpy(buf, "Hello ");
        strcat(buf, inet_ntoa(client_addr.sin_addr));
        strcat(buf,":");
        strcat(buf, atoi(ntohs(client_addr.sin_port)));
        strcat(buf,"\n");
#endif
        
        write(client, buf, strlen(buf));
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(&in_fds);
        FD_SET(client, &in_fds);
        num = select(client+1, &in_fds, 0, 0, &tv);
        if (num > 0) {
            len = read(client, buf, sizeof(buf)-1);
            buf[len] = '\0';
            diag_printf("buf = '%s'\n", buf);
        } else {
            perror("select");
        }
        close(client);
    }
    close(s);
}

void
net_test(cyg_addrword_t param)
{
    diag_printf("Start SERVER test\n");
    init_all_network_interfaces();
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up) {
        server_test(&eth0_bootp_data);
    }
#endif
    cyg_test_exit();
}

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      net_test,          // entry
                      0,                 // entry parameter
                      "Network test",    // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}
