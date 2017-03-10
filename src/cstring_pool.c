

#include "config.h"
#include "cstring.h"
#include "cstring_pool.h"
#include "dict.h"


static inline
uint64_t __hash_fn__(const void *key) {
    return dict_gen_hash_function((unsigned char*)key, cstring_length((char*)key));
}


static inline
int __compare_fn__(void *privdata, const void *key1, const void *key2) {
    int l1, l2;
    DICT_NOTUSED(privdata);

    l1 = cstring_length((cstring_t)key1);
    l2 = cstring_length((cstring_t)key2);

    if (l1 != l2) {
        return 0;
    }

    return memcmp(key1, key2, l1) == 0;
}


static inline
void __free_fn__(void *privdata, void *val) {
    DICT_NOTUSED(privdata);
    cstring_destroy(val);
}


dict_type_t __cstring_pool_dict_type__ = {
    __hash_fn__,
    NULL,
    NULL,
    __compare_fn__,
    __free_fn__,
    NULL
};


cstring_pool_t cstring_pool_create()
{
    dict_t *dict = dict_create(&__cstring_pool_dict_type__, NULL);
    return (cstring_pool_t) dict;
}


void cstring_pool_destroy(cstring_pool_t pool)
{
    dict_destroy((dict_t *) pool);
}


cstring_t cstring_pool_push_n(cstring_pool_t pool, const void *data, size_t n)
{
    cstring_t cs;

    if ((cs = cstring_create_n(data, n)) == NULL) {
        return NULL;
    }

    return cstring_pool_push(pool, cs);
}


cstring_t cstring_pool_push(cstring_pool_t pool, cstring_t cs)
{
    cstring_t ret;
    dict_entry_t *entry;

    entry = dict_add_or_find((dict_t *)pool, cs);
    if (!entry) {
        return NULL;
    }

    ret = dict_get_key(entry);
    if (ret != cs) {
        cstring_destroy(ret);
    }

    return ret;
}
