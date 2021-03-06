#ifndef MISC_H
#define MISC_H


#include <stdio.h>


#define MAXLINELEN 512
#define IOBUFSIZE 1024

/*  two date functions to convert 3 letter month
    names to integers (1 - 12) and vice-versa
 */

int month_to_int(const char*);


const char* int_to_month(int);

const char* int_to_month_full(int);


int str_rtrim(char*);

/* append src to buf without exceeding buf_len in buf */
int str_append_n(char* buf, const char* src, int buf_len);


char* get_filename_ext(const char*);


typedef struct txtline
{
    char buf[MAXLINELEN];
    struct txtline* next;

} txtline;



txtline*    text_file_read(FILE*);
void        text_file_cleanup(txtline*);


#endif
