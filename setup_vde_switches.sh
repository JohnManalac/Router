#!/usr/local/bin/bash

# Number of switches
NUM_SWITCHES=4

# Base name for files
CONTROL_FILE_BASE=/tmp/net

# Loop through switches and create interfaces
# Skip tap0; it will be created through setup_tap_ip.sh 

for ((i=1; i<$NUM_SWITCHES; i++)) 
do
    CONTROL_FILE="$CONTROL_FILE_BASE$i.vde"
    vde_switch -hub -sock "$CONTROL_FILE" -daemon
    sudo ifconfig tap$i create
    sudo ifconfig tap$i up
    sudo vde_plug2tap -s $CONTROL_FILE tap$i -daemon
done