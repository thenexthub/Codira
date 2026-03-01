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

#include "optimizer/analysis/liveness_use_table.h"

namespace ark::compiler {
UseTable::UseTable(ArenaAllocator *allocator) : table_(allocator->Adapter()), allocator_(allocator) {}

void UseTable::AddUseOnFixedLocation(const Inst *inst, Location location, LifeNumber ln)
{
    auto res = table_.try_emplace(inst, allocator_->Adapter());
    auto &uses = res.first->second;
    ASSERT(location.IsRegisterValid());
    uses[ln] = location.GetValue();
}

bool UseTable::HasUseOnFixedLocation(const Inst *inst, LifeNumber ln) const
{
    auto it = table_.find(inst);
    if (it == table_.end()) {
        return false;
    }
    const auto &uses = it->second;
    return uses.count(ln) > 0;
}

Register UseTable::GetNextUseOnFixedLocation(const Inst *inst, LifeNumber ln) const
{
    auto it = table_.find(inst);
    if (it == table_.end()) {
        return GetInvalidReg();
    }
    const auto &uses = it->second;
    auto usesIt = uses.lower_bound(ln);
    return usesIt == uses.end() ? GetInvalidReg() : usesIt->second;
}

void UseTable::Dump(std::ostream &out, Arch arch) const
{
    out << "UseTable" << std::endl;
    for (auto [inst, uses] : table_) {
        out << "Inst v" << inst->GetId() << ": ";
        auto sep = "";
        for (auto [ln, r] : uses) {
            out << sep << "{" << std::to_string(ln) << ", " << Location::MakeRegister(r, inst->GetType()).ToString(arch)
                << "}";
            sep = ", ";
        }
        out << std::endl;
    }
}
}  // namespace ark::compiler