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
#include <fcntl.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include "daqhats.h"
#include "util.h"
#include "cJSON.h"
#include "gpio.h"

//*****************************************************************************
// Constants

#define NUM_AO_CHANNELS     2       // The number of analog output channels.
#define NUM_DIO_CHANNELS    8       // The number of digital I/O channels.

#define MAX_CODE            4095
#define VOLTAGE_RANGE       5.0

// The maximum size of the serial number string, plus NULL.
#define SERIAL_SIZE         (8+1)   

#define I2C_BASE_ADDR       (0x20)

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
    uint8_t last_command;
    uint8_t output_port;
    uint8_t direction;
    struct mcc152FactoryData factory_data;   // Factory data
};
/// \endcond

static const char* const spi_device = SPI_DEVICE_1; // the spidev device
static const enum SpiBus spi_bus_num = SPI_BUS_1;   // for obtain_lock()
static const uint8_t spi_mode = SPI_MODE_1;         // use mode 1
                                                    // (CPOL=0, CPHA=1)
static const uint8_t spi_bits = 8;                  // 8 bits per transfer
static const uint32_t spi_rate = 50000000;          // maximum SPI clock
                                                    // frequency
static const uint16_t spi_delay = 0;                // delay in us before
                                                    // removing CS

#define CMD_INPUT_PORT          0x00
#define CMD_OUTPUT_PORT         0x01
#define CMD_POLARITY            0x02
#define CMD_CONFIG              0x03
#define CMD_OUTPUT_STRENGTH_0   0x40
#define CMD_OUTPUT_STRENGTH_1   0x41
#define CMD_INPUT_LATCH         0x42
#define CMD_PULL_ENABLE         0x43
#define CMD_PULL_SELECT         0x44
#define CMD_INT_MASK            0x45
#define CMD_INT_STATUS          0x46
#define CMD_OUTPUT_CONFIG       0x4F

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
  Perform a SPI transfer to the DAC
 *****************************************************************************/
int _mcc152_spi_transfer(uint8_t address, void* tx_data, uint8_t data_count)
{
    int lock_fd;
    int spi_fd;
    uint8_t temp;
    int ret;

    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    

    // open the SPI device handle
    spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0)
    {
        return RESULT_RESOURCE_UNAVAIL;
    }

    // Obtain a spi lock
    if ((lock_fd = _obtain_spi_lock(spi_bus_num)) < 0)
    {
        // could not get a lock within 5 seconds, report as a timeout
        close(spi_fd);
        return RESULT_LOCK_TIMEOUT;
    }
    
    _set_address(address);
    
    // check spi mode and change if necessary
    ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &temp);
    if (ret == -1)
    {
        _release_lock(lock_fd);
        close(spi_fd);
        return RESULT_UNDEFINED;
    }
    if (temp != spi_mode)
    {
        ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
        if (ret == -1)
        {
            _release_lock(lock_fd);
            close(spi_fd);
            return RESULT_UNDEFINED;
        }
    }
    
    // Init the spi ioctl structure
    struct spi_ioc_transfer tr = {
        .tx_buf = (uintptr_t)tx_data,
        .rx_buf = (uintptr_t)NULL,
        .len = data_count,
        .delay_usecs = spi_delay,
        .speed_hz = spi_rate,
        .bits_per_word = spi_bits,
    };

    if ((ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr)) < 1)
    {
        ret = RESULT_UNDEFINED;
    }
    else
    {
        ret = RESULT_SUCCESS;
    }
    
    // clear the SPI lock
    _release_lock(lock_fd);

    close(spi_fd);
    
    return ret;
}

static inline int _write_byte_data(int file, uint8_t command, uint8_t value)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;

	data.byte = value;

	args.read_write = 0;
	args.command = command;
	args.size = 2;
	args.data = &data;
	return ioctl(file, I2C_SMBUS, &args);
}

static inline int _read_byte_data(int file, uint8_t command)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;

	args.read_write = 1;
	args.command = command;
	args.size = 2;
	args.data = &data;
	if (ioctl(file, I2C_SMBUS, &args))
    {
        return -1;
    }
    else
    {
        return data.byte & 0xFF;
    }
}

static inline int _read_byte(int file)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;

	args.read_write = 1;
	args.command = 0;
	args.size = 1;
	args.data = &data;
	if (ioctl(file, I2C_SMBUS, &args))
    {
        return -1;
    }
    else
    {
        return data.byte & 0xFF;
    }
}

