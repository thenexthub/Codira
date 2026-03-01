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


#ifndef MRT_CARTESIAN_TREE_H
#define MRT_CARTESIAN_TREE_H

#include <cstddef>
#include <cstdint>

#include "Base/LogFile.h"
#include "Base/Panic.h"
#include "LocalDeque.h"

#if defined(MRT_DEBUG)
#define DEBUG_CARTESIAN_TREE
#endif

#ifdef DEBUG_CARTESIAN_TREE
#define CTREE_ASSERT(cond, msg) MRT_ASSERT(cond, msg)
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
namespace MapleRuntime {
class CartesianTree {
public:
    using Index = uint32_t; // abstract index for free memory in tree node.
    using Count = uint32_t; // corresponding memory count for this tree node.

    CartesianTree() = default;
    ~CartesianTree() = default;

    void Init(size_t memoryBlockCount)
    {
        // at most we need this many nodes
        size_t nodeCount = (memoryBlockCount >> 1) + 1;
        // calculate how much we need for native allocation
        // we might need some extra space for some temporaries, so set aside another 7 slots
        size_t nativeSize = (nodeCount + 7) * AllocUtilRndUp(sizeof(Node), alignof(Node));
        nodeAllocator.Init(ALLOCUTIL_PAGE_RND_UP(nativeSize));
        // calculate how much we need for the deque temporary
        size_t dequeSize = nodeCount * sizeof(Node*);
        sud.Init(ALLOCUTIL_PAGE_RND_UP(dequeSize));
        traversalSud.Init(sud.GetMemMap());
    }

    void Fini() noexcept
    {
        ClearTree();
        sud.Fini();
        nodeAllocator.Fini();
    }

    void ClearTree()
    {
        CartesianTree::Iterator it(*this);
        Node* node = it.Next();
        while (node != nullptr) {
            DeleteNode(node);
            node = it.Next();
        }

        root = nullptr;
        totalCount = 0;
    }

    Count GetTotalCount() const { return totalCount; }

    void IncTotalCount(Count cnt) { totalCount += cnt; }

    void DecTotalCount(Count cnt)
    {
        if (totalCount < cnt) {
            LOG(RTLOG_FATAL, "CartesianTree::DecTotalCount() Should not execute here, abort.");
        }
        totalCount -= cnt;
    }

    // insert a node to the tree, if we find connecting nodes, we merge them
    // (the non-merging insertion is not allowed)
    // true when insertion succeeded, false otherwise
    // if [idx, idx + num) clashes with existing node, it fails
    // if num is 0U, it always fails
    bool MergeInsert(Index idx, Count num, bool refreshRegionInfo)
    {
        if (root == nullptr) {
            root = new (nodeAllocator.Allocate()) Node(idx, num, refreshRegionInfo);
            CTREE_ASSERT(root != nullptr, "failed to allocate a new node");
            IncTotalCount(num);
            return true;
        }

        if (num == 0) {
            return false;
        }

        return MergeInsertInternal(idx, num, refreshRegionInfo);
    }

    // find a node with at least 'num' units
    bool TakeUnits(Count num, Index& idx, bool refreshRegionInfo = true)
    {
        if (root == nullptr || num == 0) {
            return false;
        }

        return TakeUnitsImpl(num, idx, refreshRegionInfo);
    }

    struct Node {
        Node(Index idx, Count num, bool refreshRegionInfo) : l(nullptr), r(nullptr), index(idx), count(num)
        {
            if (refreshRegionInfo) {
                RefreshFreeRegionInfo();
            }
        }

        ~Node()
        {
            l = nullptr;
            r = nullptr;
        }

        inline Index GetIndex() const { return index; }

        inline Count GetCount() const { return count; }

        inline void IncCount(Count num, bool refreshRegionInfo)
        {
            count += num;
            if (refreshRegionInfo && count > 0) {
                RefreshFreeRegionInfo();
            }
        }

        inline void ClearCount() { count = 0; }

