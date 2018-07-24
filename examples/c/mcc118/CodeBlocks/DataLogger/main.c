#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gtkdatabox.h"
#include "gtkdatabox_lines.h"
#include "gtkdatabox_ruler.h"
#include "gtkdatabox_grid.h"


#include "globals.h"
#include "daqhats/daqhats.h"
#include "log_file.h"
#include "errors.h"

#define OPTS_BACKGROUND 0
#define MAX_118_CHANNELS 8

void start_stop_event_handler(GtkWidget *widget, gpointer data);

int loop_count = 0;

/*******************************************************************************
    Function:               show_window_title

    Description:            Display the window title by combining the
                            application name and the CSV file name.

    Parameters:
        NONE

    Return Value:
        NONE
*******************************************************************************/
void show_window_title()
{
    char directory[512] = {'\0'};
    char filename[512] = {'\0'};
    char title[512] = {'\0'};

    // Initialize the title with the application name
    strcpy(title, "");
    strcat(title, application_name);

    // Add the filename to the title
    strcat(title, " - ");

    get_path_and_filename(csv_filename, directory, filename);

    strcat(title, directory);
    strcat(title, filename);

    // Set the window title
    gtk_window_set_title((GtkWindow*)window, title);
}

/*******************************************************************************
    Function:               open_first_hat_device

    Description:            Find all of the HAT devices that are installed, and
                            open a connection to the first onw.

    Parameters:
        NONE

    Return Value:
        address             The address of the HAT devic that was opened.
*******************************************************************************/
uint8_t open_first_hat_device()
{
    uint8_t hat_address;
    int hat_count;
    struct HatInfo* hat_info_list;
    int retval = 0;

    // Get the number of MCC 118 devices so that we can determine
    // how much memory to allocate for the hat_info_list
    hat_count = hat_list(HAT_ID_MCC_118, NULL);
    printf("hat_list returned %d\n", hat_count);

    if (hat_count > 0)
    {
        // Allocae memory for the hat_info_list
        hat_info_list = (struct HatInfo*)malloc(
            hat_count * sizeof(struct HatInfo));

        // Get list of MCC 118s.
        hat_list(HAT_ID_MCC_118, hat_info_list);

        // This application will use the first device (i.e. hat_info_list[0])
        hat_address = hat_info_list[0].address;
        printf("Board: %d\n", address);

        // Open the hat device
        retval = mcc118_open(hat_address);
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);
        }
    }
    else
    {
        show_mcc118_error(NO_HAT_DEVICES_FOUND);
    }

    // Return the address of the first HAT device.
    return hat_address;
}

/*******************************************************************************
    Function:               initialize_graph_channel_info

    Description:            Assign a legend color and channel number for each
                            channel on the device.

    Parameters:
        NONE

    Return Value:
        NONE
*******************************************************************************/
void initialize_graph_channel_info (void)
{
    for (int i = 0; i < MAX_118_CHANNELS; i++)
    {
        switch(i)
        {
        // channel 0
        case 0:
        default:
            legendColor[i].red = 1;
            legendColor[i].green = 0;
            legendColor[i].blue = 0;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
            break;
        // channel 1
        case 1:
            legendColor[i].red = 0;
            legendColor[i].green = 1;
            legendColor[i].blue = 0;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
           break;
        // channel 2
        case 2:
            legendColor[i].red = 0;
            legendColor[i].green = 0;
            legendColor[i].blue = 1;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
           break;
        // channel 3
        case 3:
            legendColor[i].red = 1;
            legendColor[i].green = 1;
            legendColor[i].blue = 0;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
            break;
        // channel 4
        case 4:
            legendColor[i].red = 1;
            legendColor[i].green = 0;
            legendColor[i].blue = 1;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
           break;
        // channel 5
        case 5:
            legendColor[i].red = 0;
            legendColor[i].green = 1;
            legendColor[i].blue = 1;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
            break;
        // channel 6
        case 6:
            legendColor[i].red = .67;
            legendColor[i].green = .67;
            legendColor[i].blue = 0;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
           break;
        // channel 7
        case 7:
            legendColor[i].red = .0;
            legendColor[i].green = .67;
            legendColor[i].blue = .67;
            legendColor[i].alpha = 1;

            graphChannelInfo[i].color = &legendColor[i];
            graphChannelInfo[i].channelNumber = i;
           break;
        }
    }
}

