

#ifndef __ARRAY__H__
#define __ARRAY__H__

#include "config.h"
#include <stdlib.h>


struct array_s {
    void        *elts;
    size_t      nelts;
    size_t      size;
    size_t      nalloc;
};


typedef struct array_s array_t;


array_t *array_create(size_t size);
array_t *array_create_n(size_t size, size_t n);
void array_destroy(array_t *array);
void *array_push(array_t *array);
void *array_push_n(array_t *array, size_t n);


#define array_pop(array)                            \
    do {                                            \
        if ((array)->nelts > 0) {                   \
            (array)->nelts--;                       \
        }                                           \
    } while(0)


#define array_pop_n(array, n)                       \
    do {                                            \
        if ((n) > (array)->nelts) {                 \
            (array)->nelts = 0;                     \
        } else {                                    \
            (array)->nelts -= n;                    \
        }                                           \
    } while(0)


#define array_prototype(array, type)                \
    ((type*)((array)->elts))


#define array_size(array)                           \
    ((array)->nelts)


#define array_capacity(array)                       \
    ((array)->nalloc - (array)->nelts)


#define array_clear(array)                          \
    ((array)->nelts = 0)


#define array_empty(array)                          \
    ((array)->nelts == 0)


#define array_foreach(array, base, index)           \
    for ((base) = (array)->elts, (index) = 0;       \
         (size_t)(index) < (array)->nelts;          \
         (index)++)


#endif
