/*
*   mcc134.c
*   author Measurement Computing Corp.
*   brief This file contains functions used with the MCC 134.
*
*   date 4 Apr 2018
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "daqhats.h"
#include "util.h"
#include "cJSON.h"
#include "gpio.h"
#include "mcc134_cjc.h"
#include "mcc134_adc.h"
#include "nist.h"

//*****************************************************************************
// Constants

#define MAX_CODE            0xFFFFFF
#define OPEN_TC_CODE        0x7FFFFF
#define OPEN_TC_VOLTAGE     0.128
#define PGA_GAIN            16
#define REFERENCE_VOLTS     2.048
#define POS_OVERRANGE_VOLTS 0.084
#define NEG_OVERRANGE_VOLTS -0.012
#define NUM_CHANNELS        4       // The number of analog input channels.

#define SERIAL_SIZE         (8+1)   // The maximum size of the serial number string, plus NULL.
#define CAL_DATE_SIZE       (10+1)  // The maximum size of the calibration date string, plus NULL.

#define MIN(a, b)           ((a < b) ? a : b)
#define MAX(a, b)           ((a > b) ? a : b)

// Constants used by the CJC thread
#define CJC_READ_INTERVAL_US    1000000     // read interval time, us
#define CJC_AVERAGE_COUNT       120         // average window in samples
#define CJC_STARTUP_TIME_US     250000      // wait at least 240ms for first reading

// Map the channel inputs to the ADC pins
const uint8_t CHAN_HI[NUM_CHANNELS] = {0, 4, 6, 2};
const uint8_t CHAN_LO[NUM_CHANNELS] = {1, 5, 7, 3};

const double CJC_OFFSETS[NUM_CHANNELS] = 
{
    0.7,
    0.7,
    0.7,
    0.7 
};

/// \cond
// Contains the device-specific data stored at the factory.
struct mcc134FactoryData
{
    // Serial number
    char serial[SERIAL_SIZE];
    // Calibration date in the format 2017-09-19
    char cal_date[CAL_DATE_SIZE];
    // Calibration coefficients - per channel slopes
    double slopes[NUM_CHANNELS];
    // Calibration coefficents - per channel offsets
    double offsets[NUM_CHANNELS];
};

// Local data for each open MCC 134 board.
struct mcc134Device
{
    uint16_t handle_count;              // the number of handles open to this device
    uint8_t tc_types[NUM_CHANNELS];     // the thermocouple types
    bool last_reading_open;             // true if last reading was railed high - used for improving settling performance
    bool cjc_valid;                     // true when the CJC sensor is ready to operate
    double cjc_temperature;             // the averaged CJC sensor reading, returned from the CJC thread
    bool stop_thread;
    pthread_t cjc_handle;
    struct mcc134FactoryData factory_data;   // Factory data
};

/// \endcond

//*****************************************************************************
// Variables

static struct mcc134Device* _devices[MAX_NUMBER_HATS];
static bool _mcc134_lib_initialized = false;

#define SPI_BITS    8       // 8 bits per transfer

//*****************************************************************************
// Local Functions

/******************************************************************************
  Validate parameters for an address
 *****************************************************************************/