/*******************************************************************************
    Function:               allocate_channel_xy_arrays

    Description:            Allocate arrays for the indices and data for each
                            channel in the scan.

    Parameters:
        current_channel_mask    A bit mask for the channels in the scan.

    Return Value:
        num_channels        The number of channels in the scan.
*******************************************************************************/
int allocate_channel_xy_arrays(uint8_t current_channel_mask,
    uint32_t numSamplesPerChannel)
{
    int i = 0;
    int chanMask = 0;
    int channel = 0;
    int num_channels = 0;

    // Delete the previous arrays for each of the channels (if they exist)
    for (i = 0; i < MAX_118_CHANNELS; i++)
    {
        if (graphChannelInfo[i].graph != NULL)
        {
            gtk_databox_graph_remove (GTK_DATABOX(box), 
                graphChannelInfo[i].graph);
            graphChannelInfo[i].graph= NULL;
        }
    }

    // Create the new arrays for the current channels in the scan
    chanMask = current_channel_mask;
    channel = 0;
    for (i = 0; i < MAX_118_CHANNELS; i++)
    {
        // If this channel is in the scan, then allocate the arrays for the 
        // indices (X) and data (Y)
        if (chanMask & 1)
        {
            graphChannelInfo[channel].X = g_new0 (gfloat, numSamplesPerChannel);
            graphChannelInfo[channel].Y = g_new0 (gfloat, numSamplesPerChannel);
            num_channels++;
        }

        // Check next channel.
        channel++;
        chanMask >>= 1;
    }

    // Return the number of channels in the scan.
    return num_channels;
}

/*******************************************************************************
    Function:               create_selected_channel_mask

    Description:            Allocate arrays for the indices and data for each
                            channel in the scan.

    Parameters:
        current_channel_mask    A bit mask for the channels in the scan.

    Return Value:
        num_channels        The number of channels in the scan.
*******************************************************************************/
int create_selected_channel_mask()
{
    gboolean checked_status = FALSE;
    int selected_channels = 0;

    for (int i = 0; i < MAX_118_CHANNELS; i++)
    {
        // Is the channel checked?
        checked_status = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(chkChan[i]));

        // if checked, add the channel to the mask
        if (checked_status == TRUE)
        {
            selected_channels += (int)pow(2,i);
        }
    }

    g_print("channels selected for scan:  %d\n", selected_channels);

    // return the channel mask
    return (selected_channels);
}

/*******************************************************************************
    Function:               set_enable_state_for_controls

    Description:            Enable/disable the controls in the main window.

    Parameters:
        state               A boolean value that specifies whether the controls
                            are to be enabled or disabled.
                                TRUE = enable controls
                                FALSE = disable controls

    Return Value:
        NONE
*******************************************************************************/
void set_enable_state_for_controls(gboolean state)
{
    // Set the state of the check boxes
    for (int i = 0; i < MAX_118_CHANNELS; i++)
    {
        gtk_widget_set_sensitive (chkChan[i], state);
    }

    // Set the state of the text boxes
    gtk_widget_set_sensitive (spinRate, state);
    gtk_widget_set_sensitive (spinNumSamples, state);

    // Set the state of the radio buttons
    gtk_widget_set_sensitive (rbFinite, state);
    gtk_widget_set_sensitive (rbContinuous, state);

    // Set the state of the buttons
    gtk_widget_set_sensitive (btnSelectLogFile, state);
    gtk_widget_set_sensitive (btnQuit, state);

}

