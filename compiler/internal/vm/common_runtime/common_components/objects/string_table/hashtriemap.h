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

#ifndef COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_H
#define COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_H

#include "common_components/heap/heap.h"
#include "common_components/log/log.h"
#include "common_interfaces/base/common.h"
#include "common_interfaces/objects/readonly_handle.h"
#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/objects/string/base_string-inl.h"

namespace panda::ecmascript {
class TaggedObject;
}

namespace common {
class TrieMapConfig {
public:
    static constexpr uint32_t ROOT_BIT = 11U;
    static constexpr uint32_t ROOT_SIZE = (1 << ROOT_BIT);
    static constexpr uint32_t ROOT_BIT_MASK = ROOT_SIZE - 1U;

    static constexpr uint32_t N_CHILDREN_LOG2 = 3U;
    static constexpr uint32_t TOTAL_HASH_BITS = 32U - ROOT_BIT;

    static constexpr uint32_t N_CHILDREN = 1 << N_CHILDREN_LOG2;
    static constexpr uint32_t N_CHILDREN_MASK = N_CHILDREN - 1U;

    static constexpr uint32_t INDIRECT_SIZE = 8U; // 8: 2^3
    static constexpr uint32_t INDIRECT_MASK = INDIRECT_SIZE - 1U;

    static constexpr uint64_t HIGH_8_BIT_MASK = 0xFFULL << 56; // used to detect 56-63 Bits

    enum SlotBarrier {
        NeedSlotBarrier,
        NoSlotBarrier,
    };
};

class HashTrieMapEntry;
class HashTrieMapIndirect;

class HashTrieMapNode {
public:
    // Do not use 57-64bits, HWAsan uses 57-64 bits as pointer tag
    static constexpr uint64_t POINTER_LENGTH = 48;
    static constexpr uint64_t ENTRY_TAG_MASK = 1ULL << POINTER_LENGTH;

    using Pointer = BitField<uint64_t, 0, POINTER_LENGTH>;
    using EntryBit = Pointer::NextFlag;

    explicit HashTrieMapNode() {}

    bool IsEntry() const
    {
        uint64_t bitField = *reinterpret_cast<const uint64_t *>(this);
        return EntryBit::Decode(bitField);
    }

    HashTrieMapEntry* AsEntry();
    HashTrieMapIndirect* AsIndirect();
};

class HashTrieMapEntry final : public HashTrieMapNode {
public:
    HashTrieMapEntry(BaseString* v) : overflow_(nullptr)
    {
        // Note: CMC GC assumes string is always a non-young object and tries to optimize it out in young GC
        ASSERT_LOGF(Heap::GetHeap().IsHeapAddress(v)
                        ? Heap::GetHeap().InRecentSpace(v) == false
                        : true,
                    "Violate CMC-GC assumption: should not be young object");

        bitField_ = (ENTRY_TAG_MASK | reinterpret_cast<uint64_t>(v));
    }

    template <TrieMapConfig::SlotBarrier SlotBarrier>
    BaseString* Value() const
    {
        uint64_t value = Pointer::Decode(bitField_);
        if constexpr (SlotBarrier == TrieMapConfig::NoSlotBarrier) {
            return reinterpret_cast<BaseString *>(static_cast<uintptr_t>(value));
        }
        return reinterpret_cast<BaseString*>(Heap::GetBarrier().ReadStringTableStaticRef(
            *reinterpret_cast<RefField<false>*>((void*)(&value))));
    }

    void SetValue(BaseString* v)
    {
        // Note: CMC GC assumes string is always a non-young object and tries to optimize it out in young GC
        ASSERT_LOGF(Heap::GetHeap().IsHeapAddress(v)
                        ? Heap::GetHeap().InRecentSpace(v) == false
                        : true,
                    "Violate CMC-GC assumption: should not be young object");

        bitField_ = ENTRY_TAG_MASK | reinterpret_cast<uint64_t>(v);
    }

    std::atomic<HashTrieMapEntry*>& Overflow()
    {
        return overflow_;
    }

    uint64_t GetBitField() const
    {
        return bitField_;
    }

private:
    uint64_t bitField_;
    std::atomic<HashTrieMapEntry*> overflow_;
};

class HashTrieMapIndirect final : public HashTrieMapNode {
public:
    std::array<std::atomic<uint64_t>, TrieMapConfig::INDIRECT_SIZE> children_{};

    explicit HashTrieMapIndirect() {}

