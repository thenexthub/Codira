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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_TYPE_RESOLVING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_TYPE_RESOLVING_H

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace ark::compiler {
/*
 * PhiTypeResolving finds Phi instructions of Any type whose inputs are CastValueToAnyType instructions with the same
 * primitive type, and creates single CastValueToAnyType instructions with Phi of this primitive type as an input
 */
class PhiTypeResolving : public Optimization {
public:
    explicit PhiTypeResolving(Graph *graph);
    NO_MOVE_SEMANTIC(PhiTypeResolving);
    NO_COPY_SEMANTIC(PhiTypeResolving);
    ~PhiTypeResolving() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "PhiTypeResolving";
    }
    void InvalidateAnalyses() override;

private:
    bool CheckInputsAnyTypesRec(Inst *phi);
    void PropagateTypeToPhi();
    ArenaVector<Inst *> phis_;
    AnyBaseType anyType_ {AnyBaseType::UNDEFINED_TYPE};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_TYPE_RESOLVING_H
