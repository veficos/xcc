

#include "config.h"
#include "dict.h"
#include "hash.h"
#include "map.h"


static inline
uint64_t __hash_fn__(const void *key) 
{
    return dict_gen_hash_function((unsigned char*)key, cstring_length((char*)key));
}


static inline
void* __key_dup__(void *privdata, const void *key)
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
    cstring_free(val);
}


dict_type_t __map_dict_type__ = {
    __hash_fn__,
    __key_dup__,
    NULL,
    __compare_fn__,
    __free_fn__,
    NULL,
};


map_t* map_create(void)
{
    dict_t *dict = dict_create(&__map_dict_type__, NULL);
    return (map_t*) dict;
}


void map_destroy(map_t *map)
{
    dict_destroy((dict_t*)map);
}


bool map_add(map_t *map, cstring_t cs, void *data)
{
    return dict_add((dict_t*)map, cs, data) == true;
}


bool map_has(map_t *map, cstring_t cs)
{
    return dict_find((dict_t*)map, cs) != NULL;
}


bool map_del(map_t *map, cstring_t cs)
{
    return dict_delete((dict_t*)map, cs) == true;
}


void *map_find(map_t *map, cstring_t cs)
{
    dict_entry_t *entry;
    if ((entry = dict_find((dict_t*)map, cs)) == NULL) {
        return NULL;
    }
    return entry->v.val;
}


static inline
void dict_scan_fn(void *privdata, const dict_entry_t *de)
{
    void **ud = privdata;
    map_scan_pt map_scan_fn = (map_scan_pt) ud[0];
    void *data = ud[1];
    map_scan_fn(data, de->key, de->v.val);
}


unsigned long map_scan(map_t *map, map_scan_pt map_fn, void *privdata)
{
    void *p[2] = { map_fn, privdata };
    unsigned long v = 0;
    do {
        v = dict_scan((dict_t*)map, v, dict_scan_fn, NULL, p);
    } while (v);
    return v;
}

