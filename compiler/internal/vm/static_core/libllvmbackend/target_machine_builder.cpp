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

#include "target_machine_builder.h"
#include "transforms/transform_utils.h"

#include <llvm/MC/TargetRegistry.h>

namespace ark::llvmbackend {

llvm::Expected<std::unique_ptr<llvm::TargetMachine>> TargetMachineBuilder::Build()
{
    ASSERT(triple_.getArch() != llvm::Triple::UnknownArch);
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget("", triple_, error);
    if (!error.empty()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       llvm::StringRef {"Could not lookupTarget by triple = '"} + triple_.str() +
                                           "', error = '" + error + "'");
    }
    ASSERT(target != nullptr);

    llvm::TargetOptions targetOptions {};
    auto machine = target->createTargetMachine(triple_.getTriple(), cpu_, features_, targetOptions, llvm::Reloc::PIC_,
                                               llvm::CodeModel::Small, optlevel_);
    ASSERT(machine != nullptr);
    return std::unique_ptr<llvm::TargetMachine>(machine);
}

TargetMachineBuilder &TargetMachineBuilder::SetFeatures(const std::vector<std::string> &features)
{
    features_ = llvm::join(features.cbegin(), features.cend(), ",");
    return *this;
}
}  // namespace ark::llvmbackend