    ~HashTrieMapIndirect()
    {
        for (std::atomic<uint64_t>& temp : children_) {
            auto &child = reinterpret_cast<std::atomic<HashTrieMapNode*>&>(temp);
            HashTrieMapNode* node = child.exchange(nullptr, std::memory_order_relaxed);
            if (node == nullptr) {
                continue;
            }
            if (!node->IsEntry()) {
                delete node->AsIndirect();
                continue;
            }
            HashTrieMapEntry* e = node->AsEntry();
            // Clear overflow chain
            for (HashTrieMapEntry* current = e->Overflow().exchange(nullptr, std::memory_order_relaxed); current !=
                 nullptr
                 ;) {
                // atom get the next node and break the link
                HashTrieMapEntry* next = current->Overflow().exchange(nullptr, std::memory_order_relaxed);
                // deletion of the current node
                delete current;
                // move to the next node
                current = next;
            }
            delete e;
        }
    }

    std::atomic<HashTrieMapNode*>& GetChild(size_t index)
    {
        return reinterpret_cast<std::atomic<HashTrieMapNode*>&>(children_[index]);
    }
};

struct HashTrieMapLoadResult {
    BaseString* value;
    HashTrieMapIndirect* current;
    uint32_t hashShift;
    std::atomic<HashTrieMapNode*>* slot;
};

inline HashTrieMapEntry* HashTrieMapNode::AsEntry()
{
    DCHECK_CC(IsEntry() && "HashTrieMap: called entry on non-entry node");
    return static_cast<HashTrieMapEntry*>(this);
}

inline HashTrieMapIndirect* HashTrieMapNode::AsIndirect()
{
    DCHECK_CC(!IsEntry() && "HashTrieMap: called indirect on entry node");
    return static_cast<HashTrieMapIndirect*>(this);
}

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
class HashTrieMap {
public:
    using WeakRefFieldVisitor = std::function<bool(RefField<>&)>;
    using WeakRootVisitor = std::function<panda::ecmascript::TaggedObject *(panda::ecmascript::TaggedObject* p)>;
    using Node = HashTrieMapNode;
    using Indirect = HashTrieMapIndirect;
    using Entry = HashTrieMapEntry;
    using LoadResult = HashTrieMapLoadResult;
    HashTrieMap() {}

    ~HashTrieMap()
    {
        Clear();
    };

#if ECMASCRIPT_ENABLE_TRACE_STRING_TABLE
    class StringTableTracer {
    public:
        static constexpr uint32_t DUMP_THRESHOLD = 40000;
        static StringTableTracer& GetInstance()
        {
            static StringTableTracer tracer;
            return tracer;
        }

        NO_COPY_SEMANTIC_CC(StringTableTracer);
        NO_MOVE_SEMANTIC_CC(StringTableTracer);

        void TraceFindSuccess(uint32_t hashShift)
        {
            totalDepth_.fetch_add(hashShift / TrieMapConfig::N_CHILDREN_LOG2 + 1, std::memory_order_relaxed);
            uint64_t currentSuccess = totalSuccessNum_.fetch_add(1, std::memory_order_relaxed) + 1;
            if (currentSuccess >= lastDumpPoint_.load(std::memory_order_relaxed) + DUMP_THRESHOLD) {
                DumpWithLock(currentSuccess);
            }
        }

        void TraceFindFail()
        {
            totalFailNum_.fetch_add(1, std::memory_order_relaxed);
        }

    private:
        StringTableTracer() = default;

        void DumpWithLock(uint64_t triggerPoint)
        {
            std::lock_guard<std::mutex> lock(mu_);

            if (triggerPoint >= lastDumpPoint_.load(std::memory_order_relaxed) + DUMP_THRESHOLD) {
                lastDumpPoint_ = triggerPoint;
                DumpInfo();
            }
        }

        void DumpInfo() const
        {
            uint64_t depth = totalDepth_.load(std::memory_order_relaxed);
            uint64_t success = totalSuccessNum_.load(std::memory_order_relaxed);
            uint64_t fail = totalFailNum_.load(std::memory_order_relaxed);

            double avgDepth = (static_cast<double>(depth) / success);

            LOG_COMMON(INFO) << "------------------------------------------------------------"
                           << "---------------------------------------------------------";
            LOG_COMMON(INFO) << "StringTableTotalSuccessFindNum: " << success;
            LOG_COMMON(INFO) << "StringTableTotalInsertNum: " << fail;
            LOG_COMMON(INFO) << "StringTableAverageDepth: " << avgDepth;
            LOG_COMMON(INFO) << "------------------------------------------------------------"
                           << "---------------------------------------------------------";
        }

