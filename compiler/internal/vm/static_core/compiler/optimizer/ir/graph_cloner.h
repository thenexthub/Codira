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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H
#define COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H

#include "inst.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/hash.h"

namespace ark::compiler {
class BasicBlock;
class Graph;
class Inst;

// NOLINTNEXTLINE(readability-redundant-declaration)
bool IsLoopSingleBackEdgeExitPoint(Loop *loop);

enum UnrollType : uint8_t {
    UNROLL_WITH_SIDE_EXITS = 0,
    UNROLL_WITHOUT_SIDE_EXITS = 1,
    UNROLL_REMOVE_BACK_EDGE = 2,
    UNROLL_POST_INCREMENT = UNROLL_REMOVE_BACK_EDGE,
    UNROLL_CONSTANT_ITERATIONS = UNROLL_WITHOUT_SIDE_EXITS | UNROLL_REMOVE_BACK_EDGE
};

enum class InstCloneType : uint8_t { CLONE_ALL, CLONE_INSTS };

enum class CloneEdgeType : uint8_t { EDGE_PRED, EDGE_SUCC };

/**
 * Helper-class, provides methods to:
 * - Clone the whole graph;
 * - Clone loop;
 * - Unroll loop;
 * - Peel loop;
 */
class GraphCloner {
    using PhiInputsMap = ArenaUnorderedMap<Inst *, Inst *>;

    struct LoopUnrollData {
        BasicBlock *header {nullptr};
        BasicBlock *backedge {nullptr};
        BasicBlock *exitBlock {nullptr};
        BasicBlock *outer {nullptr};
        ArenaVector<BasicBlock *> *blocks {nullptr};
        InstVector *phiUpdateInputs {nullptr};
        PhiInputsMap *phiReplacedInputs {nullptr};
    };

protected:
    struct LoopClonerData {
        BasicBlock *outer {nullptr};
        BasicBlock *header {nullptr};
        BasicBlock *preHeader {nullptr};
        ArenaVector<BasicBlock *> *blocks {nullptr};
    };

public:
    PANDA_PUBLIC_API explicit GraphCloner(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *localAllocator);

    PANDA_PUBLIC_API Graph *CloneGraph();
    BasicBlock *CloneLoopHeader(BasicBlock *block, BasicBlock *outer, BasicBlock *replaceablePred);
    Loop *CloneLoop(Loop *loop);

    /**
     * Make equal to the `factor` number of clones of loop body and insert them into the graph
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |    [loop-body]<----\
     *      |        |   |       |
     *      |        |   \-------/
     *      |        |
     *      |        v
     *      \--->[outside-block]
     *               |
     *               v
     *              ...
     *
     * Cloning with side-exits:
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |   [loop-body]---------\
     *      |        |              |
     *      |        v              |
     *      |   [loop-body']------->|
     *      |        |              |
     *      |        v              |
     *      |   [loop-body'']------>|
     *      |        |              |
     *      |        v              |
     *      |  [resolver-block]<----/
     *      |        |
     *      |        v
     *      \--->[outside-block]
     *              ...
     *
     *  Cloning without side-exits:
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |    [loop-body]
     *      |        |
     *      |        v
     *      |    [loop-body']
     *      |        |
     *      |        v
     *      |    [loop-body'']
     *      |        |
     *      |        v
     *      \--->[outside-block]
     */
    template <UnrollType TYPE>
    void UnrollLoopBody(Loop *loop, size_t factor)
    {
        ASSERT_PRINT(IsLoopSingleBackEdgeExitPoint(loop), "Cloning blocks doesn't have single entry/exit point");
        auto markerHolder = MarkerHolder(GetGraph());
        cloneMarker_ = markerHolder.GetMarker();
        auto unrollData = PrepareLoopToUnroll(loop, (TYPE & UnrollType::UNROLL_WITHOUT_SIDE_EXITS) == 0);

        ASSERT(factor > 0);
        auto cloneCount = factor - 1;
        for (size_t i = 0; i < cloneCount; i++) {
            ASSERT(unrollData != nullptr);
            CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, true>(*unrollData->blocks, GetGraph());
            BuildLoopUnrollControlFlow(unrollData);
            // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
            if constexpr ((TYPE & UnrollType::UNROLL_WITHOUT_SIDE_EXITS) != 0) {
                // Users update should be done on the last no-side-exits unroll iteration
                // before building loop data-flow
                if (i + 1 == cloneCount) {  // last_iteration
                    UpdateUsersAfterNoSideExitsUnroll(unrollData);
                }
            }
            BuildLoopUnrollDataFlow(unrollData);
        }

        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr ((TYPE & UnrollType::UNROLL_REMOVE_BACK_EDGE) != 0) {
            RemoveLoopBackEdge(unrollData);
        }
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr (TYPE == UnrollType::UNROLL_CONSTANT_ITERATIONS) {
            RemoveLoopPreHeader(unrollData);
        }

        // Ensure DominatorsTree is valid after CloneBlocksAndInstructions adds new blocks to the graph
        // and BuildLoopUnrollControlFlow / BuildLoopUnrollDataFlow modify control and data flow
        GetGraph()->GetValidAnalysis<DominatorsTree>();

        ssb_.FixPhisWithCheckInputs(unrollData->outer);
    }

protected:
    Graph *GetGraph()
    {
        return graph_;
    }

