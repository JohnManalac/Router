/*
 * tcp.h
 */

#ifndef TCP__H
#define TCP__H

/* Implementation Headers */

#include "c_headers.h"
#include "ethernet.h"
#include "ip.h"

/* 
    TCP DATA STRUCTURES
*/

typedef struct TCP_Header
{
    uint16_t src_port;                  /* Source port.                            */
    uint16_t dst_port;                  /* Destination port.                       */
    uint32_t seq_number;                /* Sequence number.                        */
    uint32_t ack_number;                /* Acknowledgement number.                 */
    uint16_t offset_reserved_control;   /* Data Offset (4 bits), Reserved (6 bits) 
                                           and Control Bits (6 bits).              */
    uint16_t window_size;               /* Window size.                            */
    uint16_t checksum;                  /* Header checksum computated over the     
                                           header, text, and pseudo-header.        */
    uint16_t urgent_pointer;            /* Urgent Pointer.                         */
} TCP_Header;

typedef struct TCP_Pseudoheader
{
    uint32_t src_address;               /* Source IP address.                      */
    uint32_t dst_address;               /* Destination IP address.                 */
    uint8_t  zero;                      /* Zero.                                   */
    uint8_t  protocol;                  /* IP Protocol.                            */
    uint16_t tcp_length;                /* TCP segment length (header and data).   */
} TCP_Pseudoheader;


typedef enum TCP_State
{
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} TCP_State;

typedef struct TCP_Flags
{
    int FIN_FLAG_SET;
    int SYN_FLAG_SET;
    int RST_FLAG_SET;
    int PSH_FLAG_SET;
    int ACK_FLAG_SET;
    int URG_FLAG_SET;
} TCP_Flags; 

typedef struct TCP_Connection
{
    uint32_t  src_ip;
    uint32_t  dst_ip;
    uint16_t  src_port;
    uint16_t  dst_port;
    uint16_t  window_size; 
    uint32_t  seq_number;
    uint32_t  ack_number;
    TCP_State state; 
} TCP_Connection;

typedef struct TCP_Node
{
    struct TCP_Node *next;  
    TCP_Connection  *connection; 
} TCP_Node; 

typedef struct TCP_Connections_List
{
    size_t    size;
    TCP_Node *head; 
    TCP_Node *tail; 
} TCP_Connections_List;

/* 
    TCP CONSTANTS 
*/

/* TCP Flags */

#define TCP_FIN                   0x1   /* 0b01     */
#define TCP_SYN                   0x2   /* 0b10     */ 
#define TCP_RST                   0x4   /* 0b100    */ 
#define TCP_PSH                   0x8   /* 0b1000   */ 
#define TCP_ACK                   0x10  /* 0b10000  */ 
#define TCP_URG                   0x20  /* 0b100000 */ 

/* TCP Constants */

#define SEQ_NUMBER_NOT_SET        0
#define AF_INET                   2 
#define MAX_TCP_CONNECTIONS       50
#define MAX_SEGMENT_LIFETIME      120
#define TCP_CONNECTION_TIMEOUT    (MAX_SEGMENT_LIFETIME * 2)   
#define TCP_INITIAL_WINDOW_SIZE   1024
#define MIN_TCP_PACKET_LEN        sizeof(Ethernet_Header) + sizeof(IP_Header) + sizeof(TCP_Header)
#define MAX_DATA_LEN              4096
#define DEFAULT_WINDOW_SIZE       8192
#define MAX_VALID_PORT            65535

#endif /* TCP__H */
