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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_TREAP_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_TREAP_H

#include <cstddef>
#include <cstdint>

#include "common_components/heap/allocator/local_deque.h"
#include "common_components/log/log.h"

#ifndef NDEBUG
#define CTREE_ASSERT(cond, msg) ASSERT_LOGF(cond, msg)
#define CTREE_CHECK_PARENT_AND_LCHILD(n) CheckParentAndLeftChild(n)
#define CTREE_CHECK_PARENT_AND_RCHILD(n) CheckParentAndRightChild(n)
#else
#define CTREE_ASSERT(cond, msg) (void(0))
#define CTREE_CHECK_PARENT_AND_LCHILD(n) (void(0))
#define CTREE_CHECK_PARENT_AND_RCHILD(n) (void(0))
#endif

// This is an implementation of a Cartesian tree.
// This can be used in arbitrary-sized, free-list allocation algorithm.
// The use of this tree and the algorithm is inspired by
// R. Jones, A. Hosking, E. Moss. The garbage collection handbook:
// the art of automatic memory management. Chapman and Hall/CRC, 2016.
// This data structure doesn't guarantee the multi-thread safety, so the external invoker should take some
// policy to avoid competition problems.
namespace common {
class Treap {
public:
    Treap() = default;
    ~Treap() = default;

    void Init(size_t memoryBlockCount)
    {
        // at most we need this many nodes
        size_t nodeCount = (memoryBlockCount >> 1) + 1;
        // calculate how much we need for native allocation
        // we might need some extra space for some temporaries, so set aside another 7 slots
        size_t nativeSize = (nodeCount + 7) * AllocUtilRndUp(sizeof(TreapNode), alignof(TreapNode));
        nodeAllocator_.Init(ALLOCUTIL_PAGE_RND_UP(nativeSize));
        // calculate how much we need for the deque temporary
        size_t dequeSize = nodeCount * sizeof(TreapNode*);
        sud_.Init(ALLOCUTIL_PAGE_RND_UP(dequeSize));
        traversalSud_.Init(sud_.GetMemMap());
    }

    void Fini() noexcept
    {
        ClearTree();
        sud_.Fini();
        nodeAllocator_.Fini();
    }

    void ClearTree()
    {
        Treap::Iterator it(*this);
        TreapNode* node = it.Next();
        while (node != nullptr) {
            DeleteNode(node);
            node = it.Next();
        }

        root_ = nullptr;
        totalCount_ = 0;
    }

    uint32_t GetTotalCount() const { return totalCount_; }

    void IncTotalCount(uint32_t cnt) { totalCount_ += cnt; }

    void DecTotalCount(uint32_t cnt)
    {
        if (totalCount_ < cnt) { //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "Treap::DecTotalCount() Should not execute here, abort.";
            UNREACHABLE_CC();
        }
        totalCount_ -= cnt;
    }

    // insert a node to the tree, if we find connecting nodes, we merge them
    // (the non-merging insertion is not allowed)
    // true when insertion succeeded, false otherwise
    // if [idx, idx + num) clashes with existing node, it fails
    // if num is 0U, it always fails
    bool MergeInsert(uint32_t idx, uint32_t num, bool refreshRegionDesc)
    {
        if (root_ == nullptr) {
            root_ = new (nodeAllocator_.Allocate()) TreapNode(idx, num, refreshRegionDesc);
            CTREE_ASSERT(root_ != nullptr, "fail to allocate a new node");
            IncTotalCount(num);
            return true;
        }

        if (num == 0) {
            return false;
        }

        return MergeInsertInternal(idx, num, refreshRegionDesc);
    }

    // find a node with a count of at least num, store its index into a
    // split/remove this node if found
    // return false if nothing is found or num is 0U
    bool TakeUnits(uint32_t num, uint32_t& idx, bool refreshRegionDesc = true)
    {
        if (root_ == nullptr || num == 0) {
            return false;
        }

        return TakeUnitsImpl(num, idx, refreshRegionDesc);
    }

