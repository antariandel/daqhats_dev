/*****************************************************************************

    MCC 152 Functions Demonstrated:
        mcc152_dio_reset
        mcc152_dio_config_write_port
        mcc152_dio_input_read_port
        mcc152_dio_int_status_read_port
        mcc152_info
        hat_interrupt_callback_enable
        hat_interrupt_callback_disable

    Purpose:
        Configure the inputs to interrupt on change then wait for changes.

    Description:
        This example demonstrates using the digital I/O as inputs and enable
        interrupts on change.  It waits for changes on any input and displays
        the change.

*****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "../../daqhats_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 5

void interrupt_callback(void* data)
{
    uint8_t value;
    uint8_t status;
    int result;
    uint8_t address;
    int i;
    
    // The board address must be passed using data.
    if (data != NULL)
    {
        address = *(uint8_t*)data;
    }
    else
    {
        return;
    }

    // An interrupt occurred, make sure we were the source.
    result = mcc152_dio_int_status_read_port(address, &status);
    print_error(result);
    
    // Read the inputs to clear the active interrupt.
    result = mcc152_dio_input_read_port(address, &value);
    print_error(result);
    
    if (status != 0)
    {
        printf("Input channels that changed: ");
        for (i = 0; i < mcc152_info()->NUM_DIO_CHANNELS; i++)
        {
            if ((status & (1 << i)) != 0)
            {
                printf("%d ", i);
            }
        }
        printf("\nCurrent port value: 0x%02X\n", value);
    }
}

int main(int argc, char* argv[])
{
    uint8_t address;
    int result = RESULT_SUCCESS;
    uint8_t value;
    char buffer[BUFFER_SIZE];

    printf("\nMCC 152 digital input interrupt example.\n");
    printf("Enables interrupts on the inputs and displays their state when");
    printf("they change.\n");
    printf("   Functions demonstrated:\n");
    printf("      mcc152_dio_reset\n");
    printf("      mcc152_dio_config_write_port\n");
    printf("      mcc152_dio_input_read_port\n");
    printf("      mcc152_dio_int_status_read_port\n");
    printf("      mcc152_info\n");
    printf("      hat_interrupt_callback_enable\n");
    printf("      hat_interrupt_callback_disable\n");

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

    // Read the initial input values so we don't trigger an interrupt when 
    // we enable them.
    result = mcc152_dio_input_read_port(address, &value);
    print_error(result);
    
    // Enable latched inputs so we know that a value changed even if it changes
    // back to the original value before the interrupt callback.
    result = mcc152_dio_config_write_port(address, DIO_INPUT_LATCH, 0xFF);
    print_error(result);

    // Unmask (enable) interrupts on all channels.
    result = mcc152_dio_config_write_port(address, DIO_INT_MASK, 0x00);
    print_error(result);

    printf("Current input values are 0x%02X\n", value);
    printf("Waiting for changes, enter any text to exit.\n");

    // Enable an interrupt callback function.
    result = hat_interrupt_callback_enable(interrupt_callback, &address);
    print_error(result);
    
    // Wait for the user to enter anything, then exit.
    fgets(buffer, BUFFER_SIZE, stdin);

    // Return the digital I/O to default settings.
    result = mcc152_dio_reset(address);
    print_error(result);

    // Disable the interrupt callback.
    hat_interrupt_callback_disable();
    
    // Close the device.
    result = mcc152_close(address);
    print_error(result);

    return 0;
}
