#include "trans_data.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "misc.h"



int         get_transaction_type(const char*);
const char* get_transaction_str(int type);


#define XFER_STRLEN     30
#define XFER_DESCRLEN   40

typedef struct xfer
{
    int  type;
    char str[XFER_STRLEN + 1];
    struct xfer* next;
} xfer;


static xfer xfers = { TR_ERR, "", NULL };
static xfer* lastxferlookup = 0;

static xfer* xfers_add(const char* key, int type)
{
    xfer* newtr = 0;
    xfer* tr = 0;

    newtr = malloc(sizeof(xfer));
    strncpy(newtr->str, key, XFER_STRLEN);
    newtr->str[XFER_STRLEN] = '\0';
    newtr->type = type;
    newtr->next = 0;

    tr = &xfers;

    while (tr->next)
        tr = tr->next;

    tr->next = newtr;

    return newtr;
}



void trans_data_init(void)
{
    xfers_add("Balance carried forward",TR_CLOSING_BALANCE);

    xfers_add("Bal",                    TR_BALANCE);
    xfers_add("Balance brought forward",TR_BALANCE);
    xfers_add("BROUGHT FORWARD",        TR_BALANCE);

    xfers_add("ATM",                    TR_ATM);
    xfers_add("CASH WITHDRAWAL",        TR_ATM);

    xfers_add("BAC",    TR_BAC);
    xfers_add("BGC",    TR_BGC);
    xfers_add("BSP",    TR_BSP);
    xfers_add("BP",     TR_BP);

    xfers_add("CDM",                    TR_CDM);
    xfers_add("CASH & DEP MACHINE",     TR_CDM);

    xfers_add("CHG",    TR_CHARGE);
    xfers_add("Charge", TR_CHARGE);

    xfers_add("CHP",    TR_CHP);
    xfers_add("CHQ",    TR_CHQ);
    xfers_add("CIR",    TR_CIR);
    xfers_add("CUI",    TR_CUI);

    xfers_add("CR",                     TR_CR);
    xfers_add("AUTOMATED CREDIT",       TR_CR);

    xfers_add("CWP",    TR_CWP);

    xfers_add("DD",                     TR_DD);
    xfers_add("D/D",                    TR_DD);
    xfers_add("DIRECT DEBIT",           TR_DD);

    xfers_add("DIV",    TR_DIV);
    xfers_add("DR",     TR_DR);
    xfers_add("DWP",    TR_DWP);
    xfers_add("ERTF",   TR_ERTF);
    xfers_add("IBP",    TR_IBP);
    xfers_add("INT",    TR_INT);
    xfers_add("ITL",    TR_ITL);
    xfers_add("MAE",    TR_MAE);
    xfers_add("NDC",    TR_NDC);

    xfers_add("OTR",                    TR_OTR);
    xfers_add("ONLINE TRANSACTION",     TR_OTR);

    xfers_add("POC",    TR_POC);

    xfers_add("POS",                    TR_POS);
    xfers_add("DEBIT CARD TRANSACTION", TR_POS);
    xfers_add("DEBIT CARD",             TR_POS);

    xfers_add("SO",                     TR_SO);
    xfers_add("S/O",                    TR_SO);
    xfers_add("STANDING ORDER",         TR_SO);

    xfers_add("SBT",    TR_SBT);
    xfers_add("TEL",    TR_TEL);
    xfers_add("TFR",    TR_TFR);
    xfers_add("TLR",    TR_TLR);
    xfers_add("VIS",    TR_VIS);
    xfers_add("(((",    TR_CONTACTLESS);

    {
        int n;

        for (n = TR_BALANCE; n < TR_XXX_LAST_XXX; ++n)
        {
            if (!get_transaction_str(n))
                fprintf(stderr,"ERROR: string not set for xfer id:%d\n",n);
        }
    }
}


void trans_data_cleanup(void)
{
    xfer* xf = xfers.next;

    while(xf)
    {
        xfer* tmp = xf;
        xf = xf->next;
        free(tmp);
    }
}


int get_transaction_type(const char* str)
{
    xfer* tr = xfers.next;

    if (!str)
        return TR_ERR;

    while (tr)
    {
        if (strncasecmp(str, tr->str, strlen(tr->str)) == 0)
            return tr->type;
        tr = tr->next;
    }

    return TR_ERR;
}


const char* get_transaction_str(int type)
{
    xfer* tr = 0;

    if (type <= TR_ERR || type >= TR_XXX_LAST_XXX)
        return 0;

    tr = xfers.next;

    while (tr)
    {
        if (tr->type == type)
        {
            lastxferlookup = tr;
            return tr->str;
        }

        tr = tr->next;
    }

    return 0;
}


const char* get_transaction_str_next(int type)
{
    if (!lastxferlookup)
        return 0;

    if (!(lastxferlookup = lastxferlookup->next))
        return 0;

    if (lastxferlookup->type != type)
        lastxferlookup = 0;

    return (lastxferlookup) ? lastxferlookup->str : 0;
}




