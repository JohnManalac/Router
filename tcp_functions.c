/*
 * tcp_functions.c
 */

/* Implementation Headers */

#include <sys/random.h>
#include <ctype.h>
#include "c_headers.h"
#include "cs431vde.h"
#include "util.h"
#include "router.h"
#include "router_functions.h"
#include "ethernet_functions.h"
#include "ip.h"
#include "ip_functions.h"
#include "tcp.h"
#include "tcp_functions.h"

/* 
    FUNCTION IMPLEMENTATIONS
*/

/* 
    UTILITY FUNCTIONS
*/

/* Extract the TCP flags from the TCP packet. */

TCP_Flags *  
get_tcp_flags(TCP_Header *tcp_header)
{
    uint8_t    control_bits = ntohs(tcp_header->offset_reserved_control) & 0x3F;
    TCP_Flags *flags;

    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return NULL;
    }

    /* Memset all flags to 0. */

    memset(flags, 0, sizeof(TCP_Flags));

    /* Check if each TCP control flag is set. */ 
    
    if (control_bits & TCP_FIN) 
    {
        flags->FIN_FLAG_SET = 1;
    }
    if (control_bits & TCP_SYN) 
    {
        flags->SYN_FLAG_SET = 1;
    }
    if (control_bits & TCP_RST) 
    {
        flags->RST_FLAG_SET = 1;
    }
    if (control_bits & TCP_PSH) 
    {
        flags->PSH_FLAG_SET = 1;
    }
    if (control_bits & TCP_ACK) 
    {
        flags->ACK_FLAG_SET = 1;
    }
    if (control_bits & TCP_URG) 
    {
        flags->URG_FLAG_SET = 1;
    }

    return flags; 
}

/* Generates a random sequence number (uint32_t). */

uint32_t
get_random_sequence_number()
{
    uint32_t random_seq_num;

    getrandom(&random_seq_num, sizeof(uint32_t), 0);

    return random_seq_num;
}

/* Generates a random sequence number (uint16_t, restricted to VALID PORT NUMS). */

uint16_t
get_random_port_number()
{
    uint16_t random_port_num;

    getrandom(&random_port_num, sizeof(uint16_t), 0);
    random_port_num %= MAX_VALID_PORT; 

    return random_port_num;
}

/* Verifies if a port number is being listened to. */

int 
is_listening_port(uint16_t port_num)
{
    for (int i = 0; i < NUM_LISTENING_PORTS; i++)
    {
        if (port_num == LISTENING_PORTS[i])
        {
            return 1; 
        }
    }

    return 0; 
}

/* Calculates the TCP checksum. Sets the passed tcp_header's own checksum to 0. */

uint16_t  
calculate_tcp_checksum(TCP_Header *tcp_header, uint8_t *payload, ssize_t payload_len, uint32_t ip_src, uint32_t ip_dst)
{
    TCP_Pseudoheader *pseudo_header;
    int               add_padding = 0; 
    uint16_t          checksum;

    /* Check for padding and allocate stack buffer. */

    if (payload_len % 2 != 0)
    {
        add_padding = 1;
    }

    uint8_t pseudo_header_and_tcp_segment[sizeof(TCP_Pseudoheader) + sizeof(TCP_Header) + payload_len + add_padding];

    /* Malloc space for pseudo header. */

    if ((pseudo_header = malloc(sizeof(TCP_Pseudoheader))) == NULL)
    {
        return -1;
    }

    /* Set psuedo-header fields. */

    pseudo_header->src_address = htonl(ip_src); 
    pseudo_header->dst_address = htonl(ip_dst);
    pseudo_header->zero        = 0; 
    pseudo_header->protocol    = TCP_PROTOCOL;
    pseudo_header->tcp_length  = htons(sizeof(TCP_Header) + payload_len); 
    tcp_header->checksum       = 0;

    /* Prefix pseudoheader to header and payload. */

    memcpy(pseudo_header_and_tcp_segment, pseudo_header, sizeof(TCP_Pseudoheader));
    memcpy(pseudo_header_and_tcp_segment + sizeof(TCP_Pseudoheader), tcp_header, sizeof(TCP_Header));
    memcpy(pseudo_header_and_tcp_segment + sizeof(TCP_Pseudoheader) + sizeof(TCP_Header), payload, payload_len);  

    /* Pad last byte on the right if needed. */

    if (add_padding)
    {
        memset(pseudo_header_and_tcp_segment + sizeof(TCP_Pseudoheader) + sizeof(TCP_Header) + payload_len, 0, 1);
    }

    /* Calculate checksum and free pseudo header. */

    checksum = RFC1071_checksum(pseudo_header_and_tcp_segment, sizeof(TCP_Pseudoheader) + sizeof(TCP_Header) + payload_len + add_padding);
    free(pseudo_header);

    return checksum;
}

