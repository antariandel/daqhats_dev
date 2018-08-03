/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_a_out_write

    Purpose:
        Write values to analog output 0 in a loop.

    Description:
        This example demonstrates writing output data using a software timed
        loop to generate a waveform on analog output 0.

*****************************************************************************/
#include <math.h>
#include "../../daqhats_utils.h"

#define WAVEFORM_LENGTH     32      // number of samples in the waveform
#define PERIOD              0.1     // period of the waveform in seconds
#define CHANNEL             0       // update channel
#define OPTIONS             OPTS_DEFAULT    // default output options

int main()
{
    uint8_t address;
    double waveform[WAVEFORM_LENGTH];
    int index;
    int update_interval_ms;
    int result = RESULT_SUCCESS;
    char c;

    // Determine the address of the device to be used
    if (select_hat_device(HAT_ID_MCC_152, &address) != 0)
    {
        return -1;
    }
    
    // Create a 32-point sine waveform for the MCC 152 output
    for (index = 0; index < WAVEFORM_LENGTH; index++)
    {
        waveform[index] = mcc152_info()->AO_MAX_VOLTAGE * 
            ((sin(((double)index / WAVEFORM_LENGTH) * 2 * M_PI) + 1.0) / 2.0);
    }

    // determine the interval between updates
    update_interval_ms = (int)((double)PERIOD * 1000.0 / WAVEFORM_LENGTH + 0.5);

    // Open a connection to the device
    result = mcc152_open(address);
    STOP_ON_ERROR(result);

    printf("\nMCC 152 single data value analog output example.\n");
    printf("Outputs a sine wave to the analog output.\n");
    printf("    Function demonstrated: mcc152_a_out_write\n");
    printf("    Channel: %d\n", CHANNEL);
    printf("\nPress 'Enter' to continue\n");

    scanf("%c", &c);

    printf("Writing data ... Press 'Enter' to abort\n\n");

    index = 0;
    while (!enter_press())
    {
        // Write a value to the selected channel
        result = mcc152_a_out_write(address, CHANNEL, OPTIONS, waveform[index]);
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
