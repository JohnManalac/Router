/*
 * icmp_functions.h
 */

#ifndef ICMP_FUNCTIONS__H
#define ICMP_FUNCTIONS__H

/* Implementation Headers*/

#include "c_headers.h"
#include "router.h"
#include "icmp.h"
#include "ip.h"

/* 
    ICMP FUNCTIONS (including diagnostics)
*/

int          send_icmp_packet(IP_Header *old_ip_packet, int type, int code, const Interface *interface);
ICMP_Header *construct_icmp_packet(void *payload, ssize_t payload_len, int type, int code);
void         ip_to_str(uint32_t ip, char *buf);
void         dropped_packet_diagnostics(int error, IP_Header *ip_packet, const Interface *interface);

#endif /* ICMP_FUNCTIONS__H */