        inline void UpdateNode(Index idx, Count cnt, bool refreshRegionInfo)
        {
            index = idx;
            count = cnt;
            if (refreshRegionInfo && cnt > 0) {
                // GetNextNeighborRegion in compact gc expects free-region metadata is always up-to-date.
                // otherwise we can ignore refreshRegionInfo.
                RefreshFreeRegionInfo();
            }
        }

        inline bool IsProperNode() const
        {
            Count leftCount = l == nullptr ? 0 : l->GetCount();
            Count rightCount = r == nullptr ? 0 : r->GetCount();
            return (count >= leftCount && count >= rightCount);
        }

        void RefreshFreeRegionInfo();
        void ReleaseMemory();

        Node* l;
        Node* r;

    private:
        Index index;
        Count count;
    };

    // Iterator is defined specifically for releasing physical pages.
    // It provides a "backwards" level-order traversal with right child node visited first
    // rather than left child node first. This behaviour avoids that physical pages for regions
    // are released and again committed for near future allocation.
    class Iterator {
    public:
        explicit Iterator(CartesianTree& tree) : localQueue(tree.traversalSud)
        {
            if (tree.root != nullptr) {
                localQueue.Push(tree.root);
            }
        }

        ~Iterator() = default;

        inline Node* Next()
        {
            if (localQueue.Empty()) {
                return nullptr;
            }
            Node* front = localQueue.Front();
            if (front->r != nullptr) {
                localQueue.Push(front->r);
            }
            if (front->l != nullptr) {
                localQueue.Push(front->l);
            }
            localQueue.PopFront();
            return front;
        }

    private:
        LocalDeque<Node*> localQueue;
    };

    // very much like forward iterator in c++.
    // traverse nodes by lvalue-reference.
    class ForwardIterator {
    public:
        explicit ForwardIterator(CartesianTree& tree) : localQueue(tree.sud)
        {
            if (tree.root != nullptr) {
                localQueue.Push(&tree.root);
            }
        }

        ~ForwardIterator() = default;

        inline Node** Next()
        {
            if (localQueue.Empty()) {
                return nullptr;
            }
            Node** topElement = localQueue.Front();
            Node* node = *topElement;
            if (node->r != nullptr) {
                localQueue.Push(&node->r);
            }
            if (node->l != nullptr) {
                localQueue.Push(&node->l);
            }
            localQueue.PopFront();
            return topElement;
        }

    private:
        LocalDeque<Node**> localQueue;
    };

    inline bool Empty() const { return root == nullptr; }
    inline const Node* RootNode() const { return root; }

    // root node records the largest block of memory.
    void ReleaseRootNode()
    {
        root->ReleaseMemory();
        RemoveNode(root);
    }

    using RTAllocator = RTAllocatorT<sizeof(Node), alignof(Node)>;

#ifdef DEBUG_CARTESIAN_TREE
    void DumpTree(const char* msg) const;
#endif

private:
    Count totalCount = 0u; // sum of all node counts.
    Node* root = nullptr;
    SingleUseDeque<Node**> sud;
    SingleUseDeque<Node*> traversalSud;
    RTAllocator nodeAllocator;

    void DeleteNode(Node* n) noexcept
    {
        if (n == nullptr) {
            return;
        }
        nodeAllocator.Deallocate(n);
    }

    // the following function tries to merge new node (idx, num) with n
    enum class MergeResult {
        MERGE_SUCCESS = 0, // successfully merged with the node n
        MERGE_MISS,        // the new node (idx, num) is not connected to n, cannot merge
        MERGE_ERROR        // error, operation aborted
    };

    MergeResult MergeAt(Node& n, Index idx, Count num, bool refreshRegionInfo)
    {
        Index endIdx = idx + num;

        // try to merge the inserted node to the right of n
        if (idx == n.GetIndex() + n.GetCount()) {
            return MergeToRight(n, endIdx, num, refreshRegionInfo);
        }

        // try to merge the inserted node to the left of n
        if (endIdx == n.GetIndex()) {
            return MergeToLeft(n, idx, num, refreshRegionInfo);
        }

        return MergeResult::MERGE_MISS;
    }

