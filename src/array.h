

#ifndef __ARRAY__H__
#define __ARRAY__H__


#include "config.h"


typedef struct array_s {
    void        *elts;
    size_t      nelts;
    size_t      size;
    size_t      nalloc;
} *array_t;


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
    (array ? ((type*)((array)->elts)) : NULL)


#define array_length(array)                         \
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

#define array_at(type, array, index)                \
    (*((type*)(((unsigned char*)(array)->elts) + (((array)->size) * (index)))))


array_t array_create(size_t size);
array_t array_create_n(size_t size, size_t n);
void array_destroy(array_t array);
void *array_push(array_t array);
void *array_push_n(array_t array, size_t n);
bool array_append(array_t a, array_t b);


#endif
