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

#ifndef COMPILER_OPTIMIZER_IR_LOOP_UNSWITCHER_H
#define COMPILER_OPTIMIZER_IR_LOOP_UNSWITCHER_H

#include "graph_cloner.h"

namespace ark::compiler {
/**
 * Class unswitchs loop:
 * - Clone loop;
 * - Fix control & data flow;
 */
class LoopUnswitcher : public GraphCloner {
public:
    static Inst *FindUnswitchInst(Loop *loop);
    static bool IsSmallLoop(Loop *loop);
    static void EstimateInstructionsCount(const Loop *loop, const Inst *unswitchInst, int64_t *loopSize,
                                          int64_t *trueCount, int64_t *falseCount);
    explicit LoopUnswitcher(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *localAllocator);
    Loop *UnswitchLoop(Loop *loop, Inst *inst);

private:
    void BuildLoopUnswitchControlFlow(LoopClonerData *unswitchData);
    void BuildLoopUnswitchDataFlow(LoopClonerData *unswitchData, Inst *ifInst);
    void ReplaceWithConstantCondition(Inst *ifInst);
    ArenaVector<Inst *> conditions_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_LOOP_UNSWITCHER_H
