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

#include "reg_alloc_stat.h"

namespace panda::compiler {
RegAllocStat::RegAllocStat(const ArenaVector<LifeIntervals *> &intervals)
{
    std::vector<bool> used_regs(INVALID_REG);
    std::vector<bool> used_vregs(INVALID_REG);
    std::vector<bool> used_slots(INVALID_REG);
    std::vector<bool> used_vslots(INVALID_REG);

    for (const auto &interv : intervals) {
        if (interv->IsPhysical() || interv->NoDest()) {
            continue;
        }
        auto location = interv->GetLocation();
        if (location.IsRegister()) {
            used_regs[location.GetValue()] = true;
        } else if (location.IsFpRegister()) {
            used_vregs[location.GetValue()] = true;
        } else if (location.IsStack()) {
            auto slot = location.GetValue();
            DataType::IsFloatType(interv->GetType()) ? (used_slots[slot] = true) : (used_vslots[slot] = true);
        }
    }

    regs_ = static_cast<size_t>(std::count(used_regs.begin(), used_regs.end(), true));
    vregs_ = static_cast<size_t>(std::count(used_vregs.begin(), used_vregs.end(), true));
    slots_ = static_cast<size_t>(std::count(used_slots.begin(), used_slots.end(), true));
    vslots_ = static_cast<size_t>(std::count(used_vslots.begin(), used_vslots.end(), true));
}
}  // namespace panda::compiler
