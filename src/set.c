

#include "config.h"
#include "dict.h"
#include "hash.h"
#include "set.h"


static inline
uint64_t __hash_fn__(const void *key) 
{
    return dict_gen_hash_function((unsigned char*)key, cstring_length((char*)key));
}


static inline
cstring_t __key_dup__(void *privdata, const void *key)
{
    return cstring_dup((const cstring_t) key);
}

static inline
int __compare_fn__(void *privdata, const void *key1, const void *key2)
{
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


dict_type_t __set_dict_type__ = {
    __hash_fn__,
    __key_dup__,
    NULL,
    __compare_fn__,
    __free_fn__,
    NULL
};


set_t set_create(void)
{
    dict_t dict = dict_create(&__set_dict_type__, NULL);
    return (set_t) dict;
}


void set_destroy(set_t set)
{
    dict_destroy((dict_t)set);
}


bool set_add(set_t set, cstring_t cs)
{
    return dict_add_or_find((dict_t)set, cs) != NULL;
}


bool set_has(set_t set, cstring_t cs)
{
    return dict_find((dict_t)set, cs) != NULL;
}


bool set_is_empty(set_t set)
{
    return dict_size((dict_t)set) == 0;
}


set_t set_union(set_t a, set_t b)
{
    set_t r;
    dict_iterator_t iter;
    dict_entry_t *entry;

    if ((r = set_create()) == NULL) { 
        goto done;
    }

    if ((iter = dict_get_iterator((dict_t)a)) == NULL) { 
        goto clean_set;
    }

    while (entry = dict_next(iter)) {
        if (dict_add_or_find(r, entry->key) == NULL) {
            goto clean_iter;
        }
    }
    
    dict_release_iterator(iter);

    if ((iter = dict_get_iterator((dict_t)b)) == NULL) {
        goto clean_set;
    }

    while (entry = dict_next(iter)) {
        if (dict_add_or_find(r, entry->key) == NULL) {
            goto clean_iter;
        }
    }

    dict_release_iterator(iter);
    return r;

clean_iter:
    dict_release_iterator(iter);
clean_set:
    set_destroy(r);
done:
    return NULL;
}


set_t set_intersection(set_t a, set_t b)
{
    set_t r;
    dict_iterator_t iter;
    dict_entry_t *entry;
    
    if (dict_size(a) > dict_size(b)) {
        r = a; a = b; b = a;
    }

    if ((r = set_create()) == NULL) {
        goto done;
    }

    if ((iter = dict_get_iterator((dict_t)a)) == NULL) {
        goto clean_set;
    }

    while (entry = dict_next(iter)) {
        if (set_has(b, entry->key)) {
            if (!set_add(r, entry->key)) {
                goto clean_iter;
            }
        }
    }
    
    dict_release_iterator(iter);
    return r;

clean_iter:
    dict_release_iterator(iter);
clean_set:
    set_destroy(r);
done:
    return NULL;
}


set_t set_dup(set_t set)
{
    set_t r;
    dict_iterator_t iter;
    dict_entry_t *entry;

    if ((r = set_create()) == NULL) {
        goto done;
    }

    if ((iter = dict_get_iterator((dict_t)set)) == NULL) {
        goto clean_set;
    }

    while (entry = dict_next(iter)) {
        if (!set_add(r, entry->key)) {
            goto clean_iter;
        }
    }

    dict_release_iterator(iter);
    return r;

clean_iter:
    dict_release_iterator(iter);
clean_set:
    set_destroy(r);
done:
    return NULL;
}


void set_clear(set_t set)
{
    dict_empty((dict_t)set, NULL);
}
