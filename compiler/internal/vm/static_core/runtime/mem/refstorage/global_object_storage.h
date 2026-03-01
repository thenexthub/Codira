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
#ifndef PANDA_GLOBAL_OBJECT_STORAGE_H
#define PANDA_GLOBAL_OBJECT_STORAGE_H

#include <libarkbase/os/mutex.h>

#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/include/class.h"
#include "runtime/include/panda_vm.h"
#include "reference.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/dfx.h"

namespace ark::mem::test {
class ReferenceStorageTest;
}  // namespace ark::mem::test

namespace ark::mem {

/**
 * Storage for objects which need to handle by GC. GC will handle moving these objects and will not reclaim then until
 * user haven't called Remove method on this object.
 * References will be removed automatically after Remove method or after storage's destructor.
 */
class GlobalObjectStorage {
public:
    explicit GlobalObjectStorage(mem::InternalAllocatorPtr allocator, size_t maxSize, bool enableSizeCheck);

    ~GlobalObjectStorage();

    /// Check whether ref is a valid global reference or not.
    bool IsValidGlobalRef(const Reference *ref) const;

    /// Add object to the storage and return associated pointer with this object
    PANDA_PUBLIC_API Reference *Add(const ObjectHeader *object, Reference::ObjectType type) const;

    /// Get stored object associated with given reference. Reference should be returned on Add method before.
    ObjectHeader *Get(const Reference *reference) const
    {
        if (reference == nullptr) {
            return nullptr;
        }
        auto type = reference->GetType();
        reference = Reference::GetRefWithoutType(reference);
        AssertType(type);
        ObjectHeader *result = nullptr;
        if (type == Reference::ObjectType::GLOBAL) {
            result = globalStorage_->Get(reference);
        } else if (type == Reference::ObjectType::WEAK) {
            result = weakStorage_->Get(reference);
        } else {
            result = globalFixedStorage_->Get(reference);
        }
        return result;
    }

    uintptr_t GetAddressForRef(const Reference *reference) const
    {
        ASSERT(reference != nullptr);
        auto type = reference->GetType();
        reference = Reference::GetRefWithoutType(reference);
        AssertType(type);
        uintptr_t result = 0;
        if (type == Reference::ObjectType::GLOBAL) {
            result = globalStorage_->GetAddressForRef(reference);
        } else if (type == Reference::ObjectType::WEAK) {
            result = weakStorage_->GetAddressForRef(reference);
        } else {
            result = globalFixedStorage_->GetAddressForRef(reference);
        }
        return result;
    }

    /// Remove object from storage by given reference. Reference should be returned on Add method before.
    PANDA_PUBLIC_API void Remove(const Reference *reference);

    /// Get all objects from storage. Used by debugging.
    PandaVector<ObjectHeader *> GetAllObjects();

    void VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType);

    /// Update pointers to moved Objects in global storage.
    void UpdateAndSweep(const ReferenceUpdater &updater);

    void ClearUnmarkedWeakRefs(const GC *gc, const mem::GC::ReferenceClearPredicateT &pred);

    void ClearWeakRefs(const mem::GC::ReferenceClearPredicateT &pred);

    size_t GetSize();

    void Dump();

private:
    NO_COPY_SEMANTIC(GlobalObjectStorage);
    NO_MOVE_SEMANTIC(GlobalObjectStorage);

    class ArrayStorage;

    static constexpr size_t GLOBAL_REF_SIZE_WARNING_LINE = 20;

    mem::InternalAllocatorPtr allocator_;
    ArrayStorage *globalStorage_;
    ArrayStorage *globalFixedStorage_;
    ArrayStorage *weakStorage_;

    static void AssertType([[maybe_unused]] Reference::ObjectType type)
    {
        ASSERT(type == Reference::ObjectType::GLOBAL || type == Reference::ObjectType::GLOBAL_FIXED ||
               type == Reference::ObjectType::WEAK);
    }

    friend class ::ark::mem::test::ReferenceStorageTest;

