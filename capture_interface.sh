#!/bin/sh

interface="$1"
ssh john@192.168.64.4 "sudo -S tshark -l -i tap$interface -w -" | wireshark -k -i -