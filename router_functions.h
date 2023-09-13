/*
 * router_functions.h
 */

#ifndef ROUTER_FUNCTIONS__H
#define ROUTER_FUNCTIONS__H

/* Implementation Headers */

#include "c_headers.h"
#include "router.h"

/* 
    ROUTER FUNCTIONS
*/

void            connect_to_interfaces();
const Route    *find_route(uint32_t ip_address);
const uint32_t *find_route_ip_address(uint32_t *dest_ip, const Route *route, int *on_link);
const uint8_t  *find_arp_mac_address(uint32_t ip_address);

#endif /* ROUTER_FUNCTIONS__H */
