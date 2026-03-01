/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_ADJUST_ARRAY_REFS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_ADJUST_ARRAY_REFS_H

#include "optimizer/pass.h"
#include "optimizer/ir/basicblock.h"
#include "compiler_options.h"

namespace ark::compiler {
class AdjustRefs : public Optimization {
public:
    explicit AdjustRefs(Graph *graph);

    NO_MOVE_SEMANTIC(AdjustRefs);
    NO_COPY_SEMANTIC(AdjustRefs);
    ~AdjustRefs() override = default;

    bool RunImpl() override;
    bool IsEnable() const override
    {
        return g_options.IsCompilerAdjustRefs();
    }

    const char *GetPassName() const override
    {
        return "AdjustRefs";
    }

private:
    void ProcessArrayUses();
    void WalkChainDown(BasicBlock *bb, Inst *startFrom, Inst *head);
    void ProcessChain(Inst *head);
    void ProcessIndex(Inst *mem);
    void InsertMem(Inst *org, Inst *base, Inst *index, uint8_t scale);
    Inst *InsertPointerArithmetic(Inst *input, uint64_t imm, Inst *insertBefore, uint32_t pc, bool isAdd);

    void GetHeads();

    InstVector defs_;
    InstVector workset_;
    InstVector heads_;
    InstVector instsToReplace_;
    Marker blockEntered_ {};
    Marker blockProcessed_ {};
    Marker worksetMarker_ {};
    Loop *loop_ = {nullptr};
    bool added_ = {false};
};
}  // namespace ark::compiler

#endif
