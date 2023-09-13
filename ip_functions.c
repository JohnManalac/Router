/*
 * ip_functions.c
 */

/* Implementation Headers */

#include "c_headers.h"
#include "cs431vde.h"
#include "router.h"
#include "router_functions.h"
#include "ethernet.h"
#include "ethernet_functions.h"
#include "ip.h"
#include "ip_functions.h"
#include "icmp_functions.h"
#include "tcp_functions.h"
#include "util.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* Compute Internet Checksum (from RFC 1071). */

uint16_t 
RFC1071_checksum(void *data, uint8_t data_len)
{
    uint16_t *d;
    uint32_t  sum;
    int       count;

    d     = (uint16_t *)data;
    sum   = 0;
    count = data_len; 

    while (count > 1)  
    {
        sum += *d;
        count -= 2;
        d++;
    }

    /*  Add left-over byte, if any */

    if (count > 0)
    {
        sum += (uint8_t) *d;
    }

    /*  Fold 32-bit sum to 16 bits */

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (uint16_t) ~sum & 0xFFFF;
}

/* Handle IP packet. */

void 
handle_ip_packet(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface)
{
    const Route *route;
    IP_Header   *ip_packet;
    ssize_t      ip_packet_len; 
    uint32_t     ip_destination;

    ip_packet_len  = frame_len - sizeof(Ethernet_Header); 
    ip_packet      = (IP_Header *)(ether_frame + sizeof(Ethernet_Header));
    ip_destination = ntohl(ip_packet->destination);

    /* If not TCP, subtract frame check sequence from packet length. */

    if (ip_packet->protocol != TCP_PROTOCOL)
    {
        ip_packet_len -= ETHERNET_FCS_LEN; 
    }
    
    /* Check validity and send pakcet locally or to next hop. Else, drop. */

    if (valid_ip_packet(ip_packet, ip_packet_len, interface))
    {
        if (!send_locally(ip_packet, ip_packet_len))
        {
            if ((route = find_route(ip_destination)) != NULL)
            {
                send_to_next_hop(ether_frame, frame_len, ip_packet, route, interface);
            }
            else
            {
                dropped_packet_diagnostics(NO_ROUTE, ip_packet, interface);
            }
        }
    }
}

/* Check the validity of an IP packet. Checks the version, IHL, datagram length, and
   header checksum. If all are valid, return 1, else print error diagnostic and return 
   PACKET_DROPPED (-1). */

int 
valid_ip_packet(IP_Header *ip_packet, ssize_t packet_len, const Interface *interface)
{
    uint8_t  version, ihl;
    uint16_t total_length, packet_checksum, ttl; 

    version         = ip_packet->version_and_IHL >> 4; 
    ihl             = (ip_packet->version_and_IHL & 0x0F) * 4;
    total_length    = ntohs(ip_packet->total_length);
    packet_checksum = ip_packet->checksum;
    ttl             = ip_packet->ttl;

    /* Check version is IPv4. */

    if (version != IPV4_VER)
    {
        dropped_packet_diagnostics(NOT_IPV4, ip_packet, interface);
        return PACKET_DROPPED;
    }

    /* Check Internet header length. */

    if (ihl < MIN_IHL)
    {
        dropped_packet_diagnostics(BAD_IHL, ip_packet, interface);
        return PACKET_DROPPED;
    }

    /* Check datagram length. */

    if (packet_len < total_length)
    {
        dropped_packet_diagnostics(BAD_IP_LENGTH, ip_packet, interface);
        return PACKET_DROPPED;
    }

    /* Check ttl. */

    if (ttl == 0)
    {
        dropped_packet_diagnostics(TTL_EXCEEDED, ip_packet, interface);
        return PACKET_DROPPED;
    }

    /* Verify internet checksum. */

    ip_packet->checksum = 0;
    ip_packet->checksum = RFC1071_checksum(ip_packet, ihl);

    if (ip_packet->checksum != packet_checksum)
    {
        dropped_packet_diagnostics(BAD_IP_CHECKSUM, ip_packet, NULL);
        return PACKET_DROPPED;
    }

    return 1;
}

/* Attempts to send an IP packet "locally," where local is if the destination IP 
   address is a router interface. If sent locally, prints a message and returns 1. 
   Else, return 0. */

