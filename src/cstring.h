

#ifndef __CSTRING__H__
#define __CSTRING__H__


#include "config.h"
#include "pmalloc.h"
#include <stdlib.h>
#include <string.h>


typedef struct cstring_hdr_s {
    size_t length;
    size_t unused;
    char buffer[1];
} cstring_hdr_t;


typedef char* cstring_t;


#define cstring_of(cs)                                                  \
    ((cstring_hdr_t *)(((unsigned char*)(cs)) -                         \
            (unsigned char*)(&(((cstring_hdr_t *)0)->buffer))))


#define str2ll(ptr, base)                                               \
    strtoll(ptr, NULL, base)


#define str2ull(ptr, base)                                              \
    ((unsigned long long) strtoll(ptr, NULL, base))


cstring_t cstring_create_n(const void *data, size_t size);
cstring_t cstring_cat_n(cstring_t cs, const void *data, size_t size);
cstring_t cstring_cpy_n(cstring_t cs, const void *data, size_t size);
cstring_t cstring_from_ll(long long value);
cstring_t cstring_from_ull(unsigned long long value, int base);


size_t ull2str(char *s, unsigned long long value, int base);
size_t ll2str(char *s, long long value);


static inline
cstring_t cstring_create(const char *s)
{
    size_t n = (s == NULL) ? 0 : strlen(s);
    return cstring_create_n(s, n);
}


static inline 
size_t cstring_sizeof(const cstring_t cs)
{
    cstring_hdr_t *hdr = cstring_of(cs);
    return sizeof(cstring_hdr_t) + hdr->length + hdr->unused;
}


static inline
void cstring_update_length(cstring_t cs)
{
    cstring_hdr_t *hdr = cstring_of(cs);
    size_t n = strlen(cs);
    hdr->unused += (hdr->length - n);
    hdr->length = n;
}


static inline
void cstring_clear(cstring_t cs)
{
    cstring_hdr_t *hdr = cstring_of(cs);
    hdr->unused += hdr->length;
    hdr->length = 0;
    hdr->buffer[0] = '\0';
}


static inline
size_t cstring_length(const cstring_t cs)
{
    return cstring_of(cs)->length;
}


static inline
size_t cstring_capacity(const cstring_t cs)
{
    return cstring_of(cs)->unused;
}


static inline
void cstring_destroy(cstring_t cs)
{
    if (cs) {
        pfree(cstring_of(cs));
    }
}


#endif

