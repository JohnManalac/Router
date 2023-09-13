/*
 * README.txt 
 *
 * Author: John Manalac
 * Date: May 22, 2023
 * 
 * Please check CHANGELOG.txt for changes to implementation. 
 *
 */

This is a simulation of a router connected to multiple networks. 

NETWORK TOPOLOGY: 

                                                                              ┌──────────────────────────────┐
                                                                              │                              │
                                                                              │         Interface A          │◄───┐
                                                                              │         IP: 80.1.1.2         │    │
                                                                              │    MAC: 74:2F:13:8B:72:69    │    │
                                                                              │                              │    │    ┌──────────────────────────────┐
                                                                              └──────────────────────────────┘    └────┤                              │───────────►┌──────────────────────────────┐
                                                                                                                       │         					  |            │                              │
																													   |		  NETWORK 0  		  |            │             tap0             │
																													   │							  |            │        IP: 80.1.0.5          │
                                                                                                                       │       IP: 80.1.0.0/16        │            │    MAC: 58:9C:FC:00:B2:20    │
                                                                              ┌──────────────────────────────┐         │    						  │            │                              │
                                                                              │                              │    ┌────┤                              │            └──────────────────────────────┘
                                                                              │         Interface B          │    │    └──────────────────────────────┘
                                                                              │         IP: 80.1.2.3         │    │                   ▲
                                                                              │    MAC: 09:BF:AB:CE:14:98    │◄───┘                   │
                                                                              │                              │                        │
                                                                              └──────────────────────────────┘                        │
                                                                                                                                      │
                                                                                                                                      │
                                                                                                                                      │
                                                                                                     ┌────────────────────────────────┴───────────────────────────────────┐
                                                                                                     │                                                                    │
┌──────────────────────────────┐                                                                     │                               R0_0                                 │
│                              │                                                                     │                           IP: 80.1.0.1                             │
│         Interface C          │◄───┐                                                                │                      MAC: 60:6D:67:E2:F9:6E                        │
│         IP: 90.2.4.5         │    │                                                                │                                                                    │
│    MAC: B1:07:56:E1:2C:9D    │    │                                                                │                                                                    │
│                              │    │             ┌──────────────────────────────┐                   │                                                                    │              ┌──────────────────────────────┐      ┌──────────────────────────────┐
└──────────────────────────────┘    └─────────────┤                              │                   │                                                                    │              │                              │      │                              │
                                                  │          NETWORK 1           │                   │          R0_1                                       R0_3           │              │          NETWORK 4           │      │         Interface I          │
                                                  │      				         │◄──────────────────┤      IP: 90.2.0.2           ROUTER 0           IP: 210.0.0.4       ├──────────────┤           				    ├─────►│        IP: 210.5.9.0         │
