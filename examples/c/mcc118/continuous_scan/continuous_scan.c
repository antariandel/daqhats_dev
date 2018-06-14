#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <daqhats/daqhats.h>

int read_args(int argc, char **argv, uint8_t *address, int *channel);
int print_error(int result);
int enter_press(void);

#define BUF_SIZE 1000

int main(int argc, char **argv)
{
	int channel = 0,
		ret_val = 0,
		hat_count = 0,
		hat_idx = 0,
		a_in_chan_count = 0,
		read_result = RESULT_SUCCESS;
	uint8_t address = 0;
	uint16_t read_status = 0;
	uint32_t samples_read_per_channel = 0;
	double read_buf[BUF_SIZE];
	
	if(read_args(argc, argv, &address, &channel) != 0) {
		fprintf(stderr, 
			"Usage: %s [-a address] [-c channel]\n"
			"    -a address: address [0-7] of the HAT to scan (default is 0)\n"
			"    -c channel: channel number to scan (default is 0)\n", argv[0]);
		return 1;
	}

	hat_count = hat_list(HAT_ID_ANY, NULL);

	if (hat_count > 0)
	{
		struct HatInfo* hats = (struct HatInfo*)malloc(hat_count * sizeof(struct HatInfo));
		hat_list(HAT_ID_ANY, hats);
		hat_idx = 0;
		while(hat_idx < hat_count && hats[hat_idx].address != address){
			hat_idx++;
		}
		if(hat_idx >= hat_count) {
			fprintf(stderr, "Invalid address specified.  %d address does not exist.\n", address);
			ret_val = 1;
		}
		else if(hats[hat_idx].id != HAT_ID_MCC_118) {
			fprintf(stderr, "Invalid address specified.  Device at address %d is not type MCC118.\n", address);
			ret_val = 1;
		}
		else {
			a_in_chan_count = mcc118_a_in_num_channels();
			if(channel >= a_in_chan_count || channel < 0) {
				fprintf(stderr, "Invalid channel <%d> specified.  Valid values are 0-%d.\n", channel, a_in_chan_count-1);
				ret_val = 1;
			}
			else {
				/* Open the device */
				if(print_error(mcc118_open(address)) != RESULT_SUCCESS) {
					ret_val = 1;
				}
				else {
					/* Configure the scan */
					if(print_error(mcc118_a_in_scan_start(address, 0x01 << channel, BUF_SIZE, 100.0, OPTS_CONTINUOUS)) != 0) {
						ret_val = 1;
					}
					else {
						/* Display the device name and address */
						printf("Address %d: %s\n", address, hats[hat_idx].product_name);
						
						/* Continuously update the display value until enter key is pressed */
						printf("Scanning Channel %d ... Press Enter to stop\n", channel);
						do {
							if(samples_read_per_channel > 0) {
								printf("\r%10.5f V", read_buf[samples_read_per_channel - 1]);
								fflush(stdout);
							}

							sleep(1);

							read_result = print_error(mcc118_a_in_scan_read(address, &read_status, -1, 0, read_buf, BUF_SIZE, &samples_read_per_channel));
							
						} while(read_result == RESULT_SUCCESS && (read_status & STATUS_RUNNING) == STATUS_RUNNING && !enter_press());
                        
                        if (read_status & STATUS_HW_OVERRUN)
                        {
                            printf("Hardware overrun\n");
                        }
                        else if (read_status & STATUS_BUFFER_OVERRUN)
                        {
                            printf("Buffer overrun\n");
                        }
					}
					print_error(mcc118_a_in_scan_stop(address));
					print_error(mcc118_a_in_scan_cleanup(address));
				}
				print_error(mcc118_close(address));
			}
		}
		free(hats);
	}
	else {
		fprintf(stderr, "No MCC HAT device found.\n");
		ret_val = 1;
	}
	
	return ret_val;
}

int read_args(int argc, char **argv, uint8_t *address, int *channel) {
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "ha:c:")) != -1) {
		switch (c) {
			case 'a':
				*address = atoi(optarg);
				break;
			case 'c':
				*channel = atoi(optarg);
				break;
			case 'h':
				return 1;
				break;
			case '?':
				if (optopt == 'c' || optopt == 'a')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,  "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort ();
		}
	}

	return 0;
}

int print_error(int result) {
	switch(result) {
		case RESULT_SUCCESS:
			break;
		case RESULT_BAD_PARAMETER:
			fprintf(stderr, "\nA parameter passed to the function was incorrect.\n");
			break;
		case RESULT_BUSY:
			fprintf(stderr, "\nThe device is busy.\n");
			break;
		case RESULT_TIMEOUT:
			fprintf(stderr, "\nThere was a timeout accessing a resource.\n");
			break;
		case RESULT_LOCK_TIMEOUT:
			fprintf(stderr, "\nThere was a timeout while obtaining a resource lock.\n");
			break;
		case RESULT_INVALID_DEVICE:
			fprintf(stderr, "\nThe device at the specified address is not the correct type.\n");
			break;
		case RESULT_RESOURCE_UNAVAIL:
			fprintf(stderr, "\nA needed resource was not available.\n");
			break;
		default:
		case RESULT_UNDEFINED:
			fprintf(stderr, "\nAn unknown error occrred.\n");
			break;
	}
	return result;
}

int enter_press()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}


