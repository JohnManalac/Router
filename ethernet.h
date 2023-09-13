/*
 * ethernet.h
 */

#ifndef ETHERNET__H
#define ETHERNET__H

/* Implementation Headers */

#include "c_headers.h"

/* 
    ETHERNET STRUCTS
*/

typedef struct Ethernet_Header
{
    uint8_t  destination[6];    /* Destination MAC address. */
    uint8_t  source[6];         /* Source MAC address.      */
    uint16_t type;              /* Ethernet Type.           */
} Ethernet_Header;

/* 
    ETHERNET CONSTANTS
*/

/* General */

#define BROADCAST_ADDR           "\xFF\xFF\xFF\xFF\xFF\xFF"
#define ETHERNET_FCS_LEN         4 
#define ETHERNET_MIN_DATA_LEN    46
#define ETHERNET_MAX_DATA_LEN    1500
#define ETHERNET_MIN_FRAME_LEN   sizeof(Ethernet_Header) + ETHERNET_MIN_DATA_LEN + ETHERNET_FCS_LEN
#define ETHERNET_MAX_FRAME_LEN   sizeof(Ethernet_Header) + ETHERNET_MAX_DATA_LEN + ETHERNET_FCS_LEN

/* Ether Types */

#define NON_VALID_TYPE           -1
#define IP_TYPE                  0x800
#define ARP_TYPE                 0x806

#endif /* ETHERNET__H */
