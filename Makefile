CFLAGS=-Wall -pedantic -g

stack: stack.o cs431vde.o util.o frame_crc32.o router.o router_functions.o ethernet_functions.o ip_functions.o arp_functions.o icmp_functions.o tcp_functions.o
	gcc -o $@ $^

frame_sender: frame_sender.o cs431vde.o util.o frame_crc32.o router.o router_functions.o ethernet_functions.o ip_functions.o arp_functions.o icmp_functions.o 
	gcc -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o *.core stack frame_sender