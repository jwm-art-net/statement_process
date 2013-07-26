#include "statement.h"


#include "categories.h"
#include "debug.h"
#include "misc.h"
#include "qif_output.h"
#include "trans_data.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>


#define FNAME_LEN 1024


static txtline* open_and_read_file(const char*);


void help()
{
    puts(  "Usage: statement_process [OPTION]... [FILE]...");
    puts(  "process an online banking statement to some other format.");
    puts(  "  -c, --cat-file=FILE       read categories from FILE");
    puts(  "  -q, --qif=FILE            output in QIF format to FILE");
    puts(  "  -h, --help                this help.");
}


int main(int argc, char** argv)
{
    FILE* file;
    tran* tr = 0;
    int res = 0;
    char*   catname = 0;
    char*   qifname = 0;

    txtline* alltext = 0;
    txtline* cattext = 0;


    static struct option long_opts[] =
    {
        { "cat-file",   required_argument,  0, 'c' },
        { "qif",        required_argument,  0, 'q' },
        { "help",       no_argument,        0, 'h' },
        { 0, 0, 0, 0 }
    };

    int opt_index = 0;
    int opt;


    if (argc == 1)
    {
        help();
        exit(1);
    }

    while ((opt = getopt_long(argc, argv, "c:q:h",
                              long_opts, &opt_index)) != -1)
    {
        switch(opt)
        {
        case 'c':   catname = optarg;   break;
        case 'q':   qifname = optarg;   break;
        case 'h':   help();             exit(1);
        case ':':
            fprintf(stderr, "option --%s requires an argument\n",
                            long_opts[optopt].name);
            break;
        }
    }

    while (optind < argc)
    {
        txtline* tl = 0;
        char* fname = argv[optind];

        tl = open_and_read_file(fname);

        txtline_cleanup(tl);

    }

    return 0;
}
/*
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr,
            "usage: statement_process STATEMENTFILE [CATEGORYFILE]\n");
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
*/


static txtline* open_and_read_pdf(const char fname)
{
    char cmd[FNAME_LEN] = { 0 };
    txtline* tl = 0;
    txtline* t;
    FILE* file = 0;
    int n = 0;

    snprintf(cmd, FNAME_LEN - 1, "pdftotext -layout -nopgbrk %s -", fname);

    if (!(file = popen(cmd, "r")))
    {
        fprintf(stderr, "failed to open PDF '%s'\n", fname);
        exit(-1);
    }

    tl = text_file_read(file);
    pclose(file);

    if (!tl || strcasecmp("I/O Error: Couldn't open file", text->buf) == 0)
    {
        fprintf(stderr, "failed to convert PDF '%s'\n", fname);
        return 0;
    }

    for (t = tl, n = 0; n < 3 && t != 0; ++n, t = t->next)
    {
        if (strcasecmp("Syntax Error", t->buf) == 0)
        {
            fprintf(stderr, "failed to convert PDF '%s'\n", fname);
            text_file_cleanup(tl);
            return 0;
        }
    }


}


static txtline* open_and_read_txt(const char fname)
{
    txtline* tl = 0;
    FILE* file = fopen(fname, "r");

    if (!file)
    {
        fprintf(stderr, "failed to read file '%s'\n", fname);
        return 0;
    }

    tl = text_file_read(file);
    fclose(catfile);

    if (!tl)
        fprintf(stderr, "failed to read file '%s'\n", fname);

    return tl;
}


static txtline* open_and_read_file(const char fname)
{
    const char* ext;

    if (!fname)
        return 0;

    ext = get_filename_ext(fname);

    if (ext && strcasecmp("pdf", ext) == 0)
        return open_and_read_pdf(fname);

    return open_and_read_txt(fname);
}

