#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root, i.e 'sudo ./uninstall.sh'" 1>&2
   exit 1
fi

# Remove the C library and headers
echo "Removing shared library"
echo
make -C lib uninstall
echo

# Removed compiled examples
echo "Removing compiled examples"
echo
make -C examples/c clean
echo

# Remove tools
echo "Removing tools"
echo
make -C tools uninstall
echo

# Restore changes to SPI/I2C
if [ -e "/etc/mcc/hats/enabled_spi" ]; then
   echo "Disabling SPI"
   raspi-config nonint do_spi 1
fi
if [ -e "/etc/mcc/hats/enabled_i2c" ]; then
   echo "Disabling I2C"
   raspi-config nonint do_i2c 1
fi

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
