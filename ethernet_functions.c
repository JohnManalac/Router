/*
 * ethernet_functions.c
 */

/* Implementation Headers */

#include "c_headers.h"
#include "util.h"
#include "frame_crc32.h"
#include "cs431vde.h"
#include "router.h"
#include "ethernet.h"
#include "ethernet_functions.h"
#include "ip.h"
#include "ip_functions.h"
#include "arp_functions.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* Handle Ethernet frame. */

void 
handle_ethernet_frame(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface) 
{
    IP_Header *ip_packet; 
    int        ether_type, arp_or_tcp;
    
    ether_type = NON_VALID_TYPE;
    arp_or_tcp = -1;

    /* Check minimum frame length without frame check sequence. */

    if (frame_len < ETHERNET_MIN_FRAME_LEN - ETHERNET_FCS_LEN)
    {
        printf("ignoring %ld-byte frame (short) \n", frame_len);
        return; 
    }

    /* Check ether type. If IP protocol, check if TCP.  */

    ether_type = get_ethernet_type(ether_frame);

    if (ether_type == IP_TYPE)
    {
        ip_packet = (IP_Header *)ether_frame + sizeof(Ethernet_Header);

        if (ip_packet->protocol == TCP_PROTOCOL)
        {
            arp_or_tcp = 1; 
        } 
    }

    if (ether_type == ARP_TYPE)
    {
        arp_or_tcp = 1; 
    }

    if (ether_type == NON_VALID_TYPE)
    {
        printf("ignoring %ld-byte frame (unrecognized type) \n", frame_len);
    }

    /* If not ARP or TCP, reverify length, as well as the frame check sequence. */

    if (!arp_or_tcp)
    {
        if (frame_len < ETHERNET_MIN_FRAME_LEN - ETHERNET_FCS_LEN)
        {
            printf("ignoring %ld-byte frame (short) \n", frame_len);
            return; 
        }

        if (!valid_ethernet_fcs(ether_frame, frame_len))
        {
            return; 
        }
    }
        
    /* Check destination and type, handling packets corresponding to their types. */

    if (frame_matches_mac_address(ether_frame, frame_len, interface))
    {        
        if (ether_type == IP_TYPE)
        {
            handle_ip_packet(ether_frame, frame_len, interface);
        }
        
        if (ether_type == ARP_TYPE)
        {
            handle_arp_packet(ether_frame, frame_len, interface);
        }
    }
}

/* Verifies the frame check sequence for an Ethernet frame. Prints a diagnostic message if incorrect. */

int 
valid_ethernet_fcs(uint8_t *ether_frame, ssize_t frame_len)
{
    uint32_t frame_check;
    uint32_t crc_check;

    frame_check = *(uint32_t *)(ether_frame + (frame_len - ETHERNET_FCS_LEN));
    crc_check   = crc32(0, ether_frame, frame_len - ETHERNET_FCS_LEN);

    if (frame_check != crc_check)
    {
        printf("ignoring %ld-byte frame (bad fcs: got 0x%x, expected 0x%x \n", frame_len, frame_check, crc_check);
        return 0;
    }

    return 1; 
}

/* Checks if an Ethernet frame is destined for the MAC address of router interface. If it is a broadcast
   frame, check if the type is ARP and handles it. If it is a broadcast, non-ARP frame or not 
   destined for the interface, then a diagnostic message will be printed and 0 returned. */

int 
frame_matches_mac_address(uint8_t *ether_frame, ssize_t frame_len, const Interface *interface)
{
    Ethernet_Header *ethernet_hdr;
    char            *source_addr;

    ethernet_hdr = (Ethernet_Header *)ether_frame;
    source_addr     = binary_to_hex(ethernet_hdr->source, 6);

    /* Destination matches router's MAC address. */

    if (memcmp(ethernet_hdr->destination, interface->mac_address, 6) == 0)
    {
        return 1; 
    }

    /* Broadcast frame or frame not destined for router. */

    if (memcmp(ethernet_hdr->destination, BROADCAST_ADDR, 6) == 0)
    {
        /* If ARP broadcast, handle the packet. */

        if (ntohs(ethernet_hdr->type) == ARP_TYPE)
        {
            handle_arp_packet(ether_frame, frame_len, interface);
        }
        else
        {
            printf("received %ld-byte broadcast frame from %s", frame_len, source_addr);
        }
    }
    else
    {
        printf("ignoring %ld-byte frame (not for me) \n", frame_len);
    }
    
    free(source_addr);
    return 0;
}

