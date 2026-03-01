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

#ifndef COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_INL_H
#define COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_INL_H

#include "common_components/log/log.h"
#include "common_interfaces/objects/readonly_handle.h"
#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/objects/string/line_string-inl.h"
#include "common_components/objects/string_table/hashtriemap.h"
#include "common_components/objects/string_table/integer_cache.h"

namespace common {
// Expand to get oldEntry and newEntry, with hash conflicts from 32 bits up to
// hashShift and Generate a subtree of indirect nodes to hold two new entries.
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <bool IsLock>
typename HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Node* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Expand(
    Entry* oldEntry, Entry* newEntry, uint32_t oldHash, uint32_t newHash, uint32_t hashShift, Indirect* parent)
{
    // Check for hash conflicts.
    if (oldHash == newHash) {
        // Store the old entry in the overflow list of the new entry, and then store
        // the new entry.
        newEntry->Overflow().store(oldEntry, std::memory_order_release);
        return newEntry;
    }

    // must add an indirect node. may need to add more than one.
    Indirect* newIndirect = new Indirect();
    Indirect* top = newIndirect;

    while (true) {
#ifndef NDEBUG
        if (hashShift == TrieMapConfig::TOTAL_HASH_BITS) {
            if constexpr (IsLock) {
                GetMutex().Unlock();
            }
            LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while inserting";
            UNREACHABLE_CC();
        }
#endif

        hashShift += TrieMapConfig::N_CHILDREN_LOG2; // hashShift is the level at which the parent
        // is located. Need to go deeper.
        uint32_t oldIdx = (oldHash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
        uint32_t newIdx = (newHash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
        if (oldIdx != newIdx) {
            newIndirect->GetChild(oldIdx).store(oldEntry, std::memory_order_release);
            newIndirect->GetChild(newIdx).store(newEntry, std::memory_order_release);
            break;
        }
        Indirect* nextIndirect = new Indirect();

        newIndirect->GetChild(oldIdx).store(nextIndirect, std::memory_order_release);
        newIndirect = nextIndirect;
    }
    return top;
}

// Load returns the value of the key stored in the mapping, or nullptr value if not
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <bool IsCheck, typename ReadBarrier>
BaseString* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Load(ReadBarrier&& readBarrier, const uint32_t key,
                                                                BaseString* value)
{
    uint32_t hash = key;
    Indirect* current = GetRootAndProcessHash(hash);

    for (uint32_t hashShift = 0; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift +=
         TrieMapConfig::N_CHILDREN_LOG2) {
        size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;

        std::atomic<Node*>* slot = &current->GetChild(index);
        Node* node = slot->load(std::memory_order_acquire);
        if (node == nullptr) {
            return nullptr;
        }
        if (!node->IsEntry()) {
            current = node->AsIndirect();
            continue;
        }
        for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            auto oldValue = currentEntry->Value<SlotBarrier>();
            bool valuesEqual = false;
            if (!IsNull(oldValue) && BaseString::StringsAreEqual(std::forward<ReadBarrier>(readBarrier), oldValue,
                                                                 value)) {
                valuesEqual = true;
            }
            if constexpr (IsCheck) {
                if (oldValue->GetMixHashcode() != value->GetMixHashcode()) {
                    continue;
                }
                if (!valuesEqual) {
                    return oldValue;
                }
            } else if (valuesEqual) {
                return oldValue;
            }
        }
        return nullptr;
    }

    LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
    UNREACHABLE_CC();
}

// LoadOrStore returns the existing value of the key, if it exists.
// Otherwise, call the callback function to create a value,
// store the value, and return the value
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <bool IsLock, typename LoaderCallback, typename EqualsCallback>
BaseString* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::LoadOrStore(ThreadHolder* holder, const uint32_t key,
                                                                       LoaderCallback loaderCallback,
                                                                       EqualsCallback equalsCallback)
{
    HashTrieMapInUseScope mapInUse(this);
    uint32_t hash = key;
    uint32_t hashShift = 0;
    std::atomic<Node*>* slot = nullptr;
    Node* node = nullptr;
    [[maybe_unused]] bool haveInsertPoint = false;
    ReadOnlyHandle<BaseString> str;
    bool isStrCreated = false; // Flag to track whether an object has been created
    Indirect* current = GetRootAndProcessHash(hash);
    while (true) {
        haveInsertPoint = false;
        // find the key or insert the candidate position.
        for (; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift += TrieMapConfig::N_CHILDREN_LOG2) {
            size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
            slot = &current->GetChild(index);
            node = slot->load(std::memory_order_acquire);
            if (node == nullptr) {
                haveInsertPoint = true;
                break;
            }
            // Entry, Search in overflow
            if (!node->IsEntry()) {
                // Indirect, Next level Continue to search
                current = node->AsIndirect();
                continue;
            }
            for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
                 currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
                auto oldValue = currentEntry->Value<SlotBarrier>();
                if (IsNull(oldValue)) {
                    continue;
                }
                if (std::invoke(std::forward<EqualsCallback>(equalsCallback), oldValue)) {
#if ECMASCRIPT_ENABLE_TRACE_STRING_TABLE
                    TraceFindSuccessDepth(hashShift);
#endif
                    return oldValue;
                }
            }
            haveInsertPoint = true;
            break;
        }
#ifndef NDEBUG
        if (!haveInsertPoint) {
            LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
            UNREACHABLE_CC();
        }
#endif
        // invoke the callback to create str
        if (!isStrCreated) {
            str = std::invoke(std::forward<LoaderCallback>(loaderCallback));
            isStrCreated = true; // Tag str created
        }
        // lock and double-check
        if constexpr (IsLock) {
            GetMutex().LockWithThreadState(holder);
        }

        DCHECK_CC(slot != nullptr);
        node = slot->load(std::memory_order_acquire);
        if (node == nullptr || node->IsEntry()) {
            // see is still real, so can continue to insert.
            break;
        }
        if constexpr (IsLock) {
            GetMutex().Unlock();
        }
        current = node->AsIndirect();
        hashShift += TrieMapConfig::N_CHILDREN_LOG2;
    }

#if ECMASCRIPT_ENABLE_TRACE_STRING_TABLE
    TraceFindFail();
#endif
    Entry* oldEntry = nullptr;
    uint32_t oldHash = key;
    if (node != nullptr) {
        oldEntry = node->AsEntry();
        for (Entry* currentEntry = oldEntry; currentEntry;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            auto oldValue = currentEntry->Value<SlotBarrier>();
            if (IsNull(oldValue)) {
                continue;
            }
            if (oldValue->GetMixHashcode() != key) {
                oldHash = oldValue->GetMixHashcode();
                continue;
            }
            if (std::invoke(std::forward<EqualsCallback>(equalsCallback), oldValue)) {
                if constexpr (IsLock) {
                    GetMutex().Unlock();
                }
                return oldValue;
            }
        }
    }

    BaseString* value = *str;
    DCHECK_CC(value != nullptr);
    value->SetIsInternString();
    IntegerCache::InitIntegerCache(value);
    Entry* newEntry = new Entry(value);
    oldEntry = PruneHead(oldEntry);
    if (oldEntry == nullptr) {
        // The simple case: Create a new entry and store it.
        slot->store(newEntry, std::memory_order_release);
    } else {
        // Expand an existing entry to one or more new nodes.
        // Release the node, which will make both oldEntry and newEntry visible
        auto expandedNode = Expand<IsLock>(oldEntry, newEntry,
            oldHash >> TrieMapConfig::ROOT_BIT, hash, hashShift, current);
        slot->store(expandedNode, std::memory_order_release);
    }
    if constexpr (IsLock) {
        GetMutex().Unlock();
    }
    return value;
}

// LoadOrStorForJit need to lock the object before creating the object.
// returns the existing value of the key, if it exists.
// Otherwise, call the callback function to create a value,
// store the value, and return the value
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <typename LoaderCallback, typename EqualsCallback>
BaseString* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::LoadOrStoreForJit(ThreadHolder* holder, const uint32_t key,
                                                                             LoaderCallback loaderCallback,
                                                                             EqualsCallback equalsCallback)
{
    HashTrieMapInUseScope mapInUse(this);
    uint32_t hash = key;
    uint32_t hashShift = 0;
    std::atomic<Node*>* slot = nullptr;
    Node* node = nullptr;
    [[maybe_unused]] bool haveInsertPoint = false;
    BaseString* value = nullptr;
    Indirect* current = GetRootAndProcessHash(hash);
    while (true) {
        haveInsertPoint = false;
        // find the key or insert the candidate position.
        for (; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift += TrieMapConfig::N_CHILDREN_LOG2) {
            size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
            slot = &current->GetChild(index);
            node = slot->load(std::memory_order_acquire);
            if (node == nullptr) {
                haveInsertPoint = true;
                break;
            }
            // Entry, Search in overflow
            if (!node->IsEntry()) {
                // Indirect, Next level Continue to search
                current = node->AsIndirect();
                continue;
            }
            for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
                 currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
                auto oldValue = currentEntry->Value<SlotBarrier>();
                if (IsNull(oldValue)) {
                    continue;
                }
                if (std::invoke(std::forward<EqualsCallback>(equalsCallback), oldValue)) {
                    return oldValue;
                }
            }
            haveInsertPoint = true;
            break;
        }
#ifndef NDEBUG
        if (!haveInsertPoint) {
            LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
            UNREACHABLE_CC();
        }
#endif
        // Jit need to lock the object before creating the object
        GetMutex().LockWithThreadState(holder);
        // invoke the callback to create str
        value = std::invoke(std::forward<LoaderCallback>(loaderCallback));
        DCHECK_CC(slot != nullptr);
        node = slot->load(std::memory_order_acquire);
        if (node == nullptr || node->IsEntry()) {
            // see is still real, so can continue to insert.
            break;
        }

        GetMutex().Unlock();
        current = node->AsIndirect();
        hashShift += TrieMapConfig::N_CHILDREN_LOG2;
    }

