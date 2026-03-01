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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CODE_SINK_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CODE_SINK_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace ark::compiler {
class CodeSink : public Optimization {
    using Optimization::Optimization;

public:
    explicit CodeSink(Graph *graph) : Optimization(graph) {};
    NO_COPY_SEMANTIC(CodeSink);
    NO_MOVE_SEMANTIC(CodeSink);
    ~CodeSink() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerCodeSink();
    }

    const char *GetPassName() const override
    {
        return "CodeSink";
    }

    void InvalidateAnalyses() override;

private:
    bool ProcessBlock(BasicBlock *block);
    BasicBlock *SinkInstruction(Inst *inst, InstVector *stores, bool barriered);
    bool IsAcceptableTarget(Inst *inst, BasicBlock *candidate);

private:
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  //  COMPILER_OPTIMIZER_OPTIMIZATIONS_CODE_SINK_H
