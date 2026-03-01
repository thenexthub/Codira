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

#ifndef LIBLLVMBACKEND_TARGET_MACHINE_BUILDER_H
#define LIBLLVMBACKEND_TARGET_MACHINE_BUILDER_H

#include <llvm/Target/TargetMachine.h>

namespace ark::llvmbackend {

class TargetMachineBuilder {
public:
    TargetMachineBuilder &SetFeatures(const std::vector<std::string> &features);

    TargetMachineBuilder &SetOptLevel(llvm::CodeGenOpt::Level level)
    {
        optlevel_ = level;
        return *this;
    }

    TargetMachineBuilder &SetCPU(std::string cpu)
    {
        cpu_ = std::move(cpu);
        return *this;
    }

    TargetMachineBuilder &SetTriple(llvm::Triple triple)
    {
        triple_ = std::move(triple);
        return *this;
    }

    llvm::Expected<std::unique_ptr<llvm::TargetMachine>> Build();

private:
    std::string features_;
    llvm::CodeGenOpt::Level optlevel_ {llvm::CodeGenOpt::Level::Default};
    std::string cpu_;
    llvm::Triple triple_;
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_TARGET_MACHINE_BUILDER_H