        std::mutex mu_;
        std::atomic<uint64_t> totalDepth_{0};
        std::atomic<uint64_t> totalSuccessNum_{0};
        std::atomic<uint64_t> totalFailNum_{0};
        std::atomic<uint64_t> lastDumpPoint_{0};
    };

    void TraceFindSuccessDepth(uint32_t hashShift)
    {
        StringTableTracer::GetInstance().TraceFindSuccess(hashShift);
    }

    void TraceFindFail()
    {
        StringTableTracer::GetInstance().TraceFindFail();
    }
#endif
    template <typename ReadBarrier>
    LoadResult Load(ReadBarrier&& readBarrier, const uint32_t key, BaseString* value);

    template <bool threadState = true, typename ReadBarrier, typename HandleType>
    BaseString* StoreOrLoad(ThreadHolder* holder, ReadBarrier&& readBarrier, const uint32_t key, LoadResult loadResult,
                            HandleType str);
    template <typename ReadBarrier>
    LoadResult Load(ReadBarrier&& readBarrier, const uint32_t key, const ReadOnlyHandle<BaseString>& string,
                    uint32_t offset, uint32_t utf8Len);
    template <typename LoaderCallback, typename EqualsCallback>
    BaseString* StoreOrLoad(ThreadHolder* holder, const uint32_t key, LoadResult loadResult,
                            LoaderCallback loaderCallback, EqualsCallback equalsCallback);

    template <bool IsCheck, typename ReadBarrier>
    BaseString* Load(ReadBarrier&& readBarrier, const uint32_t key, BaseString* value);
    template <bool IsLock, typename LoaderCallback, typename EqualsCallback>
    BaseString* LoadOrStore(ThreadHolder* holder, const uint32_t key, LoaderCallback loaderCallback,
                            EqualsCallback equalsCallback);

    template <typename LoaderCallback, typename EqualsCallback>
    BaseString* LoadOrStoreForJit(ThreadHolder* holder, const uint32_t key, LoaderCallback loaderCallback,
                                  EqualsCallback equalsCallback);

    static void ProcessHash(uint32_t &hash)
    {
        hash >>= TrieMapConfig::ROOT_BIT;
    }

    Indirect* GetRootAndProcessHash(uint32_t &hash)
    {
        uint32_t rootID = (hash & TrieMapConfig::ROOT_BIT_MASK);
        hash >>= TrieMapConfig::ROOT_BIT;
        auto root = root_[rootID].load(std::memory_order_acquire);
        if (root != nullptr) {
            return root;
        } else {
            Indirect* expected = nullptr;
            Indirect* newRoot = new Indirect();

            if (root_[rootID].compare_exchange_strong(expected, newRoot,
                                                      std::memory_order_release, std::memory_order_acquire)) {
                return newRoot;
            } else {
                delete newRoot;
                return expected;
            }
        }
    }

    // All other threads have stopped due to the gc and Clear phases.
    // Therefore, the operations related to atoms in ClearNodeFromGc and Clear use std::memory_order_relaxed,
    // which ensures atomicity but does not guarantee memory order
    template <TrieMapConfig::SlotBarrier Barrier = SlotBarrier,
              std::enable_if_t<Barrier == TrieMapConfig::NeedSlotBarrier, int>  = 0>
    bool ClearNodeFromGC(Indirect* parent, int index, const WeakRefFieldVisitor& visitor,
                         std::vector<Entry*>& waitDeleteEntries);

    template <TrieMapConfig::SlotBarrier Barrier = SlotBarrier,
              std::enable_if_t<Barrier == TrieMapConfig::NoSlotBarrier, int>  = 0>
    bool ClearNodeFromGC(Indirect* parent, int index, const WeakRefFieldVisitor& visitor);

    template <TrieMapConfig::SlotBarrier Barrier = SlotBarrier,
              std::enable_if_t<Barrier == TrieMapConfig::NoSlotBarrier, int>  = 0>
    bool ClearNodeFromGC(Indirect* parent, int index, const WeakRootVisitor& visitor);

    // Iterator
    template<typename ReadBarrier>
    void Range(ReadBarrier &&readBarrier, bool &isValid)
    {
        std::function<bool(Node *)> iter = [readBarrier, &isValid, this](Node *node) {
            if (node->IsEntry()) {
                BaseString *value = node->AsEntry()->Value<SlotBarrier>();
                if (!IsNull(value) && !this->CheckValidity(readBarrier, value, isValid)) {
                    return false;
                }
            }
            return true;
        };
        Range(iter);
    }

