

#include "cstring.h"
#include "pmalloc.h"


#ifndef CSTRING_MAX_PREALLOC
#define CSTRING_MAX_PREALLOC    (1024 * 1024)
#endif 


static inline cstring_t __cstring_make_space__(cstring_t cs, size_t size);


cstring_t cstring_create_n(const void *data, size_t size)
{
    cstring_hdr_t *hdr;

    if (data) {
        hdr = (cstring_hdr_t *) pmalloc(sizeof(cstring_hdr_t) + size);
    } else {
        hdr = (cstring_hdr_t *) pmalloc(sizeof(cstring_hdr_t) + size);
    }

    if (!hdr) {
        return NULL;
    }

    if (data && size) {
        memcpy(hdr->buffer, data, size);
    }

    hdr->length = size;
    hdr->unused = 0;
    hdr->buffer[size] = '\0';

    return (cstring_t) hdr->buffer;
}


void cstring_destroy(cstring_t cs)
{
    if (cs) {
        pfree(cs);
    }
}


cstring_t cstring_cat_n(cstring_t cs, const void *data, size_t size)
{
    cstring_hdr_t *hdr;
    
    cs = __cstring_make_space__(cs, size);
    if (!cs) {
        return NULL;
    }

    hdr = cstring_of(cs);

    memcpy(&hdr->buffer[hdr->length], data, size);

    hdr->length += size;
    hdr->unused -= size;
    hdr->buffer[hdr->length] = '\0';
	return cs;
}


cstring_t cstring_cpy_n(cstring_t cs, const void *data, size_t size)
{
    cstring_hdr_t *hdr = cstring_of(cs);
    size_t total = hdr->unused + hdr->length;

    if (total < size) {
        cs = __cstring_make_space__(cs, size);
        if (!cs) {
            return NULL;
        }

        hdr = cstring_of(cs);
        total = hdr->unused + hdr->length;
    }

    memcpy(cs, data, size);

    cs[size] = '\0';
    hdr->length = size;
    hdr->unused = total - size;

    return cs;
}


static inline
cstring_t __cstring_make_space__(cstring_t cs, size_t size)
{
    cstring_hdr_t *hdr, *newhdr;
    size_t newsize;

    hdr = cstring_of(cs);
    if (hdr->unused > size) {
        return cs;
    }

    newsize = hdr->length + size;
    if (newsize <= CSTRING_MAX_PREALLOC) {
        newsize *= 2;
    } else {
        newsize += CSTRING_MAX_PREALLOC;
    }

    newhdr = (cstring_hdr_t *) prealloc(hdr, sizeof(cstring_hdr_t) + newsize);
    if (!newhdr) {
        return NULL;
    }

    newhdr->unused = newsize - newhdr->length;
    return (cstring_t) newhdr->buffer;
}