    // merge free units to the right of the node. free units in the new merged node ends with endIdx,
    // and we should also try to merge the nearest right node to the new node if possible.
    MergeResult MergeToRight(Node& n, Index endIdx, Count num, bool refreshRegionInfo)
    {
        // find the nearest right n of the new merged n which ends with endIdx.
        Node* parent = &n; // the parent of the *nearest* node.
        Node* nearest = n.r;
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

        n.IncCount(num, refreshRegionInfo);
        IncTotalCount(num);

        if (nearest != nullptr) {
            n.IncCount(nearest->GetCount(), refreshRegionInfo);

            // now the node doesn't have left child, so we can fast remove it.
            if (parent == &n) {
                n.r = nearest->r;
            } else {
                parent->l = nearest->r;
            }
            nodeAllocator.Deallocate(nearest);
        }
        CTREE_CHECK_PARENT_AND_RCHILD(&n);
        return MergeResult::MERGE_SUCCESS;
    }

    // merge free units to the left of the node n. free units in the new merged node starts with startIdx,
    // and we should also try to merge the nearest left node to the new merged node if possible.
    MergeResult MergeToLeft(Node& n, Index startIdx, Count num, bool refreshRegionInfo)
    {
        Node* parent = &n; // the parent of the *nearest* node.
        Node* nearest = n.l;
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

        n.UpdateNode(startIdx, n.GetCount() + num, refreshRegionInfo);
        IncTotalCount(num);

        if (nearest != nullptr) {
            // now the node doesn't have right child, so we can fast remove it.
            if (parent == &n) {
                n.l = nearest->l;
            } else {
                parent->r = nearest->l;
            }
            n.UpdateNode(nearest->GetIndex(), n.GetCount() + nearest->GetCount(), refreshRegionInfo);
            nodeAllocator.Deallocate(nearest);
        }
        CTREE_CHECK_PARENT_AND_LCHILD(&n);
        return MergeResult::MERGE_SUCCESS;
    }

    // see the public MergeInsert()
    bool MergeInsertInternal(Index idx, Count num, bool refreshRegionInfo);

    // rotate the node and its left child to maintain heap property
    inline Node* RotateLeftChild(Node& n) const
    {
        Node* newRoot = n.l;
        n.l = newRoot->r;
        newRoot->r = &n;
        return newRoot;
    }

    // rotate the node and its right child to maintain heap property
    inline Node* RotateRightChild(Node& n) const
    {
        Node* newRoot = n.r;
        n.r = newRoot->l;
        newRoot->l = &n;
        return newRoot;
    }

