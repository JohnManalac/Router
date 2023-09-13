/*
 * ip.h
 */

#ifndef IP__H
#define IP__H

/* Implementation Headers */

#include "c_headers.h"

/* 
    IP STRUCTS 
*/

typedef struct IP_Header
{
    uint8_t  version_and_IHL;           /* Version and internet header length.   */
    uint8_t  service_type;              /* Type of service.                      */
    uint16_t total_length;              /* Total length of datagram.             */
    uint16_t id;                        /* Identification.                       */
    uint16_t flags_and_offset;          /* Flag and fragment offset.             */
    uint8_t  ttl;                       /* Time to Live.                         */
    uint8_t  protocol;                  /* Protocol.                             */
    uint16_t checksum;                  /* Header checksum.                      */
    uint32_t source;                    /* Source Address (IP).                  */
    uint32_t destination;               /* Destination Address (IP).             */
} IP_Header;

/* 
    IP CONSTANTS 
*/

/* Common Netmasks */

#define NETMASK_8                0xFF000000                 
#define NETMASK_16               0xFFFF0000   
#define NETMASK_24               0xFFFFFF00   

/* IP Information */

#define DEFAULT_GATEWAY            0 
#define OFF_LINK                   0
#define ON_LINK                    1
#define IPV4_VER                   4
#define MIN_IHL                    5
#define ICMP_PROTOCOL              1
#define TCP_PROTOCOL               6
#define UDP_PROTOCOL               17
#define IPV4_ADDRSTRLEN            16
#define DEFAULT_TTL                64

/* IP Diagnostics */

#define PACKET_DROPPED            -1
#define PACKET_SENT                0   
#define NOT_IPV4                   0
#define BAD_IHL                    1
#define BAD_IP_LENGTH              2
#define BAD_IP_CHECKSUM            3
#define NO_ROUTE                   4
#define NO_ARP                     5
#define TTL_EXCEEDED               6
#define TTL_OKAY                   7 

#endif /* IP__H */
