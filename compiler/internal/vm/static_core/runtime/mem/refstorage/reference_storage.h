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
#ifndef PANDA_RUNTIME_MEM_REFERENCES_STORAGE_H
#define PANDA_RUNTIME_MEM_REFERENCES_STORAGE_H

#include "runtime/include/object_header.h"
#include "runtime/mem/frame_allocator.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/internal_allocator.h"
#include "ref_block.h"

namespace ark {
class ObjectHeader;
namespace mem {
class Reference;
namespace test {
class ReferenceStorageTest;
}  // namespace test
}  // namespace mem
}  // namespace ark

namespace ark::mem {

/// Storage stores all References for proper interaction with GC.
class ReferenceStorage {
public:
    static_assert(offsetof(RefBlock, refs_) == 0);

    explicit ReferenceStorage(GlobalObjectStorage *globalStorage, mem::InternalAllocatorPtr allocator,
                              bool refCheckValidate);

    ~ReferenceStorage();

    bool Init();

    /// This method cleans up the internals of the reference storage and prepares it for caching
    void CleanUp();

    /**
     * This method prepares a cached reference storage instance for reuse.
     * Implies that the CleanUp() method was called before caching.
     */
    void ReInitialize();

    PANDA_PUBLIC_API static Reference::ObjectType GetObjectType(const Reference *ref);

    [[nodiscard]] static Reference *NewStackRef(ObjectHeader **objectPtr)
    {
        ASSERT(objectPtr != nullptr);
        if (*objectPtr == nullptr) {
            return nullptr;
        }
        return Reference::Create(ToUintPtr(objectPtr), Reference::ObjectType::STACK);
    }

    [[nodiscard]] PANDA_PUBLIC_API Reference *NewRef(const ObjectHeader *object, Reference::ObjectType objectType);

    void RemoveRef(const Reference *ref);

    [[nodiscard]] PANDA_PUBLIC_API ObjectHeader *GetObject(const Reference *ref);

    /**
     * Creates a new frame with at least given number of local references which can be created in this frame.
     *
     * @param capacity minimum number of local references in the frame.
     * @return true if frame was successful allocated, false otherwise.
     */
    PANDA_PUBLIC_API bool PushLocalFrame(uint32_t capacity);

    /**
     * Pops the last frame, frees all local references in this frame and moves given reference to the previous frame and
     * return it's new reference. Should be NULL if you don't want to return any value to previous frame.
     *
     * @param result reference which you want to return in the previous frame.
     * @return new reference in the previous frame for given reference.
     */
    Reference *PopLocalFrame(const Reference *result);

    /**
     * Ensure that capacity in current frame can contain at least `size` references.
     * @param capacity minimum number of references for this frame
     * @return true if current frame can store at least `size` references, false otherwise.
     */
    bool EnsureLocalCapacity(uint32_t capacity);

    /// Get all objects in global & local storage. Use for debugging only
    PandaVector<ObjectHeader *> GetAllObjects();

    void VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType);

    /// Dump the last several local references info(max MAX_DUMP_LOCAL_NUMS).
    void DumpLocalRef();

    /// Dump the top MAX_DUMP_LOCAL_NUMS(if exists) classes of local references.
    void DumpLocalRefClasses();
    bool IsValidRef(const Reference *ref);
    void SetRefCheckValidate(bool refCheckValidate);

private:
    NO_COPY_SEMANTIC(ReferenceStorage);
    NO_MOVE_SEMANTIC(ReferenceStorage);

    ObjectHeader *FindLocalObject(const Reference *ref);

    RefBlock *CreateBlock();

    void RemoveBlock(RefBlock *block);

    static constexpr size_t MAX_DUMP_LOCAL_NUMS = 10;

    static constexpr Alignment BLOCK_ALIGNMENT = Alignment::LOG_ALIGN_8;
    static constexpr size_t BLOCK_SIZE = sizeof(RefBlock);

    static_assert(GetAlignmentInBytes(BLOCK_ALIGNMENT) >= BLOCK_SIZE);
    static_assert(GetAlignmentInBytes(static_cast<Alignment>(BLOCK_ALIGNMENT - 1)) <= BLOCK_SIZE);

    static constexpr size_t MAX_STORAGE_SIZE = 128_MB;
    static constexpr size_t MAX_STORAGE_BLOCK_COUNT = MAX_STORAGE_SIZE / BLOCK_SIZE;

    using StorageFrameAllocator = mem::FrameAllocator<BLOCK_ALIGNMENT, false>;

    GlobalObjectStorage *globalStorage_;
    mem::InternalAllocatorPtr internalAllocator_;
    PandaVector<RefBlock *> *localStorage_ {nullptr};
    StorageFrameAllocator *frameAllocator_ {nullptr};
    size_t blocksCount_ {0};
    // NOTE(alovkov): remove it when storage will be working over mmap
    RefBlock *cachedBlock_ {nullptr};

    bool refCheckValidate_;
    // private methods for test purpose only
    size_t GetGlobalObjectStorageSize();

    size_t GetLocalObjectStorageSize();

    void RemoveAllLocalRefs();

    bool StackReferenceCheck(const Reference *stackRef);

    friend class ark::mem::test::ReferenceStorageTest;
};

/**
 * Handle the reference of object that might be moved by gc, note that
 * it should be only used in Managed code(with ScopedObjectFix).
 */
class ReferenceHandle {
public:
    ~ReferenceHandle() = default;

    template <typename T>
    ReferenceHandle(const ReferenceHandle &rhs, T *object, Reference::ObjectType type = Reference::ObjectType::LOCAL)
        : rs_(rhs.rs_), ref_(rs_->NewRef(reinterpret_cast<ObjectHeader *>(object), type))
    {
        ASSERT(ref_ != nullptr);
    }

    template <typename T>
    ReferenceHandle(ReferenceStorage *rs, T *object, Reference::ObjectType type = Reference::ObjectType::LOCAL)
        : rs_(rs), ref_(rs_->NewRef(reinterpret_cast<ObjectHeader *>(object), type))
    {
        ASSERT(ref_ != nullptr);
    }

    template <typename T>
    T *GetObject()
    {
        return reinterpret_cast<T *>(rs_->GetObject(ref_));
    }

    template <typename T>
    Reference *NewRef(T *object, bool releaseOld = true, Reference::ObjectType type = Reference::ObjectType::LOCAL)
    {
        if (releaseOld && ref_ != nullptr) {
            rs_->RemoveRef(ref_);
        }
        ref_ = rs_->NewRef(reinterpret_cast<ObjectHeader *>(object), type);
        return ref_;
    }

    /**
     * Remove a reference explicitly,
     * suggest not doing this unless the ReferenceStorage will be out of capacity,
     * and the reference is created in caller scope and not used by any other place
     */
    void RemoveRef()
    {
        rs_->RemoveRef(ref_);
        ref_ = nullptr;
    }

private:
    ReferenceStorage *rs_;
    Reference *ref_;
    NO_COPY_SEMANTIC(ReferenceHandle);
    NO_MOVE_SEMANTIC(ReferenceHandle);
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_REFERENCES_STORAGE_H
