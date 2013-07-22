#ifndef CATEGORIES_H
#define CATEGORIES_H


#include "misc.h"


int     categories_init(txtline*);
void    categories_cleanup(void);


const char* category_get(const char* descr);


#endif
