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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_RESERVE_STRING_BUILDER_BUFFER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_RESERVE_STRING_BUILDER_BUFFER_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace ark::compiler {

using BlockWeightsMap = ArenaMap<BasicBlock *, uint64_t>;
using BlockPredicate = std::function<bool(BasicBlock *)>;

class ReserveStringBuilderBuffer : public Optimization {
public:
    explicit ReserveStringBuilderBuffer(Graph *graph);

    NO_MOVE_SEMANTIC(ReserveStringBuilderBuffer);
    NO_COPY_SEMANTIC(ReserveStringBuilderBuffer);
    ~ReserveStringBuilderBuffer() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerReserveStringBuilderBuffer();
    }

    const char *GetPassName() const override
    {
        return "ReserveStringBuilderBuffer";
    }

private:
    bool isApplied_ {false};
    uint64_t tlabArraySizeMax_ {0};
    BlockWeightsMap blockWeightsMap_;
    SaveStateBridgesBuilder ssb_ {};

    uint64_t FindLongestPathLength(Inst *instance, Loop *loop, Marker visited);
    uint64_t FindLongestPathLength(
        Inst *instance, BasicBlock *block, Marker visited,
        const BlockPredicate &stopAtBlock = [](auto) { return false; });

    void ReplaceInitialBufferSizeConstantInlined(Inst *instance, uint64_t appendCallsCount);
    void ReplaceInitialBufferSizeConstantNotInlined(Inst *instance, uint64_t appendCallsCount);
    uint64_t CountStringBuilderAppendCalls(Inst *instance);
    uint64_t GetBufferSizeMin(Inst *instance) const;
    uint64_t GetBufferSizeMax() const;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_RESERVE_STRING_BUILDER_BUFFER_H
