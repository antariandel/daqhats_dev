# Multi-HAT Synchronous Scan Example

## About
This example demonstrates how to acquire synchronous data from multiple 
MCC 118 HAT devices using the external clock and external trigger analog 
input scan options. 

A table for each device displays the the samples read for each scan, the 
total count of samples read for the acquisition, and the current value of 
each active channel.

The acquisition is stopped when the specified number of samples are acquired, 
the Enter key is pressed, a read timeout error occurs, or an overrun error 
occurs. 

This example is compiled and ready-to-run, and can be customized to suit 
your needs.

## Configuring the MCC 118 HATs for a Synchronous Acquisition
Perform the following procedure to configure a stack of MCC 118 HATs to 
synchronously acquire data.

> The board stack can contain up to eight MCC 118 HATs.

1. Stack the MCC 118 HATs onto the Rasberry Pi; refer to 
[Installing the HAT board](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/hardware.html)
for instructions.</li>
2. Wire the CLK terminals together on each MCC 118 HAT board.</li>
3. Connect an external trigger source to the TRIG terminal on each MCC 118 
HAT board.</li>
4. Set the OPTS_EXTCLOCK scan option on all but one of the MCC 118 
HAT boards.

   The internal clock on the MCC 118 for which OPTS_EXTCLOCK is *not* set
   is used to pace the synchronous acquisition.
</li>
5. Set the OPTS_EXTTRIGGER scan option on all MCC 118 HAT boards.</li></ol>

> Refer to 
[mcc118_a_in_scan_start()](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/c.html#c.mcc118_a_in_scan_start) 
for information about the OPTS_EXTCLOCK and OPTS_EXTTRIGGER scan options.

## Running the example
To run the example, open a terminal window and enter the following commands:
```
   cd ~/daqhats/examples/c/mcc118/multi-hat_synchronous_scan
   ./multi-hat_synchronous_scan
```

>   **Note**: The example may need to be run with sudo.

## Support/Feedback
Contact technical support through our 
[support page](https://www.mccdaq.com/support/support_form.aspx).