

#include "config.h"
#include "array.h"
#include "pmalloc.h"


static inline int __array_resize__(array_t array, size_t n);


array_t array_create(size_t size)
{
    array_t array;

    if ((array = (array_t) pmalloc(sizeof(struct array_s))) == NULL) {
        return NULL;
    }

    array->elts   = NULL;
    array->nelts  = 0;
    array->nalloc = 0;
    array->size   = size;
    return array;
}


array_t array_create_n(size_t size, size_t n)
{
    array_t array;

    if ((array = (array_t) pmalloc(sizeof(struct array_s))) == NULL) {
        return NULL;
    }

    if ((array->elts = (void *) pmalloc(size * n)) == NULL) {
        pfree(array);
        return NULL;
    }

    array->nelts   = 0;
    array->nalloc  = n;
    array->size    = size;
    return array;
}


void array_destroy(array_t array)
{
    assert(array && array->elts);

    if (array->elts != NULL) {
        pfree(array->elts);
    }

    pfree(array);
}


void *array_push(array_t array)
{
    void *elt;

    if (array->nelts == array->nalloc) {
        size_t n;

        if (array->nalloc == 0) {
            n = 2;

        } else {
            n = array->nalloc * 2;
        }

        if (!__array_resize__(array, n)) {
            return NULL;
        }
    }

    elt = (char *) array->elts + array->size * array->nelts;
    array->nelts++;
    return elt;
}


void *array_push_n(array_t array, size_t n)
{
    void *elt;

    if (array->nelts + n > array->nalloc) {
        size_t nalloc = 2 * (n >= array->nalloc ? n : array->nalloc);

        if (!__array_resize__(array, nalloc)) {
            return NULL;
        }
    }

    elt = (char *)array->elts + array->size * array->nelts;
    array->nelts += n;
    return elt;
}


bool array_append(array_t a, array_t b)
{
    void *chunk;

    if (a->size != b->size) {
        return false;
    }

    chunk = array_push_n(a, b->nelts);

    memcpy(chunk, b->elts, b->size * b->nelts);

    return true;
}


static inline
int __array_resize__(array_t array, size_t n)
{
    size_t resize  = array->size * n;

    if (resize) {
        if ((array->elts = prealloc(array->elts, resize)) == NULL) {
            return 0;
        }
    }
    
    array->nalloc = n;
    return n;
}