/* Print connection info. */

void 
print_connection_info(TCP_Connection *connection, int conn_num)
{
    uint32_t        net_src_ip, net_dst_ip; 
    char            src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN]; 

    /* Convert IP source and destination to 0.0.0.0 format. */

    net_src_ip = htonl(connection->src_ip);
    net_dst_ip = htonl(connection->dst_ip);
    inet_ntop(AF_INET, &(net_src_ip), src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(net_dst_ip), dst_ip, INET_ADDRSTRLEN);

    /* Print out connection. */

    if (conn_num == -1 )
    {
        printf("    Src IP: %s Src port: %d Dst IP: %s Dst Port: %d \n", src_ip, connection->src_port, dst_ip, connection->dst_port);
    }
    else 
    {
        printf("    %d. Src IP: %s Src port: %d Dst IP: %s Dst Port: %d \n", conn_num, src_ip, connection->src_port, dst_ip, connection->dst_port);
    }
}

/* Extract ip and port number from input string. */

int
extract_ip_and_port(char *input, uint32_t *ip, uint16_t *port)
{
    char ip_str[16];
    char port_str[6];
    int  port_conv; 

    /* Parse the string, extracting IP address and port. */

    sscanf(input, "%s %s", ip_str, port_str);

    /* Convert IP address. */

    if (inet_aton(ip_str, (struct in_addr*)ip) == 0) 
    {
        printf("Invalid IP address. \n\n");
        return -1;
    }
    
    /* Convert port address. */

    port_conv = atoi(port_str);

    if (port_conv <= 0 || port_conv > MAX_VALID_PORT)
    {
        printf("\nInvalid port number.\n\n");
        return -1;   
    }

    *ip   = ntohl(*ip);
    *port = (uint16_t)port_conv;

    return 0; 
}

/* 
    SEGMENT HANDLER FUNCTIONS
*/

/* Handle a TCP segment, encapsulated in an IP packet. NOTE: When creating or 
   finding a connection, the received packet's source and destination IP/ports 
   are switched. Also, when creating a new connection, the connection's seq number
   will be randomized and its ack number will be the initial syn packet's seq number. */

void 
handle_tcp_segment(IP_Header *ip_packet, ssize_t ip_packet_len)
{
    TCP_Header       *tcp_header;
    TCP_Flags        *flags;
    TCP_Connection   *connection; 
    uint8_t           ihl, data_offset;
    uint16_t          src_port, dst_port, window_size; 
    uint32_t          ip_src, ip_dst, seq_number;
    ssize_t           segment_len, tcp_payload_len;  

    /* Get TCP header, and calculate the segment and payload length. */

    ihl               = (ip_packet->version_and_IHL & 0x0F) * 4;
    tcp_header        = (TCP_Header *)((uint8_t *)ip_packet + ihl);
    data_offset       = ntohs(tcp_header->offset_reserved_control) >> 12;
    segment_len       = data_offset * 4;
    tcp_payload_len   = ip_packet_len - ihl - sizeof(TCP_Header); 

    /* Extract TCP and IP fields. */

    ip_src            = ntohl(ip_packet->source);
    ip_dst            = ntohl(ip_packet->destination);
    src_port          = ntohs(tcp_header->src_port); 
    dst_port          = ntohs(tcp_header->dst_port);
    window_size       = ntohs(tcp_header->window_size);
    seq_number        = ntohl(tcp_header->seq_number);

    /* Verify the validity of the packet. */

    if (!valid_tcp_packet(tcp_header, ip_src, ip_dst, tcp_payload_len, segment_len))
    {
        return; 
    }

    /* Extract TCP flags. */

    if ((flags = get_tcp_flags(tcp_header)) == NULL)
    {
        return; 
    }

    /* Initialize the connections list. */

    if (TCP_CONNECTIONS_LIST == NULL)
    {
        init_tcp_connections_list();
    }

    /* Find the TCP connection and handle it. If it doesn't exist, create and add it to the list. */

    if ((connection = find_tcp_connection(ip_dst, ip_src, dst_port, src_port)) == NULL)
    {
        connection = create_tcp_connection(ip_dst, ip_src, dst_port, src_port, window_size, get_random_sequence_number(), seq_number, TCP_LISTEN);
        add_tcp_connection(TCP_CONNECTIONS_LIST, connection);
    }

    /* Handle TCP connection. */

    handle_tcp_connection(tcp_header, flags, connection, tcp_payload_len);
}

/* Verifies the length, checksum, and destination port of a TCP packet. Note that the
   actual segment len only needs to be greater than or equal to the passed segment length 
   calculated from the data offset. Returns 0 if any of the above verifications fail. */

