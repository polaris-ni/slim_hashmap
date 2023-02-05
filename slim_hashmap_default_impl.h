//
// Created by NiLY on 2023/1/28.
//

#ifndef SLIM_HASHMAP_DEFAULT_IMPL_H
#define SLIM_HASHMAP_DEFAULT_IMPL_H

#ifdef _cplusplus
extern "C" {
#endif

#include "slim_hashmap_types.h"

// 默认哈希函数
uint32_t default_hash_func(const SlimHashMap *map, const void *key);

uint32_t java_string_hash_func(const SlimHashMap *map, const void *key);

bool str_cmp_func(const void *key1, const void *key2);

void *str_dup_func(const void *data);

void str_free_func(void *data);

void *int8_dup_func(const void *data);

void *int16_dup_func(const void *data);

void *int32_dup_func(const void *data);

void *int64_dup_func(const void *data);

void int_free_func(void *data);

#ifdef _cplusplus
}
#endif

#endif //SLIM_HASHMAP_DEFAULT_IMPL_H
