/*
 * frame_sender.c
 */

/* Implementation Headers*/

#include "c_headers.h"
#include "util.h"
#include "cs431vde.h"
#include "frame_crc32.h"
#include "ethernet_functions.h"
#include "ip.h"
#include "ip_functions.h"
#include "arp.h"
#include "arp_functions.h"

void 
send_ip_packet(int *fds, uint8_t *mac_dest, uint8_t *mac_source, uint8_t protocol,
               char *ip_source, char *ip_dest, int ttl);

int
main(int argc, char *argv[])
{
    int fds[2];
    uint8_t *frame; 
    ssize_t frame_len;

    int connect_to_remote_switch = 0;
    char *vde_file               = "/tmp/net0.vde";
    char *local_vde_cmd[]        = { "vde_plug", vde_file, NULL };
    char *remote_vde_cmd[]       = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                     "/home/pjohnson/cs431/bin/vde_plug", NULL };
    char **vde_cmd               = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

    if (connect_to_vde_switch(fds, vde_cmd) < 0) 
    {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }

    /* 
        Frame 1. 

        Expected output: 

            received 64-byte broadcast frame from 77 88 99 aa bb cc  
    */

    frame = (uint8_t*)  "\xff\xff\xff\xff\xff\xff\x77\x88"
                        "\x99\xaa\xbb\xcc\xff\xff\xff\xff"
                        "\x01\x02\x03\x04\x05\x06\x07\x08"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\x3e\x93\x37\x2f";
    frame_len = 64;

    printf("Sending broadcast frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);

    /*  
        Frame 2. 

        Expected output: 

            ignoring 64-byte frame (not for me)
    */

    frame = (uint8_t*)  "\xab\xcc\xdd\xee\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xe7\xad\x92\xf1";
    frame_len = 64;

    printf("Sending frame for not corresponding mac, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);


    /* 
        Frame 3. 

        Expected output: 

        ignoring 64-byte frame (bad fcs: got 0xdeadbeef, expected 0xe88f7195)
    */

    frame = (uint8_t*)  "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xef\xbe\xad\xde";
    frame_len = 64;

    printf("Sending frame bad fcs, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);


    /* 
        Frame 4. 

        Expected output: 

            ignoring 54-byte frame (short)
    */

    frame = (uint8_t*)  "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff\xff\xff"
                        "\xff\xff\xff\xff\xff\xff";
    frame_len = 54;

    printf("Sending frame too short, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);

    /* 
        Frame 5. 

        Expected output: 

            No output. Sends IP packet to next hop.  
    */

    /* IP PACKET */
    uint8_t mac_source[]      = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    uint8_t mac_destination[] = { 0x60, 0x6D, 0x67, 0xE2, 0xF9, 0x6E };
    send_ip_packet(fds, mac_destination, mac_source, TCP_PROTOCOL, "100.3.0.5", "80.1.1.2", 64);
    send_ip_packet(fds, mac_destination, mac_source, TCP_PROTOCOL, "80.1.2.3", "80.1.1.2", 1);

    /* ARP Request */
    uint8_t arp_source[]      = { 0x74, 0x2F, 0x13, 0x8B, 0x72, 0x69 }; // Device Interface A on network connected to tap0 
    uint8_t arp_dest[]        = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t *arp_packet       = construct_arp_packet(inet_addr("80.1.1.2"), inet_addr("80.1.0.1"), ARP_OP_REQUEST, arp_source, arp_dest);
    send_ethernet_frame(fds[1], arp_packet, ETHERNET_MIN_FRAME_LEN);
    
    /* If the program exits immediately after sending its frames, there is a
     * possibility the frames won't actually be delivered.  If, for example,
     * the "remote_vde_cmd" above is used, the user might not even finish
     * typing their password (which is accepted by a child process) before
     * this process terminates, which would result in send frames not actually
     * arriving.  Therefore, we pause and let the user manually end this
     * process. */

    printf("Press Control-C to terminate sender.\n");
    pause();

    return 0;
}

void 
send_ip_packet(int *fds, uint8_t *mac_dest, uint8_t *mac_source, uint8_t protocol,
               char *ip_source, char *ip_dest, int ttl)
{
    uint8_t  frame[1600];
    uint32_t fcs; 
    size_t   frame_len, ip_packet_len, frame_without_fcs_len, header_len;  

    Ethernet_Header ethernet_header = {
        .destination       = { mac_dest[0], mac_dest[1], mac_dest[2], 
                               mac_dest[3], mac_dest[4], mac_dest[5] },                              
        .source            = { mac_source[0], mac_source[1], mac_source[2], 
                               mac_source[3], mac_source[4], mac_source[5] }, 
        .type              = htons(0x800)
    };

    IP_Header ip_header             = {
        .version_and_IHL   = 0x45,
        .service_type      = 0x00,
        .total_length      = 0,
        .id                = htons(12345),
        .flags_and_offset  = htons(0x4000),
        .ttl               = ttl,
        .protocol          = protocol,
        .checksum          = 0,
        .source            = inet_addr(ip_source),
        .destination       = inet_addr(ip_dest)
    };

    uint8_t data[200] = "Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!"
                        "Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!"
                        "Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!Hello we are testing the data!!!";

    /* Replace total length and checksum. */
    header_len             = sizeof(ethernet_header) + sizeof(ip_header); 
    ip_packet_len          = sizeof(ip_header) + sizeof(data);
    ip_header.total_length = htons(ip_packet_len);
    ip_header.checksum     = RFC1071_checksum(&ip_header, sizeof(ip_header));

    /* Memcopy ethernet header, ip header, and ip packet payload. */
    memcpy(frame, &ethernet_header, sizeof(ethernet_header));
    memcpy(frame + sizeof(ethernet_header), &ip_header, sizeof(ip_header));
    memcpy(frame + header_len, &data, sizeof(data));

    /* Get fcs and memcopy. */
    frame_without_fcs_len = sizeof(ethernet_header) + ip_packet_len; 
    fcs                   = crc32(0, frame, frame_without_fcs_len);
    memcpy(frame + frame_without_fcs_len, &fcs, sizeof(fcs)); 

    /* Send packet*/   
    frame_len = frame_without_fcs_len + sizeof(fcs);
    printf("Sending ip frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
}