    class ArrayStorage {
#ifndef NDEBUG
        // for better coverage of EnsureCapacity
        static constexpr size_t INITIAL_SIZE = 2;
#else
        static constexpr size_t INITIAL_SIZE = 128;
#endif  // NDEBUG
        static constexpr size_t FREE_INDEX_BIT = 0;
        static constexpr size_t BITS_FOR_TYPE = 2U;
        static constexpr size_t BITS_FOR_INDEX = 1U;
        static constexpr size_t ENSURE_CAPACITY_MULTIPLIER = 2;

        /**
         * There are 2 cases:
         * 1) When index is busy - then we store jobject in storage_ and 0 in the lowest bit (cause of alignment).
         * Reference* contains it's index shifted by 2 with reference-type in lowest bits which we return to user and
         * doesn't stores inside storage explicity.
         *
         * 2) When index if free - storage[index] stores next free index (shifted by 1) with lowest bit equals to 1
         */
        /*
        |-----------------------------------------------------|------------------|------------------|
        |      Case          |         Highest bits           | [1] lowest bit   | [0] lowest bit   |
        --------------------------------------------------------------------------------------------|
        | busy-index         |                                |                  |                  |
        | Reference* (index) |            index               |   0/1 (ref-type) |   0/1 (ref-type) |
        | storage[index]     |                           xxx                     |        0         |
        ---------------------|--------------------------------|------------------|-------------------
        | free-index         |                                                   |                  |
        | storage[index]     |                        xxx                        |        1         |
        ---------------------------------------------------------------------------------------------
        */

        PandaVector<uintptr_t> storage_ GUARDED_BY(mutex_) {};
        /// Index of first available block in list
        uintptr_t firstAvailableBlock_;
        /// How many blocks are available in current storage (can be increased if size less than max size)
        size_t blocksAvailable_;

        bool enableSizeCheck_;
        bool isFixed_;
        size_t maxSize_;

        mutable os::memory::RWLock mutex_;
        mem::InternalAllocatorPtr allocator_;

    public:
        explicit ArrayStorage(mem::InternalAllocatorPtr allocator, size_t maxSize, bool enableSizeCheck,
                              bool isFixed = false)
            : enableSizeCheck_(enableSizeCheck), isFixed_(isFixed), maxSize_(maxSize), allocator_(allocator)
        {
            ASSERT(maxSize < (std::numeric_limits<uintptr_t>::max() >> (BITS_FOR_TYPE)));

            blocksAvailable_ = isFixed ? maxSize : INITIAL_SIZE;
            firstAvailableBlock_ = 0;

            storage_.resize(blocksAvailable_);
            for (size_t i = 0; i < storage_.size() - 1; i++) {
                storage_[i] = EncodeNextIndex(i + 1);
            }
            storage_[storage_.size() - 1] = 0;
        }

        ~ArrayStorage() = default;

        NO_COPY_SEMANTIC(ArrayStorage);
        NO_MOVE_SEMANTIC(ArrayStorage);

        Reference *Add(const ObjectHeader *object)
        {
            ASSERT(object != nullptr);
            os::memory::WriteLockHolder lk(mutex_);

            if (blocksAvailable_ == 0) {
                if (storage_.size() * ENSURE_CAPACITY_MULTIPLIER <= maxSize_ && !isFixed_) {
                    EnsureCapacity();
                } else {
                    LOG(ERROR, GC) << "Global reference storage is full";
                    Dump();
                    return nullptr;
                }
            }
            ASSERT(blocksAvailable_ != 0);
            auto nextBlock = DecodeIndex(storage_[firstAvailableBlock_]);
            auto currentIndex = firstAvailableBlock_;
            AssertIndex(currentIndex);

            auto addr = reinterpret_cast<uintptr_t>(object);
            [[maybe_unused]] uintptr_t lastBit = BitField<uintptr_t, FREE_INDEX_BIT>::Get(addr);
            ASSERT(lastBit == 0);  // every object should be alignmented

            storage_[currentIndex] = addr;
            auto ref = IndexToReference(currentIndex);
            firstAvailableBlock_ = nextBlock;
            blocksAvailable_--;

            CheckAlmostOverflow();
            return ref;
        }

        void EnsureCapacity() REQUIRES(mutex_)
        {
            auto prevLength = storage_.size();
            size_t newLength = storage_.size() * ENSURE_CAPACITY_MULTIPLIER;
            blocksAvailable_ = firstAvailableBlock_ = prevLength;
            storage_.resize(newLength);
            for (size_t i = prevLength; i < newLength - 1; i++) {
                storage_[i] = EncodeNextIndex(i + 1);
            }
            storage_[storage_.size() - 1] = 0;
            LOG(DEBUG, GC) << "Increase global storage from: " << prevLength << " to: " << newLength;
        }

