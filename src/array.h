

#ifndef __ARRAY__H__
#define __ARRAY__H__


#include "config.h"


typedef struct array_s {
    void        *elts;
    size_t      nelts;
    size_t      size;
    size_t      nalloc;
} *array_t;


#define array_pop_back(array)                       \
    do {                                            \
        if ((array)->nelts > 0) {                   \
            (array)->nelts--;                       \
        }                                           \
    } while(0)


#define array_pop_back_n(array, n)                  \
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


#define array_cast_at(type, a, index)               \
    (*((type*)__array_at(a, index)))


#define array_cast_front(type, a)                   \
    (*((type*)__array_front(a)))
    

#define array_cast_back(type, a)                    \
    (*((type*)__array_back(a)))


#define array_cast_append(type, a, element)         \
    do {                                            \
        (*(type*)array_push_back((a))) = element;   \
    } while (false)


array_t array_create(size_t size);
array_t array_create_n(size_t size, size_t n);
void array_destroy(array_t a);
void *array_push_back(array_t a);
void *array_push_back_n(array_t a, size_t n);
bool array_extend(array_t a, array_t b);


static inline
void *__array_at(array_t a, size_t i)
{
    assert(a->nelts > i);
    return (((unsigned char *)a->elts) + (a->size * i));
}


static inline
void *__array_front(array_t a)
{
    assert(a->nelts > 0);
    return a->elts;
}


static inline
void *__array_back(array_t a)
{
    assert(a->nelts > 0);
    return (((unsigned char *)a->elts) + (a->size * (a->nelts - 1)));
}


#endif