    Entry* oldEntry = nullptr;
    uint32_t oldHash = key;
    if (node != nullptr) {
        oldEntry = node->AsEntry();
        for (Entry* currentEntry = oldEntry; currentEntry;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            auto oldValue = currentEntry->Value<SlotBarrier>();
            if (IsNull(oldValue)) {
                continue;
            }
            if (oldValue->GetMixHashcode() != key) {
                oldHash = oldValue->GetMixHashcode();
                continue;
            }
            if (std::invoke(std::forward<EqualsCallback>(equalsCallback), oldValue)) {
                GetMutex().Unlock();
                return oldValue;
            }
        }
    }

    DCHECK_CC(value != nullptr);
    value->SetIsInternString();
    IntegerCache::InitIntegerCache(value);
    Entry* newEntry = new Entry(value);
    oldEntry = PruneHead(oldEntry);
    if (oldEntry == nullptr) {
        // The simple case: Create a new entry and store it.
        slot->store(newEntry, std::memory_order_release);
    } else {
        // Expand an existing entry to one or more new nodes.
        // Release the node, which will make both oldEntry and newEntry visible
        auto expandedNode = Expand<true>(oldEntry, newEntry,
            oldHash >> TrieMapConfig::ROOT_BIT, hash, hashShift, current);
        slot->store(expandedNode, std::memory_order_release);
    }
    GetMutex().Unlock();
    return value;
}

