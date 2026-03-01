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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/rpo.h"
#include "dominators_tree.h"

namespace ark::compiler {
DominatorsTree::DominatorsTree(Graph *graph) : Analysis(graph) {}

bool DominatorsTree::RunImpl()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        block->ClearDominatedBlocks();
        block->ClearDominator();
    }

    Init(GetGraph()->GetVectorBlocks().size());
    DfsNumbering(GetGraph()->GetStartBlock());
    auto dfsBlocks = static_cast<size_t>(dfsNum_);
    ASSERT_PRINT(dfsBlocks == (GetGraph()->GetBlocksRPO().size() - 1), "There is an unreachable block");

    for (size_t i = dfsBlocks; i > 0; i--) {
        ComputeImmediateDominators(GetVertex(i));
    }

    for (size_t i = 1; i <= dfsBlocks; i++) {
        AdjustImmediateDominators(GetVertex(i));
    }
    return true;
}

/*
 * Adjust immediate dominators,
 * Update dominator information for 'block'
 */
void DominatorsTree::AdjustImmediateDominators(BasicBlock *block)
{
    ASSERT(block != nullptr);

    if (GetIdom(block) != GetVertex(GetSemi(block))) {
        SetIdom(block, GetIdom(GetIdom(block)));
    }
    SetDomPair(GetIdom(block), block);
}

/*
 * Compute initial values for semidominators,
 * store blocks with the same semidominator in the same bucket,
 * compute immediate dominators for blocks in the bucket of 'block' parent
 */
void DominatorsTree::ComputeImmediateDominators(BasicBlock *block)
{
    ASSERT(block != nullptr);

    for (auto pred : block->GetPredsBlocks()) {
        auto eval = Eval(pred);
        if (GetSemi(eval) < GetSemi(block)) {
            SetSemi(block, GetSemi(eval));
        }
    }

    auto vertex = GetVertex(GetSemi(block));
    GetBucket(vertex).push_back(block);
    auto parent = GetParent(block);
    Link(parent, block);

    auto &bucket = GetBucket(parent);
    while (!bucket.empty()) {
        auto v = *bucket.rbegin();
        auto eval = Eval(v);
        if (GetSemi(eval) >= GetSemi(v)) {
            SetIdom(v, parent);
        } else {
            SetIdom(v, eval);
        }
        bucket.pop_back();
    }
}

/*
 * Compress ancestor path to 'block' to the block whose label has the maximal semidominator number
 */
void DominatorsTree::Compress(BasicBlock *block)
{
    auto anc = GetAncestor(block);
    ASSERT(anc != nullptr);

    if (GetAncestor(anc) != nullptr) {
        Compress(anc);
        if (GetSemi(GetLabel(anc)) < GetSemi(GetLabel(block))) {
            SetLabel(block, GetLabel(anc));
        }
        SetAncestor(block, GetAncestor(anc));
    }
}

/*
 *  Depth-first search with numbering blocks in order they are reaching
 */
void DominatorsTree::DfsNumbering(BasicBlock *block)
{
    dfsNum_++;
    ASSERT_PRINT(static_cast<size_t>(dfsNum_) < vertices_->size(), "DFS-number overflow");
    ASSERT(block != nullptr);

    SetVertex(dfsNum_, block);
    SetLabel(block, block);
    SetSemi(block, dfsNum_);
    SetAncestor(block, nullptr);

    for (auto succ : block->GetSuccsBlocks()) {
        if (GetSemi(succ) == DEFAULT_DFS_VAL) {
            SetParent(succ, block);
            DfsNumbering(succ);
        }
    }
}

/*
 * Return 'block' if it is the root of a tree
 * Otherwise, after tree compressing
 * return the block in the ancestors chain with the minimal semidominator DFS-number
 */
BasicBlock *DominatorsTree::Eval(BasicBlock *block)
{
    ASSERT(block != nullptr);
    if (GetAncestor(block) == nullptr) {
        return block;
    }
    Compress(block);
    return GetLabel(block);
}

/*
 * Initialize data structures to start DFS
 */
void DominatorsTree::Init(size_t blocksCount)
{
    auto allocator = GetGraph()->GetLocalAllocator();
    ancestors_ = allocator->New<BlocksVector>(allocator->Adapter());
    buckets_ = allocator->New<ArenaVector<BlocksVector>>(allocator->Adapter());
    idoms_ = allocator->New<BlocksVector>(allocator->Adapter());
    labels_ = allocator->New<BlocksVector>(allocator->Adapter());
    parents_ = allocator->New<BlocksVector>(allocator->Adapter());
    semi_ = allocator->New<ArenaVector<int32_t>>(allocator->Adapter());
    vertices_ = allocator->New<BlocksVector>(allocator->Adapter());

    auto maxSize = GetGraph()->GetVectorBlocks().size();
    if (blocksCount > maxSize) {
        blocksCount = maxSize;
    }
    ancestors_->resize(blocksCount);
    idoms_->resize(blocksCount);
    labels_->resize(blocksCount);
    parents_->resize(blocksCount);
    vertices_->resize(blocksCount);
    semi_->resize(blocksCount);

    std::fill(vertices_->begin(), vertices_->end(), nullptr);
    std::fill(semi_->begin(), semi_->end(), DEFAULT_DFS_VAL);

    buckets_->resize(blocksCount, BlocksVector(allocator->Adapter()));
    for (auto &bucket : *buckets_) {
        bucket.clear();
    }

    dfsNum_ = DEFAULT_DFS_VAL;
}

/* static */
void DominatorsTree::SetDomPair(BasicBlock *dominator, BasicBlock *block)
{
    ASSERT(block != nullptr);
    block->SetDominator(dominator);
    ASSERT(dominator != nullptr);
    dominator->AddDominatedBlock(block);
}

/*
 * Check if there is path from `start_block` to `target_block` excluding `exclude_block`
 */
static bool IsPathBetweenBlocks(BasicBlock *startBlock, BasicBlock *targetBlock, BasicBlock *excludeBlock)
{
    auto markerHolder = MarkerHolder(targetBlock->GetGraph());
    auto marker = markerHolder.GetMarker();
    return BlocksPathDfsSearch(marker, startBlock, targetBlock, excludeBlock);
}

void DominatorsTree::UpdateAfterResolverInsertion(BasicBlock *predecessor, BasicBlock *successor, BasicBlock *resolver)
{
    SetValid(true);
    SetDomPair(predecessor, resolver);

    if (successor->GetDominator() == predecessor) {
        bool resolverDominatePhiBlock = true;
        for (auto succ : predecessor->GetSuccsBlocks()) {
            if (succ == resolver) {
                continue;
            }
            if (IsPathBetweenBlocks(succ, successor, resolver)) {
                resolverDominatePhiBlock = false;
                break;
            }
        }

        if (resolverDominatePhiBlock) {
            predecessor->RemoveDominatedBlock(successor);
            SetDomPair(resolver, successor);
        }
    }
}

/* static */
inline uint32_t DominatorsTree::GetBlockId(BasicBlock *block)
{
    return block->GetId();
}
}  // namespace ark::compiler