int 
valid_tcp_packet(TCP_Header *tcp_header, uint32_t ip_src, uint32_t ip_dst, ssize_t payload_len, ssize_t calculated_segment_len)
{
    uint8_t  segment_len         = sizeof(TCP_Header) + payload_len; 
    uint8_t *payload             = (uint8_t *)(tcp_header) + sizeof(TCP_Header);
    uint16_t old_checksum        = tcp_header->checksum;
    uint16_t calculated_checksum = calculate_tcp_checksum(tcp_header, payload, payload_len, ip_src, ip_dst); 
    
    /* Verify TCP segment length. */

    if (segment_len < calculated_segment_len)
    {
        printf("Dropping TCP segment. Bad segment length.\n");
        return 0; 
    }

    /* Verify TCP header checksum. */

    if (calculated_checksum != old_checksum)
    {
        printf("Dropping TCP packet. Bad checksum.\n");
        return 0; 
    }

    /* Verify listening on the destination port. */

    if (!is_listening_port(ntohs(tcp_header->dst_port)))
    {
        printf("Dropping TCP packet. Not listening on port.\n");
        return 0; 
    }

    return 1; 
}

/* 
    CONNECTION IMPLEMENTATION FUNCTIONS
*/

/* Initialize the list of TCP connections. If malloc fails, return -1.*/

int
init_tcp_connections_list()
{
    TCP_Connections_List *connections; 

    /* Malloc space for connections list. */

    if ((connections = malloc(sizeof(TCP_Connections_List))) == NULL)
    {
        return -1;
    }

    /* Set list fields and initialize global list. */

    connections->size = 0;
    connections->head = NULL;
    connections->tail = NULL;

    TCP_CONNECTIONS_LIST = connections;

    return 1; 
}

/* Free all connections, nodes and the list of TCP connections itself. */

void 
free_tcp_connections_list(TCP_Connections_List *connections_list) 
{
    TCP_Node *curr = connections_list->head;
    TCP_Node *prev = NULL; 

    /* Free all connections and nodes until end of list. */

    while (curr != NULL)
    {
        free(curr->connection);
        prev = curr; 
        curr = curr->next; 
        free(prev);
    }
    
    /* Free the list. */

    free(connections_list);
}

/* Create a connection and returns a pointer to it. If malloc fails, return NULL. */

TCP_Connection *
create_tcp_connection(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port,
                      uint16_t window_size, uint32_t seq_number, uint32_t ack_number, TCP_State state)
{
    TCP_Connection *tcp_connection; 

    /* Malloc space for connection. */

    if ((tcp_connection = malloc(sizeof(TCP_Connection))) == NULL)
    {
        return NULL; 
    }

    /* Set connection fields. */

    tcp_connection->src_ip      = src_ip;
    tcp_connection->dst_ip      = dst_ip;
    tcp_connection->src_port    = src_port;
    tcp_connection->dst_port    = dst_port;
    tcp_connection->window_size = window_size;
    tcp_connection->seq_number  = seq_number;
    tcp_connection->ack_number  = ack_number;
    tcp_connection->state       = state;

    return tcp_connection;
}

/* Add a TCP connection to the connections list. If malloc fails, return -1. */

int 
add_tcp_connection(TCP_Connections_List *connections_list, TCP_Connection *connection)
{
    TCP_Node *tcp_node;

    /* Malloc space for node. */

    if ((tcp_node = malloc(sizeof(TCP_Node))) == NULL)
    {
        return -1; 
    }

    /* Check if the list is empty. */

    if (connections_list->head == NULL)
    {
        connections_list->head       = tcp_node;
    }
    else
    {
        connections_list->tail->next = tcp_node; 
    }

    /* Increase the list size and insert the connection. */

    connections_list->size++; 
    connections_list->tail           = tcp_node;
    tcp_node->next                   = NULL;
    tcp_node->connection             = connection; 

    return 1; 
}

/* Search for a TCP connection. Returns NULL if not found. */

TCP_Connection * 
find_tcp_connection(uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port)
{
    TCP_Connection *connection; 
    TCP_Node       *curr = TCP_CONNECTIONS_LIST->head;
    int             found_connection; 

    while (curr != NULL)
    {
        connection           = curr->connection;
        found_connection     = 1;

        /* Check if connection fields are the same. If they are,
           return the connection. Else, continue searching. */

        if (connection->src_ip != src_ip)
        {
            found_connection = 0;
        }
        if (connection->dst_ip != dst_ip)
        {
            found_connection = 0;
        }
        if (connection->src_port != src_port)
        {
            found_connection = 0;
        }
        if (connection->dst_port != dst_port)
        {
            found_connection = 0;
        }
        
        if (found_connection)
        {
            return connection;
        }

        curr = curr->next;
    }

    return NULL;
}

