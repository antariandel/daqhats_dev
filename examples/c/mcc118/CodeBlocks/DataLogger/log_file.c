#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "globals.h"

FILE* log_file_ptr = NULL;

extern GTimer *timer;
extern gulong msec;

/***********************************************************************************
    Function:                   get_path_and_filename

    Description:                Get the error message for the specified error code.

    Parameters:
        full_path				Pointer to the full path for the file (directory
								and file name).
		path					Pointer to location to receve the path portion of
								the full path.
		filename				Pointer to location to receve the file name portion
								of the full path.

    Return Value:
        NONE
***********************************************************************************/
void get_path_and_filename(char* full_path, char* path, char* filename)
{
    char* p;

    int path_len = 0;

 	// Get the pointer to the last occurance of '/'.  This is the
	// pointer to the end of the path part of the full path.
    p = strrchr(full_path, '/');

	// get the lengthe of the path
	path_len = p - full_path + 1;

	// copy the path part of the full path
    strncpy(path, full_path, path_len);

	// copy the fime name part of the full path
    strcpy(filename, full_path+path_len);

    return;
}

/***********************************************************************************
    Function:                   choose_log_file

    Description:                Show the Open File dialog to allow the name of the
								log file to be chosen.

    Parameters:
        parent_window			The parent window of the Open File dialog.
		initial_path			The initial path to be selected when the
								Open File dialog is displayed.

    Return Value:
        new_filename			The path for the selected file.
***********************************************************************************/
char* choose_log_file(GtkWidget *parent_window, char* default_path)
{
    struct stat st;
    char path[512] = {'\0'};
    char filename[256] = {'\0'};
    char* new_filename;
    GtkWidget *dialog;
    gint res;

    // Get the path and filename.
    get_path_and_filename(default_path, path, filename);

    // Create the path if it does not already exist.
    if (stat(path, &st) == -1)
    {
        mkdir(path, 0700);
    }

    // Create the Open File dialog.
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          (GtkWindow*)parent_window,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          ("_Cancel"),
                                          GTK_RESPONSE_CANCEL,
                                          ("_Open"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

	// Set the initial directory
    gtk_file_chooser_set_current_folder ((GtkFileChooser*)dialog, path);

	// Set the initial fime name.
    gtk_file_chooser_set_current_name((GtkFileChooser*)dialog, filename);

	// Show the dialog.
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
		// get the selected file name path
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        new_filename = gtk_file_chooser_get_filename (chooser);
    }
    else
		// Use the initial path if the selection has  been canceled
        new_filename = default_path;

    // destoy the dialog.
    gtk_widget_destroy (dialog);

	// Return the path to the selected file.
    return new_filename;
}

/***********************************************************************************
    Function:                   open_log_file

    Description:                Open the specified file for writing.

    Parameters:
        path					The path to the file to be opened.

    Return Value:
        log_file_ptr			The file pointer.
***********************************************************************************/
FILE* open_log_file (char* path)
{
    struct stat st;
    char directory[512] = {'\0'};
    char filename[256] = {'\0'};;

    // Get the path and filename.

    get_path_and_filename(path, directory, filename);

    // Create the path if it does not already exist.
    if (stat(directory, &st) == -1)
    {
        mkdir(directory, 0700);
    }

	// Open the log file for writing.
    log_file_ptr = fopen(path, "w");

	// Return the file pointer.
    return log_file_ptr;
}

gboolean in_write_log_file = FALSE;
/***********************************************************************************
    Function:                   write_log_file

    Description:                Convert the numeric data to ASCII values seperated
								by commas (CSV) and write the data using the specified
								file pointer.
	.

    Parameters:
        log_file_ptr			The file pointer.
		read_buf				Pointer to the buffer containing the  data to be
								written.
		samplesPerChannel		The number of samples per channel to be written.
		numberOfChannels		The number of channels.

    Return Value:
        write_status			The file I/O status.
***********************************************************************************/
int write_log_file(FILE* log_file_ptr, double* read_buf, int samplesPerChannel, int numberOfChannels)
{
    int i = 0;
    int j = 0;
    int write_status = 1;

    char str[1000];
    char buf[256];
    gdouble t1, t2;
    int scan_start_index = 0;

    if (in_write_log_file == TRUE)
        g_print("write_log_file is re-entrant\n");
    in_write_log_file = TRUE;

    strcpy(str, "");

t1 = g_timer_elapsed(timer, &msec);
	// Write the data to the file.
    for (i=0; i<samplesPerChannel; i++)
    {
		// Initialize the string to be written  to the file.
        strcpy(str, "");

        // Write a sample for each channel in the scan.
        for (j=0; j<numberOfChannels; j++)
        {
			// Convert the data sample to ASCII
            sprintf(buf,"%2.6lf,", read_buf[scan_start_index+j]);

            // Add the data sample to the string to be written.
            strcat(str, buf);
        }

		// Write the ASCII scan data to the file.
        write_status = fprintf(log_file_ptr, "%s\n", str);
        if (write_status <= 0)
        {
			// Break if an error occurred.
            break;
        }

		// Get the index to the start of the next scan.
        scan_start_index += numberOfChannels;
    }

	// Flush the file to insure all data is written.
    fflush(log_file_ptr);
t2 = g_timer_elapsed(timer, &msec);
g_print ("write_log_file:  t1 = %lf, t2 = %lf, elapsed time = %lf\n", t1, t2, t2 - t1);

    in_write_log_file = FALSE;

	// Return the error code.
    return write_status;
}