// Based on the loadResult, try the store first
// StoreOrLoad returns the existing value of the key, store the value, and return the value
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <typename LoaderCallback, typename EqualsCallback>
BaseString* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::StoreOrLoad(ThreadHolder* holder, const uint32_t key,
                                                                       HashTrieMapLoadResult loadResult,
                                                                       LoaderCallback loaderCallback,
                                                                       EqualsCallback equalsCallback)
{
    HashTrieMapInUseScope mapInUse(this);
    uint32_t hash = key;
    ProcessHash(hash);
    uint32_t hashShift = loadResult.hashShift;
    std::atomic<Node*>* slot = loadResult.slot;
    Node* node = nullptr;
    [[maybe_unused]] bool haveInsertPoint = true;
    Indirect* current = loadResult.current;
    ReadOnlyHandle<BaseString> str = std::invoke(std::forward<LoaderCallback>(loaderCallback));
    // lock and double-check
    GetMutex().LockWithThreadState(holder);
    node = slot->load(std::memory_order_acquire);
    if (node != nullptr && !node->IsEntry()) {
        GetMutex().Unlock();
        current = node->AsIndirect();
        hashShift += TrieMapConfig::N_CHILDREN_LOG2;
        while (true) {
            haveInsertPoint = false;
            // find the key or insert the candidate position.
            for (; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift += TrieMapConfig::N_CHILDREN_LOG2) {
                size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
                slot = &current->GetChild(index);
                node = slot->load(std::memory_order_acquire);
                if (node == nullptr) {
                    haveInsertPoint = true;
                    break;
                }
                // Entry, Search in overflow
                if (node->IsEntry()) {
                    for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
                         currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
                        auto oldValue = currentEntry->Value<SlotBarrier>();
                        if (!IsNull(oldValue) && std::invoke(std::forward<EqualsCallback>(equalsCallback),
                                                             oldValue)) {
                            return oldValue;
                        }
                    }
                    haveInsertPoint = true;
                    break;
                }
                // Indirect, Next level Continue to search
                current = node->AsIndirect();
            }
#ifndef NDEBUG
            if (!haveInsertPoint) {
                LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
                UNREACHABLE_CC();
            }
#endif
            // lock and double-check
            GetMutex().LockWithThreadState(holder);
            node = slot->load(std::memory_order_acquire);
            if (node == nullptr || node->IsEntry()) {
                // see is still real, so can continue to insert.
                break;
            }
            GetMutex().Unlock();
            current = node->AsIndirect();
            hashShift += TrieMapConfig::N_CHILDREN_LOG2;
        }
    }
    Entry* oldEntry = nullptr;
    uint32_t oldHash = key;
    if (node != nullptr) {
        oldEntry = node->AsEntry();
        for (Entry* currentEntry = oldEntry; currentEntry != nullptr;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            auto oldValue = currentEntry->Value<SlotBarrier>();
            if (IsNull(oldValue)) {
                continue;
            }
            if (oldValue->GetMixHashcode() != key) {
                oldHash = oldValue->GetMixHashcode();
                continue;
            }
            if (std::invoke(std::forward<EqualsCallback>(equalsCallback), oldValue)) {
                GetMutex().Unlock();
                return oldValue;
            }
        }
    }

    BaseString* value = *str;
    DCHECK_CC(value != nullptr);
    value->SetIsInternString();
    IntegerCache::InitIntegerCache(value);
    Entry* newEntry = new Entry(value);
    oldEntry = PruneHead(oldEntry);
    if (oldEntry == nullptr) {
        // The simple case: Create a new entry and store it.
        slot->store(newEntry, std::memory_order_release);
    } else {
        // Expand an existing entry to one or more new nodes.
        // Release the node, which will make both oldEntry and newEntry visible
        auto expandedNode = Expand<true>(oldEntry, newEntry,
            oldHash >> TrieMapConfig::ROOT_BIT, hash, hashShift, current);
        slot->store(expandedNode, std::memory_order_release);
    }

    GetMutex().Unlock();
    return value;
}

