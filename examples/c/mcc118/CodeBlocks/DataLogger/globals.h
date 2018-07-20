#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

// GLOBALS should be defined in the file globals.c so that the global
// variables are declared in that file.  All other .c files should not
// define GLOBALS so that the varaibles are handled as extern.


#ifdef GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

#include <gtk/gtk.h>
#include "gtkdatabox.h"

#include "daqhats/daqhats.h"

#define MAX_118_CHANNELS 8
#define OPTS_BACKGROUND 0


typedef struct graph_channel_info
{
    GtkDataboxGraph*    graph;
    GdkRGBA*            color;
    uint                channelNumber;
    uint                readBufStartIndex;
    gfloat*             X;
    gfloat*             Y;
} GraphChannelInfo;

EXTERN GtkWidget *window;

EXTERN GtkWidget *box;
EXTERN GtkWidget *table;


EXTERN GtkWidget *rbContinuous, *rbFinite;
    //,*chkBurstMode, *chkExtTrig, *chkExtClock;
EXTERN GtkWidget *spinRate;
EXTERN GtkWidget *spinNumSamples;
EXTERN GtkWidget *btnSelectLogFile;
EXTERN GtkWidget *btnQuit;
EXTERN GtkWidget *chkChan[MAX_118_CHANNELS];
EXTERN GtkWidget *btnStart_Stop;

EXTERN GraphChannelInfo graphChannelInfo[MAX_118_CHANNELS];
EXTERN GdkRGBA legendColor[MAX_118_CHANNELS];

//EXTERN char* application_name;// = "MCC 118 Data Logger";
//EXTERN char *filename;// = "/home/pi/Desktop/csv_test.csv";

EXTERN uint8_t address;
EXTERN uint8_t channel_mask;//, numchannels;
//EXTERN uint16_t read_status;// = 0;
//EXTERN uint32_t samples_read_per_channel;//  = 0;
//EXTERN uint32_t buffer_size_samples;//  = 0;
//EXTERN int iNumSamplesPerChannel;//  = 0;
//EXTERN uint16_t options = OPTS_BACKGROUND;
EXTERN double scan_timeout;//  = 0.0;
EXTERN gboolean done;//  = FALSE;

EXTERN int iNumSamplesPerChannel;
EXTERN double iRatePerChannel;

EXTERN pthread_t threadh;
EXTERN struct thread_info *tinfo;
//
//EXTERN int loop_count;//  = 0;
//EXTERN gint gtk_status;//  = 0;
//EXTERN double iRatePerChannel;//  = 0.0;
//
//
EXTERN char application_name[512];//  = "MCC 118 Data Logger";
EXTERN char csv_filename[512];//  = "/home/pi/Desktop/csv_test.csv";
//
EXTERN GMutex data_mutex;
//
//EXTERN GCond data_cond;
//EXTERN GSource *source;//  = NULL;
EXTERN GMainContext *context;
//EXTERN guint id;
//
//EXTERN GCond error_cond;
//EXTERN GSource *error_source;//  = NULL;
//EXTERN GMainContext *error_context;
//EXTERN guint error_id;
//
EXTERN GTimer *timer;
EXTERN gulong msec;

EXTERN char error_message[256];
EXTERN GMainContext *error_context;

#endif // GLOBALS_H_INCLUDED
