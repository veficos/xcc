

#include "config.h"
#include "diag.h"
#include "pmalloc.h"


static inline
void __default_alloc_oom_handler__(const char *fn, int line, void *ud)
{
    printf("%s:%d, %p", fn, line, ud);
}


enum {
    idx_alloc_oom_handler = 0,
    idx_user_data,
};


void* palloc_oom_handler[2] = {
    (void *)__default_alloc_oom_handler__,
    NULL
};


static void __oom_handler__(const char *fn, int line)
{
    palloc_oom_handler_pt alloc_oom_handler = (palloc_oom_handler_pt) palloc_oom_handler[idx_alloc_oom_handler];
    void *ud = palloc_oom_handler[idx_user_data];

    assert(alloc_oom_handler != NULL);
    alloc_oom_handler(fn, line, palloc_oom_handler[1]);
}


void* p_malloc(const char *fn, int line, size_t size)
{
    void *ptr;
    
    if ((ptr = malloc(size)) == NULL) {
        __oom_handler__(fn, line);
        return NULL;
    }

    return ptr;
}


void* p_calloc(const char *fn, int line, size_t nmemb, size_t size)
{
    void *ptr;

    if ((ptr = calloc(nmemb, size)) == NULL) {
        __oom_handler__(fn, line);
        return NULL;
    }

    return ptr;
}


void* p_realloc(const char *fn, int line, void *ptr, size_t size)
{
    if ((ptr = realloc(ptr, size)) == NULL) {
        __oom_handler__(fn, line);
        return NULL;
    }
    return ptr;
}


void p_free(const char *fn, int line, void *ptr)
{
    assert(ptr != NULL);
    free(ptr);
}


void set_alloc_oom_handler(palloc_oom_handler_pt handler, void *ud)
{
    palloc_oom_handler[idx_alloc_oom_handler] = (void*) handler;
    palloc_oom_handler[idx_user_data] = ud;
}
