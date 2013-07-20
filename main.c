#include "statement.h"


#include "debug.h"
#include "misc.h"
#include "qif_output.h"
#include "trans_data.h"


#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    FILE* file;
    tran* tr = 0;
    int res = 0;

    if (argc != 2)
    {
        fprintf(stderr, "usage: statement_process [FILE]\n");
        exit(1);
    }

    if (!(file = fopen(argv[1], "r")))
    {
        fprintf(stderr,"could not open file:'%s'\n", argv[1]);
        exit(1);
    }

    trans_data_init();

    st_init();

    tr = st_process(file);

    #if DEBUG
    st_dump(tr);
    #endif

    res = qif_output(tr);

    st_free(tr);

    fclose(file);

    trans_data_cleanup();

    return res;
}
