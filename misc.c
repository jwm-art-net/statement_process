#include "misc.h"


static const char* months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char* months_full[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};


int month_to_int(const char* str)
{
    int m;

    if (!str)
        return -1;

    for (m = 0; m < 12; ++m)
        if (strcasecmp(months[m], str) == 0)
            return m + 1;
}


const char* int_to_month(int m)
{
    if (m < 1 || m > 12)
        return 0;

    return months[m - 1];
}


const char* int_to_month_full(int m)
{
    if (m < 1 || m > 12)
        return 0;

    return months_full[m - 1];
}

