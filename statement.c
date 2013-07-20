#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "debug.h"
#include "statement.h"
#include "trans_data.h"
#include "misc.h"


enum {  AC_ERR = -1,
        AC_UNKNOWN = 0,
        AC_HSBC,
        AC_NATWEST,
        AC_XXX_LAST_XXX
};


/*  struct for storing strings and positions of strings which aid
    in identifying the bank the statement is issued from
 */
typedef struct ac_id
{
    const char* id0;
    const char* id1;
    int pos0;
    int pos1;
} acid;


/*  statement field types, they're all pretty static regarding order,
    with the exception being STF_CR and STF_DR (paid in, and paid out)
    which are one way round in HSBC statements, and the other way round
    in Natwest statments.

    my code may appear to work with statements with more variations
    of field order, but that is not the case.
 */
enum {  STF_ERR = -1,
        STF_DATE = 0,
        STF_TYPE,
        STF_DESCR,
        STF_CR,
        STF_DR,
        STF_BAL,
        STF_XXX_LAST_XXX,
        STF_AMT1 = STF_CR,
        STF_AMT2 = STF_DR
};


/* struct for storing information about field/column position
 */
typedef struct st_field
{
    int type;
    int pos;
    int width;

} stf;


static char* fieldnames[STF_XXX_LAST_XXX];
static char* banknames[AC_XXX_LAST_XXX];

static stf  layout[STF_XXX_LAST_XXX];
static acid bankid[AC_XXX_LAST_XXX];


/* str should be first line of statement */
static int identify_bank(const char* str)
{
    int i;

    if (!str)
        return AC_ERR;

    /* try to identify bank */
    for (i = AC_UNKNOWN + 1; i < AC_XXX_LAST_XXX; ++i)
    {
        size_t l;
        const char* p = str;

        if (bankid[i].pos0 == -1)
            while(*p == ' ')
                ++p;
        else
            p = str + bankid[i].pos0;

        l = strlen(bankid[i].id0);

        if (strncmp(p, bankid[i].id0, l) != 0)
            continue; /* no match, try next bank id */

        if (bankid[i].pos1 == -1)
        {
            p += l;

            while(*p == ' ')
                ++p;
        }
        else
            p = str + bankid[i].pos1;

        l = strlen(bankid[i].id1);

        if (strncmp(p, bankid[i].id1, l) == 0)
            return i;
    }

    return AC_UNKNOWN;
}


/*  function to discover column position/spacing from column
    headings.
    str is the string containing the headings.
    *ptr will point to first non white space in str (ie first heading
    which is assumed to be "Date").
 */
static int position_init(const char* str, int bank)
{
    int f;

    const char* p = str;

    while (*p == ' ')
        ++p;

    if (strncasecmp(p, "Date", 4) != 0)
        return -1;

    layout[0].pos = p - str;
    p += 4 + 1;

    for (f = 1; *p != '\0' && f != STF_XXX_LAST_XXX;)
    {
        int i;

        while(*p == ' ')
            ++p;

        for (i = 0; i < STF_XXX_LAST_XXX; ++i)
        {
            size_t flen = strlen(fieldnames[i]);

            if (strncasecmp(p, fieldnames[i], flen) == 0)
            {
                layout[f].type = i;
                layout[f].pos = p - str;
                ++f;
                p += flen;

                break;
            }
        }

        if (i == STF_XXX_LAST_XXX)
        {
            fprintf(stderr, "Unrecognized field\n");
            /* unrecognized field, give up */
            return -1;
        }
    }

    switch(bank)
    {
    case AC_NATWEST:
        /* stable layout, calculate */
        for (f = 0; f < STF_XXX_LAST_XXX - 1; ++f)
            layout[f].width = layout[f + 1].pos - layout[f].pos - 1;
        break;

    case AC_HSBC:
        for (f = 0; f < STF_XXX_LAST_XXX; ++f)
        {
            switch(layout[f].type)
            {
            case STF_DATE:  layout[f].width = 6;    break;
            case STF_TYPE:  layout[f].width = 3;    break;
            case STF_DESCR: layout[f].width = 18;   break;
            case STF_CR:
            case STF_DR:
            case STF_BAL:   layout[f].width = 9;    break;
            }
        }

        break;
    }

    return 0;
}


/*  function to read a date from a string, where the date is in the
    format dd mmm yyyy (ie 23 Jun 2005). the year may be ommitted
    (ie HSBC) or be two or four digits (Natwest).
 */
