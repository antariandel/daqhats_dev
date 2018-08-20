/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_a_out_write
        mcc152_info

    Purpose:
        Write values to analog output 0 in a loop.

    Description:
        This example demonstrates writing output data using analog output 0.

*****************************************************************************/
#include <stdbool.h>
#include "../../daqhats_utils.h"

#define CHANNEL             0               // output channel
#define OPTIONS             OPTS_DEFAULT    // default output options
#define BUFFER_SIZE         32

int main()
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    char buffer[BUFFER_SIZE];
    double value;
    double min;
    double max;
    bool error;

    printf("\nMCC 152 single channel analog output example.\n");
    printf("Writes the specified voltage to the analog output.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_a_out_write\n");
    printf("      mcc152_info\n");
    printf("   Channel: %d\n\n", CHANNEL);

    // Select the device to be used
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return 1;
    }
    
    printf("\nUsing address %d.\n", address);
    
    // Open a connection to the device
    result = mcc152_open(address);
    if (result != RESULT_SUCCESS)
    {
        printf("Unable to open device at address %d\n", address);
        print_error(result);
        return 1;
    }
        
    min = mcc152_info()->AO_MIN_RANGE;
    max = mcc152_info()->AO_MAX_RANGE;
    error = false;
    while (1)
    {
        // Get the value from the user
        printf("Enter a voltage between %.1f and %.1f, non-numeric character to exit: ",
            min, max);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            // empty string or error
            break;
        }
        else
        {
            if (buffer[0] == '\n')
            {
                // Enter
                break;
            }
            
            if (sscanf(buffer, "%lf", &value) == 0)
            {
                // Not a number
                break;
            }
            
            if ((value < min) || (value > max))
            {
                printf("Value out of range.\n");
                continue;
            }
            
            // Write a value to the selected channel
            result = mcc152_a_out_write(address, CHANNEL, OPTIONS, value);
            if (result != RESULT_SUCCESS)
            {
                print_error(result);
                error = true;
                break;
            }
        }
    }

    if (!error)
    {
        result = mcc152_a_out_write(address, CHANNEL, OPTIONS, 0.0);
        print_error(result);
    }
    
    result = mcc152_close(address);
    print_error(result);

    return (int)error;
}
