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

#include "runtime/mem/refstorage/reference_storage.h"

#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/dfx.h"
#include "runtime/include/thread.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/include/stack_walker-inl.h"
#include "libarkbase/utils/bit_utils.h"

namespace ark::mem {

// NOTE(alovkov): remove check for null, create managed thread in test instead of std::thread
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_THREAD_STATE() \
    ASSERT(MTManagedThread::GetCurrent() == nullptr || !MTManagedThread::GetCurrent()->IsInNativeCode())

ReferenceStorage::ReferenceStorage(GlobalObjectStorage *globalStorage, mem::InternalAllocatorPtr allocator,
                                   bool refCheckValidate)
    : globalStorage_(globalStorage), internalAllocator_(allocator), refCheckValidate_(refCheckValidate)
{
}

ReferenceStorage::~ReferenceStorage()
{
    if (frameAllocator_ != nullptr) {
        internalAllocator_->Delete(frameAllocator_);
    }
    if (localStorage_ != nullptr) {
        internalAllocator_->Delete(localStorage_);
    }
}

bool ReferenceStorage::Init()
{
    if (localStorage_ != nullptr || frameAllocator_ != nullptr || blocksCount_ != 0) {
        return false;
    }
    localStorage_ = internalAllocator_->New<PandaVector<RefBlock *>>(internalAllocator_->Adapter());
    if (localStorage_ == nullptr) {
        return false;
    }
    frameAllocator_ = internalAllocator_->New<StorageFrameAllocator>();
    if (frameAllocator_ == nullptr) {
        return false;
    }
    // main frame should always be created
    auto *firstBlock = CreateBlock();
    if (firstBlock == nullptr) {
        return false;
    }
    firstBlock->Reset();
    blocksCount_ = 1;
    localStorage_->push_back(firstBlock);
    return true;
}

void ReferenceStorage::CleanUp()
{
    if (cachedBlock_ != nullptr) {
        RemoveBlock(std::exchange(cachedBlock_, nullptr));
    }
    for (auto *refBlockHead : *localStorage_) {
        while (refBlockHead != nullptr) {
            RemoveBlock(std::exchange(refBlockHead, refBlockHead->GetPrev()));
        }
    }
    localStorage_->clear();
    ASSERT(blocksCount_ == 0);
}

void ReferenceStorage::ReInitialize()
{
    ASSERT(localStorage_ != nullptr);
    ASSERT(frameAllocator_ != nullptr);
    ASSERT(blocksCount_ == 0);

    auto *firstBlock = CreateBlock();
    ASSERT(firstBlock != nullptr);
    ASSERT(blocksCount_ == 1);

    firstBlock->Reset();
    localStorage_->push_back(firstBlock);
}

bool ReferenceStorage::IsValidRef(const Reference *ref)
{
    ASSERT(ref != nullptr);
    auto type = Reference::GetType(ref);

    bool res = false;
    if (type == mem::Reference::ObjectType::STACK) {
        res = StackReferenceCheck(ref);
    } else if (type == mem::Reference::ObjectType::GLOBAL || type == mem::Reference::ObjectType::WEAK) {
        // global-storage should accept ref with type
        res = globalStorage_->IsValidGlobalRef(ref);
    } else {
        auto refWithoutType = Reference::GetRefWithoutType(ref);
        // NOTE(alovkov): can be optimized with mmap + make additional checks that we really have ref in slots,
        // Issue 3645
        res = frameAllocator_->Contains(reinterpret_cast<void *>(refWithoutType));
    }
    return res;
}

Reference::ObjectType ReferenceStorage::GetObjectType(const Reference *ref)
{
    return ref->GetType();
}

// NOLINTNEXTLINE(readability-function-size)
Reference *ReferenceStorage::NewRef(const ObjectHeader *object, Reference::ObjectType type)
{
    ASSERT(type != Reference::ObjectType::STACK);
    ASSERT_THREAD_STATE();
    if (object == nullptr) {
        return nullptr;
    }
    ValidateObject(nullptr, object);
    Reference *ref = nullptr;
    if (type == Reference::ObjectType::GLOBAL || type == Reference::ObjectType::WEAK) {
        ref = static_cast<Reference *>(globalStorage_->Add(object, type));
    } else {
        auto *lastBlock = localStorage_->back();
        ASSERT(lastBlock != nullptr);

        RefBlock *curBlock = nullptr;
        if (lastBlock->IsFull()) {
            curBlock = CreateBlock();
            if (curBlock == nullptr) {
                // NOTE(alovkov): make free-list of holes in all blocks. O(n) operations, but it will be done only once
                LOG(ERROR, GC) << "Can't allocate local ref for object: " << object
                               << ", cls: " << object->ClassAddr<ark::Class>()->GetName()
                               << " with type: " << static_cast<int>(type);
                DumpLocalRef();
                return nullptr;
            }
            curBlock->Reset(lastBlock);
            (*localStorage_)[localStorage_->size() - 1] = curBlock;
        } else {
            curBlock = lastBlock;
        }
        ref = curBlock->AddRef(object, type);
    }
    LOG(DEBUG, GC) << "Add reference to object: " << object << " type: " << static_cast<int>(type) << " ref: " << ref;
    return ref;
}

void ReferenceStorage::RemoveRef(const Reference *ref)
{
    if (ref == nullptr) {
        return;
    }

    if (refCheckValidate_) {
        if (UNLIKELY(!IsValidRef(ref))) {
            // Undefined behavior, we just print warning here.
            LOG(WARNING, GC) << "Try to remove not existed ref: " << ref;
            return;
        }
    }
    Reference::ObjectType objectType = ref->GetType();
    if (objectType == Reference::ObjectType::GLOBAL || objectType == Reference::ObjectType::WEAK) {
        // When the global or weak global ref is created by another thread, we can't suppose current thread is in
        // MANAGED_CODE state.
        LOG(DEBUG, GC) << "Remove global reference: " << ref << " obj: " << globalStorage_->Get(ref);
        globalStorage_->Remove(ref);
    } else if (objectType == Reference::ObjectType::LOCAL) {
        ASSERT_THREAD_STATE();
        auto addr = ToUintPtr(ref);
        auto blockAddr = (addr >> BLOCK_ALIGNMENT) << BLOCK_ALIGNMENT;
        auto *block = reinterpret_cast<RefBlock *>(blockAddr);

        LOG(DEBUG, GC) << "Remove local reference: " << ref << " obj: " << FindLocalObject(ref);
        block->Remove(ref);
    } else if (objectType == Reference::ObjectType::STACK) {
        LOG(ERROR, GC) << "Cannot remove stack type: " << ref;
    } else {
        LOG(FATAL, GC) << "Unknown reference type: " << ref;
    }
}

ObjectHeader *ReferenceStorage::GetObject(const Reference *ref)
{
    if (UNLIKELY(ref == nullptr)) {
        return nullptr;
    }

    if (refCheckValidate_) {
        if (UNLIKELY(!IsValidRef(ref))) {
            // Undefined behavior, we just print warning here.
            LOG(WARNING, GC) << "Try to GetObject from a not existed ref: " << ref;
            return nullptr;
        }
    }
    Reference::ObjectType objectType = ref->GetType();
    switch (objectType) {
        case Reference::ObjectType::GLOBAL:
        case Reference::ObjectType::WEAK: {
            ObjectHeader *obj = globalStorage_->Get(ref);
#ifndef NDEBUG
            // only weakly reachable objects can be null in storage
            if (objectType == mem::Reference::ObjectType::GLOBAL) {
                ASSERT(obj != nullptr);
            }
#endif
            return obj;
        }
        case Reference::ObjectType::STACK: {
            // In current scheme object passed in 64-bit argument
            // But compiler may store 32-bit and trash in hi-part
            // That's why need cut object pointer
            return reinterpret_cast<ObjectHeader *>(
                (*reinterpret_cast<ObjectPointerType *>(Reference::GetRefWithoutType(ref))));
        }
        case Reference::ObjectType::LOCAL: {
            ObjectHeader *obj = FindLocalObject(ref);
            ASSERT(obj != nullptr);
#ifndef NDEBUG
            /*
             * classes are not movable objects, so they can be read from storage in native code, but general objects are
             * not
             */
            auto baseCls = obj->ClassAddr<BaseClass>();
            ASSERT(baseCls != nullptr);
            if (!baseCls->IsDynamicClass()) {
                auto cls = static_cast<Class *>(baseCls);
                if (!cls->IsClassClass()) {
                    ASSERT_THREAD_STATE();
                }
            }
#endif
            return obj;
        }
        default: {
            LOG(FATAL, RUNTIME) << "Unknown reference: " << ref << " type: " << static_cast<int>(objectType);
        }
    }
    return nullptr;
}

bool ReferenceStorage::PushLocalFrame(uint32_t capacity)
{
    ASSERT_THREAD_STATE();
    size_t needBlocks = (static_cast<uint64_t>(capacity) + RefBlock::REFS_IN_BLOCK - 1) / RefBlock::REFS_IN_BLOCK;

    size_t blocksFree = MAX_STORAGE_BLOCK_COUNT - blocksCount_;
    if (needBlocks > blocksFree) {
        LOG(ERROR, GC) << "Free size of local reference storage is less than capacity: " << capacity
                       << " blocks_count_: " << blocksCount_ << " need_blocks: " << needBlocks
                       << " blocks_free: " << blocksFree;
        return false;
    }
    auto *newBlock = CreateBlock();
    if (newBlock == nullptr) {
        LOG(FATAL, GC) << "Can't allocate new frame";
        UNREACHABLE();
    }
    newBlock->Reset();
    localStorage_->push_back(newBlock);
    return true;
}

Reference *ReferenceStorage::PopLocalFrame(const Reference *result)
{
    ASSERT_THREAD_STATE();

    ObjectHeader *obj;
    if (result != nullptr) {
        obj = GetObject(result);
    } else {
        obj = nullptr;
    }

    if (cachedBlock_ != nullptr) {
        RemoveBlock(cachedBlock_);
        cachedBlock_ = nullptr;
    }

    // We should add a log which refs are deleted under debug
    auto *lastBlock = localStorage_->back();
    auto isFirst = localStorage_->size() == 1;
    while (lastBlock != nullptr) {
        auto *prev = lastBlock->GetPrev();
        if (prev == nullptr && isFirst) {
            // it's the first block, which we don't delete
            break;
        }
        // cache the last block for ping-pong effect
        if (prev == nullptr) {
            if (cachedBlock_ == nullptr) {
                cachedBlock_ = lastBlock;
                break;
            }
        }
        RemoveBlock(lastBlock);
        lastBlock = prev;
    }

    if (obj == nullptr) {
        localStorage_->pop_back();
        return nullptr;
    }

    Reference::ObjectType type = result->GetType();
    localStorage_->pop_back();
    return NewRef(obj, type);
}

bool ReferenceStorage::EnsureLocalCapacity(uint32_t capacity)
{
    size_t needBlocks = (static_cast<uint64_t>(capacity) + RefBlock::REFS_IN_BLOCK - 1) / RefBlock::REFS_IN_BLOCK;
    size_t blocksFreed = MAX_STORAGE_BLOCK_COUNT - blocksCount_;
    if (needBlocks > blocksFreed) {
        LOG(ERROR, GC) << "Can't store size: " << capacity << " in local references";
        return false;
    }
    return true;
}

ObjectHeader *ReferenceStorage::FindLocalObject(const Reference *ref)
{
    ref = Reference::GetRefWithoutType(ref);
    ObjectPointer<ObjectHeader> objPointer = *(reinterpret_cast<const ObjectPointer<ObjectHeader> *>(ref));
    return objPointer;
}

PandaVector<ObjectHeader *> ReferenceStorage::GetAllObjects()
{
    auto objects = globalStorage_->GetAllObjects();
    for (const auto &currentFrame : *localStorage_) {
        auto lastBlock = currentFrame;
        const PandaVector<mem::Reference *> &refs = lastBlock->GetAllReferencesInFrame();
        for (const auto &ref : refs) {
            ObjectHeader *obj = FindLocalObject(ref);
            objects.push_back(reinterpret_cast<ObjectHeader *>(obj));
        }
    }
    return objects;
}

void ReferenceStorage::VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType)
{
    for (const auto &frame : *localStorage_) {
        frame->VisitObjects(gcRootVisitor, rootType);
    }
}