static int /* returns number of characters read */
read_date(const char* buf, int* pday, int* pmonth, int* pyear)
{
    const char* p = buf;
    char tmp[4];
    int day = -1;
    int month = -1;
    int year = -1;

    if (pday)
        *pday = -1;
    if (pmonth)
        *pmonth = -1;
    if (pyear)
        *pyear = -1;

    if (*p < '0' || *p > '9')
        return -1;

    if (sscanf(p, "%d", &day) != 1)
        return -1;

    if (day < 1 || day > 31)
        return -1;

    /* can't assume day has leading zero */
    while (*++p != ' ' && *p != '\0');

    if (*p == '\0')
        return -1;

    ++p;

    /* read month */
    strncpy(tmp, p, 3);
    tmp[3] = '\0';

    if ((month = month_to_int(tmp)) == -1)
        return -1;

    p += 3;

    if (*p != ' ')
        return -1;

    ++p;

    if (*p >= '0' && *p <= '9')
    {
        if (sscanf(p, "%d", &year) == 1)
        {
            if (year < 0)
                return -1;
            else if (year < 100)
                p += 2;
            else if (year < 1970)
                return -1;
            else
                p += 4;
        }
        /*  force date to have a space immediately after it,
            or insist that it be last thing on line...
         */
        if (*p != ' ' && *p != '\0')
            return -1;

        ++p;
    }

    if (pday)
        *pday = day;
    if (pmonth)
        *pmonth = month;
    if (pyear)
        *pyear = year;

    return p - buf - 1;
}


int st_init(void)
{
    int i;

    for (i = 0; i < STF_XXX_LAST_XXX; ++i)
    {   /* Date hard coded as first field */
        layout[i].type = (i == 0) ? STF_DATE : STF_ERR;
        layout[i].pos = -1;
        layout[i].width = (i == 0) ? 6 : STF_ERR;
    }

    fieldnames[STF_DATE] =  "Date";
    fieldnames[STF_TYPE] =  "Type";
    fieldnames[STF_DESCR] = "Description";
    fieldnames[STF_CR] =    "Paid in";
    fieldnames[STF_DR] =    "Paid out";
    fieldnames[STF_BAL] =   "Balance";

    banknames[AC_UNKNOWN] = "Unknown";
    banknames[AC_HSBC] =    "HSBC";
    banknames[AC_NATWEST] = "Natwest";

    bankid[AC_UNKNOWN].id0 = NULL;
    bankid[AC_UNKNOWN].id1 = NULL;

    bankid[AC_HSBC].id0 = "Previous statements";
    bankid[AC_HSBC].pos0 = 0;
    bankid[AC_HSBC].id1 = "https://www.hsbc.co.uk/";
    bankid[AC_HSBC].pos1 = -1;
    bankid[AC_NATWEST].id0 = "Statement";
    bankid[AC_NATWEST].pos0 = -1;
    bankid[AC_NATWEST].id1 = "Account Number:";
    bankid[AC_NATWEST].pos1 = -1;

    return 0;
}


/*  although we garner field/column positions from the column headings,
    there's still some work to do... in particular for HSBC statements
    which seem to have a life of their own when it comes to column
    position and alignment
 */
static int find_positions(txtline* tl, int bank)
{
    const char* p;
    int n;

    while ((tl = tl->next))
    {
        p = tl->buf;

        while(*p == ' ')
            ++p;

        if ((n = read_date(p, 0, 0, 0)) > 0)
            break;
    }

    if (n == -1)
        return -1;

    layout[STF_DATE].width = n;

    p += n + 1;

    if (bank == AC_HSBC)
    {   /*  HSBC statements can only be downloaded dependent on your
            system capability to print to PDF file (for example). when
            converted to text, the transaction columns do not align with
            the header and require some tweaking.
         */

        const char* min;

        /*  type position nearly always seperated from date by a single
            space. there are of course random exceptions where the type
            actually aligns under the header.
         */
        layout[STF_TYPE].pos = p - tl->buf;

        /*  description always starts a few characters to the right of
            the description column.
         */
        p = tl->buf + layout[STF_DESCR].pos;

        while (*p == ' ')
            ++p;

        if (*p == '\0')
            return -1;

        layout[STF_DESCR].pos = p - tl->buf;

        /*  the balance sometimes starts a few characters to the left
            of the balance column.
         */

        min = tl->buf + layout[STF_BAL - 1].pos + layout[STF_BAL - 1].width;

        while ((tl = tl->next))
        {
            if ((int)strlen(tl->buf) < layout[STF_BAL].pos)
                continue;

            p = tl->buf;

            while(*p == ' ')
                ++p;

            if ((n = read_date(p, 0, 0, 0)) < 0)
                break;

            p = tl->buf + layout[STF_BAL].pos;

            while (*p != ' ' && p > min)
            {
                --p;

                if (*p != ' ')
                    --layout[STF_BAL].pos;
            }
        }

    }

    return 0;
}


