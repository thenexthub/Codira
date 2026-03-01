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

#ifndef PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_IDIOMS_H
#define PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_IDIOMS_H

#include "optimizer/optimizations/loop_transform.h"
#include "compiler_options.h"

// Find loops representing some idiom (like memcpy or memset) and replace
// it with an intrinsics.
namespace ark::compiler {

struct CountableLoopInfo;

class LoopIdioms : public LoopTransform<LoopExitPoint::LOOP_EXIT_HEADER> {
public:
    explicit LoopIdioms(Graph *graph) : LoopTransform(graph) {}
    ~LoopIdioms() override = default;
    NO_COPY_SEMANTIC(LoopIdioms);
    NO_MOVE_SEMANTIC(LoopIdioms);

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerLoopIdioms();
    }

    const char *GetPassName() const override
    {
        return "LoopIdioms";
    }

    void InvalidateAnalyses() override;

private:
    // Jump into intrinsic only if there will be more iterations
    static constexpr size_t ITERATIONS_THRESHOLD = 6;

    bool TransformLoop(Loop *loop) override;
    bool TryTransformArrayInitIdiom(Loop *loop);
    Inst *CreateArrayInitIntrinsic(StoreInst *store, CountableLoopInfo *info);
    bool ReplaceArrayInitLoop(Loop *loop, CountableLoopInfo *loopInfo, StoreInst *store, bool alwaysJump);

    bool TryTransformArrayMoveIdiom(Loop *loop);

    bool isApplied_ {false};
};

}  // namespace ark::compiler

#endif  // PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_IDIOMS_H