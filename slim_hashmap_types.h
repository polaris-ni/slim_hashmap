//
// Created by NiLY on 2023/1/28.
//

#ifndef SLIM_HASHMAP_TYPES_H
#define SLIM_HASHMAP_TYPES_H

#ifdef _cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>

#define RET_SUCCESS 0
#define RET_FAILED (-1)

struct entry;
typedef struct entry Entry;

struct entry {
    void *key;
    void *value;
    Entry *next;
};

struct hash_map;
typedef struct hash_map SlimHashMap;

typedef uint32_t (*hash_func)(const SlimHashMap *map, const void *key);

typedef bool(*cmp_func)(const void *key1, const void *key2);

typedef void *(*dup_func)(const void *ptr);

typedef void (*free_func)(void *ptr);

typedef struct {
    dup_func dup;
    free_func free;
} DupFreeFuncPair;

#ifdef _cplusplus
}
#endif

#endif //SLIM_HASHMAP_TYPES_H
