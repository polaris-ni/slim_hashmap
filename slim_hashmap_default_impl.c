//
// Created by NiLY on 2023/1/28.
//
#include <string.h>
#include "slim_hashmap_default_impl.h"
#include "slim_hashmap.h"

#define HASH_FACTOR 31

uint32_t default_hash_func(const SlimHashMap *hashMap, const void *key) {
    char *k = (char *) key;
    uint32_t h = 0;
    while (*k != '\0') {
        h = (h << 4) + *k++;
        uint32_t g = h & 0xF0000000L;
        if (g != 0) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h % hashMap->bucketSize;
}

uint32_t java_string_hash_func(const SlimHashMap *hashMap, const void *key) {
    char *data = (char *) key;
    uint32_t hash = 0;
    if (data == NULL) {
        return hash;
    }

    while (*data != '\0') {
        hash = hash * HASH_FACTOR + *data++;
    }
    return hash % hashMap->bucketSize;
}

bool str_equal_func(const void *key1, const void *key2) {
    return strcmp(key1, key2) == 0;
}

void *str_dup_func(const void *data) {
    size_t len = strlen(data);
    char *new = (char *) malloc(sizeof(char) * len);
    if (new == NULL) {
        return NULL;
    }
    if (strcpy_s(new, len * sizeof(char), data) != RET_SUCCESS) {
        free(new);
        return NULL;
    }

    return new;
}

void str_free_func(void *data) {
    free(data);
}