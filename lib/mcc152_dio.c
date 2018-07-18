/*
*   mcc152_dio.c
*   Measurement Computing Corp.
*   This file contains functions used with the I/O expander on the MCC 152.
*
*   07/18/2018
*/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "daqhats.h"
#include "util.h"
#include "mcc152_dio.h"

#define I2C_BASE_ADDR       (0x20)

#define NUM_DIO_CHANNELS    8

// Local data for all MCC 152 boards.
struct dio_device
{
    uint8_t last_command;
    uint8_t output_port;
    uint8_t direction;
} dio_devices[MAX_NUMBER_HATS];


/******************************************************************************
  Write data to an I2C device.
 *****************************************************************************/
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

/******************************************************************************
  Read data from an I2C device.
 *****************************************************************************/
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

/******************************************************************************
  Read a single byte from an I2C device.
 *****************************************************************************/
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
static int _mcc152_i2c_write(uint8_t address, uint8_t command, uint8_t value)
{
    int i2c_fd;
    uint8_t addr;
    int ret;

    if ((address >= MAX_NUMBER_HATS))                // check address failed
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
    
    dio_devices[address].last_command = command;
    close(i2c_fd);
    
    return ret;
}

/******************************************************************************
  Perform an I2C read from the IO expander
 *****************************************************************************/
static int _mcc152_i2c_read(uint8_t address, uint8_t command, uint8_t* value)
{
    int i2c_fd;
    uint8_t addr;
    int ret;

    if ((address >= MAX_NUMBER_HATS) ||                // check address failed
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
    if (command == dio_devices[address].last_command)
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
        dio_devices[address].last_command = command;
    }
    
    close(i2c_fd);
    
    return ret;
}

int _mcc152_dio_reg_write(uint8_t address, uint8_t command, uint8_t channel,
    uint8_t value, bool use_cache)
{
    int ret;
    uint8_t reg_value;
    
    if ((address >= MAX_NUMBER_HATS) ||                // check address failed
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
            case DIO_CMD_OUTPUT_PORT:
                reg_value = dio_devices[address].output_port;
                break;
            case DIO_CMD_CONFIG:
                reg_value = dio_devices[address].direction;
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
    case DIO_CMD_OUTPUT_PORT:
        dio_devices[address].output_port = reg_value;
        break;
    case DIO_CMD_CONFIG:
        dio_devices[address].direction = reg_value;
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
int _mcc152_dio_reg_read(uint8_t address, uint8_t command, uint8_t channel,
    uint8_t* value)
{
    int ret;
    uint8_t reg_value;
    
    if ((address >= MAX_NUMBER_HATS) ||                // check address failed
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
    case DIO_CMD_OUTPUT_PORT:
        dio_devices[address].output_port = reg_value;
        break;
    case DIO_CMD_CONFIG:
        dio_devices[address].direction = reg_value;
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

int _mcc152_dio_init(int address)
{
    int ret;
    
    if ((address >= MAX_NUMBER_HATS))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }
    
    // read the registers that have local cache
    dio_devices->last_command = 0xFF;
    
    ret = _mcc152_i2c_read(address, DIO_CMD_OUTPUT_PORT, 
        &dio_devices[address].output_port);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    ret = _mcc152_i2c_read(address, DIO_CMD_CONFIG, 
        &dio_devices[address].direction);
    if (ret != RESULT_SUCCESS)
    {
        return ret;
    }
    
    return RESULT_SUCCESS;
}