┌──────────────────────────────┐                  │    	  IP: 90.2.0.0/16	     │                   │ MAC: 60:6D:67:CA:7A:04                     MAC: 60:6D:67:52:61:EC  │              │   	 IP: 210.5.0.0/16       │      │    MAC: C1:42:13:EF:0C:F7    │
│                              │    ┌─────────────┤                              │                   │                                                                    │              │                              │      │                              │
│         Interface D          │    │             └──────────────────────────────┘                   │                                                                    │              └─────────────┬────────────────┘      └──────────────────────────────┘
│         IP: 90.2.6.7         │    │                                                                │                                                                    │                            │
│    MAC: 89:98:D0:8D:62:1A    │◄───┘                                                                │                               R0_2                                 │                            │
│                              │                                                                     │                          IP: 100.3.0.3                             │                            │
└──────────────────────────────┘                                                                     │                      MAC: 60:6D:67:A7:13:23                        │                            ▼
                                                                                                     │                                                                    │              ┌───────────────────────────────────────────┐
                                                                                                     └────────────────────────────────┬───────────────────────────────────┘              │                                           │
                                                                                                                                      │                                                  │           R2_0                            │
                                                                                                                                      │                                                  │      IP: 210.5.0.7                        │
                                                                                                                                      │                                                  │  MAC: 9D:0D:24:54:87:8C                   │
                                                                                                                                      │                                                  │                                           │
                                                                                                                                      │                                                  │                              ROUTER 2     │
                                                                                                                                      │                                                  │           R2_1                            │
                                                                                                                                      │                                                  │      IP: 250.6.0.8                        │
                                                                                                                                      │                                                  │  MAC: 9D:0D:24:11:3A:B7                   │
                                                                                                                                      │                                                  │                                           │
                                                                                                                                      │                                                  └──────────────┬────────────────────────────┘
                                                                             ┌──────────────────────────────┐                         │                                                                  │
                                                                             │                              │                         │                                                                  │
                                                                             │         Interface E          │◄───┐                    │                                                                  │
                                                                             │        IP: 100.3.8.9         │    │                    ▼                                                                  │
                                                                             │    MAC: 3F:D1:8C:31:98:AE    │    │   ┌───────────────────────────────┐                                                   ▼
                                                                             │                              │    │   │                               │                                   ┌──────────────────────────────┐     ┌──────────────────────────────┐
                                                                             └──────────────────────────────┘    └───┤           NETWORK 2           │                                   │                              │     │                              │
                                                                                                                     │     					         │                                   │          NETWORK 5           │     │         Interface J          │
                                                                                                                 ┌───┤       IP: 100.3.0.0/16	     │                                   │       					    ├────►│        IP: 250.6.7.8         │
                                                                             ┌──────────────────────────────┐    │   │                               │                                   │    	 IP: 250.6.0.0/16       │     │    MAC: 8F:5A:51:FC:D9:6C    │
                                                                             │                              │    │   └───────────────┬───────────────┘                                   │                              │     │                              │
                                                                             │         Interface F          │    │                   │                                                   └──────────────────────────────┘     └──────────────────────────────┘
                                                                             │        IP: 100.3.1.2         │    │                   │
                                                                             │    MAC: F7:BD:8F:07:71:ED    │◄───┘                   │
                                                                             │                              │                        │
                                                                             └──────────────────────────────┘                        │
                                                                                                                                     │
                                                                                                                                     ▼
                                                                                                                      ┌──────────────────────────────┐
                                                                                                                      │                              │
                                                                                                                      │             R1_0             │
                                                                                                                      │        IP: 100.3.0.5         │
                                                                                                                      │    MAC: D5:A1:BE:B7:84:36    │
                                                                                                                      │                              │
                                                                                                                      │                              │
                                                                                                                      │           ROUTER 1           │
                                                                                                                      │                              │
                                                                                                                      │             R1_1             │
                                                                                                                      │       IP: 160.4.0.6          │
                                                                                                                      │    MAC: D5:A1:BE:71:95:20    │
                                                                                                                      │                              │
                                                                                                                      └──────────────┬───────────────┘       ┌──────────────────────────────┐
                                                                                                                                     │                       │                              │
                                                                                                                                     │                       │         Interface G          │
                                                                                                                                     │                   ┌──►│        IP: 160.4.3.4         │
                                                                                                                                     ▼                   │   │    MAC: 1D:2E:06:CA:F0:9F    │
                                                                                                                      ┌──────────────────────────────┐   │   │                              │
                                                                                                                      │                              │   │   └──────────────────────────────┘
                                                                                                                      │          NETWORK 3           ├───┘
                                                                                                                      │      				         │
                                                                                                                      │    	  IP: 160.4.0.0/16	     ├───┐
                                                                                                                      │                              │   │   ┌──────────────────────────────┐
                                                                                                                      └──────────────────────────────┘   │   │                              │
                                                                                                                                                         │   │         Interface H          │
                                                                                                                                                         │   │        IP: 160.4.5.6         │
                                                                                                                                                         └──►│    MAC: 4B:23:27:7B:4C:BB    │
                                                                                                                                                             │                              │
                                                                                                                                                             └──────────────────────────────┘
