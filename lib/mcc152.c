/*
*   mcc152.c
*   author Measurement Computing Corp.
*   brief This file contains functions used with the MCC 152.
*
*   06/29/2018
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "daqhats.h"
#include "util.h"
#include "cJSON.h"
#include "gpio.h"
#include "mcc152_dac.h"
#include "mcc152_dio.h"

//*****************************************************************************
// Constants

#define NUM_AO_CHANNELS     2       // The number of analog output channels.
#define NUM_DIO_CHANNELS    8       // The number of digital I/O channels.

#define MAX_CODE            4095
#define LSB_SIZE            (5.0 / 4096.0)
#define MAX_VOLTAGE         (MAX_CODE * LSB_SIZE)

// The maximum size of the serial number string, plus NULL.
#define SERIAL_SIZE         (8+1)   

#define MIN(a, b)           ((a < b) ? a : b)
#define MAX(a, b)           ((a > b) ? a : b)

/// \cond
// Contains the device-specific data stored at the factory.
struct mcc152FactoryData
{
    // Serial number
    char serial[SERIAL_SIZE];
};

// Local data for each open MCC 152 board.
struct mcc152Device
{
    uint16_t handle_count;        // the number of handles open to this device
    //uint8_t last_command;
    //uint8_t output_port;
    //uint8_t direction;
    struct mcc152FactoryData factory_data;   // Factory data
};
/// \endcond

//*****************************************************************************
// Variables

static struct mcc152Device* _devices[MAX_NUMBER_HATS];
static bool _mcc152_lib_initialized = false;

//*****************************************************************************
// Local Functions

/******************************************************************************
  Validate parameters for an address
 *****************************************************************************/
static bool _check_addr(uint8_t address)
{
    if ((address >= MAX_NUMBER_HATS) ||     // Address is invalid
        !_mcc152_lib_initialized ||         // Library is not initialized
        (_devices[address] == NULL))        // Device structure is not allocated
    {
        return false;
    }
    else
    {
        return true;
    }
}


/******************************************************************************
  Sets an mcc152FactoryData to default values.
 *****************************************************************************/
static void _set_defaults(struct mcc152FactoryData* data)
{
    if (data)
    {
        strcpy(data->serial, "00000000");
    }
}

/******************************************************************************
  Parse the factory data JSON structure. Does not recurse so we can support 
  multiple processes.

  Expects a JSON structure like:

    {
        "serial": "00000000",
    }

  If it finds all of these keys it will return 1, otherwise will return 0.
 *****************************************************************************/
