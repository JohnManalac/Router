/*
 * router.h
 */

#ifndef ROUTER__H
#define ROUTER__H

/* Implementation Headers */

#include "c_headers.h"
#include "tcp.h"

/* 
    ROUTER STRUCTS 
*/

typedef struct ARP_Entry
{
    const uint32_t           ip_address;         /* Corresponding IP Address for entry.  */
    const uint8_t            mac_address[6];     /* Corresponding MAC address for entry. */
} ARP_Entry;

typedef struct Interface 
{
    const int                interface_num;      /* Interface number (starting at 0)     */
    const uint32_t           ip_address;         /* Interface IP address (host-endian)   */
    const uint8_t            mac_address[6];     /* Interface MAC address                */
    int                     *fds;                /* Interface fds for input and output.  */
} Interface; 

typedef struct Route
{
    const uint32_t           network_destination; /* Destination network (IP) address.   */
    const uint32_t           netmask;             /* IP netmask for network.             */
    const uint32_t           gateway;             /* Gateway IP address.                 */
    const Interface         *interface;           /* Interface to send next hop.         */
} Route;

/* 
    ROUTER CONSTANTS
*/

/* Control File */

extern const char            FILE_EXTENSION[];
extern const char            CONTROL_FILE_PATH[];
extern const int             CONTROL_FILE_LEN; 
extern const int             MAX_VDE_FILE_LEN; 

/* Router Interfaces */

extern const int             NUM_INTERFACES;   
extern const Interface       ROUTER_INTERFACES[];

/* Routing Table */

extern const Route           ROUTING_TABLE[];
extern const int             ROUTING_TABLE_LEN;

/* Router ARP */

extern const ARP_Entry       ROUTER_ARP_CACHE[];
extern const int             ROUTER_ARP_CACHE_LEN;

/* Router TCP */

extern int                   LISTENING_PORTS[];
extern int                   NUM_LISTENING_PORTS;  
extern int                   ACTIVE_SENDING_PORT;
extern TCP_Connections_List *TCP_CONNECTIONS_LIST;
extern TCP_Connection       *CURRENT_CONNECTION;

#endif /* ROUTER__H */
