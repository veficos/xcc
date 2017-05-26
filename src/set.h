

#ifndef __SET__H__
#define __SET__H__


#include "config.h"
#include "cstring.h"


#define set_s dict_s

typedef struct set_s set_t;


set_t* set_create(void);
void set_destroy(set_t *set);
bool set_add(set_t *set, cstring_t cs);
bool set_del(set_t *set, cstring_t cs);
bool set_has(set_t *set, cstring_t cs);
bool set_is_empty(set_t *set);
void set_concat_union(set_t *a, set_t *b);
void set_concat_intersection(set_t *a, set_t *b);
set_t* set_union(set_t *a, set_t *b);
set_t* set_intersection(set_t *a, set_t *b);
set_t* set_dup(set_t *set);
void set_clear(set_t *set);


#endif
