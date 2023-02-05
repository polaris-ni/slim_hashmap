//
// Created by NiLY on 2023/1/28.
//
#include "slim_hashmap.h"
#include "slim_hashmap_default_impl.h"

#define MIN_HASHMAP_SIZE 16

struct hash_map {
    uint32_t dataSize;
    uint32_t bucketSize;
    hash_func hash;
    cmp_func cmp;
    Entry *list;
    bool autoAssign;
    DupFreeFuncPair keyFuncPair;
    DupFreeFuncPair valueFuncPair;
};

void hash_map_set_hash_func(SlimHashMap *hashMap, const hash_func *hashFunc) {
    if (hashMap == NULL) {
        return;
    }

    hashMap->hash = hashFunc == NULL ? default_hash_func : *hashFunc;
}

void hash_map_set_key_cmp_func(SlimHashMap *hashMap, const cmp_func *cmpFunc) {
    if (hashMap == NULL) {
        return;
    }

    hashMap->cmp = cmpFunc == NULL ? str_cmp_func : *cmpFunc;
}

void hash_map_set_auto_assign(SlimHashMap *hashMap, bool autoAssign) {
    if (hashMap == NULL) {
        return;
    }

    hashMap->autoAssign = autoAssign;
}

void hash_map_set_key_func(SlimHashMap *hashMap, const DupFreeFuncPair *pair) {
    if (hashMap == NULL) {
        return;
    }

    hashMap->keyFuncPair.dup = pair == NULL ? str_dup_func : *pair->dup;
    hashMap->keyFuncPair.free = pair == NULL ? str_free_func : *pair->free;
}

void hash_map_set_value_func(SlimHashMap *hashMap, const DupFreeFuncPair *pair){
    if (hashMap == NULL) {
        return;
    }

    hashMap->valueFuncPair.dup = pair == NULL ? str_dup_func : *pair->dup;
    hashMap->valueFuncPair.free = pair == NULL ? str_free_func : *pair->free;
}

SlimHashMap *hash_map_create_default() {
    SlimHashMap *hashMap = (SlimHashMap *) malloc(sizeof(SlimHashMap));
    hashMap->dataSize = 0;
    hashMap->bucketSize = MIN_HASHMAP_SIZE;
    hashMap->hash = default_hash_func;
    hashMap->cmp = str_cmp_func;
    hashMap->keyFuncPair.dup = str_dup_func;
    hashMap->keyFuncPair.free = str_free_func;
    hashMap->valueFuncPair.dup = str_dup_func;
    hashMap->valueFuncPair.free = str_free_func;
    hashMap->autoAssign = true;
    hashMap->list = (Entry *) malloc((hashMap->bucketSize) * sizeof(Entry));
    Entry *p = hashMap->list;
    for (uint32_t i = 0; i < hashMap->bucketSize; i++) {
        p[i].key = p[i].value = p[i].next = NULL;
    }
    return hashMap;
}

static void hash_map_free_entry_single(const SlimHashMap *map, Entry **pEntry) {
    if ((pEntry == NULL) || (*pEntry == NULL)) {
        return;
    }
    map->keyFuncPair.free((*pEntry)->key);
    map->valueFuncPair.free((*pEntry)->value);
    free(*pEntry);
    *pEntry = NULL;
}

static void hash_map_clear_entry_single(const SlimHashMap *map, Entry *pEntry) {
    if ((pEntry == NULL) || (pEntry->next != NULL)) {
        return;
    }
    map->keyFuncPair.free(pEntry->key);
    map->valueFuncPair.free(pEntry->value);
    pEntry->key = NULL;
    pEntry->value = NULL;
}

