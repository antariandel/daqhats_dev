/*
*   mcc152_dac.h
*   Measurement Computing Corp.
*   This file contains functions used with the DAC on the MCC 152.
*
*   07/18/2018
*/
#ifndef _MCC152_DAC_H
#define _MCC152_DAC_H

#include <stdint.h>

int _mcc152_dac_write(uint8_t address, uint8_t channel, uint16_t code);
int _mcc152_dac_write_both(uint8_t address, uint16_t code0, uint16_t code1);
int _mcc152_dac_init(uint8_t address);

#endif