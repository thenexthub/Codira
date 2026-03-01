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

#include "locations.h"
#include "inst.h"

namespace ark::compiler {
LocationsInfo::LocationsInfo(ArenaAllocator *allocator, Inst *inst)
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    : locations_(allocator->New<Location[]>(inst->GetInputsCount()), inst->GetInputsCount())
{
    ASSERT(inst->IsOperandsDynamic());
    static_cast<DynamicInputsInst *>(inst)->SetLocationsInfo(this);
}

void Location::Dump(std::ostream &stm, Arch arch)
{
    Target target {arch};

    switch (GetKind()) {
        case Kind::REGISTER:
        case Kind::FP_REGISTER:
            if (IsUnallocatedRegister()) {
                stm << GetName() << '?';
            } else if (GetValue() == GetAccReg()) {
                stm << "acc";
            } else {
                stm << target.GetRegName(GetValue(), IsFpRegister());
            }
            break;
        case Kind::IMMEDIATE:
        case Kind::STACK:
        case Kind::STACK_PARAMETER:
        case Kind::STACK_ARGUMENT:
            stm << GetName() << GetValue();
            break;
        default:
            stm << GetName();
            break;
    }
}

std::string Location::ToString(Arch arch)
{
    std::stringstream ss;
    Dump(ss, arch);
    return ss.str();
}
}  // namespace ark::compiler