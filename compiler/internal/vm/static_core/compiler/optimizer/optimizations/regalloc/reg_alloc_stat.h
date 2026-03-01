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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_STAT_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_STAT_H

#include "optimizer/analysis/liveness_analyzer.h"

namespace ark::compiler {
class RegAllocStat {
public:
    explicit RegAllocStat(const ArenaVector<LifeIntervals *> &intervals);
    size_t GetRegCount() const
    {
        return regs_;
    }
    size_t GetVRegCount() const
    {
        return vregs_;
    }
    size_t GetSlotCount() const
    {
        return slots_;
    }
    size_t GetVSlotCount() const
    {
        return vslots_;
    }

private:
    size_t regs_ = 0;
    size_t vregs_ = 0;
    size_t slots_ = 0;
    size_t vslots_ = 0;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_STAT_H