/* Remove a connection from the connections list. Returns -1 on failure (cannot find connection). */

int
remove_tcp_connection(TCP_Connections_List *connections_list, TCP_Connection *connection)
{
    TCP_Node *curr          = connections_list->head; 
    TCP_Node *prev          = NULL; 
    int       deleting_curr = 0; 

    while (curr != NULL)
    {
        /* Check for a matching connection. */

        if (memcmp(curr->connection, connection, sizeof(TCP_Connection)) == 0)
        {
            /* If removing current connection to send data to. */

            if (CURRENT_CONNECTION == curr->connection)
            {
                deleting_curr = 1; 
            }
            
            /* If removing the list head. */

            if (prev == NULL)
            {
                connections_list->head = curr->next; 
            }
            else
            {
                prev->next = curr->next; 
            }

            /* If removing the list tail. */

            if (connections_list->tail == curr)
            {
                connections_list->tail = prev; 
            }

            /* Free structs and reduce size. */

            free(curr->connection);
            free(curr);
            connections_list->size--;

            /* If deleting the currently selected connection, reset to the 
               first established connection in list. */

            if (deleting_curr)
            {
                reset_selected_connection();
            }

            return 1; 
        }

        prev = curr; 
        curr = curr->next; 
    }

    return -1;
}

/* Reset the selected connection to send data to as the first established connection or NULL. */

int 
reset_selected_connection()
{
    TCP_Node *curr = TCP_CONNECTIONS_LIST->head; 

    while (curr != NULL)
    {
        if (curr->connection->state == TCP_ESTABLISHED)
        {
            CURRENT_CONNECTION = curr->connection; 
            return 1; 
        }

        curr = curr->next; 
    }

    /* No remaining established connections. Set to NULL */
    
    CURRENT_CONNECTION = NULL; 

    return 0;
}

/* 
    USER INPUT AND COMMAND FUNCTIONS
*/

/* Handle input from user. Sends data to currently selected established 
   connection OR runs a specific command. */

int                  
handle_input(char *input, ssize_t input_len)
{
    TCP_Flags *flags; 
    char      *num_pos;
    int        conn_num;

    /* Command /HELP  */

    if (memcmp(input, "/HELP\n", sizeof("/HELP\n") - 1) == 0)
    {
        show_help();
        return 1; 
    }

    /* Command /SHOWALL */

    if (memcmp(input, "/SHOWALL\n", sizeof("/SHOWALL\n") - 1) == 0)
    {
        /* Connections list not established or no existing connections. */

        if (TCP_CONNECTIONS_LIST == NULL || TCP_CONNECTIONS_LIST->head == NULL)
        {
            printf("No connections to show.\n\n");
            return 0;
        }

        show_all_connections();
        return 1; 
    }

    /* Command /SWITCHTO */

    if (memcmp(input, "/SWITCHTO ", sizeof("/SWITCHTO ") - 1) == 0)
    {
        /* Connections list not established or no existing connections. */

        if (TCP_CONNECTIONS_LIST == NULL || TCP_CONNECTIONS_LIST->head == NULL)
        {
            printf("No connections to switch to.\n\n");
            return 0;
        }

        /* Extract the connection number to switch to. */

        num_pos = input + (sizeof("/SWITCHTO ") - 1);

        if (isdigit(*(num_pos)))
        {
            sscanf(num_pos, "%d", &conn_num);
        }
        else 
        {
            printf("Please input a valid connection number to switch to.\n\n");
            return -1;
        }

        switch_to_connection(conn_num);
        return 1; 
    }

    /* Command /CLOSE */

    if (memcmp(input, "/CLOSE ", sizeof("/CLOSE ") - 1) == 0)
    {
        /* Connections list not established or no existing connections. */

        if (TCP_CONNECTIONS_LIST == NULL || TCP_CONNECTIONS_LIST->head == NULL)
        {
            printf("No connections to close.\n\n");
            return 0;
        }

        /* Extract the connection number to close. */

        num_pos = input + (sizeof("/CLOSE ") - 1);

        if (isdigit(*(num_pos)))
        {
            sscanf(num_pos, "%d", &conn_num);
        }
        else 
        {
            printf("Please input a valid connection number to close.\n\n");
            return -1;
        }

        close_connection(conn_num);
        return 1; 
    }

    /* Command /CONNECT */

    if (memcmp(input, "/CONNECT ", sizeof("/CONNECT ") - 1) == 0)
    {
        uint16_t dst_port;
        uint32_t ip_dst;

        /* Extract the IP address and port number. */

        num_pos = input + (sizeof("/CONNECT ") - 1);

        if (extract_ip_and_port(num_pos, &ip_dst, &dst_port) == -1)
        {
            return -1; 
        }

        /* Create connection and attempt to connect. */
        
        if (active_create_connection(ip_dst, dst_port) != -1)
        {
            printf("Attempting to connect. Use /SHOWALL to see established connections.\n\n");
            return 0;
        }
        
        return -1; 
    }

    /* Command /ACTIVEPORT */

    if (memcmp(input, "/ACTIVEPORT\n", sizeof("/ACTIVEPORT\n") - 1) == 0)
    {
        if (!ACTIVE_SENDING_PORT)
        {
            ACTIVE_SENDING_PORT = LISTENING_PORTS[0]; 
        }

        printf("\nCurrent active port: %d \n\n", ACTIVE_SENDING_PORT);

        return 0; 
    }

    if (memcmp(input, "/ACTIVEPORT ", sizeof("/ACTIVEPORT ") - 1) == 0)
    {
        uint16_t port = 0;  

        sscanf(input + sizeof("/ACTIVEPORT ") - 1, "%hu", &port);
        change_active_port(port);
        
        return 1; 
    }

    /* Sending data. Check if connection is non-NULL. */

    if (CURRENT_CONNECTION == NULL)
    {
        printf("No connection to send data to. \n\n");

        return 0; 
    }

    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return -1;
    }

    /* Set flags. */

    memset(flags, 0, sizeof(TCP_Flags));
    flags->ACK_FLAG_SET = 1;
    flags->PSH_FLAG_SET = 1;

    /* Construct and send packet. Update sequence number by input length. */

    construct_and_send_tcp_packet(flags, CURRENT_CONNECTION, input, input_len);
    update_connection_seq_ack(CURRENT_CONNECTION, input_len, 0);
    free(flags);

    return 0;
}