void ReferenceStorage::DumpLocalRefClasses()
{
    PandaMap<PandaString, int> classesInfo;

    for (const auto &frame : *localStorage_) {
        auto lastBlock = frame;
        auto refs = lastBlock->GetAllReferencesInFrame();
        for (const auto &ref : refs) {
            ObjectHeader *obj = FindLocalObject(ref);
            PandaString clsName = ConvertToString(obj->ClassAddr<ark::Class>()->GetName());
            classesInfo[clsName]++;
        }
    }
    using InfoPair = std::pair<PandaString, int>;
    PandaVector<InfoPair> infoVec(classesInfo.begin(), classesInfo.end());
    size_t size = std::min(MAX_DUMP_LOCAL_NUMS, infoVec.size());
    std::partial_sort(infoVec.begin(), infoVec.begin() + size, infoVec.end(),
                      [](const InfoPair &lhs, const InfoPair &rhs) { return lhs.second < rhs.second; });
    LOG(ERROR, GC) << "The top " << size << " classes of local references are:";
    for (size_t i = 0; i < size; i++) {
        LOG(ERROR, GC) << "\t" << infoVec[i].first << ": " << infoVec[i].second;
    }
}

void ReferenceStorage::DumpLocalRef()
{
    if (DfxController::IsInitialized() && DfxController::GetOptionValue(DfxOptionHandler::REFERENCE_DUMP) != 1) {
        return;
    }
    LOG(ERROR, GC) << "--- local reference storage dump ---";
    LOG(ERROR, GC) << "Local reference storage addr: " << &localStorage_;
    LOG(ERROR, GC) << "Dump the last several local references info(max " << MAX_DUMP_LOCAL_NUMS << "):";
    size_t nDump = 0;

    for (auto it = localStorage_->rbegin(); it != localStorage_->rend(); ++it) {
        auto *frame = *it;
        auto refs = frame->GetAllReferencesInFrame();
        for (const auto &ref : refs) {
            ObjectHeader *res = FindLocalObject(ref);
            PandaString clsName = ConvertToString(res->ClassAddr<ark::Class>()->GetName());
            LOG(ERROR, GC) << "\t local reference: " << ref << ", object: " << res << ", cls: " << clsName;
            nDump++;
            if (nDump == MAX_DUMP_LOCAL_NUMS) {
                DumpLocalRefClasses();
                LOG(ERROR, GC) << "---";
                LOG(ERROR, GC) << "Storage dumped maximum number of references";
                return;
            }
        }
    }
}

