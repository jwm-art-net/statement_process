#ifndef STATEMENT_H
#define STATEMENT_H


#include <stdio.h>


#include "misc.h"
#include "trans_data.h"


int     st_init(void);

tran*   st_process(txtline*);

void    st_dump(tran*);

void    st_free(tran*);

#endif
