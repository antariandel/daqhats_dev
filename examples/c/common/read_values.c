#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <daqhats/daqhats.h>

#define KCLR  "\x1B[2J\x1B[1;1H"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

int keep_running;

/* Signal Handler for SIGINT */
void sigint_handler(int sig_num)
{
    /* Do a graceful cleanup of the program like: free memory/resources/etc and exit */
    keep_running = 0;
}

size_t snprintfcat(char* buf, size_t bufSize, char const* fmt, ...)
{
    size_t result;
    va_list args;
    size_t len = strnlen(buf, bufSize);
    
    va_start(args, fmt);
    result = vsnprintf(buf + len, bufSize - len, fmt, args);
    va_end(args);
    
    return result + len;
}

int main(int argc, char* argv[])
{
    int info_count;
    int index;
    struct HatInfo* info_list;
    double value;
    uint8_t address;
    uint8_t channel;
    char disp_buffer[2048];
    
    // get list of MCC HATs
    info_count = hat_list(HAT_ID_ANY, NULL);
    
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
    
    printf("Initializing...\n");
    
    // open the boards
    for (index = 0; index < info_count; index++)
    {
        address = info_list[index].address;
        if (info_list[index].id == HAT_ID_MCC_118)
        {
            mcc118_open(address);
        }
        else if (info_list[index].id == HAT_ID_MCC_134)
        {
            mcc134_open(address);
        }
    }
    
    keep_running = 1;
    signal(SIGINT, sigint_handler);
    
    // Update the screen every second
    while (keep_running)
    {
        sprintf(disp_buffer,         "%sBoard      Channel Values\n", KCLR);
        snprintfcat(disp_buffer, 2048, "---------  --------------\n");
        
        // display analog input values for every board
        for (index = 0; index < info_count; index++)
        {
            address = info_list[index].address;
            if (info_list[index].id == HAT_ID_MCC_118)
            {
                snprintfcat(disp_buffer, 2048,  "%d-MCC 118  ", address);
                for (channel = 0; channel < mcc118_a_in_num_channels(); channel++)
                {
                    mcc118_a_in_read(address, channel, 0, &value);
                    snprintfcat(disp_buffer, 2048,  "%d: %6.2f ", channel, value);
                }
            }
            else if (info_list[index].id == HAT_ID_MCC_134)
            {
                snprintfcat(disp_buffer, 2048,  "%d-MCC 134  ", address);
                for (channel = 0; channel < mcc134_a_in_num_channels(); channel++)
                {
                    mcc134_t_in_read(address, channel, &value);
                    if (value == OPEN_TC_VALUE)
                    {
                        snprintfcat(disp_buffer, 2048, "%d:   %sOpen%s ", channel, KRED, KNRM);
                    }
                    else if (value == OVERRANGE_TC_VALUE)
                    {
                        snprintfcat(disp_buffer, 2048, "%d:   %sOver%s ", channel, KYEL, KNRM);
                    }
                    else
                    {
                        snprintfcat(disp_buffer, 2048, "%d: %6.2f ", channel, value);
                    }
                }
            }
            
            snprintfcat(disp_buffer, 2048, "\n");
        }
        
        //system("clear");
        printf("%s", disp_buffer);
        
        sleep(1);
    }

    // close the boards
    for (index = 0; index < info_count; index++)
    {
        address = info_list[index].address;
        if (info_list[index].id == HAT_ID_MCC_118)
        {
            mcc118_close(address);
        }
        else if (info_list[index].id == HAT_ID_MCC_134)
        {
            mcc134_close(address);
        }
    }

    free(info_list);
    return 0;
}