RefBlock *ReferenceStorage::CreateBlock()
{
    if (blocksCount_ == MAX_STORAGE_BLOCK_COUNT) {
        return nullptr;
    }

    if (cachedBlock_ != nullptr) {
        RefBlock *newBlock = cachedBlock_;
        cachedBlock_ = nullptr;
        return newBlock;
    }

    blocksCount_++;
    return static_cast<RefBlock *>(frameAllocator_->Alloc(BLOCK_SIZE));
}

void ReferenceStorage::RemoveBlock(RefBlock *block)
{
    frameAllocator_->Free(block);
    blocksCount_--;
}

void ReferenceStorage::RemoveAllLocalRefs()
{
    ASSERT_THREAD_STATE();

    for (const auto &frame : *localStorage_) {
        auto lastBlock = frame;
        auto refs = lastBlock->GetAllReferencesInFrame();
        for (const auto &ref : refs) {
            lastBlock->Remove(ref);
        }
    }
}

size_t ReferenceStorage::GetGlobalObjectStorageSize()
{
    return globalStorage_->GetSize();
}

size_t ReferenceStorage::GetLocalObjectStorageSize()
{
    size_t size = 0;
    for (const auto &block : *localStorage_) {
        auto *currentBlock = block;
        size += currentBlock->GetAllReferencesInFrame().size();
    }
    return size;
}

void ReferenceStorage::SetRefCheckValidate(bool refCheckValidate)
{
    refCheckValidate_ = refCheckValidate;
}

bool ReferenceStorage::StackReferenceCheck(const Reference *stackRefInput)
{
    ASSERT(stackRefInput->IsStack());
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);

    for (auto pframe = StackWalker::Create(thread); pframe.HasFrame(); pframe.NextFrame()) {
        if (!pframe.IsCFrame()) {
            return false;
        }

        auto cframe = pframe.GetCFrame();
        if (!cframe.IsNative()) {
            return false;
        }

        bool res = false;
        pframe.IterateObjectsWithInfo([&cframe, &stackRefInput, &res](auto &regInfo, [[maybe_unused]] auto &vreg) {
            auto slotTypeRef = cframe.GetValuePtrFromSlot(regInfo.GetValue());
            auto objectHeader = bit_cast<ObjectHeader **, const ark::CFrame::SlotType *>(slotTypeRef);
            auto stackRef = NewStackRef(objectHeader);
            if (stackRef == stackRefInput) {
                res = true;
                return false;
            }
            return true;
        });

        if (res) {
            return true;
        }
    }
    return false;
}

}  // namespace ark::mem