/******************************************************************************
  Perform an I2C write to the IO expander
 *****************************************************************************/
int _mcc152_i2c_write(uint8_t address, uint8_t command, uint8_t value)
{
    int i2c_fd;
    uint8_t addr;
    int ret;

    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    
    addr = I2C_BASE_ADDR + address;

    // open the I2C device handle
    i2c_fd = open(I2C_DEVICE_1, O_RDWR);
    if (i2c_fd < 0)
    {
        return RESULT_RESOURCE_UNAVAIL;
    }

    // set slave address
    ret = ioctl(i2c_fd, I2C_SLAVE, addr);
    if (ret == -1)
    {
        close(i2c_fd);
        return RESULT_UNDEFINED;
    }
    
    // write the value
	ret = _write_byte_data(i2c_fd, command, value);    
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
    }
    else
    {
        ret = RESULT_SUCCESS;
    }
    
    _devices[address]->last_command = command;
    close(i2c_fd);
    
    return ret;
}

/******************************************************************************
  Perform an I2C read from the IO expander
 *****************************************************************************/
int _mcc152_i2c_read(uint8_t address, uint8_t command, uint8_t* value)
{
    int i2c_fd;
    uint8_t addr;
    int ret;

    if (!_check_addr(address) ||                // check address failed
        (value == NULL))                        // bad pointer
    {
        return RESULT_BAD_PARAMETER;
    }    
    addr = I2C_BASE_ADDR + address;

    // open the I2C device handle
    i2c_fd = open(I2C_DEVICE_1, O_RDWR);
    if (i2c_fd < 0)
    {
        return RESULT_RESOURCE_UNAVAIL;
    }

    // set the slave address
    ret = ioctl(i2c_fd, I2C_SLAVE, addr);
    if (ret == -1)
    {
        close(i2c_fd);
        return RESULT_UNDEFINED;
    }
    
    // If the command has not changed since the last transfer then just perform
    // a read byte; otherwise, perform read byte data which writes the command
    // to the I/O expander.
    if (command == _devices[address]->last_command)
    {
        ret = _read_byte(i2c_fd);
    }
    else
    {
        // read from the device
        ret = _read_byte_data(i2c_fd, command);
    }
    
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
    }
    else
    {
        *value = (uint8_t)ret;
        ret = RESULT_SUCCESS;
        _devices[address]->last_command = command;
    }
    
    close(i2c_fd);
    
    return ret;
}

