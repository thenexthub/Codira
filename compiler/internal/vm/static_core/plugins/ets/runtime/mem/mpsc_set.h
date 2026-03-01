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

#ifndef PLUGINS_ETS_RUNTIME_MEM_MPSC_SET_H
#define PLUGINS_ETS_RUNTIME_MEM_MPSC_SET_H

#include <cstddef>
#include <libarkbase/macros.h>
#include <libarkbase/os/mutex.h>
#include <libarkbase/os/thread.h>
#include <unordered_map>

namespace ark::mem {

/**
 * @brief MPSCSet is tool to optimize usage of set in next case:
 * - You need to store unique elements in set.
 * - You want to collect them in one set from many threads
 * - You want to process elements from one thread
 * - You have splitted steps of execution with collecting and processing
 * @tparam StoredSetType - type of set you want to use inside
 */
template <class StoredSetType>
class MPSCSet {
public:
    using ValueType = typename StoredSetType::value_type;
    using AllocatorType = typename StoredSetType::allocator_type;

    MPSCSet() = default;
    NO_COPY_SEMANTIC(MPSCSet);
    NO_MOVE_SEMANTIC(MPSCSet);
    ~MPSCSet() = default;

    /**
     * @brief Method adds value in thread local set
     * @param val: instance of ValueType, that you want to add
     */
    void Insert(ValueType val);
    /**
     * @brief Method flushes all data from local sets to single global set. Only after this operation content of the
     * MPSCSet will be observable.
     */
    void FlushSets();

    /**
     * @brief Method extract one value from the global set. Please, check if global set contains values with IsEmpty()
     * method.
     * @see MPSCSet::FlushSets()
     * @returns instance of ValueType, that was presented in global set.
     */
    ValueType Extract();
    /// @returns true if global set is empty, otherwise @returns false.
    bool IsEmpty() const;
    /// @returns size of global set
    size_t Size() const;

private:
    template <class Key, class Value>
    using SetStorageType =
        std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>,
                           typename AllocatorType::template rebind<std::pair<const Key, Value>>::other>;

    void AddNewSetInStorage();

    mutable os::memory::RWLock setStorageLock_;
    SetStorageType<os::thread::ThreadId, StoredSetType> setStorage_ GUARDED_BY(setStorageLock_);

    StoredSetType globalSet_;
};

template <class StoredSetType>
void MPSCSet<StoredSetType>::Insert(ValueType val)
{
    setStorageLock_.ReadLock();
    auto setIterator = setStorage_.find(os::thread::GetCurrentThreadId());
    if UNLIKELY (setIterator == setStorage_.end()) {
        // start slow path
        setStorageLock_.Unlock();
        AddNewSetInStorage();
        setStorageLock_.ReadLock();
        setIterator = setStorage_.find(os::thread::GetCurrentThreadId());
    }
    // getting of set with additional search
    auto &set = setIterator->second;
    set.insert(val);
    setStorageLock_.Unlock();
}

template <class StoredSetType>
void MPSCSet<StoredSetType>::AddNewSetInStorage()
{
    os::memory::WriteLockHolder wLockHolder(setStorageLock_);
    setStorage_.insert({os::thread::GetCurrentThreadId(), StoredSetType()});
}

template <class StoredSetType>
void MPSCSet<StoredSetType>::FlushSets()
{
    os::memory::WriteLockHolder wLockHolder(setStorageLock_);
    for ([[maybe_unused]] auto &[id, set] : setStorage_) {
        globalSet_.merge(set);
    }
    setStorage_.clear();
}

template <class StoredSetType>
typename MPSCSet<StoredSetType>::ValueType MPSCSet<StoredSetType>::Extract()
{
    return globalSet_.extract(globalSet_.begin()).value();
}

template <class StoredSetType>
bool MPSCSet<StoredSetType>::IsEmpty() const
{
    return globalSet_.empty();
}

template <class StoredSetType>
size_t MPSCSet<StoredSetType>::Size() const
{
    return globalSet_.size();
}

}  // namespace ark::mem

#endif  // PLUGINS_ETS_RUNTIME_MEM_MPSC_SET_H