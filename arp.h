/*
 * arp.h
 */

#ifndef ARP__H
#define ARP__H

/* Implementation Headers */

#include "c_headers.h"

/* 
    ARP STRUCTS 
*/

/* Arp Packets (add packed attribute to prevent auto-padding by compiler) */

typedef struct __attribute__((packed)) ARP_Packet
{
    uint16_t hardware_type;             /* Hardware Type (Usually Ethernet)      */
    uint16_t protocol_type;             /* Protocol Type (Usually IP)            */
    uint8_t  hardware_size;             /* Hardware Size (Usually 6 for MAC)     */
    uint8_t  protocol_size;             /* Protocol Size (Usually 4 for IP)      */
    uint16_t opcode;                    /* Op Code (Accept only 1, Request)      */
    uint8_t  sender_mac_address[6];     /* Sender MAC (Hardware) Address         */
    uint32_t sender_ip_address;         /* Sender IP Address                     */
    uint8_t  target_mac_address[6];     /* Destination MAC (Hardware) Address    */
    uint32_t target_ip_address;         /* Destination IP Address                */
} ARP_Packet;

/* 
    ARP CONSTANTS 
*/

/* Op Codes */

#define ARP_OP_REQUEST             1
#define ARP_OP_REPLY               2

/* Hardware Types and Sizes */

#define ARP_ETHERNET_HW_TYPE       1
#define ARP_ETHERNET_HW_SIZE       6

/* Protocol Type and Sizes */

#define ARP_IPV4_PROTOCOL_TYPE     0x800
#define ARP_IPV4_PROTOCOL_SIZE     4

#endif /* ARP__H */
