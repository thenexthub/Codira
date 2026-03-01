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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYZER_H
#define COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYZER_H

#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace panda::compiler {
class BasicBlock;
class Graph;

class Loop final {
public:
    Loop(ArenaAllocator *allocator, BasicBlock *header, uint32_t id)
        : header_(header),
          back_edges_(allocator->Adapter()),
          blocks_(allocator->Adapter()),
          inner_loops_(allocator->Adapter()),
          id_(id)
    {
    }

    DEFAULT_MOVE_SEMANTIC(Loop);
    DEFAULT_COPY_SEMANTIC(Loop);
    ~Loop() = default;

    bool operator==(const Loop &other) const
    {
        return std::tie(header_, pre_header_, is_irreducible_, is_root_, is_infinite_) ==
                   std::tie(other.header_, other.pre_header_, other.is_irreducible_, other.is_root_,
                            other.is_infinite_) &&
               IsEqualBlocks(blocks_, other.blocks_) && IsEqualBlocks(back_edges_, other.back_edges_);
    }

    BasicBlock *GetHeader() const
    {
        return header_;
    }
    void SetPreHeader(BasicBlock *pre_header)
    {
        pre_header_ = pre_header;
    }

    BasicBlock *GetPreHeader() const
    {
        return pre_header_;
    }
    void AppendBackEdge(BasicBlock *block)
    {
        ASSERT(std::find(back_edges_.begin(), back_edges_.end(), block) == back_edges_.end());
        back_edges_.push_back(block);
    }

    void ReplaceBackEdge(BasicBlock *block, BasicBlock *new_block)
    {
        ASSERT(block != new_block);
        ASSERT(std::find(back_edges_.begin(), back_edges_.end(), new_block) == back_edges_.end());
        auto it = std::find(back_edges_.begin(), back_edges_.end(), block);
        ASSERT(it != back_edges_.end());
        ASSERT(std::find(it + 1, back_edges_.end(), block) == back_edges_.end());
        back_edges_[std::distance(back_edges_.begin(), it)] = new_block;
    }

    bool HasBackEdge(BasicBlock *block) const
    {
        auto it = std::find(back_edges_.begin(), back_edges_.end(), block);
        return it != back_edges_.end();
    }

    void RemoveBackEdge(BasicBlock *block)
    {
        auto it = std::find(back_edges_.begin(), back_edges_.end(), block);
        ASSERT(it != back_edges_.end());
        ASSERT(std::find(it + 1, back_edges_.end(), block) == back_edges_.end());
        back_edges_[std::distance(back_edges_.begin(), it)] = back_edges_.back();
        back_edges_.pop_back();
    }

    void MoveHeaderToSucc();

    const ArenaVector<BasicBlock *> &GetBackEdges() const
    {
        return back_edges_;
    }

    void AppendBlock(BasicBlock *block);

    // NB! please use carefully, ensure that the block
    // 1. is not a header of the loop
    // 2. is not a back-edge of the loop
    // 3. is not a pre-header of any inner loop
    void RemoveBlock(BasicBlock *block);

    ArenaVector<BasicBlock *> &GetBlocks()
    {
        return blocks_;
    }
    const ArenaVector<BasicBlock *> &GetBlocks() const
    {
        return blocks_;
    }
    void AppendInnerLoop(Loop *inner_loop)
    {
        inner_loops_.push_back(inner_loop);
    }
    ArenaVector<Loop *> &GetInnerLoops()
    {
        return inner_loops_;
    }
    const ArenaVector<Loop *> &GetInnerLoops() const
    {
        return inner_loops_;
    }
    void SetOuterLoop(Loop *outer_loop)
    {
        outer_loop_ = outer_loop;
    }
    Loop *GetOuterLoop() const
    {
        return outer_loop_;
    }
    void SetIsIrreducible(bool is_irreducible)
    {
        is_irreducible_ = is_irreducible;
    }
    bool IsIrreducible() const
    {
        return is_irreducible_;
    }
    bool IsInfinite() const
    {
        return is_infinite_;
    }
    void SetIsInfinite(bool is_infinite)
    {
        is_infinite_ = is_infinite;
    }
    void SetAsRoot()
    {
        is_root_ = true;
    }
    bool IsRoot() const
    {
        return is_root_;
    }
    uint32_t GetId() const
    {
        return id_;
    }

    bool IsOsrLoop() const;

    bool IsTryCatchLoop() const;

    bool IsInside(Loop *other);

private:
    void CheckInfinity();

    template <typename T>
    static inline bool IsEqualBlocks(const ArenaVector<T> &blocks, const ArenaVector<T> &others)
    {
        return blocks.size() == others.size() && std::is_permutation(blocks.begin(), blocks.end(), others.begin());
    }

private:
    BasicBlock *header_ {nullptr};
    BasicBlock *pre_header_ {nullptr};
    ArenaVector<BasicBlock *> back_edges_;
    ArenaVector<BasicBlock *> blocks_;
    ArenaVector<Loop *> inner_loops_;
    Loop *outer_loop_ {nullptr};
    bool is_irreducible_ {false};
    bool is_infinite_ {false};
    bool is_root_ {false};
    uint32_t id_ {INVALID_ID};

    friend class LoopAnalyzer;
};

class LoopAnalyzer final : public Analysis {
public:
    using Analysis::Analysis;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "LoopAnalysis";
    }

    void CreateRootLoop();
    Loop *CreateNewLoop(BasicBlock *loop_header);

private:
    void ResetLoopInfo();
    void CollectBackEdges();
    void BackEdgeSearch(BasicBlock *block);
    void ProcessNewBackEdge(BasicBlock *header, BasicBlock *back_edge);
    void FindAndInsertPreHeaders(Loop *loop);
    void MovePhiInputsToPreHeader(BasicBlock *header, BasicBlock *pre_header, const ArenaVector<int> &fw_edges_indexes);
    void UpdateControlFlowWithPreHeader(BasicBlock *header, BasicBlock *pre_header,
                                        const ArenaVector<int> &fw_edges_indexes);
    ArenaVector<int> GetForwardEdgesIndexes(BasicBlock *header);
    bool PreHeaderExists(Loop *loop);
    BasicBlock *CreatePreHeader(BasicBlock *header);
    void PopulateLoops();
    void NaturalLoopSearch(Loop *loop, BasicBlock *block);
    void SearchInfiniteLoops(Loop *loop);

private:
    Marker black_marker_ {};
    Marker gray_marker_ {};
    uint32_t loop_counter_ {0};
};

BasicBlock *GetLoopOutsideSuccessor(Loop *loop);
// NOLINTNEXTLINE(readability-redundant-declaration)
bool IsLoopSingleBackEdgeExitPoint(Loop *loop);

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYZER_H
