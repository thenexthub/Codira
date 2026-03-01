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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_RPO_H
#define COMPILER_OPTIMIZER_ANALYSIS_RPO_H

#include "libarkbase/utils/arena_containers.h"
#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"
#include <algorithm>

namespace ark::compiler {
class BasicBlock;
class Graph;

/**
 * This class builds blocks list for reverse postorder traversal.
 * There is an option to invalidate blocks list. In this case the list
 * will be build from scratch after the next request for it.
 * Also it provides methods for updating an existing tree.
 */
class Rpo : public Analysis {
public:
    explicit Rpo(Graph *graph);

    NO_MOVE_SEMANTIC(Rpo);
    NO_COPY_SEMANTIC(Rpo);
    ~Rpo() override = default;

    const char *GetPassName() const override
    {
        return "RPO";
    }

public:
    void RemoveBasicBlock(BasicBlock *rmBlock)
    {
        ASSERT_PRINT(IsValid(), "RPO is invalid");
        auto it = std::find(rpoVector_.begin(), rpoVector_.end(), rmBlock);
        if (it != rpoVector_.end()) {
            rpoVector_.erase(it);
        }
    }

    void AddBasicBlockAfter(BasicBlock *curBlock, BasicBlock *newBlock)
    {
        ASSERT_PRINT(IsValid(), "RPO is invalid");
        auto it = std::find(rpoVector_.begin(), rpoVector_.end(), curBlock);
        rpoVector_.insert(it + 1, newBlock);
    }

    void AddBasicBlockBefore(BasicBlock *curBlock, BasicBlock *newBlock)
    {
        ASSERT_PRINT(IsValid(), "RPO is invalid");
        auto it = std::find(rpoVector_.begin(), rpoVector_.end(), curBlock);
        rpoVector_.insert(it, newBlock);
    }

    void AddVectorAfter(BasicBlock *curBlock, const ArenaVector<BasicBlock *> &newVector)
    {
        ASSERT_PRINT(IsValid(), "RPO is invalid");
        auto it = std::find(rpoVector_.begin(), rpoVector_.end(), curBlock);
        rpoVector_.insert(it + 1, newVector.begin(), newVector.end());
    }

    const ArenaVector<BasicBlock *> &GetBlocks() const
    {
        return rpoVector_;
    }

private:
    bool RunImpl() override;

    void DFS(BasicBlock *block, size_t *blocksCount);

private:
    Marker marker_ {UNDEF_MARKER};
    ArenaVector<BasicBlock *> rpoVector_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_RPO_H
