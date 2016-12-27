

#include "array.h"
#include "pmalloc.h"


static int __array_resize__(array_t *array, size_t n);


array_t *array_create(size_t size)
{
    array_t *array;

    array = (array_t *) pmalloc(sizeof(array_t));
    if (!array) {
        return NULL;
    }

    array->elts   = NULL;
    array->nelts  = 0;
    array->nalloc = 0;
    array->size   = size;

    return array;
}


array_t *array_create_n(size_t size, size_t n)
{
    array_t *array;

    array = (array_t *) pmalloc(sizeof(array_t));
    if (!array) {
        return NULL;
    }

    array->elts = (void *) pmalloc(size * n);
    if (!array->elts) {
        pfree(array);
        return NULL;
    }

    array->nelts   = 0;
    array->nalloc  = n;
    array->size    = size;
    return array;
}


void array_destroy(array_t *array)
{
    if (array->elts) {
        pfree(array->elts);
    }

    pfree(array);
}


void *array_push(array_t *array)
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


void *array_push_n(array_t *array, size_t n)
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


static int __array_resize__(array_t *array, size_t n)
{
    size_t resize  = array->size * n;

    if (resize) {
        array->elts = prealloc(array->elts, resize);
        if (!array->elts) {
            return 0;
        }
    }
    
    array->nalloc = n;
    return n;
}

