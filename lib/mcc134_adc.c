/*
*   mcc134_adc.c
*   Measurement Computing Corp.
*   This file contains functions used with the ADC on the MCC 134.
*
*   11 Apr 2018
*/
#include <unistd.h>
#include <stdio.h>
#include "daqhats.h"
#include "mcc134_adc.h"
#include "util.h"

// SPI stuff
#define SPI_BUS     0
#define SPI_RATE    2000000
#define SPI_MODE    1
#define SPI_DELAY   2
    
//*****************************
// Register definitions
#define REG_MUX0        0x00
#define REG_VBIAS		0x01
#define REG_MUX1		0x02
#define REG_SYS0		0x03
#define REG_OFC0		0x04
#define REG_OFC1		0x05
#define REG_OFC2		0x06
#define REG_FSC0		0x07
#define REG_FSC1		0x08
#define REG_FSC2		0x09
#define REG_IDAC0		0x0A
#define REG_IDAC1		0x0B
#define REG_GPIOCFG		0x0C
#define REG_GPIODIR		0x0D
#define REG_GPIODAT		0x0E

//*****************************
// Command definitions
#define CMD_WAKEUP		0x00
#define CMD_SLEEP		0x02
#define CMD_SYNC		0x04
#define CMD_RESET		0x06
#define CMD_NOP			0xFF
#define CMD_RDATA		0x12
#define CMD_RDATAC		0x14
#define CMD_SDATAC		0x16
#define CMD_RREG		0x20
#define CMD_WREG		0x40
#define CMD_SYSOCAL		0x60
#define CMD_SYSGCAL		0x61
#define CMD_SELFOCAL	0x62

// The MCC 134 uses the 20Hz data rate for maximum 50/60Hz noise rejection.
#define DATA_RATE_INDEX     2
// The gain is fixed at 16x for a +/-128 mV range to cover all TC types.
#define PGA_GAIN_INDEX      4

// These are the times from the data sheet based on a 4.096 MHz clock.  However, the internal
// oscillator has a 5% tolerance, so add 5% to the times for worst case timing.
static const uint32_t _calibration_times_us[] = 
{
    (uint32_t)(3201.01 * 1.05 * 1000), 
    (uint32_t)(1601.01 * 1.05 * 1000),
    (uint32_t)(801.012 * 1.05 * 1000),
    (uint32_t)(400.26 * 1.05 * 1000),
    (uint32_t)(200.26 * 1.05 * 1000),
    (uint32_t)(100.14 * 1.05 * 1000),
    (uint32_t)(50.14 * 1.05 * 1000),
    (uint32_t)(25.14 * 1.05 * 1000),
    (uint32_t)(16.14 * 1.05 * 1000),
    (uint32_t)(8.07 * 1.05 * 1000)
};

static const uint32_t _conversion_times_us[] = 
{
    (uint32_t)(199.258 * 1.05 * 1000),
    (uint32_t)(99.633 * 1.05 * 1000),
    (uint32_t)(49.820 * 1.05 * 1000),
    (uint32_t)(24.92 * 1.05 * 1000),
    (uint32_t)(12.467 * 1.05 * 1000),
    (uint32_t)(6.240 * 1.05 * 1000),
    (uint32_t)(3.124 * 1.05 * 1000),
    (uint32_t)(1.569 * 1.05 * 1000),
    (uint32_t)(1.014 * 1.05 * 1000),
    (uint32_t)(0.514 * 1.05 * 1000)
};

extern int _mcc134_spi_transfer(uint8_t address, uint8_t spi_bus, uint8_t spi_mode, 
    uint32_t spi_rate, uint8_t spi_delay, void* tx_data, void* rx_data, uint8_t data_count);
    