    BasicBlock *GetClone(const BasicBlock *block)
    {
        ASSERT(block != nullptr);
        ASSERT_PRINT(block->GetGraph() == GetGraph(), "GraphCloner probably caught disconnected block");
        ASSERT_DO(HasClone(block), block->Dump(&std::cerr));
        return cloneBlocks_[block->GetId()];
    }

    template <CloneEdgeType EDGE_TYPE>
    void CloneEdges(BasicBlock *block)
    {
        auto clone = GetClone(block);

        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr (EDGE_TYPE == CloneEdgeType::EDGE_SUCC) {
            auto blockEdges = &block->GetSuccsBlocks();
            auto cloneEdges = &clone->GetSuccsBlocks();
            ASSERT(cloneEdges->empty());
            cloneEdges->reserve(blockEdges->size());
            for (auto edge : *blockEdges) {
                cloneEdges->push_back(GetClone(edge));
            }
        } else {
            auto blockEdges = &block->GetPredsBlocks();
            auto cloneEdges = &clone->GetPredsBlocks();

            ASSERT(cloneEdges->empty());
            cloneEdges->reserve(blockEdges->size());
            for (auto edge : *blockEdges) {
                cloneEdges->push_back(GetClone(edge));
            }
        }
    }

    /**
     * For each block of input vector create a new one empty block and populate it with the instructions,
     * cloned form the original block
     */
    template <InstCloneType TYPE, bool SKIP_SAFEPOINTS>
    void CloneBlocksAndInstructions(const ArenaVector<BasicBlock *> &blocks, Graph *targetGraph)
    {
        cloneBlocks_.clear();
        cloneBlocks_.resize(GetGraph()->GetVectorBlocks().size(), nullptr);
        cloneInstructions_.clear();
        size_t instCount = 0;
        for (const auto &block : blocks) {
            if (block != nullptr) {
                auto clone = block->Clone(targetGraph);
                cloneBlocks_[block->GetId()] = clone;
                CloneInstructions<TYPE, SKIP_SAFEPOINTS>(block, clone, &instCount);
                if (block->IsTryBegin()) {
                    clone->SetTryBegin(true);
                    targetGraph->AppendTryBeginBlock(clone);
                }
                if (block->IsTryEnd()) {
                    clone->SetTryEnd(true);
                }
                if (block->IsTry()) {
                    clone->SetTry(true);
                }
                if (block->IsCatch()) {
                    clone->SetCatch(true);
                }
            }
        }
    }

    void MakeLoopCloneInfo(LoopClonerData *unrollData);
    LoopClonerData *PrepareLoopToClone(Loop *loop);
    void ProcessMarkedInsts(LoopClonerData *data);
    BasicBlock *CreateNewOutsideSucc(BasicBlock *outsideSucc, BasicBlock *backEdge, BasicBlock *preHeader);

    /**
     * Use the following rules cloning the inputs:
     * - if input of the original instruction has clone - insert this clone as input
     * - otherwise - use original input as clone instruction input
     */
    template <bool REPLACE_HEADER_PHI>
    void SetCloneInputs(Inst *inst, BasicBlock *block = nullptr)
    {
        auto clone = GetClone(inst);
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto input = inst->GetInput(i).GetInst();
            if (REPLACE_HEADER_PHI && IsInstLoopHeaderPhi(input, inst->GetBasicBlock()->GetLoop())) {
                ASSERT(block != nullptr);
                input = input->CastToPhi()->GetPhiInput(block);
            } else if (HasClone(input)) {
                input = GetClone(input);
            }

            if (clone->IsOperandsDynamic()) {
                clone->AppendInput(input);
                if (clone->IsSaveState()) {
                    static_cast<SaveStateInst *>(clone)->SetVirtualRegister(
                        i, static_cast<SaveStateInst *>(inst)->GetVirtualRegister(i));
                } else if (clone->IsPhi()) {
                    clone->CastToPhi()->SetPhiInputBbNum(i, inst->CastToPhi()->GetPhiInputBbNum(i));
                }
            } else {
                clone->SetInput(i, input);
            }
        }
    }

    void UpdateCaller(Inst *inst);

    Inst *GetClone(Inst *inst)
    {
        ASSERT(inst != nullptr);
        ASSERT(HasClone(inst));

        // We don't use clone_marker_ when cloning the whole graph, so lets at least check the basic block here
        ASSERT(inst->GetBasicBlock() != nullptr);
        ASSERT_PRINT(inst->GetBasicBlock()->GetGraph() == GetGraph(),
                     "GraphCloner probably caught an instruction from disconnected block");
        // Empty clone_blocks_ means we are cloning only one basic block
        ASSERT(cloneBlocks_.empty() || HasClone(inst->GetBasicBlock()));

        return cloneInstructions_[inst->GetCloneNumber()];
    }

