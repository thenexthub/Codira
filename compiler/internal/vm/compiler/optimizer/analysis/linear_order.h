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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LINEAR_ORDER_H
#define COMPILER_OPTIMIZER_ANALYSIS_LINEAR_ORDER_H

#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
class BasicBlock;
class Graph;

/*
 * For each `If` block place its false-successor in the next position of the `linear_blocks_` vector:
 * - inverse type of IfInst if true-successor is placed first;
 * - marks `If` block with `JumpFlag` (so `Codegen` could insert `jmp`) if there are no `If` successors
 *   placed just after it;
 */
class LinearOrder : public Analysis {
public:
    explicit LinearOrder(Graph *graph);

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "BlocksLinearOrder";
    }

    ArenaVector<BasicBlock *> &GetBlocks()
    {
        return linear_blocks_;
    }

    const ArenaVector<BasicBlock *> &GetBlocks() const
    {
        return linear_blocks_;
    }

    NO_MOVE_SEMANTIC(LinearOrder);
    NO_COPY_SEMANTIC(LinearOrder);
    ~LinearOrder() override = default;

private:
    void HandlePrevInstruction(BasicBlock *block, BasicBlock *prev_block);
    void HandleIfBlock(BasicBlock *if_true_block, BasicBlock *next_block);
    template <class T>
    void MakeLinearOrder(const T &blocks);

private:
    BasicBlock *LeastLikelySuccessor(const BasicBlock *block);
    // similar to DFS but move least frequent branch to the end
    template <bool defer_least_frequent>
    void DFSAndDeferLeastFrequentBranches(BasicBlock *block, size_t *blocks_count);
    Marker marker_ {UNDEF_MARKER};
    ArenaVector<BasicBlock *> linear_blocks_;
    ArenaList<BasicBlock *> rpo_blocks_;
    ArenaVector<BasicBlock *> reordered_blocks_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LINEAR_ORDER_H
