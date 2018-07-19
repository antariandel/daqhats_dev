# Common MCC DAQ HAT Examples

## About
These Python console examples can be run with any MCC DAQ HAT board.

## Example Programs
- **list_boards.py**: Detects the MCC HATs connected to a Raspberry Pi, 
and displays the board address, type, hardware version, product name, 
firmware version, and bootloader version for each device.

- **read_values.py**: continuously acquires analog input data from each channel
on each detected MCC HAT device. The board address, product ID, and channel 
values display. Press **Ctrl+C** to stop the acquisition.

## Running the example
To run an example, open a terminal window and enter the following commands:
```
   cd ~/daqhats/examples/python/common
   ./<example_name>
```

>   **Note**: The example may need to be run with sudo.

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx).