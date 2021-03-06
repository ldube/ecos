//==========================================================================
//
//      lib/tftp_client.c
//
//      TFTP client support
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
// Date:         2000-04-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// TFTP client support

#include <network.h>
#include <arpa/tftp.h>
#include <tftp_support.h>

#define min(x,y) (x<y ? x : y)

//
// Read a file from a host into a local buffer.  Returns the
// number of bytes actually read, or (-1) if an error occurs.
// On error, *err will hold the reason.
//
int
tftp_get(char *filename,
         struct sockaddr_in *server,
         char *buf,
         int len,
         int mode,
         int *err)
{
    int res = 0;
    int s, actual_len, data_len, recv_len, from_len;
    static int get_port = 7700;
    struct sockaddr_in local_addr, server_addr, from_addr;
    char data[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *hdr = (struct tftphdr *)data;
    char *cp, *fp;
    struct timeval timeout;
    int last_good_block = 0;
    struct servent *server_info;
    fd_set fds;
    int total_timeouts = 0;

    *err = 0;  // Just in case

    // Create initial request
    hdr->th_opcode = htons(RRQ);  // Read file
    cp = (char *)&hdr->th_stuff;
    fp = filename;
    while (*fp) *cp++ = *fp++;
    *cp++ = '\0';
    if (mode == TFTP_NETASCII) {
        fp = "NETASCII";
    } else if (mode == TFTP_OCTET) {
        fp = "OCTET";
    } else {
        *err = TFTP_INVALID;
        return -1;
    }
    while (*fp) *cp++ = *fp++;
    *cp++ = '\0';
    server_info = getservbyname("tftp", "udp");
    if (server_info == (struct servent *)0) {
        *err = TFTP_NETERR;
        return -1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        // Couldn't open a communications channel
        *err = TFTP_NETERR;
        return -1;
    }
    memset((char *)&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(get_port++);
    if (bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        // Problem setting up my end
        *err = TFTP_NETERR;
        close(s);
        return -1;
    }
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = server->sin_addr;
    if (server->sin_port == 0) {
        server_addr.sin_port = server_info->s_port; // Network order already
    } else {
        server_addr.sin_port = server->sin_port;
    }

    // Send request
    if (sendto(s, data, sizeof(data), 0, 
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        // Problem sending request
        *err = TFTP_NETERR;
        close(s);
        return -1;
    }

    // Read data
    fp = buf;
    while (true) {
        timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(s, &fds);
        if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            if ((++total_timeouts > TFTP_TIMEOUT_MAX) || (last_good_block == 0)) {
                // Timeout - no data received
                *err = TFTP_TIMEOUT;
                close(s);
                return -1;
            }
            // Try resending last ACK
            hdr->th_opcode = htons(ACK);
            hdr->th_block = htons(last_good_block);
            if (sendto(s, data, 4 /* FIXME */, 0, 
                       (struct sockaddr *)&from_addr, from_len) < 0) {
                // Problem sending request
                *err = TFTP_NETERR;
                close(s);
                return -1;
            }
        } else {
            recv_len = sizeof(data);
            from_len = sizeof(from_addr);
            if ((data_len = recvfrom(s, &data, recv_len, 0, 
                                     (struct sockaddr *)&from_addr, &from_len)) < 0) {
                // What happened?
                *err = TFTP_NETERR;
                close(s);
                return -1;
            }
            if (ntohs(hdr->th_opcode) == DATA) {
                actual_len = 0;
                if (ntohs(hdr->th_block) == (last_good_block+1)) {
                    // Consume this data
                    cp = hdr->th_data;
                    data_len -= 4;  /* Sizeof TFTP header */
                    actual_len = data_len;
                    res += actual_len;
                    while (data_len-- > 0) {
                        if (len-- > 0) {
                            *fp++ = *cp++;
                        } else {
                            // Buffer overflow
                            *err = TFTP_TOOLARGE;
                            close(s);
                            return -1;
                        }
                    }
                    last_good_block++;
                }
                // Send out the ACK
                hdr->th_opcode = htons(ACK);
                hdr->th_block = htons(last_good_block);
                if (sendto(s, data, 4 /* FIXME */, 0, 
                           (struct sockaddr *)&from_addr, from_len) < 0) {
                    // Problem sending request
                    *err = TFTP_NETERR;
                    close(s);
                    return -1;
                }
                if ((actual_len >= 0) && (actual_len < SEGSIZE)) {
                    // End of data
                    close(s);
                    return res;
                }
            } else 
            if (ntohs(hdr->th_opcode) == ERROR) {
                *err = ntohs(hdr->th_code);
                close(s);
                return -1;
            } else {
                // What kind of packet is this?
                *err = TFTP_PROTOCOL;
                close(s);
                return -1;
            }
        }
    }
}

//
// Send data to a file on a server via TFTP.
//
int
tftp_put(char *filename,
         struct sockaddr_in *server,
         char *buf,
         int len,
         int mode,
         int *err)
{
    int res = 0;
    int s, actual_len, data_len, recv_len, from_len;
    static int put_port = 7800;
    struct sockaddr_in local_addr, server_addr, from_addr;
    char data[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *hdr = (struct tftphdr *)data;
    char *cp, *fp, *sfp;
    struct timeval timeout;
    int last_good_block = 0;
    struct servent *server_info;
    fd_set fds;
    int total_timeouts = 0;

    *err = 0;  // Just in case

    server_info = getservbyname("tftp", "udp");
    if (server_info == (struct servent *)0) {
        *err = TFTP_NETERR;
        return -1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        // Couldn't open a communications channel
        *err = TFTP_NETERR;
        return -1;
    }
    memset((char *)&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(put_port++);
    if (bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        // Problem setting up my end
        *err = TFTP_NETERR;
        close(s);
        return -1;
    }
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = server->sin_addr;
    if (server->sin_port == 0) {
        server_addr.sin_port = server_info->s_port; // Network order already
    } else {
        server_addr.sin_port = server->sin_port;
    }

    while (1) {
        // Create initial request
        hdr->th_opcode = htons(WRQ);  // Create/write file
        cp = (char *)&hdr->th_stuff;
        fp = filename;
        while (*fp) *cp++ = *fp++;
        *cp++ = '\0';
        if (mode == TFTP_NETASCII) {
            fp = "NETASCII";
        } else if (mode == TFTP_OCTET) {
            fp = "OCTET";
        } else {
            *err = TFTP_INVALID;
            return -1;
        }
        while (*fp) *cp++ = *fp++;
        *cp++ = '\0';
        // Send request
        if (sendto(s, data, sizeof(data), 0, 
                   (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            // Problem sending request
            *err = TFTP_NETERR;
            close(s);
            return -1;
        }
        // Wait for ACK
        timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(s, &fds);
        if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            if (++total_timeouts > TFTP_TIMEOUT_MAX) {
                // Timeout - no ACK received
                *err = TFTP_TIMEOUT;
                close(s);
                return -1;
            }
        } else {
            recv_len = sizeof(data);
            from_len = sizeof(from_addr);
            if ((data_len = recvfrom(s, &data, recv_len, 0, 
                                     (struct sockaddr *)&from_addr, &from_len)) < 0) {
                // What happened?
                *err = TFTP_NETERR;
                close(s);
                return -1;
            }
            if (ntohs(hdr->th_opcode) == ACK) {
                // Write request accepted - start sending data
                break;
            } else 
            if (ntohs(hdr->th_opcode) == ERROR) {
                *err = ntohs(hdr->th_code);
                close(s);
                return -1;
            } else {
                // What kind of packet is this?
                *err = TFTP_PROTOCOL;
                close(s);
                return -1;
            }
        }
    }

    // Send data
    sfp = buf;
    last_good_block = 1;
    while (res < len) {
        // Build packet of data to send
        data_len = min(SEGSIZE, len-res);
        hdr->th_opcode = htons(DATA);
        hdr->th_block = htons(last_good_block);
        cp = hdr->th_data;
        fp = sfp;
        actual_len = data_len + 4;
        // FIXME - what about "netascii" data?
        while (data_len-- > 0) *cp++ = *fp++;
        // Send data packet
        if (sendto(s, data, actual_len, 0, 
                   (struct sockaddr *)&from_addr, from_len) < 0) {
            // Problem sending request
            *err = TFTP_NETERR;
            close(s);
            return -1;
        }
        // Wait for ACK
        timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(s, &fds);
        if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            if (++total_timeouts > TFTP_TIMEOUT_MAX) {
                // Timeout - no data received
                *err = TFTP_TIMEOUT;
                close(s);
                return -1;
            }
        } else {
            recv_len = sizeof(data);
            from_len = sizeof(from_addr);
            if ((data_len = recvfrom(s, &data, recv_len, 0, 
                                     (struct sockaddr *)&from_addr, &from_len)) < 0) {
                // What happened?
                *err = TFTP_NETERR;
                close(s);
                return -1;
            }
            if (ntohs(hdr->th_opcode) == ACK) {
                if (ntohs(hdr->th_block) == last_good_block) {
                    // Advance pointers, etc
                    sfp = fp;
                    res += (actual_len - 4);
                    last_good_block++;
                } else {
                    diag_printf("Send block #%d, got ACK for #%d\n", 
                                last_good_block, ntohs(hdr->th_block));
                }
            } else 
            if (ntohs(hdr->th_opcode) == ERROR) {
                *err = ntohs(hdr->th_code);
                close(s);
                return -1;
            } else {
                // What kind of packet is this?
                *err = TFTP_PROTOCOL;
                close(s);
                return -1;
            }
        }
    }
    close(s);
    return res;
}

