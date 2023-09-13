/*
 * tcp_functions.h
 */

#ifndef TCP_FUNCTIONS__H
#define TCP_FUNCTIONS__H

/* Implementation Headers */

#include "c_headers.h"
#include "ip.h"
#include "tcp.h"

/* 
    TCP FUNCTIONS
*/

/* Utility */

TCP_Flags            *get_tcp_flags(TCP_Header *tcp_header);
uint32_t              get_random_sequence_number();
uint16_t              get_random_port_number();
int                   is_listening_port(uint16_t port_num);
uint16_t              calculate_tcp_checksum(TCP_Header *tcp_header, uint8_t *payload, ssize_t payload_len, uint32_t ip_src, uint32_t ip_dst);
void                  print_connection_info(TCP_Connection *connection, int conn_num);
int                   extract_ip_and_port(char *input, uint32_t *ip, uint16_t *port);

/* Segment handler */

void                  handle_tcp_segment(IP_Header *ip_packet, ssize_t ip_packet_len);
int                   valid_tcp_packet(TCP_Header *tcp_header, uint32_t ip_src, uint32_t ip_dst, ssize_t payload_len, ssize_t calculated_segment_len);

/* Connection implementation */

int                   init_tcp_connections_list();
void                  free_tcp_connections_list(TCP_Connections_List *connections_list);
TCP_Connection       *create_tcp_connection(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, 
                                            uint16_t window_size, uint32_t seq_number, uint32_t ack_number, TCP_State state);
int                   add_tcp_connection(TCP_Connections_List *connections_list, TCP_Connection *connection);
TCP_Connection       *find_tcp_connection(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port);
int                   remove_tcp_connection(TCP_Connections_List *connections_list, TCP_Connection *connection);
int                   reset_selected_connection();

/* User input and commands */

int                   handle_input(char *input, ssize_t input_len);
void                  show_help();
void                  show_all_connections();
int                   switch_to_connection(int conn_num);
int                   active_create_connection(uint32_t ip_dst, uint16_t dst_port);
int                   close_connection(int conn_num);
int                   active_create_connection(uint32_t ip_dst, uint16_t dst_port);
void                  change_active_port(uint16_t port);

/* State machine */

void                  handle_tcp_connection(TCP_Header *tcp_header, TCP_Flags *flags, TCP_Connection *connection, ssize_t tcp_payload_len);
void                  establish_conn_and_print(TCP_Connection *connection, char *dst_ip);
void                  remove_conn_and_print(TCP_Connection *connection, char *dst_ip);
void                  dest_closes_connection(TCP_Connection *connection);
void                  update_connection_seq_ack(TCP_Connection *connection, uint32_t seq_num_increment, uint32_t ack_num_increment);
void                  display_tcp_data(TCP_Header *tcp_header, ssize_t payload_len, TCP_Connection *connection);

/* Constructing and sending */

uint8_t              *construct_tcp_packet(TCP_Connection *connection, TCP_Flags *flags, uint16_t id, void *payload, ssize_t payload_len);
int                   construct_and_send_tcp_packet(TCP_Flags *flags, TCP_Connection *connection, void *payload, ssize_t payload_len);
int                   send_syn(TCP_Connection *connection);
int                   send_syn_ack(TCP_Connection *connection);
int                   send_fin_ack(TCP_Connection *connection);
int                   send_ack(TCP_Connection *connection);

#endif /* TCP_FUNCTIONS__H */
