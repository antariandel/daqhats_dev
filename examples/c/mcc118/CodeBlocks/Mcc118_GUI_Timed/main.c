#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <daqhats/daqhats.h>
#include <gtk/gtk.h>
#include <time.h>
#include <pthread.h>

    GtkWidget *window = NULL;
    GtkWidget *btnAIn_Start_Stop;
    GtkWidget *lbl_Ch0, *lbl_Ch1, *lbl_Ch2, *lbl_Ch3, *lbl_Ch4, *lbl_Ch5, *lbl_Ch6, *lbl_Ch7;

    int info_count;
    struct HatInfo* info_list;
    double value;
    uint8_t address;
    uint8_t channel;

    int status;
    gchar buf[1000];
    gboolean done = FALSE;

    pthread_t threadh;
    struct thread_info *tinfo;

/*************************************************************************/
static void activate (GtkApplication *app, gpointer user_data)
{}

static void *Analog_In (void *arg)
{
    //Read each analog input channel, and get one sample
    while (done == FALSE)
    {
        mcc118_open(address);
        for (channel = 0; channel < mcc118_a_in_num_channels(); channel++)
        {
            mcc118_a_in_read(address, channel, 0, &value);
            //printf("   Channel %d: %7.3f\n", channel, value);
            sprintf(buf, "%7.3f", value );
            switch (channel)
            {
            case 0:
                gtk_label_set_text(GTK_LABEL(lbl_Ch0), buf);
                break;

            case 1:
                gtk_label_set_text(GTK_LABEL(lbl_Ch1), buf);
                break;

            case 2:
                gtk_label_set_text(GTK_LABEL(lbl_Ch2), buf);
                break;

            case 3:
                gtk_label_set_text(GTK_LABEL(lbl_Ch3), buf);
                break;

            case 4:
                gtk_label_set_text(GTK_LABEL(lbl_Ch4), buf);
                break;

            case 5:
                gtk_label_set_text(GTK_LABEL(lbl_Ch5), buf);
                break;

            case 6:
                gtk_label_set_text(GTK_LABEL(lbl_Ch6), buf);
                break;

            case 7:
                gtk_label_set_text(GTK_LABEL(lbl_Ch7), buf);
                break;
            }
        }
        usleep(250 *1000);  //Wait 250 mS
    }
 }

 static void Start_Stop (GtkWidget *widget, gpointer   data)
{
    //read the button's label (Start or Stop)
    gchar* StartStopBtnLbl = gtk_button_get_label (btnAIn_Start_Stop);

    if (strcmp(StartStopBtnLbl , "Start")==0)
    {
        //start
        gtk_button_set_label (btnAIn_Start_Stop, "Stop");
        done =  FALSE;
        if (pthread_create (&threadh, NULL, Analog_In, &tinfo) != 0)
        {
            printf("error creating thread...so there!\n");
        }
    }
    else
    {
        //stop
        g_print ("timer stopped\n");
        gtk_button_set_label (btnAIn_Start_Stop, "Start");
        done = TRUE;
        pthread_join (threadh, NULL);
	}
}

static void flash_led()
{
    //Flash the devices LED 5 times.
    int Dev_status =  mcc118_blink_led(address, 5);
}

int main (int argc, char *argv[])
{

    GtkWidget *vboxMain, *label;
    GtkWidget *hboxCh0, *hboxCh1, *hboxCh2, *hboxCh3, *hboxCh4, *hboxCh5, *hboxCh6, *hboxCh7;
    GtkWidget *hbox_Control, *hbox, *separator;
    GtkWidget *btnFlashLED, *btnQuit;


    GtkApplication *app;

    /* Initialize GTK+ */
    gtk_init (&argc, &argv);

    app = gtk_application_new ("org.mcc.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_widget_realize (window);

    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vboxMain=gtk_box_new(TRUE,0);
    gtk_container_add(GTK_CONTAINER(window), vboxMain);

    label = gtk_label_new("Analog Input:\n    ");
    gtk_box_pack_start(GTK_BOX(vboxMain), label,0,0,0);

    hboxCh0 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh0);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh0),1);

    label = gtk_label_new("Channel 0:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh0), label,0,0,0);

    lbl_Ch0 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh0), lbl_Ch0,0,0,0);



    hboxCh1 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh1);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh1),1);

    label = gtk_label_new("Channel 1:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh1), label,0,0,0);

    lbl_Ch1 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh1), lbl_Ch1,0,0,0);



    hboxCh2 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh2);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh2),1);

    label = gtk_label_new("Channel 2:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh2), label,0,0,0);

    lbl_Ch2 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh2), lbl_Ch2,0,0,0);


    hboxCh3 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh3);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh3),1);

    label = gtk_label_new("Channel 3:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh3), label,0,0,0);

    lbl_Ch3 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh3), lbl_Ch3,0,0,0);



    hboxCh4 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh4);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh4),1);

    label = gtk_label_new("Channel 4:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh4), label,0,0,0);

    lbl_Ch4 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh4), lbl_Ch4,0,0,0);



    hboxCh5 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh5);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh5),1);

    label = gtk_label_new("Channel 5:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh5), label,0,0,0);

    lbl_Ch5 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh5), lbl_Ch5,0,0,0);



    hboxCh6 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh6);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh6),1);

    label = gtk_label_new("Channel 6:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh6), label,0,0,0);

    lbl_Ch6 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh6), lbl_Ch6,0,0,0);



    hboxCh7 = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh7);
    gtk_box_set_homogeneous (GTK_BOX(hboxCh7),1);

    label = gtk_label_new("Channel 7:    ");
    gtk_box_pack_start(GTK_BOX(hboxCh7), label,0,0,0);

    lbl_Ch7 = gtk_label_new("0.000");
    gtk_box_pack_start(GTK_BOX(hboxCh7), lbl_Ch7,0,0,0);


    hbox_Control = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hbox_Control);
    gtk_box_set_homogeneous (GTK_BOX(hbox_Control),1);

    btnAIn_Start_Stop = gtk_button_new_with_label( "Start");
    g_signal_connect (btnAIn_Start_Stop, "clicked", G_CALLBACK (Start_Stop), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_Control), btnAIn_Start_Stop,0,0,5);

    separator = gtk_separator_new (1);
    gtk_box_pack_start (GTK_BOX (vboxMain), separator, FALSE, TRUE, 5);
    gtk_widget_show (separator);

    hbox = gtk_box_new(0,0);
    gtk_container_add(GTK_CONTAINER(vboxMain), hbox);
    gtk_box_set_homogeneous (GTK_BOX(hbox),1);

    btnFlashLED = gtk_button_new_with_label ( "Flash LED");
    g_signal_connect(btnFlashLED, "clicked", G_CALLBACK(flash_led), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), btnFlashLED,0,0,0);

    btnQuit = gtk_button_new_with_label ( "Quit");
    g_signal_connect (btnQuit, "clicked", G_CALLBACK (gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), btnQuit,0,0,0);


    // get list of MCC 118s
    info_count = hat_list(HAT_ID_MCC_118, NULL);

    if (info_count > 0)
    {
        info_list = (struct HatInfo*)malloc(info_count * sizeof(struct HatInfo));
        hat_list(HAT_ID_ANY, info_list);

        //use the first one only (for this app)
        address = info_list[0].address;
        printf("Board: %d\n", address);
        mcc118_open(address);
    }
    else
    {
        printf("hat_list returned %d\n", info_count);
    }


  /* Enter the main loop */
    gtk_widget_show_all (window);
    gtk_main ();

    //Exit app...
    mcc118_close(address);
    free(tinfo);
    return 0;
}