/*******************************************************************************
    Function:               copy_data_to_xy_arrays

    Description:            Copy the data for the specified channel from the
                            interleaved HAT buffer to the array for the
                            specified channel.

    Parameters:
        hat_read_buf        The HAT buffer containing the interleaved channel
                            data.
        read_buf_start_index    The index to start reading for this channel.
        channel             The channel whose data is being copied.
        stride              The distance (in samples) between samples for this
                            channel.
        buffer_size_samples The total number of samples in the buffer (for all
                            channels).
        first_block         Flag to indicate if this is the first block and if 
                            so, initialize the indices (X array) for the
                            channel.

    Return Value:
        NONE
*******************************************************************************/
void copy_data_to_xy_arrays(double* hat_read_buf, int read_buf_start_index, 
    int channel, int stride, int buffer_size_samples, gboolean first_block)
{
    // Get the arrays for this channels
    gfloat* X = graphChannelInfo[channel].X;
    gfloat* Y = graphChannelInfo[channel].Y;

    int ii = 0;

    // For the first block, set the indices and data.  For all other blocks,
    // just set the data
    if (first_block)
    {
        // Set indices and data
        for (int i = read_buf_start_index; i < buffer_size_samples; i+=stride)
        {
            X[ii] = (gfloat)ii;
            Y[ii] = (gfloat)hat_read_buf[i];

            ii++;
        }
    }
    else
    {
         // Set data only
       for (int i = read_buf_start_index; i < buffer_size_samples; i+=stride)
        {
            Y[ii] = (gfloat)hat_read_buf[i];

            ii++;
        }
    }
}

/*******************************************************************************
    Function:               analog_in_finite

    Description:            Wait for the scan to complete, then read the data
                            and write it to a CSV file, and plot it in the
                            graph.

    Parameters:
        NONE

    Return Value:
        NONE
*******************************************************************************/
void analog_in_finite ()
{
    int chan_mask = 0;
    int channel = 0;
 	uint32_t samples_read_per_channel = 0;
    uint32_t buffer_size_samples = 0;
    int retval = 0;
    uint16_t read_status = 0;

    // Allocate arrays for the indices and data for each channel in the scan.
    int num_channels = allocate_channel_xy_arrays(channel_mask, 
        iNumSamplesPerChannel);

    // set the timeout
    double scan_timeout = num_channels * 
                          iNumSamplesPerChannel / iRatePerChannel * 10; 
                          /*set duration of timeout */

    // Setup the sample buffer.
    double hat_read_buf[iNumSamplesPerChannel*num_channels];
    buffer_size_samples = iNumSamplesPerChannel*num_channels;

    // Wait for the scan to start running.
    do
    {
        retval =(mcc118_a_in_scan_read(address, &read_status, 0, scan_timeout,
            hat_read_buf, buffer_size_samples, &samples_read_per_channel));
        //was the scan started?
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);
            break;
        }
    } while ((retval == RESULT_SUCCESS) && 
             ((read_status & STATUS_RUNNING) != STATUS_RUNNING));

    if (retval == RESULT_SUCCESS)
    {
        // Read all the samples requested
        retval = mcc118_a_in_scan_read(address, &read_status,
            iNumSamplesPerChannel, scan_timeout, hat_read_buf,
            buffer_size_samples, &samples_read_per_channel);
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);
        }
        else if (read_status & STATUS_HW_OVERRUN)
        {
            show_mcc118_error(HW_OVERRUN);
        }
        else if (read_status & STATUS_BUFFER_OVERRUN)
        {
            show_mcc118_error(BUFFER_OVERRUN);
        }
    }

    g_print("AInScanRead status:  %d\n", read_status | STATUS_RUNNING );
    g_print("iNumSamplesPerChannel:  %d\n", iNumSamplesPerChannel);
    g_print("samples_read_per_channel:  %d\n", samples_read_per_channel);

    if (retval == RESULT_SUCCESS)
    {
        // Write the data to the CSV file.
        retval = write_log_file(log_file_ptr, hat_read_buf,
            iNumSamplesPerChannel, num_channels);
        if (retval < 0)
        {
            switch (retval)
            {
            case -1:
                show_mcc118_error(MAXIMUM_FILE_SIZE_EXCEEDED);
                break;

            default:
                show_mcc118_error(UNKNOWN_ERROR);
                break;
            }
        }
    }

    // Parse the data and display it on the graph.  Convert the 1D
    // interleaved data into a 2D array
    if (retval > 0)
    {
        chan_mask = channel_mask;
        channel = 0;
        int read_buf_index = 0;

        // Display the data for each channel.
        while (chan_mask > 0)
        {
            // Is this channel part of the scan?
            if (chan_mask & 1)
            {
                g_print("\n Channel %d\n", channel);

                // Get the data for this channel
                copy_data_to_xy_arrays(hat_read_buf, read_buf_index, channel,
                    num_channels, buffer_size_samples, TRUE);

                // Graph the data
                gfloat* X = graphChannelInfo[channel].X;
                gfloat* Y = graphChannelInfo[channel].Y;

                graphChannelInfo[channel].graph = 
                    gtk_databox_lines_new ((guint)iNumSamplesPerChannel, X, Y,
                    graphChannelInfo[channel].color, 2);
                gtk_databox_graph_add(GTK_DATABOX (box),
                    GTK_DATABOX_GRAPH(graphChannelInfo[channel].graph));
                gtk_databox_set_total_limits(GTK_DATABOX (box), 0.,
                    (gfloat)iNumSamplesPerChannel, 11., -11.);

                read_buf_index++;
            }

            // Next channel in the mask
            channel++;
            chan_mask >>= 1;
        }
    }

    // Re-enable all of the controls in the min window.
    set_enable_state_for_controls(TRUE);

    // Stop the scan completes
    retval = mcc118_a_in_scan_stop(address);
    if (retval != RESULT_SUCCESS)
    {
        show_mcc118_error(retval);
    }

    // Clean up after the scan completes
    retval = mcc118_a_in_scan_cleanup(address);
    if (retval != RESULT_SUCCESS)
    {
        show_mcc118_error(retval);
    }

    gtk_button_set_label(GTK_BUTTON(btnStart_Stop), "Start");
}

