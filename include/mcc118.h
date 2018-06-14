/**
*   @file mcc118.h
*   @author Measurement Computing Corp.
*   @brief This file contains definitions for the MCC 118.
*
*   11/3/2017
*/
#ifndef _MCC_118_H
#define _MCC_118_H

#include <stdint.h>

/// Scan trigger input modes.
enum TriggerMode
{
    TRIG_RISING_EDGE    = 0,    ///< Trigger on a rising edge.
    TRIG_FALLING_EDGE   = 1,    ///< Trigger on a falling edge.
    TRIG_ACTIVE_HIGH    = 2,    ///< Trigger any time the signal is high.
    TRIG_ACTIVE_LOW     = 3     ///< Trigger any time the signal is low.
};

// Scan status bits
#define STATUS_HW_OVERRUN       (0x0001)    ///< A hardware overrun occurred.
#define STATUS_BUFFER_OVERRUN   (0x0002)    ///< A scan buffer overrun occurred.
#define STATUS_TRIGGERED        (0x0004)    ///< The trigger event occurred.
#define STATUS_RUNNING          (0x0008)    ///< The scan is running.


#ifdef __cplusplus
extern "C" {
#endif

/**
*   @brief Open a connection to the MCC 118 device at the specified address.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_open(uint8_t address);

/**
*   @brief Check if an MCC 118 is open.
*
*   @param address  The board address (0 - 7).
*   @return 1 if open, 0 if not open.
*/
int mcc118_is_open(uint8_t address);

/**
*   @brief Close a connection to an MCC 118 device and free allocated resources.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_close(uint8_t address);

/**
*   @brief Blink the LED on the MCC 118.
*
*   @param address  The board address (0 - 7).
*   @param count    The number of times to blink (0 - 255).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_blink_led(uint8_t address, uint8_t count);

/**
*   @brief Return the board firmware and bootloader versions.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param version  Receives the firmware version.  The version will be
*       in BCD hexadecimal with the high byte as the major version and low byte as minor,
*       i.e. 0x0103 is version 1.03.
*   @param boot_version  Receives the bootloader version.  The version
*       will be in BCD hexadecimal as above.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_firmware_version(uint8_t address, uint16_t* version, uint16_t* boot_version);

/**
*   @brief Read the MCC 118 serial number
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer   Pass a user-allocated buffer pointer to receive the serial number as a
*       string.  The buffer must be at least 9 characters in length.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_serial(uint8_t address, char* buffer);

/**
*   @brief Read the MCC 118 calibration date
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer   Pass a user-allocated buffer pointer to receive the date as a
*       string (format "YYYY-MM-DD").  The buffer must be at least 11 characters in length.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_calibration_date(uint8_t address, char* buffer);

/**
*   @brief Read the MCC 118 calibration coefficients for a single channel.
*
*   The coefficients are applied in the library as:
*
*       calibrated_ADC_code = (raw_ADC_code * slope) + offset
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The channel number (0 - 7).
*   @param slope    Receives the slope.
*   @param offset   Receives the offset.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_calibration_coefficient_read(uint8_t address, uint8_t channel, double* slope, double* offset);

/**
*   @brief Temporarily write the MCC 118 calibration coefficients for a single channel.
*
*   The user can apply their own calibration coefficients by writing to these values.  The values
*   will reset to the factory values from the EEPROM whenever mcc118_open() is called.  This
*   function will fail and return [RESULT_BUSY](@ref RESULT_BUSY) if a scan is running when it is called.
*
*   The coefficients are applied in the library as:
*
*       calibrated_ADC_code = (raw_ADC_code * slope) + offset
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The channel number (0 - 7).
*   @param slope    The new slope value.
*   @param offset   The new offset value.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_calibration_coefficient_write(uint8_t address, uint8_t channel, double slope, double offset);

/**
*   @brief Return the number of analog input channels on the MCC 118.
*
*   @return The number of channels.
*/
int mcc118_a_in_num_channels(void);

/**
*   @brief Perform a single reading of an analog input channel and return the value.
*
*   Will return [RESULT_BUSY](@ref RESULT_BUSY) if called while a scan is running.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number, 0 - 7.
*   @param options  Options bitmask (only [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA) and
*       [OPTS_NOCALIBRATEDATA](@ref OPTS_NOCALIBRATEDATA) are supported)
*   @param value    Receives the analog input value.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_a_in_read(uint8_t address, uint8_t channel, uint32_t options, double* value);

/**
*   @brief Set the trigger input mode.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param mode     One of the [trigger mode](@ref TriggerMode) values.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_trigger_mode(uint8_t address, uint8_t mode);

/**
*   @brief Read the actual sample rate per channel for a requested sample rate.
*
*   The internal scan clock is generated from a 16 MHz clock source so only discrete frequency steps
*   can be achieved.  This function will return the actual rate for a requested channel count, rate and burst mode setting.
*   This function does not perform any actions with a board, it simply calculates the rate.
*
*   @param channel_count    The number of channels in the scan.
*   @param sample_rate_per_channel   The desired sampling rate in samples per second per channel, max 100,000.
*   @param actual_sample_rate_per_channel   The actual sample rate that would occur when requesting this rate on an MCC 118, 
*       or 0 if there is an error.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful,
*       [RESULT_BAD_PARAMETER](@ref RESULT_BAD_PARAMETER) if the scan parameters are not achievable on an MCC 118.
*/
int mcc118_a_in_scan_actual_rate(uint8_t channel_count, double sample_rate_per_channel, double* actual_sample_rate_per_channel);

/**
*   @brief Start a hardware-paced analog input scan.
*
*   The scan runs as a separate thread from the user's code.  The function will allocate a
*   scan buffer and read data from the device into that buffer.  The user reads the data
*   from this buffer and the scan status using the [mcc118_a_in_scan_read()](@ref mcc118_a_in_scan_read)
*   function.  [mcc118_a_in_scan_stop()](@ref mcc118_a_in_scan_stop) is used to stop a
*   continuous scan, or to stop a finite scan before it completes.  The user must call
*   [mcc118_a_in_scan_cleanup()](@ref mcc118_a_in_scan_cleanup) after the scan has finished
*   and all desired data has been read; this frees all resources from the scan and allows
*   additional scans to be performed.
*
*   The buffer size will be allocated as follows:
*
*   \b Finite \b mode: Total number of samples in the scan
*
*   \b Continuous \b mode (buffer size is per channel): Either \b samples_per_channel
*   or the value in the following table, whichever is greater
*
*   \verbatim embed:rst:leading-asterisk
*   ==============      =========================
*   Sample Rate         Buffer Size (per channel)
*   ==============      =========================
*   Not specified       10 kS
*   0-100 S/s           1 kS
*   100-10k S/s         10 kS
*   10k-100k S/s        100 kS
*   ==============      =========================
*   \endverbatim
*
*   Specifying a very large value for \b samples_per_channel could use too much of the Raspberry Pi
*   memory.  If the memory allocation fails, the function will return [RESULT_RESOURCE_UNAVAIL](@ref RESULT_RESOURCE_UNAVAIL).
*   The allocation could succeed, but the lack of free memory could cause other problems in the Raspberry
*   Pi.  If you need to acquire a high number of samples then it is better to run the scan in
*   continuous mode and stop it when you have acquired the desired amount of data.  If a scan is
*   already running this function will return [RESULT_BUSY](@ref RESULT_BUSY).
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel_mask  A bit mask of the channels to be scanned. Set each bit to enable
*       the associated channel (0x01 - 0xFF.)
*   @param samples_per_channel  The number of samples to acquire for each channel in the scan.
*   @param sample_rate_per_channel   The sampling rate in samples per second per channel, max 100,000.
*       When using an external sample clock set this value to the maximum expected rate of the clock.
*   @param options  The options for the scan. This is a bitmask and may be an ORed combination of:
*       - [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA): Return ADC codes instead of voltage.
*       - [OPTS_NOCALIBRATEDATA](@ref OPTS_NOCALIBRATEDATA): Return uncalibrated values.
*       - [OPTS_EXTCLOCK](@ref OPTS_EXTCLOCK): Use an external sample clock on the CLK input.
*       - [OPTS_EXTTRIGGER](@ref OPTS_EXTTRIGGER): Use an external trigger source on the TRIG input.
*       - [OPTS_CONTINUOUS](@ref OPTS_CONTINUOUS): Scan until stopped (samples_per_channel is only used for buffer allocation.)
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful,
*       [RESULT_BUSY](@ref RESULT_BUSY) if a scan is already running.
*/
int mcc118_a_in_scan_start(uint8_t address, uint8_t channel_mask, uint32_t samples_per_channel,
    double sample_rate_per_channel, uint32_t options);

/**
*   @brief Returns the size of the internal scan data buffer.
*
*   An internal data buffer is allocated for the scan when mcc118_a_in_scan_start() is called.
*   This function returns the total size of that buffer in samples.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer_size_samples  Receives the size of the buffer in samples.  Each sample is a \b double.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful,
*       [RESULT_RESOURCE_UNAVAIL](@ref RESULT_RESOURCE_UNAVAIL) if a scan is not currently running
*       under this instance of the device, or [RESULT_BAD_PARAMETER](@ref RESULT_BAD_PARAMETER) if the address is invalid or
*       buffer_size_samples is NULL.
*/
int mcc118_a_in_scan_buffer_size(uint8_t address, uint32_t* buffer_size_samples);

/**
*   @brief Reads status and multiple samples from an analog input scan.
*
*   The scan is started with [mcc118_a_in_scan_start()](@ref mcc118_a_in_scan_start) and
*   runs in a background thread that reads the data from the board into an internal scan buffer.
*   This function reads the data from the scan buffer, and returns the current scan status.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param status   Receives the scan status, an ORed combination of the following flags:
*       - [STATUS_HW_OVERRUN](@ref STATUS_HW_OVERRUN): The device scan buffer was not read fast enough and data was lost.
*       - [STATUS_BUFFER_OVERRUN](@ref STATUS_BUFFER_OVERRUN): The thread scan buffer was not read by the user fast enough and data was lost.
*       - [STATUS_TRIGGERED](@ref STATUS_TRIGGERED): The trigger conditions have been met.
*       - [STATUS_RUNNING](@ref STATUS_RUNNING): The scan is running.
*   @param samples_per_channel  The number of samples per channel to read.  Specify \b -1 to read
*       all available samples, or \b 0 to only read the scan status and not return data.
*       If buffer does not contain enough space then the function will read as many samples
*       per channel as will fit in buffer.
*   @param timeout  The amount of time in seconds to wait for the samples to be read.  Specify a
*       negative number to wait indefinitely or \b 0 to return immediately with whatever samples are
*       available.
*   @param buffer   The buffer to read samples into.  May be NULL if samples_per_channel is \b 0.
*   @param buffer_size_samples  The size of the buffer in samples.  Each sample is a \b double.
*   @param samples_read_per_channel Returns the actual number of samples read from each channel.
*       May be \b NULL if samples_per_channel is \b 0.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful,
*       [RESULT_RESOURCE_UNAVAIL](@ref RESULT_RESOURCE_UNAVAIL) if a scan is not currently running
*       under this instance of the device.
*/
int mcc118_a_in_scan_read(uint8_t address, uint16_t* status, int32_t samples_per_channel, double timeout, double* buffer,
    uint32_t buffer_size_samples, uint32_t* samples_read_per_channel);

/**
*   @brief Stops an analog input scan.
*
*   The scan is stopped immediately.  The scan data that has been read into the scan buffer
*   is available until [mcc118_a_in_scan_cleanup()](@ref mcc118_a_in_scan_cleanup)
*   is called.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_a_in_scan_stop(uint8_t address);

/**
*   @brief Free analog input scan resources after the scan is complete.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_a_in_scan_cleanup(uint8_t address);

/**
*   @brief Return the number of channels in the current analog input scan.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @return The number of channels, 0 - 8.
*/
int mcc118_a_in_scan_channel_count(uint8_t address);

/**
*   @brief Test the CLK pin.
*
*   This function will return RESULT_BUSY if called while a scan is running.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param mode The CLK pin mode
*       - 0 = input
*       - 1 = output low
*       - 2 = output high
*       - 3 = output 1 kHz square wave
*   @param value   Receives the value at the CLK pin after setting the mode.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_test_clock(uint8_t address, uint8_t mode, uint8_t* value);

/**
*   @brief Test the TRIG pin by returning the current state.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param state    Receives the TRIG pin state (0 or 1.)
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc118_test_trigger(uint8_t address, uint8_t* state);

#ifdef __cplusplus
}
#endif

#endif