/* Show help message. */

void 
show_help()
{
    printf("\nHELP:\n");
    printf("    Use /SHOWALL to show all connections and currently connected connection (to send data to).\n");
    printf("    Use /SWITCHTO 0 to switch to an established connection when sending data (replace 0). \n");
    printf("    Use /CLOSE 0 to close an established connection (replace 0).\n");
    printf("    Use /CONNECT 0.0.0.0 4000 to actively connect to an IP and port (replace 0.0.0.0 and 4000).\n");    
    printf("    Use /ACTIVEPORT to view the current port to actively create connections.\n");
    printf("    Use /ACTIVEPORT 4000 to replace the current port to actively create connections (replace 4000).\n\n");
}

/* Show currently connected connection and all ESTABLISHED connections. */

void 
show_all_connections()
{
    TCP_Node       *curr = TCP_CONNECTIONS_LIST->head; 
    int             conn_num = 0;

    /* Print current connection to send to. */

    if (CURRENT_CONNECTION == NULL)
    {
        printf("\nCurrent connection (sending to): NONE \n");
    }
    else 
    {
        printf("\nCurrent connection (sending to): \n");
        print_connection_info(CURRENT_CONNECTION, -1);
    }

    /* Print all established connections. */

    printf("\nShowing all established connections:\n");

    while (curr != NULL)
    {
        if (curr->connection->state == TCP_ESTABLISHED)
        {
            print_connection_info(curr->connection, conn_num);
            conn_num++; 
        }

        curr = curr->next; 
    }

    printf("\n");
}

/* Switch to an established connection based on its connection number (shown in through /SHOWALL). */

int 
switch_to_connection(int conn_num)
{
    TCP_Node       *curr       = TCP_CONNECTIONS_LIST->head; 
    int             curr_conn  = 0; 

    /* Search for matching connection. */

    while (curr != NULL)
    {
        if (curr->connection->state == TCP_ESTABLISHED)
        {
            if (curr_conn == conn_num)
            {
                printf("Switching to connection %d. \n", conn_num);
                CURRENT_CONNECTION = curr->connection;
                return 1;
            }

            curr_conn++;
        }

        curr = curr->next; 
    }

    printf("Connection %d is not established or does not exist. \n\n", conn_num);

    return 0; 
}

/* Close a connection based on its connection number (shown in through /SHOWALL). */

int 
close_connection(int conn_num)
{
    TCP_Node        *curr       = TCP_CONNECTIONS_LIST->head; 
    int              curr_conn  = 0;  

    /* Search for matching connection. */

    while (curr != NULL)
    {
        if (curr->connection->state == TCP_ESTABLISHED)
        {            
            if (curr_conn == conn_num)
            {
                printf("Closing connection %d. \n\n", conn_num);
                send_fin_ack(curr->connection); 
                curr->connection->state = TCP_FIN_WAIT_1;
                reset_selected_connection();
                return 1;          
            }
            
            curr_conn++; 
        }

        curr = curr->next; 
    } 
    
    printf("Connection %d is not established or does not exist. \n\n", conn_num);

    return 0;
}

