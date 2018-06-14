#include <stdio.h>
#include <stdlib.h>
#include <daqhats/daqhats.h>

int main(int argc, char* argv[])
{
    int info_count;
    int index;
    struct HatInfo* info_list;
    double value;
    uint8_t address;
    uint8_t channel;

    // get list of MCC 118s
    info_count = hat_list(HAT_ID_MCC_118, NULL);

    if (info_count > 0)
    {
        info_list = (struct HatInfo*)malloc(info_count * sizeof(struct HatInfo));
        hat_list(HAT_ID_ANY, info_list);
    }
    else
    {
        printf("hat_list returned %d\n", info_count);
        return 1;
    }

    // display analog input values for every board
    for (index = 0; index < info_count; index++)
    {
        if (info_list[index].id == HAT_ID_MCC_118)
        {
            address = info_list[index].address;

            printf("Board: %d\n", address);
            mcc118_open(address);
            for (channel = 0; channel < mcc118_a_in_num_channels(); channel++)
            {
                mcc118_a_in_read(address, channel, 0, &value);
                printf("   Channel %d: %7.3f\n", channel, value);
            }
            mcc118_close(address);
        }

        printf("\n");
    }

    free(info_list);
    return 0;
}
