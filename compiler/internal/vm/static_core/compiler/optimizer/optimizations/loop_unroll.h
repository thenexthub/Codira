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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H

#include "optimizer/optimizations/loop_transform.h"
#include "compiler_options.h"

namespace ark::compiler {
/**
 * Loop unroll optimization
 * @param inst_limit - the maximum number of loop instructions after its unrolling
 * @param unroll_factor - the number of loop body copies including the original one
 */
class LoopUnroll : public LoopTransform<LoopExitPoint::LOOP_EXIT_BACKEDGE> {
    struct UnrollParams {
        uint32_t unrollFactor;
        uint32_t cloneableInsts;
        bool hasCall;
    };

public:
    LoopUnroll(Graph *graph, uint32_t instLimit, uint32_t unrollFactor)
        : LoopTransform(graph), instLimit_(instLimit), unrollFactor_(unrollFactor)
    {
    }

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerLoopUnroll();
    }

    const char *GetPassName() const override
    {
        return "LoopUnroll";
    }

    void InvalidateAnalyses() override;
    static bool HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loopInfo);

private:
    bool TransformLoop(Loop *loop) override;
    bool UnrollWithBranching(uint32_t unrollFactor, Loop *loop, std::optional<CountableLoopInfo> loopInfo,
                             std::optional<uint64_t> optIterations);
    UnrollParams GetUnrollParams(Loop *loop);
    void TransformLoopImpl(Loop *loop, std::optional<uint64_t> optIterations, bool noSideExits, uint32_t unrollFactor,
                           std::optional<CountableLoopInfo> loopInfo);
    void FixCompareInst(const CountableLoopInfo &loopInfo, BasicBlock *header, uint32_t unrollFactor);
    Inst *CreateNewTestInst(const CountableLoopInfo &loopInfo, Inst *constInst, Inst *preHeaderCmp);

private:
    const uint32_t instLimit_ {0};
    const uint32_t unrollFactor_ {0};
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H
