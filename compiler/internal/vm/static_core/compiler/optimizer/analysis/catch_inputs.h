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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_CATCH_INPUT_H
#define COMPILER_OPTIMIZER_ANALYSIS_CATCH_INPUT_H

#include "optimizer/pass.h"

namespace ark::compiler {
/**
 * Mark all instructions that are used within catch blocks (i.e. catch block inputs)
 * with CATCH_INPUT flag.
 * Analysis is aimed to help optimization passes to decide whether or not an instruction
 * could be safely removed from a method having catch block (as catch block will be
 * removed from the CFG after try-catch resolution).
 * For example, scalar replacement optimization may eliminate allocation of
 * objects that don't escape a method. In case of an exception a catch block is yet
 * another escapement site and EscapeAnalysis pass is relying on CATCH_INPUT flag to
 * check if an allocation can escape through a catch.
 */
class CatchInputs : public Analysis {
public:
    using Analysis::Analysis;
    NO_MOVE_SEMANTIC(CatchInputs);
    NO_COPY_SEMANTIC(CatchInputs);
    ~CatchInputs() override = default;

    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "Catch Inputs";
    }
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_CATCH_INPUT_H
