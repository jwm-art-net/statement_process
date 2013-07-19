#include "statement.h"

#include "trans_data.h"

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    FILE* file;

    if (argc != 2)
    {
        fprintf(stderr, "usage: txt_statement_process [FILE]\n");
        exit(1);
    }

    if (!(file = fopen(argv[1], "r")))
    {
        fprintf(stderr,"could not open file:'%s'\n", argv[1]);
        exit(1);
    }

    trans_data_init();

    st_init();

    st_process(file);

    fclose(file);

    return 0;
}
