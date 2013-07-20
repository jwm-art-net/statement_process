#include "misc.h"

#include <string.h>



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

    return -1;
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


int str_rtrim(char* p)
{
    char* lc = p;

    while (*p != '\0')
    {
        if (*p != ' ' && *p !='\t')
            lc = p;
        ++p;
    }

    *++lc = '\0';

    return p - lc;
}


int str_append_n(char* buf, const char* src, int buf_len)
{
    size_t l1, l2;
    char* p = buf;

    p = buf + strlen(buf);

    if (p > buf)
        *p++ = ' ';

    l1 = buf_len - (p - buf);
    l2 = strlen(src);

    strncpy(p, src, l1);
    buf[buf_len - 1] = '\0';

    return (l2 < l1 ? l2 : l1);
}

