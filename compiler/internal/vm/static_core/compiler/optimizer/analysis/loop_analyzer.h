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
#ifndef COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYSIS_H
#define COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYSIS_H

#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/countable_loop_parser.h"

namespace ark::compiler {
class BasicBlock;
class Graph;

class Loop final {
public:
    Loop(ArenaAllocator *allocator, BasicBlock *header, uint32_t id)
        : header_(header),
          backEdges_(allocator->Adapter()),
          blocks_(allocator->Adapter()),
          innerLoops_(allocator->Adapter()),
          id_(id)
    {
    }

    DEFAULT_MOVE_SEMANTIC(Loop);
    DEFAULT_COPY_SEMANTIC(Loop);
    ~Loop() = default;

    bool operator==(const Loop &other) const
    {
        return std::tie(header_, preHeader_, isIrreducible_, isRoot_, isInfinite_) ==
                   std::tie(other.header_, other.preHeader_, other.isIrreducible_, other.isRoot_, other.isInfinite_) &&
               IsEqualBlocks(blocks_, other.blocks_) && IsEqualBlocks(backEdges_, other.backEdges_);
    }

    BasicBlock *GetHeader() const
    {
        return header_;
    }
    void SetPreHeader(BasicBlock *preHeader);
    void SetPreHeader(std::nullptr_t preHeader);
    PANDA_PUBLIC_API BasicBlock *GetPreHeader() const;

    void AppendBackEdge(BasicBlock *block)
    {
        ASSERT(std::find(backEdges_.begin(), backEdges_.end(), block) == backEdges_.end());
        backEdges_.push_back(block);
    }

    void ReplaceBackEdge(BasicBlock *block, BasicBlock *newBlock)
    {
        ASSERT(block != newBlock);
        ASSERT(std::find(backEdges_.begin(), backEdges_.end(), newBlock) == backEdges_.end());
        auto it = std::find(backEdges_.begin(), backEdges_.end(), block);
        ASSERT(it != backEdges_.end());
        ASSERT(std::find(it + 1, backEdges_.end(), block) == backEdges_.end());
        backEdges_[std::distance(backEdges_.begin(), it)] = newBlock;
    }

    bool HasBackEdge(BasicBlock *block) const
    {
        auto it = std::find(backEdges_.begin(), backEdges_.end(), block);
        return it != backEdges_.end();
    }

    void RemoveBackEdge(BasicBlock *block)
    {
        auto it = std::find(backEdges_.begin(), backEdges_.end(), block);
        ASSERT(it != backEdges_.end());
        ASSERT(std::find(it + 1, backEdges_.end(), block) == backEdges_.end());
        backEdges_[std::distance(backEdges_.begin(), it)] = backEdges_.back();
        backEdges_.pop_back();
    }

    void MoveHeaderToSucc();

    const ArenaVector<BasicBlock *> &GetBackEdges() const
    {
        return backEdges_;
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
    void AppendInnerLoop(Loop *innerLoop)
    {
        innerLoops_.push_back(innerLoop);
    }
    ArenaVector<Loop *> &GetInnerLoops()
    {
        return innerLoops_;
    }
    const ArenaVector<Loop *> &GetInnerLoops() const
    {
        return innerLoops_;
    }
    void SetOuterLoop(Loop *outerLoop)
    {
        outerLoop_ = outerLoop;
    }
    Loop *GetOuterLoop() const
    {
        return outerLoop_;
    }
    void SetIsIrreducible(bool isIrreducible)
    {
        isIrreducible_ = isIrreducible;
    }
    bool IsIrreducible() const
    {
        return isIrreducible_;
    }
    bool IsInfinite() const
    {
        return isInfinite_;
    }
    void SetIsInfinite(bool isInfinite)
    {
        isInfinite_ = isInfinite;
    }
    void SetAsRoot()
    {
        isRoot_ = true;
    }
    bool IsRoot() const
    {
        return isRoot_;
    }
    uint32_t GetId() const
    {
        return id_;
    }

    bool IsOsrLoop() const;

    bool IsTryCatchLoop() const;

    bool IsInside(Loop *other);

    auto GetDepth() const
    {
        return depth_;
    }

    bool IsPostExitBlock(const BasicBlock *block) const;

private:
    void CheckInfinity();

    void SetDepth(uint32_t depth)
    {
        depth_ = depth;
    }

    template <typename T>
    static inline bool IsEqualBlocks(const ArenaVector<T> &blocks, const ArenaVector<T> &others)
    {
        return blocks.size() == others.size() && std::is_permutation(blocks.begin(), blocks.end(), others.begin());
    }

private:
    BasicBlock *header_ {nullptr};
    BasicBlock *preHeader_ {nullptr};
    ArenaVector<BasicBlock *> backEdges_;
    ArenaVector<BasicBlock *> blocks_;
    ArenaVector<Loop *> innerLoops_;
    Loop *outerLoop_ {nullptr};
    uint32_t id_ {INVALID_ID};
    uint32_t depth_ {0};
    bool isIrreducible_ {false};
    bool isInfinite_ {false};
    bool isRoot_ {false};

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
    Loop *CreateNewLoop(BasicBlock *loopHeader);

private:
    void ResetLoopInfo();
    void CollectBackEdges();
    void BackEdgeSearch(BasicBlock *block);
    void ProcessNewBackEdge(BasicBlock *header, BasicBlock *backEdge);
    void FindAndInsertPreHeaders(Loop *loop);
    void MovePhiInputsToPreHeader(BasicBlock *header, BasicBlock *preHeader, const ArenaVector<int> &fwEdgesIndexes);
    void UpdateControlFlowWithPreHeader(BasicBlock *header, BasicBlock *preHeader,
                                        const ArenaVector<int> &fwEdgesIndexes);
    ArenaVector<int> GetForwardEdgesIndexes(BasicBlock *header);
    bool PreHeaderExists(Loop *loop);
    BasicBlock *CreatePreHeader(BasicBlock *header);
    void PopulateLoops();
    void PopulateIrreducibleLoop(Loop *loop);
    void NaturalLoopSearch(Loop *loop, BasicBlock *block);
    void SetLoopProperties(Loop *loop, uint32_t depth);

private:
    Marker blackMarker_ {};
    Marker grayMarker_ {};
    uint32_t loopCounter_ {0};
};

BasicBlock *GetLoopOutsideSuccessor(Loop *loop);
// NOLINTNEXTLINE(readability-redundant-declaration)
bool IsLoopSingleBackEdgeExitPoint(Loop *loop);

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LOOP_ANALYSIS_H
