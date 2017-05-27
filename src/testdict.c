

#include "config.h"
#include "unittest.h"
#include "cstring.h"
#include "dict.h"


static uint64_t hash_callback(const void *key) {
    return dict_gen_hash_function((unsigned char*)key, cstring_length((cstring_t)key));
}


static int compare_callback(void *ud, const void *key1, const void *key2) {
    int l1,l2;

    DICT_NOTUSED(ud);

    l1 = cstring_length((cstring_t)key1);
    l2 = cstring_length((cstring_t)key2);

    if (l1 != l2) return 0;

    return memcmp(key1, key2, l1) == 0;
}


static void free_callback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);
    cstring_free(val);
}


dict_type_t dict_type = {
        hash_callback,
        NULL,
        NULL,
        compare_callback,
        free_callback,
        NULL,
};


static void test_dict(void)
{
    dict_t *dict;
    dict_entry_t *de;
    dict_iterator_t *iter;
    int j;

    dict = dict_create(&dict_type, NULL);

    for (j = 0; j < 1000; j++) {
        cstring_t key = cstring_from_ll(j);
        TEST_COND("dict_add", dict_add(dict, key, (void*)j));
    }

    for (j = 0; j < 1000; j++) {
        cstring_t key = cstring_from_ll(j);
        dict_entry_t *de = dict_find(dict, key);
        TEST_COND("dict_find", de != NULL);
        cstring_free(key);
    }

    iter = dict_get_iterator(dict);
    while (de = dict_next(iter)) {
        TEST_COND("dict_iter", (int)str2ll(dict_get_key(de), 10) == (int)dict_get_val(de));
    }
    dict_release_iterator(iter);

    {
        dict_stat_t stat;
        int i;

        dict_get_stats(dict, &stat);

        printf("Main hash table stats:\n"
               " table size: %ld\n"
               " number of elements: %ld\n"
               " different slots: %ld\n"
               " max chain length: %ld\n"
               " avg chain length (counted): %.02f\n"
               " avg chain length (computed): %.02f\n"
               " Chain length distribution:\n",
            stat.main.table_size,
            stat.main.number_of_elements,
            stat.main.different_slots,
            stat.main.max_chain_length,
            stat.main.counted_avg_chain_length,
            stat.main.computed_avg_chain_length);

        for (i = 0; i < DICT_STATS_VECTLEN-1; i++) {
            if (stat.main.clvector[i] == 0) continue;
            printf("   %s%ld: %ld (%.02f%%)\n",
                   (i == DICT_STATS_VECTLEN-1)?">= ":"",
                   i, stat.main.clvector[i], ((double)stat.main.clvector[i]/(double)stat.main.table_size)*100);
        }

        printf("Rehashing hash table stats:\n"
                       " table size: %ld\n"
                       " number of elements: %ld\n"
                       " different slots: %ld\n"
                       " max chain length: %ld\n"
                       " avg chain length (counted): %.02f\n"
                       " avg chain length (computed): %.02f\n"
                       " Chain length distribution:\n",
               stat.rehashing.table_size,
               stat.rehashing.number_of_elements,
               stat.rehashing.different_slots,
               stat.rehashing.max_chain_length,
               stat.rehashing.counted_avg_chain_length,
               stat.rehashing.computed_avg_chain_length);

        for (i = 0; i < DICT_STATS_VECTLEN-1; i++) {
            if (stat.rehashing.clvector[i] == 0) continue;
            printf("   %s%ld: %ld (%.02f%%)\n",
                   (i == DICT_STATS_VECTLEN-1)?">= ":"",
                   i, stat.rehashing.clvector[i], ((double)stat.rehashing.clvector[i]/(double)stat.rehashing.table_size)*100);
        }
    }

    dict_destroy(dict);
}


int main(void)
{
#ifdef WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    test_dict();

    TEST_REPORT();
    return 0;
}
