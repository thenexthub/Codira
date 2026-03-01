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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINE_INTRINSICS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINE_INTRINSICS_H

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace ark::compiler {
/*
 * InlineIntrinsics(based on the results of the TypesAnalysis) tries to replace dynamic intrinsics to static
 * instructions For intrinsics that use bytecode profile for inlining (with flag inline_need_types set to true),
 * InlineIntrinsics collects assumed types(dynamic type) of intrinsics inputs:
 *  - If all inputs have undefined assumes type, the optimization isn't applied.
 *  - If some inputs have undefined assumed type they are assigned the assumed type of another input.
 * Thus each input has an assumed type.
 * Next, optimization tries to replace the intrinsic with static instructions, taking into account the types of inputs.
 * In case of success, the corresponding types are put in AnyTypeCheck, which are the inputs of the intrinsic
 */
class InlineIntrinsics : public Optimization {
public:
    explicit InlineIntrinsics(Graph *graph);
    NO_MOVE_SEMANTIC(InlineIntrinsics);
    NO_COPY_SEMANTIC(InlineIntrinsics);
    ~InlineIntrinsics() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "InlineIntrinsics";
    }
    void InvalidateAnalyses() override;
#include "intrinsics_inline.inl.h"

private:
    AnyBaseType GetAssumedAnyType(const Inst *inst);
    bool TryInline(CallInst *callInst);
    bool TryInline(IntrinsicInst *intrinsic);
    bool DoInline(IntrinsicInst *intrinsic);
    ArenaVector<AnyBaseType> types_;
    ArenaVector<Inst *> savedInputs_;
    ArenaVector<RuntimeInterface::NamedAccessProfileData> namedAccessProfile_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINE_INTRINSICS_H
