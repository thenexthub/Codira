/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_TRANSFORM_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_TRANSFORM_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
enum class LoopExitPoint : uint8_t { ALL_LOOP, LOOP_EXIT_HEADER, LOOP_EXIT_BACKEDGE };

template <const LoopExitPoint EXIT_POINT>
class LoopTransform : public Optimization {
protected:
    explicit LoopTransform(Graph *graph) : Optimization(graph) {}

    virtual bool TransformLoop(Loop *loop) = 0;

    void RunLoopsVisitor()
    {
        GetGraph()->template RunPass<LoopAnalyzer>();
        ASSERT(GetGraph()->GetRootLoop() != nullptr);
        if (GetGraph()->GetRootLoop()->GetInnerLoops().empty()) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Graph doesn't have loops";
        }

        auto markerHolder = MarkerHolder(GetGraph());
        auto markerLoopExit = markerHolder.GetMarker();
        MarkLoopExits(GetGraph(), markerLoopExit);
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            LoopVisitLRN(loop, markerLoopExit);
        }
    }

    bool IsSupportedLoopType(const Loop *loop)
    {
        if (loop->IsIrreducible()) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Irreducible loop isn't visited, id = " << loop->GetId();
            return false;
        }
        if (loop->IsOsrLoop()) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "OSR entry isn't visited, loop id = " << loop->GetId();
            return false;
        }
        if (loop->IsTryCatchLoop()) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Try-catch loop isn't visited, loop id = " << loop->GetId();
            return false;
        }
        if (loop->GetBackEdges().size() > 1) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                << "Loop with more than 1 back-edge isn't visited, id = " << loop->GetId();
            return false;
        }
        return true;
    }

    bool LoopVisitLRN(Loop *loop, Marker marker)
    {
        ASSERT(loop != nullptr);
        const auto &innerLoops = loop->GetInnerLoops();
        bool result = true;
        for (auto innerLoop : innerLoops) {
            result &= LoopVisitLRN(innerLoop, marker);
        }

        if (result && IsSupportedLoopType(loop)) {
            return VisitLoop(loop, marker);
        }
        return false;
    }

#ifndef __clang_analyzer__
    bool VisitBlockInLoop(BasicBlock *block, Loop *loop, Marker marker)
    {
        if constexpr (EXIT_POINT == LoopExitPoint::LOOP_EXIT_HEADER) {
            // NOTE (a.popov) Support infinite loops unrolling
            if (!block->IsMarked(marker) && block->IsLoopHeader()) {
                COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                    << "Loop without exit-point from loop-header isn't visited, id = " << loop->GetId();
                return false;
            }
            if (block->IsMarked(marker) && !block->IsLoopHeader()) {
                COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                    << "Loop with loop-exit not from loop-header isn't visited, id = " << loop->GetId();
                return false;
            }
        } else if constexpr (EXIT_POINT == LoopExitPoint::LOOP_EXIT_BACKEDGE) {
            ASSERT(loop->GetBackEdges().size() == 1);
            auto back_edge = loop->GetBackEdges()[0];
            if (!block->IsMarked(marker) && block == back_edge) {
                COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                    << "Loop without exit-point from back-edge isn't visited, id = " << loop->GetId();
                return false;
            }
            if (block->IsMarked(marker) && block != back_edge) {
                COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                    << "Loop with loop-exit not from last block isn't visited, id = " << loop->GetId();
                return false;
            }
        }
        return true;
    }
#endif

    bool VisitLoop(Loop *loop, [[maybe_unused]] Marker marker)
    {
#ifndef __clang_analyzer__
        if constexpr (EXIT_POINT != LoopExitPoint::ALL_LOOP) {
            for (auto block : loop->GetBlocks()) {
                if (!VisitBlockInLoop(block, loop, marker)) {
                    return false;
                }
            }
        }
#endif
        return TransformLoop(loop);
    }

    BasicBlock *GetLoopOuterBlock(BasicBlock *exitBlock)
    {
        ASSERT(exitBlock->GetSuccsBlocks().size() == 2U);
        auto loop = exitBlock->GetLoop();
        auto outer = exitBlock->GetTrueSuccessor();
        if (outer->GetLoop() == loop) {
            outer = exitBlock->GetFalseSuccessor();
        }
        ASSERT(outer->GetLoop() != loop);
        return outer;
    }
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_TRANSFORM_H
