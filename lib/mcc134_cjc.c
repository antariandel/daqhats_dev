/*
*   mcc134_cjc.c
*   Measurement Computing Corp.
*   This file contains functions used with the CJC sensor on the MCC 134.
*
*   1 Feb 2018
*/
#include <unistd.h>
#include "daqhats.h"
#include "mcc134_cjc.h"

// SPI stuff
#define SPI_BUS     1
#define SPI_RATE    4000000
#define SPI_MODE    3
#define SPI_DELAY   0

//*****************************
// Register definitions
#define REG_STATUS  (0x00 << 3)
#define REG_CONFIG  (0x01 << 3)
#define REG_VALUE   (0x02 << 3)
#define REG_ID      (0x03 << 3)
#define REG_TCRIT   (0x04 << 3)
#define REG_THYST   (0x05 << 3)
#define REG_THIGH   (0x06 << 3)
#define REG_TLOW    (0x07 << 3)

#define READ_REG    (0x01 << 6)
#define CONTINUOUS  (0x01 << 2)

#define ID_VALUE    0xC0
#define ID_MASK     0xF8

#define C_PER_BIT   (1.0/128.0)

extern int _mcc134_spi_transfer(uint8_t address, uint8_t spi_bus, uint8_t spi_mode, 
    uint32_t spi_rate, uint8_t spi_delay, void* tx_data, void* rx_data, uint8_t data_count);
    

int _mcc134_cjc_init(uint8_t address)
{
    uint8_t buffer[4];
    int result;

    // reset the SPI interface
    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
    buffer[2] = 0xFF;
    buffer[3] = 0xFF;
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 4)) != RESULT_SUCCESS)
    {
        return result;
    }
            
    // wait >500us before addressing the device
    usleep(600);

    // read the ID register
    buffer[0] = REG_ID | READ_REG;
    buffer[1] = 0;
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, buffer, 2)) != RESULT_SUCCESS)
    {
        return result;
    }
    
    // check the ID portion
    if ((buffer[1] & ID_MASK) != ID_VALUE)
    {
        return RESULT_INVALID_DEVICE;
    }

    // configure for 16 bits, 1 SPS mode
    buffer[0] = REG_CONFIG;
    buffer[1] = 0xC0;
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 2)) != RESULT_SUCCESS)
    {
        return result;
    }
   
    return RESULT_SUCCESS;
}

int _mcc134_cjc_read_temp(uint8_t address, double* temp)
{
    uint8_t buffer[3];
    int16_t code;
    double value;
    int result;
    
    // read the temperature value register
    buffer[0] = REG_VALUE | READ_REG;
    buffer[1] = 0;
    buffer[2] = 0;
    
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, buffer, 3)) != RESULT_SUCCESS)
    {
        return result;
    }
    code = ((int16_t)buffer[1] << 8) | buffer[2];
    value = (double)code * C_PER_BIT;
    if (temp)
    {
        *temp = value;
    }
    return RESULT_SUCCESS;
}
