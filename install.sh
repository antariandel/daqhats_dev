#!/bin/bash

if [ "$(id -u)" == "0" ]; then
   echo "This script must NOT be run as root. It's for installing within a Python virtualenv." 1>&2
   exit 1
fi

# Build / install the C library and headers
echo "Building and installing library"
echo
make -C lib all
make -C lib install
make -C lib clean

echo

# Build / install tools
echo "Building and installing tools"
echo
make -C tools all
make -C tools install
make -C tools clean

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
      pip2 install . --upgrade
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
      pip3 install . --upgrade
   fi
fi

echo

echo "Install complete"
