#include "categories.h"

#include "debug.h"

#include <regex.h>
#include <string.h>
#include <stdlib.h>


typedef struct category_pattern
{
    char*   str;
    regex_t preg;
    struct category_pattern* next;
} catpat;


typedef struct category
{
    char*   str;
    catpat* pat;

    struct category* next;
} cat;


static cat cat_first = { 0, 0, 0};


static catpat* category_pattern_new(const char* str)
{
    catpat* cp = malloc(sizeof(*cp));

    if (!cp)
        return 0;

    cp->str = strdup(str);

    if (regcomp(&cp->preg, cp->str, REG_EXTENDED))
    {
        fprintf(stderr, "bad regex pattern '%s'\n", cp->str);
        free(cp->str);
        free(cp);
        return 0;
    }

    cp->next = 0;
    return cp;
}


static cat* category_new(const char* str)
{
    cat* c = malloc(sizeof(*c));

    if (!c)
        return 0;

    c->str = strdup(str);
    c->pat = 0;
    c->next = 0;

    return c;
}


int categories_init(txtline* tl)
{
    catpat* cp = 0;
    cat*    c = &cat_first;

    int addpatterns = 0;

    while(tl)
    {
        char* p = tl->buf;

        if (addpatterns)
        {
            if (*p == ' ' || *p == '\t')
            {
                /* skip white space */
                while(*p == ' ' || *p == '\t')
                    ++p;

                if (*p != '#' && *p != '\0')
                {   /* if not a comment, then pattern */
                    debug("\tadding pattern '%s'...\n", p);

                    catpat* pat = category_pattern_new(p);

                    if (!cp)
                        c->pat = pat;
                    else
                        cp->next = pat;

                    cp = pat;
                }
                else
                    addpatterns = 0;
            }
            else if (*p != '#')
            {
                /* if not space nor comment, then new cat */
                dbg(" ");
                addpatterns = 0;
            }
        }

        if (!addpatterns && *p != '\0')
        {   /* new category */
            cat* oc = c;
            debug("new category '%s'...\n", p);

            if (!(c = category_new(p)))
                fprintf(stderr, "failed to create new category '%s'\n", p);
            else
            {
                addpatterns = 1;
                oc->next = c;
                cp = 0;
            }
        }

        tl = tl->next;
    }

    return 0;
}


void categories_cleanup(void)
{
    cat* c = cat_first.next;

    while(c)
    {
        cat* oc = c;
        catpat* cp = c->pat;

        while(cp)
        {
            catpat* ocp = cp;
            cp = cp->next;
            regfree(&ocp->preg);
            free(ocp->str);
            free(ocp);
        }

        c = c->next;
        free(oc->str);
        free(oc);
    }
}


const char* category_get(const char* descr)
{
    cat* c = cat_first.next;

    while(c)
    {
        catpat* cp = c->pat;

        while(cp)
        {
            if (regexec(&cp->preg, descr, 0, NULL, 0) == 0)
                return c->str;

            cp = cp->next;
        }

        c = c->next;
    }

    return 0;
}

