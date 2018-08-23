/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_dio_input_read_port
        mcc152_info

    Purpose:
        Read all digital inputs in a single calluntil terminated by the user.

    Description:
        This example demonstrates using the digital I/O as inputs and reading
        them in a port read.

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
    char buffer[BUFFER_SIZE];

    printf("\nMCC 152 digital input read example.\n");
    printf("Reads the inputs as a port and displays their state.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_dio_input_read_port\n\n");

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
    
    error = false;
    run_loop = true;
    // Loop until the user terminates or we get a library error.
    while (run_loop && !error)
    {
        // Read and display the individual channels.
        result = mcc152_dio_input_read_port(address, &value);
        if (result != RESULT_SUCCESS)
        {
            // We got an error from the library.
            print_error(result);
            error = true;
            continue;
        }
        
        printf("Digital inputs: 0x%02X\n", value);
        
        if (!error)
        {
            printf("Enter Q to exit, anything else to read again: ");
            
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