char* mon_val_brut(const char* str)
{
    char tmp[80];
    char* r = tmp;
    char* w = tmp;

    sscanf(str, "%[+-0123456789,. D]", tmp);

    while (*r != '\0')
    {
        if (*r != ',')
            *w++ = *r;
        ++r;
    }

    *w = *r;

    return strdup(tmp);
}


tran*   transaction_new(int day, int month, int year, int type,
                                                const char* type_str,
                                                const char* descr,
                                                const char* amount,
                                                int amount_sign,
                                                const char* balance)
{
    const char* descr_p = descr;
    const char* type_p = type_str;
    const char* amt_p = 0;
    const char* amt_dp = 0;
    const char* bal_p = 0;
    const char* bal_dp = 0;
    char* amt = 0;
    char* bal = 0;
    int bal_sign = 1;
    tran* tr;

    if (day < 1 || day > 31)
    {
        fprintf(stderr, "transaction has invalid day:'%d'\n", day);
        return 0;
    }

    if (month < 1 || month > 12)
    {
        fprintf(stderr, "transaction has invalid month:'%d'\n", month);
        return 0;
    }

    if (year < 0 || year > 2100)
    {
        fprintf(stderr, "transaction has invalid year:'%d'\n", year);
        return 0;
    }

    if ((type < TR_BALANCE || type >= TR_XXX_LAST_XXX)
        && type != TR_CLOSING_BALANCE /* hmmmmph */)
    {
        fprintf(stderr, "transaction has invalid type:'%d'\n", type);
        return 0;
    }

    while (*descr_p == ' ')
        ++descr_p;

    if (amount)
    {
        amt_p = amt = mon_val_brut(amount);

        while (*amt_p == ' ')
            ++amt_p;

        if (*amt_p == '\0')
        {
            free(amt);
            return 0;
        }

        amt_dp = amt_p;

        while (*amt_dp != '.' && *amt_dp !=' ' && *amt_dp != '\0')
            ++amt_dp;

        if (*amt_dp != '.')
            amt_dp = 0;
    }

    if (balance)
    {
        bal_p = bal = mon_val_brut(balance);

        while (*bal_p == ' ')
            ++bal_p;

        if (*bal_p != '\0')
        {
            const char* bal_dr = bal_p;

            if (*bal_p == '-')
            {
                bal_sign = -1;
                ++bal_p;
            }

            bal_dp = bal_p;

            while (*bal_dp != '.' && *bal_dp != ' ' && *bal_dp != '\0')
                ++bal_dp;

            if (*bal_dp != '.')
                bal_dp = 0;

            while (*bal_dr != '\0')
            {
                if (*bal_dr == 'd' || *bal_dr == 'D')
                {
                    bal_sign = -1;
                    break;
                }

                ++bal_dr;
            }
        }
    }

    if (year < 100)
        year += 2000;

    tr = malloc(sizeof(tran));
    tr->day = day;
    tr->month = month;
    tr->year = year;
    tr->type = type;

    strncpy(tr->descr, descr_p, TR_DESCR_LEN - 1);
    tr->descr[TR_DESCR_LEN - 1] = '\0';
    str_rtrim(tr->descr);

    strncpy(tr->type_str, type_p, TR_TYPE_LEN - 1);
    tr->type_str[TR_TYPE_LEN - 1] = '\0';
    str_rtrim(tr->type_str);

    tr->amt = tr->bal = 0;

    if (amount)
    {
        int small = 0;

        if (sscanf(amt_p, "%d", &tr->amt) != 1)
        {
            free(tr);
            free(amt);
            free(bal);
            return 0;
        }

        if (amt_dp)
        {
            ++amt_dp;

            sscanf(amt_dp, "%d", &small);

            if (small < 0 || small > 100)
            {
                fprintf(stderr, "transaction amount error:%d\n", small);
                free(tr);
                free(amt);
                free(bal);
                return 0;
            }
        }

        tr->amt *= 100 * amount_sign;
        tr->amt += small * amount_sign;
    }

    if (balance)
    {
        int small = 0;

        if (sscanf(bal_p, "%d", &tr->bal) != 1)
        {
            fprintf(stderr, "failed to read balance\n");
            free(tr);
            free(amt);
            free(bal);
            return 0;
        }

        if (bal_dp)
        {
            ++bal_dp;

            sscanf(bal_dp, "%d", &small);

            if (small < 0 || small > 100)
            {
                fprintf(stderr, "transaction balance error:%d\n", small);
                free(tr);
                free(amt);
                free(bal);
                return 0;
            }
        }

        tr->bal *= 100 * bal_sign;
        tr->bal += small * bal_sign;
    }

    tr->next = 0;

    free(amt);
    free(bal);

    return tr;
}


tran*
transaction_append(tran* tr, const char* type_str, const char* descr)
{
    if (!tr)
        return 0;

    if (type_str)
    {
        str_append_n(tr->type_str, type_str, TR_TYPE_LEN);
        tr->type = get_transaction_type(tr->type_str);
    }

    if (descr)
        str_append_n(tr->descr, descr, TR_DESCR_LEN);

    return tr;
}