/*******************************************************************************
    Function:               refresh_graph

    Description:            Refresh the graph with the new data.

    Parameters:
        box                 Pointer to the GTK Databox to be refreshed.

    Return Value:
        FALSE
*******************************************************************************/
gboolean refresh_graph(GtkWidget *box)
{
gdouble t1, t2;

    // set a mutex to prevent the data from changing while it is being plotted
    g_mutex_lock (&data_mutex);

    t1 = g_timer_elapsed(timer, &msec);

    // Tell the graph to update
    gtk_widget_queue_draw((box));

    // release the mutex
    g_mutex_unlock (&data_mutex);

    t2 = g_timer_elapsed(timer, &msec);
    g_print("redraw_graph:  t1 = %lf, t2 = %lf, elapsed time = %lf\n",
        t1, t2, t2 - t1);

    return FALSE;
}

/*******************************************************************************
    Function:               analog_in_continuous

    Description:            While the scan is running, then read the data,
                            write it to a CSV file, and plot it in the graph.

                            This function runs as a background thread for the
                            duration of the scan.

    Parameters:
        arg                 Pointer to user defined parametrs.  This parameter
                            is not used.

    Return Value:
        NONE
*******************************************************************************/
void analog_in_continuous (void *arg)
{

    int chanMask = 0;
    int channel = 0;
    uint32_t samples_read_per_channel = 0;
    uint32_t buffer_size_samples = 0;
    int retval = 0;
    uint16_t read_status;
    gdouble t1, t2;

    // Allocate arrays for the indices and data for each channel in the scan.
    int num_channels = allocate_channel_xy_arrays(channel_mask,
        iNumSamplesPerChannel);

    // set the timeout
    double scan_timeout = num_channels * 
                          iNumSamplesPerChannel / iRatePerChannel * 10;
                          /*set duration of timeout */

    // Setup the sample buffer.
    buffer_size_samples = iNumSamplesPerChannel*num_channels;
    double hat_read_buf[buffer_size_samples];

    do
    {
        retval = mcc118_a_in_scan_read(address, &read_status, 0, scan_timeout,
            hat_read_buf, buffer_size_samples, &samples_read_per_channel);
        //was the scan started?
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);

            // If the scan fails to start clear it and reset the app to start
            // again
            g_print("Scan failure, Return code:  %d,  ReadStatus:  %d\n",
                retval, read_status);
            retval = mcc118_a_in_scan_stop(address);
            if (retval != RESULT_SUCCESS)
            {
                show_mcc118_error(retval);
            }

            retval = mcc118_a_in_scan_cleanup(address);
            if (retval != RESULT_SUCCESS)
            {
                show_mcc118_error(retval);
            }

            done = TRUE;
            pthread_join (threadh, NULL);

            gtk_button_set_label (GTK_BUTTON(btnStart_Stop), "Start");
            return;
        }
    } while ((retval == RESULT_SUCCESS) && 
             ((read_status & STATUS_RUNNING) != STATUS_RUNNING));

    // Loop to read data continuously
    gboolean first_block = TRUE;
    while  (done == FALSE)
    {
        t1 = g_timer_elapsed(timer, &msec);

        // Read the data from the device
        retval = mcc118_a_in_scan_read(address, &read_status,
            iNumSamplesPerChannel, scan_timeout, hat_read_buf,
            buffer_size_samples, &samples_read_per_channel);
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);
            break;
        }
        else if (read_status & STATUS_HW_OVERRUN)
        {
            show_mcc118_error(HW_OVERRUN);
            break;
        }
        else if (read_status & STATUS_BUFFER_OVERRUN)
        {
            show_mcc118_error(BUFFER_OVERRUN);
            break;
        }

        t2 = g_timer_elapsed(timer, &msec);
        g_print ("analog_in_continuous(read data):  t1 = %lf, t2 = %lf, "
            "elapsed time = %lf\n", t1, t2, t2 - t1);
        g_print ("analog_in_continuous(read data):  timeout = %f, "
            "samples read = %d\n", scan_timeout, samples_read_per_channel);

        // Write the data to a log file as CSV data
        retval = write_log_file(log_file_ptr, hat_read_buf,
            samples_read_per_channel, num_channels);
        if (retval < 0)
        {
            int error_code;
            switch (retval)
            {
            case -1:
                error_code = MAXIMUM_FILE_SIZE_EXCEEDED;
                break;

            default:
                error_code = UNKNOWN_ERROR;
                break;
            }

            // error dialog must be displayed on the main thread
            g_main_context_invoke(context, 
                (GSourceFunc)show_mcc118_error_main_thread, &error_code);

            // Call the Start?Stop event handler to reset the UI
            start_stop_event_handler(btnStart_Stop, NULL);

            done = TRUE;

            break;
        }

        // Set a mutex to prevent the data from changing while we plot it
        g_mutex_lock (&data_mutex);

        chanMask = channel_mask;
        channel = 0;
        int read_buf_index = 0;

        // While there are channels to plot
        while (chanMask > 0)
        {
            // if this channel is included in the acquisition, plot its data
            if (chanMask & 1)
            {
                if (first_block)
                {
                    // If this is the first block we need to set the indices and
                    // the data
                    copy_data_to_xy_arrays(hat_read_buf, read_buf_index,
                        channel, num_channels, buffer_size_samples, TRUE);

                    gfloat* X = graphChannelInfo[channel].X;
                    gfloat* Y = graphChannelInfo[channel].Y;

                    graphChannelInfo[channel].graph = gtk_databox_lines_new 
                        ((guint)iNumSamplesPerChannel, X, Y, 
                        graphChannelInfo[channel].color, 2);
                    gtk_databox_graph_add(GTK_DATABOX (box), 
                        GTK_DATABOX_GRAPH(graphChannelInfo[channel].graph));
                    gtk_databox_set_total_limits(GTK_DATABOX (box), 0.0, 
                        (gfloat)iNumSamplesPerChannel, 11.0, -11.0);
                }
                else
                {
                    // If this is not the first block, just update the data
                    copy_data_to_xy_arrays(hat_read_buf, read_buf_index,
                        channel, num_channels, buffer_size_samples, FALSE);
                }

                // Set the index to start at the first sample of the next
                // channel
                read_buf_index++;
            }
            channel++;
            chanMask >>= 1;
        }

        // Done with the data so fill the buffer with NaN.
        memset(hat_read_buf, 0, buffer_size_samples * sizeof(double));

        first_block = FALSE;

        // Set the condition to cause the display update function to update the
        // display
        g_print("\nWakeup update thread\n");
        //g_cond_signal (&data_cond);
        g_main_context_invoke(context, refresh_graph, box);

        // Release the mutex
        g_mutex_unlock(&data_mutex);

        t2 = g_timer_elapsed(timer, &msec);
        g_print("analog_in_continuous:  t1 = %lf, t2 = %lf, "
            "elapsed time = %lf\n", t1, t2, t2 - t1);

        if (loop_count % 10 == 0)
        {
            g_print("Loop Count = %d\n\n", loop_count);
        }
        loop_count++;

        // allow idle time for the diplay to update
        usleep(1);
    }

    // Stop the scan completes
    retval = mcc118_a_in_scan_stop(address);
    if (retval != RESULT_SUCCESS)
    {
        show_mcc118_error(retval);
    }

    // Clean up after the scan completes
    retval = mcc118_a_in_scan_cleanup(address);
    if (retval != RESULT_SUCCESS)
    {
        show_mcc118_error(retval);
    }
}