// Load returns the value of the key stored in the mapping, or HashTrieMapLoadResult for StoreOrLoad
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <typename ReadBarrier>
HashTrieMapLoadResult HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Load(ReadBarrier&& readBarrier,
                                                                          const uint32_t key, BaseString* value)
{
    uint32_t hash = key;
    Indirect* current = GetRootAndProcessHash(hash);
    for (uint32_t hashShift = 0; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift +=
         TrieMapConfig::N_CHILDREN_LOG2) {
        size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;

        std::atomic<Node*>* slot = &current->GetChild(index);
        Node* node = slot->load(std::memory_order_acquire);
        if (node == nullptr) {
            return {nullptr, current, hashShift, slot};
        }
        if (node->IsEntry()) {
            for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
                 currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
                auto oldValue = currentEntry->Value<SlotBarrier>();
                if (IsNull(oldValue)) {
                    continue;
                }
                if (BaseString::StringsAreEqual(std::forward<ReadBarrier>(readBarrier), oldValue, value)) {
                    return {oldValue, nullptr, hashShift, nullptr};
                }
            }
            return {nullptr, current, hashShift, slot};
        }
        current = node->AsIndirect();
    }

    LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
    UNREACHABLE_CC();
}

// Load returns the value of the key stored in the mapping, or HashTrieMapLoadResult for StoreOrLoad
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <typename ReadBarrier>
HashTrieMapLoadResult HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Load(ReadBarrier&& readBarrier, const uint32_t key,
                                                                          const ReadOnlyHandle<BaseString>& string,
                                                                          uint32_t offset, uint32_t utf8Len)
{
    uint32_t hash = key;
    Indirect* current = GetRootAndProcessHash(hash);
    const uint8_t* utf8Data = ReadOnlyHandle<LineString>::Cast(string)->GetDataUtf8() + offset;
    for (uint32_t hashShift = 0; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift +=
         TrieMapConfig::N_CHILDREN_LOG2) {
        size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;

        std::atomic<Node*>* slot = &current->GetChild(index);
        Node* node = slot->load(std::memory_order_acquire);
        if (node == nullptr) {
            return {nullptr, current, hashShift, slot};
        }
        if (!node->IsEntry()) {
            current = node->AsIndirect();
            continue;
        }
        for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            auto oldValue = currentEntry->Value<SlotBarrier>();
            if (IsNull(oldValue)) {
                continue;
            }
            if (BaseString::StringIsEqualUint8Data(std::forward<ReadBarrier>(readBarrier), oldValue, utf8Data, utf8Len,
                                                   true)) {
                return {oldValue, nullptr, hashShift, nullptr};
            }
        }
        return {nullptr, current, hashShift, slot};
    }

    LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
    UNREACHABLE_CC();
}