    struct TreapNode {
        TreapNode(uint32_t idx, uint32_t num, bool refreshRegionDesc) : l(nullptr), r(nullptr), index_(idx), count_(num)
        {
            if (refreshRegionDesc) {
                RefreshFreeRegionDesc();
            }
        }

        ~TreapNode()
        {
            l = nullptr;
            r = nullptr;
        }

        inline uint32_t GetIndex() const { return index_; }

        inline uint32_t GetCount() const { return count_; }

        inline void IncCount(uint32_t num, bool refreshRegionDesc)
        {
            count_ += num;
            if (refreshRegionDesc && count_ > 0) {
                RefreshFreeRegionDesc();
            }
        }

        inline void ClearCount() { count_ = 0; }

        inline void UpdateNode(uint32_t idx, uint32_t cnt, bool refreshRegionDesc)
        {
            index_ = idx;
            count_ = cnt;
            if (refreshRegionDesc && cnt > 0) {
                // GetNextNeighborRegion in compact gc expects free-region metadata is always up-to-date.
                // otherwise we can ignore refreshRegionDesc.
                RefreshFreeRegionDesc();
            }
        }

        inline bool IsProperNode() const
        {
            uint32_t leftCount = l == nullptr ? 0 : l->GetCount();
            uint32_t rightCount = r == nullptr ? 0 : r->GetCount();
            return (count_ >= leftCount && count_ >= rightCount);
        }

        void RefreshFreeRegionDesc();
        void ReleaseMemory();

        TreapNode* l;
        TreapNode* r;

    private:
        uint32_t index_;
        uint32_t count_;
    };

    // Iterator is defined specifically for releasing physical pages.
    // It provides a "backwards" level-order traversal with right child node visited first
    // rather than left child node first. This behaviour avoids that physical pages for regions
    // are released and again committed for near future allocation.
    class Iterator {
    public:
        explicit Iterator(Treap& tree) : localQueue_(tree.traversalSud_)
        {
            if (tree.root_ != nullptr) {
                localQueue_.Push(tree.root_);
            }
        }

        ~Iterator() = default;

        inline TreapNode* Next()
        {
            if (localQueue_.Empty()) {
                return nullptr;
            }
            TreapNode* front = localQueue_.Front();
            if (front->r != nullptr) {
                localQueue_.Push(front->r);
            }
            if (front->l != nullptr) {
                localQueue_.Push(front->l);
            }
            localQueue_.PopFront();
            return front;
        }

    private:
        LocalDeque<TreapNode*> localQueue_;
    };

    // very much like copy iterator in c++.
    // traverse nodes by lvalue-reference.
    class CopyIterator {
    public:
        explicit CopyIterator(Treap& tree) : localQueue_(tree.sud_)
        {
            if (tree.root_ != nullptr) {
                localQueue_.Push(&tree.root_);
            }
        }

        ~CopyIterator() = default;

        inline TreapNode** Next()
        {
            if (localQueue_.Empty()) {
                return nullptr;
            }
            TreapNode** topElement = localQueue_.Front();
            TreapNode* node = *topElement;
            if (node->r != nullptr) {
                localQueue_.Push(&node->r);
            }
            if (node->l != nullptr) {
                localQueue_.Push(&node->l);
            }
            localQueue_.PopFront();
            return topElement;
        }

    private:
        LocalDeque<TreapNode**> localQueue_;
    };

    inline bool Empty() const { return root_ == nullptr; }
    inline const TreapNode* RootNode() const { return root_; }

    // root node records the largest block of memory.
    void ReleaseRootNode()
    {
        root_->ReleaseMemory();
        RemoveNode(root_);
    }

    using RTAllocator = RTAllocatorT<sizeof(TreapNode), alignof(TreapNode)>;

#ifndef NDEBUG
    void DumpTree(const char* msg) const;
#endif

private:
    uint32_t totalCount_ = 0u; // sum of all node counts.
    TreapNode* root_ = nullptr;
    SingleUseDeque<TreapNode**> sud_;
    SingleUseDeque<TreapNode*> traversalSud_;
    RTAllocator nodeAllocator_;

    void DeleteNode(TreapNode* n) noexcept
    {
        if (n == nullptr) {
            return;
        }
        nodeAllocator_.Deallocate(n);
    }

