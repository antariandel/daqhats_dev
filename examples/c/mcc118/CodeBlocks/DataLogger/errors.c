#include <gtk/gtk.h>

#include "globals.h"
#include "daqhats/daqhats.h"
#include "errors.h"

/***********************************************************************************
    Function:                   get_mcc118_error_message

    Description:                Get the error message for the specified error code.

    Parameters:
        error_code				The error code whose message will be returned.

    Return Value:
        msg						The error message associated with the specified
								error code.
***********************************************************************************/
gpointer get_mcc118_error_message(int error_code)
{
    char* msg;

    switch(error_code)
    {
        // MCC118 library errors
        case RESULT_SUCCESS:
            msg = "Success, no errors.";
            break;

        case RESULT_BAD_PARAMETER:
            msg = "A parameter passed to the function was incorrect.";
            break;

        case RESULT_BUSY:
            msg = "The device is busy.";
            break;

        case RESULT_TIMEOUT:
            msg = "There was a timeout accessing a resource.";
            break;

        case RESULT_LOCK_TIMEOUT:
            msg = "Success, no errors";
            break;

        case RESULT_INVALID_DEVICE:
            msg = "The device at the specified address is not the correct type.";
            break;

        case RESULT_RESOURCE_UNAVAIL:
            msg = "A needed resource was not available.";
            break;

        case RESULT_UNDEFINED:
            msg = "Some other error occurred.";
            break;

        // Logger application errors
        case NO_HAT_DEVICES_FOUND:
            msg = "No MCC-118 Hat devices were found.";
            break;

        case HW_OVERRUN:
            msg = "Hardware overrun has occurred.";
            break;

        case BUFFER_OVERRUN:
            msg = "Buffer overrun has occurred.";
            break;

        case UNABLE_TO_OPEN_FILE:
            msg = "Unable to open the log file.";
            break;

        case MAXIMUM_FILE_SIZE_EXCEEDED:
            msg = "The maximum file size of 2GB has been exceeded.";
            break;

		// unknown error ... most likely an unhandled system error
		case UNKNOWN_ERROR:
		default:
            msg = "Unknown error.";
    }

   return msg;
}


/***********************************************************************************
    Function:                   show_error

    Description:                Create a dialog box and display the error message.

    Parameters:
        errmsg					The error message to display.

    Return Value:
        FALSE
***********************************************************************************/
gboolean show_error(gpointer errmsg)
{
	// Create a dialog box.
    GtkWidget* error_dialog = gtk_message_dialog_new ((GtkWindow*)window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, errmsg);
    //gtk_widget_show_all(error_dialog);

    // Set the window title
    gtk_window_set_title ((GtkWindow*)error_dialog, "Error");

	// Show the dialog box.
    gtk_dialog_run (GTK_DIALOG (error_dialog));

	// We are done with the dialog, so destroy it.
    gtk_widget_destroy (error_dialog);

    return FALSE;
}


/***********************************************************************************
    Function:                   show_mcc118_error_main_thread

    Description:                Get the error message for the specified error code,
								display it in a dialog box.

								This function is called from the background thread,
								but executes in the main thread

    Parameters:
        error_code_ptr			Pointetr to the error code whose message is to be
                                displayed.

    Return Value:
        NONE
***********************************************************************************/
void show_mcc118_error_main_thread(gpointer error_code_ptr)
{
    int error_code = *(int*)error_code_ptr;
    show_mcc118_error(error_code);
}

/***********************************************************************************
    Function:                   show_mcc118_error

    Description:                Get the error message for the specified error code,
								display it in a dialog box.

    Parameters:
        error_code				The error code whose message is to be displayed.

    Return Value:
        NONE
***********************************************************************************/
void show_mcc118_error(int error_code)
{
	// Get the error message.
    char* errmsg = get_mcc118_error_message(error_code);

 	// display the rror message in a a dialog box.
   show_error (errmsg);
}

