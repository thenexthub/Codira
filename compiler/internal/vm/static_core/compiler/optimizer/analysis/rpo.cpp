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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "rpo.h"

namespace ark::compiler {
Rpo::Rpo(Graph *graph) : Analysis(graph), rpoVector_(graph->GetAllocator()->Adapter()) {}

/**
 *  Depth-first search
 * `blocks_count` needs for filling `rpo_vector_` in reverse order
 */
void Rpo::DFS(BasicBlock *block, size_t *blocksCount)
{
    ASSERT(block != nullptr);
    block->SetMarker(marker_);

    for (auto succBlock : block->GetSuccsBlocks()) {
        if (!succBlock->IsMarked(marker_)) {
            DFS(succBlock, blocksCount);
        }
    }

    ASSERT(blocksCount != nullptr && *blocksCount > 0);
    rpoVector_[--(*blocksCount)] = block;
}

bool Rpo::RunImpl()
{
    size_t blocksCount = GetGraph()->GetAliveBlocksCount();
    rpoVector_.resize(blocksCount);
    if (blocksCount == 0) {
        return true;
    }
    marker_ = GetGraph()->NewMarker();
    ASSERT_PRINT(marker_ != UNDEF_MARKER, "There are no free markers. Please erase unused markers");
    DFS(GetGraph()->GetStartBlock(), &blocksCount);
#ifndef NDEBUG
    if (blocksCount != 0) {
        std::cerr << "There are unreachable blocks:\n";
        for (auto bb : *GetGraph()) {
            if (bb != nullptr && !bb->IsMarked(marker_)) {
                bb->Dump(&std::cerr);
            }
        }
        UNREACHABLE();
    }
#endif  // !NDEBUG
    GetGraph()->EraseMarker(marker_);
    return true;
}
}  // namespace ark::compiler