    // the following function tries to merge new node (idx, num) with n
    enum class MergeResult {
        MERGE_SUCCESS = 0, // successfully merged with the node n
        MERGE_MISS,        // the new node (idx, num) is not connected to n, cannot merge
        MERGE_ERROR        // error, operation aborted
    };

    MergeResult MergeAt(TreapNode& n, uint32_t idx, uint32_t num, bool refreshRegionDesc)
    {
        uint32_t endIdx = idx + num;

        // try to merge the inserted node to the right of n
        if (idx == n.GetIndex() + n.GetCount()) {
            return MergeToRight(n, endIdx, num, refreshRegionDesc);
        }

        // try to merge the inserted node to the left of n
        if (endIdx == n.GetIndex()) {
            return MergeToLeft(n, idx, num, refreshRegionDesc);
        }

        return MergeResult::MERGE_MISS;
    }

    // merge free units to the right of the node. free units in the new merged node ends with endIdx,
    // and we should also try to merge the nearest right node to the new node if possible.
    MergeResult MergeToRight(TreapNode& n, uint32_t endIdx, uint32_t num, bool refreshRegionDesc)
    {
        // find the nearest right n of the new merged n which ends with endIdx.
        TreapNode* parent = &n; // the parent of the *nearest* node.
        TreapNode* nearest = n.r;
        while (nearest != nullptr) {
            if (nearest->GetIndex() == endIdx) {
                if (nearest->l != nullptr) {
                    CTREE_ASSERT(false, "merging failed case 1");
                    return MergeResult::MERGE_ERROR;
                }
                break;
            } else if (nearest->GetIndex() < endIdx) {
                CTREE_ASSERT(false, "merging failed case 2");
                return MergeResult::MERGE_ERROR;
            } else {
                parent = nearest;
                nearest = nearest->l;
            }
        }

        n.IncCount(num, refreshRegionDesc);
        IncTotalCount(num);

        if (nearest != nullptr) {
            n.IncCount(nearest->GetCount(), refreshRegionDesc);

            // now the node doesn't have left child, so we can fast remove it.
            if (parent == &n) {
                n.r = nearest->r;
            } else {
                parent->l = nearest->r;
            }
            nodeAllocator_.Deallocate(nearest);
        }
        CTREE_CHECK_PARENT_AND_RCHILD(&n);
        return MergeResult::MERGE_SUCCESS;
    }

    // merge free units to the left of the node n. free units in the new merged node starts with startIdx,
    // and we should also try to merge the nearest left node to the new merged node if possible.
    MergeResult MergeToLeft(TreapNode& n, uint32_t startIdx, uint32_t num, bool refreshRegionDesc)
    {
        TreapNode* parent = &n; // the parent of the *nearest* node.
        TreapNode* nearest = n.l;
        while (nearest != nullptr) {
            if (nearest->GetIndex() + nearest->GetCount() == startIdx) {
                if (nearest->r != nullptr) {
                    CTREE_ASSERT(false, "merging failed case 3");
                    return MergeResult::MERGE_ERROR;
                }
                break;
            } else if (nearest->GetIndex() + nearest->GetCount() > startIdx) {
                CTREE_ASSERT(false, "merging failed case 4");
                return MergeResult::MERGE_ERROR;
            } else {
                parent = nearest;
                nearest = nearest->r;
            }
        }

        n.UpdateNode(startIdx, n.GetCount() + num, refreshRegionDesc);
        IncTotalCount(num);

        if (nearest != nullptr) {
            // now the node doesn't have right child, so we can fast remove it.
            if (parent == &n) {
                n.l = nearest->l;
            } else {
                parent->r = nearest->l;
            }
            n.UpdateNode(nearest->GetIndex(), n.GetCount() + nearest->GetCount(), refreshRegionDesc);
            nodeAllocator_.Deallocate(nearest);
        }
        CTREE_CHECK_PARENT_AND_LCHILD(&n);
        return MergeResult::MERGE_SUCCESS;
    }

    // see the public MergeInsert()
    bool MergeInsertInternal(uint32_t idx, uint32_t num, bool refreshRegionDesc);

