/*
 * stack.c
 *
 * Router 0 Simulation. 
 * Please read the README!   
 */

/* Implementation Headers */

#include "c_headers.h"
#include "util.h"
#include "cs431vde.h"
#include "frame_crc32.h"
#include "router.h"
#include "router_functions.h"
#include "ethernet_functions.h"
#include "ip_functions.h"
#include "icmp_functions.h"
#include "arp_functions.h"
#include "tcp_functions.h"

/* Function Prototypes */

void print_message();
void print_color_message();

/* MAIN */

int main(int argc, char *argv[])
{
    const Interface *interface; 
    struct pollfd    poll_fds[NUM_INTERFACES + 1];      // Add 1 for stdin        
    uint8_t          ether_frame[ETHERNET_MAX_FRAME_LEN];
    ssize_t          frame_len, input_len;
    int              i, received_data;

    /* Connect to all interfaces. */

    connect_to_interfaces();

    /* Add all interface file descriptors to poll fds. */

    for (i = 0; i < NUM_INTERFACES; i++)
    {
        interface          = &ROUTER_INTERFACES[i];
        poll_fds[i].fd     = interface->fds[0];  
        poll_fds[i].events = POLLIN;
    }

    /* Add stdin fd to the poll fds. */

    poll_fds[NUM_INTERFACES].fd     = STDIN_FILENO;
    poll_fds[NUM_INTERFACES].events = POLLIN; 

    /* Print program message. */

    print_message();

    /* Continously receive data and frames. */

    while (1)
    {
        received_data = poll(poll_fds, NUM_INTERFACES + 1, -1);

        if (received_data == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        /* Receive data from interfaces. */

        for (i = 0; i < NUM_INTERFACES; i++)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if ((frame_len = receive_ethernet_frame(poll_fds[i].fd, ether_frame)) < 0)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                
                handle_ethernet_frame(ether_frame, frame_len, &ROUTER_INTERFACES[i]);
            }
        }

        /* Receive data from stdin. */

        if (poll_fds[NUM_INTERFACES].revents & POLLIN)
        {
            char    input[MAX_DATA_LEN];

            input_len = read(STDIN_FILENO, input, sizeof(input));

            if (input_len < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            /* Add case for ^C if user terminates program. */

            if (input_len > 0)
            {
                handle_input(input, input_len);
                fflush(stdout); 
            }
        }

    }

    return 0;
}

/* 
    PRINTING UTILITY 
*/

void 
print_color_message(char *msg, char *color)
{
    /* Change color. */

    printf("%s", color);

    /* Print message. */

    printf("%s", msg); 

    /* Reset color. */

    printf("\033[0m");
    printf(" ");  
}

void
print_message()
{
    /* Print messages. */

    printf("\033[1;34m");
    printf("STACK PROGRAM. ");
    printf("\033[1;34m");
    printf("BY JOHN MANALAC. ");
    printf("\033[1;34m"); 
    printf("LAST UPDATED MAY 22, 2023.\n");
    printf("\033[1;37m");
    printf("ENTER /HELP TO VIEW COMMANDS. ");
    printf("TYPE INTO STDIN TO SEND DATA.\n");

    /* Reset color. */

    printf("\033[0m"); 
    printf("\n");
}