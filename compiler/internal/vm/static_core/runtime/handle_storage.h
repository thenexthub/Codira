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

#ifndef PANDA_RUNTIME_HANDLE_STORAGE_H
#define PANDA_RUNTIME_HANDLE_STORAGE_H

#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {
class LocalScope;
class EscapeLocalScope;

template <class T>
class HandleScope;

template <class T>
class EscapeHandleScope;

using InternalAllocatorPtr = mem::AllocatorPtr<mem::AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL>;

// storage is the storage structure of the object pointer
template <typename T>
class HandleStorage {
public:
    explicit HandleStorage(InternalAllocatorPtr allocator) : allocator_(allocator)
    {
        ASSERT(allocator_ != nullptr);
    };
    ~HandleStorage()
    {
        for (auto n : nodes_) {
            allocator_->Delete(n);
        }
        nodes_.clear();
    }

    NO_COPY_SEMANTIC(HandleStorage);
    NO_MOVE_SEMANTIC(HandleStorage);

private:
    static const uint32_t NODE_BLOCK_SIZE_LOG2 = 10;
    static const uint32_t NODE_BLOCK_SIZE = 1U << NODE_BLOCK_SIZE_LOG2;
    static const uint32_t NODE_BLOCK_SIZE_MASK = NODE_BLOCK_SIZE - 1;

    void ZapFreedHandles();

    void FreeExtraNodes(uint32_t nid);

    void ZapFreedHandlesForNode(std::array<T, NODE_BLOCK_SIZE> *node, uint32_t start = 0);

    void UpdateHeapObjectForNode(std::array<coretypes::TaggedType, NODE_BLOCK_SIZE> *node, uint32_t size,
                                 const GCRootUpdater &gcRootUpdater);

    void VisitGCRootsForNode(std::array<coretypes::TaggedType, NODE_BLOCK_SIZE> *node, uint32_t size,
                             const GCRootVisitor &cb);

    void UpdateHeapObjectForNode(std::array<ObjectHeader *, NODE_BLOCK_SIZE> *node, uint32_t size,
                                 const GCRootUpdater &gcRootUpdater);

    void VisitGCRootsForNode(std::array<ObjectHeader *, NODE_BLOCK_SIZE> *node, uint32_t size, const GCRootVisitor &cb);

    // TaggedType has been specialized for js, Other types are empty implementation
    void UpdateHeapObject([[maybe_unused]] const GCRootUpdater &gcRootUpdater) {}

    // TaggedType has been specialized for js, Other types are empty implementation
    void VisitGCRoots([[maybe_unused]] const GCRootVisitor &cb) {}

    uintptr_t NewHandle(T value = 0);

    void FreeHandles(uint32_t beginIdx);

    uintptr_t GetNodeAddress(uint32_t index) const;

    uint32_t lastIndex_ {0};
    PandaVector<std::array<T, NODE_BLOCK_SIZE> *> nodes_;
    InternalAllocatorPtr allocator_ {nullptr};
    friend class ManagedThread;
    friend class HandleScope<T>;
    friend class LocalScope;
    friend class EscapeLocalScope;
    friend class EscapeHandleScope<T>;
};
}  // namespace ark
#endif  // PANDA_RUNTIME_HANDLE_STORAGE_H
