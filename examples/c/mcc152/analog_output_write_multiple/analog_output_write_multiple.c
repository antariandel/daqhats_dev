/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_a_out_write_all

    Purpose:
        Write values to both analog outputs in a loop.

    Description:
        This example demonstrates writing output data using a software timed
        loop to generate a waveform on both analog outputs.

*****************************************************************************/
#include <math.h>
#include "../../daqhats_utils.h"

#define WAVEFORM_LENGTH     32      // number of samples in the waveform
#define PERIOD              0.1     // period of the waveform in seconds

int main()
{
    uint8_t address;
    uint32_t options = OPTS_DEFAULT;
    double waveform_0[WAVEFORM_LENGTH];
    double waveform_1[WAVEFORM_LENGTH];
    double write_data[2];
    int index;
    int update_interval_ms;
    int result = RESULT_SUCCESS;
    char c;

    // Determine the address of the device to be used
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return -1;
    }
    
    // Create a 32-point sine waveform for output 0 and a cosine waveform for
    // output 1.
    for (index = 0; index < WAVEFORM_LENGTH; index++)
    {
        waveform_0[index] = mcc152_a_out_voltage_max() * 
            ((sin(((double)index / WAVEFORM_LENGTH) * 2 * M_PI) + 1) / 2);
        waveform_1[index] = mcc152_a_out_voltage_max() * 
            ((cos(((double)index / WAVEFORM_LENGTH) * 2 * M_PI) + 1) / 2);
    }

    // determine the interval between updates
    update_interval_ms = (int)((double)PERIOD * 1000.0 / WAVEFORM_LENGTH + 0.5);

    // Open a connection to the device
    result = mcc152_open(address);
    STOP_ON_ERROR(result);

    printf("\nMCC 152 analog output example - all channel update.\n");
    printf("Outputs a sine wave to analog output 0 and a cosine wave to analog\n"
           "output 1.\n");
    printf("    Function demonstrated: mcc152_a_out_write_all\n");
    printf("\nPress 'Enter' to continue\n");

    scanf("%c", &c);

    printf("Writing data ... Press 'Enter' to abort\n\n");

    index = 0;
    while (!enter_press())
    {
        // Write a value to the selected channel
        write_data[0] = waveform_0[index];
        write_data[1] = waveform_1[index];
        result = mcc152_a_out_write_all(address, options, write_data);
        STOP_ON_ERROR(result);

        if (++index >= WAVEFORM_LENGTH)
        {
            index = 0;
        }
        usleep(update_interval_ms * 1000);
    }

stop:
    result = mcc152_close(address);
    print_error(result);

    return 0;
}
