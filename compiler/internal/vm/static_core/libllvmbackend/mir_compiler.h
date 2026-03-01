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

#ifndef LIBLLVMBACKEND_MIR_COMPILER_H
#define LIBLLVMBACKEND_MIR_COMPILER_H

#include <unordered_map>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Target/TargetMachine.h>

#include "object_code/created_object_file.h"

namespace ark::llvmbackend {

class InsertingPassManager : public llvm::legacy::PassManager {
public:
    void add(llvm::Pass *p) override;

    void InsertBefore(llvm::AnalysisID before, llvm::Pass *pass)
    {
        befores_[before] = pass;
    }

private:
    std::unordered_map<llvm::AnalysisID, llvm::Pass *> befores_;
};

class MIRCompiler {
public:
    using PassInserterFunction = std::function<void(InsertingPassManager *manager)>;

    // Construct a compile functor with the given target builder.
    explicit MIRCompiler(const std::unique_ptr<llvm::TargetMachine> &targetMachine, PassInserterFunction insertPasses)
        : targetMachine_(std::move(targetMachine)), insertPasses_(std::move(insertPasses))
    {
    }

    // Compile a Module to an ObjectFile.
    llvm::Expected<std::unique_ptr<CreatedObjectFile>> CompileModule(llvm::Module &module);

    const std::unique_ptr<llvm::TargetMachine> &GetTargetMachine()
    {
        return targetMachine_;
    }

private:
    const std::unique_ptr<llvm::TargetMachine> &targetMachine_;
    PassInserterFunction insertPasses_;
};

}  // namespace ark::llvmbackend

#endif  //  LIBLLVMBACKEND_MIR_COMPILER_H