/*******************************************************************************
    Function:               start_stop_event_handler

    Description:            Event handler for the Start/Stop button.

                            If starting, change the button text to "Stop" and
                            parse the UI settings before starting the 
                            acquisition.

                            If stopping, change the button text to "Start" and
                            stop the acquisition.

    Parameters:
        widget              A pointer to the start/stop button widget
        data                A pointer to user data for the event handler (not
                            used by this function)

    Return Value:
        NONE
*******************************************************************************/
void start_stop_event_handler(GtkWidget *widget, gpointer data)
{
    const gchar* StartStopBtnLbl = gtk_button_get_label(GTK_BUTTON(widget));
    uint16_t options = 0;
    int retval = 0;

    if (strcmp(StartStopBtnLbl , "Start") == 0)
    {
        // open the log file
        log_file_ptr = open_log_file(csv_filename);

        if (log_file_ptr == NULL)
        {
            show_mcc118_error(UNABLE_TO_OPEN_FILE);
            done = TRUE;
            return;
        }

        set_enable_state_for_controls(FALSE);

        g_print ("Starting ...\n");

        timer = g_timer_new();
        g_timer_start(timer);

        // Change the label on the start button to "Stop".
        gtk_button_set_label(GTK_BUTTON(widget), "Stop");

        done =  FALSE;

        // Set variables based on the UI settings.
        channel_mask = create_selected_channel_mask();
        iNumSamplesPerChannel = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(spinNumSamples));  //1000;
        iRatePerChannel =  gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(spinRate));  //1000;

        // Initialize the scan options so that the scan runs in the background
        options = OPTS_BACKGROUND;

        // Set the continuous option based on the UI setting.
        gboolean bContinuous =  gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(rbContinuous));
        if (bContinuous == TRUE)
        {
            options |= OPTS_CONTINUOUS;
        }

        g_print("Scan options selected:  %d\n", options);

        // Start the analog scan.
        retval = mcc118_a_in_scan_start(address, channel_mask,
            iNumSamplesPerChannel, iRatePerChannel, options);
        if (retval != RESULT_SUCCESS)
        {
            show_mcc118_error(retval);

            gtk_button_set_label(GTK_BUTTON(btnStart_Stop), "Start");
            done = TRUE;
            return;
        }

        context = g_main_context_default();

        if ((options & OPTS_CONTINUOUS) == OPTS_CONTINUOUS)
        {
            // If continuous scan, then start a thread to read the data from the
            // device
            if (pthread_create(&threadh, NULL, analog_in_continuous, &tinfo) != 
                0)
            {
                printf("error creating thread..\n");
            }
        }
        else
        {
            // Its a finite scan, so call function to wait for all of the
            // samples to be acquired
            analog_in_finite();
        }

        loop_count = 0;
    }
    else
    {
        set_enable_state_for_controls(TRUE);

        // Change the label on the stop button to "Start".current
        gtk_button_set_label(GTK_BUTTON(widget), "Start");

        done = TRUE;
        retval =  mcc118_a_in_scan_cleanup(address);

        pthread_join(threadh, NULL);
        g_print("thread %d stopped\n", (int)threadh);

        g_timer_stop(timer);
        g_timer_destroy(timer);
    }

}

