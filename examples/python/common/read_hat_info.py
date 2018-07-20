#!/usr/bin/env python
"""
MCC HAT example program
"""
from __future__ import print_function
import daqhats as hats

def main():
    """ Main function """
    mylist = hats.hat_list()

    for entry in mylist:
        print("Address: " + str(entry.address))
        print("ID: 0x{:04X}".format(entry.id))
        print("Name: " + entry.product_name)
        print("HW version: " + str(entry.version))

        if entry.id == hats.HatIDs.MCC_118:
            board = hats.mcc118(entry.address)
            serial = board.serial()
            print("Serial: " + serial)
            fw_versions = board.firmware_version()
            print("FW version: " + fw_versions.version)
            print("Bootloader version: " + fw_versions.bootloader_version)
        print("")

if __name__ == '__main__':
    main()
