/*
 * icmp_functions.c
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
#include "icmp.h"
#include "icmp_functions.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* Sends an ICMP packet back to the sender of an IP packet. */

int 
send_icmp_packet(IP_Header *old_ip_packet, int type, int code, const Interface *interface)
{
    const Route *route;
    ICMP_Header *icmp_packet;
    IP_Header   *ip_packet; 
    uint32_t     new_ip_source, new_ip_dest;
    uint16_t     ip_id; 
    uint8_t      ihl, *frame;
    ssize_t      ip_payload_len, ip_packet_len, icmp_payload_len, icmp_packet_len, frame_len; 
    int          data_bits;
    
    /* Construct ICMP Packet from old IP packet. */

    ihl              = (old_ip_packet->version_and_IHL & 0x0F) * 4;
    ip_payload_len   = ntohs(old_ip_packet->total_length) - ihl; 
    data_bits        = (ip_payload_len > ICMP_MAX_PAYLOAD_BYTES) ? ICMP_MAX_PAYLOAD_BYTES: ip_payload_len;
    icmp_payload_len = ihl + data_bits; 
    icmp_packet_len  = sizeof(ICMP_Header) + icmp_payload_len;
    icmp_packet      = construct_icmp_packet(old_ip_packet, icmp_payload_len, type, code);

    /* Construct IP Packet from old IP packet and ICMP packet. */

    new_ip_source    = interface->ip_address;
    new_ip_dest      = ntohl(old_ip_packet->source);
    ip_packet_len    = sizeof(IP_Header) + icmp_packet_len;
    ip_id            = ntohl(old_ip_packet->id);
    ip_packet        = construct_ip_packet(new_ip_source, new_ip_dest, ip_id, ICMP_PROTOCOL, 
                                           DEFAULT_TTL, icmp_packet, icmp_packet_len);

    /* If construct ICMP or IP packet fails, drop packet. */

    if (icmp_packet == NULL || ip_packet == NULL)
    {
        return PACKET_DROPPED; 
    }    

    /* Find route to send back to source. If route returns NULL, drop packet. */

    if ((route = find_route(new_ip_dest)) == NULL)
    {
        return PACKET_DROPPED;
    }

    /* Get IP and MAC addresses. If ARP lookup returns NULL, drop packet. */

    const uint32_t route_dest_ip_address  = find_route_ip_address(new_ip_dest, route, NULL);
    const uint8_t *route_dest_mac_address = find_arp_mac_address(route_dest_ip_address);

    if (route_dest_mac_address == NULL)
    {
        return PACKET_DROPPED;
    }

    /* Construct new Ethernet frame. If construction fails, drop packet. */

    frame = construct_ethernet_frame(interface->mac_address, route_dest_mac_address, 
                                     IP_TYPE, ip_packet, ip_packet_len);

    if (frame == NULL)
    {
        return PACKET_DROPPED;
    }

    /* Send and free the frame. */

    frame_len = sizeof(Ethernet_Header) + ip_packet_len + ETHERNET_FCS_LEN;
    send_ethernet_frame(interface->fds[1], frame, frame_len);
    free(frame);

    return PACKET_SENT;
}

/* Construct an ICMP packet. Returns a pointer to the malloced space 
   for the packet. Caller is responsible for freeing the packet. */

ICMP_Header *
construct_icmp_packet(void *payload, ssize_t payload_len, int type, int code)
{
    ICMP_Header *icmp_packet; 
    size_t       icmp_packet_len = sizeof(ICMP_Header) + payload_len;

    /* Malloc space for packet. */

    if ((icmp_packet = malloc(icmp_packet_len)) == NULL)
    {
        return NULL;
    }

    /* Set ICMP fields. */

    icmp_packet->type     = type; 
    icmp_packet->code     = code; 
    icmp_packet->checksum = 0;
    icmp_packet->unused   = 0;

    /* Copy payload after header and get checksum. */

    memcpy(icmp_packet+1, payload, payload_len);
    icmp_packet->checksum = RFC1071_checksum(icmp_packet, icmp_packet_len);

    return icmp_packet;
}

/* Convert IP address to string format. */

void
ip_to_str(uint32_t ip, char *buf)
{
    sprintf(buf, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
}

/*  
    Prints diagnostics if a packet is dropped. If it is an ICMP error, 
    will also send an ICMP packet. 

    Error values: 

        Packet Validity: 

            Not IPv4                    = 0 
            IHL not correct             = 1
            Wrong length                = 2
            Bad IP header checksum      = 3

        ICMP: 

            No Route                    = 4 (ICMP Network Unreachable)
            No ARP                      = 5 (ICMP Host unreachable)
            TTL exceeded                = 6 (ICMP TTL Exceeded)
*/

void 
dropped_packet_diagnostics(int error, IP_Header *ip_packet, const Interface *interface)
{
    char ip_source[IPV4_ADDRSTRLEN];
    char ip_destination[IPV4_ADDRSTRLEN];

    /* Convert IP addresses. */

    ip_to_str(ntohl(ip_packet->source), ip_source);
    ip_to_str(ntohl(ip_packet->destination), ip_destination);

    /* Packet Validity. */

    if (error == NOT_IPV4)
    {
        printf("dropping packet from %s (not IPv4) \n", ip_source);
    }
    if (error == BAD_IHL)
    {
        printf("dropping packet from %s (incorrect IHL) \n", ip_source);
    }
    if (error == BAD_IP_LENGTH)
    {
        printf("dropping packet from %s (wrong length) \n", ip_source);
    }
    if (error == BAD_IP_CHECKSUM)
    {
        printf("dropping packet from %s (bad IP header checksum) \n", ip_source);
    }

    /* ICMP Diagnostics. Sends an ICMP packet. */

    if (error == NO_ROUTE)
    {
        printf("dropping packet from %s to %s (no route) \n", ip_source, ip_destination);
        send_icmp_packet(ip_packet, ICMP_TYPE_DEST_UNREACHABLE, ICMP_CODE_NET_UNREACHABLE, interface);
    }
    if (error == NO_ARP)
    {
        printf("dropping packet from %s to %s (no ARP) \n", ip_source, ip_destination);
        send_icmp_packet(ip_packet, ICMP_TYPE_DEST_UNREACHABLE, ICMP_CODE_HOST_UNREACHABLE, interface);

    }
    if (error == TTL_EXCEEDED)
    {
        printf("dropping packet from %s to %s (TTL exceeded) \n", ip_source, ip_destination);
        send_icmp_packet(ip_packet, ICMP_TYPE_TTL_EXCEEDED, ICMP_CODE_TTL_EXCEEDED, interface);
    }

    /* Flush print statement. */
    
    fflush(stdout);
}
