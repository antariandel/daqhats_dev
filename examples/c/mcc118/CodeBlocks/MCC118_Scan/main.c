#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <mcchats/mcchats.h>
#include <gtk/gtk.h>
#include <time.h>
#include <pthread.h>
#include "gtkdatabox.h"
#include "gtkdatabox_points.h"
#include "gtkdatabox_ruler.h"

#define OPTS_BACKGROUND 0

    //Set up the form to include these objects/widgets.
    //Those listed are only called in the main.
    //Globally called widgets are declared at/close to top of app.
    GtkWidget *window = NULL;
    GtkWidget *chkContinuous, *chkBurstMode, *chkExtTrig, *chkExtClock;
    GtkWidget *spinRate;
    GtkWidget *spinNumSamples;
    GtkWidget *chkChan[8];
    GtkWidget *vbox;
    GtkWidget *btnStart_Stop;

    gfloat *X;
    gfloat *Y;
    GtkWidget *box;
    GtkWidget *table;
    GtkDataboxGraph *graph;

    GdkRGBA color;
    GdkRGBA background_color;
    gint i;

    int info_count;
    struct HatInfo* info_list;
    uint8_t address;
    uint8_t channels, numchannels;
    int RetVal = 0;
    uint16_t read_status = 0;
    int read_result = RESULT_SUCCESS;
	uint32_t samples_read_per_channel = 0;
	double buffer_size_samples = 0;
	int iNumSamplesPerChannel = 0;
	uint16_t options = OPTS_BACKGROUND;
	double scan_timeout = 0.0;
    gboolean done = FALSE;

    pthread_t threadh;
    struct thread_info *tinfo;

/*************************************************************************/
static void activate (GtkApplication *app, gpointer user_data) {}

static void create_basics (void)
{
/*just some dummy data that looks nice on the screen*/
    int POINTS = 2000;


   /* show some data */
   X = g_new0 (gfloat, POINTS);
   Y = g_new0 (gfloat, POINTS);

   for (i = 0; i < POINTS; i++)
   {
      X[i] = i;
      Y[i] = 10 *sin (i * 4 * G_PI / POINTS);
   }

   /* Add your data data in some color */
   color.red = 1;
   color.green = 1;
   color.blue = 1;
   color.alpha = 1;

   graph = gtk_databox_points_new (POINTS, X, Y, &color, 1);
   gtk_databox_graph_add (GTK_DATABOX (box), graph);

  // gtk_databox_set_total_limits (GTK_DATABOX (box), 0., (double)POINTS, 10., -10.);
   gtk_databox_set_total_limits (GTK_DATABOX (box), -1000., 5000., -10000., 23000.);
   gtk_databox_auto_rescale (GTK_DATABOX (box), 0.05);

   gtk_widget_show_all (window);
   gdk_window_set_cursor (gtk_widget_get_window(box), gdk_cursor_new (GDK_CROSS));
}

static void *analog_in_background (void *arg)
{
/*Use this routine if scan options do NOT include continuous... */

    /*Setup the sample buffer  */
    double read_buf[iNumSamplesPerChannel*numchannels];
    buffer_size_samples = iNumSamplesPerChannel*numchannels;

    RetVal =(mcc118_a_in_scan_read(address, &read_status, 0, scan_timeout, read_buf, buffer_size_samples, &samples_read_per_channel)); //was the scan started?
    if((RetVal == RESULT_SUCCESS) && (read_status | STATUS_RUNNING == STATUS_RUNNING)) // STATUS_RUNNING = 0x0008
    {
        /*Read all the samples requested*/
        RetVal =(mcc118_a_in_scan_read(address, &read_status, iNumSamplesPerChannel , scan_timeout, read_buf, buffer_size_samples, &samples_read_per_channel));
        if (samples_read_per_channel >= iNumSamplesPerChannel)
        {
            done = TRUE;
        }
    }

    g_print ("AInScanRead status:  %d\n", read_status | STATUS_RUNNING );
    g_print ("iNumSamplesPerChannel:  %d\n",  iNumSamplesPerChannel);
    g_print ("samples_read_per_channel:  %d\n", samples_read_per_channel);
    int ArrayLength = sizeof(read_buf)/sizeof(read_buf[0]);
    g_print ("number of samples acquired:  %d\n", ArrayLength);

    /*parse the data and display it on the graph*/
    /*Convert the 1D interleaved data into a 2D array*/
    gtk_databox_graph_remove_all (box);  //clear the graph
    for (int j = 0; j < numchannels ; j++)
    {
        X = g_new0 (gfloat, iNumSamplesPerChannel);  //These have to be here else
        Y = g_new0 (gfloat, iNumSamplesPerChannel);  //it will not plot multiple channels

        g_print("\n Channel %d\n", j);
        int ii = 0;
        for (int i = j; i < ArrayLength; i+=numchannels)
        {
            X[ii]= ii;
            Y[ii]= read_buf[i];
            //g_print("buffer(%4d):  %.5f,  Y(%d):  %.5f   \n", i,read_buf[i], ii, Y[ii] );
            ii++;
        }

        /*change the pen color*/
        color.red = 0.5 + 0.5 * (j+ 2) / 8;
        color.green = 1 - 0.5 * j / 8;
        color.blue = 1;
        color.alpha = 1;

        /*Graph the data*/
        graph = gtk_databox_points_new ((guint)iNumSamplesPerChannel, X, Y, &color, 1);
        gtk_databox_graph_add (GTK_DATABOX (box), GTK_DATABOX_GRAPH(graph));
        gtk_databox_set_total_limits (GTK_DATABOX (box), 0., (gfloat)iNumSamplesPerChannel, 11., -11.);
        //gtk_databox_auto_rescale (GTK_DATABOX (box), 0.001);
        gtk_widget_show_all (window);
        gdk_window_set_cursor (gtk_widget_get_window(box), gdk_cursor_new (GDK_CROSS));
    }

    /*Clean up after the scan completes*/
    RetVal = mcc118_a_in_scan_stop(address);
    g_print ("AInScanStop:  %d\n", RetVal);

    RetVal =  mcc118_a_in_scan_cleanup(address);
    g_print ("AInScan_cleanup:  %d\n", RetVal);

    gtk_button_set_label (btnStart_Stop, "Start");

    return(0);
}

