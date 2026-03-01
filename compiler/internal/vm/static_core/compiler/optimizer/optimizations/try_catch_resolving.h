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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace ark::compiler {
class TryCatchResolving : public Optimization {
public:
    explicit TryCatchResolving(Graph *graph);
    NO_MOVE_SEMANTIC(TryCatchResolving);
    NO_COPY_SEMANTIC(TryCatchResolving);
    ~TryCatchResolving() override
    {
        tryBlocks_.clear();
        throwInsts_.clear();
        throwInsts0_.clear();
        catchBlocks_.clear();
        phiInsts_.clear();
        cphi2phi_.clear();
        catch2cphis_.clear();
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "TryCatchResolving";
    }
    void InvalidateAnalyses() override;

private:
    void DeoptimizeIfs();
    void CollectCandidates();
    BasicBlock *FindCatchBeginBlock(BasicBlock *bb);
    void VisitTryInst(TryInst *tryInst);
    void ConnectThrowCatch();
    void ConnectThrowCatchImpl(BasicBlock *catchBlock, BasicBlock *throwBlock, uint32_t catchPc, Inst *newObj,
                               Inst *thr0w);
    void DeleteTryCatchEdges(BasicBlock *tryBegin, BasicBlock *tryEnd);
    void RemoveCatchPhis(BasicBlock *cphisBlock, BasicBlock *catchBlock, Inst *throwInst, Inst *phiInst);
    void RemoveCatchPhisImpl(CatchPhiInst *catchPhi, BasicBlock *catchBlock, Inst *throwInst);

private:
    ArenaVector<BasicBlock *> tryBlocks_;
    ArenaVector<Inst *> throwInsts_;
    ArenaVector<Inst *> throwInsts0_;
    ArenaMap<uint32_t, BasicBlock *> catchBlocks_;
    ArenaMap<uint32_t, PhiInst *> phiInsts_;
    ArenaMap<CatchPhiInst *, PhiInst *> cphi2phi_;
    ArenaMap<BasicBlock *, BasicBlock *> catch2cphis_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