    bool TakeUnitsImpl(Count num, Index& idx, bool refershRegionInfo)
    {
        ForwardIterator it(*this);
        Node** nodePtr = it.Next(); // pointer to root node
        if (UNLIKELY(nodePtr == nullptr)) {
            return false;
        }
        Node* node = *nodePtr;
        if (node != nullptr && node->GetCount() < num) {
            DLOG(REGION, "c-tree %p fail to take %u free units", this, num);
            return false;
        }
        Node** nextNodePtr = nullptr;
        while ((nextNodePtr = it.Next()) != nullptr) {
            Node* nextNode = *nextNodePtr;
            if (nextNode != nullptr && nextNode->GetCount() < num) {
                break;
            }

            nodePtr = nextNodePtr;
        }

        node = *nodePtr;
        idx = node->GetIndex();
        auto count = node->GetCount();

        node->UpdateNode(idx + num, count - num, refershRegionInfo);
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

    bool AllocateLowestAddressFromNode(Node*& node, Count count, Index& index)
    {
        Count nodeCount = node->GetCount();
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
    ATTR_NO_INLINE Node* LowerNode(Node* n)
    {
        CTREE_ASSERT(n, "lowering node failed");
        Node* tmp = nullptr;

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
    Node*& LowerImproperNodeOnce(Node*& n) const
    {
        CTREE_ASSERT(n, "failed to lower node once");
        if (n->l != nullptr) {
            // use the child which has the max count to instead of node.
            if (n->r != nullptr && n->r->GetCount() >= n->l->GetCount()) {
                Node* newRoot = RotateRightChild(*n);
                CTREE_CHECK_PARENT_AND_LCHILD(newRoot);
                n = newRoot;
                return newRoot->l;
            }
            Node* newRoot = RotateLeftChild(*n);
            CTREE_CHECK_PARENT_AND_RCHILD(newRoot);
            n = newRoot;
            return newRoot->r;
        }
        // the node have right child only
        if (n->r != nullptr) {
            Node* newRoot = RotateRightChild(*n);
            CTREE_CHECK_PARENT_AND_LCHILD(newRoot);
            n = newRoot;
            return newRoot->l;
        }
        return n;
    }

    // maitain the heap property for subtree with root node n. assume n->GetCount() returns 0 for now.
    // return the new position of node n.
    Node*& MaintainHeapPropertyForZeroNode(Node*& n) const
    {
        Node** nodePtr = &n;
        while ((*nodePtr)->l != nullptr || (*nodePtr)->r != nullptr) {
            nodePtr = &LowerImproperNodeOnce(*nodePtr);
        }
        return *nodePtr;
    }

    // maitain the heap property for subtree with root node n. assume n->GetCount() is less than its children.
    // return the new position of node n.
    void MaintainHeapPropertyForNonZeroNode(Node*& n) const
    {
        Node** nodePtr = &n;
        // *nodePtr won't be nullptr, don't need to check it.
        while (!(**nodePtr).IsProperNode()) {
            nodePtr = &LowerImproperNodeOnce(*nodePtr);
        }
    }

    void RemoveZeroNode(Node*& n)
    {
        Node*& nodeRef = MaintainHeapPropertyForZeroNode(n);
        CHECK_DETAIL((nodeRef->l == nullptr && nodeRef->r == nullptr), "UNEXPECT");
        nodeAllocator.Deallocate(nodeRef);
        nodeRef = nullptr;
    }

    void RemoveNode(Node*& n)
    {
        DecTotalCount(n->GetCount());
        n->ClearCount();
        RemoveZeroNode(n);
    }

    // move node n down in the tree to maintain the heap property
    void LowerNonZeroNode(Node*& n) const { MaintainHeapPropertyForNonZeroNode(n); }

#ifdef DEBUG_CARTESIAN_TREE
    inline void CheckParentAndLeftChild(const Node* n) const
    {
        if (n != nullptr) {
            const Node* l = n->l;
            if (l != nullptr) {
                CTREE_ASSERT((n->GetIndex() > (l->GetIndex() + l->GetCount())), "left child overlapped with parent");
                CTREE_ASSERT((n->GetCount() >= l->GetCount()), "left child bigger than parent");
            }
        }
    }
    inline void CheckParentAndRightChild(const Node* n) const
    {
        if (n != nullptr) {
            const Node* r = n->r;
            if (r != nullptr) {
                CTREE_ASSERT(((n->GetIndex() + n->GetCount()) < r->GetIndex()), "right child overlapped with parent");
                CTREE_ASSERT((n->GetCount() >= r->GetCount()), "right child bigger than parent");
            }
        }
    }

    inline void CheckNode(const Node* n) const
    {
        CheckParentAndLeftChild(n);
        CheckParentAndRightChild(n);
    }

    void VerifyTree()
    {
        Count total = 0;
        CartesianTree::Iterator it(*this);
        Node* node = it.Next();
        while (node != nullptr) {
            total += node->GetCount();
            CheckNode(node);
            node = it.Next();
        }

        if (total != GetTotalCount()) {
            DLOG(REGION, "c-tree %p total unit count %u (expect %u)", this, GetTotalCount(), total);
            DumpTree("internal error tree");
            LOG(RTLOG_FATAL, "CartesianTree::VerifyTree() Should not execute here, abort.");
        }
    }
#endif
};
} // namespace MapleRuntime
#endif // MRT_CARTESIAN_TREE_H
