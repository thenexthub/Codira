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

#ifndef PANDA_RUNTIME_HANDLE_STORAGE_INL_H
#define PANDA_RUNTIME_HANDLE_STORAGE_INL_H

#include "libarkbase/mem/mem.h"
#include "runtime/handle_storage.h"
#include "runtime/mem/object_helpers.h"

namespace ark {
template <typename T>
inline uintptr_t HandleStorage<T>::GetNodeAddress(uint32_t index) const
{
    uint32_t id = index >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = index & NODE_BLOCK_SIZE_MASK;
    auto node = nodes_[id];
    auto location = &(*node)[offset];
    return reinterpret_cast<uintptr_t>(location);
}

template <typename T>
// CC-OFFNXT(G.FUD.06) solid logic
inline uintptr_t HandleStorage<T>::NewHandle(T value)
{
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    if (nodes_.size() <= nid) {
        auto n = allocator_->New<std::array<T, NODE_BLOCK_SIZE>>();
        nodes_.push_back(n);
    }
    auto node = nodes_[nid];
    auto loc = &(*node)[offset];
    *loc = value;
    lastIndex_++;
    return reinterpret_cast<uintptr_t>(loc);
}

template <typename T>
inline void HandleStorage<T>::FreeExtraNodes(uint32_t nid)
{
    for (size_t i = nid; i < nodes_.size(); ++i) {
        allocator_->Delete(nodes_[i]);
    }
    if (nid < nodes_.size()) {
        nodes_.erase(nodes_.begin() + nid, nodes_.end());
    }
}

template <typename T>
inline void HandleStorage<T>::FreeHandles(uint32_t beginIdx)
{
    lastIndex_ = beginIdx;
#ifndef NDEBUG
    ZapFreedHandles();
#endif
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    // reserve at least one block for perf.
    nid++;
    FreeExtraNodes(nid);
}

template <typename T>
void HandleStorage<T>::ZapFreedHandlesForNode(std::array<T, NODE_BLOCK_SIZE> *node, uint32_t start)
{
    for (uint32_t j = start; j < NODE_BLOCK_SIZE; ++j) {
        node->at(j) = reinterpret_cast<T>(static_cast<uint64_t>(0));
    }
}

template <typename T>
void HandleStorage<T>::ZapFreedHandles()
{
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    for (size_t i = nid; i < nodes_.size(); ++i) {
        auto node = nodes_.at(i);
        if (i != nid) {
            ZapFreedHandlesForNode(node);
        } else {
            ZapFreedHandlesForNode(node, offset);
        }
    }
}

template <>
inline void HandleStorage<coretypes::TaggedType>::UpdateHeapObjectForNode(
    std::array<coretypes::TaggedType, NODE_BLOCK_SIZE> *node, uint32_t size, const GCRootUpdater &gcRootUpdater)
{
    for (uint32_t j = 0; j < size; ++j) {
        coretypes::TaggedValue obj(node->at(j));
        if (!obj.IsHeapObject()) {
            continue;
        }
        ObjectHeader *objH = obj.GetHeapObject();
        if (gcRootUpdater(&objH)) {
            (*node)[j] = coretypes::TaggedValue(objH).GetRawData();
        }
    }
}

template <>
// CC-OFFNXT(G.FUD.06) solid logic
inline void HandleStorage<coretypes::TaggedType>::UpdateHeapObject(const GCRootUpdater &gcRootUpdater)
{
    if (lastIndex_ == 0) {
        return;
    }
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    for (uint32_t i = 0; i <= nid; ++i) {
        auto node = nodes_.at(i);
        uint32_t count = (i != nid) ? NODE_BLOCK_SIZE : offset;
        UpdateHeapObjectForNode(node, count, gcRootUpdater);
    }
}

template <>
inline void HandleStorage<coretypes::TaggedType>::VisitGCRootsForNode(
    std::array<coretypes::TaggedType, NODE_BLOCK_SIZE> *node, uint32_t size, const GCRootVisitor &cb)
{
    for (uint32_t j = 0; j < size; ++j) {
        coretypes::TaggedValue obj(node->at(j));
        if (obj.IsHeapObject()) {
            ObjectHeader *objH = obj.GetHeapObject();
            cb({mem::RootType::ROOT_THREAD, reinterpret_cast<ObjectPointerType *>(&obj)});
            (*node)[j] = coretypes::TaggedValue(objH).GetRawData();
        }
    }
}

template <>
// CC-OFFNXT(G.FUD.06) solid logic
inline void HandleStorage<coretypes::TaggedType>::VisitGCRoots([[maybe_unused]] const GCRootVisitor &cb)
{
    if (lastIndex_ == 0) {
        return;
    }
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    if (offset == 0) {
        nid -= 1;
        offset = NODE_BLOCK_SIZE;
    }
    for (uint32_t i = 0; i <= nid; ++i) {
        auto node = nodes_.at(i);
        uint32_t count = (i != nid) ? NODE_BLOCK_SIZE : offset;
        VisitGCRootsForNode(node, count, cb);
    }
}

template <>
inline void HandleStorage<ObjectHeader *>::UpdateHeapObjectForNode(std::array<ObjectHeader *, NODE_BLOCK_SIZE> *node,
                                                                   uint32_t size, const GCRootUpdater &gcRootUpdater)
{
    for (uint32_t j = 0; j < size; ++j) {
        gcRootUpdater(reinterpret_cast<ObjectHeader **>(&(*node)[j]));
    }
}

template <>
// CC-OFFNXT(G.FUD.06) solid logic
inline void HandleStorage<ObjectHeader *>::UpdateHeapObject(const GCRootUpdater &gcRootUpdater)
{
    if (lastIndex_ == 0) {
        return;
    }
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    if (offset == 0) {
        nid -= 1;
        offset = NODE_BLOCK_SIZE;
    }
    for (uint32_t i = 0; i <= nid; ++i) {
        auto node = nodes_.at(i);
        uint32_t count = (i != nid) ? NODE_BLOCK_SIZE : offset;
        UpdateHeapObjectForNode(node, count, gcRootUpdater);
    }
}

template <>
inline void HandleStorage<ObjectHeader *>::VisitGCRootsForNode(std::array<ObjectHeader *, NODE_BLOCK_SIZE> *node,
                                                               uint32_t size, const GCRootVisitor &cb)
{
    for (uint32_t j = 0; j < size; ++j) {
        cb({mem::RootType::ROOT_THREAD, &(node->at(j))});
    }
}

template <>
// CC-OFFNXT(G.FUD.06) solid logic
inline void HandleStorage<ObjectHeader *>::VisitGCRoots([[maybe_unused]] const GCRootVisitor &cb)
{
    if (lastIndex_ == 0) {
        return;
    }
    uint32_t nid = lastIndex_ >> NODE_BLOCK_SIZE_LOG2;
    uint32_t offset = lastIndex_ & NODE_BLOCK_SIZE_MASK;
    for (uint32_t i = 0; i <= nid; ++i) {
        auto node = nodes_.at(i);
        uint32_t count = (i != nid) ? NODE_BLOCK_SIZE : offset;
        VisitGCRootsForNode(node, count, cb);
    }
}

template class HandleStorage<coretypes::TaggedType>;
}  // namespace ark

#endif  // PANDA_RUNTIME_HANDLE_STORAGE_INL_H
