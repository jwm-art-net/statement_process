#include "qif_output.h"

#include "misc.h"

#include <stdio.h>


int qif_output(tran* first, FILE* file)
{
    if (!first)
        return -1;

    fprintf(file, "!Type:Bank\n");

    tran* tr = first;

    while(tr)
    {
        fprintf(file, "D%d/%d/%d\n", tr->month, tr->day, tr->year);

        if (tr == first)
        {
            fprintf(file, "T%d.%02d\n", tr->bal / 100,
                       (tr->bal % 100) * (tr->bal > 0 ? 1 : -1));
            fprintf(file, "POpening Balance\n");
        }
        else
        {
            fprintf(file, "T%d.%02d\n", tr->amt / 100,
                       (tr->amt % 100) * (tr->amt > 0 ? 1 : -1));

            fprintf(file, "P%s\n", tr->descr);
        }

        fprintf(file, "N%s\n", get_transaction_str(tr->type));

        fprintf(file, "^\n");
        tr = tr->next;
    }

    return 0;
}


