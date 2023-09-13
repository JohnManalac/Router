/*
 * router_functions.c
 */

/* Implementation Headers */

#include "c_headers.h"
#include "cs431vde.h"
#include "router.h"
#include "ip.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* Connects to all interfaces. */

void 
connect_to_interfaces()
{
    char             vde_file[MAX_VDE_FILE_LEN];
    const Interface *interface; 

    for (int i = 0; i < NUM_INTERFACES; i++)
    {
        interface = &ROUTER_INTERFACES[i];

        /* Get corresponding vde_file and vde_cmd. */

        snprintf(vde_file, MAX_VDE_FILE_LEN, "%s%d%s", CONTROL_FILE_PATH, i, FILE_EXTENSION);
        char *vde_cmd[] = { "vde_plug", vde_file, NULL };

        /* Connect to vde switch. */

        if (connect_to_vde_switch(interface->fds, vde_cmd) < 0) 
        {
            printf("Could not connect to switch, exiting. \n");
            exit(EXIT_FAILURE);
        }
    }
}

/* Find a route to an IP address. Returns a pointer to the route in
   the routing table if found. Else, returns NULL. */

const Route *
find_route(uint32_t ip_address)
{
    const Route *route;
    uint32_t     masked_address;

    for (int i = 0; i < ROUTING_TABLE_LEN; i++)
    {
        route          = &ROUTING_TABLE[i];
        masked_address = ip_address & route->netmask;

        if (masked_address == route->network_destination)
        {
            return route;
        }
    }

    /* No route found. */
    
    return NULL; 
}

/* Find the corresponding IP address for a route and if on_link is non-NULL, 
   sets its value to either ON_LINK or OFF_LINK, depending on the Gateway. */

const uint32_t *
find_route_ip_address(uint32_t *dest_ip, const Route *route, int *on_link)
{
    if (route->gateway == DEFAULT_GATEWAY)
    {
        if (on_link != NULL) *on_link = ON_LINK;
        return dest_ip;
    }
    else
    {
        if (on_link != NULL) *on_link = OFF_LINK;
        return &route->gateway;
    }
}

/* Find the corresponding MAC address for an IP address in the router's ARP cache. 
   Searches through the ARP cache and returns the corresponding MAC address. 
   If there is no ARP entry for the IP address, return NULL. */

const uint8_t *
find_arp_mac_address(uint32_t ip_address)
{
    const ARP_Entry *arp_entry;

    for (int i = 0; i < ROUTER_ARP_CACHE_LEN; i++)
    {
        arp_entry = &ROUTER_ARP_CACHE[i];
        
        if (ROUTER_ARP_CACHE[i].ip_address == ip_address)
        {
            return arp_entry->mac_address;
        }
    }

    /* No ARP found. */
    return NULL;
}
