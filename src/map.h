

#ifndef __MAP__H__
#define __MAP__H__


#include "config.h"
#include "cstring.h"


#define map_s dict_s


typedef struct map_s map_t;
typedef void (*map_scan_pt)(void *privdata, const void *key, const void *value);


map_t* map_create(void);
void map_destroy(map_t *map);
bool map_add(map_t *map, cstring_t key, void *val);
bool map_has(map_t *map, cstring_t key);
bool map_del(map_t *map, cstring_t key);
void *map_find(map_t *map, cstring_t key);
unsigned long map_scan(map_t *map, map_scan_pt map_fn, void *privdata);


#endif
