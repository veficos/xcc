

#ifndef __CSTRING__POOL__H__
#define __CSTRING__POOL__H__


#include "config.h"
#include "cstring.h"
#include "dict.h"

#include <stdint.h>


#ifndef DISABLE_CSTRING_POOL
#define DISABLE_CSTRING_POOL
#endif

typedef struct cstring_pool_s {
    dict_t* d;
} *cstring_pool_t;


cstring_pool_t cstring_pool_create();
void cstring_pool_destroy(cstring_pool_t pool);
cstring_t cstring_pool_push_n(cstring_pool_t pool, const void *data, size_t n);
cstring_t cstring_pool_push(cstring_pool_t pool, cstring_t cs);

#endif