        void CheckAlmostOverflow() REQUIRES_SHARED(mutex_)
        {
            size_t nowSize = GetSize();
            if (enableSizeCheck_ && nowSize >= maxSize_ - GLOBAL_REF_SIZE_WARNING_LINE) {
                LOG(INFO, GC) << "Global reference storage almost overflow. now size: " << nowSize
                              << ", max size: " << maxSize_;
                // NOTE(xucheng): Dump global reference storage info now. May use Thread::Dump() when it can be used.
                Dump();
            }
        }

        ObjectHeader *Get(const Reference *ref) const
        {
            os::memory::ReadLockHolder lk(mutex_);
            auto index = ReferenceToIndex(ref);
            return reinterpret_cast<ObjectHeader *>(storage_[index]);
        }

        uintptr_t GetAddressForRef(const Reference *ref) const
        {
            os::memory::ReadLockHolder lk(mutex_);
            ASSERT(isFixed_);
            auto index = ReferenceToIndex(ref);
            return reinterpret_cast<uintptr_t>(&storage_[index]);
        }

        void Remove(const Reference *ref)
        {
            os::memory::WriteLockHolder lk(mutex_);
            auto index = ReferenceToIndex(ref);
            storage_[index] = EncodeNextIndex(firstAvailableBlock_);
            firstAvailableBlock_ = index;
            blocksAvailable_++;
        }

        void UpdateAndSweep(const ReferenceUpdater &updater)
        {
            os::memory::WriteLockHolder lk(mutex_);
            // NOLINTNEXTLINE(modernize-loop-convert)
            for (size_t index = 0; index < storage_.size(); index++) {
                auto ref = storage_[index];
                if (IsBusy(ref) && ref != 0) {
                    ObjectStatus status = updater(reinterpret_cast<ObjectHeader **>(&storage_[index]));
                    if (status == ObjectStatus::DEAD_OBJECT) {
                        LOG(DEBUG, GC) << "Clear weak-reference: " << storage_[index];
                        storage_[index] = 0;
                    }
                }
            }
        }

        void VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType)
        {
            os::memory::ReadLockHolder lk(mutex_);

            for (auto &ref : storage_) {
                if (IsBusy(ref)) {
                    auto obj = reinterpret_cast<ObjectHeader *>(ref);
                    if (obj != nullptr) {
                        LOG(DEBUG, GC) << " Found root from global storage: " << mem::GetDebugInfoAboutObject(obj);
                        gcRootVisitor({rootType, reinterpret_cast<ObjectHeader **>(&ref)});
                    }
                }
            }
        }

        void ClearWeakRefs(const mem::GC::ReferenceClearPredicateT &pred)
        {
            os::memory::WriteLockHolder lk(mutex_);

            for (auto &ref : storage_) {
                if (IsBusy(ref)) {
                    auto obj = reinterpret_cast<ObjectHeader *>(ref);
                    if (obj != nullptr && pred(obj)) {
                        LOG(DEBUG, GC) << "Clear weak-reference: " << obj;
                        ref = reinterpret_cast<uintptr_t>(nullptr);
                    }
                }
            }
        }

        PandaVector<ObjectHeader *> GetAllObjects()
        {
            auto objects = PandaVector<ObjectHeader *>(allocator_->Adapter());
            {
                os::memory::ReadLockHolder lk(mutex_);
                for (const auto &ref : storage_) {
                    // we don't return nulls on GetAllObjects
                    if (ref != 0 && IsBusy(ref)) {
                        auto obj = reinterpret_cast<ObjectHeader *>(ref);
                        objects.push_back(obj);
                    }
                }
            }
            return objects;
        }

        bool IsValidGlobalRef(const Reference *ref)
        {
            ASSERT(ref != nullptr);
            os::memory::ReadLockHolder lk(mutex_);
            uintptr_t index = ReferenceToIndex<false>(ref);
            if (index >= storage_.size()) {
                return false;
            }
            if (IsFreeIndex(index)) {
                return false;
            }
            return index < storage_.size();
        }