/*******************************************************************************
    Function:               select_log_file_event_handler

    Description:            Event handler for the Select Log File button.

                            Displays a GTK Open File dialog to select the log
                            file to be opened.

                            The file name will be shown in the main window's
                            title bar.

    Parameters:
        widget              A pointer to the Select Log File button widget
        data                A pointer to user data for the event handler 
                            (contains a pointer to the initial CSV file name)

    Return Value:
        NONE
*******************************************************************************/
void select_log_file_event_handler(GtkWidget* widget, gpointer user_data)
{
    // Get the initial file name.
    char* initial_filename = user_data;

    // Select the log file.
    strcpy(csv_filename, choose_log_file(window, initial_filename));

    // Display the window title which consists of the application
    // name and the CSV log file name.
    show_window_title();
}

/*******************************************************************************
    Function:           activate_event_handler

    Description:        Event handler that is called when the application is
                        launched to create the main window and its controls.

    Parameters:
        app             Pointer to this application's GtkApplication structure.
        user_data       Pointer to user data (not used in this example)
*******************************************************************************/
void activate_event_handler(GtkApplication *app, gpointer user_data)
{
//    GtkWidget *vboxMain, *vboxConfig, *label;
    GtkWidget *hboxMain;
    GtkWidget *vboxConfig, *label;
    GtkWidget *hboxChannel, *vboxChannel, *vboxLegend, *hboxNumSamples,
        *hboxRate;
    GtkDataboxGraph* graph;
    int i = 0;

    // Create the top level gtk window.
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(window, 900, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);

    // Create the GDK resources for the main window
    gtk_widget_realize(window);

    // Connect the event handler to the "delete_event" event
    g_signal_connect(window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);

    // Display the window title which consists of the applicatoin
    // name and the CSV log file name.
    show_window_title();

    hboxMain = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), hboxMain);

    vboxConfig = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(hboxMain), vboxConfig);

    hboxChannel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxChannel);

    label = gtk_label_new("    Channel select:  ");
    gtk_box_pack_start(GTK_BOX(hboxChannel), label, 0, 0, 0);

    vboxChannel=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(hboxChannel), vboxChannel);

    vboxLegend=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(hboxChannel), vboxLegend);

    char* chanBase = "Channel ";
    i = 0;
    char intBuf[5];
    char chanNameBuffer[20];
    GtkWidget *legend[MAX_118_CHANNELS];
    for (i = 0; i < MAX_118_CHANNELS; i++)
    {
        strcpy(chanNameBuffer, chanBase);
        sprintf(intBuf, "%d", i);
        strcat(chanNameBuffer, intBuf);

        chkChan[i] = gtk_check_button_new_with_label(chanNameBuffer);
        gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[i], 0, 0, 0);

        legend[i] = gtk_label_new("  ");
        gtk_box_pack_start(GTK_BOX(vboxLegend), legend[i], 5, 0, 0);
        gtk_widget_override_background_color (legend[i], GTK_STATE_FLAG_NORMAL,
            &legendColor[i]);
    }
    gtk_toggle_button_set_active(GTK_CHECK_BUTTON(chkChan[0]), TRUE);

    hboxNumSamples = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxNumSamples);

    label = gtk_label_new("Num Samples: ");
    gtk_box_pack_start(GTK_BOX(hboxNumSamples), label, 0, 0, 0);

    spinNumSamples = gtk_spin_button_new_with_range (10, 100000, 10);
    gtk_box_pack_start(GTK_BOX(hboxNumSamples), spinNumSamples, 0, 0, 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinNumSamples), 500.);

    hboxRate = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxRate);

    label = gtk_label_new("                 Rate:  ");
    gtk_box_pack_start(GTK_BOX(hboxRate), label, 0, 0, 0);

    spinRate = gtk_spin_button_new_with_range (10, 100000, 10);
    gtk_box_pack_start(GTK_BOX(hboxRate), spinRate, 0, 0, 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinRate), 1000.);

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_BOX(hboxMain), separator);

    gtk_databox_create_box_with_scrollbars_and_rulers_positioned (&box, &table,
        FALSE, FALSE, TRUE, TRUE, FALSE, TRUE);
    gtk_box_pack_start(GTK_BOX(hboxMain), table, TRUE, TRUE, 0);

    GtkDataboxRuler* ruler = gtk_databox_get_ruler_y(box);
    gtk_databox_ruler_set_text_orientation(ruler, GTK_ORIENTATION_HORIZONTAL);

    GdkRGBA grid_color;
    grid_color.red = 0;
    grid_color.green = 0;
    grid_color.blue = 0;
    grid_color.alpha = 1;
    graph = (GtkDataboxGraph*)gtk_databox_grid_new(5, 5, &grid_color, 1);
    gtk_databox_graph_add(GTK_DATABOX (box), GTK_DATABOX_GRAPH(graph));

    rbContinuous = gtk_radio_button_new_with_label(NULL, "Continuous");
    gtk_box_pack_start(GTK_BOX(vboxConfig), rbContinuous, 0, 0, 0);
    rbFinite = gtk_radio_button_new_with_label(NULL, "Finite");
    gtk_box_pack_start(GTK_BOX(vboxConfig), rbFinite, 0, 0, 0);
    gtk_radio_button_join_group((GtkRadioButton*)rbFinite,
        (GtkRadioButton*)rbContinuous);

    btnSelectLogFile = gtk_button_new_with_label ( "Select Log File ...");
    g_signal_connect(btnSelectLogFile, "clicked", 
        G_CALLBACK(select_log_file_event_handler), csv_filename);
    gtk_box_pack_start(GTK_BOX(vboxConfig), btnSelectLogFile, 0, 0, 5);

    btnStart_Stop = gtk_button_new_with_label("Start");
    g_signal_connect(btnStart_Stop, "clicked",
        G_CALLBACK(start_stop_event_handler), NULL);
    gtk_box_pack_start(GTK_BOX(vboxConfig), btnStart_Stop, 0, 0, 5);

    btnQuit = gtk_button_new_with_label( "Quit");
    g_signal_connect(btnQuit, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(vboxConfig), btnQuit, 0, 0, 0);

    // Show the top level window and all of its controls
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[])
{
    GtkApplication *app;

    // Set the application name.
    strcpy(application_name, "MCC 118 Data Logger");

    // Set the default filename.
    getcwd(csv_filename, sizeof(csv_filename));
    strcat(csv_filename, "/LogFiles/csv_test.csv");



    //csv_filename = "../LogFiles/csv_test.csv";

    // Initialize the GTK.
    gtk_init(&argc, &argv);

    initialize_graph_channel_info();

    // Create the application structure and set an event handler for the 
    // activate event.
    app = gtk_application_new("org.mcc.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate_event_handler),
        NULL);

    // Start running the GTK appliction.
    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Find the hat devices and open the first one.
    address = open_first_hat_device();

    error_context = g_main_context_default();

    // Start the GTK message loop.
    gtk_main();

    // Close the device.
    mcc118_close(address);

    //Exit app...
    return 0;
}
