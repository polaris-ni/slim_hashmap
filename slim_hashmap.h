//
// Created by NiLY on 2023/1/28.
//

#ifndef SLIM_HASHMAP_H
#define SLIM_HASHMAP_H

#ifdef _cplusplus
extern "C" {
#endif

#include "slim_hashmap_types.h"

SlimHashMap *hash_map_create();

int32_t hash_map_resize(SlimHashMap *hashMap, uint32_t newSize);

bool hash_map_put_r(SlimHashMap *hashMap, const void *key, const void *value, void **oldValue);

bool hash_map_put(SlimHashMap *hashMap, const void *key, const void *value);

void *hash_map_get(const SlimHashMap *hashMap, const void *key);

bool hash_map_remove(SlimHashMap *hashMap, const void *key);

bool hash_map_is_exists(const SlimHashMap *hashMap, const void *key);

void hash_map_clear(SlimHashMap *hashMap);

void hash_map_destroy(SlimHashMap **hashMap);

#ifdef _cplusplus
}
#endif

#endif //SLIM_HASHMAP_H
