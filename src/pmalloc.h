

#ifndef __PALLOC__H__
#define __PALLOC__H__


#include "config.h"


typedef void(*palloc_oom_handler_pt)(const char *fn, int line, void *ud);


#if defined(USE_MALLOC)

#   define pmalloc(size)                        malloc(size)
#   define pcalloc(nmemb, size)                 calloc((nmemb), (size))
#   define prealloc(ptr, size)                  realloc(ptr, size)
#   define pfree(ptr)                           free(ptr)
#   define set_alloc_oom_handler(handler, ud)   

#else

#   define pmalloc(size)                    p_malloc(__FILE__, __LINE__, (size))
#   define pcalloc(nmemb, size)             p_calloc(__FILE__, __LINE__, (nmemb), (size))
#   define prealloc(ptr, size)              p_realloc(__FILE__, __LINE__, (ptr), (size))
#   define pfree(ptr)                       p_free(__FILE__, __LINE__, (ptr))

void* p_malloc(const char *fn, int line, size_t size);
void* p_calloc(const char *fn, int line, size_t nmemb, size_t size);
void* p_realloc(const char *fn, int line, void *ptr, size_t size);
void p_free(const char *fn, int line, void *ptr);
void set_alloc_oom_handler(palloc_oom_handler_pt handler, void *ud);

#endif


#endif
