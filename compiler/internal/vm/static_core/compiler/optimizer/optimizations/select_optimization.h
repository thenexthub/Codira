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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SELECT_OPTIMIZATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SELECT_OPTIMIZATION_H

#include "optimizer/ir/graph.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class BasicBlock;
class Graph;

class SelectOptimization : public Optimization {
public:
    explicit SelectOptimization(Graph *graph) : Optimization(graph), sameOperandsMap_(graph->GetAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(SelectOptimization);
    NO_COPY_SEMANTIC(SelectOptimization);
    ~SelectOptimization() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerSelectOptimization();
    }

    const char *GetPassName() const override
    {
        return "SelectOptimization";
    }

    void InvalidateAnalyses() override;

private:
    struct Key {
        std::array<Inst *, 3U> args;  // NOLINT(misc-non-private-member-variables-in-classes)
        uint64_t constant;            // NOLINT(misc-non-private-member-variables-in-classes)
        ConditionCode cc;             // NOLINT(misc-non-private-member-variables-in-classes)

        bool operator<(const Key &other) const;
    };
    ArenaMap<Key, Inst *> sameOperandsMap_;
    bool TryOptimizeSelectInst(Inst *selectInst);
    bool TryOptimizeSelectInstWithSameOperands(Inst *selectInst, Inst *v0, Inst *v1);
    bool TryOptimizeSelectInstWithSameOperandsChecked(Inst *selectInst, Inst *v0, Inst *v1, uint64_t const3,
                                                      ConditionCode cc);
    Inst *FindOrCreateXorI(Inst *selectInst, Inst *arg, uint64_t imm);
    void ReplaceUsersWithInvert(Inst *selectInst, Inst *arg);
    bool TryOptimizeSelectInstWithConstantBoolOperands(Inst *selectInst, Inst *v0, Inst *v1);
    // CC-OFFNXT(huge_cyclomatic_complexity[C], huge_method[C], G.FUN.01-CPP) big switch case
    bool TryOptimizeSelectInstWithConstantBoolOperandsChecked(Inst *selectInst, ConditionCode cc, uint64_t const0,
                                                              uint64_t const1, Inst *v2, uint64_t const3);
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SELECT_OPTIMIZATION_H
