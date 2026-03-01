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

#include "wrapped_module.h"
#include "transforms/gc_utils.h"

namespace ark::llvmbackend {

WrappedModule::WrappedModule(std::unique_ptr<llvm::LLVMContext> llvmContext, std::unique_ptr<llvm::Module> module,
                             std::unique_ptr<llvm::TargetMachine> targetMachine,
                             std::unique_ptr<ark::llvmbackend::LLVMArkInterface> arkInterface,
                             std::unique_ptr<ark::llvmbackend::DebugDataBuilder> debugData)
    : llvmContext_(std::move(llvmContext)),
      module_(std::move(module)),
      targetMachine_(std::move(targetMachine)),
      arkInterface_(std::move(arkInterface)),
      debugData_(std::move(debugData)),
      codeInfoProducer_(std::make_unique<CodeInfoProducer>(arkInterface_->IsArm64() ? Arch::AARCH64 : Arch::X86_64,
                                                           arkInterface_.get()))
{
    ASSERT(llvmContext_ != nullptr);
    ASSERT(module_ != nullptr);
    ASSERT(targetMachine_ != nullptr);
    ASSERT(arkInterface_ != nullptr);
    ASSERT(debugData_ != nullptr);
    ASSERT(codeInfoProducer_ != nullptr);
}

void WrappedModule::AddMethod(ark::compiler::RuntimeInterface::MethodPtr method)
{
    methods_.push_back(method);
}

void WrappedModule::SetCompiled(std::unique_ptr<ark::llvmbackend::CreatedObjectFile> objectFile)
{
    objectFile_ = std::move(objectFile);
#ifndef NDEBUG
    arkInterface_->MarkAsCompiled();
#endif
    module_ = nullptr;
    debugData_ = nullptr;
    llvmContext_ = nullptr;
}

bool WrappedModule::IsCompiled()
{
    return objectFile_ != nullptr;
}

bool WrappedModule::HasFunctionDefinition(ark::compiler::RuntimeInterface::MethodPtr method)
{
    llvm::Function *function = GetFunctionByMethodPtr(method);
    return function != nullptr && !function->isDeclarationForLinker();
}

llvm::Function *WrappedModule::GetFunctionByMethodPtr(ark::compiler::RuntimeInterface::MethodPtr method)
{
    return module_->getFunction(arkInterface_->GetUniqMethodName(method));
}

const std::unique_ptr<llvm::LLVMContext> &WrappedModule::GetLLVMContext()
{
    return llvmContext_;
}

const std::unique_ptr<llvm::Module> &WrappedModule::GetModule()
{
    return module_;
}

const std::unique_ptr<llvm::TargetMachine> &WrappedModule::GetTargetMachine()
{
    return targetMachine_;
}

const std::unique_ptr<ark::llvmbackend::LLVMArkInterface> &WrappedModule::GetLLVMArkInterface()
{
    return arkInterface_;
}

const std::unique_ptr<ark::llvmbackend::DebugDataBuilder> &WrappedModule::GetDebugData()
{
    return debugData_;
}

const std::unique_ptr<ark::llvmbackend::CodeInfoProducer> &WrappedModule::GetCodeInfoProducer()
{
    return codeInfoProducer_;
}

const std::vector<ark::compiler::RuntimeInterface::MethodPtr> &WrappedModule::GetMethods()
{
    return methods_;
}

const std::unique_ptr<ark::llvmbackend::CreatedObjectFile> &WrappedModule::GetObjectFile()
{
    return objectFile_;
}

uint32_t WrappedModule::GetModuleId()
{
    return moduleId_;
}

std::unique_ptr<ark::llvmbackend::CreatedObjectFile> WrappedModule::TakeObjectFile()
{
    return std::move(objectFile_);
}

void WrappedModule::Dump(llvm::raw_ostream *stream)
{
    *stream << "Wrapped module:\n";

    size_t inlineFunctions {0};
    size_t functions {0};

    for (const auto &function : *module_) {
        if (function.getMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE) == nullptr &&
            !gc_utils::IsFunctionSupplemental(function)) {
            *stream << "\tPrimary function '" << function.getName() << "'\n";
            functions++;
        }
    }
    for (const auto &function : *module_) {
        if (function.getMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE) != nullptr &&
            !gc_utils::IsFunctionSupplemental(function)) {
            *stream << "\tInline function '" << function.getName() << "'\n";
            inlineFunctions++;
        }
    }

    *stream << "\tTotal: functions = " << functions << ", inlineFunctions = " << inlineFunctions << "\n";
}

}  // namespace ark::llvmbackend