tran* st_process(txtline* txtlines)
{
    /*char buf[BUFSIZE + 1];*/
    int ln = 1;
    int bank;
    int st_date_header = 1;
    int st_date_read = 0;
    int st_day = -1;
    int st_month = -1;
    int st_year = -1;
    int day = -1;
    int month = -1;
    int year = -1;
    int maxl = 0;

    int ready = 0;

    txtline* tl = txtlines;

    tran first = { -1, -1, -1, TR_ERR, "", "", 0, 0, NULL };

    tran* tr = &first;
    tran* tr_ml = 0; /* multi-line transaction */

    if ((bank = identify_bank(tl->buf)) == -1)
        return 0;

    debug("Statement looks like it's from %s\n", banknames[bank]);

    while (1)
    {
        int i;
        int n;
        int len;
        int type;
        int amt_sign = 1;

        char* p = 0;
        char* descr = 0;
        char* amt = 0;
        char* bal = 0;
        char* type_str = 0;

        if (!(tl = tl->next))
            break;

        p = tl->buf;

        if (bank == AC_NATWEST)
        {
            while (*p == '.')
                *p++ = ' ';
        }

        while (*p == ' ')
            ++p;

        len = strlen(tl->buf);

        if (maxl < len)
            maxl = len;

        debug("line: %3d (%3d chars) '%-*s'", ++ln, len, maxl, tl->buf);

        if (*tl->buf != ' ') /* skip lines not starting with space */
        {
            dbg("XXX skip");
            tr_ml = 0;
            continue;       /* (p points to first non space */
        }

        if (st_date_header)
        {
            if (bank == AC_HSBC)
            {
                const char* date_header = "Statement date:";

                if (strncasecmp(p, date_header, strlen(date_header)) == 0)
                {
                    dbg("XXX date header");
                    st_date_read = 1;
                    continue;
                }
            }
            else if (bank == AC_NATWEST)
            {
                const char* date_header[] = {   "Branch Details",
                                                "Your Details",
                                                "Period", 0     };

                for (n = 0; date_header[n] != 0; ++n)
                {
                    size_t dhl = strlen(date_header[n]);

                    if (strncmp(date_header[n], p, dhl) != 0)
                        break;

                    p += dhl;

                    while (*p == ' ')
                        ++p;

                    if (*p == '\0')
                        break;
                }

                if (date_header[n] == 0)
                {   /* wonderful! */
                    while (*p == ' ')
                        ++p;

                    st_date_header = 0;
                    st_date_read = 1;
                }
            }
        }

        if (st_date_read)
        {   /* HSBC statement date, p will point to first char of it */
            n = read_date(p, &st_day, &st_month, &st_year);

            if (n < 0)
                fprintf(stderr, "failed to read statement date\n");

            dbg("XXX statement date");

            st_date_read = 0;
            /* continue to next line, regardless */
            continue;
        }

        if (position_init(tl->buf, bank) == 0)
        {
            dbg("XXX header");

            if (find_positions(tl, bank) != 0)
                return 0;

            tr_ml = 0;
            ready = 1;
            continue;
        }

        if (!ready)
        {
            dbg("XXX ignore");
            continue;
        }

        /*  The Natwest "brought forward" line breaks everything so
            requires "special" handling...
         */
        if (bank == AC_NATWEST)
        {
            char bal_header[] = "BROUGHT FORWARD";

            if (strncmp(bal_header, p, strlen(bal_header)) == 0)
            {
                type = TR_BALANCE;
                day = st_day;
                month = st_month;
                year = st_year;
                descr = p;
                type_str = p;
                dbg("XXX balance");
                /* ok here we go, the special bit... */
                goto find_balance;
            }
        }

        if (len < layout[STF_DATE].pos + layout[STF_DATE].width)
        {
            dbg("XXX ignored");
            tr_ml = 0;
            continue;
        }

        p = tl->buf + layout[STF_DATE].pos;

        /*  Allow for transactions for which no date is specified, ie
            Natwest, where the date is specified only for the first
            transaction of the day - (date will contain spaces only).
         */
        for (n = layout[STF_DATE].width; n > 0; --n)
            if (*(p + n) != ' ' || *(p + n) == '\0')
                break;

        if (n) /* only read date if date column not empty */
            n = read_date(p, &day, &month, &year);

        if (n < 0)
        {
            dbg("XXX ignored");
            tr_ml = 0;
            continue;
        }
        else if (n == 0)
        {
            if (bank == AC_NATWEST)
            {
                if (transaction_append(tr_ml,
                                        tl->buf + layout[STF_TYPE].pos,
                                        tl->buf + layout[STF_DESCR].pos))
                {
                    dbg("XXX multiline");
                    continue;
                }
            }
        }

        if (year == -1)
        {
            year = st_year;

            if (st_month == 1 && month == 12)
                --year;
        }

        if (len < layout[STF_AMT1].pos)
        {   /* not a transaction (accepting spaces as date gets us here) */
            dbg("XXX ignored");
            tr_ml = 0;
            continue;
        }

        if (layout[STF_TYPE].pos < len)
        {
            p = tl->buf + layout[STF_TYPE].pos;

            if (bank == AC_HSBC)
                while(*p == ' ')
                    ++p;

            type_str = p;

            if ((type = get_transaction_type(p)) == TR_ERR)
            {
                dbg("XXX ignored");
                tr_ml = 0;
                continue;
            }
        }

        dbg("XXX ok");

        if (layout[STF_DESCR].pos < len)
            p = descr = tl->buf + layout[STF_DESCR].pos;

        for (i = STF_AMT1; i <= STF_AMT2; ++i)
        {
            if (layout[i].pos < len)
            {
                /* is column empty? */
                p = amt = tl->buf + layout[i].pos;

                while (*p == ' ' && p < amt + layout[i].width)
                    ++p;

                if (*p != ' ')
                {
                    amt_sign = (layout[i].type == STF_DR) ? -1 : 1;
                    break;
                }

                amt = 0;
            }
        }

find_balance:
        if (layout[STF_BAL].pos < len)
        {
            p = bal = tl->buf + layout[STF_BAL].pos;

            while (*p == ' ' && *p != '\0')
            /*p < amt + layout[STF_BAL].width)*/
                ++p;

            if (*p == ' ' || *p == '\0')
                bal = 0;
        }

        /*  trim description and amount to field width, but do not
            do the same for the balance, because a) it might be followed
            by a D to indicate overdrawn, and b) it's the last field
            so trimming (by length restricted duplication) is
            unnecessary. */

        if (descr)
            descr = strndup(descr, layout[STF_DESCR].width);

        if (amt)
            amt = strndup(amt, layout[STF_AMT1].width);

        if ((tr->next = transaction_new(day, month, year, type, type_str, 
                                                descr, amt, amt_sign, bal)))
        {
            tr = tr->next;
            tr_ml = tr;
        }

        if (descr)
            free(descr);

        if (amt)
            free(amt);
    }

    return first.next;
}


