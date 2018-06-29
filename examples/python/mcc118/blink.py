#!/usr/bin/env python
#
# MCC 118 example program
# Blink LED on each board 10 times
#
import sys
import daqhats as hats

# get hat list of MCC 118 boards
list = hats.hat_list(filter_by_id = hats.HatIDs.MCC_118)
if not list:
    print("No boards found")
    sys.exit()

# send the blink command to each board
for entry in list: 
    hats.mcc118(entry.address).blink_led(10)
    
