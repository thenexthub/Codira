/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_H

#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"

namespace ark::compiler {
class Graph;

/**
 * Some intrinsics with String arguments can work with flat strings only.
 * This pass does following:
 * 1. inserts StringFlatCheck IR instruction between String input and specific intrinsic
 * 2. if several intrinsics use the same argument, moves corresponding StringFlatCheck to the common dominating basic
 * block
 * 2. updates users of original input which are dominated by StringFlatCheck
 * Note: StringFlatCheck instruction and its users should not be separated by OSR entry since it is possible that
 * StringFlatCheck was not executed in compiled code.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StringFlatCheck : public Optimization, GraphVisitor {
public:
    static constexpr const char *NAME = "StringFlatCheck";
    explicit StringFlatCheck(Graph *graph);

    NO_MOVE_SEMANTIC(StringFlatCheck);
    NO_COPY_SEMANTIC(StringFlatCheck);
    ~StringFlatCheck() override = default;

    bool RunImpl() override;

    bool IsEnable() const override;

    static bool IsEnable(RuntimeInterface *runtime);

    const char *GetPassName() const override
    {
        return NAME;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    bool SeparatedByOsrEntry(BasicBlock *bb1, BasicBlock *bb2);
    void InsertStringFlatCheck(IntrinsicInst *intrinsic, uint32_t stringFlatCheckArgMask);
    Inst *GetStringFlatCheckUser(IntrinsicInst *intrinsic, Inst *inst);
    bool MoveThroughDominationTree(Inst *flatCheck, IntrinsicInst *intrinsic);
    Inst *InsertInputStringFlatCheck(IntrinsicInst *intrinsic, Inst *inputInst);
    void ReplaceUsers(Inst *inputInst, Inst *flatCheck);
    bool CanUpdateInput(Inst *userInst, Inst *flatCheck);
    void InsertStringFlatCheckSaveState(Inst *flatCheck, SaveStateInst *saveState);
    void FixSaveStates();
    ArenaVector<std::pair<size_t, Inst *>> users_;
    bool applied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_H
