# C++ Console-Based Examples

## About
Console-based examples show how to perform continuous and finite scans, trigger an acquisition, 
and how to connect multiple DAQ HATs together to synchronously acquire data.
Console-based programs shipped are fully compiled.

### continuous_scan
The continuous_scan example continuously acquires blocks of analog input data for a user-specified group of channels until the 
acquisition is stopped by the user.  The last sample of data for each channel is displayed for each block of 
data received from the device.

### finite_scan
The finite_scan example acquires blocks of analog input data for a user-specified group of channels.  The last sample of data 
for each channel is displayed for each block of data received from the device.  The acquisition is stopped when the 
specified number of samples is acquired for each channel.
	
### finite_scan_with_trigger	   
The finite_scan_with_trigger example acquires waits for an external trigger to occur and then acquires blocks of analog input data for a user-specified group of channels.  The last sample of data for each channel is displayed for each block  of data received from the device.  The acquisition is stopped when  the specified number of samples is acquired for each channel.

### multi_hat_synchronous_scan
The multi_hat_synchronous_scan example gets synchronous data from multiple MCC118 HAT devices using the external clock and external trigger scan options.  
 - Stack the DAQ HAT boards onto the Pi using the instructions in [Installing the HAT board](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/hardware.html).
 - Wire the CLK terminals together on all MCC 118 HAT devices being used.
 - Connect an external trigger source to the TRIG terminals on all MCC 118 HAT devices being used 
 - Set the OPTS_EXTCLOCK scan option on all but one of the MCC 118 HAT devices.
 - Set the OPTS_EXTTRIGGER scan option on all MCC 118 HAT devices.

### Single_value_read	MCC118 Functions Demonstrated:
The Single_value_read example demonstrates acquiring data using a software timed loop to read a single value from each selected channel on each iteration of the loop.

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 