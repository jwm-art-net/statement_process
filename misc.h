#ifndef MISC_H
#define MISC_H


/*  two date functions to convert 3 letter month
    names to integers (1 - 12) and vice-versa
 */

int month_to_int(const char*);


const char* int_to_month(int);

const char* int_to_month_full(int);



#endif
