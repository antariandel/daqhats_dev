/**
*   @file mcc152.h
*   @author Measurement Computing Corp.
*   @brief This file contains definitions for the MCC 152.
*
*   06/29/2018
*/
#ifndef _MCC_152_H
#define _MCC_152_H

#include <stdint.h>

/// Write or read a value for all channels.
#define DIO_CHANNEL_ALL    (0xFF)   

#ifdef __cplusplus
extern "C" {
#endif

/**
*   @brief Open a connection to the MCC 152 device at the specified address.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode), 
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_open(uint8_t address);

/**
*   @brief Check if an MCC 152 is open.
*
*   @param address  The board address (0 - 7).
*   @return 1 if open, 0 if not open.
*/
int mcc152_is_open(uint8_t address);

/**
*   @brief Close a connection to an MCC 152 device and free allocated resources.
*
*   @param address  The board address (0 - 7).
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_close(uint8_t address);

/**
*   @brief Read the MCC 152 serial number
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param buffer   Pass a user-allocated buffer pointer to receive the serial
*       number as a string. The buffer must be at least 9 characters in length.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_serial(uint8_t address, char* buffer);

/**
*   @brief Return the number of analog output channels on the MCC 152.
*
*   @return The number of channels.
*/
int mcc152_a_out_num_channels(void);

/**
*   @brief Return the number of digital I/O on the MCC 152.
*
*   @return The number of I/O.
*/
int mcc152_dio_num_channels(void);

/**
*   @brief Perform a write to an analog output channel.
*
*   Updates the analog output channel in either volts or DAC code (set the
*   [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA) option to use DAC code.) The
*   voltage must be 0.0-5.0 and DAC code 0.0-4095.0.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The analog output channel number, 0 - 1.
*   @param options  Options bitmask
*   @param value    The analog output value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_a_out_write(uint8_t address, uint8_t channel, uint32_t options,
    double value);

/**
*   @brief Perform a write to all analog output channels simultaneously.
*
*   Update all analog output channels in either volts or DAC code (set the
*   [OPTS_NOSCALEDATA](@ref OPTS_NOSCALEDATA) option to use DAC code.) The
*   outputs will update at the same time.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param options  Options bitmask
*   @param values   The array of analog output values; there must be at least
*       2 values, but only the first two values will be used.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_a_out_write_all(uint8_t address, uint32_t options, double* values);


/**
*   @brief Reset the DIO to the default configuration.
*
*   Resets the DIO interface to the power on defaults:
*   - All channels input
*   - Output registers set to 1
*   - Input inversion disabled
*   - No input latching
*   - Pull-up resistors enabled
*   - All interrupts disabled
*   - Push-pull output type
*
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_reset(uint8_t address);

/**
*   @brief Read the DIO input(s).
*
*   Read a single digital channel input value or all inputs at once.  Will
*   return 0 or 1 in \b value if a single channel is specified, or an 8-bit
*   value representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL)
*   is specified.
*
*   If the specified channel is configured as an output this will return the
*   value present at the terminal.
*
*   This function reads the entire input register even if a single channel is
*   specified, so care must be taken when latched inputs are enabled. If a
*   latched input changes between input reads then changes back to its original
*   value, the next input read will report the change to the first value then
*   the following read will show the original value. If another input is read
*   then this input change could be missed so it is best to use
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) when using latched inputs.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the input value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_input_read(uint8_t address, uint8_t channel, uint8_t* value);

/**
*   @brief Write the DIO output(s).
*
*   Write a single digital channel output value or all outputs at once. Pass 0
*   or 1 if a single channel is specified, or an 8-bit value representing the
*   desired output for all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL)
*   is specified.
*
*   If the specified channel is configured as an input this will not have any
*   effect at the terminal, but allows the output register to be loaded before
*   configuring the channel as an output.
*
*   For example, to set channels 0 - 3 to 0 and channels 4 - 7 to 1 call:
*
*       mcc152_dio_output_write(address, DIO_CHANNEL_ALL, 0xF0);
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The output value(s).
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_output_write(uint8_t address, uint8_t channel, uint8_t value);

/**
*   @brief Read the DIO output register(s).
*
*   Read the value of a single digital channel output or all outputs at once.
*   Returns 0 or 1 if a single channel is specified, or an 8-bit value
*   representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is
*   specified.
*
*   This function returns the value stored in the output register. It may not
*   represent the value at the terminal if the channel is configured as input or
*   open-drain output.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the output value(s).
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_output_read(uint8_t address, uint8_t channel, uint8_t* value);

/**
*   @brief Set the DIO channel direction(s).
*
*   Set the direction of a single digital channel or all channels at once. A 0
*   sets the channel to output, a 1 sets it to input. Pass 0 or 1 if a single
*   channel is specified, or an 8-bit value representing all channels if
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   For example, to set channels 0 - 3 to output and channels 4 - 7 to input
*   call:
*
*       mcc152_dio_direction_write(address, DIO_CHANNEL_ALL, 0xF0);
*
*   When switching a channel from input to output the value that is in the
*   channel output register will be driven onto the terminal. This is set with
*   mcc152_dio_output_write().
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The direction value(s).
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_direction_write(uint8_t address, uint8_t channel, uint8_t value);

/**
*   @brief Read the DIO channel direction(s).
*
*   Reads the direction of a single digital channel or all channels at once. A 0
*   indicates the channel is set to output, a 1 indicates input. Returns 0 or 1
*   if a single channel is specified, or an 8-bit value representing all
*   channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the direction value(s).
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_direction_read(uint8_t address, uint8_t channel, uint8_t* value);

/**
*   @brief Configure the DIO pull-up / pull-down resistor(s).
*
*   Configure the pull-up / pull-down resistor for a single digital channel or
*   all channels at once. A 0 sets the resistor to pull-down, a 1 sets it to
*   pull-up. Pass 0 or 1 if a single channel is specified, or an 8-bit value
*   representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is
*   specified.
*
*   The pull resistor is enabled or disabled with
*   mcc152_dio_pull_enable_write().
*
*   For example, to configure and enable pull-down resistors on all channels
*   call:
*
*       mcc152_dio_pull_config_write(address, DIO_CHANNEL_ALL, 0x00);
*       mcc152_dio_pull_enable_write(address, DIO_CHANNEL_ALL, 0xFF);
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The pull-up/pull-down configuration.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_pull_config_write(uint8_t address, uint8_t channel,
    uint8_t value);

/**
*   @brief Read the DIO pull-up / pull-down resistor configuration.
*
*   Reads the pull-up / pull-down resistor configuration for a single digital
*   channel or all channels at once. A 0 indicates pull-down, a 1 indicates
*   pull-up. Returns 0 or 1 if a single channel is specified, or an 8-bit value
*   representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is
*   specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the pull-up/pull-down configuration.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_pull_config_read(uint8_t address, uint8_t channel,
    uint8_t* value);

/**
*   @brief Enable the DIO pull-up / pull-down resistor(s).
*
*   Enable or disable the pull-up / pull-down resistor for a single digital
*   channel or all channels at once. A 0 disables the resistor, a 1 enables it.
*   Pass 0 or 1 if a single channel is specified, or an 8-bit value representing
*   all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   The pull resistor is configured as pull-up or pull-down with
*   mcc152_dio_pull_config_write().
*
*   For example, to configure and enable pull-down resistors on all channels
*   call:
*
*       mcc152_dio_pull_config_write(address, DIO_CHANNEL_ALL, 0x00);
*       mcc152_dio_pull_enable_write(address, DIO_CHANNEL_ALL, 0xFF);
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The pull enable value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_pull_enable_write(uint8_t address, uint8_t channel,
    uint8_t value);

/**
*   @brief Read the DIO pull-up / pull-down resistor enable value.
*
*   Reads the pull-up / pull-down resistor enable value for a single digital
*   channel or all channels at once. A 0 indicates the resistor is disabled, a 1
*   indicates enabled. Returns 0 or 1 if a single channel is specified, or an
*   8-bit value representing all channels if
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the pull-up/pull-down enable value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_pull_enable_read(uint8_t address, uint8_t channel, uint8_t* value);

/**
*   @brief Configure the DIO input polarity inversion.
*
*   Configure input polarity inversion for a single digital channel or all
*   channels at once. A 0 sets the input to normal polarity, a 1 sets it to
*   inverted. Pass 0 or 1 if a single channel is specified, or an 8-bit value
*   representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is
*   specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The polarity inversion value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_input_invert_write(uint8_t address, uint8_t channel,
    uint8_t value);

/**
*   @brief Read the DIO input polarity inversion configuration.
*
*   Reads the current input polarity inversion configuration for a single
*   digital channel or all channels at once. A 0 represents normal polarity, 1
*   represents inverted.  Returns 0 or 1 if a single channel is specified, or an
*   8-bit value representing all channels in a single value if 
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the polarity inversion value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_input_invert_read(uint8_t address, uint8_t channel,
    uint8_t* value);

/**
*   @brief Configure the DIO input latching.
*
*   Configure input latching for a single digital channel or all channels at
*   once. When input latching is set to 0 the corresponding input state is not
*   latched, so reads show the current status of the input. A state change in
*   the corresponding input generates an interrupt (if it is not masked). A read
*   of the input clears the interrupt. If the input goes back to its initial
*   logic state before the input is read, then the interrupt is cleared.
*
*   When it is set to 1, the corresponding input state is latched. A change of
*   state of the input generates an interrupt and the input logic value is
*   loaded into the input port register. A read of the input will clear the
*   interrupt. If the input returns to its initial logic state before the input
*   is read, then the interrupt is not cleared and the input register keeps the
*   logic value that initiated the interrupt. The next read of the input will
*   show the initial state.
*
*   If the input terminal is changed from latched to non-latched input, a read
*   from the input reflects the current terminal logic level. If the input
*   terminal is changed from non-latched to latched input, the read from the
*   input represents the latched logic level.
*
*   Pass 0 or 1 if a single channel is specified, or an 8-bit value representing
*   all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The input latch value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_input_latch_write(uint8_t address, uint8_t channel,
    uint8_t value);

/**
*   @brief Read the DIO input latching configuration.
*
*   Read the input latching configuration for a single digital channel or all
*   channels at once. Returns 0 or 1 if a single channel is specified, or an
*   8-bit value representing all channels in a single value if 
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the input latch value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_input_latch_read(uint8_t address, uint8_t channel,
    uint8_t* value);

/**
*   @brief Configure the DIO output type.
*
*   Configure digital outputs as push-pull or open-drain. This is a single value
*   that affects all of the digital outputs on the MCC 152. Pass a 0 for
*   push-pull or a 1 for open-drain.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param value    The output type value, 0 or 1.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_output_type_write(uint8_t address, uint8_t value);

/**
*   @brief Read the DIO output type configuration.
*
*   Read the digital output type configuration.  Returns 0 for push-pull, 1 for
*   open-drain.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param value    Receives the output type value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_output_type_read(uint8_t address, uint8_t* value);

/**
*   @brief Write the DIO interrupt mask.
*
*   Configures the interrupt mask. A 1 disables (masks) the interrupt for the
*   specified channel, a 0 enables it. Pass 0 or 1 if a single channel is
*   specified, or an 8-bit value representing all channels if 
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   The current interrupt state may be read with hat_interrupt_state(). A user
*   program may wait for the interrupt to become active with
*   hat_wait_for_interrupt().  This allows the user to wait for a change on
*   one or more inputs without constantly reading the inputs. The interrupt is
*   cleared by reading the input(s) with mcc152_dio_input_read(). Multiple
*   MCC 152s will share a single interrupt signal, so the source of the 
*   interrupt may be determined by reading the interrupt status of each board
*   with mcc152_dio_interrupt_status() and all active interrupt sources must be
*   cleared before the interrupt will become inactive.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to write all channels at once.
*   @param value    The interrupt mask value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_interrupt_mask_write(uint8_t address, uint8_t channel,
    uint8_t value);

/**
*   @brief Read the DIO interrupt mask.
*
*   Reads the interrupt mask for a single digital channel or all channels at
*   once. A 0 indicates the interrupt is enabled, 1 indicates interrupt is
*   disabled. Returns 0 or 1 if a single channel is specified, or an 8-bit value
*   representing all channels if [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is
*   specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the interrupt mask value.
*   @return [Result code](@ref ResultCode),
*      [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_interrupt_mask_read(uint8_t address, uint8_t channel,
    uint8_t* value);

/**
*   @brief Read the DIO interrupt status.
*
*   Reads the interrupt status for a single digital channel or all channels at
*   once. A 0 indicates the channel is not the source of the interrupt, 1
*   indicates the channel was a source of the interrupt. Returns 0 or 1 if a
*   single channel is specified, or an 8-bit value representing all channels if
*   [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) is specified.
*
*   @param address  The board address (0 - 7). Board must already be opened.
*   @param channel  The DIO channel number, 0 - 7 or
*       [DIO_CHANNEL_ALL](@ref DIO_CHANNEL_ALL) to read all channels at once.
*   @param value    Receives the interrupt status value.
*   @return [Result code](@ref ResultCode),
*       [RESULT_SUCCESS](@ref RESULT_SUCCESS) if successful.
*/
int mcc152_dio_interrupt_status_read(uint8_t address, uint8_t channel,
    uint8_t* value);

#ifdef __cplusplus
}
#endif

#endif
