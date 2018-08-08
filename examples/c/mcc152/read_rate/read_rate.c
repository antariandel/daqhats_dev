#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <daqhats/daqhats.h>

int main(int argc, char* argv[])
{
    int read_count;
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
    read_count = 10000;

    if (mcc152_open(address) != RESULT_SUCCESS)
    {
        printf("No MCC 152 at address 0\n");
    }
    // reset to defaults
    mcc152_dio_reset(address);
    
    printf("Reading %d samples...\n", read_count);

    // Read channel 0 10000 times and calculate the rate
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (count = 0; count < read_count; count++)
    {
        mcc152_dio_input_read_bit(address, channel, &value);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = ((double)end_time.tv_sec + (double)end_time.tv_nsec / 1e9) -
                   ((double)start_time.tv_sec + (double)start_time.tv_nsec / 1e9);
    rate = (double)read_count / elapsed_time;
    printf("Rate: %f\n", rate);

    mcc152_close(address);
    return 0;
}