/* Actively create a conncetion and connect to a specific IP and port. */

int 
active_create_connection(uint32_t ip_dst, uint16_t dst_port)
{
    TCP_Connection *connection; 
    uint16_t        src_port; 
    uint32_t        ip_src   = ROUTER_INTERFACES[0].ip_address;

    /* Initialize the connections list. */

    if (TCP_CONNECTIONS_LIST == NULL)
    {
        init_tcp_connections_list();
    }

    /* Get source port. */

    if (!ACTIVE_SENDING_PORT)
    {
        ACTIVE_SENDING_PORT = LISTENING_PORTS[0]; 
    }
    
    src_port = ACTIVE_SENDING_PORT; 

    /* Search for existing connection or create a new one. */
    
    if ((connection = find_tcp_connection(ip_src, ip_dst, src_port, dst_port)) == NULL)
    {
        connection = create_tcp_connection(ip_src, ip_dst, src_port, dst_port, DEFAULT_WINDOW_SIZE, get_random_sequence_number(), 0, TCP_LISTEN);
        add_tcp_connection(TCP_CONNECTIONS_LIST, connection);
    }
    else if (connection->state == TCP_ESTABLISHED)
    {
        printf("\nConnection already established. \n\n");
        return 0;
    }

    /* Attempt to connect to connection if not established. */

    if (send_syn(connection) == -1)
    {
        printf("\nFailed to send SYN packet. \n\n");
        return -1;
    }

    connection->state = TCP_SYN_SENT;

    return 1;
}

void
change_active_port(uint16_t port)
{
    if (port == 0 || !is_listening_port(port))
    {
        printf("\nPlease choose a valid source port. \n");
        printf("\nCurrently listening on the following ports: \n");

        for (int i = 0; i < NUM_LISTENING_PORTS; i++)
        {
            if (i % 5 == 0)
            {
                printf("\n      "); 
            }

            printf("%d  ", LISTENING_PORTS[i]);
        }

        printf("\n\n");
    }
    else 
    {
        printf("\nChanging active port number to %d. \n\n", port);
        ACTIVE_SENDING_PORT = port; 
    }
}

/* 
    STATE MACHINE FUNCTIONS
*/

void 
handle_tcp_connection(TCP_Header *tcp_header, TCP_Flags *flags, TCP_Connection *connection, ssize_t tcp_payload_len)
{
    uint32_t net_dst_ip;
    char     dst_ip[INET_ADDRSTRLEN]; 

    /* Diagnostic info. */
    net_dst_ip = htonl(connection->dst_ip);
    inet_ntop(AF_INET, &(net_dst_ip), dst_ip, INET_ADDRSTRLEN);

    switch (connection->state) 
    {
        case TCP_CLOSED:

            break;

        case TCP_LISTEN:

            if (flags->SYN_FLAG_SET) 
            {
                update_connection_seq_ack(connection, 0, 1);
                send_syn_ack(connection);
                connection->state = TCP_SYN_RECEIVED;
            }
            break;

        case TCP_SYN_SENT:

            if (flags->SYN_FLAG_SET && flags->ACK_FLAG_SET) 
            {
                update_connection_seq_ack(connection, 0, ntohl(tcp_header->seq_number) + 1);
                send_ack(connection);
                establish_conn_and_print(connection, dst_ip);
            } 
            break;

        case TCP_SYN_RECEIVED:

            if (flags->FIN_FLAG_SET)
            {
                dest_closes_connection(connection);          
            }
            else if (flags->ACK_FLAG_SET) 
            {
                establish_conn_and_print(connection, dst_ip);
            } 
            break;

        case TCP_ESTABLISHED:
            
            if (flags->FIN_FLAG_SET)
            {
                dest_closes_connection(connection);          
            }
            else if (tcp_payload_len > 0)
            {
                /* Receive and acknowledge TCP segments containing data. */
                display_tcp_data(tcp_header, tcp_payload_len, connection);
                update_connection_seq_ack(connection, 0, tcp_payload_len);
                send_ack(connection);
            }
            break;

        case TCP_FIN_WAIT_1:

            if (flags->ACK_FLAG_SET)
            {
                connection->state = TCP_FIN_WAIT_2;
            }
            break;

        case TCP_FIN_WAIT_2:

            if (flags->FIN_FLAG_SET)
            {
                update_connection_seq_ack(connection, 1, 1);
                send_ack(connection);
                connection->state = TCP_CLOSING;
            }
            break;

        case TCP_CLOSE_WAIT:

            /* Not implemented. */
            break;
        
        case TCP_CLOSING:

            if (flags->ACK_FLAG_SET)
            {
                remove_conn_and_print(connection, dst_ip);
            }
            break;

        case TCP_LAST_ACK:

            if (flags->ACK_FLAG_SET) 
            {
                remove_conn_and_print(connection, dst_ip);
            } 

            break; 

        case TCP_TIME_WAIT:
        
            /* Not implemented. */
            break; 
    }
}

