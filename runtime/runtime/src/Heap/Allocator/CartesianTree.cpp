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


#include "CartesianTree.h"

#include "Allocator/RegionInfo.h"

namespace MapleRuntime {
bool CartesianTree::MergeInsertInternal(Index idx, Count num, bool refreshRegionInfo)
{
    //     +-------------+       +--------------+
    //     | parent node |  n--> | current node |
    //     |             |       |              |
    // pn ---> Node* l; -------> |              |
    //     |   Node* r;  |       |              |
    //     +-------------+       +--------------+
    // suppose current node is parent node's left child, then
    // n points to the current node,
    // pn points to the 'l' field in the parent node
    Node* n = root;    // root is current node
    Node** pn = &root; // pointer to the 'root' field in this tree
    // stack of pn recording how to go from root to the current node
    LocalDeque<Node**> pnStack(sud); // this uses another deque as container
    Index m = idx + num;
    // this loop insert the new node (idx, num) at the proper place
    do {
        if (n == nullptr) {
            n = new (nodeAllocator.Allocate()) Node(idx, num, refreshRegionInfo);
            CTREE_ASSERT(n != nullptr, "failed to allocate a new node");
            *pn = n;
            IncTotalCount(num);
            break;
        }
        MergeResult res = MergeAt(*n, idx, num, refreshRegionInfo);
        if (res == MergeResult::MERGE_SUCCESS) {
            break;
        } else if (UNLIKELY(res == MergeResult::MERGE_ERROR)) {
            return false;
        }
        // MergeResult::MERGE_MISS: (idx, num) cannot be connected to n
        if (m < n->GetIndex()) {
            // should insert into left subtree
            pnStack.Push(pn);
            pn = &(n->l);
            n = n->l;
        } else if (idx > n->GetIndex() + n->GetCount()) {
            // should insert into right subtree
            pnStack.Push(pn);
            pn = &(n->r);
            n = n->r;
        } else {
            // something clashes
            CTREE_ASSERT(false, "merge insertion failed");
            return false;
        }
    } while (true);

    // this loop bubbles the inserted node up the tree to satisfy heap property
    while (!pnStack.Empty()) {
        pn = pnStack.Top();
        pnStack.Pop();
        n = *pn;
        CTREE_ASSERT(n, "merge insertion bubbling failed case 1");
        if (m < n->GetIndex()) {
            // (idx, num) was inserted into n's left subtree, do rotate l, if needed
            if (n->GetCount() < n->l->GetCount()) {
                *pn = RotateLeftChild(*n);
                CTREE_CHECK_PARENT_AND_RCHILD(*pn);
            } else {
                break;
            }
        } else if (idx > n->GetIndex() + n->GetCount()) {
            // (idx, num) was inserted into n's right subtree, do rotate r, if needed
            if (n->GetCount() < n->r->GetCount()) {
                *pn = RotateRightChild(*n);
                CTREE_CHECK_PARENT_AND_LCHILD(*pn);
            } else {
                break;
            }
        } else {
            CTREE_ASSERT(false, "merge insertion bubbling failed case 2");
            return false;
        }
    }
    return true;
}

#ifdef DEBUG_CARTESIAN_TREE
void CartesianTree::DumpTree(const char* msg) const
{
    if (Empty()) {
        return;
    }

    VLOG(REPORT, "dump %s %p in graphviz .dot:", msg, this);
    VLOG(REPORT, "digraph tree%p {", this);
    CartesianTree::Iterator it(*const_cast<CartesianTree*>(this));
    auto node = it.Next();
    while (node != nullptr) {
        VLOG(REPORT, "c-tree %p N%p [label=\"%p:%u+%u=%u\"]", this, node, node, node->GetIndex(),
             node->GetCount(), node->GetIndex() + node->GetCount());

        if (node->l != nullptr) {
            VLOG(REPORT, "c-tree %p N%p -> N%p", this, node, node->l);
        }

        VLOG(REPORT, "c-tree %p N%p -> D%p [style=invis]", this, node, node);
        VLOG(REPORT, "c-tree %p D%p [width=0, style=invis]", this, node);

        if (node->r != nullptr) {
            VLOG(REPORT, "c-tree %p N%p -> N%p", this, node, node->r);
        }

        node = it.Next();
    }
    VLOG(REPORT, "}");
}
#endif

void CartesianTree::Node::RefreshFreeRegionInfo()
{
    Index idx = GetIndex();
    Count cnt = GetCount();
    RegionInfo::InitFreeRegion(idx, cnt);
}

void CartesianTree::Node::ReleaseMemory()
{
    Index idx = GetIndex();
    Count cnt = GetCount();
    RegionInfo::ReleaseUnits(idx, cnt);
}
} // namespace MapleRuntime
