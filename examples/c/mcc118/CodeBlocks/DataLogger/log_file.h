#ifndef LOG_FILE_H_INCLUDED
#define LOG_FILE_H_INCLUDED

#include <stdio.h>

extern FILE* log_file_ptr;

extern char* choose_log_file(GtkWidget *parent_window, char* default_file_name);
extern FILE* open_log_file (char* filename);
extern int write_log_file(FILE* log_file_ptr, double* read_buf, 
    int numberOfSamplesPerChannel, int numberOfChannels);

#endif // LOG_FILE_H_INCLUDED
