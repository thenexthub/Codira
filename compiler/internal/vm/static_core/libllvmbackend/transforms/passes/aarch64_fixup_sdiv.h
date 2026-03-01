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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_AARCH64_FIXUP_SDIV_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_AARCH64_FIXUP_SDIV_H

#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

class AArch64FixupSDiv : public llvm::PassInfoMixin<AArch64FixupSDiv> {
public:
    static constexpr llvm::StringRef ARG_NAME = "aarch64-fixup-sdiv";

    explicit AArch64FixupSDiv(LLVMArkInterface *arkInterface = nullptr);

    static AArch64FixupSDiv Create(LLVMArkInterface *arkInterface,
                                   const ark::llvmbackend::LLVMCompilerOptions *options);

    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    bool ReplaceSelect(llvm::Instruction *selectInst);

    LLVMArkInterface *arkInterface_;
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_AARCH64_FIXUP_SDIV_H
