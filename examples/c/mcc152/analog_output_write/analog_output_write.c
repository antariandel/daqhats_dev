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
#define OPTIONS             OPTS_DEFAULT    // default output options (voltage)
#define BUFFER_SIZE         32

bool get_input_value(double* p_value)
{
    char buffer[BUFFER_SIZE];
    double min;
    double max;
    double value;

    if (p_value == NULL)
    {
        return false;
    }
    
    // Get the min and max voltage values for the analog outputs to validate
    // the user input.
    min = mcc152_info()->AO_MIN_RANGE;
    max = mcc152_info()->AO_MAX_RANGE;

    while (1)
    {
        printf("Enter a voltage between %.1f and %.1f, non-numeric character to"
            " exit: ", min, max);

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
            
            // Got a string - convert to a number.
            if (sscanf(buffer, "%lf", &value) == 0)
            {
                // Not a number.
                return false;
            }
            
            // Compare the number to min and max allowed.
            if ((value < min) || (value > max))
            {
                // Out of range, ask again.
                printf("Value out of range.\n");
            }
            else
            {
                // Valid value.
                *p_value = value;
                return true;
            }
        }
    }
}

int main(void)
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    double value;
    bool error;
    bool run_loop;

    printf("\nMCC 152 single channel analog output example.\n");
    printf("Writes the specified voltage to the analog output.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_a_out_write\n");
    printf("      mcc152_info\n");
    printf("   Channel: %d\n\n", CHANNEL);

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
        
    error = false;
    run_loop = true;
    // Loop until the user terminates or we get a library error.
    while (run_loop && !error)
    {
        // Get the value from the user as a string.
        if (get_input_value(&value))
        {
            // Write the value to the selected channel.
            result = mcc152_a_out_write(address, CHANNEL, OPTIONS, value);
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

    // If there was no library error reset the output to 0V.
    if (!error)
    {
        result = mcc152_a_out_write(address, CHANNEL, OPTIONS, 0.0);
        print_error(result);
    }
    
    // Close the device.
    result = mcc152_close(address);
    print_error(result);

    return (int)error;
}