    // rotate the node and its left child to maintain heap property
    inline TreapNode* RotateLeftChild(TreapNode& n) const
    {
        TreapNode* newRoot = n.l;
        n.l = newRoot->r;
        newRoot->r = &n;
        return newRoot;
    }

    // rotate the node and its right child to maintain heap property
    inline TreapNode* RotateRightChild(TreapNode& n) const
    {
        TreapNode* newRoot = n.r;
        n.r = newRoot->l;
        newRoot->l = &n;
        return newRoot;
    }

    bool TakeUnitsImpl(uint32_t num, uint32_t& idx, bool refershRegionDesc)
    {
        CopyIterator it(*this);
        TreapNode** nodePtr = it.Next(); // pointer to root node
        if (UNLIKELY_CC(nodePtr == nullptr)) {
            return false;
        }
        TreapNode* node = *nodePtr;
        if (node != nullptr && node->GetCount() < num) {
            DLOG(REGION, "c-tree %p fail to take %u free units", this, num);
            return false;
        }
        TreapNode** nextNodePtr = nullptr;
        while ((nextNodePtr = it.Next()) != nullptr) {
            TreapNode* nextNode = *nextNodePtr;
            if (nextNode != nullptr && nextNode->GetCount() < num) {
                break;
            }

            nodePtr = nextNodePtr;
        }

        node = *nodePtr;
        if (UNLIKELY_CC(node == nullptr)) {
            return false;
        }
        idx = node->GetIndex();
        auto count = node->GetCount();

        node->UpdateNode(idx + num, count - num, refershRegionDesc);
        DecTotalCount(num);

        if (node->GetCount() == 0) {
            RemoveZeroNode(*nodePtr);
        } else {
            LowerNonZeroNode(*nodePtr);
        }

        CTREE_CHECK_PARENT_AND_LCHILD(*nodePtr);
        CTREE_CHECK_PARENT_AND_RCHILD(*nodePtr);

        return true;
    }

    bool AllocateLowestAddressFromNode(TreapNode*& node, uint32_t count, uint32_t& index)
    {
        uint32_t nodeCount = node->GetCount();
        if (nodeCount < count) {
            return false;
        }

        index = node->GetIndex();
        DLOG(REGION, "c-tree %p v-alloc %u units from [%u+%u, %u)", this, count, index, nodeCount, index + nodeCount);

        node->UpdateNode(index + count, nodeCount - count, false);
        DecTotalCount(count);
        if (node->GetCount() == 0) {
            RemoveZeroNode(node);
        } else {
            LowerNonZeroNode(node);
        }
        return true;
    }

    // move node n down in the tree to maintain the heap property
    NO_INLINE_CC TreapNode* LowerNode(TreapNode* n)
    {
        CTREE_ASSERT(n, "lowering node failed");
        TreapNode* tmp = nullptr;

        if (n->l != nullptr && n->l->GetCount() > n->GetCount()) {
            // this makes right tree slightly taller
            if (n->r != nullptr && n->r->GetCount() > n->l->GetCount()) {
                tmp = RotateRightChild(*n);
                tmp->l = LowerNode(tmp->l);
                CTREE_CHECK_PARENT_AND_LCHILD(tmp);
                return tmp;
            } else {
                tmp = RotateLeftChild(*n);
                tmp->r = LowerNode(tmp->r);
                CTREE_CHECK_PARENT_AND_RCHILD(tmp);
                return tmp;
            }
        }

        if (n->r != nullptr && n->r->GetCount() > n->GetCount()) {
            tmp = RotateRightChild(*n);
            tmp->l = LowerNode(tmp->l);
            CTREE_CHECK_PARENT_AND_LCHILD(tmp);
            return tmp;
        }

        return n;
    }

