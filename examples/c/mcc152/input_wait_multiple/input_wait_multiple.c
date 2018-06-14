#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <daqhats/daqhats.h>

// Use MCC 152 boards at addresses 0-3
#define NUM_BOARDS  4
const uint8_t addresses[NUM_BOARDS] =
{
    0, 1, 2, 3
};

int main(int argc, char* argv[])
{
    uint8_t address;
    uint8_t channel;
    uint8_t value;
    uint8_t status;
    uint8_t mask;
    int i;

    for (i = 0; i < NUM_BOARDS; i++)
    {
        address = addresses[i];
        
        if (mcc152_open(address) != RESULT_SUCCESS)
        {
            printf("No MCC 152 at address %d\n", address);
            return 1;
        }

        // reset to default DIO settings
        mcc152_dio_reset(address);
        // read initial value
        mcc152_dio_input_read(address, DIO_CHANNEL_ALL, &value);
        // enable interrupts on all inputs
        mcc152_dio_interrupt_mask_write(address, DIO_CHANNEL_ALL, 0x00);
    }

    printf("Waiting for change\n");
    
    // Wait for any input on any board to change.
    while (1)
    {
        // wait forever for interrupt
        hat_wait_for_interrupt(-1);
        // interrupt occurred, determine the source
        for (i = 0; i < NUM_BOARDS; i++)
        {
            address = addresses[i];
            
            mcc152_dio_interrupt_status_read(address, DIO_CHANNEL_ALL, &status);
            if (status != 0)
            {
                mcc152_dio_input_read(address, DIO_CHANNEL_ALL, &value);
                for (channel = 0; channel < 8; channel++)
                {
                    mask = (1 << channel);
                    if ((status & mask) != 0)
                    {
                        printf("Address %d channel %d changed to %d\n", address, channel, (value & mask) != 0);
                    }
                }
            }
        }
    }
    
    mcc152_close(address);
    return 0;
}
