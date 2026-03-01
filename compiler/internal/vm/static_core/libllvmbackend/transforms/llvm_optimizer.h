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

#ifndef LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H
#define LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H

#include "llvm_compiler_options.h"

#include <memory>

namespace llvm {
class Module;
class TargetMachine;
}  // namespace llvm

namespace ark::llvmbackend {
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend {
class LLVMOptimizer {
public:
    void DumpModuleBefore(llvm::Module *module) const;
    void DumpModuleAfter(llvm::Module *module) const;
    void OptimizeModule(llvm::Module *module) const;
    void ProcessInlineModule(llvm::Module *inlineModule) const;

    explicit LLVMOptimizer(ark::llvmbackend::LLVMCompilerOptions options, LLVMArkInterface *arkInterface,
                           const std::unique_ptr<llvm::TargetMachine> &targetMachine);

private:
    ark::llvmbackend::LLVMCompilerOptions options_;
    ark::llvmbackend::LLVMArkInterface *arkInterface_;
    const std::unique_ptr<llvm::TargetMachine> &targetMachine_;
};

}  // namespace ark::llvmbackend

#endif  // LIBLLVMBACKEND_TRANSFORMS_LLVM_OPTIMIZER_H