static bool _check_addr(uint8_t address)
{
    if ((address >= MAX_NUMBER_HATS) ||     // Address is invalid
        !_mcc134_lib_initialized ||         // Library is not initialized
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
  Perform a SPI transfer to the ADC or CJC sensor
 *****************************************************************************/
int _mcc134_spi_transfer(uint8_t address, uint8_t spi_bus, uint8_t spi_mode, uint32_t spi_rate, uint8_t spi_delay, void* tx_data, void* rx_data, uint8_t data_count)
{
    int lock_fd;
    int spi_fd;
    const char* spi_device;
    uint8_t temp;
    int ret;

    if (!_check_addr(address))                // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    

    switch (spi_bus)
    {
    case 0:
        spi_device = SPI_DEVICE_0;
        break;
    case 1:
        spi_device = SPI_DEVICE_1;
        break;
    default:
        return RESULT_BAD_PARAMETER;
    }

    // open the SPI device handle
    spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0)
    {
        return RESULT_RESOURCE_UNAVAIL;
    }

    // Obtain a spi lock
    if ((lock_fd = _obtain_spi_lock(spi_bus)) < 0)
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
        .rx_buf = (uintptr_t)rx_data,
        .len = data_count,
        .delay_usecs = spi_delay,
        .speed_hz = spi_rate,
        .bits_per_word = SPI_BITS,
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


/******************************************************************************
  Sets an mcc134FactoryData to default values.
 *****************************************************************************/
static void _set_defaults(struct mcc134FactoryData* data)
{
    int i;
    
    if (data)
    {
        strcpy(data->serial, "00000000");
        strcpy(data->cal_date, "1970-01-01");
        for (i = 0; i < NUM_CHANNELS; i++)
        {
            data->slopes[i] = 1.0;
            data->offsets[i] = 0.0;
        }
    }
}

/******************************************************************************
  Parse the factory data JSON structure. Does not recurse so we can support 
  multiple processes.

  Expects a JSON structure like:

    {
        "serial": "00000000",
        "calibration": 
        {
            "date": "2017-09-19",
            "slopes":
            [
                1.000000,
                1.000000,
                1.000000,
                1.000000,
                1.000000,
                1.000000,
                1.000000,
                1.000000
            ],
            "offsets":
            [
                0.000000,
                0.000000,
                0.000000,
                0.000000,
                0.000000,
                0.000000,
                0.000000,
                0.000000
            ]
        }
    }

  If it finds all of these keys it will return 1, otherwise will return 0.
 *****************************************************************************/
static int _parse_factory_data(cJSON* root, struct mcc134FactoryData* data)
{ 
    bool got_serial = false;
    bool got_date = false;
    bool got_slopes = false;
    bool got_offsets = false;
    cJSON* child;
    cJSON* calchild;
    cJSON* subchild;
    int index;

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
        else if (!strcmp(child->string, "calibration") &&
                (child->type == cJSON_Object))
        {
            // Found the calibration object, must go down a level
            calchild = child->child;
            
            while (calchild)
            {
                if (!strcmp(calchild->string, "date") &&
                    (calchild->type == cJSON_String) && 
                    calchild->valuestring)
                {
                    // Found the calibration date
                    strncpy(data->cal_date, calchild->valuestring, CAL_DATE_SIZE);
                    got_date = true;
                }
                else if (!strcmp(calchild->string, "slopes") &&
                        (calchild->type == cJSON_Array))
                {
                    // Found the slopes array, must go down a level
                    subchild = calchild->child;
                    index = 0;
                    
                    while (subchild)
                    {
                        // Iterate through the slopes array
                        if ((subchild->type == cJSON_Number) &&
                            (index < NUM_CHANNELS))
                        {
                            data->slopes[index] = subchild->valuedouble;
                            index++;
                        }
                        subchild = subchild->next;
                    }
                    
                    if (index == NUM_CHANNELS)
                    {
                        // Must have all channels to be successful
                        got_slopes = true;
                    }
                }
                else if (!strcmp(calchild->string, "offsets") &&
                        (calchild->type == cJSON_Array))
                {
                    // Found the offsets array, must go down a level
                    subchild = calchild->child;
                    index = 0;
                    
                    while (subchild)
                    {
                        // Iterate through the offsets array
                        if ((subchild->type == cJSON_Number) &&
                            (index < NUM_CHANNELS))
                        {
                            data->offsets[index] = subchild->valuedouble;
                            index++;
                        }
                        subchild = subchild->next;
                    }
                    
                    if (index == NUM_CHANNELS)
                    {
                        // Must have all channels to be successful
                        got_offsets = true;
                    }
                }
                
                calchild = calchild->next;
            }
        }
        child = child->next;
    }
    
    if (got_serial && got_date && got_slopes && got_offsets)
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
static void _mcc134_lib_init(void)
{
    int i;
    
    if (!_mcc134_lib_initialized)
    {
        for (i = 0; i < MAX_NUMBER_HATS; i++)
        {
            _devices[i] = NULL;
        }
        
        _mcc134_lib_initialized = true;
    }
}

/******************************************************************************
 Read and average the CJC sensor.
 *****************************************************************************/
static void* _cjc_thread(void* arg)
{
    uint8_t address = *(uint8_t*)arg;
    double val;
    struct mcc134Device* dev = _devices[address];
    int index;
    double* cjc_average_buffer;
    uint8_t cjc_average_index;          
    uint8_t cjc_average_count;  
    int result;
    
    free(arg);
    
    if (!_check_addr(address) ||
        (dev == NULL))
    {
        return NULL;
    }
    
    cjc_average_buffer = (double*)malloc(CJC_AVERAGE_COUNT * sizeof(double));
    if (!cjc_average_buffer)
    {
        return NULL;
    }
    
    usleep(CJC_STARTUP_TIME_US);        // wait for CJC sensor to be ready
    
    // make first reading and initialize variables
    do
    {
        result = _mcc134_cjc_read_temp(address, &val);
        if (result != RESULT_SUCCESS)
        {
            usleep(1000);
        }
    } while ((result != RESULT_SUCCESS) && !dev->stop_thread);
    
    cjc_average_buffer[0] = val;
    cjc_average_index = 1;
    cjc_average_count = 1;
    dev->cjc_valid = true;
    dev->cjc_temperature = val;
    
    while (!dev->stop_thread)
    {
        // sleep until ready to read CJC sensor
        usleep(CJC_READ_INTERVAL_US);           
        
        // read the sensor
        do
        {
            result = _mcc134_cjc_read_temp(address, &val);
            if (result != RESULT_SUCCESS)
            {
                usleep(1000);
            }
            else
            {
                // store it in the average buffer
                if (CJC_AVERAGE_COUNT > 1)
                {
                    cjc_average_buffer[cjc_average_index++] = val;
                    if (cjc_average_index >= CJC_AVERAGE_COUNT)
                    {
                        cjc_average_index = 0;
                    }
                    if (cjc_average_count < CJC_AVERAGE_COUNT)
                    {
                        cjc_average_count++;
                    }

                    // calculate the CJC reading average with a window average
                    val = 0.0;
                    for (index = 0; index < cjc_average_count; index++)
                    {
                        val += cjc_average_buffer[index];
                    }
                    val /= cjc_average_count;
                
                }

                // store the average in the device structure
                dev->cjc_temperature = val;
            }
        } while ((result != RESULT_SUCCESS) && !dev->stop_thread);
    }
    
    free(cjc_average_buffer);
    return NULL;
}

//*****************************************************************************
// Global Functions

/******************************************************************************
  Open a connection to the MCC 134 device at the specified address.
 *****************************************************************************/
int mcc134_open(uint8_t address)
{
    struct HatInfo info;
    char* custom_data;
    uint16_t custom_size;
    struct mcc134Device* dev;
    int result;
        
    _mcc134_lib_init();
    
    // validate the parameters
    if ((address >= MAX_NUMBER_HATS))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    if (_devices[address] == NULL)
    {
        // this is either the first time this device is being opened or it is not a 134
    
        // read the EEPROM file(s), verify that it is an MCC 134, and get the cal data
        if (_hat_info(address, &info, NULL, &custom_size) == RESULT_SUCCESS)
        {
            if (info.id == HAT_ID_MCC_134)
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
            // no EEPROM info was found - allow opening the board with an uninitialized EEPROM
            custom_size = 0;
        }
    
        // create a struct to hold device instance data
        _devices[address] = (struct mcc134Device*)calloc(1, sizeof(struct mcc134Device));
        dev = _devices[address];
        
        // initialize the struct elements
        dev->handle_count = 1;
        dev->cjc_handle = 0;
        dev->stop_thread = false;
        dev->cjc_valid = false;
        dev->tc_types[0] = TC_TYPE_J;
        dev->tc_types[1] = TC_TYPE_J;
        dev->tc_types[2] = TC_TYPE_J;
        dev->tc_types[3] = TC_TYPE_J;
        dev->last_reading_open = false;
        
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

        // initialize the CJC sensor
        if ((result = _mcc134_cjc_init(address)) != RESULT_SUCCESS)
        {
            // the sensor ID was not valid
            free(dev);
            _devices[address] = NULL;
            return result;
        }

        // start the CJC thread
        pthread_attr_t attr;
        if ((result = pthread_attr_init(&attr)) != 0)
        {
            free(dev);
            _devices[address] = NULL;
            return RESULT_RESOURCE_UNAVAIL;
        }
        
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        
        uint8_t* temp_address = (uint8_t*)malloc(sizeof(uint8_t));
        *temp_address = address;
        if ((result = pthread_create(&dev->cjc_handle, &attr, &_cjc_thread, temp_address))
            != 0)
        {
            free(temp_address);
            pthread_attr_destroy(&attr);
            free(dev);
            _devices[address] = NULL;
            return RESULT_RESOURCE_UNAVAIL;
        }    
        
        pthread_attr_destroy(&attr);

        // initialize the ADC
        if ((result = _mcc134_adc_init(address)) != RESULT_SUCCESS)
        {
            mcc134_close(address);
            return result;
        }

        // perform an offset correction
        if ((result = _mcc134_adc_calibrate_self_offset(address)) != RESULT_SUCCESS)
        {
            mcc134_close(address);
            return result;
        }
    }
    else
    {
        // the device has already been opened and initialized, increment reference count
        dev = _devices[address];
        dev->handle_count++;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Check if an MCC 134 is open.
 *****************************************************************************/
int mcc134_is_open(uint8_t address)
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
  Close a connection to an MCC 134 device and free allocated resources.
 *****************************************************************************/
int mcc134_close(uint8_t address)
{
    if (!_check_addr(address))
    {
        return RESULT_BAD_PARAMETER;
    }

    _devices[address]->handle_count--;
    if (_devices[address]->handle_count == 0)
    {
        // stop CJC thread
        if (_devices[address]->cjc_handle != 0)
        {
            // If the thread is running then tell it to stop and wait for it.
            _devices[address]->stop_thread = true;
            
            pthread_join(_devices[address]->cjc_handle, NULL);
            _devices[address]->cjc_handle = 0;
        }
        
        free(_devices[address]);
        _devices[address] = NULL;
    }
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Return the number of analog input channels on the board.
 *****************************************************************************/
int mcc134_a_in_num_channels(void)
{
    return NUM_CHANNELS;
}

/******************************************************************************
  Read the serial number.
 *****************************************************************************/
int mcc134_serial(uint8_t address, char* buffer)
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
  Read the calibration date.
 *****************************************************************************/
int mcc134_calibration_date(uint8_t address, char* buffer)
{
    // validate parameters
    if (!_check_addr(address) ||
        (buffer == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    strcpy(buffer, _devices[address]->factory_data.cal_date);
    return RESULT_SUCCESS;
}

/******************************************************************************
  Read the calibration coefficients.
 *****************************************************************************/
int mcc134_calibration_coefficient_read(uint8_t address, uint8_t channel, double* slope, double* offset)
{
    // validate parameters
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (slope == NULL) ||
        (offset == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    *slope = _devices[address]->factory_data.slopes[channel];
    *offset = _devices[address]->factory_data.offsets[channel];
    return RESULT_SUCCESS;
}

/******************************************************************************
  Write the calibration coefficients.
 *****************************************************************************/
int mcc134_calibration_coefficient_write(uint8_t address, uint8_t channel, double slope, double offset)
{
    // validate parameters
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    _devices[address]->factory_data.slopes[channel] = slope;
    _devices[address]->factory_data.offsets[channel] = offset;
    return RESULT_SUCCESS;
}

/******************************************************************************
  Perform an ADC self offset correction.
 *****************************************************************************/
int mcc134_self_offset_correction(uint8_t address)
{
    if (!_check_addr(address))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    return _mcc134_adc_calibrate_self_offset(address);
}

/******************************************************************************
  Read an analog input channel.
 *****************************************************************************/
int mcc134_a_in_read(uint8_t address, uint8_t channel, uint32_t options, double* value)
{
    int32_t code;
    double val;
    int result;
    
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (value == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    if (_devices[address]->last_reading_open)
    {
        // perform an extra reading for better settling
        if ((result = _mcc134_adc_read_code(address, CHAN_HI[channel], CHAN_LO[channel], &code)) != RESULT_SUCCESS)
        {
            return result;
        }
    }
    if ((result = _mcc134_adc_read_code(address, CHAN_HI[channel], CHAN_LO[channel], &code)) != RESULT_SUCCESS)
    {
        return result;
    }
    
    if (code == OPEN_TC_CODE)
    {
        // don't perform all the calcs if the ADC is railed
        _devices[address]->last_reading_open = true;
        if (options & OPTS_NOSCALEDATA)
        {
            val = OPEN_TC_CODE;
        }
        else
        {
            val = OPEN_TC_VOLTAGE;
        }
    }
    else
    {
        _devices[address]->last_reading_open = false;
        
        // calibrate?
        if (options & OPTS_NOCALIBRATEDATA)
        {
            val = (double)code;
        }
        else
        {
            val = ((double)code * _devices[address]->factory_data.slopes[channel]) + 
                _devices[address]->factory_data.offsets[channel];
        }

        // calculate voltage?
        if ((options & OPTS_NOSCALEDATA) == 0)
        {
            val = (val * (REFERENCE_VOLTS / PGA_GAIN)) / ((double)(MAX_CODE + 1) / 2);
        }
    }
    
    *value = val;

    return RESULT_SUCCESS;
}

/******************************************************************************
  Set the TC type for a channel.
 *****************************************************************************/
int mcc134_tc_type_write(uint8_t address, uint8_t channel, uint8_t type)
{
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (type > TC_TYPE_N))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    _devices[address]->tc_types[channel] = type;
    return RESULT_SUCCESS;
}

/******************************************************************************
  Read the TC type for a channel.
 *****************************************************************************/
int mcc134_tc_type_read(uint8_t address, uint8_t channel, uint8_t* type)
{
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (!type))
    {
        return RESULT_BAD_PARAMETER;
    }
    
    *type = _devices[address]->tc_types[channel];
    return RESULT_SUCCESS;
}

/******************************************************************************
  Read a temperature channel.
 *****************************************************************************/
int mcc134_t_in_read(uint8_t address, uint8_t channel, double* temperature)
{
    double val;
    double cjc_temperature;
    double cjc_voltage;
    double voltage_reading;
    struct mcc134Device* dev;
    int result;
    
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (temperature == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    dev = _devices[address];

    val = 0.0;
    
    // wait for initial CJC reading on startup
    while(!dev->cjc_valid)
    {
        usleep(10000);
    }

    // read the ADC voltage
    result = mcc134_a_in_read(address, channel, 0, &voltage_reading);
    if (result != RESULT_SUCCESS)
    {
        return result;
    }
   
    cjc_temperature = dev->cjc_temperature - CJC_OFFSETS[channel];

    if (voltage_reading == OPEN_TC_VOLTAGE)
    {
        val = OPEN_TC_VALUE;
    }
    else if ((voltage_reading > POS_OVERRANGE_VOLTS) || (voltage_reading < NEG_OVERRANGE_VOLTS))
    {
        val = OVERRANGE_TC_VALUE;
    }
    else
    {
        // calculate the temperature value
        val = voltage_reading * 1000.0;      // convert to mV
        
        // calculate the CJC offset voltage for the specific TC type
        cjc_voltage = NISTCalcVoltage(dev->tc_types[channel], cjc_temperature);
        // add the CJC voltage to the thermocouple voltage
        val += cjc_voltage;
        // calculate the temperature
        val = NISTCalcTemperature(dev->tc_types[channel], val);
    }
    *temperature = val;
    
    return RESULT_SUCCESS;
}

/******************************************************************************
  Read the CJC temperature.
 *****************************************************************************/
int mcc134_cjc_read(uint8_t address, uint8_t channel, double* cjc_temp)
{
    struct mcc134Device* dev;
    
    if (!_check_addr(address) ||
        (channel >= NUM_CHANNELS) ||
        (cjc_temp == NULL))
    {
        return RESULT_BAD_PARAMETER;
    }
    dev = _devices[address];

    // wait for initial CJC reading on startup
    while(!dev->cjc_valid)
    {
        usleep(10000);
    }

    *cjc_temp = dev->cjc_temperature - CJC_OFFSETS[channel];

    return RESULT_SUCCESS;
}
