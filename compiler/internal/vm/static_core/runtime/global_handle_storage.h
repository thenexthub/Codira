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

#ifndef PANDA_RUNTIME_GLOABL_HANDLE_STORAGE_H
#define PANDA_RUNTIME_GLOABL_HANDLE_STORAGE_H

#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {
using InternalAllocatorPtr = mem::AllocatorPtr<mem::AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL>;

template <class T>
class HandleScope;

// storage is the storage structure of the object pointer
template <typename T>
class GlobalHandleStorage {
public:
    static const int32_t GLOBAL_BLOCK_SIZE = 256;

    explicit GlobalHandleStorage(InternalAllocatorPtr allocator) : allocator_(allocator)
    {
        ASSERT(allocator_ != nullptr);
        globalNodes_ = allocator_->New<PandaVector<std::array<Node, GLOBAL_BLOCK_SIZE> *>>(allocator_->Adapter());
    };
    ~GlobalHandleStorage();

    NO_COPY_SEMANTIC(GlobalHandleStorage);
    NO_MOVE_SEMANTIC(GlobalHandleStorage);

    class Node {
    public:
        void PushNodeToFreeList();

        T GetObject() const
        {
            return obj_;
        }

        Node *GetNext() const
        {
            return next_;
        }

        void SetNext(Node *node)
        {
            next_ = node;
        }

        void SetObject(T obj)
        {
            obj_ = obj;
        }

        uintptr_t GetObjectAddress() const
        {
            return reinterpret_cast<uintptr_t>(&obj_);
        }

    private:
        T obj_;
        Node *next_;
    };

    inline uintptr_t NewGlobalHandle(T value);

    inline void DisposeGlobalHandle(uintptr_t addr);

    inline PandaVector<std::array<Node, GLOBAL_BLOCK_SIZE> *> *GetNodes() const
    {
        return globalNodes_;
    }

    inline int32_t GetCount() const
    {
        return count_;
    }

    inline void FreeHandles();

private:
    // TaggedType has been specialized for js, Other types are empty implementation
    inline void UpdateHeapObject([[maybe_unused]] const GCRootUpdater &gcRootUpdater) {}

    // TaggedType has been specialized for js, Other types are empty implementation
    inline void VisitGCRoots([[maybe_unused]] const GCRootVisitor &cb) {}

    // TaggedType has been specialized for js, Other types are empty implementation
    inline void DealUpdateObject([[maybe_unused]] std::array<Node, GLOBAL_BLOCK_SIZE> *block,
                                 [[maybe_unused]] size_t index, [[maybe_unused]] const GCRootUpdater &gcRootUpdater)
    {
    }
    // TaggedType has been specialized for js, Other types are empty implementation
    inline void DealVisitGCRoots([[maybe_unused]] std::array<Node, GLOBAL_BLOCK_SIZE> *block,
                                 [[maybe_unused]] size_t index, [[maybe_unused]] const GCRootVisitor &cb)
    {
    }

    inline void UpdateGCRootsInBlock(std::array<Node, GLOBAL_BLOCK_SIZE> *block, size_t size,
                                     const GCRootUpdater &gcRootUpdater);
    inline void VisitGCRootsInBlock(std::array<Node, GLOBAL_BLOCK_SIZE> *block, size_t size, const GCRootVisitor &cb);

    inline void FreeGlobalNodes()
    {
        for (auto block : *globalNodes_) {
            allocator_->Delete(block);
        }
    }

    PandaVector<std::array<Node, GLOBAL_BLOCK_SIZE> *> *globalNodes_ {nullptr};
    InternalAllocatorPtr allocator_;
    int32_t count_ {GLOBAL_BLOCK_SIZE};
    Node *freeList_ {nullptr};

    friend class ManagedThread;
};
}  // namespace ark
#endif  // PANDA_RUNTIME_GLOABL_HANDLE_STORAGE_H