int 
send_locally(IP_Header *ip_packet, ssize_t ip_packet_len)
{
    int      i;
    uint32_t ip_destination = ntohl(ip_packet->destination);

    for (i = 0; i < NUM_INTERFACES; i++)
    {
        if (ip_destination == ROUTER_INTERFACES[i].ip_address)
        {
            /* Handle TCP Packet/Segment. */

            if (ip_packet->protocol == TCP_PROTOCOL)
            {
                handle_tcp_segment(ip_packet, ntohs(ip_packet->total_length));
            }
            else
            {
                printf("    Delivering locally. \n");
            }

            return 1; 
        }
    }
    return 0; 
}

/* Sends an IP packet to the next hop destination. Modifies the packet and ensures 
   validity of all fields before sending, including the TTL, IP checksum, and fcs. */

int 
send_to_next_hop(uint8_t *ether_frame, ssize_t frame_len, IP_Header *ip_packet, const Route *route, const Interface *interface)
{
    int              on_link;
    const Interface *hop_router_interface;
    const uint8_t   *hop_ip_address, *hop_mac_address, *source_mac_address;
    uint32_t         ip_dest; 

    ip_dest              = ntohl(ip_packet->destination);
    hop_ip_address       = find_route_ip_address(&ip_dest, route, &on_link);
    hop_mac_address      = find_arp_mac_address(*hop_ip_address);
    hop_router_interface = route->interface;
    on_link              = OFF_LINK; 

    /* Check ARP. */

    if (hop_mac_address == NULL)
    {
        dropped_packet_diagnostics(NO_ARP, ip_packet, interface);
        return PACKET_DROPPED;   
    }

    source_mac_address   = hop_router_interface->mac_address;

    /* Modify IP packet with new destination. Check TTL.*/

    if (modify_ip_packet(ip_packet, on_link) == TTL_EXCEEDED)
    {
        dropped_packet_diagnostics(TTL_EXCEEDED, ip_packet, interface);
        return PACKET_DROPPED;   
    }

    /* Modify and send modified frame/packet to next hop. */

    modify_ethernet_frame(ether_frame, frame_len, source_mac_address, hop_mac_address);
    send_ethernet_frame(hop_router_interface->fds[1], ether_frame, frame_len);

    return PACKET_SENT;
}

/* Directly modify an IP packet, decrementing the TTL and recalculating the checksum. */

int 
modify_ip_packet(IP_Header *ip_packet, int on_link)
{
    uint8_t new_ttl, ihl; 

    new_ttl = ip_packet->ttl - 1;
    ihl     = (ip_packet->version_and_IHL & 0x0F) * 4;

    if ((on_link && new_ttl >= 1) || new_ttl > 1)
    {
        ip_packet->ttl      = new_ttl;
        ip_packet->checksum = 0;
        ip_packet->checksum = RFC1071_checksum(ip_packet, ihl);

        return TTL_OKAY;
    }

    /* TTL exceeded. */

    return TTL_EXCEEDED;
}

/* Construct an IP packet. Returns a pointer to the malloced space 
   for the packet. Caller is responsible for freeing this space. */

IP_Header *
construct_ip_packet(uint32_t ip_source, uint32_t ip_dest, uint16_t id, uint8_t protocol,
                    uint8_t ttl, void *payload, ssize_t payload_len)
{
    IP_Header *ip_packet;
    ssize_t    ip_packet_len;

    ip_packet_len = sizeof(IP_Header) + payload_len;

    /* Malloc space for packet. */

    if ((ip_packet = malloc(ip_packet_len)) == NULL)
    {
        return NULL;
    }

    /* Set IP fields. */

    ip_packet->version_and_IHL   = 0x45,
    ip_packet->service_type      = 0x00,
    ip_packet->total_length      = htons(ip_packet_len),
    ip_packet->id                = htons(id),
    ip_packet->flags_and_offset  = 0,
    ip_packet->ttl               = ttl,
    ip_packet->protocol          = protocol,
    ip_packet->checksum          = 0,
    ip_packet->source            = htonl(ip_source);
    ip_packet->destination       = htonl(ip_dest);

    /* Memcopy payload after header and get checksum. */
    
    memcpy(ip_packet+1, payload, payload_len);
    ip_packet->checksum = RFC1071_checksum(ip_packet, sizeof(IP_Header));

    return ip_packet; 
}
