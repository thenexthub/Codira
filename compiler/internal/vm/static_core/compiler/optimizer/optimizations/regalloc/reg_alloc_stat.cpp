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

#include "reg_alloc_stat.h"
#include "optimizer/ir/datatype.h"

namespace ark::compiler {
RegAllocStat::RegAllocStat(const ArenaVector<LifeIntervals *> &intervals)
{
    std::vector<bool> usedRegs(GetInvalidReg());
    std::vector<bool> usedVregs(GetInvalidReg());
    std::vector<bool> usedSlots(GetInvalidReg());
    std::vector<bool> usedVslots(GetInvalidReg());

    for (const auto &interv : intervals) {
        if (interv->IsPhysical()) {
            continue;
        }
        auto location = interv->GetLocation();
        if (location.IsRegister()) {
            usedRegs[location.GetValue()] = true;
        } else if (location.IsFpRegister()) {
            usedVregs[location.GetValue()] = true;
        } else if (location.IsStack()) {
            auto slot = location.GetValue();
            DataType::IsFloatType(interv->GetType()) ? (usedSlots[slot] = true) : (usedVslots[slot] = true);
        }
    }

    regs_ = static_cast<size_t>(std::count(usedRegs.begin(), usedRegs.end(), true));
    vregs_ = static_cast<size_t>(std::count(usedVregs.begin(), usedVregs.end(), true));
    slots_ = static_cast<size_t>(std::count(usedSlots.begin(), usedSlots.end(), true));
    vslots_ = static_cast<size_t>(std::count(usedVslots.begin(), usedVslots.end(), true));
}
}  // namespace ark::compiler