/* Set connection as established and current, and print diagnostic information. */

void 
establish_conn_and_print(TCP_Connection *connection, char *dst_ip)
{
    connection->state  = TCP_ESTABLISHED;    
    CURRENT_CONNECTION = connection; 
    printf("\n    NOTIFICATION: a connection has been established from %s on port %d.\n", dst_ip, connection->dst_port);
    printf("    Use /SHOWALL to view current connections. \n\n");
    fflush(stdout);
} 

/* Remove connection and print diagnostic information. */

void 
remove_conn_and_print(TCP_Connection *connection, char *dst_ip)
{
    remove_tcp_connection(TCP_CONNECTIONS_LIST, connection);
    printf("\n    NOTIFICATION: a connection has been closed from %s on port %d.\n", dst_ip, connection->dst_port);
    printf("    Use /SHOWALL to view current connections. \n\n");
    fflush(stdout);
} 

/* Destination sends a fin-ack and closes the connection. Send an ACK and a FIN-ACK. */

void 
dest_closes_connection(TCP_Connection *connection)
{
    update_connection_seq_ack(connection, 0, 1);
    send_ack(connection);
    send_fin_ack(connection);
    connection->state = TCP_LAST_ACK;
}

/* Update a TCP connection's sequence and ack numbers. */

void
update_connection_seq_ack(TCP_Connection *connection, uint32_t seq_num_increment, uint32_t ack_num_increment)
{
    connection->seq_number += seq_num_increment;
    connection->ack_number += ack_num_increment; 
}

/* Display/print received data from a TCP connection. */

void 
display_tcp_data(TCP_Header *tcp_header, ssize_t payload_len, TCP_Connection *connection)
{
    uint8_t  payload_str[payload_len + 1]; 
    uint8_t *payload     = (uint8_t *)tcp_header + sizeof(TCP_Header);
    uint32_t net_dst_ip  =  htonl(connection->dst_ip); 
    char     dst_ip[INET_ADDRSTRLEN]; 

    if (payload_len > 0)
    {        
        /* Insert null byte. */

        memcpy(payload_str, payload, payload_len);
        payload_str[payload_len] = '\0'; 

        /* Convert IP to 0.0.0.0. format and print. */

        inet_ntop(AF_INET, &(net_dst_ip), dst_ip, INET_ADDRSTRLEN);
        printf("\n(%s port %d): %s \n", dst_ip, connection->dst_port, payload_str);
    }
}

/*
    CONSTRUCTING AND SENDING FUNCTIONS
*/

/* Construct an IP packet with a TCP segment. */

