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

typedef bool(*is_equal_func)(const void *key1, const void *key2);

typedef void *(*dup_func)(const void *ptr);

typedef void (*free_func)(void *ptr);


struct hash_map {
    uint32_t dataSize;
    uint32_t bucketSize;
    hash_func hash;
    is_equal_func isEqual;
    Entry *list;
    bool autoAssign;
    dup_func keyDup;
    free_func keyFree;
    dup_func valueDup;
    free_func valueFree;
};

#ifdef _cplusplus
}
#endif

#endif //SLIM_HASHMAP_TYPES_H
