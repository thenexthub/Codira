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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "rpo.h"

namespace panda::compiler {
Rpo::Rpo(Graph *graph) : Analysis(graph), rpo_vector_(graph->GetAllocator()->Adapter()) {}

/**
 *  Depth-first search
 * `blocks_count` needs for filling `rpo_vector_` in reverse order
 */
void Rpo::DFS(BasicBlock *block, size_t *blocks_count)
{
    ASSERT(block != nullptr);
    block->SetMarker(marker_);

    for (auto succ_block : block->GetSuccsBlocks()) {
        if (!succ_block->IsMarked(marker_)) {
            DFS(succ_block, blocks_count);
        }
    }

    ASSERT(blocks_count != nullptr && *blocks_count > 0);
    rpo_vector_[--(*blocks_count)] = block;
}

bool Rpo::RunImpl()
{
    size_t blocks_count = GetGraph()->GetAliveBlocksCount();
    rpo_vector_.resize(blocks_count);
    if (blocks_count == 0) {
        return true;
    }
    marker_ = GetGraph()->NewMarker();
    ASSERT_PRINT(marker_ != UNDEF_MARKER, "There are no free markers. Please erase unused markers");
    DFS(GetGraph()->GetStartBlock(), &blocks_count);
#ifndef NDEBUG
    if (blocks_count != 0) {
        std::cerr << "There are unreachable blocks:\n";
        for (auto bb : *GetGraph()) {
            if (bb != nullptr && !bb->IsMarked(marker_)) {
                bb->Dump(&std::cerr);
            }
        }
        UNREACHABLE();
    }
#endif  // NDEBUG
    GetGraph()->EraseMarker(marker_);
    return true;
}
}  // namespace panda::compiler