void st_dump(tran* tr)
{
    int tot = (tr ? tr->bal : 0);

    if (!tr)
        return;

    while (tr)
    {
        int amt_maj, amt_min, bal_maj, bal_min, tot_maj, tot_min;

        tot += tr->amt;

        printf("transaction: %02d/%02d/%4d\ttype: %3s\tdescr: %-*s",
                tr->day, tr->month, tr->year,
                get_transaction_str(tr->type), layout[STF_DESCR].width,
                tr->descr);

        amt_maj = tr->amt / 100;
        amt_min = (tr->amt % 100) * (tr->amt < 0 ? -1 : 1);

        bal_maj = tr->bal / 100;
        bal_min = (tr->bal % 100) * (tr->bal < 0 ? -1 : 1);

        tot_maj = tot / 100;
        tot_min = (tot % 100) * (tot < 0 ? -1 : 1);

        if (amt_maj == 0 && tr->amt < 0)
            printf("\tamount:    -0.%02d", amt_min);
        else
            printf("\tamount:%6d.%02d",  amt_maj, amt_min);

        if (bal_maj == 0 && tr->bal < 0)
            printf("\tbalance:    -0.%02d", bal_min);
        else
            printf("\tbalance:%6d.%02d", bal_maj, bal_min);

        if (tot_maj == 0 && tot < 0)
            printf("\ttotal:    -0.%02d\n", tot_min);
        else
            printf("\ttotal:%6d.%02d\n", tot_maj, tot_min);

        tr = tr->next;
    }
}


void st_free(tran* tr)
{
    while(tr)
    {
        tran* tmp = tr;
        tr = tr->next;
        free(tmp);
    }
}
