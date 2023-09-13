/*
 * arp_functions.h
 */

#ifndef ARP_FUNCTIONS__H
#define ARP_FUNCTIONS__H

/* Implementation Headers */

#include "c_headers.h"
#include "router.h"
#include "arp.h"

/* 
    ARP FUNCTIONS
*/

void     handle_arp_packet(uint8_t *frame, ssize_t frame_len, const Interface *interface);
int      valid_arp_packet(ARP_Packet *arp_packet);
void     send_arp_reply(uint8_t *frame, ssize_t frame_len, ARP_Packet *arp_packet, const Interface *interface);
void     modify_arp_packet(ARP_Packet *arp_packet, const Interface *interface);
uint8_t *construct_arp_packet(uint32_t source_ip, uint32_t target_ip, uint16_t opcode,
                              uint8_t *mac_source, uint8_t *mac_target);

#endif /* ARP_FUNCTIONS__H */
