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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
class PANDA_PUBLIC_API Licm : public Optimization {
public:
    explicit Licm(Graph *graph, uint32_t hoistLimit = std::numeric_limits<uint32_t>::max());
    NO_MOVE_SEMANTIC(Licm);
    NO_COPY_SEMANTIC(Licm);
    ~Licm() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerLicm();
    }

    const char *GetPassName() const override
    {
        return "LICM";
    }

    void InvalidateAnalyses() override;

    bool IsBlockLoopExit(BasicBlock *block);

private:
    bool IsLoopVisited(const Loop &loop) const;
    void VisitLoop(Loop *loop);
    bool IsInstHoistable(Inst *inst);
    void LoopSearchDFS(Loop *loop);
    bool InstDominatesLoopExits(Inst *inst);
    bool InstInputDominatesPreheader(Inst *inst);
    Inst *FindSaveStateForHoist(Inst *hoisted, const BasicBlock *preHeader, Inst **insertBefore);
    void TryAppendHoistableInst(Inst *inst, BasicBlock *block, Loop *loop);
    void UnmarkHoistUsers(Inst *inst);
    void MoveInstructions(BasicBlock *preHeader, Loop *loop);

private:
    const uint32_t hoistLimit_ {0};
    uint32_t hoistedInstCount_ {0};
    Marker markerLoopExit_ {UNDEF_MARKER};
    Marker markerHoistInst_ {UNDEF_MARKER};
    SaveStateBridgesBuilder ssb_;
    InstVector hoistableInstructions_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H
