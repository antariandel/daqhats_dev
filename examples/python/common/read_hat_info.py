#!/usr/bin/env python
#
# MCC HAT example program
#
import sys
import daqhats as hats

my_list = hats.hat_list()

for entry in my_list:
    print("Address: " + str(entry["address"]))
    print("ID: 0x{:04X}".format(entry["id"]))
    print("Name: " + entry["product_name"])
    print("HW version: " + str(entry["version"]))

    if entry["id"] == hats.HatIDs.MCC_118:
        board = hats.mcc118(entry["address"])
        serial = board.serial()
        print("Serial: " + serial)
        fw_versions = board.firmware_version()
        print("FW version: " + fw_versions["version"])
        print("Bootloader version: " + fw_versions["bootloader_version"])
    print("")