static int _parse_factory_data(cJSON* root, struct mcc152FactoryData* data)
{ 
    bool got_serial = false;
    cJSON* child;

    if (!data)
    {
        return 0;
    }
    
    // root should just have an object type and a child
    if ((root->type != cJSON_Object) ||
        (!root->child))
    {
        return 0;
    }
    
    child = root->child;
    
    // parse the structure
    while (child)
    {
        if (!strcmp(child->string, "serial") &&
            (child->type == cJSON_String) &&
            child->valuestring)
        {
            // Found the serial number
            strncpy(data->serial, child->valuestring, SERIAL_SIZE);
            got_serial = true;
        }
        child = child->next;
    }
    
    if (got_serial)
    {
        // Report success if all required items were found
        return 1;
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
  Perform any library initialization.
 *****************************************************************************/
static void _mcc152_lib_init(void)
{
    int i;
    
    if (!_mcc152_lib_initialized)
    {
        for (i = 0; i < MAX_NUMBER_HATS; i++)
        {
            _devices[i] = NULL;
        }
        
        _mcc152_lib_initialized = true;
    }
}

//*****************************************************************************
// Global Functions

/******************************************************************************
  Open a connection to the MCC 152 device at the specified address.
 *****************************************************************************/
int mcc152_open(uint8_t address)
{
    struct HatInfo info;
    char* custom_data;
    uint16_t custom_size;
    struct mcc152Device* dev;
    int result;
        
    _mcc152_lib_init();
    
    // validate the parameters
    if ((address >= MAX_NUMBER_HATS))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    if (_devices[address] == NULL)
    {
        // this is either the first time this device is being opened or it is
        // not a 152
    
        // read the EEPROM file(s), verify that it is an MCC 152, and get the
        // data
        if (_hat_info(address, &info, NULL, &custom_size) == RESULT_SUCCESS)
        {
            if (info.id == HAT_ID_MCC_152)
            {
                custom_data = malloc(custom_size);
                _hat_info(address, &info, custom_data, &custom_size);
            }
            else
            {
                return RESULT_BAD_PARAMETER;
            }
        }
        else
        {
            // no EEPROM info was found - allow opening the board with an
            // uninitialized EEPROM
            custom_size = 0;
        }
    
        // create a struct to hold device instance data
        _devices[address] = (struct mcc152Device*)calloc(1,
            sizeof(struct mcc152Device));
        dev = _devices[address];
        
        // initialize the struct elements
        dev->handle_count = 1;
        
        if (custom_size > 0) 
        {
            // convert the JSON custom data to parameters
            cJSON* root = cJSON_Parse(custom_data);
            if (!_parse_factory_data(root, &dev->factory_data))
            {
                // invalid custom data, use default values
                _set_defaults(&dev->factory_data);
            }
            cJSON_Delete(root);
            
            free(custom_data);
        }
        else
        {
            // use default parameters, board probably has an empty EEPROM.
            _set_defaults(&dev->factory_data);
        }

        // initialize the DAC
        if ((result = _mcc152_dac_init(address)) != RESULT_SUCCESS)
        {
            mcc152_close(address);
            return result;
        }
        
        // initialize the DIO
        if ((result = _mcc152_dio_init(address)) != RESULT_SUCCESS)
        {
            mcc152_close(address);
            return result;
        }
    }
    else
    {
        // the device has already been opened and initialized, increment
        // reference count
        dev = _devices[address];
        dev->handle_count++;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Check if an MCC 152is open.
 *****************************************************************************/
int mcc152_is_open(uint8_t address)
{
    if ((address >= MAX_NUMBER_HATS) ||
        (_devices[address] == NULL))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


/******************************************************************************
  Close a connection to an MCC 152 device and free allocated resources.
 *****************************************************************************/
int mcc152_close(uint8_t address)
{
    if (!_check_addr(address))
    {
        return RESULT_BAD_PARAMETER;
    }

    _devices[address]->handle_count--;
    if (_devices[address]->handle_count == 0)
    {
        free(_devices[address]);
        _devices[address] = NULL;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Return the number of analog output channels on the board.
 *****************************************************************************/
int mcc152_a_out_num_channels(void)
{
    return NUM_AO_CHANNELS;
}

int mcc152_a_out_code_max(void)
{
    return MAX_CODE;
}

int mcc152_a_out_code_min(void)
{
    return 0;
}

double mcc152_a_out_voltage_max(void)
{
    return MAX_VOLTAGE;
}

double mcc152_a_out_voltage_min(void)
{
    return 0.0;
}

/******************************************************************************
  Return the number of digital I/O channels on the board.
 *****************************************************************************/
int mcc152_dio_num_channels(void)
{
    return NUM_DIO_CHANNELS;
}

/******************************************************************************
  Read the serial number.
 *****************************************************************************/
int mcc152_serial(uint8_t address, char* buffer)
{
    // validate parameters
    if (!_check_addr(address) ||
        (buffer == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    strcpy(buffer, _devices[address]->factory_data.serial);
    return RESULT_SUCCESS;
}

/******************************************************************************
  Write an analog output channel.
 *****************************************************************************/
int mcc152_a_out_write(uint8_t address, uint8_t channel, uint32_t options,
    double value)
{
    uint16_t code;
    
    if (!_check_addr(address) ||
        (channel >= NUM_AO_CHANNELS))
    {
        return RESULT_BAD_PARAMETER;
    }

    if ((options & OPTS_NOSCALEDATA) == 0)
    {
        // user passed voltage
        if (value < 0.0)
        {
            value = 0.0;
        }
        else if (value > MAX_VOLTAGE)
        {
            value = MAX_VOLTAGE;
        }
        code = (uint16_t)((value / LSB_SIZE) + 0.5);
    }
    else
    {
        // user passed code
        if (value < 0.0)
        {
            value = 0.0;
        }
        else if (value > MAX_CODE)
        {
            value = MAX_CODE;
        }
        code = (uint16_t)(value + 0.5);
    }
    
    return _mcc152_dac_write(address, channel, code);
}

/******************************************************************************
  Write all analog output channels.
 *****************************************************************************/
int mcc152_a_out_write_all(uint8_t address, uint32_t options, double* values)
{
    uint16_t codes[NUM_AO_CHANNELS];
    int i;
    
    if (!_check_addr(address) ||
        (values == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }

    for (i = 0; i < NUM_AO_CHANNELS; i++)
    {
        if ((options & OPTS_NOSCALEDATA) == 0)
        {
            // user passed voltages
            if (values[i] < 0.0)
            {
                values[i] = 0.0;
            }
            else if (values[i] > MAX_VOLTAGE)
            {
                values[i] = MAX_VOLTAGE;
            }
            codes[i] = (uint16_t)((values[i] / LSB_SIZE) + 0.5);
        }
        else
        {
            // user passed codes
            if (values[i] < 0.0)
            {
                values[i] = 0.0;
            }
            else if (values[i] > MAX_CODE)
            {
                values[i] = MAX_CODE;
            }
            codes[i] = (uint16_t)(values[i] + 0.5);
        }
    }
    
    return _mcc152_dac_write_both(address, codes[0], codes[1]);
}

/******************************************************************************
  Reset DIO to default configuration.
 *****************************************************************************/
int mcc152_dio_reset(uint8_t address)
{
    int ret;

    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    

    // write the register values
    
    // interrupt mask
    ret = _mcc152_dio_reg_write(address, DIO_CMD_INT_MASK, DIO_CHANNEL_ALL,
        0xFF, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    // switch to inputs
    ret = _mcc152_dio_reg_write(address, DIO_CMD_CONFIG, DIO_CHANNEL_ALL, 0xFF,
        false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    // pull-up setting
    ret = _mcc152_dio_reg_write(address, DIO_CMD_PULL_SELECT, DIO_CHANNEL_ALL,
        0xFF, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    // pull-up enable
    ret = _mcc152_dio_reg_write(address, DIO_CMD_PULL_ENABLE, DIO_CHANNEL_ALL,
        0xFF, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    // input invert
    ret = _mcc152_dio_reg_write(address, DIO_CMD_POLARITY, DIO_CHANNEL_ALL, 0,
        false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }

    // input latch
    ret = _mcc152_dio_reg_write(address, DIO_CMD_INPUT_LATCH, DIO_CHANNEL_ALL,
        0, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    // output type
    ret = _mcc152_dio_reg_write(address, DIO_CMD_OUTPUT_CONFIG, DIO_CHANNEL_ALL,
        0, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }

    // output latch
    ret = _mcc152_dio_reg_write(address, DIO_CMD_OUTPUT_PORT, DIO_CHANNEL_ALL,
        0, false);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    return RESULT_SUCCESS;    
}

/******************************************************************************
  Read DIO input(s).
 *****************************************************************************/
int mcc152_dio_input_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_INPUT_PORT, channel, value);
}

/******************************************************************************
  Write DIO output(s).
 *****************************************************************************/
int mcc152_dio_output_write(uint8_t address, uint8_t channel, uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_OUTPUT_PORT, channel, value,
        true);
}

/******************************************************************************
  Read DIO output(s).
 *****************************************************************************/
int mcc152_dio_output_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_OUTPUT_PORT, channel, value);
}

/******************************************************************************
  Configure DIO direction.
 *****************************************************************************/
int mcc152_dio_direction_write(uint8_t address, uint8_t channel, uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_CONFIG, channel, value, true);
}

/******************************************************************************
  Read DIO direction.
 *****************************************************************************/
int mcc152_dio_direction_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_CONFIG, channel, value);
}

/******************************************************************************
  Configure DIO pull-up / pull-down resistor(s).
 *****************************************************************************/
int mcc152_dio_pull_config_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_PULL_SELECT, channel, value,
        false);
}

/******************************************************************************
  Read DIO pull-up / pull-down configuration.
 *****************************************************************************/
int mcc152_dio_pull_config_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_PULL_SELECT, channel, value);
}

/******************************************************************************
  Enable DIO pull-up / pull-down resistor(s).
 *****************************************************************************/
int mcc152_dio_pull_enable_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_PULL_ENABLE, channel, value, false);
}

/******************************************************************************
  Read DIO pull-up / pull-down enable setting.
 *****************************************************************************/
int mcc152_dio_pull_enable_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_PULL_ENABLE, channel, value);
}

