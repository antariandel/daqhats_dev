# C/C++ Console-Based Examples

## About
The C/C++ console-based examples demonstrate various scan modes and trigger options to familiarize yourself
with the daqhats library and MCC 118. All examples are shipped fully compiled and ready-to-run, and can
be customized to suit your needs.

## Example Programs
- **single_value_read**: acquire data from user-specified channels using a software-timed loop. On each loop iteration a single value is read from each channel.

- **continuous_scan**: continuously acquire analog input data from user-specified channels until the scan is stopped. The last sample of data for each channel is displayed for each block of data received from the device.

- **finite_scan**: acquire analog input data from user-specified channels. The last sample of data for each channel is displayed for each block of data received from the device. The acquisition stops when the specified number of samples is acquired for each channel.

- **finite_scan_with_trigger**: acquire analog input data from user-specified channels after an external trigger occurs. The last sample of data for each channel is displayed for each block of data received from the device. The acquisition stops when the specified number of samples is acquired for each channel.

- **multi_hat_synchronous_scan**: acquire synchronous data from multiple MCC 118 HATs using the external clock and external trigger scan options. 
To configure multiple MCC 118 HATs to synchronously acquire data:
  * Stack the DAQ HAT boards onto the Pi using the instructions in [Installing the HAT board](https://www.mccdaq.com/PDFs/Manuals/DAQ-HAT/hardware.html).</li>
  * Wire the CLK terminals together on each MCC 118 HAT device.</li>
  * Connect an external trigger source to the TRIG terminal on each MCC 118 HAT device.</li>
  * Set the OPTS_EXTCLOCK scan option on all but one of the MCC 118 HAT devices.</li>
  * Set the OPTS_EXTTRIGGER scan option on all MCC 118 HAT devices.</li></ol>

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx).