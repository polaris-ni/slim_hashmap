//
// Created by NiLY on 2023/1/28.
//

#ifndef SLIM_HASHMAP_H
#define SLIM_HASHMAP_H

#ifdef _cplusplus
extern "C" {
#endif

#include "slim_hashmap_types.h"

SlimHashMap *hash_map_create_default();

int32_t hash_map_resize(SlimHashMap *hashMap, uint32_t newSize);

bool hash_map_put_r(SlimHashMap *hashMap, const void *key, const void *value, void **oldValue);

bool hash_map_put(SlimHashMap *hashMap, const void *key, const void *value);

void *hash_map_get(const SlimHashMap *hashMap, const void *key);

bool hash_map_remove(SlimHashMap *hashMap, const void *key);

bool hash_map_is_exists(const SlimHashMap *hashMap, const void *key);

void hash_map_clear(SlimHashMap *hashMap);

void hash_map_destroy(SlimHashMap **hashMap);

uint32_t hash_map_size(const SlimHashMap *hashMap);

void hash_map_set_hash_func(SlimHashMap *hashMap, const hash_func *hashFunc);

void hash_map_set_key_cmp_func(SlimHashMap *hashMap, const cmp_func *cmpFunc);

void hash_map_set_auto_assign(SlimHashMap *hashMap, bool autoAssign);

void hash_map_set_key_func(SlimHashMap *hashMap, const DupFreeFuncPair *pair);

void hash_map_set_value_func(SlimHashMap *hashMap, const DupFreeFuncPair *pair);

#ifdef _cplusplus
}
#endif

#endif //SLIM_HASHMAP_H