uint8_t *
construct_tcp_packet(TCP_Connection *connection, TCP_Flags *flags, uint16_t id, void *payload, ssize_t payload_len)
{
    ssize_t        tcp_segment_len = sizeof(TCP_Header) + payload_len;
    TCP_Header    *tcp_segment;
    IP_Header     *ip_packet;
    uint8_t       *tcp_packet;
    const uint8_t *mac_src, *mac_dst;
    uint16_t       data_offset, offset_reserved_control, checksum;

    /* Malloc space for segment. */

    if ((tcp_segment = malloc(tcp_segment_len)) == NULL)
    {
        return NULL;
    }

    /* Set TCP fields. */

    tcp_segment->src_port       = htons(connection->src_port);
    tcp_segment->dst_port       = htons(connection->dst_port);
    tcp_segment->seq_number     = htonl(connection->seq_number);
    tcp_segment->ack_number     = htonl(connection->ack_number);
    tcp_segment->window_size    = htons(connection->window_size); 
    tcp_segment->checksum       = 0;
    tcp_segment->urgent_pointer = 0;

    /* Set Data Offset and Control Bits. */

    data_offset                 = sizeof(TCP_Header) / 4;
    offset_reserved_control     = data_offset << 12;
    
    if (flags->FIN_FLAG_SET) 
    {
        offset_reserved_control |= TCP_FIN;
    }
    if (flags->SYN_FLAG_SET) 
    {
        offset_reserved_control |= TCP_SYN;
    }
    if (flags->RST_FLAG_SET) 
    {
        offset_reserved_control |= TCP_RST;
    }
    if (flags->PSH_FLAG_SET) 
    {
        offset_reserved_control |= TCP_PSH;
    }
    if (flags->ACK_FLAG_SET) 
    {
        offset_reserved_control |= TCP_ACK;
    }
    if (flags->URG_FLAG_SET) 
    {
        offset_reserved_control |= TCP_URG;
    }

    tcp_segment->offset_reserved_control = htons(offset_reserved_control);

    /* Memcopy payload after header and get checksum. */

    if (payload_len > 0)
    {
        memcpy(tcp_segment + 1, payload, payload_len);
    }

    checksum              = calculate_tcp_checksum(tcp_segment, payload, payload_len, connection->src_ip, connection->dst_ip);
    tcp_segment->checksum = checksum;

    /* Construct IP packet.*/

    ip_packet             = construct_ip_packet(connection->src_ip, connection->dst_ip, id, TCP_PROTOCOL, DEFAULT_TTL, tcp_segment, tcp_segment_len);

    /* Construct Ethernet Frame. */

    mac_src               = ROUTER_INTERFACES[0].mac_address;
    mac_dst               = find_arp_mac_address(connection->dst_ip);

    /* ARP translation failed: no ARP for given IP address.  s*/

    if (mac_dst == NULL)
    {
        free(tcp_segment);
        free(ip_packet);
        return NULL;         
    }

    tcp_packet            = construct_ethernet_frame(mac_src, mac_dst, IP_TYPE, ip_packet, tcp_segment_len + sizeof(IP_Header));

    return tcp_packet;     
}

/* Construct and send a TCP packet (through interface R0_0). */

int
construct_and_send_tcp_packet(TCP_Flags *flags, TCP_Connection *connection, void *payload, ssize_t payload_len)
{
    uint8_t *tcp_packet; 

    tcp_packet = construct_tcp_packet(connection, flags, 12345, payload, payload_len);

    /* Send packet ONLY if no errors occur (malloc or ARP fails). */

    if (tcp_packet == NULL)
    {
        return -1;
    }
    else 
    {
        send_ethernet_frame(ROUTER_INTERFACES[0].fds[1], tcp_packet, MIN_TCP_PACKET_LEN + payload_len);
    }

    free(tcp_packet);
    return 0;
}

/* Send a SYN packet. */

int 
send_syn(TCP_Connection *connection)
{
    TCP_Flags *flags; 

    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return -1;
    }

    /* Set flags */

    memset(flags, 0, sizeof(TCP_Flags));
    flags->SYN_FLAG_SET = 1;

    /* Construct and send packet. Increase the sequence number by 1 if successful. */

    if (construct_and_send_tcp_packet(flags, connection, NULL, 0) == -1)
    {
        free(flags);
        return -1;
    }

    connection->seq_number += 1; 
    free(flags);
    
    return 0;
}


/* Send a SYN-ACK packet. */

int 
send_syn_ack(TCP_Connection *connection)
{
    TCP_Flags *flags; 

    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return -1;
    }

    /* Set flags */

    memset(flags, 0, sizeof(TCP_Flags));
    flags->SYN_FLAG_SET = 1;
    flags->ACK_FLAG_SET = 1;

    /* Construct and send packet. Increase the sequence number by 1 if successful. */

    if (construct_and_send_tcp_packet(flags, connection, NULL, 0) == -1)
    {
        free(flags);
        return -1;
    }

    connection->seq_number += 1; 
    free(flags);

    return 0;
}

/* Send a FIN-ACK packet. */

int 
send_fin_ack(TCP_Connection *connection)
{
    TCP_Flags *flags; 
    
    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return -1;
    }

    /* Set flags */

    memset(flags, 0, sizeof(TCP_Flags));
    flags->FIN_FLAG_SET = 1;
    flags->ACK_FLAG_SET = 1;

    /* Construct and send packet. */

    if (construct_and_send_tcp_packet(flags, connection, NULL, 0) == -1)
    {
        free(flags);
        return -1;
    }

    free(flags);

    return 0;
}

/* Send an ACK packet. */

int
send_ack(TCP_Connection *connection)
{
    TCP_Flags *flags;
    
    /* Malloc space for flags. */

    if ((flags = malloc(sizeof(TCP_Flags))) == NULL)  
    {
        return -1;
    }

    /* Set flags */

    memset(flags, 0, sizeof(TCP_Flags));
    flags->ACK_FLAG_SET = 1;

    /* Construct and send packet. */

    if (construct_and_send_tcp_packet(flags, connection, NULL, 0) == -1)
    {
        free(flags);
        return -1;
    }

    free(flags);

    return 0;
}
