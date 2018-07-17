#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root, i.e 'sudo ./install.sh'" 1>&2
   exit 1
fi

# Build / install the C library and headers
echo "Buiding and installing library"
echo
make -C lib all
make -C lib install
make -C lib clean

# Build / install tools
make -C tools all
make -C tools install
make -C tools clean

# Build examples
make -C examples/c all

# Read HAT EEPROMs to /etc/mcc/hats
daqhats_read_eeproms

echo

# Install the Python package
if [ $(which python | wc -l) -ne 0 ]; then
   echo -n "Do you want to install support for Python 2? [y/n] "
   read input
   if [ "$input" == "y" ]; then
      echo "Installing library for Python 2"
      dpkg-query -l python-pip &> /dev/null
      if [ "$?" != "0" ]; then
         apt-get -qy install python-pip
      fi
      pip -q install . --upgrade
      touch ./python2_installed
   fi
fi

echo

if [ $(which python3 | wc -l) -ne 0 ]; then
   echo -n "Do you want to install support for Python 3? [y/n] "
   read input
   if [ "$input" == "y" ]; then
      echo "Installing library for Python 3"
      dpkg-query -l python3-pip &> /dev/null
      if [ "$?" != "0" ]; then
         apt-get -qy install python3-pip
      fi
      pip3 -q install . --upgrade
      touch ./python3_installed
   fi
fi

echo

echo "Install complete"
