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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_OPTIMIZE_STRING_CONCAT_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_OPTIMIZE_STRING_CONCAT_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace ark::compiler {

/// Optimize String.concat calls by replacing it with StringBuilder.append

class OptimizeStringConcat : public Optimization {
public:
    explicit OptimizeStringConcat(Graph *graph);

    NO_MOVE_SEMANTIC(OptimizeStringConcat);
    NO_COPY_SEMANTIC(OptimizeStringConcat);
    ~OptimizeStringConcat() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerOptimizeStringConcat();
    }

    const char *GetPassName() const override
    {
        return "OptimizeStringConcat";
    }
    void InvalidateAnalyses() override;

private:
    void CreateAppendArgsIntrinsic(Inst *instance, Inst *arg, SaveStateInst *saveState);
    void CreateAppendArgsIntrinsics(Inst *instance, SaveStateInst *saveState);
    void CreateAppendArgsIntrinsics(Inst *instance, Inst *args, uint64_t arrayLengthValue, SaveStateInst *saveState);
    BasicBlock *CreateAppendArgsLoop(Inst *instance, Inst *str, Inst *args, LengthMethodInst *arrayLength,
                                     Inst *concatCall);
    void FixBrokenSaveStates(Inst *source, Inst *target);
    bool HasStoreArrayUsersOnly(Inst *newArray, Inst *removable);
    void ReplaceStringConcatWithStringBuilderAppend(Inst *concatCall);

private:
    SaveStateBridgesBuilder ssb_ {};
    InstVector arrayElements_;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_OPTIMIZE_STRING_CONCAT_H
