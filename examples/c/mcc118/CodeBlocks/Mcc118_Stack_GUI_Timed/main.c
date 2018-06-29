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

    GtkWidget *window = NULL;
    GtkWidget *btnAIn_Start_Stop;
    GtkWidget *lbl_Ch[64];



/*************************************************************************/
static void activate (GtkApplication *app, gpointer user_data)
{}

static void my_getsize(GtkWidget *widget, GtkAllocation *allocation, void *data) {
    printf("width = %d, height = %d\n", allocation->width, allocation->height);
}

static void *Analog_In (void *arg)
{
    //Read each analog input channel, and get one sample
    while (done == FALSE)
    {
        for (int index = 0; index < info_count; index++)
        {
            address = info_list[index].address;
            mcc118_open(address);
            //mcc118_open(info_list[index].address);
            for (int channel = 0; channel < mcc118_a_in_num_channels(); channel++)
            {
                mcc118_a_in_read(address, channel, 0, &value);
                //printf("   Channel %d: %7.3f\n", channel, value);
                sprintf(buf, "%7.3f", value );
                gtk_label_set_text(GTK_LABEL(lbl_Ch[index *8 + channel]), buf);
            }
            mcc118_close(address);
        }
        usleep(500 *1000);  //Wait 250 mS
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
     for (int index = 0; index < info_count; index++)
    {
        //Flash the devices LED 5 times.
        address = info_list[index].address;
        int Dev_status =  mcc118_blink_led(address, 5);
    }
}

int main (int argc, char *argv[])
{

    GtkWidget *vboxMain, *label;
    GtkWidget *hboxCh[info_count * 8];
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
   // g_signal_connect(window, "size-allocate", G_CALLBACK(my_getsize), NULL);
    g_signal_connect (window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_widget_realize (window);

    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vboxMain=gtk_box_new(TRUE,0);
    //g_signal_connect(vboxMain, "size-allocate", G_CALLBACK(my_getsize), NULL);
    gtk_container_add(GTK_CONTAINER(window), vboxMain);



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

        for (int index = 0; index < info_count; index++)
        {
            gchar Board_Label[50] = "Board ";  //"";
            gchar buf[10]="";
            sprintf(buf, "%d: ", index );
            strcat(Board_Label,  buf );
            label = gtk_label_new(Board_Label);
            gtk_box_pack_start(GTK_BOX(vboxMain), label,0,0,0);

            mcc118_open(address);
            for (int channel = 0; channel < mcc118_a_in_num_channels(); channel++)
            {

                    hboxCh[index *8 + channel] = gtk_box_new(0,0);
                    gtk_container_add(GTK_CONTAINER(vboxMain), hboxCh[index *8 + channel]);
                    //g_signal_connect(hboxCh[index *8 + channel], "size-allocate", G_CALLBACK(my_getsize), NULL);
                    gtk_box_set_homogeneous (GTK_BOX(hboxCh[index *8 + channel]),1);

                    gchar Channel_Label[50] = ("Channel ");
                    sprintf(buf, "%d: ", channel );
                    strcat(Channel_Label, buf);
                    label = gtk_label_new(Channel_Label);
                    //g_signal_connect(label, "size-allocate", G_CALLBACK(my_getsize), NULL);
                    gtk_box_pack_start(GTK_BOX(hboxCh[index *8 + channel]), label,0,0,0);

                    lbl_Ch[index *8 + channel] = gtk_label_new("0.000");
                    //g_signal_connect(lbl_Ch[index *8 + channel], "size-allocate", G_CALLBACK(my_getsize), NULL);
                    gtk_box_pack_start(GTK_BOX(hboxCh[index *8 + channel]), lbl_Ch[index *8 + channel] ,0,0,0);
            }

            separator = gtk_separator_new (1);
            gtk_box_pack_start (GTK_BOX (vboxMain), separator, FALSE, TRUE, 5);
            gtk_widget_show (separator);
        }
    }
    else
    {
        printf("hat_list returned %d\n", info_count);
    }

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



  /* Enter the main loop */
    gtk_widget_show_all (window);
    gtk_main ();

    //Exit app...
    mcc118_close(address);
    return 0;
}
