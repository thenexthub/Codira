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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNSWITCH_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNSWITCH_H

#include "optimizer/optimizations/loop_transform.h"
#include "compiler_options.h"

namespace ark::compiler {
class LoopUnswitch : public LoopTransform<LoopExitPoint::LOOP_EXIT_BACKEDGE> {
public:
    explicit LoopUnswitch(Graph *graph, uint32_t maxLevel, uint32_t maxInsns)
        : LoopTransform(graph), loops_(graph->GetLocalAllocator()->Adapter()), maxLevel_(maxLevel), maxInsns_(maxInsns)
    {
    }
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "LoopUnswitch";
    }

    void InvalidateAnalyses() override;
    bool IsEnable() const override
    {
        return g_options.IsCompilerLoopUnswitch();
    }

private:
    bool TransformLoop(Loop *loop) override;
    bool IsHoistable(Inst *inst, Loop *loop);
    bool AllInputsConst(Inst *inst);
    ArenaQueue<Loop *> loops_;
    const uint32_t maxLevel_ {0};
    const uint32_t maxInsns_ {0};
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNSWITCH_H
