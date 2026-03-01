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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_HOTNESS_PROPAGATION_H
#define COMPILER_OPTIMIZER_ANALYSIS_HOTNESS_PROPAGATION_H

#include <optional>
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

class HotnessPropagation {
public:
    explicit HotnessPropagation(Graph *graph) : graph_(graph) {}

    void Run()
    {
        if (graph_->HasIrreducibleLoop()) {
            return;
        }
        auto markerHolder = MarkerHolder(graph_);
        mrk_ = markerHolder.GetMarker();
        auto *eb = graph_->GetEndBlock();
        auto *bb = graph_->GetStartBlock();
        if (eb != nullptr) {
            auto retHotness = EnsureBackedgeResolvable<true>(eb, nullptr);
            bb->SetHotness(retHotness);
        }
        bb->SetMarker(mrk_);

        ASSERT(bb->GetPredsBlocks().empty());
        ASSERT(bb->GetSuccsBlocks().size() == 1);

        PropagateHotness(bb->GetSuccessor(0));
    }

private:
    void PropagateHotness(BasicBlock *bb)
    {
        if (!bb->IsMarked(mrk_)) {
            // The block hotness is unknown, resolve it:
            if (bb->IsLoopHeader()) {
                auto canPropagateToOuterSuccessor = FindLoopHeaderHotness(bb);
                if (!canPropagateToOuterSuccessor) {
                    return;
                }
            }

            // Check all preds are resolved, if not - come back here later via currently unresolved pred:
            for (auto *p : bb->GetPredsBlocks()) {
                ASSERT(p->GetSuccsBlocks().size() != 2U || p->IsTryBegin() || p->IsTryCatch() ||
                       p->GetLastInst()->GetOpcode() == Opcode::IfImm || p->GetLastInst()->GetOpcode() == Opcode::If ||
                       p->GetLastInst()->GetOpcode() == Opcode::AddOverflow ||
                       p->GetLastInst()->GetOpcode() == Opcode::SubOverflow);

                auto predLastOpcode = p->GetLastInst() == nullptr ? Opcode::INVALID : p->GetLastInst()->GetOpcode();
                bool predIsBranch = (predLastOpcode == Opcode::IfImm) || (predLastOpcode == Opcode::If);
                if (!p->IsMarked(mrk_) && !p->IsCatch() && !predIsBranch) {
                    return;
                }
            }

            size_t predsHtn = 0;
            for (auto *p : bb->GetPredsBlocks()) {
                predsHtn += GetEdgeHotness(p, bb);
            }
            bb->SetHotness(predsHtn);
            bb->SetMarker(mrk_);
        }

        if (bb->IsTryBegin() || bb->IsTryEnd()) {
            // Do not propagate hotness to catch blocks:
            if (bb->GetSuccsBlocks().size() > 1) {
                auto *tb = bb->GetSuccessor(0);
                if (!tb->IsMarked(mrk_)) {
                    PropagateHotness(tb);
                }
            }
        } else {
            ASSERT(bb->GetSuccsBlocks().size() <= 2U);
            for (auto *s : bb->GetSuccsBlocks()) {
                if (!s->IsMarked(mrk_)) {
                    PropagateHotness(s);
                }
            }
        }
    }

    bool FindLoopHeaderHotness(BasicBlock *loopHeader)
    {
        // Check that loop has conditional branches (i.e. has profile info).
        // There may be loops in try-block without conditional branches, they considered as finite, so
        // it is neccessary to conservatively check the loop if a graph has try-begin construction.
        if (loopHeader->GetLoop()->IsInfinite() || !loopHeader->GetGraph()->GetTryBeginBlocks().empty()) {
            if (!ProcessInfiniteLoop(loopHeader->GetLoop())) {
                return false;
            }
        }
        auto ol = loopHeader->GetLoop()->GetOuterLoop();
        bool allNonBackEdgesPredsVisited = true;
        for (auto *p : loopHeader->GetPredsBlocks()) {
            if (ol == p->GetLoop()) {
                allNonBackEdgesPredsVisited &= p->IsMarked(mrk_);
            } else {
                if (!p->IsMarked(mrk_)) {
                    // This should be resolveable for each non-infinite-loop because of existance of loop exits.
                    // The result is not used here. There are two scenarios:
                    // 1. `p` contains `IfInst`, so later actual profiled info will be loaded (again).
                    // 2. `loopHeader` is the single successor of `p` so `p` hotness will be resolved and stored in
                    // p::hotness_ and `p` will be marked.
                    EnsureBackedgeResolvable(p, loopHeader);
                }
            }
        }
        return allNonBackEdgesPredsVisited;
    }