/******************************************************************************
  Configure DIO input inversion.
 *****************************************************************************/
int mcc152_dio_input_invert_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_POLARITY, channel, value, false);
}

/******************************************************************************
  Read DIO input inversion configuration.
 *****************************************************************************/
int mcc152_dio_input_invert_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_POLARITY, channel, value);
}

/******************************************************************************
  Configure DIO input latching.
 *****************************************************************************/
int mcc152_dio_input_latch_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_INPUT_LATCH, channel, value,
        false);
}

/******************************************************************************
  Read DIO input latching configuration.
 *****************************************************************************/
int mcc152_dio_input_latch_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_INPUT_LATCH, channel, value);
}

/******************************************************************************
  Configure DIO output type.
 *****************************************************************************/
int mcc152_dio_output_type_write(uint8_t address, uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_OUTPUT_CONFIG, DIO_CHANNEL_ALL,
        value == 0 ? 0 : 1, false);
}

/******************************************************************************
  Read DIO output type.
 *****************************************************************************/
int mcc152_dio_output_type_read(uint8_t address, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_OUTPUT_CONFIG, DIO_CHANNEL_ALL,
        value);
}

/******************************************************************************
  Write DIO interrupt mask.
 *****************************************************************************/
int mcc152_dio_interrupt_mask_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, DIO_CMD_INT_MASK, channel, value, false);
}

/******************************************************************************
  Read DIO interrupt mask.
 *****************************************************************************/
int mcc152_dio_interrupt_mask_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_INT_MASK, channel, value);
}

/******************************************************************************
  Read DIO interrupt status.
 *****************************************************************************/
int mcc152_dio_interrupt_status_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, DIO_CMD_INT_STATUS, channel, value);
}
