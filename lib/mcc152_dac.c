/*
*   mcc152_dac.c
*   Measurement Computing Corp.
*   This file contains functions used with the DAC on the MCC 152.
*
*   07/18/2018
*/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "daqhats.h"
#include "util.h"
#include "mcc152_dac.h"

//*****************************
// DAC register definitions
#define DAC_A				0x00
#define DAC_B				0x01
#define DAC_BOTH			0x07

#define DACCMD_WRITE		(0x00 << 3)
#define DACCMD_LDAC			(0x01 << 3)
#define DACCMD_WRITE_LOAD_ALL	(0x02 << 3)
#define DACCMD_WRITE_LOAD	(0x03 << 3)
#define DACCMD_POWER_MODE	(0x04 << 3)
#define DACCMD_RESET		(0x05 << 3)
#define DACCMD_LDAC_REGS	(0x06 << 3)
#define DACCMD_REF_MODE		(0x07 << 3)

#define MAX_CHANNEL     1
#define MAX_CODE        4095

static const char* const spi_device = SPI_DEVICE_1; // the spidev device
static const enum SpiBus spi_bus_num = SPI_BUS_1;   // for obtain_lock()
static const uint8_t spi_mode = SPI_MODE_1;         // use mode 1
                                                    // (CPOL=0, CPHA=1)
static const uint8_t spi_bits = 8;                  // 8 bits per transfer
static const uint32_t spi_rate = 50000000;          // maximum SPI clock
                                                    // frequency
static const uint16_t spi_delay = 0;                // delay in us before
                                                    // removing CS

/******************************************************************************
  Perform a SPI transfer to the DAC
 *****************************************************************************/
static int _mcc152_spi_transfer(uint8_t address, void* tx_data,
    uint8_t data_count)
{
    int lock_fd;
    int spi_fd;
    uint8_t temp;
    int ret;

    if (address >= MAX_NUMBER_HATS)                // check address failed
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
    
int _mcc152_dac_write(uint8_t address, uint8_t channel, uint16_t code)
{
    uint8_t data[3];
    uint16_t value;
    
    if ((address >= MAX_NUMBER_HATS) ||         // check address failed
        (channel > MAX_CHANNEL) ||              // bad channel
        (code > MAX_CODE))                      // bad DAC code
    {
        return RESULT_BAD_PARAMETER;
    }    
    
    if (channel == 0)
    {
        data[0] = DACCMD_WRITE_LOAD | DAC_A;
    }
    else
    {
        data[0] = DACCMD_WRITE_LOAD | DAC_B;
    }
    value = code << 4;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)value;
    
    return _mcc152_spi_transfer(address, data, 3);
}

int _mcc152_dac_write_both(uint8_t address, uint16_t code0, uint16_t code1)
{
    uint8_t data[3];
    uint16_t value;
    int result;
    
    if ((address >= MAX_NUMBER_HATS) ||             // check address failed
        (code0 > MAX_CODE) || (code1 > MAX_CODE))   // bad DAC code
    {
        return RESULT_BAD_PARAMETER;
    }    
    
    data[0] = DACCMD_WRITE | DAC_A;
    value = code0 << 4;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)value;
    result = _mcc152_spi_transfer(address, data, 3);
    if (result != RESULT_SUCCESS)
    {
        return result;
    }
    
    data[0] = DACCMD_WRITE_LOAD_ALL | DAC_B;
    value = code1 << 4;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)value;
    return _mcc152_spi_transfer(address, data, 3);    
}

int _mcc152_dac_init(uint8_t address)
{
    uint8_t data[3];
    
    if (address >= MAX_NUMBER_HATS)                 // check address failed
    {
        return RESULT_BAD_PARAMETER;
    }    
    
    // the DAC defaults to external reference so switch it to the internal
    // reference
    data[0] = DACCMD_REF_MODE;
    data[1] = 0;
    data[2] = 1;
    return _mcc152_spi_transfer(address, data, 3);
}

