#include "qif_output.h"

#include "misc.h"

#include <stdio.h>


int qif_output(tran* first)
{
    printf("!Type:Bank\n");

    tran* tr = first;

    while(tr)
    {
        printf("D%d/%d/%d\n", tr->month, tr->day, tr->year);

        if (tr == first)
        {
            printf("T%d.%02d\n",
                        tr->bal / 100,
                       (tr->bal % 100) * (tr->bal > 0 ? 1 : -1));
            printf("POpening Balance\n");
        }
        else
        {
            printf("T%d.%02d\n",
                        tr->amt / 100,
                       (tr->amt % 100) * (tr->amt > 0 ? 1 : -1));

            printf("P%s\n", tr->descr);
        }

        printf("N%s\n", get_transaction_str(tr->type));

        printf("^\n");
        tr = tr->next;
    }

}


