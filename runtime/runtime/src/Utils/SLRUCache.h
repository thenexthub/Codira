/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef CODIRARUNTIME_SLRUCACHE_H
#define CODIRARUNTIME_SLRUCACHE_H

#include <mutex>
#include <vector>
#include <unordered_map>

#include "Base/CString.h"

namespace MapleRuntime {
/*
 * An LRU Cache maintains a list of data items, with the most recently accessed items at the front and the least
 * recently accessed items at the back. When an item is accessed or added, it’s moved to the front of the list. If the
 * cache reaches its capacity, the item at the back, being the least recently used, is removed.
 */
template<typename K, typename V>
class LRUCache {
public:
    struct DLinkedNode {
        K key;
        V value;
        DLinkedNode* prev;
        DLinkedNode* next;
        DLinkedNode() : key(K()), value(V()), prev(nullptr), next(nullptr) {}
        DLinkedNode(K _key, V _value) : key(_key), value(_value), prev(nullptr), next(nullptr) {}
    };

    explicit LRUCache(int _capacity) : capacity(_capacity)
    {
        head = new DLinkedNode();
        tail = new DLinkedNode();
        head->next = tail;
        tail->prev = head;
    }

    ~LRUCache()
    {
        for (auto pair : cache) {
            delete pair.second;
        }
        cache.clear();
        delete head;
        delete tail;
    }

    V Get(K key)
    {
        if (!cache.count(key)) {
            return V();
        }
        // if key is in cache, move its value to the head.
        DLinkedNode* node = cache[key];
        MoveToHead(node);
        return node->value;
    }

    std::pair<K, V> Put(K key, const V& value)
    {
        std::pair<K, V> nodeEliminated = std::pair<K, V>();
        if (!cache.count(key)) {
            // if key is not in cache, create a new node and place it to the head.
            auto* node = new DLinkedNode(key, value);
            cache[key] = node;
            AddToHead(node);
            ++size;
            if (size > capacity) {
                // if the cache is full, remove the tail node.
                DLinkedNode* removed = RemoveTail();
                nodeEliminated = std::pair<K, V>(removed->key, removed->value);
                cache.erase(removed->key);
                // prevent memory leak
                delete removed;
                --size;
            }
        } else {
            // if key is in the cache, modify its value and then move it to the head.
            DLinkedNode* node = cache[key];
            node->value = value;
            MoveToHead(node);
        }
        return nodeEliminated;
    }

    void AddToHead(DLinkedNode* node)
    {
        node->prev = head;
        node->next = head->next;
        head->next->prev = node;
        head->next = node;
    }

    void RemoveNode(DLinkedNode* node)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void MoveToHead(DLinkedNode* node)
    {
        RemoveNode(node);
        AddToHead(node);
    }

    DLinkedNode* RemoveTail()
    {
        DLinkedNode* node = tail->prev;
        RemoveNode(node);
        return node;
    }

    DLinkedNode* RemoveHead()
    {
        DLinkedNode* node = head->next;
        RemoveNode(node);
        return node;
    }

private:
    std::unordered_map<K, DLinkedNode*> cache;
    DLinkedNode* head;
    DLinkedNode* tail;
    int size = 0;
    int capacity;
};

/*
 * An SLRU (Segmented LRU) cache is divided into two segments:
 * i.  a probational segment which is an LRU cache that keeps items that have been accessed once.
 * ii. a protected segment which is an LRU cache that keeps items that have been accessed more than once.
 *
 * An SLRU cache item has the following lifecycle:
 * i.   New item is inserted to probational segment. This item becomes the most recently used item in the probational
 * segment.
 * ii.  If an item in the probational segment is accessed (with get or set), the item is migrated to the protected
 * segment. This item becomes the most recently used item of the protected segment.
 * iii. If an item in the protected segment is accessed, it becomes the most recently used item of the protected
 * segment.
 */
class SLRU {
public:
    SLRU(int protectedSize, int probationalSize) : protectedLRU(protectedSize), probationalLRU(probationalSize) {}

    ~SLRU() = default;

    void Put(uint32_t key, const std::vector<CString>& value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<CString> valueProtected = protectedLRU.Get(key);
        if (!valueProtected.empty()) {
            return;
        }
        std::vector<CString> valueProbational = probationalLRU.Get(key);
        if (!valueProbational.empty()) {
            MigrateFromProbationalToProtected(key, value);
            return;
        }

        probationalLRU.Put(key, value);
    }

    std::vector<CString> Get(uint32_t key)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<CString> valueProtected = protectedLRU.Get(key);
        if (!valueProtected.empty()) {
            return valueProtected;
        }

        std::vector<CString> valueProbational = probationalLRU.Get(key);
        if (!valueProbational.empty()) {
            MigrateFromProbationalToProtected(key, valueProbational);
        }
        return valueProbational;
    }

    void MigrateFromProbationalToProtected(uint32_t key, const std::vector<CString>& value)
    {
        std::pair<uint32_t, std::vector<CString>> valueEliminated = protectedLRU.Put(key, value);
        probationalLRU.RemoveHead();
        if (!valueEliminated.second.empty()) {
            probationalLRU.Put(valueEliminated.first, valueEliminated.second);
        }
    }

private:
    LRUCache<uint32_t, std::vector<CString>> protectedLRU;
    LRUCache<uint32_t, std::vector<CString>> probationalLRU;
    std::mutex mtx;
};
} // namespace MapleRuntime

#endif // CODIRARUNTIME_SLRUCACHE_H
