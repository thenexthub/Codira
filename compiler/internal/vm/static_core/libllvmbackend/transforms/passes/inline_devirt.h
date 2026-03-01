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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINEDEVIRT_PASS_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINEDEVIRT_PASS_H

#include "llvm_ark_interface.h"
#include "transforms/passes/devirt.h"
#include "transforms/passes/check_external.h"

#include <llvm/Transforms/IPO/Inliner.h>

namespace ark::llvmbackend::passes {

class InlineDevirt : public llvm::PassInfoMixin<InlineDevirt> {
public:
    explicit InlineDevirt(LLVMArkInterface *arkInterface = nullptr, bool doVirtualInline = true);

    static bool ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options);

    static InlineDevirt Create(LLVMArkInterface *arkInterface, const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::LazyCallGraph::SCC &initialSCC, llvm::CGSCCAnalysisManager &analysisManager,
                                llvm::LazyCallGraph &callGraph, llvm::CGSCCUpdateResult &updateResult);

private:
    LLVMArkInterface *arkInterface_;
    bool doVirtualInline_;
    llvm::FunctionAnalysisManager *functionAnalysisManager_ {nullptr};
    llvm::LazyCallGraph::SCC *currentSCC_ {nullptr};
    llvm::PassInstrumentation *passInstrumentation_ {nullptr};
    llvm::CGSCCAnalysisManager *analysisManager_ {nullptr};
    llvm::CGSCCUpdateResult *updateResult_ {nullptr};
    llvm::LazyCallGraph *callGraph_ {nullptr};
    llvm::PreservedAnalyses preservedAnalyses_;

public:
    static constexpr llvm::StringRef ARG_NAME = "inline-devirt";

private:
    bool RunInlining(llvm::InlinerPass &inlinePass, llvm::SmallPtrSetImpl<llvm::Function *> &changedFunctions);
    bool RunDevirt(ark::llvmbackend::passes::Devirt &devirtPass);
    void RunCheckExternal(ark::llvmbackend::passes::CheckExternal &externalPass);
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINEDEVIRT_PASS_H
