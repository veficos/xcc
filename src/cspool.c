

#include "config.h"
#include "dict.h"
#include "cspool.h"
#include "cstring.h"


static inline
uint64_t __hash_fn__(const void *key)
{
    return dict_gen_hash_function((unsigned char*)key, strlen((char*)key));
}


static inline
int __compare_fn__(void *privdata, const void *key1, const void *key2)
{
    DICT_NOTUSED(privdata);
    return cstring_compare((cstring_t)key2, (char*)key1) == 0;
}


static inline
void __free_fn__(void *privdata, void *val) {
    DICT_NOTUSED(privdata);
    cstring_free((cstring_t)val);
}


dict_type_t __cspool_dict_type__ = {
    __hash_fn__,
    NULL,
    NULL,
    __compare_fn__,
    __free_fn__,
    NULL
};


cspool_t cspool_create(void)
{
    cspool_t pool = (cspool_t)pmalloc(sizeof(struct cspool_s));
    pool->d = dict_create(&__cspool_dict_type__, NULL);
    return pool;
}


void cspool_destroy(cspool_t pool)
{
    dict_destroy(pool->d);
    pfree(pool);
}


cstring_t cspool_push(cspool_t pool, const char *s)
{
    void *ret;
    dict_entry_t *entry;

    entry = dict_add_or_find(pool->d, (void*)s);
    if (!entry) {
        return NULL;
    }

    ret = dict_get_key(entry);
    if (ret == s) {
        ret = cstring_new(s);
        dict_set_key(pool->d, entry, ret);
    }

    return ret;
}


cstring_t cspool_push_cs(cspool_t pool, cstring_t cs)
{
    cstring_t ret;
    dict_entry_t *entry;

    entry = dict_add_or_find(pool->d, cs);
    if (!entry) {
        return NULL;
    }

    ret = dict_get_key(entry);
    if (ret != cs) {
        cstring_free(cs);
    }

    return ret;
}


void cspool_pop(cspool_t pool, const char *key)
{
    dict_delete(pool->d, key);
}
