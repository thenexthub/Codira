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

#ifndef LIBLLVMBACKEND_LLVM_COMPILER_H
#define LIBLLVMBACKEND_LLVM_COMPILER_H

#include "compiler_interface.h"
#include "mir_compiler.h"
#include "llvm_compiler_options.h"
#include "transforms/llvm_optimizer.h"

#include <llvm/ADT/Triple.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>

namespace ark::llvmbackend {

class LLVMCompiler : public CompilerInterface {
public:
    explicit LLVMCompiler(Arch arch);

    Arch GetArch() const
    {
        return arch_;
    }

protected:
    bool IsInliningDisabled();
    bool IsInliningDisabled(ark::compiler::Graph *graph);

    bool IsInliningDisabled(compiler::RuntimeInterface *runtime, compiler::RuntimeInterface::MethodPtr method);

    ark::llvmbackend::LLVMCompilerOptions InitializeLLVMCompilerOptions();
    void InitializeDefaultLLVMOptions();
    void InitializeLLVMOptions();

    void SetLLVMOption(const char *option, const std::string &value);

    static llvm::Triple GetTripleForArch(Arch arch);
    static std::string GetCPUForArch(Arch arch);

    static void InitializeLLVMTarget(Arch arch);
    static void InitializeLLVMPasses();

    llvm::LLVMContext *GetLLVMContext()
    {
        return &context_;
    }

private:
    Arch arch_;
    llvm::LLVMContext context_;
    std::string llvmPreOptions_;
#ifndef NDEBUG
    bool parsedOptions_ {false};
#endif
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_COMPILER_H