int32_t hash_map_resize(SlimHashMap *hashMap, uint32_t newSize) {

    /* 新map的大小应当大于最小值，否则不做任何处理 */
    if ((hashMap == NULL) || (newSize < MIN_HASHMAP_SIZE)) {
        return RET_SUCCESS;
    }

    /* 最大不应超过dataSize能表示的范围 */
    if (newSize > UINT32_MAX) {
        return RET_FAILED;
    }

    /* 重新申请空间并初始化 */
    Entry *new = (Entry *) malloc(newSize * sizeof(Entry));
    if (new == NULL) {
        return RET_FAILED;
    }
    for (uint32_t i = 0; i < newSize; ++i) {
        new[i].key = NULL;
        new[i].value = NULL;
        new[i].next = NULL;
    }

    Entry *old = hashMap->list;
    uint32_t oldSize = hashMap->bucketSize;
    hashMap->list = new;
    hashMap->bucketSize = newSize;
    for (uint32_t i = 0; i < oldSize; ++i) {
        Entry *cur = &old[i];
        if ((cur->key == NULL) && (cur->value == NULL) && (cur->next == NULL)) {
            /* 三者同时为NULL表明该节点没有存放任何数据，跳过 */
            continue;
        }

        /* 将该节点及冲突节点重新放置 */
        while (cur != NULL) {
            if (!hash_map_put(hashMap, cur->key, cur->value)) {
                return RET_FAILED;
            }
            Entry *oldEntry = cur;
            cur = cur->next;
            /* 清理该节点 */
            hashMap->keyFuncPair.free(oldEntry->key);
            hashMap->keyFuncPair.free(oldEntry->value);
            oldEntry->key = NULL;
            oldEntry->value = NULL;
            oldEntry->next = NULL;
        }
    }

    free(old);
    return RET_SUCCESS;
}

bool hash_map_put_r(SlimHashMap *hashMap, const void *key, const void *value, void **oldValue) {
    if (hashMap->dataSize == INT32_MAX) {
        /* 超出表示范围，无法放入 */
        return false;
    }
    if (hashMap->autoAssign && hashMap->dataSize >= hashMap->bucketSize) {
        // 内存扩充至原来的两倍
        // *注: 扩充时考虑的是当前存储元素数量与存储空间的大小关系，而不是存储空间是否已经存满，
        // 例如: 存储空间为10，存入了10个键值对，但是全部冲突了，所以存储空间空着9个，其余的全部挂在一个上面，
        // 这样检索的时候和遍历查询没有什么区别了，可以简单这样理解，当我存入第11个键值对的时候一定会发生冲突，
        // 这是由哈希函数本身的特性(取模)决定的，冲突就会导致检索变慢，所以这时候扩充存储空间，对原有键值对进行
        // 再次散列，会把冲突的数据再次分散开，加快索引定位速度。
        if (hash_map_resize(hashMap, hashMap->bucketSize * 2) != RET_SUCCESS) {
            return false;
        }
    }

    uint32_t index = hashMap->hash(hashMap, key) % hashMap->bucketSize;
    if (hashMap->list[index].key == NULL) {
        hashMap->dataSize++;
        Entry *entry = &hashMap->list[index];
        entry->key = hashMap->keyFuncPair.dup(key);
        entry->value = hashMap->valueFuncPair.dup(value);
        return (entry->key != NULL) && (entry->value != NULL);
    } else {
        Entry *current = &hashMap->list[index];
        while (current != NULL) {
            if (hashMap->cmp(key, current->key)) {
                /* 对于键值已经存在的直接覆盖，并且视情况返回被替换的值 */
                if (oldValue != NULL) {
                    *oldValue = hashMap->valueFuncPair.dup(current->value);
                }
                current->value = hashMap->valueFuncPair.dup(value);
                return current->value != NULL;
            }
            current = current->next;
        }

        /* 发生冲突则使用链表 */
        Entry *entry = (Entry *) malloc(sizeof(Entry));
        if (entry == NULL) {
            return false;
        }
        entry->key = hashMap->keyFuncPair.dup(key);
        entry->value = hashMap->valueFuncPair.dup(value);
        entry->next = hashMap->list[index].next;
        hashMap->list[index].next = entry;
        hashMap->dataSize++;
        return (entry->key != NULL) && (entry->value != NULL);
    }
}