static int _mcc152_dio_reg_write(uint8_t address, uint8_t command,
    uint8_t channel, uint8_t value, bool use_cache)
{
    int ret;
    uint8_t reg_value;
    
    if (!_check_addr(address) ||                // check address failed
        // bad channel
        ((channel > NUM_DIO_CHANNELS) && (channel != DIO_CHANNEL_ALL))) 
    {
        return RESULT_BAD_PARAMETER;
    }    

    if (channel == DIO_CHANNEL_ALL)
    {
        reg_value = value;
    }
    else
    {
        value &= 0x01;      // force to single bit
        if (use_cache)
        {
            switch (command)
            {
            case CMD_OUTPUT_PORT:
                reg_value = _devices[address]->output_port;
                break;
            case CMD_CONFIG:
                reg_value = _devices[address]->direction;
                break;
            default:
                // no cache available
                _mcc152_i2c_read(address, command, &reg_value);
                break;
            }
        }
        else
        {
            _mcc152_i2c_read(address, command, &reg_value);
        }
        reg_value = (reg_value & ~(1 << channel)) | (value << channel);
    }
    
    ret = _mcc152_i2c_write(address, command, reg_value);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }   

    // update cache
    switch (command)
    {
    case CMD_OUTPUT_PORT:
        _devices[address]->output_port = reg_value;
        break;
    case CMD_CONFIG:
        _devices[address]->direction = reg_value;
        break;
    default:
        // no cache available
        break;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Read I/O expander register.
 *****************************************************************************/
static int _mcc152_dio_reg_read(uint8_t address, uint8_t command,
    uint8_t channel, uint8_t* value)
{
    int ret;
    uint8_t reg_value;
    
    if (!_check_addr(address) ||                // check address failed
        // bad channel        
        ((channel > NUM_DIO_CHANNELS) && (channel != DIO_CHANNEL_ALL)) ||
        (value == NULL))                        // bad pointer
    {
        return RESULT_BAD_PARAMETER;
    }    

    ret = _mcc152_i2c_read(address, command, &reg_value);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }   

    // update cache
    switch (command)
    {
    case CMD_OUTPUT_PORT:
        _devices[address]->output_port = reg_value;
        break;
    case CMD_CONFIG:
        _devices[address]->direction = reg_value;
        break;
    default:
        // no cache available
        break;
    }
    
    if (channel == DIO_CHANNEL_ALL)
    {
        *value = reg_value;
    }
    else
    {
        *value = (reg_value >> channel) & 0x01;
    }
    
    return RESULT_SUCCESS;
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

static int _mcc152_dac_write(int address, uint16_t code)
{
    if (!_check_addr(address) ||                // check address failed
        (code > MAX_CODE))                      // bad pointer
    {
        return RESULT_BAD_PARAMETER;
    }    
    return RESULT_SUCCESS;
}

static int _mcc152_dac_init(int address)
{
    _mcc152_dac_write(address, 0);
    return RESULT_SUCCESS;
}

static int _mcc152_dio_init(int address)
{
    int ret;
    
    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }
    
    // read the registers that have local cache
    ret = _mcc152_i2c_read(address, CMD_OUTPUT_PORT, 
        &_devices[address]->output_port);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    ret = _mcc152_i2c_read(address, CMD_CONFIG, &_devices[address]->direction);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    return RESULT_SUCCESS;
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
        dev->last_command = 0xFF;
        
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

#if 0
/******************************************************************************
  Write an analog output channel.
 *****************************************************************************/
int mcc152_a_out_write(uint8_t address, uint8_t channel, uint32_t options,
    double value)
{
    uint16_t code;
    int result;
    
    if (!_check_addr(address) ||
        (channel >= NUM_AO_CHANNELS))
    {
        return RESULT_BAD_PARAMETER;
    }

    if ((options & OPTS_NOSCALEDATA) == 0)
    {
        // user passed voltage
        if ((value >= 0.0) && (value <= 5.0))
        {
            // valid
            code = value * (MAX_CODE + 1) / VOLTAGE_RANGE;
            if (code > MAX_CODE)
            {
                code = MAX_CODE;
            }
        }
        else
        {
            return RESULT_BAD_PARAMETER;
        }
    }
    else
    {
        // user passed code
        if ((value >= 0.0) && (value <= MAX_CODE))
        {
            // valid
        }
        else
        {
            return RESULT_BAD_PARAMETER;
        }
    }
    
    if ((result = _mcc152_dac_write(address, code)) != RESULT_SUCCESS)
    {
        return result;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Write all analog output channels.
 *****************************************************************************/
int mcc152_a_out_write_all(uint8_t address, uint32_t options, double* values)
{
    return RESULT_SUCCESS;
}

#endif

/******************************************************************************
  Reset DIO to default configuration.
 *****************************************************************************/
int mcc152_dio_reset(uint8_t address)
{
    int i2c_fd;
    uint8_t addr;
    int ret;

    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    
    addr = I2C_BASE_ADDR + address;

    // open the I2C device handle
    i2c_fd = open(I2C_DEVICE_1, O_RDWR);
    if (i2c_fd < 0)
    {
        return RESULT_RESOURCE_UNAVAIL;
    }

    // set slave address
    ret = ioctl(i2c_fd, I2C_SLAVE, addr);
    if (ret == -1)
    {
        close(i2c_fd);
        return RESULT_UNDEFINED;
    }
    
    // write the register values
    
    // interrupt mask
    ret = _write_byte_data(i2c_fd, CMD_INT_MASK, 0xFF);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    
    // switch to inputs
    ret = _write_byte_data(i2c_fd, CMD_CONFIG, 0xFF);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    _devices[address]->direction = 0xFF;
    
    // pull-up setting
    ret = _write_byte_data(i2c_fd, CMD_PULL_SELECT, 0xFF);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    
    // pull-up enable
    ret = _write_byte_data(i2c_fd, CMD_PULL_ENABLE, 0xFF);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    
    // input invert
    ret = _write_byte_data(i2c_fd, CMD_POLARITY, 0);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }

    // input latch
    ret = _write_byte_data(i2c_fd, CMD_INPUT_LATCH, 0);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    
    // output type
    ret = _write_byte_data(i2c_fd, CMD_OUTPUT_CONFIG, 0);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }

    // output latch
    ret = _write_byte_data(i2c_fd, CMD_OUTPUT_PORT, 0xFF);
    if (ret == -1)
    {
        ret = RESULT_UNDEFINED;
        close(i2c_fd);
        return ret;    
    }
    _devices[address]->output_port = 0xFF;
    
    _devices[address]->last_command = CMD_OUTPUT_PORT;
    close(i2c_fd);
    
    return RESULT_SUCCESS;    
}

/******************************************************************************
  Read DIO input(s).
 *****************************************************************************/
int mcc152_dio_input_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_INPUT_PORT, channel, value);
}

