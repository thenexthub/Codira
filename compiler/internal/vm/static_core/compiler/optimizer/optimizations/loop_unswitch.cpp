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
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/loop_unswitcher.h"
#include "loop_unswitch.h"

namespace ark::compiler {
bool LoopUnswitch::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return isApplied_;
}

void LoopUnswitch::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool LoopUnswitch::TransformLoop(Loop *loop)
{
    if (!loop->GetInnerLoops().empty()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop wasn't unswitched since it contains loops. Loop id = " << loop->GetId();
        return false;
    }
    loops_.push(loop);

    int64_t budget = maxInsns_;
    for (uint32_t level = 0; !loops_.empty() && level < maxLevel_ && budget > 0; ++level) {
        auto levelSize = loops_.size();
        while (levelSize-- != 0) {
            auto origLoop = loops_.front();
            loops_.pop();

            if (LoopUnswitcher::IsSmallLoop(origLoop)) {
                COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                    << "Level #" << level << ": estimated loop iterations < 2, skip loop " << loop->GetId();
                continue;
            }
            auto unswitchInst = LoopUnswitcher::FindUnswitchInst(origLoop);
            if (unswitchInst == nullptr) {
                COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                    << "Level #" << level << ": cannot find unswitch instruction, skip loop " << loop->GetId();
                continue;
            }

            int64_t loopSize = 0;
            int64_t trueCount = 0;
            int64_t falseCount = 0;
            LoopUnswitcher::EstimateInstructionsCount(loop, unswitchInst, &loopSize, &trueCount, &falseCount);
            if (trueCount + falseCount >= budget + loopSize) {
                break;
            }

            auto loopUnswitcher =
                LoopUnswitcher(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
            auto newLoop = loopUnswitcher.UnswitchLoop(origLoop, unswitchInst);
            if (newLoop == nullptr) {
                continue;
            }

            if (trueCount + falseCount > loopSize) {
                budget -= trueCount + falseCount - loopSize;
            }

            COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                << "Level #" << level << ": unswitch loop " << origLoop->GetId() << ", new loop " << newLoop->GetId();

            loops_.push(origLoop);
            loops_.push(newLoop);

            isApplied_ = true;
        }
    }
    return true;
}
}  // namespace ark::compiler
