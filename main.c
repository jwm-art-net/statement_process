#include "statement.h"


#include "categories.h"
#include "debug.h"
#include "misc.h"
#include "qif_output.h"
#include "trans_data.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FNAME_LEN 1024


int main(int argc, char** argv)
{
    FILE* file;
    tran* tr = 0;
    int res = 0;
    char* fnext = 0;
    char* fname = argv[1];

    txtline* text = 0;
    txtline* cattext = 0;

    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "usage: statement_process STATEMENTFILE [CATEGORYFILE]\n");
        exit(1);
    }

    fnext = get_filename_ext(fname);

    if (fnext != 0 && strcasecmp("PDF", fnext) == 0)
    {
        char cmd[FNAME_LEN];
        txtline* tl;
        int n;

        free(fnext);

        snprintf(cmd, FNAME_LEN - 1,
                "pdftotext -layout -nopgbrk %s -", fname);

        if (!(file = popen(cmd, "r")))
        {
            fprintf(stderr, "failed to convert PDF '%s'\n", fname);
            exit(-1);
        }

        text = text_file_read(file);
        pclose(file);

        if (!text
         || strcasecmp("I/O Error: Couldn't open file", text->buf) == 0)
        {
            fprintf(stderr, "failed to convert PDF '%s'\n", fname);
            exit(-1);
        }

        for (tl = text, n = 0; n < 3 && tl != 0; ++n)
        {
            if (strcasecmp("Syntax Error", tl->buf) == 0)
            {
                fprintf(stderr, "failed to convert PDF '%s'\n", fname);
                text_file_cleanup(text);
                exit(-1);
            }
            tl = tl->next;
        }
    }
    else
    {
        free(fnext);

        if (!(file = fopen(fname, "r")))
        {
            fprintf(stderr, "failed to read file '%s'\n", fname);
            exit(-1);
        }

        text = text_file_read(file);
        fclose(file);

        if (!text)
        {
            fprintf(stderr, "failed to read file '%s'\n", fname);
            exit(-1);
        }
    }


    if (argc == 3)
    {
        char* catfname = argv[2];
        FILE* catfile = fopen(catfname, "r");

        if (!catfile)
            fprintf(stderr, "failed to read category file '%s'\n",
                                                            catfname);
        else
        {
            cattext = text_file_read(catfile);
            fclose(catfile);

            if (!cattext)
                fprintf(stderr, "failed to read category file '%s'\n",
                                                            catfname);
        }
    }


    trans_data_init();

    st_init();
    tr = st_process(text);
    text_file_cleanup(text);
    categories_init(cattext);
    text_file_cleanup(cattext);

    #if DEBUG
    st_dump(tr);
    #endif

    res = qif_output(tr, stdout);

    categories_cleanup();
    trans_data_cleanup();
    st_free(tr);

    return res;
}
