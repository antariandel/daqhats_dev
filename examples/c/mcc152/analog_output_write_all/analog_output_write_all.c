/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_a_out_write_all
        mcc152_info

    Purpose:
        Write values to both analog outputs in a loop.

    Description:
        This example demonstrates writing output data to both outputs
        simultaneously.

*****************************************************************************/
#include <stdbool.h>
#include "../../daqhats_utils.h"

#define OPTIONS             OPTS_DEFAULT    // default output options
#define BUFFER_SIZE         32

int main()
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    char buffer[BUFFER_SIZE];
    double values[2];
    double min;
    double max;
    bool error;

    printf("\nMCC 152 all channel analog output example.\n");
    printf("Writes the specified voltages to the analog outputs.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_a_out_write_all\n");
    printf("      mcc152_info\n");
    printf("\nMCC 152s detected:\n\n");

    // Select the device to be used
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return 1;
    }
    
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
        // Get the values from the user
        printf("Enter voltages between %.1f and %.1f, non-numeric character to exit:\n",
            min, max);
            
        printf("   Ch 0: ");

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
            
            if (sscanf(buffer, "%lf", &values[0]) == 0)
            {
                // Not a number
                break;
            }
            
            if ((values[0] < min) || (values[0] > max))
            {
                printf("Value out of range.\n");
                continue;
            }
            
        }

        printf("   Ch 1: ");

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
            
            if (sscanf(buffer, "%lf", &values[1]) == 0)
            {
                // Not a number
                break;
            }
            
            if ((values[1] < min) || (values[1] > max))
            {
                printf("Value out of range.\n");
                continue;
            }
            
        }

        // Write the values
        result = mcc152_a_out_write_all(address, OPTIONS, values);
        if (result != RESULT_SUCCESS)
        {
            print_error(result);
            error = true;
            break;
        }
    }

    if (!error)
    {
        values[0] = values[1] = 0.0;
        result = mcc152_a_out_write_all(address, OPTIONS, values);
        print_error(result);
    }
    
    result = mcc152_close(address);
    print_error(result);

    return (int)error;
}
