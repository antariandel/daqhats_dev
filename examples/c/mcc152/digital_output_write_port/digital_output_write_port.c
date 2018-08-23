/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_dio_reset
        mcc152_dio_output_write_port
        mcc152_dio_config_write_port
        mcc152_info

    Purpose:
        Write all digital outputs until terminated by the user.

    Description:
        This example demonstrates using the digital I/O as outputs and writing
        them as an entire port.

*****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "../../daqhats_utils.h"

#define BUFFER_SIZE 8

bool get_input(uint8_t* p_value)
{
    char buffer[BUFFER_SIZE];
    int value;

    if (p_value == NULL)
    {
        return false;
    }
    
    while (1)
    {
        printf("Enter the output value, non-numeric character to exit: ");

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            // Empty string or input error.
            return false;
        }
        else
        {
            if (buffer[0] == '\n')
            {
                // Enter.
                return false;
            }
            
            // Got a string - convert to a number with automatic format
            // conversion.
            if (sscanf(buffer, "%i", &value) == 0)
            {
                // Not a number.
                return false;
            }

            // Compare the number to min and max allowed.
            if ((value < 0) || (value > 255))
            {
                // Out of range, ask again.
                printf("Value out of range.\n");
            }
            else
            {
                // Valid value.
                *p_value = (uint8_t)value;
                return true;
            }
        }
    }
}

int main(void)
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    bool error;
    bool run_loop;
    uint8_t value;

    printf("\nMCC 152 digital output write example.\n");
    printf("Sets all digital I/O channels to outputs then gets values from\n");
    printf("the user and updates the outputs. The value can be specified\n");
    printf("as decimal (0 - 255,) hexadecimal (0x0 - 0xFF,) or octal ");
    printf("(0 - 0377.)\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_dio_reset\n");
    printf("      mcc152_dio_output_write_port\n");
    printf("      mcc152_dio_config_write_port\n");
    printf("      mcc152_info\n\n");

    // Select the device to be used.
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return 1;
    }
    
    printf("\nUsing address %d.\n\n", address);
    
    // Open a connection to the device.
    result = mcc152_open(address);
    print_error(result);
    if (result != RESULT_SUCCESS)
    {
        // Could not open the device - exit.
        printf("Unable to open device at address %d\n", address);
        return 1;
    }

    // Reset the DIO to defaults (all channels input, pull-up resistors
    // enabled).
    result = mcc152_dio_reset(address);
    print_error(result);
    
    // Set all channels as outputs.
    result = mcc152_dio_config_write_port(address, DIO_DIRECTION, 0x00);
    print_error(result);
    
    error = false;
    run_loop = true;
    // Loop until the user terminates or we get a library error.
    while (run_loop && !error)
    {
        if (get_input(&value))
        {
            // Write the values to the outputs.
            result = mcc152_dio_output_write_port(address, value);
            if (result != RESULT_SUCCESS)
            {
                // We got an error from the library.
                print_error(result);
                error = true;
            }
        }
        else
        {
            run_loop = false;
        }
    }

    if (!error)
    {
        // Return the digital I/O to default settings.
        result = mcc152_dio_reset(address);
        print_error(result);
    }
    
    // Close the device.
    result = mcc152_close(address);
    print_error(result);

    return (int)error;
}
