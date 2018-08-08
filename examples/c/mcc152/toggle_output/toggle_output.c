#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <daqhats/daqhats.h>

int main(int argc, char* argv[])
{
    int write_count;
    int count;
    uint8_t address;
    uint8_t channel;
    uint8_t value;
    struct timespec start_time;
    struct timespec end_time;
    double elapsed_time;
    double rate;

    // Use MCC 152 at address 0
    address = 0;
    channel = 0;
    write_count = 50;

    if (mcc152_open(address) != RESULT_SUCCESS)
    {
        printf("No MCC 152 at address 0\n");
    }
    // reset to defaults
    mcc152_dio_reset(address);

    // set the channel to output
    mcc152_dio_config_write_bit(address, channel, DIO_DIRECTION, 0);

    printf("Toggling %d times...\n", write_count);

    // Write channel 0 write_count times and calculate the rate
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    value = 0;
    for (count = 0; count < write_count; count++)
    {
        mcc152_dio_output_write_bit(address, channel, value);
        value = ~value;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = ((double)end_time.tv_sec + (double)end_time.tv_nsec/1e9) -
                   ((double)start_time.tv_sec + (double)start_time.tv_nsec/1e9);
    rate = (double)write_count / elapsed_time;
    printf("Rate: %f\n", rate);

    // return to default settings
    mcc152_dio_reset(address);
    mcc152_close(address);
    return 0;
}
