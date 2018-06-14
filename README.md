# MCC HAT Development
<table>
	<tr><td>Info<td>Contains C and Python Libraries for interacting with Measurement Computing HAT boards.
	<tr><td>Author<td>Measurement Computing
</table>

## About
This is the development repository for Measurement Computing HAT boards. The **daqhats** package was created and is supported by Measurement Computing Corporation (MCC).

## Prerequisites
- Rasbian or Rasbian Lite image
- Raspberry Pi A+, B+, 2, and 3
- C, C++, Python 2.7 or Python 3.4
- *other requirements?*

## Raspberry Pi Configuration
Follow the instructions at https://www.raspberrypi.org/help/ for setting up a Raspberry Pi.

## Build Instructions
1. Power off the Raspberry Pi and attach one or more HAT boards, using unique address settings for each. Refer to [Installing the HAT board](https://nwright98.github.io/daqhats/hardware.html) for detailed information. (*update link to mccdaq repo when available*)   
   When using a single board, leave it at address 0 (all address jumpers removed.) One board must always be at address 0 to ensure that the OS reads a HAT EEPROM and initializes the hardware correctly.
2. Power on the Pi, log in, and open a terminal window (if using the graphical interface.)
3. Update your installation packages and install git (if not installed):

   ```
   sudo apt-get update
   sudo apt-get install git
   ```
4. Download the daqhats package to the root of your home folder:

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

#### Uninstall the daqhats Package

```
cd ~/daqhats
sudo ./uninstall.sh
```
#### Update the EEPROM
If you change your board stack, you must update the saved EEPROM images so that the library has the correct board information:

```
sudo daqhats_read_eeproms
```

## Examples
The daqhats package provides example programs in C and Python for each HAT board under ~/daqhats/examples. Some examples are ready-to-run and others will need to be built. The example programs are available in the following formats:

- console/terminal
- GUI
- Python web server
- IFTTT (If This Then That) web server

Refer to the Examples README for information about the MCC HAT examples that are available (*insert readme link when available*).

## Usage
The following is a basic Python example demonstrating how to read and display analog data from the MCC 118 voltage measurement board.
```
    #!/usr/bin/env python
    #
    # Read and display analog input values
    #
    import time
    import daqhats as hats

    # get hat list of MCC HAT boards
    list = hats.hat_list(filter_by_id = hats.HatIDs.MCC_118)
    if not list:
        print("No boards found")
        sys.exit()

    # Read and display every channel from the first board
    board = hats.mcc118(list[0]['address'])
    print("Board {}:".format(entry['address']))
    for channel in range(board.a_in_num_channels()):
        value = board.a_in_read(channel)
        print("  Ch {0}: {1:.3f}".format(channel, value))
```
	
## Support/Feedback
The **daqhats** package is supported by MCC. Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 

## Documentation 
Documentation for the daqhats package is available at https://nwright98.github.io/daqhats/ (*replace with mccdaq/daqhats link when available*). 
