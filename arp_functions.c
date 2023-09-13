/*
 * arp_functions.c
 */

/* Implementation Headers */

#include "c_headers.h"
#include "cs431vde.h"
#include "router.h"
#include "ethernet.h"
#include "ethernet_functions.h"
#include "arp.h"
#include "arp_functions.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* Handle ARP packet (only requests). */

void 
handle_arp_packet(uint8_t *frame, ssize_t frame_len, const Interface *interface)
{
    ARP_Packet *arp_packet = (ARP_Packet *)(frame + sizeof(Ethernet_Header));

    if (valid_arp_packet(arp_packet))
    {
        if (ntohs(arp_packet->opcode) == ARP_OP_REQUEST)
        {
            send_arp_reply(frame, frame_len, arp_packet, interface);
        }
    }
}

/* Checks for valid ARP packet. Must be of ethernet hardware type 
   and size, as well as IP protocol type and size. */

int 
valid_arp_packet(ARP_Packet *arp_packet)
{
    if (ntohs(arp_packet->hardware_type) != ARP_ETHERNET_HW_TYPE)
    {
        printf("ARP packet is not ethernet HW type.\n");
        return 0;
    }
    if (ntohs(arp_packet->protocol_type) != ARP_IPV4_PROTOCOL_TYPE)
    {
        printf("ARP packet is not protocol type.\n");
        return 0;
    }
    if (arp_packet->hardware_size != ARP_ETHERNET_HW_SIZE)
    {
        printf("ARP packet is not hardware size.\n");
        return 0;
    }
    if (arp_packet->protocol_size != ARP_IPV4_PROTOCOL_SIZE)  
    {
        printf("ARP packet is not protocol size.\n");
        return 0;
    } 

    return 1;
}

/* Send an ARP reply to the original source. */

void
send_arp_reply(uint8_t *frame, ssize_t frame_len, ARP_Packet *arp_packet, const Interface *interface)
{
    /* Check if destined for this interface. Else, ignore. */

    if (ntohl(arp_packet->target_ip_address) == interface->ip_address)
    {
        modify_arp_packet(arp_packet, interface);
        modify_ethernet_frame(frame, frame_len, interface->mac_address, arp_packet->target_mac_address);        
        send_ethernet_frame(interface->fds[1], frame, frame_len);
    }
}

/* Modify an ARP request packet into an ARP response. */

void
modify_arp_packet(ARP_Packet *arp_packet, const Interface *interface)
{
    /* Set opcode to reply.*/

    arp_packet->opcode             = htons(ARP_OP_REPLY);

    /* Set target IP and MAC addresses to source. */

    arp_packet->target_ip_address  = arp_packet->sender_ip_address; 
    memcpy(arp_packet->target_mac_address, arp_packet->sender_mac_address, 6);

    /* Set source IP and MAC addresses to interface. */

    arp_packet->sender_ip_address  = htonl(interface->ip_address);
    memcpy(arp_packet->sender_mac_address, interface->mac_address, 6);
}

/* Construct an ARP packet. Returns a pointer to the malloced space for 
   for the packet. Caller is responsible for freeing the packet. */

uint8_t *
construct_arp_packet(uint32_t source_ip, uint32_t target_ip, uint16_t opcode, uint8_t *mac_source, uint8_t *mac_target)
{
    ARP_Packet *arp_packet;
    uint8_t    *frame; 

    if ((arp_packet = malloc(sizeof(ARP_Packet))) == NULL)
    {
        return NULL;
    }

    /* Set fields. */
    
    arp_packet->hardware_type      = htons(ARP_ETHERNET_HW_TYPE),
    arp_packet->protocol_type      = htons(ARP_IPV4_PROTOCOL_TYPE),
    arp_packet->hardware_size      = ARP_ETHERNET_HW_SIZE,
    arp_packet->protocol_size      = ARP_IPV4_PROTOCOL_SIZE,
    arp_packet->opcode             = htons(opcode); 
    memcpy(arp_packet->sender_mac_address, mac_source, 6);
    memcpy(arp_packet->target_mac_address, mac_target, 6);
    arp_packet->sender_ip_address  = source_ip;
    arp_packet->target_ip_address  = target_ip;

    /* Construct ethernet frame and return. */

    frame = construct_ethernet_frame(mac_source, mac_target, ARP_TYPE, arp_packet, sizeof(ARP_Packet)); 
    
    return frame; 
}