    void Range(std::function<bool(Node*)> &iter)
    {
        for (uint32_t i = 0; i < TrieMapConfig::ROOT_SIZE; i++) {
            if (!Iter(root_[i].load(std::memory_order_acquire), iter)) {
                return;
            }
        }
    }

    void Clear()
    {
        for (uint32_t i = 0; i < TrieMapConfig::ROOT_SIZE; i++) {
            // The atom replaces the root node with nullptr and obtains the old root node
            Indirect* oldRoot = root_[i].exchange(nullptr, std::memory_order_relaxed);
            if (oldRoot != nullptr) {
                // Clear the entire HashTreeMap based on the Indirect destructor
                delete oldRoot;
            }
        }
    }

    // ut used
    const std::atomic<Indirect*>& GetRoot(uint32_t index) const
    {
        DCHECK_CC(index < TrieMapConfig::ROOT_SIZE);
        return root_[index];
    }

    void IncreaseInuseCount()
    {
        inuseCount_.fetch_add(1);
    }

    void DecreaseInuseCount()
    {
        inuseCount_.fetch_sub(1);
    }

    Mutex& GetMutex()
    {
        return mu_;
    }

    template <TrieMapConfig::SlotBarrier Barrier = SlotBarrier,
              std::enable_if_t<Barrier == TrieMapConfig::NeedSlotBarrier, int>  = 0>
    void CleanUp()
    {
        for (const HashTrieMapEntry* entry : waitFreeEntries_) {
            delete entry;
        }
        waitFreeEntries_.clear();
    }

    void StartSweeping()
    {
        GetMutex().Lock();
        isSweeping = true;
        GetMutex().Unlock();
    }

    void FinishSweeping()
    {
        GetMutex().Lock();
        isSweeping = false;
        GetMutex().Unlock();
    }
    std::atomic<Indirect*> root_[TrieMapConfig::ROOT_SIZE] = {};
private:
    Mutex mu_;
    std::vector<Entry*> waitFreeEntries_{};
    std::atomic<uint32_t> inuseCount_{0};
    bool isSweeping{false};
    template <bool IsLock>
    Node* Expand(Entry* oldEntry, Entry* newEntry,
        uint32_t oldHash, uint32_t newHash, uint32_t hashShift, Indirect* parent);

    bool Iter(Indirect *node, std::function<bool(Node *)> &iter);

    bool CheckWeakRef(const WeakRefFieldVisitor& visitor, Entry* entry);
    bool CheckWeakRef(const WeakRootVisitor& visitor, Entry* entry);

    template <typename ReadBarrier>
    bool CheckValidity(ReadBarrier&& readBarrier, BaseString* value, bool& isValid);

    constexpr static bool IsNull(BaseString* value)
    {
        if constexpr (SlotBarrier == TrieMapConfig::NoSlotBarrier) {
            return false;
        }
        return value == nullptr;
    }

    constexpr Entry* PruneHead(Entry* entry)
    {
        if constexpr (SlotBarrier == TrieMapConfig::NoSlotBarrier) {
            return entry;
        }
        if (entry == nullptr || isSweeping) {
            // can't be pruned when sweeping.
            return entry;
        }
        if (entry->Value<SlotBarrier>() == nullptr) {
            waitFreeEntries_.push_back(entry);
            return entry->Overflow();
        }
        return entry;
    }
};

template <typename Mutex, typename ThreadHolder, TrieMapConfig::SlotBarrier SlotBarrier>
class HashTrieMapInUseScope {
public:
    HashTrieMapInUseScope(HashTrieMap<Mutex, ThreadHolder, SlotBarrier>* hashTrieMap) : hashTrieMap_(hashTrieMap)
    {
        hashTrieMap_->IncreaseInuseCount();
    }

    ~HashTrieMapInUseScope()
    {
        hashTrieMap_->DecreaseInuseCount();
    }

    NO_COPY_SEMANTIC_CC(HashTrieMapInUseScope);
    NO_MOVE_SEMANTIC_CC(HashTrieMapInUseScope);

private:
    HashTrieMap<Mutex, ThreadHolder, SlotBarrier>* hashTrieMap_;
};
}
#endif //COMMON_COMPONENTS_OBJECTS_STRING_TABLE_HASHTRIEMAP_H

