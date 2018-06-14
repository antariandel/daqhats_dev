/**
*   @file mcc134.h
*   @author Measurement Computing Corp.
*   @brief This file contains definitions for the MCC 134.
*
*   1 Feb 2018
*/
#ifndef _MCC_134_H
#define _MCC_134_H

#include <stdint.h>

// TC types from nist.c
/// Thermocouple type constants
enum tcTypes
{
    TC_TYPE_J = 0,      ///< J type
    TC_TYPE_K,          ///< K type
    TC_TYPE_T,          ///< T type
    TC_TYPE_E,          ///< E type
    TC_TYPE_R,          ///< R type
    TC_TYPE_S,          ///< S type
    TC_TYPE_B,          ///< B type
    TC_TYPE_N           ///< N type
};


#define OPEN_TC_VALUE           (-9999.0)   ///< Return value for an open thermocouple.
#define OVERRANGE_TC_VALUE      (-8888.0)   ///< Return value for thermocouple voltage outside the valid range.

#ifdef __cplusplus
extern "C" {
#endif

/**
*   @brief Open a connection to the MCC 134 device at the specified address.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_open(uint8_t address);

/**
*   @brief Check if an MCC 134 is open.
*
*   @param address  The board address (0 - 7).
*   @return 1 if open, 0 if not open.
*/
int mcc134_is_open(uint8_t address);

/**
*   @brief Close a connection to an MCC 134 device and free allocated resources.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_close(uint8_t address);

/**
*   @brief Read the MCC 134 serial number
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer   Pass a user-allocated buffer pointer to receive the serial number as a 
*       string.  The buffer must be at least 9 characters in length.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_serial(uint8_t address, char* buffer);

/**
*   @brief Read the MCC 134 calibration date
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer   Pass a user-allocated buffer pointer to receive the date as a 
*       string (format "YYYY-MM-DD").  The buffer must be at least 11 characters in length.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_calibration_date(uint8_t address, char* buffer);

/**
*   @brief Read the MCC 134 calibration coefficients for a single channel.
*
*   The coefficients are applied in the library as:
*
*       calibrated_ADC_code = (raw_ADC_code * slope) + offset
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The channel number (0 - 3).
*   @param slope    Receives the slope.
*   @param offset   Receives the offset.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_calibration_coefficient_read(uint8_t address, uint8_t channel, double* slope, double* offset);

/**
*   @brief Temporarily write the MCC 134 calibration coefficients for a single channel.
*
*   The user can apply their own calibration coefficients by writing to these values.  The values
*   will reset to the factory values from the EEPROM whenever mcc134_open() is called.
*
*   The coefficients are applied in the library as:
*
*       calibrated_ADC_code = (raw_ADC_code * slope) + offset
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The channel number (0 - 3).
*   @param slope    The new slope value.
*   @param offset   The new offset value.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_calibration_coefficient_write(uint8_t address, uint8_t channel, double slope, double offset);

/**
*   @brief Return the number of analog input channels on the MCC 134.
*
*   @return The number of channels.
*/
int mcc134_a_in_num_channels(void);

/**
*   @brief Clear any internal ADC offset error.
*
*   Performs an ADC self offset correction. This is automatically run when opening the board.
*   For absolute accuracy this should be peformed whenever the board temperature changes by more than
*   5 degrees C from the last time the offset error was cleared. The board temperature can be 
*   monitored with [mcc134_cjc_read()](@ref mcc134_cjc_read). This function takes approximately 
*   800ms to complete.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_self_offset_correction(uint8_t address);

/**
*   @brief Read an analog input channel.
*
*   Reads the specified channel and returns the value as voltage or ADC code. ADC code will be
*   returned if the [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA) option is specified; otherwise, 
*   voltage will be returned.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number (0 - 3).
*   @param options  Options bitmask (only [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA) and 
*       [OPTS_NOCALIBRATEDATA](@ref OPTS_NOCALIBRATEDATA) are used)
*   @param value    Receives the analog input value.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_a_in_read(uint8_t address, uint8_t channel, uint32_t options, double* value);

/**
*   @brief Write the thermocouple type for a channel.
*
*   Tells the MCC 134 library what thermocouple type is connected to the specified channel.
*   This is needed for correct temperature calculations. The type is one of [tcTypes](@ref tcTypes)
*   and the board will default to all channels set to [TC_TYPE_J](@ref TC_TYPE_J) when it is
*   first opened.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number (0 - 3).
*   @param type  The thermocouple type, one of [tcTypes](@ref tcTypes).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_tc_type_write(uint8_t address, uint8_t channel, uint8_t type);

/**
*   @brief Read the thermocouple type for a channel.
*
*   Reads the current thermocouple type for the specified channel. The type is one of 
*   [tcTypes](@ref tcTypes) and the board will default to all channels set to 
*   [TC_TYPE_J](@ref TC_TYPE_J) when it is first opened.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number (0 - 3).
*   @param type  Receives the thermocouple type, one of [tcTypes](@ref tcTypes).
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_tc_type_read(uint8_t address, uint8_t channel, uint8_t* type);

/**
*   @brief Read a temperature input channel.
*   
*   Reads the specified channel and returns the value as degrees Celsius. The returned temperature 
*   can have some special values to indicate abnormal conditions:
*       - [OPEN_TC_VALUE](@ref OPEN_TC_VALUE) if an open thermocouple is detected on the channel.
*       - [OVERRANGE_TC_VALUE](@ref OVERRANGE_TC_VALUE) if an overrange is detected on the channel.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number, 0 - 3.
*   @param temperature    Receives the temperature value in degrees C.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_t_in_read(uint8_t address, uint8_t channel, double* temperature);

/**
*   @brief Read the cold junction compensation temperature.
*   
*   Returns the temperature of the channel terminal for cold junction compensation.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog input channel number, 0 - 3.
*   @param cjc_temp   Receives the cold junction compensation temperature in degrees C.
*   @return [Result code](@ref ResultCode), [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc134_cjc_read(uint8_t address, uint8_t channel, double* cjc_temp);

#ifdef __cplusplus
}
#endif

#endif
