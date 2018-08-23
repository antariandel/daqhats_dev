/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_dio_reset
        mcc152_dio_input_read_bit
        mcc152_info

    Purpose:
        Read individual digital inputs until terminated by the user.

    Description:
        This example demonstrates using the digital I/O as inputs and reading
        them individually.

*****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "../../daqhats_utils.h"

#define BUFFER_SIZE 5

int main(void)
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    bool error;
    bool run_loop;
    uint8_t value;
    int num_channels;
    int channel;
    char buffer[BUFFER_SIZE];

    printf("\nMCC 152 digital input read example.\n");
    printf("Reads the inputs individually and displays their state.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_dio_reset\n");
    printf("      mcc152_dio_input_read_bit\n");
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

    num_channels = mcc152_info()->NUM_DIO_CHANNELS;
    
    // Reset the DIO to defaults (all channels input, pull-up resistors
    // enabled).
    result = mcc152_dio_reset(address);
    print_error(result);
    
    error = false;
    run_loop = true;
    // Loop until the user terminates or we get a library error.
    while (run_loop && !error)
    {
        // Read and display the individual channels.
        for (channel = 0; (channel < num_channels) && !error; channel++)
        {
            result = mcc152_dio_input_read_bit(address, channel, &value);
            if (result != RESULT_SUCCESS)
            {
                // We got an error from the library.
                print_error(result);
                error = true;
                continue;
            }
            else
            {
                printf("DIO%d: %d\t", channel, value);
            }
        }
        
        if (!error)
        {
            printf("\nEnter Q to exit, anything else to read again: ");
            
            // Read a string from the user
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
            {
                if ((buffer[0] == 'Q') || (buffer[0] == 'q'))
                {
                    // Exit the loop
                    run_loop = false;
                    continue;
                }
            }
        }
    }

    // Close the device.
    result = mcc152_close(address);
    print_error(result);

    return (int)error;
}
