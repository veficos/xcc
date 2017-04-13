

#ifndef __CSPOOL__H__
#define __CSPOOL__H__


#include "config.h"
#include "cstring.h"


typedef struct dict_s *dict_t;

typedef struct cspool_s {
    dict_t d;
} *cspool_t;


cspool_t cspool_create(void);
void cspool_destroy(cspool_t pool);
cstring_t cspool_push(cspool_t pool, const char *s);
cstring_t cspool_push_cs(cspool_t pool, cstring_t cs);
void cspool_pop(cspool_t pool, const char *key);


#endif
