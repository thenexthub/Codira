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

#ifndef LIBLLVMBACKEND_LOWERING_WRAPPED_MODULE_H
#define LIBLLVMBACKEND_LOWERING_WRAPPED_MODULE_H

#include "compiler/optimizer/ir/runtime_interface.h"
#include "debug_data_builder.h"
#include "llvm_ark_interface.h"
#include "object_code/code_info_producer.h"

#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

#include <vector>

namespace ark::llvmbackend {
/// Wrapper around everything we need to compile an llvm::Module.
class WrappedModule {
public:
    explicit WrappedModule(std::unique_ptr<llvm::LLVMContext> llvmContext, std::unique_ptr<llvm::Module> module,
                           std::unique_ptr<llvm::TargetMachine> targetMachine,
                           std::unique_ptr<ark::llvmbackend::LLVMArkInterface> arkInterface,
                           std::unique_ptr<ark::llvmbackend::DebugDataBuilder> debugData);

    void SetId(uint32_t id)
    {
        moduleId_ = id;
    }

    NO_COPY_SEMANTIC(WrappedModule);

    DEFAULT_MOVE_SEMANTIC(WrappedModule);

    ~WrappedModule() = default;

    void AddMethod(ark::compiler::RuntimeInterface::MethodPtr method);

    void SetCompiled(std::unique_ptr<ark::llvmbackend::CreatedObjectFile> objectFile);

    bool IsCompiled();

    bool HasFunctionDefinition(ark::compiler::RuntimeInterface::MethodPtr method);

    llvm::Function *GetFunctionByMethodPtr(ark::compiler::RuntimeInterface::MethodPtr method);

    const std::unique_ptr<llvm::LLVMContext> &GetLLVMContext();

    const std::unique_ptr<llvm::Module> &GetModule();

    const std::unique_ptr<llvm::TargetMachine> &GetTargetMachine();

    const std::unique_ptr<ark::llvmbackend::LLVMArkInterface> &GetLLVMArkInterface();

    const std::unique_ptr<ark::llvmbackend::DebugDataBuilder> &GetDebugData();

    const std::unique_ptr<ark::llvmbackend::CodeInfoProducer> &GetCodeInfoProducer();

    const std::vector<ark::compiler::RuntimeInterface::MethodPtr> &GetMethods();

    const std::unique_ptr<ark::llvmbackend::CreatedObjectFile> &GetObjectFile();

    uint32_t GetModuleId();

    std::unique_ptr<ark::llvmbackend::CreatedObjectFile> TakeObjectFile();

    void Dump(llvm::raw_ostream *stream);

private:
    // Do not move llvmContext_ below module, the context must be destroyed after module
    std::unique_ptr<llvm::LLVMContext> llvmContext_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::TargetMachine> targetMachine_;
    std::unique_ptr<ark::llvmbackend::CreatedObjectFile> objectFile_;
    std::unique_ptr<ark::llvmbackend::LLVMArkInterface> arkInterface_;
    std::unique_ptr<ark::llvmbackend::DebugDataBuilder> debugData_;
    std::unique_ptr<ark::llvmbackend::CodeInfoProducer> codeInfoProducer_;
    std::vector<ark::compiler::RuntimeInterface::MethodPtr> methods_;
    uint32_t moduleId_ {0};
};

}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LOWERING_WRAPPED_MODULE_H
