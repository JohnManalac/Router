/*
 * ip_functions.h
 */

#ifndef IP_FUNCTIONS__H
#define IP_FUNCTIONS__H

/* Implementation Headers */

#include "c_headers.h"
#include "ip.h"
#include "router.h"

/* 
    IP FUNCTIONS
*/

uint16_t   RFC1071_checksum(void *data, uint8_t data_len);
void       handle_ip_packet(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface);
int        valid_ip_packet(IP_Header *ip_packet, ssize_t packet_size, const Interface *interface);
int        send_locally(IP_Header *ip_packet, ssize_t ip_packet_len);
int        send_to_next_hop(uint8_t *ether_frame, ssize_t frame_len, IP_Header *ip_packet, 
                            const Route *route, const Interface *interface);
int        modify_ip_packet(IP_Header *ip_packet, int on_link);
IP_Header *construct_ip_packet(uint32_t ip_source, uint32_t ip_dest, uint16_t id, uint8_t protocol,
                               uint8_t ttl, void *payload, ssize_t payload_len);

#endif /* IP_FUNCTIONS__H */