    // Cloned blocks and instructions getters
    bool HasClone(const BasicBlock *block)
    {
        return (block->GetId() < cloneBlocks_.size()) && (cloneBlocks_[block->GetId()] != nullptr);
    }

    bool HasClone(Inst *inst)
    {
        return inst->IsMarked(cloneMarker_) && (inst->GetCloneNumber() < cloneInstructions_.size());
    }

private:
    // Whole graph cloning
    void CopyLoop(Loop *loop, Loop *clonedLoop);
    void CloneLinearOrder(Graph *newGraph);
    void BuildControlFlow();
    void BuildTryCatchLogic(Graph *newGraph);
    void BuildDataFlow();
    void CloneConstantsInfo(Graph *newGraph);
    void CloneAnalyses(Graph *newGraph);
    // Loop cloning
    void SplitPreHeader(Loop *loop);
    GraphCloner::LoopClonerData *PopulateLoopClonerData(Loop *loop, BasicBlock *preHeader, BasicBlock *outsideSucc);
    void BuildLoopCloneControlFlow(LoopClonerData *unrollData);
    void BuildLoopCloneDataFlow(LoopClonerData *unrollData);
    // Unroll cloning
    LoopUnrollData *PrepareLoopToUnroll(Loop *loop, bool cloneSideExits);
    BasicBlock *CreateResolverBlock(Loop *loop, BasicBlock *backEdge);
    BasicBlock *SplitBackEdge(LoopUnrollData *unrollData, Loop *loop, BasicBlock *backEdge);
    Inst *GetCompareInst(Inst *ifimm);
    void UpdateUsersAfterNoSideExitsUnroll(const LoopUnrollData *unrollData);
    void UpdateOutloopUsers(Loop *loop, Inst *inst);
    void BuildLoopUnrollControlFlow(LoopUnrollData *unrollData);
    void BuildLoopUnrollDataFlow(LoopUnrollData *unrollData);
    void RemoveLoopBackEdge(const LoopUnrollData *unrollData);
    void RemoveLoopPreHeader(const LoopUnrollData *unrollData);
    // Loop header cloning
    void BuildClonedLoopHeaderDataFlow(const BasicBlock &block, BasicBlock *resolver, BasicBlock *clone);
    void UpdateUsersForClonedLoopHeader(Inst *inst, BasicBlock *outerBlock);

    /// Clone block's instructions and append to the block's clone
    template <InstCloneType TYPE, bool SKIP_SAFEPOINTS>
    void CloneInstructions(const BasicBlock *block, BasicBlock *clone, size_t *instCount)
    {
        for (auto inst : block->Insts()) {
            if constexpr (SKIP_SAFEPOINTS) {  // NOLINT
                if (inst->GetOpcode() == Opcode::SafePoint) {
                    continue;
                }
            }
            if constexpr (TYPE != InstCloneType::CLONE_ALL) {  // NOLINT
                if (inst->GetOpcode() == Opcode::NOP) {
                    continue;
                }
            }

            ASSERT(clone != nullptr);
            auto *clonedInst = CloneInstruction(inst, instCount, clone->GetGraph());

            cloneInstMap_[inst] = clonedInst;
            cloneInstMap_[clonedInst] = inst;

            clone->AppendInst(clonedInst);
        }

        if constexpr (TYPE == InstCloneType::CLONE_ALL) {  // NOLINT
            for (auto phi : block->PhiInsts()) {
                auto phiClone = CloneInstruction(phi, instCount, clone->GetGraph());
                cloneInstMap_[phi] = phiClone;
                cloneInstMap_[phiClone] = phi;
                clone->AppendPhi(phiClone->CastToPhi());
            }
        }
    }

    /// Clone instruction and mark both: clone and cloned
    Inst *CloneInstruction(Inst *inst, size_t *instCount, Graph *targetGraph)
    {
        inst->SetCloneNumber((*instCount)++);
        auto instClone = inst->Clone(targetGraph);
        cloneInstructions_.push_back(instClone);
        inst->SetMarker(cloneMarker_);
        instClone->SetMarker(cloneMarker_);
        if (inst->GetBasicBlock()->GetGraph() != targetGraph) {
            instClone->SetId(inst->GetId());
        }
        return instClone;
    }

    bool IsInstLoopHeaderPhi(Inst *inst, Loop *loop);

    // clone throwable inst -> catch blocks mapping
    void CloneThrowableInstHandlers(Graph *newGraph);

protected:
    Marker cloneMarker_ {UNDEF_MARKER};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Graph *graph_;
    ArenaAllocator *allocator_;
    ArenaAllocator *localAllocator_;
    ArenaVector<BasicBlock *> cloneBlocks_;
    InstVector cloneInstructions_;
    SaveStateBridgesBuilder ssb_;
    ArenaMap<const Inst *, const Inst *> cloneInstMap_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H
