

#ifndef __PALLOC__H__
#define __PALLOC__H__


#include "config.h"
#include <malloc.h>
#include <stdlib.h>


#if defined(DEBUG) || defined(USE_MALLOC)
#   define pmalloc(size)					malloc(size)
#   define pfree(ptr)						free(ptr)
#   define pcalloc(nmemb, size)				calloc(nmemb, size)
#   define prealloc(ptr, size)				realloc(ptr, size)
#   define pmused()							(0)
#else
void    *pmalloc(size_t size);
void    pfree(void *ptr);
void    *pcalloc(size_t nmemb, size_t size);
void    *prealloc(void *ptr, size_t size);
size_t  pmused(void);
#endif


#endif

