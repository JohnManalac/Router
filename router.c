/*
 * router.c
 */

/* Implementation Headers */

#include "c_headers.h"
#include "router.h"
#include "ip.h"

/* 
    ROUTER CONSTANTS
*/

/* Control File */

const int            NUM_INTERFACES         = 4;
const char           FILE_EXTENSION[]       = ".vde";
const char           CONTROL_FILE_PATH[]    = "/tmp/net";
const int            CONTROL_FILE_LEN       = sizeof(CONTROL_FILE_PATH) + sizeof(FILE_EXTENSION); 
const int            MAX_VDE_FILE_LEN       = CONTROL_FILE_LEN + (NUM_INTERFACES / 10); 

/* TCP */
 
int                   LISTENING_PORTS[]     = { 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009 };
int                   NUM_LISTENING_PORTS   = sizeof(LISTENING_PORTS) / sizeof(int);  
TCP_Connections_List *TCP_CONNECTIONS_LIST  = NULL;
TCP_Connection       *CURRENT_CONNECTION    = NULL;

/* Router Interfaces */

int R0_0_fds[2], R0_1_fds[2], R0_2_fds[2], R0_3_fds[2];

const Interface ROUTER_INTERFACES[]         = {
   { 0, 0x50010001, {0x60, 0x6D, 0x67, 0xE2, 0xF9, 0x6E}, R0_0_fds },   /* Interface R0_0 */
   { 1, 0x5A020002, {0x60, 0x6D, 0x67, 0xCA, 0x7A, 0x04}, R0_1_fds },   /* Interface R0_1 */
   { 2, 0x64030003, {0x60, 0x6D, 0x67, 0xA7, 0x13, 0x23}, R0_2_fds },   /* Interface R0_2 */
   { 3, 0xD2000004, {0x60, 0x6D, 0x67, 0x52, 0x61, 0xEC}, R0_3_fds },   /* Interface R0_3 */
};

/* Routing Table */

const Route ROUTING_TABLE[]                 = { 
    { 0x50010000, NETMASK_16, DEFAULT_GATEWAY, &ROUTER_INTERFACES[0] }, /*    Network 0   */      
    { 0x5A020000, NETMASK_16, DEFAULT_GATEWAY, &ROUTER_INTERFACES[1] }, /*    Network 1   */
    { 0x64030000, NETMASK_16, DEFAULT_GATEWAY, &ROUTER_INTERFACES[2] }, /*    Network 2   */
    { 0xA0040000, NETMASK_16, 0x64030005,      &ROUTER_INTERFACES[2] }, /*    Network 3   */
    { 0xD2050000, NETMASK_16, DEFAULT_GATEWAY, &ROUTER_INTERFACES[3] }, /*    Network 4   */
    { 0xFA060000, NETMASK_16, 0xD2050007,      &ROUTER_INTERFACES[3] }, /*    Network 5   */
};  

const int ROUTING_TABLE_LEN                 = sizeof(ROUTING_TABLE) / sizeof(Route);

/* Router ARP */

const ARP_Entry ROUTER_ARP_CACHE[]          = { 

    { 0x50010005, {0x58, 0x9C, 0xFC, 0x00, 0xB2, 0x20} },               /*  Device tap0   */
    { 0x50010102, {0x74, 0x2F, 0x13, 0x8B, 0x72, 0x69} },               /*  Interface A   */
    { 0x50010203, {0x09, 0xBF, 0xAB, 0xCE, 0x14, 0x98} },               /*  Interface B   */
    { 0x5A020405, {0xB1, 0x07, 0x56, 0xE1, 0x2C, 0x9D} },               /*  Interface C   */          
    { 0x5A020607, {0x89, 0x98, 0xD0, 0x8D, 0x62, 0x1A} },               /*  Interface D   */
    { 0x64030809, {0x3F, 0xD1, 0x8C, 0x31, 0x98, 0xAE} },               /*  Interface E   */
    { 0x64030102, {0xF7, 0xBD, 0x8F, 0x07, 0x71, 0xED} },               /*  Interface F   */
    { 0x64030005, {0xD5, 0xA1, 0xBE, 0xB7, 0x84, 0x36} },               /* Interface R1_0 */
    { 0xD2050900, {0xC1, 0x42, 0x13, 0xEF, 0x0C, 0xF7} },               /*  Interface I   */
    { 0xD2050007, {0x9D, 0x0D, 0x24, 0x54, 0x87, 0x8C} },               /* Interface R2_0 */
};

const int ROUTER_ARP_CACHE_LEN              = sizeof(ROUTER_ARP_CACHE) / sizeof(ARP_Entry);