int _mcc134_adc_init(uint8_t address)
{
    uint8_t buffer[6];
    int lock_fd;
    int result;

    // lock the board
    if ((lock_fd = _obtain_board_lock(address)) < 0)
    {
        // could not get a lock within 5 seconds, report as a timeout
        return RESULT_LOCK_TIMEOUT;
    }
    
    // reset the ADC to defaults
    buffer[0] = CMD_RESET;
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 1)) != RESULT_SUCCESS)
    {
        _release_lock(lock_fd);
        return result;
    }
    
    // wait >0.6ms before using the ADC
    usleep(700);
    
    // Initialize the registers that don't use default values
    buffer[0] = CMD_WREG | REG_MUX0;
    buffer[1] = 4-1;        // count - 1
    buffer[2] = 0x01;       // MUX0
    buffer[3] = 0x00;       // VBIAS
    buffer[4] = 0x30;       // MUX1: internal ref always on and selected, normal mux operation
    buffer[5] = (PGA_GAIN_INDEX << 4) + DATA_RATE_INDEX; // SYS0: specified gain and data rate
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 6)) != RESULT_SUCCESS)
    {
        _release_lock(lock_fd);
        return RESULT_SUCCESS;
    }
        
    usleep(1000);
    
    _release_lock(lock_fd);
    return RESULT_SUCCESS;
}

int _mcc134_adc_calibrate_self_offset(uint8_t address)
{
    uint8_t buffer[1];
    int lock_fd;
    int result;

    // lock the board
    if ((lock_fd = _obtain_board_lock(address)) < 0)
    {
        // could not get a lock within 5 seconds, report as a timeout
        return RESULT_LOCK_TIMEOUT;
    }
    
    buffer[0] = CMD_SELFOCAL;
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 1)) != RESULT_SUCCESS)
    {
        _release_lock(lock_fd);
        return result;
    }
    
    // wait for calibration complete, time based on data rate
    usleep(_calibration_times_us[DATA_RATE_INDEX]);
    
    _release_lock(lock_fd);
    return RESULT_SUCCESS;
}

int _mcc134_adc_read_code(uint8_t address, uint8_t hi_input, uint8_t lo_input, int32_t* code)
{
    int32_t mycode[2];
    int index;
    uint8_t regval;
    uint8_t buffer[4];
    uint8_t rbuffer[4];
    int lock_fd;
    int result;
    int match;

    // lock the board
    if ((lock_fd = _obtain_board_lock(address)) < 0)
    {
        // could not get a lock within 5 seconds, report as a timeout
        return RESULT_LOCK_TIMEOUT;
    }
    
    // write the mux register, conversion will automatically start
    regval = (hi_input << 3) | (lo_input);
    buffer[0] = CMD_WREG | REG_MUX0;
    buffer[1] = 1-1;
    buffer[2] = regval; // MUX0: select positive and negative ADC inputs
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, NULL, 3)) != RESULT_SUCCESS)
    {
        _release_lock(lock_fd);
        return result;
    }
    
    // wait for conversion
    usleep(_conversion_times_us[DATA_RATE_INDEX]);
    
    // Since we are not using the DRDY signal there is a chance we will read the result while the 
    // data register is being updated, resulting in corrupted data.  Read multiple times to ensure the correct value.
    buffer[0] = CMD_RDATA;
    buffer[1] = CMD_NOP;
    buffer[2] = CMD_NOP;
    buffer[3] = CMD_NOP;
    
    if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, rbuffer, 4)) != RESULT_SUCCESS)
    {
        _release_lock(lock_fd);
        return result;
    }
    mycode[0] = (int32_t)rbuffer[1] << 16 |
                (int32_t)rbuffer[2] << 8  |
                rbuffer[3];

    index = 1;
    match = 0;
    while (!match)
    {
        usleep(100);
        
        if ((result = _mcc134_spi_transfer(address, SPI_BUS, SPI_MODE, SPI_RATE, SPI_DELAY, buffer, rbuffer, 4)) != RESULT_SUCCESS)
        {
            _release_lock(lock_fd);
            return result;
        }
        mycode[index] = (int32_t)rbuffer[1] << 16 |
                        (int32_t)rbuffer[2] << 8  |
                        rbuffer[3];
        
        // compare results
        if (mycode[0] == mycode[1])
        {
            match = 1;
        }
        
        if (index != 0)
        {
            index = 0;
        }
    }
    
    _release_lock(lock_fd);
    
    // sign extend
    if (mycode[0] & 0x00800000)
    {
        mycode[0] |= 0xFF000000;
    }
    
    if (code)
    {
        *code = mycode[0];
    }
    
    return RESULT_SUCCESS;
}
