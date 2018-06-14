/*
*   mcc134_cjc.h
*   Measurement Computing Corp.
*   This file contains functions used with the CJC sensor on the MCC 134.
*
*   1 Feb 2018
*/
#ifndef _MCC134_CJC_H
#define _MCC134_CJC_H

#include <stdint.h>

int _mcc134_cjc_init(uint8_t address);
int _mcc134_cjc_read_temp(uint8_t address, double* temp);

#endif