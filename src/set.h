

#ifndef __SET__H__
#define __SET__H__


#include "config.h"
#include "cstring.h"


typedef struct dict_s* set_t;


set_t set_create(void);
void set_destroy(set_t set);
bool set_add(set_t set, cstring_t cs);
bool set_has(set_t set, cstring_t cs);
set_t set_union(set_t a, set_t b);
set_t set_intersection(set_t a, set_t b);


#endif
