

#ifndef __PALLOC__H__
#define __PALLOC__H__

#include "config.h"

#include <stdlib.h>


void *pmalloc(size_t size);
void pfree(void *ptr);
void *pcalloc(size_t nmemb, size_t size);
void *prealloc(void *ptr, size_t size);
size_t pmused(void);


#endif

