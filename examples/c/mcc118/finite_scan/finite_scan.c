#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <daqhats/daqhats.h>

const char* filename = "test.csv";

int main(int argc, char* argv[])
{
    uint32_t samples_per_channel;
    uint32_t samples_read_per_channel;
    int num_channels;
    uint8_t address;
    uint8_t channels;
    uint8_t options;
    uint16_t status;
    double* data;
    double sample_rate_per_channel = 1000;
    FILE* logfile;
    int index;
    int channel;
    int info_count;
    struct HatInfo* info_list;

    // get device list
    info_count = hat_list(HAT_ID_MCC_118, NULL);

    if (info_count > 0)
    {
        info_list = (struct HatInfo*)malloc(info_count * sizeof(struct HatInfo));
        hat_list(HAT_ID_MCC_118, info_list);
    }
    else
    {
        printf("No MCC 118s found\n");
        return 1;
    }

    // Use the first MCC 118 found
    address = info_list[0].address;
    free(info_list);
    printf("Using MCC 118 at address %d\n", address);
    
    // Read channels 0 and 1
    num_channels = 2;
    channels = (1 << 0) | (1 << 1);
    samples_per_channel = 1000;
    options = 0;

    if (mcc118_open(address) != RESULT_SUCCESS)
    {
        printf("mcc118_open failed\n");
        mcc118_close(address);
        return 1;
    }

    printf("Scanning %d samples...\n", samples_per_channel);
    if (mcc118_a_in_scan_start(address, channels, samples_per_channel, sample_rate_per_channel, options)
        != RESULT_SUCCESS)
    {
        printf("mcc118_a_in_scan_start failed\n");
        mcc118_close(address);
        return 1;
    }    
    
    // wait for scan to complete
    do
    {
        if (mcc118_a_in_scan_status(address, &status, NULL)
            != RESULT_SUCCESS)
        {
            printf("mcc118_a_in_scan_status failed\n");
            mcc118_a_in_scan_stop(address);
            mcc118_a_in_scan_cleanup(address);
            mcc118_close(address);
            return 1;
        }
        usleep(1000);
    } while ((status & STATUS_RUNNING) == STATUS_RUNNING);

    // read all the data at once
    data = (double*)malloc(samples_per_channel * num_channels * sizeof(double));
    if (mcc118_a_in_scan_read(address, &status, samples_per_channel, 0.0, data, samples_per_channel * num_channels, &samples_read_per_channel)
        != RESULT_SUCCESS)
    {
        printf("mcc118_a_in_scan_read failed\n");
        mcc118_a_in_scan_stop(address);
        mcc118_a_in_scan_cleanup(address);
        mcc118_close(address);
        return 1;
    }
    printf("Read %d samples\n", samples_read_per_channel);
    mcc118_a_in_scan_stop(address);
    mcc118_a_in_scan_cleanup(address);
    mcc118_close(address);

    printf("Saving data to %s\n", filename);
    logfile = fopen(filename, "wt");
    if (logfile == NULL)
    {
        printf("Can't open %s\n", filename);
        free(data);
        return 1;
    }
    for (index = 0; index < samples_read_per_channel; index++)
    {
        for (channel = 0; channel < num_channels; channel++)
        {
            fprintf(logfile, "%f,", data[(index * num_channels) + channel]);
        }
        fprintf(logfile, "\n");
    }
    
    fclose(logfile);
    free(data);
    return 0;
}
