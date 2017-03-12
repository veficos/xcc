

#ifndef __PALLOC__H__
#define __PALLOC__H__

#include "config.h"

#include <stdlib.h>


#define pmalloc(size)           malloc(size)
#define pfree(ptr)              free(ptr)
#define pcalloc(nmemb, size)    calloc((nmemb), (size))
#define prealloc(ptr, size)     realloc(ptr, size)
#define pmused()                (0)


#endif

