#!/usr/bin/env python
"""
MCC 118 example program
Blink LED on each board 10 times
"""
from __future__ import print_function
import sys
import daqhats as hats

def main():
    """ Main function """

    # get hat list of MCC 118 boards
    mylist = hats.hat_list(filter_by_id=hats.HatIDs.MCC_118)
    if not mylist:
        print("No boards found")
        sys.exit()

    # send the blink command to each board
    for entry in mylist:
        hats.mcc118(entry.address).blink_led(10)

if __name__ == '__main__':
    main()