/******************************************************************************
  Write DIO output(s).
 *****************************************************************************/
int mcc152_dio_output_write(uint8_t address, uint8_t channel, uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_OUTPUT_PORT, channel, value,
        true);
}

/******************************************************************************
  Read DIO output(s).
 *****************************************************************************/
int mcc152_dio_output_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_OUTPUT_PORT, channel, value);
}

/******************************************************************************
  Configure DIO direction.
 *****************************************************************************/
int mcc152_dio_direction_write(uint8_t address, uint8_t channel, uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_CONFIG, channel, value, true);
}

/******************************************************************************
  Read DIO direction.
 *****************************************************************************/
int mcc152_dio_direction_read(uint8_t address, uint8_t channel, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_CONFIG, channel, value);
}

/******************************************************************************
  Configure DIO pull-up / pull-down resistor(s).
 *****************************************************************************/
int mcc152_dio_pull_config_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_PULL_SELECT, channel, value,
        false);
}

/******************************************************************************
  Read DIO pull-up / pull-down configuration.
 *****************************************************************************/
int mcc152_dio_pull_config_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_PULL_SELECT, channel, value);
}

/******************************************************************************
  Enable DIO pull-up / pull-down resistor(s).
 *****************************************************************************/
int mcc152_dio_pull_enable_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_PULL_ENABLE, channel, value, false);
}

/******************************************************************************
  Read DIO pull-up / pull-down enable setting.
 *****************************************************************************/
int mcc152_dio_pull_enable_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_PULL_ENABLE, channel, value);
}

/******************************************************************************
  Configure DIO input inversion.
 *****************************************************************************/
int mcc152_dio_input_invert_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_POLARITY, channel, value, false);
}

/******************************************************************************
  Read DIO input inversion configuration.
 *****************************************************************************/
int mcc152_dio_input_invert_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_POLARITY, channel, value);
}

/******************************************************************************
  Configure DIO input latching.
 *****************************************************************************/
int mcc152_dio_input_latch_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_INPUT_LATCH, channel, value,
        false);
}

/******************************************************************************
  Read DIO input latching configuration.
 *****************************************************************************/
int mcc152_dio_input_latch_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_INPUT_LATCH, channel, value);
}

/******************************************************************************
  Configure DIO output type.
 *****************************************************************************/
int mcc152_dio_output_type_write(uint8_t address, uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_OUTPUT_CONFIG, DIO_CHANNEL_ALL,
        value == 0 ? 0 : 1, false);
}

/******************************************************************************
  Read DIO output type.
 *****************************************************************************/
int mcc152_dio_output_type_read(uint8_t address, uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_OUTPUT_CONFIG, DIO_CHANNEL_ALL,
        value);
}

/******************************************************************************
  Write DIO interrupt mask.
 *****************************************************************************/
int mcc152_dio_interrupt_mask_write(uint8_t address, uint8_t channel,
    uint8_t value)
{
    return _mcc152_dio_reg_write(address, CMD_INT_MASK, channel, value, false);
}

/******************************************************************************
  Read DIO interrupt mask.
 *****************************************************************************/
int mcc152_dio_interrupt_mask_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_INT_MASK, channel, value);
}

/******************************************************************************
  Read DIO interrupt status.
 *****************************************************************************/
int mcc152_dio_interrupt_status_read(uint8_t address, uint8_t channel,
    uint8_t* value)
{
    return _mcc152_dio_reg_read(address, CMD_INT_STATUS, channel, value);
}
