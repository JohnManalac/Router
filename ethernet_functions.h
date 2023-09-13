/*
 * ethernet_functions.h
 */

#ifndef ETHERNET_FUNCTIONS__H
#define ETHERNET_FUNCTIONS__H

/* Implementation Headers */

#include "c_headers.h"
#include "router.h"
#include "ethernet.h"

/* 
    ETHERNET FUNCTIONS
*/

void     handle_ethernet_frame(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface);
int      valid_ethernet_fcs(uint8_t *ether_frame, ssize_t frame_len);
int      frame_matches_mac_address(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface);
int      get_ethernet_type(uint8_t *ether_frame);
void     modify_ethernet_frame(uint8_t *ether_frame, ssize_t frame_len, const uint8_t *source, const uint8_t *dest);
uint8_t *construct_ethernet_frame(const uint8_t *mac_source, const uint8_t *mac_dest, uint16_t type, 
                                  void *payload, ssize_t payload_len);
                                  
#endif /* ETHERNET_FUNCTIONS__H */
