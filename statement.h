#ifndef STATEMENT_H
#define STATEMENT_H


#include <stdio.h>


#include "trans_data.h"


int     st_init(void);

tran*   st_process(FILE*);

void    st_text_dump(tran*);


#endif
