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
#ifndef COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H
#define COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H

#include "optimizer/ir/inst.h"

namespace ark::compiler {
class Loop;

/**
 * Example of code
 *  ---------------
 * for (init(a); if_imm(compare(a,test)); update(a)) {...}
 */
enum WalkType {
    INC,
    DEC,
    SHL,
    SHR,
    ASHR,
};

struct CountableLoopInfo {
    Inst *ifImm;
    Inst *init;
    Inst *test;
    Inst *update;
    Inst *index;
    uint64_t constStep;
    ConditionCode normalizedCc;  // cc between `update` and `test`
    bool isInc;
    WalkType walkType;
};

/// Helper class to check if loop is countable and to get its parameters
class CountableLoopParser {
public:
    explicit CountableLoopParser(const Loop &loop) : loop_(loop) {}

    NO_MOVE_SEMANTIC(CountableLoopParser);
    NO_COPY_SEMANTIC(CountableLoopParser);
    ~CountableLoopParser() = default;

    std::optional<CountableLoopInfo> Parse();
    bool ParseLoopExit();
    static bool HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loopInfo);
    static std::optional<uint64_t> GetLoopIterations(const CountableLoopInfo &loopInfo);
    static std::optional<uint64_t> GetLoopIterationsForIncAndDec(const CountableLoopInfo &loopInfo);
    static std::optional<uint64_t> GetLoopIterationsForShl(const CountableLoopInfo &loopInfo);
    static std::optional<uint64_t> GetLoopIterationsForShrAndAshr(const CountableLoopInfo &loopInfo);

private:
    bool IsInstIncOrDec(Inst *inst);
    bool IsInstShlShrAshr(Inst *inst);
    bool SetUpdateAndTestInputs();
    void SetIndexAndConstStep();
    void SetIndexAndConstStepForIncAndDec();
    void SetIndexAndConstStepForShlAndShrAndAshr();
    void SetNormalizedConditionCode();
    bool IsConditionCodeAcceptable();
    BasicBlock *FindLoopExitBlock();
    bool CheckParsingLoopCorrectness();
    bool TryProcessBackEdge();

protected:
    const Loop &loop_;               // NOLINT(misc-non-private-member-variables-in-classes)
    CountableLoopInfo loopInfo_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
    bool isHeadLoopExit_ = false;    // NOLINT(misc-non-private-member-variables-in-classes)
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_COUNTABLE_LOOP_PARSER_H