// Based on the loadResult, try the store first
// StoreOrLoad returns the existing value of the key, store the value, and return the value
template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <bool threadState, typename ReadBarrier, typename HandleType>
BaseString* HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::StoreOrLoad(ThreadHolder* holder, ReadBarrier&& readBarrier,
                                                                       const uint32_t key,
                                                                       HashTrieMapLoadResult loadResult,
                                                                       HandleType str)
{
    HashTrieMapInUseScope mapInUse(this);
    uint32_t hash = key;
    ProcessHash(hash);
    uint32_t hashShift = loadResult.hashShift;
    std::atomic<Node*>* slot = loadResult.slot;
    Node* node = nullptr;
    [[maybe_unused]] bool haveInsertPoint = true;
    Indirect* current = loadResult.current;
    if constexpr (threadState) {
        GetMutex().LockWithThreadState(holder);
    } else {
        GetMutex().Lock();
    }
    node = slot->load(std::memory_order_acquire);
    if (node != nullptr && !node->IsEntry()) {
        GetMutex().Unlock();
        current = node->AsIndirect();
        hashShift += TrieMapConfig::N_CHILDREN_LOG2;
        while (true) {
            haveInsertPoint = false;
            for (; hashShift < TrieMapConfig::TOTAL_HASH_BITS; hashShift += TrieMapConfig::N_CHILDREN_LOG2) {
                size_t index = (hash >> hashShift) & TrieMapConfig::N_CHILDREN_MASK;
                slot = &current->GetChild(index);
                node = slot->load(std::memory_order_acquire);
                if (node == nullptr) {
                    haveInsertPoint = true;
                    break;
                }
                // Entry, Search in overflow
                if (!node->IsEntry()) {
                    // Indirect, Next level Continue to search
                    current = node->AsIndirect();
                    continue;
                }
                for (Entry* currentEntry = node->AsEntry(); currentEntry != nullptr;
                     currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
                    BaseString* oldValue = currentEntry->Value<SlotBarrier>();
                    if (IsNull(oldValue)) {
                        continue;
                    }
                    if (BaseString::StringsAreEqual(std::forward<ReadBarrier>(readBarrier), oldValue, *str)) {
                        return oldValue;
                    }
                }
                haveInsertPoint = true;
                break;
            }
#ifndef NDEBUG
            if (!haveInsertPoint) {
                LOG_COMMON(FATAL) << "StringTable: ran out of hash bits while iterating";
                UNREACHABLE_CC();
            }
#endif
            // lock and double-check
            if constexpr (threadState) {
                GetMutex().LockWithThreadState(holder);
            } else {
                GetMutex().Lock();
            }
            node = slot->load(std::memory_order_acquire);
            if (node == nullptr || node->IsEntry()) {
                // see is still real, so can continue to insert.
                break;
            }
            GetMutex().Unlock();
            current = node->AsIndirect();
            hashShift += TrieMapConfig::N_CHILDREN_LOG2;
        }
    }

    Entry* oldEntry = nullptr;
    uint32_t oldHash = key;
    if (node != nullptr) {
        oldEntry = node->AsEntry();
        for (Entry* currentEntry = oldEntry; currentEntry != nullptr;
             currentEntry = currentEntry->Overflow().load(std::memory_order_acquire)) {
            BaseString* oldValue = currentEntry->Value<SlotBarrier>();
            if (IsNull(oldValue)) {
                continue;
            }
            if (oldValue->GetMixHashcode() != key) {
                oldHash = oldValue->GetMixHashcode();
                continue;
            }
            if (BaseString::StringsAreEqual(std::forward<ReadBarrier>(readBarrier), oldValue, *str)) {
                GetMutex().Unlock();
                return oldValue;
            }
        }
    }

    BaseString* value = *str;
    DCHECK_CC(value != nullptr);
    value->SetIsInternString();
    IntegerCache::InitIntegerCache(value);
    Entry* newEntry = new Entry(value);
    oldEntry = PruneHead(oldEntry);
    if (oldEntry == nullptr) {
        // The simple case: Create a new entry and store it.
        slot->store(newEntry, std::memory_order_release);
    }  else {
        // Expand an existing entry to one or more new nodes.
        // Release the node, which will make both oldEntry and newEntry visible
        auto expandedNode = Expand<true>(oldEntry, newEntry,
            oldHash >> TrieMapConfig::ROOT_BIT, hash, hashShift, current);
        slot->store(expandedNode, std::memory_order_release);
    }
    GetMutex().Unlock();
    return value;
}


