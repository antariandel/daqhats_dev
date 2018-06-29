# MCC DAQ HAT Library
<table>
	<tr><td>Info<td>Contains C and Python Libraries for interacting with Measurement Computing DAQ HAT boards.
	<tr><td>Author<td>Measurement Computing
</table>

## About
This is the development repository for Measurement Computing DAQ HAT boards. The **daqhats** library was created and is supported by Measurement Computing Corporation (MCC).

## Prerequisites
- Raspbian or Raspbian Lite image (may work with other Raspberry Pi operating systems)
- Raspberry Pi A+, B+, 2, or 3
- C, C++, Python 2.7 or Python 3.4

## Raspberry Pi Configuration
Follow the instructions at https://www.raspberrypi.org/help/ for setting up a Raspberry Pi.

## Build Instructions
1. Power off the Raspberry Pi and attach one or more DAQ HAT boards, using unique address settings for each. Refer to [Installing the HAT board](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/hardware.html) for detailed information.
   When using a single board, leave it at address 0 (all address jumpers removed.) One board must always be at address 0 to ensure that the OS reads a HAT EEPROM and initializes the hardware correctly.
2. Power on the Pi, log in, and open a terminal window (if using the graphical interface.)
3. Update your installation packages and install git (if not installed):

   ```
   sudo apt-get update
   sudo apt-get install git
   ```
4. Download the daqhats library to the root of your home folder:

   ```
   cd ~
   git clone https://github.com/nwright98/daqhats
   ```
5. Build and install the shared library, tools, and optional Python support. The installer will ask if you want to install Python 2 and Python 3 support. It will also detect the HAT board EEPROMs and save the contents, if needed.

   ```
   cd ~/daqhats
   sudo ./install.sh
   ```   
6. [Optional] Use the firmware update tool to update the firmware on your MCC HAT board. The "0" in the example below is the board address. The line with the "-b" option updates the bootloader. Repeat the two commands for each HAT address in your board stack. This example demonstrates how to update the firmware on the MCC 118 HAT that is installed at address 0.

   ```
   mcc118_firmware_update -b 0 ~/daqhats/tools/MCC_118.hex
   mcc118_firmware_update 0 ~/daqhats/tools/MCC_118.hex
   ```
You can now run the example programs under ~/daqhats/examples and create your own programs. Refer to the [Examples](#examples) section below for more information.

#### Update the EEPROM images
If you change your board stack, you must update the saved EEPROM images so that the library has the correct board information:

```
sudo daqhats_read_eeproms
```
#### Uninstall the daqhats library
If you want to uninstall the the daqhats library, use the following commands:
```
cd ~/daqhats
sudo ./uninstall.sh
```

## Examples
The daqhats library includes common and board-specific example programs developed with C/C++ and Python. The examples are available under ~/daqhats/examples, and are provided in the following formats:

- console-based (C/C++)
- User interface
  - Web server (Python)
  - IFTTT (If This Then That) web service (Python)
  - CodeBlocks (C/C++)

Refer to the README.md file in each example folder for more information.

## Usage
The following is a basic Python example demonstrating how to read MCC 118 voltage inputs and display channel values.
```
	#!/usr/bin/env python
	#
	# MCC 118 example program
	# Read and display analog input values
	#
	import sys
	import daqhats as hats

	# get hat list of MCC HAT boards
	list = hats.hat_list(filter_by_id = hats.HatIDs.ANY)
	if not list:
		print("No boards found")
		sys.exit()

	# Read and display every channel
	for entry in list: 
    if entry.id == hats.HatIDs.MCC_118:
        print("Board {}: MCC 118".format(entry.address))
        board = hats.mcc118(entry.address)
        for channel in range(board.a_in_num_channels()):
            value = board.a_in_read(channel)
            print("Ch {0}: {1:.3f}".format(channel, value))	
```
	
## Support/Feedback
The **daqhats** library is supported by MCC. Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx).

## Documentation 
Documentation for the daqhats library is available at [mccdaq.com](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/).