    // return the new position of node n.
    TreapNode*& LowerImproperNodeOnce(TreapNode*& n) const
    {
        CTREE_ASSERT(n, "failed to lower node once");
        if (n->l != nullptr) {
            // use the child which has the max count to instead of node.
            if (n->r != nullptr && n->r->GetCount() >= n->l->GetCount()) {
                TreapNode* newRoot = RotateRightChild(*n);
                CTREE_CHECK_PARENT_AND_LCHILD(newRoot);
                n = newRoot;
                return newRoot->l;
            }
            TreapNode* newRoot = RotateLeftChild(*n);
            CTREE_CHECK_PARENT_AND_RCHILD(newRoot);
            n = newRoot;
            return newRoot->r;
        }
        // the node have right child only
        if (n->r != nullptr) {
            TreapNode* newRoot = RotateRightChild(*n);
            CTREE_CHECK_PARENT_AND_LCHILD(newRoot);
            n = newRoot;
            return newRoot->l;
        }
        return n;
    }

    // maitain the heap property for subtree with root node n. assume n->GetCount() returns 0 for now.
    // return the new position of node n.
    TreapNode*& MaintainHeapPropertyForZeroNode(TreapNode*& n) const
    {
        TreapNode** nodePtr = &n;
        while ((*nodePtr)->l != nullptr || (*nodePtr)->r != nullptr) {
            nodePtr = &LowerImproperNodeOnce(*nodePtr);
        }
        return *nodePtr;
    }

    // maitain the heap property for subtree with root node n. assume n->GetCount() is less than its children.
    // return the new position of node n.
    void MaintainHeapPropertyForNonZeroNode(TreapNode*& n) const
    {
        TreapNode** nodePtr = &n;
        // *nodePtr won't be nullptr, don't need to check it.
        while (!(**nodePtr).IsProperNode()) {
            nodePtr = &LowerImproperNodeOnce(*nodePtr);
        }
    }

    void RemoveZeroNode(TreapNode*& n)
    {
        TreapNode*& nodeRef = MaintainHeapPropertyForZeroNode(n);
        LOGF_CHECK((nodeRef->l == nullptr && nodeRef->r == nullptr)) << "UNEXPECT";
        nodeAllocator_.Deallocate(nodeRef);
        nodeRef = nullptr;
    }

    void RemoveNode(TreapNode*& n)
    {
        DecTotalCount(n->GetCount());
        n->ClearCount();
        RemoveZeroNode(n);
    }

    // move node n down in the tree to maintain the heap property
    void LowerNonZeroNode(TreapNode*& n) const { MaintainHeapPropertyForNonZeroNode(n); }

#ifndef NDEBUG
    inline void CheckParentAndLeftChild(const TreapNode* n) const
    {
        if (n != nullptr) {
            const TreapNode* l = n->l;
            if (l != nullptr) {
                CTREE_ASSERT((n->GetIndex() > (l->GetIndex() + l->GetCount())), "left child overlapped with parent");
                CTREE_ASSERT((n->GetCount() >= l->GetCount()), "left child bigger than parent");
            }
        }
    }
    inline void CheckParentAndRightChild(const TreapNode* n) const
    {
        if (n != nullptr) {
            const TreapNode* r = n->r;
            if (r != nullptr) {
                CTREE_ASSERT(((n->GetIndex() + n->GetCount()) < r->GetIndex()), "right child overlapped with parent");
                CTREE_ASSERT((n->GetCount() >= r->GetCount()), "right child bigger than parent");
            }
        }
    }

    inline void CheckNode(const TreapNode* n) const
    {
        CheckParentAndLeftChild(n);
        CheckParentAndRightChild(n);
    }

    void VerifyTree()
    {
        uint32_t total = 0;
        Treap::Iterator it(*this);
        TreapNode* node = it.Next();
        while (node != nullptr) {
            total += node->GetCount();
            CheckNode(node);
            node = it.Next();
        }

        if (total != GetTotalCount()) { //LCOV_EXCL_BR_LINE
            DLOG(REGION, "c-tree %p total unit count %u (expect %u)", this, GetTotalCount(), total);
            DumpTree("internal error tree");
            LOG_COMMON(FATAL) << "Treap::VerifyTree() Should not execute here, abort.";
            UNREACHABLE_CC();
        }
    }
#endif
};
} // namespace common

#endif  // COMMON_COMPONENTS_HEAP_ALLOCATOR_TREAP_H