        void DumpWithLock()
        {
            os::memory::ReadLockHolder lk(mutex_);
            Dump();
        }

        void Dump() REQUIRES_SHARED(mutex_)
        {
            if (DfxController::IsInitialized() &&
                DfxController::GetOptionValue(DfxOptionHandler::REFERENCE_DUMP) != 1) {
                return;
            }
            static constexpr size_t DUMP_NUMS = 20;
            size_t num = 0;
            LOG(INFO, GC) << "Dump the last " << DUMP_NUMS << " global references info:";

            for (auto it = storage_.rbegin(); it != storage_.rend(); it++) {
                uintptr_t ref = *it;
                if (IsBusy(ref)) {
                    auto obj = reinterpret_cast<ObjectHeader *>(ref);
                    LOG(INFO, GC) << "\t Index: " << GetSize() - num << ", Global reference: " << std::hex << ref
                                  << ", Object: " << std::hex << obj
                                  << ", Class: " << obj->ClassAddr<ark::Class>()->GetName();
                    num++;
                    if (num == DUMP_NUMS || num > GetSize()) {
                        break;
                    }
                }
            }
        }

        size_t GetSize() const REQUIRES_SHARED(mutex_)
        {
            return storage_.size() - blocksAvailable_;
        }

        size_t GetSizeWithLock() const
        {
            os::memory::ReadLockHolder globalLock(mutex_);
            return GetSize();
        }

        bool IsFreeIndex(uintptr_t index) REQUIRES_SHARED(mutex_)
        {
            return IsFreeValue(storage_[index]);
        }

        bool IsFreeValue(uintptr_t value)
        {
            uintptr_t lastBit = BitField<uintptr_t, FREE_INDEX_BIT>::Get(value);
            return lastBit == 1;
        }

        bool IsBusy(uintptr_t value)
        {
            return !IsFreeValue(value);
        }

        static uintptr_t EncodeObjectIndex(uintptr_t index)
        {
            ASSERT(index < (std::numeric_limits<uintptr_t>::max() >> BITS_FOR_INDEX));
            return index << BITS_FOR_INDEX;
        }

        static uintptr_t EncodeNextIndex(uintptr_t index)
        {
            uintptr_t shiftedIndex = EncodeObjectIndex(index);
            BitField<uintptr_t, FREE_INDEX_BIT>::Set(1, &shiftedIndex);
            return shiftedIndex;
        }

        static uintptr_t DecodeIndex(uintptr_t index)
        {
            return index >> BITS_FOR_INDEX;
        }

        /**
         * We need to add 1 to not return nullptr to distinct it from situation when we couldn't create a reference.
         * Shift by 2 is needed because every Reference stores type in lowest 2 bits.
         */
        Reference *IndexToReference(uintptr_t encodedIndex) const REQUIRES_SHARED(mutex_)
        {
            AssertIndex(DecodeIndex(encodedIndex));
            return reinterpret_cast<Reference *>((encodedIndex + 1) << BITS_FOR_TYPE);
        }

        template <bool CHECK_ASSERT = true>
        uintptr_t ReferenceToIndex(const Reference *ref) const REQUIRES_SHARED(mutex_)
        {
            if (CHECK_ASSERT) {
                AssertIndex(ref);
            }
            return (reinterpret_cast<uintptr_t>(ref) >> BITS_FOR_TYPE) - 1;
        }

        void AssertIndex(const Reference *ref) const REQUIRES_SHARED(mutex_)
        {
            auto decodedIndex = (reinterpret_cast<uintptr_t>(ref) >> BITS_FOR_TYPE) - 1;
            AssertIndex(DecodeIndex(decodedIndex));
        }

        void AssertIndex([[maybe_unused]] uintptr_t index) const REQUIRES_SHARED(mutex_)
        {
            ASSERT(static_cast<uintptr_t>(index) < storage_.size());
        }

        // test usage only
        size_t GetVectorSize()
        {
            os::memory::ReadLockHolder lk(mutex_);
            return storage_.size();
        }

        friend class ::ark::mem::test::ReferenceStorageTest;
    };
};
}  // namespace ark::mem
#endif  // PANDA_GLOBAL_OBJECT_STORAGE_H
