

#include "pmalloc.h"
#include "diag.h"

#include <malloc.h>


void *pmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (!ptr) {
        diag_errorf("out of memory");
    }

    return ptr;
}


void pfree(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}


void *pcalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);

    if (!ptr) {
        diag_errorf("out of memory");
    }

    return ptr;
}


void *prealloc(void *ptr, size_t size)
{
    void *reptr = realloc(ptr, size);

    if (!reptr) {
        diag_errorf("out of memory");
    }

    return reptr;
}


size_t pmused(void)
{
    return 0;
}

