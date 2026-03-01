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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_MAP_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_MAP_H

#include "utils/arena_containers.h"
#include "location_mask.h"
#include "optimizer/ir/constants.h"
#include "utils/arch.h"

namespace panda::compiler {

/**
 * Since the set of available codegen's registers can be sparse, we create local regalloc registers vector with size
 * equal to the number of given registers and save map to restore actual codegen register number from the regalloc's
 * register number.
 *
 * For example
 * - there are 3 available registers: r16, r18, r20;
 * - r18 has priority to be assigned;
 *
 * - `codegen_reg_map_` will be equal to [18, 20, 16]
 * - RegAlloc locally assigns registers with numbers 0, 1 and 2 and then replace them by the codegen's registers 18,
 * 20 and 16 accordingly.
 */
class RegisterMap {
public:
    explicit RegisterMap(ArenaAllocator *allocator) : codegen_reg_map_(allocator->Adapter()) {}
    ~RegisterMap() = default;
    NO_MOVE_SEMANTIC(RegisterMap);
    NO_COPY_SEMANTIC(RegisterMap);

    void SetMask(const LocationMask &reg_mask, size_t priority_reg);
    void SetCallerFirstMask(const LocationMask &reg_mask, size_t first_callee_reg, size_t last_callee_reg);
    size_t Size() const;
    size_t GetAvailableRegsCount() const;
    bool IsRegAvailable(Register reg, Arch arch) const;
    Register CodegenToRegallocReg(Register codegen_reg) const;
    Register RegallocToCodegenReg(Register regalloc_reg) const;
    Register GetBorder() const
    {
        return border_;
    }
    void Dump(std::ostream *out) const;

private:
    ArenaVector<Register> codegen_reg_map_;
    size_t available_regs_count_ {0};
    Register border_ {0};
};

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_MAP_H
