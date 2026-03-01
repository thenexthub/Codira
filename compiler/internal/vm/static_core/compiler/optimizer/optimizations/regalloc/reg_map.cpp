/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "reg_map.h"
#include <algorithm>

namespace ark::compiler {

void RegisterMap::SetMask(const LocationMask &regMask, size_t priorityReg)
{
    codegenRegMap_.clear();

    // Firstly map registers available for register allocator starting with the highest priority one
    auto maskSize = regMask.GetSize();
    for (size_t reg = priorityReg; reg < maskSize; ++reg) {
        if (!regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }
    border_ = static_cast<Register>(codegenRegMap_.size());

    // Add caller registers
    for (size_t reg = 0; reg < priorityReg; ++reg) {
        if (!regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }
    availableRegsCount_ = codegenRegMap_.size();

    // Now map unavailable registers, since they can be assigned to the instructions
    MapUnavailableRegisters(regMask, maskSize);
}

void RegisterMap::SetCallerFirstMask(const LocationMask &regMask, size_t firstCalleeReg, size_t lastCalleeReg)
{
    codegenRegMap_.clear();

    // Add caller registers
    for (size_t reg = 0; reg < firstCalleeReg; ++reg) {
        if (!regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }

    // Add caller registers after callees onece
    auto maskSize = regMask.GetSize();
    for (size_t reg = lastCalleeReg + 1; reg < maskSize; ++reg) {
        if (!regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }
    border_ = static_cast<Register>(codegenRegMap_.size());

    // Add callee registers
    for (size_t reg = firstCalleeReg; reg <= lastCalleeReg; ++reg) {
        if (!regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }
    availableRegsCount_ = codegenRegMap_.size();

    // Now map unavailable registers, since they can be assigned to the instructions
    MapUnavailableRegisters(regMask, maskSize);
}

void RegisterMap::MapUnavailableRegisters(const LocationMask &regMask, size_t maskSize)
{
    for (size_t reg = 0; reg < maskSize; ++reg) {
        if (regMask.IsSet(reg)) {
            codegenRegMap_.push_back(reg);
        }
    }
}

size_t RegisterMap::Size() const
{
    return codegenRegMap_.size();
}

size_t RegisterMap::GetAvailableRegsCount() const
{
    return availableRegsCount_;
}

bool RegisterMap::IsRegAvailable(Register reg, Arch arch) const
{
    return arch != Arch::AARCH32 || reg < availableRegsCount_;
}

Register RegisterMap::CodegenToRegallocReg(Register codegenReg) const
{
    auto it = std::find(codegenRegMap_.cbegin(), codegenRegMap_.cend(), codegenReg);
    ASSERT(it != codegenRegMap_.end());
    return std::distance(codegenRegMap_.cbegin(), it);
}

Register RegisterMap::RegallocToCodegenReg(Register regallocReg) const
{
    ASSERT(regallocReg < codegenRegMap_.size());
    return codegenRegMap_[regallocReg];
}

void RegisterMap::Dump(std::ostream *out) const
{
    *out << "Regalloc -> Codegen" << std::endl;
    for (size_t i = 0; i < codegenRegMap_.size(); i++) {
        if (i == availableRegsCount_) {
            *out << "Unavailable for RA:" << std::endl;
        }
        *out << "r" << std::to_string(i) << " -> r" << std::to_string(codegenRegMap_[i]) << std::endl;
    }
}

}  // namespace ark::compiler
