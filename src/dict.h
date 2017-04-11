/* Hash Tables Implementation.
*
* This file implements in-memory hash tables with insert/del/replace/find/
* get-random-element operations. Hash tables will auto-resize if needed
* tables of power of two in size are used, collisions are handled by
* chaining. See the source code for more information... :)
*
* Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __DICT__H__
#define __DICT__H__


#include "config.h"
#include <stdint.h>


#define DICT_OK 0
#define DICT_ERR 1


typedef struct dict_entry_s {
    void *key;
    union {
        void    *val;
        uint64_t u64;
        int64_t  s64;
        double   d;
    } v;
    struct dict_entry_s *next;
} dict_entry_t;


typedef struct dict_type_s {
    uint64_t(*hash_function)(const void *key);
    void *(*key_dup)(void *privdata, const void *key);
    void *(*val_dup)(void *privdata, const void *obj);
    int(*key_compare)(void *privdata, const void *key1, const void *key2);
    void(*key_destructor)(void *privdata, void *key);
    void(*val_destructor)(void *privdata, void *obj);
} dict_type_t;


/* This is our hash table structure. Every dictionary has two of this as we
* implement incremental rehashing, for the old to the new table. */
typedef struct dict_hash_table_s {
    dict_entry_t **table;
    unsigned long  size;
    unsigned long  sizemask;
    unsigned long  used;
} dict_hash_table_t;


typedef struct dict_s {
    dict_type_t      *type;
    void             *privdata;
    dict_hash_table_t ht[2];
    long              rehashidx;
    bool              dict_can_resize;
    unsigned long     iterators;
} *dict_t;


/* If safe is set to 1 this is a safe iterator, that means, you can call
* dictAdd, dictFind, and other functions against the dictionary even while
* iterating. Otherwise it is a non safe iterator, and only dictNext()
* should be called while iterating. */
typedef struct dict_iterator_s {
    dict_t d;
    long index;
    int table, safe;
    dict_entry_t *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
} *dict_iterator_t;


typedef void (*dict_scan_function_pt)(void *privdata, const dict_entry_t *de);
typedef void (*dict_scan_bucket_function_pt)(void *privdata, dict_entry_t **bucketref);


#define DICT_HASH_TABLE_INITIAL_SIZE        4
#define DICT_STATS_VECTLEN                  50
#define DICT_NOTUSED(D)                     ((void)D)


#define dict_free_val(d, entry)                                         \
    if ((d)->type->val_destructor)                                      \
        (d)->type->val_destructor((d)->privdata, (entry)->v.val)


#define dict_set_val(d, entry, new_val)                                 \
    do {                                                                \
        if ((d)->type->val_dup)                                         \
            (entry)->v.val = (d)->type->val_dup((d)->privdata, new_val);\
        else                                                            \
            (entry)->v.val = (new_val);                                 \
    } while(0)


#define dict_set_signed_integer_val(entry, new_val)                     \
    do { (entry)->v.s64 = new_val; } while(0)


#define dict_set_unsigned_integer_val(entry, new_val)                   \
    do { (entry)->v.u64 = new_val; } while(0)


#define dict_set_double_val(entry, new_val)                             \
    do { (entry)->v.d = new_val; } while(0)


#define dict_free_key(d, entry)                                         \
    if ((d)->type->key_destructor)                                      \
        (d)->type->key_destructor((d)->privdata, (entry)->key)


#define dict_set_key(d, entry, new_key)                                 \
    do {                                                                \
        if ((d)->type->key_dup)                                         \
            (entry)->key = (d)->type->key_dup((d)->privdata, new_key);  \
        else                                                            \
            (entry)->key = (new_key);                                   \
    } while(0)


#define dict_compare_keys(d, key1, key2)                                \
    (((d)->type->key_compare) ?                                         \
        (d)->type->key_compare((d)->privdata, key1, key2) :             \
        (key1) == (key2))


#define dict_hash_key(d, key)               (d)->type->hash_function(key)
#define dict_get_key(he)                    ((he)->key)
#define dict_get_val(he)                    ((he)->v.val)
#define dict_get_signed_integer_val(he)     ((he)->v.s64)
#define dict_get_unsigned_integer_val(he)   ((he)->v.u64)
#define dict_get_double_val(he)             ((he)->v.d)
#define dict_slots(d)                       ((d)->ht[0].size+(d)->ht[1].size)
#define dict_size(d)                        ((d)->ht[0].used+(d)->ht[1].used)
#define dict_is_rehashing(d)                ((d)->rehashidx != -1)


dict_t dict_create(dict_type_t *type, void *privDataPtr);
void dict_destroy(dict_t d);
int dict_expand(dict_t d, unsigned long size);
int dict_add(dict_t d, void *key, void *val);
dict_entry_t *dict_add_raw(dict_t d, void *key, dict_entry_t **existing);
dict_entry_t *dict_add_or_find(dict_t d, void *key);
int dict_replace(dict_t d, void *key, void *val);
int dict_delete(dict_t d, const void *key);
dict_entry_t *dict_unlink(dict_t ht, const void *key);
void dict_free_unlinked_entry(dict_t d, dict_entry_t *he);
dict_entry_t *dict_find(dict_t d, const void *key);
void *dict_fetch_value(dict_t d, const void *key);
int dict_resize(dict_t d);
dict_iterator_t dict_get_iterator(dict_t d);
dict_iterator_t dict_get_safe_iterator(dict_t d);
dict_entry_t *dict_next(dict_iterator_t iter);
void dict_release_iterator(dict_iterator_t iter);
void dict_get_stats(char *buf, size_t bufsize, dict_t d);

uint64_t dict_gen_hash_function(const void *key, int len);
uint64_t dict_gen_case_hash_function(const unsigned char *buf, int len);
void dict_empty(dict_t d, void(callback)(void*));
void dict_enable_resize(dict_t d);
void dict_disable_resize(dict_t d);
int dict_rehash(dict_t d, int n);
void dict_set_hash_function_seed(uint8_t *seed);
uint8_t *dict_get_hash_function_seed(void);
unsigned long dict_scan(dict_t d, unsigned long v, dict_scan_function_pt fn, dict_scan_bucket_function_pt bucketfn, void *privdata);
unsigned int dict_get_hash(dict_t d, const void *key);
dict_entry_t **dict_find_entry_ref_by_ptr_and_hash(dict_t d, const void *oldptr, unsigned int hash);


#endif