bool hash_map_put(SlimHashMap *hashMap, const void *key, const void *value) {
    return hash_map_put_r(hashMap, key, value, NULL);
}

void *hash_map_get(const SlimHashMap *hashMap, const void *key) {
    if (hashMap == NULL) {
        return NULL;
    }

    uint32_t index = hashMap->hash(hashMap, key);
    Entry *entry = &hashMap->list[index];
    while ((entry != NULL) && (!hashMap->cmp(entry->key, key))) {
        entry = entry->next;
    }
    return entry == NULL ? NULL : entry->value;
}

bool hash_map_remove(SlimHashMap *hashMap, const void *key) {
    if ((hashMap == NULL) || (hashMap->dataSize == 0)) {
        return false;
    }

    uint32_t index = (hashMap->hash(hashMap, key)) % hashMap->bucketSize;
    Entry *entry = &hashMap->list[index];
    /* 找不到该key */
    if (entry->key == NULL) {
        return false;
    }

    Entry *last = NULL;
    while ((entry != NULL) && (!hashMap->cmp(entry->key, key))) {
        last = entry;
        entry = entry->next;
    }

    /* 找不到该key */
    if (entry == NULL) {
        return false;
    }

    /* 找到需要删除的节点，则节点数减一 */
    hashMap->dataSize--;

    /* 该节点有前置节点，即不为冲突链表第一个节点，将前置节点后继指向当前节点后继并删除当前节点即可 */
    if (last != NULL) {
        last->next = entry->next;
        hash_map_free_entry_single(hashMap, &entry);
        goto END;
    }

    /* 该节点为首个节点 */
    if (entry->next == NULL) {
        /* 没有后续节点，则将该节点释放即可 */
        hash_map_clear_entry_single(hashMap, &hashMap->list[index]);
    } else {
        /* 有后续节点，则释放该节点并将后续节点赋值为首节点并删除后续节点 */
        hashMap->keyFuncPair.free(hashMap->list[index].key);
        hashMap->valueFuncPair.free(hashMap->list[index].value);
        hashMap->list[index].key = entry->next->key;
        hashMap->list[index].value = entry->next->value;
        hashMap->list[index].next = entry->next->next;
        free(entry->next);
    }

    END:
    /* 如果空间占用不足一半，则释放多余内存 */
    if (hashMap->autoAssign && hashMap->dataSize < hashMap->bucketSize / 2) {
        hash_map_resize(hashMap, hashMap->bucketSize / 2);
    }
    return true;
}

bool hash_map_is_exists(const SlimHashMap *hashMap, const void *key) {
    if (hashMap == NULL) {
        return false;
    }

    uint32_t index = (hashMap->hash(hashMap, key)) % hashMap->bucketSize;
    Entry *entry = &hashMap->list[index];
    if (entry->key == NULL) {
        return false;
    }

    while ((entry != NULL) && (!hashMap->cmp(entry->key, key))) {
        entry = entry->next;
    }

    return entry != NULL;
}

void hash_map_clear(SlimHashMap *hashMap) {
    if (hashMap == NULL) {
        return;
    }

    for (int i = 0; i < hashMap->bucketSize; i++) {
        /* 释放冲突值内存 */
        Entry *entry = hashMap->list[i].next;
        Entry *next = NULL;
        while (entry != NULL) {
            next = entry->next;
            hash_map_free_entry_single(hashMap, &entry);
            entry = next;
        }
        hashMap->list[i].next = NULL;
        hash_map_clear_entry_single(hashMap, &hashMap->list[i]);
    }
    hashMap->dataSize = 0;
}

void hash_map_destroy(SlimHashMap **hashMap) {
    if ((hashMap == NULL) || (*hashMap == NULL)) {
        return;
    }

    hash_map_clear(*hashMap);
    free((*hashMap)->list);
    free(*hashMap);
    *hashMap = NULL;
}

uint32_t hash_map_size(const SlimHashMap *hashMap) {
    if (hashMap == NULL) {
        return 0;
    }
    return hashMap->dataSize;
}