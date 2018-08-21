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
    double *values;
    double min;
    double max;
    bool error;
    bool run_loop;
    bool valid_value;
    int channel;

    printf("\nMCC 152 all channel analog output example.\n");
    printf("Writes the specified voltages to the analog outputs.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_a_out_write_all\n");
    printf("      mcc152_info\n\n");

    // Select the device to be used.
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return 1;
    }
    
    printf("\nUsing address %d.\n", address);
    
    // Open a connection to the device.
    result = mcc152_open(address);
    print_error(result);
    if (result != RESULT_SUCCESS)
    {
        // Could not open the device - exit.
        printf("Unable to open device at address %d\n", address);
        return 1;
    }
        
    // Get the min and max voltage values for the analog outputs to validate
    // the user input.
    min = mcc152_info()->AO_MIN_RANGE;
    max = mcc152_info()->AO_MAX_RANGE;
    // Allocate memory for the values.
    values = (double*)malloc(mcc152_info()->NUM_AO_CHANNELS * sizeof(double));
    if (values == NULL)
    {
        mcc152_close(address);
        printf("Could not allocate memory.\n");
        return 1;
    }
    error = false;
    run_loop = true;
    // Loop until the user terminates or we get a library error.
    while (run_loop && !error)
    {
        printf("Enter voltages between %.1f and %.1f, non-numeric character to"
            " exit:\n", min, max);
            
        for (channel = 0; 
             (channel < mcc152_info()->NUM_AO_CHANNELS) && run_loop; 
             channel++)
        {
            do
            {
                // Loop if the user enters an out of range value.
                valid_value = true;
                
                // Get the channel value from the user as a string.
                printf("   Ch %d: ", channel);

                if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
                {
                    // Empty string or input error.
                    run_loop = false;
                    break;
                }
                else
                {
                    if (buffer[0] == '\n')
                    {
                        // Enter.
                        run_loop = false;
                        break;
                    }
                    
                    if (sscanf(buffer, "%lf", &values[channel]) == 0)
                    {
                        // Not a number.
                        run_loop = false;
                        break;
                    }
                        
                    // Compare the number to min and max allowed.
                    if ((values[channel] < min) || (values[channel] > max))
                    {
                        // Out of range, ask again.
                        printf("Value out of range.\n");
                        valid_value = false;
                        continue;
                    }
                }
            } while (!valid_value);
        }

        // If the user entered valid values then update the outputs.
        if (run_loop)
        {
            result = mcc152_a_out_write_all(address, OPTIONS, values);
            if (result != RESULT_SUCCESS)
            {
                print_error(result);
                error = true;
                break;
            }
        }
    }

    // If there was no library error reset the output to 0V.
    if (!error)
    {
        for (channel = 0; channel < mcc152_info()->NUM_AO_CHANNELS; channel++)
        {
            values[channel] = 0.0;
        }
        result = mcc152_a_out_write_all(address, OPTIONS, values);
        print_error(result);
    }
    
    // Close the device.
    result = mcc152_close(address);
    print_error(result);
    
    // Free allocated memory.
    free(values);
    
    return (int)error;
}
