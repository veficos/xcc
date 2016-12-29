

#include "cstring.h"

#include <stdarg.h>


#ifndef CSTRING_MAX_PREALLOC
#define CSTRING_MAX_PREALLOC    (1024 * 1024)
#endif 


#ifndef MAX_LONGLONG_STRING_SIZE
#define MAX_LONGLONG_STRING_SIZE (64)
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


cstring_t cstring_from_ll(long long value)
{
    char buf[MAX_LONGLONG_STRING_SIZE];
    size_t len = ll2str(buf, value);
    return cstring_create_n(buf, len);
}


cstring_t cstring_from_ull(unsigned long long value, int base)
{
    char buf[MAX_LONGLONG_STRING_SIZE];
    size_t len = ull2str(buf, value, base);
    return cstring_create_n(buf, len);
}


cstring_t cstring_catfmt(cstring_t cs, const char *fmt, ...)
{
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


size_t ll2str(char *s, long long value)
{
    char *p, aux;
    unsigned long long v;
    size_t l;
    
    v = (value < 0) ? -value : value;

    p = s;

    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);

    if (value < 0) {
        *p++ = '-';
    }

    l = p-s;
    *p = '\0';

    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }

    return l;
}


size_t ull2str(char *s, unsigned long long v, int base)
{
    char *p, aux, *radix="ABCDEFGHIJKLMNOPQRSTUV";
    size_t l;
    unsigned long long remainder;

    if (base < 0 || base > 32) {
        return 0;
    }

    p = s;
    do {
        remainder = (v % base);
        *p++ = (base > 10 && remainder >= 10) ? radix[remainder-10] : '0' + remainder;
        v /= base;
    } while (v);

    l = p - s;
    *p = '\0';

    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }

    return l;
}