static void *analog_in_continuous (void *arg)
{ /*Use this routine if scan options include continuous... */

    double read_buf0[numchannels];  //just to hold some dummy data
    buffer_size_samples = numchannels;

    RetVal =(mcc118_a_in_scan_read(address, &read_status, 0, scan_timeout, read_buf0, buffer_size_samples, &samples_read_per_channel)); //was the scan started?
    if((RetVal != RESULT_SUCCESS) && (read_status | STATUS_RUNNING != STATUS_RUNNING)) // STATUS_RUNNING = 0x0008
    {
        /*If the scan fails to start clear it and reset the app to start again*/
        g_print ("Scan failure, Return code:  %d,  ReadStatus:  %d\n", RetVal, read_status);
        RetVal = mcc118_a_in_scan_stop(address);
        g_print ("AInScanStop:  %d\n", RetVal);

        RetVal =  mcc118_a_in_scan_cleanup(address);
        g_print ("AInScan_cleanup:  %d\n", RetVal);

        done = TRUE;
        pthread_join (threadh, NULL);

        gtk_button_set_label (btnStart_Stop, "Start");
       return 0;
    }

    /*Loop to read data continuously */
    while  (done == FALSE)
    {
        double read_buf[iNumSamplesPerChannel*numchannels];  /*Setup the sample buffer  */
        buffer_size_samples = iNumSamplesPerChannel*numchannels;

        samples_read_per_channel =0;
        while ( samples_read_per_channel  < iNumSamplesPerChannel)
        {
            RetVal =(mcc118_a_in_scan_read(address, &read_status, iNumSamplesPerChannel , scan_timeout, read_buf, buffer_size_samples, &samples_read_per_channel));
             g_print ("iNumSamplesPerChannel:  %d\n",  iNumSamplesPerChannel);
             g_print ("samples_read_per_channel:  %d\n", samples_read_per_channel);
        }

        g_print ("AInScanRead status:  %d\n", read_status | STATUS_RUNNING );
        g_print ("samples_read_per_channel:  %d\n", samples_read_per_channel);
        int ArrayLength = sizeof(read_buf)/sizeof(read_buf[0]);
        g_print ("number of samples acquired:  %d\n", ArrayLength);

        /*parse the data and display it on the graph*/
        /*Convert the 1D interleaved data into a 2D array*/
        gtk_databox_graph_remove_all (box);
        for (int j = 0; j < numchannels ; j++)
        {
            X = g_new0 (gfloat, iNumSamplesPerChannel);
            Y = g_new0 (gfloat, iNumSamplesPerChannel);

            g_print("\n Channel %d\n", j);
            int ii = 0;
            for (int i = j; i < ArrayLength; i+=numchannels)
            {
                X[ii]= ii;
                Y[ii]= read_buf[i];
                //g_print("buffer(%4d):  %.5f,  Y(%d):  %.5f   \n", i,read_buf[i], ii, Y[ii] );
                ii++;
            }

            /*change the pen color*/
            color.red = 0.5 + 0.5 * (j+ 2) / 8;
            color.green = 1 - 0.5 * j / 8;
            color.blue = 1;
            color.alpha = 1;

            /*Graph the data*/
            graph = gtk_databox_points_new ((guint)iNumSamplesPerChannel, X, Y, &color, 1);
            gtk_databox_graph_add (GTK_DATABOX (box), GTK_DATABOX_GRAPH(graph));
            gtk_databox_set_total_limits (GTK_DATABOX (box), 0., (gfloat)iNumSamplesPerChannel, 11., -11.);
            //gtk_databox_auto_rescale (GTK_DATABOX (box), 0.001);
            gtk_widget_show_all (window);
            gdk_window_set_cursor (gtk_widget_get_window(box), gdk_cursor_new (GDK_CROSS));
        }
        usleep(500* 1000); /*Wait 500mS, and do it again, also allows user to stop the scan */
    }

    /*Clean up after the scan completes*/
    RetVal = mcc118_a_in_scan_stop(address);
    g_print ("AInScanStop:  %d\n", RetVal);

    RetVal =  mcc118_a_in_scan_cleanup(address);
    g_print ("AInScan_cleanup:  %d\n", RetVal);

    gtk_button_set_label (btnStart_Stop, "Start");

    return(0);
}

