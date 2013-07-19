#ifndef TRANS_DATA_H
#define TRANS_DATA_H

#define TR_DESCR_LEN 80


enum {  TRDIR_OUT = -1,
        TRDIR_BI = 0,
        TRDIR_IN = 1
};


enum {  TR_ERR = -1,
        TR_BALANCE = 0,

        TR_ATM,         /* Automated Teller (Cash) Machine */
        TR_BAC,         /* Automated Credit */
        TR_BGC,         /* Bank Giro Credit */
        TR_BSP,         /* Branch Single Payment */
        TR_BP,          /* Bill Payment */
        TR_CDM,         /* Cash & Deposit Machine */
        TR_CHARGE,      /* Charge */
        TR_CHP,         /* Payment by CHAPS */
        TR_CHQ,         /* Cheque */
        TR_CIR,         /* Cirrus/Maestro card transaction */
        TR_CUI,         /* Centralised Unpaid In */
        TR_CR,          /* Credit */
        TR_CWP,         /* Cold Weather Payment */
        TR_DD,          /* Direct Debit */
        TR_DIV,         /* Dividend */
        TR_DR,          /* Debit */
        TR_DWP,         /* Department for Work and Pensions */
        TR_ERTF,        /* Exchange Rate Transaction Fee */
        TR_IBP,         /* Inter-Branch Payment */
        TR_INT,         /* Interest */
        TR_ITL,         /* International */
        TR_MAE,         /* Maestro Debit Card Transaction */
        TR_NDC,         /* Non Dividend Counterfoil */
        TR_OTR,         /* Online Banking Transaction */
        TR_POC,         /* Post Office Counter */
        TR_POS,         /* Point of Sale/debit card */
        TR_SO,          /* Standing Order */
        TR_SBT,         /* Screen Based Transaction */
        TR_TEL,         /* Telephone Banking */
        TR_TFR,         /* Transfer */
        TR_TLR,         /* Teller Transaction */
        TR_VIS,         /* Visa */
        TR_CONTACTLESS, /* ((( */

        TR_XXX_LAST_XXX
};


void        trans_data_init(void);

int         get_transaction_type(const char*);
const char* get_transaction_str(int type);
const char* get_transaction_str_next(int type);



typedef struct transaction
{
    int     day;
    int     month;
    int     year;

    int     type;

    char    descr[TR_DESCR_LEN];

    int     amt_sign;
    int     amt_major;
    int     amt_minor;

    int     bal_sign;
    int     bal_major;
    int     bal_minor;

    struct  transaction* next;

} tran;


tran*   transaction_new(int day, int month, int year, int type,
                                            const char* descr,
                                            const char* amount,
                                            int amount_sign,
                                            const char* balance);

#endif