/* Return the type of an Ethernet frame. If non-valid (neither IP or ARP), return NON_VALID_TYPE. */

int 
get_ethernet_type(uint8_t *ether_frame)
{
    Ethernet_Header *ethernet_hdr;
    uint16_t         ether_type;

    ethernet_hdr = (Ethernet_Header *)ether_frame;
    ether_type   = ntohs(ethernet_hdr->type);

    if (ether_type == IP_TYPE)
    {
        return IP_TYPE; 
    }
    
    if (ether_type == ARP_TYPE)
    {
        return ARP_TYPE;
    }

    return NON_VALID_TYPE;
}

/* Directly modifies an Ethernet frame, changing the source and destination 
   MAC addresses and recalculating and replacing the frame check sequence. */

void 
modify_ethernet_frame(uint8_t *ether_frame, ssize_t frame_len, const uint8_t *source, const uint8_t *dest)
{
    Ethernet_Header *ethernet_hdr; 
    uint32_t        *frame_check, new_frame_check;

    ethernet_hdr     = (Ethernet_Header *)ether_frame;
    frame_check      = (uint32_t *)(ether_frame + (frame_len - ETHERNET_FCS_LEN));

    /* Change source and destination MAC addresses. */

    memcpy(ethernet_hdr->source, source, 6);
    memcpy(ethernet_hdr->destination, dest, 6);

    /* Recalculate and replace frame check sequence. */

    new_frame_check  = crc32(0, ether_frame, frame_len - ETHERNET_FCS_LEN);
    *frame_check     = new_frame_check;
}

/* Construct an Ethernet frame. Returns a pointer to the malloced space 
   for the frame. Caller is responsible for freeing this space. If the payload 
   length is too large, NULL will be returned. If too small, then the 
   data will be padded with zeros. The payload is then freed. */

uint8_t * 
construct_ethernet_frame(const uint8_t *mac_source, const uint8_t *mac_dest, 
                         uint16_t type, void *payload, ssize_t payload_len)
{
    Ethernet_Header *ethernet_hdr;
    uint32_t         fcs; 
    uint8_t         *frame;
    ssize_t          frame_len;           
    int              pad_data; 

    frame_len = sizeof(Ethernet_Header) + payload_len + ETHERNET_FCS_LEN;           
    pad_data  = 0; 

    /* Check size. */

    if (payload_len > ETHERNET_MAX_DATA_LEN)
    {
        return NULL;
    }

    if (payload_len < ETHERNET_MIN_DATA_LEN)
    {
        frame_len = ETHERNET_MIN_FRAME_LEN; 
        pad_data = 1; 
    }

    /* Malloc space for frame. */

    if ((frame = malloc(frame_len)) == NULL)
    {
        return NULL;
    }

    /* Set Ethernet fields. */

    ethernet_hdr       = (Ethernet_Header *)frame;
    ethernet_hdr->type = htons(type); 

    memcpy(ethernet_hdr->source, mac_source, 6);
    memcpy(ethernet_hdr->destination, mac_dest, 6); 

    /* Insert payload and pad data if needed. */

    memcpy(frame + sizeof(Ethernet_Header), payload, payload_len);   

    if (pad_data)
    {
        memset(frame + sizeof(Ethernet_Header) + payload_len, '\0', ETHERNET_MIN_DATA_LEN - payload_len);
    }

    /* Calculate and insert fcs. */

    fcs = crc32(0, ethernet_hdr, frame_len - ETHERNET_FCS_LEN);
    memcpy(frame + frame_len - ETHERNET_FCS_LEN, &fcs, ETHERNET_FCS_LEN);

    /* Free payload and return frame. */

    free(payload);

    return frame;
}