static void start_stop(GtkWidget *widget, gpointer data){
/*read the button's label (Start or Stop) and
  parse the various required settings to start the analog input scan*/

    gchar* StartStopBtnLbl = gtk_button_get_label(btnStart_Stop);

    if (strcmp(StartStopBtnLbl , "Start")==0)
    {
        //start
        gtk_button_set_label (btnStart_Stop, "Stop");
        done =  FALSE;
        channels = channel_select();
        iNumSamplesPerChannel = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spinNumSamples));  //1000;
        double iRatePerChannel =  gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spinRate));  //1000;  //

        scan_timeout = numchannels * iNumSamplesPerChannel / iRatePerChannel * 10; /*set duration of timeout */

        options = OPTS_BACKGROUND;

        gboolean bContinuous =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (chkContinuous));
        if (bContinuous == TRUE)
            options |= OPTS_CONTINUOUS;

        gboolean bBurstMode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (chkBurstMode));
        if(bBurstMode == TRUE)
            options|= OPTS_BURSTMODE;

        gboolean bExtTrig = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (chkExtTrig));
        if(bExtTrig == TRUE)
            options|= OPTS_EXTTRIGGER;

        gboolean bExtClock = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (chkExtClock));
        if(bExtClock == TRUE)
            options|= OPTS_EXTCLOCK;

        g_print ("Scan options selected:  %d\n", options);

        RetVal = mcc118_a_in_scan_start(address, channels , iNumSamplesPerChannel, iRatePerChannel, options); //OPTS_CONTINUOUS);  //
        g_print ("mcc118_a_in_scan_start:  %d\n", RetVal);
        if (RetVal != 0)
        {
            g_print("ERROR Scan Failed to start:  error %d\n", RetVal);
            gtk_button_set_label (btnStart_Stop, "Start");
            done = TRUE;
            return;
        }

        /* What kind of scan options were selected*/
        if ( (options & OPTS_CONTINUOUS) == OPTS_CONTINUOUS)
        {
            if (pthread_create (&threadh, NULL, analog_in_continuous, &tinfo) != 0)
            {
                printf("error creating thread..\n");
            }
        }
        else
        {
            analog_in_background(NULL);
        }
    }
    else
    {
        //stop
        g_print ("timer stopped\n");
        gtk_button_set_label (btnStart_Stop, "Start");
        done = TRUE;
        RetVal =  mcc118_a_in_scan_cleanup(address);
        pthread_join (threadh, NULL);
	}

}

int channel_select()
{
    /*create the value for the channle parameter and number of channels */
    gboolean checked_status = FALSE;
    int selected_channels = 0;
    numchannels = 0;
    for (int i = 0; i < 8; i++)
    {
        checked_status = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (chkChan[i]));
        if(checked_status == TRUE)
        {
            selected_channels += (int)pow(2,i);
            numchannels += 1;
        }
    }

    g_print("channels selected for scan:  %d\n", selected_channels);
    return (selected_channels);
}

static void initialize(){

// get list of MCC 118s, but only use the first one.
    info_count = hat_list(HAT_ID_MCC_118, NULL);

    if (info_count > 0)
    {
        info_list = (struct HatInfo*)malloc(info_count * sizeof(struct HatInfo));
        hat_list(HAT_ID_ANY, info_list);

        //use the first one only (for this app)
        address = info_list[0].address;
        printf("Board: %d\n", address);
        RetVal = mcc118_open(address);
    }
    else
    {
        g_print("hat_list returned %d\n", info_count);
    }
}