template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::CheckWeakRef(const WeakRootVisitor& visitor, Entry* entry)
{
    panda::ecmascript::TaggedObject* object = reinterpret_cast<panda::ecmascript::TaggedObject*>(entry->Value<
        SlotBarrier>());
    auto fwd = visitor(object);
    if (fwd == nullptr) {
        LOG_COMMON(VERBOSE) << "StringTable: delete string " << std::hex << object;
        return true;
    } else if (fwd != object) {
        entry->SetValue(reinterpret_cast<BaseString*>(fwd));
        LOG_COMMON(VERBOSE) << "StringTable: forward " << std::hex << object << " -> " << fwd;
    }
    return false;
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <typename ReadBarrier>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::CheckValidity(ReadBarrier&& readBarrier, BaseString* value,
                                                                  bool& isValid)
{
    if (value->IsTreeString()) {
        isValid = false;
        return false;
    }
    uint32_t hashcode = value->GetHashcode(std::forward<ReadBarrier>(readBarrier));
    if (this->template Load<true>(std::forward<ReadBarrier>(readBarrier), hashcode, value) != nullptr) {
        isValid = false;
        return false;
    }
    return true;
}

template<typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::Iter(Indirect *node, std::function<bool(Node *)> &iter)
{
    if (node == nullptr) {
        return true;
    }
    if (!iter(node)) {
        return false;
    }
    for (std::atomic<uint64_t> &temp: node->children_) {
        auto &child = reinterpret_cast<std::atomic<HashTrieMapNode *> &>(temp);
        Node *childNode = child.load(std::memory_order_acquire);
        if (childNode == nullptr)
            continue;

        if (!(childNode->IsEntry())) {
            // Recursive traversal of indirect nodes
            Iter(childNode->AsIndirect(), iter);
            continue;
        }

        for (Entry *e = childNode->AsEntry(); e != nullptr; e = e->Overflow().load(std::memory_order_acquire)) {
            if (!iter(e)) {
                return false;
            }
        }
    }
    return true;
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::CheckWeakRef(const WeakRefFieldVisitor& visitor,
                                                                 HashTrieMap::Entry* entry)
{
    // RefField only support 64-bit value, so could not cirectly pass `Entry::Value` to WeakRefFieldVisitor
    // int 32-bit machine
    if (SlotBarrier == TrieMapConfig::NeedSlotBarrier) {
        uint64_t str = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(entry->Value<SlotBarrier>()));
        bool isAlive = visitor(reinterpret_cast<RefField<>&>(str));
        entry->SetValue(reinterpret_cast<BaseString*>(static_cast<uintptr_t>(str)));
        return isAlive;
    }
    uint64_t str = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(entry->Value<SlotBarrier>()));
    bool isAlive = visitor(reinterpret_cast<RefField<>&>(str));
    if (!isAlive) {
        return true;
    }
    entry->SetValue(reinterpret_cast<BaseString*>(static_cast<uintptr_t>(str)));
    return false;
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <TrieMapConfig::SlotBarrier Barrier, std::enable_if_t<Barrier == TrieMapConfig::NeedSlotBarrier, int>>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::ClearNodeFromGC(Indirect* parent, int index,
                                                                    const WeakRefFieldVisitor& visitor,
                                                                    std::vector<Entry*>& waitDeleteEntries)
{
    // load sub-nodes
    Node* child = parent->GetChild(index).load(std::memory_order_relaxed);
    if (child == nullptr)
        return true;

    if (child->IsEntry()) {
        // Processing the overflow linked list
        for (Entry *prev = nullptr, *current = child->AsEntry(); current != nullptr; current = current->
             Overflow().load(std::memory_order_acquire)) {
            if (!CheckWeakRef(visitor, current) && prev != nullptr) {
                prev->Overflow().store(current->Overflow().load(std::memory_order_acquire), std::memory_order_release);
                waitDeleteEntries.push_back(current);
            } else {
                prev = current;
            }
        }
        return false;
    } else {
        // Recursive processing of the Indirect node
        Indirect* indirect = child->AsIndirect();
        uint32_t cleanCount = 0;
        for (uint32_t i = 0; i < TrieMapConfig::INDIRECT_SIZE; ++i) {
            if (ClearNodeFromGC(indirect, i, visitor, waitDeleteEntries)) {
                cleanCount += 1;
            }
        }
        return false;
    }
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <TrieMapConfig::SlotBarrier Barrier, std::enable_if_t<Barrier == TrieMapConfig::NoSlotBarrier, int>>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::ClearNodeFromGC(Indirect* parent, int index,
                                                                    const WeakRefFieldVisitor& visitor)
{
    // load sub-nodes
    Node* child = parent->GetChild(index).load(std::memory_order_relaxed);
    if (child == nullptr) {
        return true;
    }
    if (child->IsEntry()) {
        Entry* entry = child->AsEntry();
        // Processing the overflow linked list
        for (Entry *prev = nullptr, *current = entry->Overflow().load(std::memory_order_relaxed); current != nullptr
             ;) {
            Entry* next = current->Overflow().load(std::memory_order_relaxed);
            if (CheckWeakRef(visitor, current)) {
                // Remove and remove a node from a linked list
                if (prev) {
                    prev->Overflow().store(next, std::memory_order_relaxed);
                } else {
                    entry->Overflow().store(next, std::memory_order_relaxed);
                }
                delete current;
            } else {
                prev = current;
            }
            current = next;
        }
        // processing entry node
        if (CheckWeakRef(visitor, entry)) {
            Entry* e = entry->Overflow().load(std::memory_order_relaxed);
            if (e == nullptr) {
                // Delete the empty Entry node and update the parent reference
                delete entry;
                parent->GetChild(index).store(nullptr, std::memory_order_relaxed);
                return true;
            }
            // Delete the Entry node and update the parent reference
            delete entry;
            parent->GetChild(index).store(e, std::memory_order_relaxed);
        }
        return false;
    } else {
        // Recursive processing of the Indirect node
        Indirect* indirect = child->AsIndirect();
        uint32_t cleanCount = 0;
        for (uint32_t i = 0; i < TrieMapConfig::INDIRECT_SIZE; ++i) {
            if (ClearNodeFromGC(indirect, i, visitor)) {
                cleanCount += 1;
            }
        }
        // Check whether the indirect node is empty
        if (cleanCount == TrieMapConfig::INDIRECT_SIZE) {
            // Remove the empty Indirect and update the parent reference
            delete indirect;
            parent->GetChild(index).store(nullptr, std::memory_order_relaxed);
            return true;
        }
        return false;
    }
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
template <TrieMapConfig::SlotBarrier Barrier, std::enable_if_t<Barrier == TrieMapConfig::NoSlotBarrier, int>>
bool HashTrieMap<Mutex, ThreadHolder, SlotBarrier>::ClearNodeFromGC(Indirect* parent, int index,
                                                                    const WeakRootVisitor& visitor)
{
    // load sub-nodes
    Node* child = parent->GetChild(index).load(std::memory_order_relaxed);
    if (child == nullptr)
        return true;

    if (child->IsEntry()) {
        Entry* entry = child->AsEntry();
        // Processing the overflow linked list
        for (Entry *prev = nullptr, *current = entry->Overflow().load(std::memory_order_relaxed); current != nullptr;) {
            Entry* next = current->Overflow().load(std::memory_order_relaxed);
            if (CheckWeakRef(visitor, current)) {
                // Remove and remove a node from a linked list
                if (prev != nullptr) {
                    prev->Overflow().store(next, std::memory_order_relaxed);
                } else {
                    entry->Overflow().store(next, std::memory_order_relaxed);
                }
                delete current;
            } else {
                prev = current;
            }
            current = next;
        }
        // processing entry node
        if (CheckWeakRef(visitor, entry)) {
            Entry* e = entry->Overflow().load(std::memory_order_relaxed);
            if (e == nullptr) {
                // Delete the empty Entry node and update the parent reference
                delete entry;
                parent->GetChild(index).store(nullptr, std::memory_order_relaxed);
                return true;
            }
            // Delete the Entry node and update the parent reference
            delete entry;
            parent->GetChild(index).store(e, std::memory_order_relaxed);
        }
        return false;
    } else {
        // Recursive processing of the Indirect node
        Indirect* indirect = child->AsIndirect();
        uint32_t cleanCount = 0;
        for (uint32_t i = 0; i < TrieMapConfig::INDIRECT_SIZE; ++i) {
            if (ClearNodeFromGC(indirect, i, visitor)) {
                cleanCount += 1;
            }
        }
        // Check whether the indirect node is empty
        if (cleanCount == TrieMapConfig::INDIRECT_SIZE && inuseCount_ == 0) {
            // Remove the empty Indirect and update the parent reference
            delete indirect;
            parent->GetChild(index).store(nullptr, std::memory_order_relaxed);
            return true;
        }
        return false;
    }
}
}
#endif //COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_INL_H

