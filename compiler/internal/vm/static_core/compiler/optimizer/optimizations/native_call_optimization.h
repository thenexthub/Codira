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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_NATIVE_CALL_OPTIMIZATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_NATIVE_CALL_OPTIMIZATION_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"

namespace ark::compiler {

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API NativeCallOptimization : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit NativeCallOptimization(Graph *graph) : Optimization(graph) {}

    NO_MOVE_SEMANTIC(NativeCallOptimization);
    NO_COPY_SEMANTIC(NativeCallOptimization);
    ~NativeCallOptimization() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "NativeCallOptimization";
    }

    void SetIsApplied()
    {
        isApplied_ = true;
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    void InvalidateAnalyses() override;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitCallStatic(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    static void OptimizePrimitiveNativeCall(GraphVisitor *v, CallInst *callInst);
    static void OptimizeNativeCallWithObjects(GraphVisitor *v, CallInst *callInst);

    IntrinsicInst *CreateNativeApiIntrinsic(DataType::Type type, uint32_t pc, RuntimeInterface::IntrinsicId id,
                                            const MethodDataMixin *methodData);

    bool isApplied_ {false};
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_NATIVE_CALL_OPTIMIZATION_H