int main (int argc, char *argv[])
{
    GtkWidget *vboxMain, *vboxConfig, *label;
    GtkWidget *hboxChannel, *vboxChannel, *hboxNumSamples, *hboxRate;
    GtkWidget *hbox;
    GtkWidget *btnQuit;

    GtkApplication *app;

    gtk_init (&argc, &argv);

    app = gtk_application_new ("org.mcc.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    /* Connect signal handlers to the constructed widgets. */
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request (window, 800, 500);
    gtk_widget_realize (window);
    g_signal_connect (window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);

    vboxMain=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(window), vboxMain);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hbox);

    vboxConfig=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(hbox), vboxConfig);

    hboxChannel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxChannel);

    label = gtk_label_new("    Channel select:  ");
    gtk_box_pack_start(GTK_BOX(hboxChannel), label,0,0,0);

     vboxChannel=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(hboxChannel), vboxChannel);

    chkChan[0] = gtk_check_button_new_with_label(("Channel 0"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[0],0,0,0);
    gtk_toggle_button_set_active(chkChan[0], TRUE);

    chkChan[1] = gtk_check_button_new_with_label(("Channel 1"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[1],0,0,0);

    chkChan[2] = gtk_check_button_new_with_label(("Channel 2"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[2],0,0,0);

    chkChan[3] = gtk_check_button_new_with_label(("Channel 3"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[3],0,0,0);

    chkChan[4] = gtk_check_button_new_with_label(("Channel 4"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[4],0,0,0);

    chkChan[5] = gtk_check_button_new_with_label(("Channel 5"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[5],0,0,0);

    chkChan[6] = gtk_check_button_new_with_label(("Channel 6"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[6] ,0,0,0);

    chkChan[7] = gtk_check_button_new_with_label(("Channel 7"));
    gtk_box_pack_start(GTK_BOX(vboxChannel), chkChan[7],0,0,0);

    hboxNumSamples = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxNumSamples);

    label = gtk_label_new("Num Samples: ");
    gtk_box_pack_start(GTK_BOX(hboxNumSamples), label,0,0,0);

    spinNumSamples = gtk_spin_button_new_with_range (100,10000,100);
    gtk_box_pack_start(GTK_BOX(hboxNumSamples), spinNumSamples, 0,0,0);
    gtk_spin_button_set_value(spinNumSamples, 1000.);
//    gtk_widget_set_size_request (GTK_SPIN_BUTTON(spinNumSamples), 0, 0);

    hboxRate = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_add(GTK_CONTAINER(vboxConfig), hboxRate);

    label = gtk_label_new("                 Rate:  ");
    gtk_box_pack_start(GTK_BOX(hboxRate), label,0,0,0);

    spinRate = gtk_spin_button_new_with_range (1000,100000,100);
    gtk_box_pack_start(GTK_BOX(hboxRate), spinRate, 0,0,0);
  //  gtk_widget_set_size_request (GTK_SPIN_BUTTON(spinRate), 0, 0);
    GtkWidget *separator =gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_BOX(hbox), separator);

    gtk_databox_create_box_with_scrollbars_and_rulers (&box, &table, TRUE, TRUE, TRUE, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

    background_color.red = 0;
    background_color.green = 0;
    background_color.blue = 0;
    background_color.alpha = 1;
    gtk_widget_override_background_color (box, GTK_STATE_FLAG_NORMAL, &background_color);

    chkBurstMode = gtk_check_button_new_with_label(("Burstmode"));
    gtk_box_pack_start(GTK_BOX(vboxConfig), chkBurstMode,0,0,0);

    chkContinuous = gtk_check_button_new_with_label(("Continuous"));
    gtk_box_pack_start(GTK_BOX(vboxConfig), chkContinuous,0,0,0);

    chkExtTrig = gtk_check_button_new_with_label(("Ext. Trigger"));
    gtk_box_pack_start(GTK_BOX(vboxConfig), chkExtTrig,0,0,0);

    chkExtClock = gtk_check_button_new_with_label(("Ext. Clock"));
    gtk_box_pack_start(GTK_BOX(vboxConfig), chkExtClock,0,0,0);

    btnStart_Stop = gtk_button_new_with_label( "Start");
    g_signal_connect (btnStart_Stop, "clicked", G_CALLBACK (start_stop), NULL);
    gtk_box_pack_start(GTK_BOX(vboxConfig), btnStart_Stop,0,0,5);


    btnQuit = gtk_button_new_with_label ( "Quit");
    g_signal_connect (btnQuit, "clicked", G_CALLBACK (gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(vboxConfig), btnQuit,0,0,0);


   /* Enter the main loop */
    gtk_widget_show_all (window);

    create_basics ();
    initialize();
    gtk_main ();

    //Exit app...
    mcc118_close(address);
    return 0;
}
