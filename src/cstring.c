

#include "config.h"
#include "cstring.h"


#ifndef CSTRING_MAX_PREALLOC
#define CSTRING_MAX_PREALLOC    (1024 * 1024)
#endif 


#ifndef MAX_LONGLONG_STRING_SIZE
#define MAX_LONGLONG_STRING_SIZE (64)
#endif


#ifndef MAX_LOCAL_STATIC_BUFFER_SIZE 
#define MAX_LOCAL_STATIC_BUFFER_SIZE (1024)
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

        hdr->length = size;
        hdr->unused = 0;
        hdr->buffer[size] = '\0';

    } else {
        hdr->length = 0;
        hdr->unused = size;
        hdr->buffer[0] = '\0';
    }

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
        if ((cs = __cstring_make_space__(cs, size)) == NULL) {
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


cstring_t cstring_cat_vpf(cstring_t cs, const char *fmt, va_list ap)
{
    va_list cpy;
    char staticbuf[MAX_LOCAL_STATIC_BUFFER_SIZE];
    char *buf;
    char *t;
    size_t buflen = strlen(fmt) * 2;


    if (buflen > sizeof(staticbuf)) {
        buf = pmalloc(buflen);
        if (!buf) {
            return NULL;
        }
    } else {
        buf = staticbuf;
        buflen = sizeof(staticbuf);
    }

    while (1) {
        buf[buflen-2] = '\0';

        va_copy(cpy, ap);

        vsnprintf(buf, buflen, fmt, cpy);

        va_end(cpy);

        if (buf[buflen - 2] != '\0') {
            if (buf != staticbuf) {
                pfree(buf);
            }

            buflen *= 2;
            if ((buf = pmalloc(buflen)) == NULL) {
                return NULL;
            }
            continue;
        }

        break;
    }

    t = cstring_cat_n(cs, buf, strlen(buf));
    if (buf != staticbuf) {
        pfree(buf);
    }

    return t;
}


cstring_t cstring_cat_pf(cstring_t cs, const char *fmt, ...)
{
    va_list ap;
    char *t;

    va_start(ap, fmt);

    t = cstring_cat_vpf(cs, fmt, ap);

    va_end(ap);

    return t;
}


cstring_t cstring_trim(cstring_t cs, const char *cset)
{
    cstring_hdr_t *hdr;
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = cs;
    ep = end = cs + cstring_length(cs) - 1;

    while (sp <= end && strchr(cset, *sp)) sp++;
    while (ep > sp && strchr(cset, *ep)) ep--;

    len = (sp > ep) ? 0 : ((ep - sp) + 1);

    hdr = cstring_of(cs);
    if (hdr->buffer != sp) {
        memmove(cs, sp, len);
    }
    
    hdr->buffer[len] = '\0';
    hdr->unused = hdr->unused + (hdr->length - len);
    hdr->length = len;

    return cs;
}


cstring_t cstring_trim_all(cstring_t cs, const char *cset)
{
    char *cp, *sp, *ep;

    sp = cp = cs;
    ep = cs + cstring_length(cs) - 1;

    while (sp <= ep) {
        if (!strchr(cset, *sp)) {
            *cp++ = *sp++;
        } else {
            sp++;
        }
    }

    *cp = '\0';

    cstring_update_length(cs);

    return cs;
}


int cstring_cmp(const cstring_t cs, const char *str)
{
    size_t llen, rlen, minlen;
    int cmp;

    llen = cstring_length(cs);
    rlen = strlen(str);
    minlen = llen < rlen ? llen : rlen;

    cmp = memcmp(cs, str, minlen);

    if (cmp == 0) {
        return llen - rlen;
    }

    return cmp;
}


int cstring_cmp_cs(const cstring_t l, const cstring_t r)
{
    size_t llen, rlen, minlen;
    int cmp;

    llen = cstring_length(l);
    rlen = cstring_length(r);
    minlen = llen < rlen ? llen : rlen;

    cmp = memcmp(l, r, minlen);

    if (cmp == 0) {
        return llen - rlen;
    }

    return cmp;
}


int cstring_cmp_n(const cstring_t cs, const void *data, size_t n)
{
    size_t cslen, minlen;
    int cmp;

    cslen = cstring_length(cs);
    minlen = cslen < n ? cslen : n;

    cmp = memcmp(cs, data, minlen);

    if (cmp == 0) {
        return cslen - n;
    }

    return cmp;
}


void cstring_tolower(cstring_t cs)
{
    size_t i, len = cstring_length(cs);

    for (i = 0; i < len; i++) {
        cs[i] = (char) tolower(cs[i]);
    }
}


void cstring_toupper(cstring_t cs)
{
    size_t i, len = cstring_length(cs);

    for (i = 0; i < len; i++) {
        cs[i] = (char) toupper(cs[i]);
    }
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
        *p++ = (char) ((base > 10 && remainder >= 10) ? radix[remainder-10] : '0' + remainder);
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