INFORMATION 

    The ROUTER being simulated in this implementation is Router 0. The hardcoded 
    ROUTING TABLE will be based on this router and its four interfaces.

    NETWORKS are simply defined as Network 0, Network 1, Network 2 ...

    DEVICE interfaces on a network will be defined as Interface A, without prefixes. 

    ROUTER interfaces will be referred to as R1_0, with the prefix being the 
    specific router and the numeral following the underscore as the interface number. 

    Every simulated ROUTER 0 interface will be connected to a VDE switch (tap devices). 
    The tap0 switch will have its own IP address and will be used for TCP. 
    
    There are four interfaces for ROUTER 0: 
    
        R0_0, 
        R0_1, 
        R0_2, 
        R0_3

    The router has its own ARP cache. This cache is static and every interface directly 
    connected to the router interfaces will have their MAC address translations hardcoded. 
    Interfaceson other routers that are directly connected will also have their MAC address
    translations hardcoded. These hardcoded values can be found and modified in router.c.

    The simulated interfaces will be represented by a list of INTERFACES for 
    every router interface, defined as ROUTER_INTERFACES. Indexes in this list will 
    represent the corresponding interface for a particular router interface. Indexes
    correspond to the router interface, with ROUTER_INTERFACES[0] representing the 
    interface for R0_0, and ROUTER_INTERFACES[1] for R0_1, and so on. Every interface 
    will have its own IP address, MAC address, and fds for connecting and sending. 

    TCP: 

    For TCP, a tap0 device will be created with its own set IP through ./setup_tap_ip.sh. This 
    will be connected R0_0, which will have its ARP IP-MAC translation coded in the R0_0 ARP cache. 
    Through netcat, the user may connect to various other clients/servers and send data across each. 
    When a connection has been established or closed, a corresponding message will be printed. 

    There are various commands to use for this. /help to show all commands, /showall to show all 
    established connecitons, /switchto to switch to an established connection for sending data,
    and /close to close a connection. Note that closing a connection only closes and removes that 
    connection from our side; netcat still needs to close its own side of the connection! 
    
    The implementation uses various source and header files. These must be included: 

        Stack: 

            stack.c                 (main program)

        Router: 

            router.h                (router structs and constants)
            router.c                (hardcoded router constants) 
            router_functions.h      (router function prototypes)
            router_functions.c      (router function implementations)

        Ethernet: 

            ethernet.h              (ethernet structs and constants)
            ethernet_functions.h    (ethernet function prototypes)
            ethernet_functions.c    (ethernet function implementations)

        Internet Protocol (IP):

            ip.h                    (IP structs and constants)
            ip_functions.h          (IP function prototypes)
            ip_functions.c          (IP function implementations)

        Address Resolution Protocol (ARP): 

            arp.h                   (ARP structs and constants)
            arp_functions.h         (ARP function prototypes)
            arp_functions.c         (ARP function implementations)
        
        Internet Control Message Protocol (ICMP):

            icmp.h                  (ICMP structs and constants)
            icmp_functions.h        (ICMP function and diagnostics prototypes)
            icmp_functions.c        (ICMP function and diagnostics implementations)

        Transmission Control Protocol (TCP):

            tcp.h                   (TCP structs and constants)
            tcp_functions.h         (TCP function and diagnostics prototypes)
            tcp_functions.c         (TCP function and diagnostics implementations)

        Utilities: 

            c_headers.h             (all required C header files)
            util.h                  (diagnostic function prototypes) 
            util.c                  (diagnostic function implementations)
            cs431vde.h              (sending/receiving prototypes)
            cs431vde.c              (sending/receiving implementations)
            frame_crc32.h           (crc_check prototype)
            frame_crc32.c           (crc_check implementation)

        Switch setup and diagnostics: 

            frame_sender.c
            setup_tap_ip.sh
            setup_vde_switches.sh 
            capture_interface.sh 

    To use this implementation/simulation: 

        Make the main program: 

            gmake stack 

        Make the test program:

            gmake frame_sender 

        Start up the VDE switches by running the scripts:

            ./setup_tap_ip.sh

            ./setup_vde_switches.sh 

        For diagnostics, run the wireshark script before running stack and frame_sender:

            ./capture_interface.sh 0 

            (replace 0 with any interface number, from 0 to 3)