#include "misc.h"


#include <string.h>
#include <stdlib.h>



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


char* get_filename_ext(const char* fname)
{
    const char* d = strrchr(fname, '.');

    if (!d)
        return 0;

    ++d;

    if (*d == '\0')
        return 0;

    return strdup(d);
}


txtline* text_file_read(FILE* file)
{
    char buf[IOBUFSIZE];
    txtline first = { "", NULL };
    txtline* tl = &first;

    int ln = 1;

    while(fgets(buf, IOBUFSIZE * sizeof(char), file))
    {
        size_t nl;
        buf[IOBUFSIZE - 1] = '\0';
        nl = strlen(buf) - 1;

        if (nl >= MAXLINELEN)
        {
            fprintf(stderr, "truncating line %d\n", ln);
            nl = MAXLINELEN - 1;
            buf[nl] = '\0';
        }

        if (buf[nl] =='\r' || buf[nl] == '\n')
            buf[nl] = '\0';

        if (!(tl->next = malloc(sizeof(*tl))))
            break;

        tl = tl->next;

        strcpy(tl->buf, buf);
        ++ln;
    }

    tl->next = 0;

    return first.next;
}


void text_file_cleanup(txtline* tl)
{
    while(tl)
    {
        txtline* tmp = tl;
        tl = tl->next;
        free(tmp);
    }
}