    bool ProcessInfiniteLoop(Loop *l)
    {
        bool hasBranches = false;
        auto checkHasBranches = [&hasBranches](Loop *loop) {
            for (auto *loopBlock : loop->GetBlocks()) {
                if ((loopBlock->GetSuccsBlocks().size() == 2U) && (loopBlock->GetLastInst() != nullptr)) {
                    auto opc = loopBlock->GetLastInst()->GetOpcode();
                    hasBranches |= (opc == Opcode::If || opc == Opcode::IfImm) && !loopBlock->IsCatch();
                }
            }
        };

        int64_t infLoopHotness = 0;
        // Avoid catch-loops (i.e. when catch handler contains associated try-block itself):
        if (!l->GetHeader()->IsCatchBegin()) {
            checkHasBranches(l);
            if (!hasBranches) {
                for (auto *innerLoop : l->GetInnerLoops()) {
                    checkHasBranches(innerLoop);
                }
            }
            infLoopHotness = std::numeric_limits<int64_t>::max() / 2U;
        }

        if (!hasBranches) {
            // Set some large value to mark infinite loop with no profile info:
            auto setInfiniteHotness = [=](Loop *loop) {
                for (auto *loopBlock : loop->GetBlocks()) {
                    loopBlock->SetHotness(infLoopHotness);
                    loopBlock->SetMarker(mrk_);
                }
            };
            setInfiniteHotness(l);
            for (auto *innerLoop : l->GetInnerLoops()) {
                setInfiniteHotness(innerLoop);
            }
            return false;
        }
        return true;
    }

    size_t GetEdgeHotness(BasicBlock *pred, BasicBlock *succ)
    {
        ASSERT(std::find(pred->GetSuccsBlocks().begin(), pred->GetSuccsBlocks().end(), succ) !=
               pred->GetSuccsBlocks().end());
        if (pred->IsCatch()) {
            return 0;
        }
        if (pred->IsTryBegin() || pred->IsTryEnd()) {
            return pred->GetHotness();
        }
        if (pred->GetSuccsBlocks().size() == 2U) {
            ASSERT(pred->GetTrueSuccessor() == succ || pred->GetFalseSuccessor() == succ);
            return pred->GetGraph()->GetBranchCounter(pred, pred->GetTrueSuccessor() == succ);
        }
        ASSERT(pred->GetSuccsBlocks().size() == 1);
        return pred->GetHotness();
    }

    std::optional<size_t> TryGetProfiledEdge(BasicBlock *pred, BasicBlock *succ)
    {
        if (pred->IsCatch()) {
            // Catch-blocks are considered as cold blocks:
            return 0;
        }
        // This algo shouldn't reach catch-begin:
        ASSERT(!pred->IsCatchBegin());

        if (pred->IsTryEnd() || pred->IsTryBegin()) {
            // Try-end and try-begin are transitive blocks:
            return std::nullopt;
        }
        auto pli = pred->GetLastInst();
        if (pli != nullptr && (pli->GetOpcode() == Opcode::AddOverflow || pli->GetOpcode() == Opcode::SubOverflow)) {
            if (pred->GetTrueSuccessor() == succ) {
                return 0;
            }
            ASSERT(pred->GetFalseSuccessor() == succ);
            return std::nullopt;
        }
        switch (pred->GetSuccsBlocks().size()) {
            case 0:
            case 1: {
                return std::nullopt;
            }
            case 2U: {
                if (pred->GetGraph()->IsThrowApplied() && pli != nullptr && pli->GetOpcode() == Opcode::Throw) {
                    return 0;
                }
                ASSERT(pli != nullptr && (pli->GetOpcode() == Opcode::If || pli->GetOpcode() == Opcode::IfImm));
                ASSERT(pred->GetTrueSuccessor() == succ || pred->GetFalseSuccessor() == succ);
                return pred->GetGraph()->GetBranchCounter(pred, pred->GetTrueSuccessor() == succ);
            }
        }

        UNREACHABLE();
    }

    template <bool IS_END_BLOCK = false>
    size_t EnsureBackedgeResolvable(BasicBlock *pred, [[maybe_unused]] BasicBlock *succ)
    {
        ASSERT(pred != nullptr);
        ASSERT(!pred->IsMarked(mrk_));
        if constexpr (!IS_END_BLOCK) {
            auto res = TryGetProfiledEdge(pred, succ);
            if (res) {
                // There is immediate branch, return actual profiled info:
                return res.value();
            }
        } else {
            ASSERT(succ == nullptr);
        }

        // Otherwise, it is a transition-block (i.e. have only one successor), resolve its hotness firstly and return
        // it:
        if constexpr (!IS_END_BLOCK) {
            ASSERT(pred->GetSuccsBlocks().size() == 1 || pred->IsTryBegin() || pred->IsTryEnd() ||
                   pred->GetLastInst()->GetOpcode() == Opcode::AddOverflow ||
                   pred->GetLastInst()->GetOpcode() == Opcode::SubOverflow ||
                   (pred->GetGraph()->IsThrowApplied() && pred->IsEndWithThrow()));
        }
        size_t sumPreds = 0;
        for (auto *newPred : pred->GetPredsBlocks()) {
            sumPreds += EnsureBackedgeResolvable(newPred, pred);
        }

        // These blocks can be safely marked as each of them end up in a backedge and cannot transfer CF out of loop
        // (so no side-exits skipped):
        pred->SetMarker(mrk_);
        pred->SetHotness(sumPreds);

        return sumPreds;
    }

private:
    Marker mrk_ {UNDEF_MARKER};
    Graph *graph_ {};
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_HOTNESS_PROPAGATION_H
