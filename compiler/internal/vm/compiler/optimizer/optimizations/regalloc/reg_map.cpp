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

#include "reg_map.h"

namespace panda::compiler {

void RegisterMap::SetMask(const LocationMask &reg_mask, size_t priority_reg)
{
    codegen_reg_map_.clear();

    // Firstly map registers available for register allocator starting with the highest priority one
    for (size_t reg = priority_reg; reg < reg_mask.GetSize(); ++reg) {
        if (!reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
    border_ = static_cast<Register>(codegen_reg_map_.size());

    // Add caller registers
    for (size_t reg = 0; reg < priority_reg; ++reg) {
        if (!reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
    available_regs_count_ = codegen_reg_map_.size();

    // Now map unavailable registers, since they can be assigned to the instructions
    for (size_t reg = 0; reg < reg_mask.GetSize(); ++reg) {
        if (reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
}

void RegisterMap::SetCallerFirstMask(const LocationMask &reg_mask, size_t first_callee_reg, size_t last_callee_reg)
{
    codegen_reg_map_.clear();

    // Add caller registers
    for (size_t reg = 0; reg < first_callee_reg; ++reg) {
        if (!reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }

    // Add caller registers after callees onece
    for (size_t reg = last_callee_reg + 1; reg < reg_mask.GetSize(); ++reg) {
        if (!reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
    border_ = static_cast<Register>(codegen_reg_map_.size());

    // Add callee registers
    for (size_t reg = first_callee_reg; reg <= last_callee_reg; ++reg) {
        if (!reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
    available_regs_count_ = codegen_reg_map_.size();

    // Now map unavailable registers, since they can be assigned to the instructions
    for (size_t reg = 0; reg < reg_mask.GetSize(); ++reg) {
        if (reg_mask.IsSet(reg)) {
            codegen_reg_map_.push_back(reg);
        }
    }
}

size_t RegisterMap::Size() const
{
    return codegen_reg_map_.size();
}

size_t RegisterMap::GetAvailableRegsCount() const
{
    return available_regs_count_;
}

bool RegisterMap::IsRegAvailable(Register reg, Arch arch) const
{
    return arch != Arch::AARCH32 || reg < available_regs_count_;
}

Register RegisterMap::CodegenToRegallocReg(Register codegen_reg) const
{
    auto it = std::find(codegen_reg_map_.cbegin(), codegen_reg_map_.cend(), codegen_reg);
    ASSERT(it != codegen_reg_map_.end());
    return std::distance(codegen_reg_map_.cbegin(), it);
}

Register RegisterMap::RegallocToCodegenReg(Register regalloc_reg) const
{
    ASSERT(regalloc_reg < codegen_reg_map_.size());
    return codegen_reg_map_[regalloc_reg];
}

void RegisterMap::Dump(std::ostream *out) const
{
    *out << "Regalloc -> Codegen" << std::endl;
    for (size_t i = 0; i < codegen_reg_map_.size(); i++) {
        if (i == available_regs_count_) {
            *out << "Unavailable for RA:" << std::endl;
        }
        *out << "r" << std::to_string(i) << " -> r" << std::to_string(codegen_reg_map_[i]) << std::endl;
    }
}

}  // namespace panda::compiler
