/*
 * icmp.h
 */

#ifndef ICMP__H
#define ICMP__H

/* Implementation Headers */

#include "c_headers.h"

/* 
    ICMP STRUCTS 
*/

typedef struct ICMP_Header
{
    uint8_t  type;                      /* ICMP Type                            */
    uint8_t  code;                      /* ICMP Code                            */
    uint16_t checksum;                  /* Internet Checksum                    */
    uint32_t unused;                    /* Unused Data (4 bytes)                */
} ICMP_Header; 

/* 
    ICMP CONSTANTS 
*/

/* ICMP General */
#define ICMP_MAX_PAYLOAD_BYTES     8

/* ICMP Types */

#define ICMP_TYPE_DEST_UNREACHABLE 3
#define ICMP_TYPE_TTL_EXCEEDED     11

/* ICMP Codes */

#define ICMP_CODE_TTL_EXCEEDED     0
#define ICMP_CODE_NET_UNREACHABLE  0
#define ICMP_CODE_HOST_UNREACHABLE 1

#endif /* ICMP__H */
