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

#ifndef LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H
#define LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H

#include "compiler/code_info/code_info_builder.h"
#include "compiler/optimizer/ir/runtime_interface.h"

#include "llvm_ark_interface.h"
#include "llvm_compiler.h"
#include "llvm_compiler_options.h"
#include "lowering/debug_data_builder.h"
#include "object_code/created_object_file.h"

#include <llvm/Support/Error.h>

namespace ark::compiler {
class CompiledMethod;
class Graph;
}  // namespace ark::compiler

namespace ark::llvmbackend {

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LLVMIrtocCompiler final : public LLVMCompiler, public IrtocCompilerInterface {
public:
    explicit LLVMIrtocCompiler(ark::compiler::RuntimeInterface *runtime, ark::ArenaAllocator *allocator, ark::Arch arch,
                               std::string filename);

    Expected<bool, std::string> TryAddGraph(ark::compiler::Graph *graph) override;

    void FinishCompile() override;

    bool IsEmpty() override
    {
        return methods_.empty();
    }

    bool HasCompiledCode() override
    {
        return objectFile_ != nullptr;
    }

    bool IsIrFailed() override
    {
        return false;
    }
    void WriteObjectFile(std::string_view output) override;

    CompiledCode GetCompiledCode(std::string_view functionName) override;

private:
    std::string GetFastPathFeatures() const;

    static std::vector<std::string> GetFeaturesForArch(Arch arch);

    void InitializeSpecificLLVMOptions(Arch arch);

    void InitializeModule();

    size_t GetObjectFileSize() override;

private:
    llvm::ExitOnError exitOnErr_;

    ArenaVector<ark::Method *> methods_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<DebugDataBuilder> debugData_;
    std::unique_ptr<ark::llvmbackend::CreatedObjectFile> objectFile_ {nullptr};
    std::string filename_;

    LLVMArkInterface arkInterface_;
    std::unique_ptr<ark::llvmbackend::MIRCompiler> mirCompiler_;
    std::unique_ptr<ark::llvmbackend::LLVMOptimizer> optimizer_;
    std::unique_ptr<llvm::TargetMachine> targetMachine_ {nullptr};
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H
