#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root, i.e 'sudo ./uninstall.sh'" 1>&2
   exit 1
fi

# Remove the C library and headers
echo "Removing shared library"
echo
make -C lib uninstall

# Remove tools
echo "Removing tools"
echo
make -C tools uninstall

# Remove EEPROM images
echo "Removing EEPROM images"
rm -rf /etc/mcc/hats

echo

# Remove the Python packages
if [ -e "python2_files.txt" ]; then
   echo "Removing Python 2 package"
   cat python2_files.txt | xargs rm -rf
   rm python2_files.txt
   echo
fi

if [ -e "python3_files.txt" ]; then
   echo "Removing Python 3 package"
   cat python3_files.txt | xargs rm -rf
   rm python3_files.txt
   echo
fi

echo "Uninstall complete. Remove this folder to completely remove daqhats."
