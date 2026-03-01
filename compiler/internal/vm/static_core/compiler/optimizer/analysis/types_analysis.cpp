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

#include "types_analysis.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {
bool TypesAnalysis::RunImpl()
{
    marker_ = GetGraph()->NewMarker();
    VisitGraph();
    GetGraph()->EraseMarker(marker_);
    return true;
}

void TypesAnalysis::MarkedPhiRec(PhiInst *phi, AnyBaseType type)
{
    if (phi->SetMarker(marker_)) {
        auto phiType = phi->GetAnyType();
        // Phi has 2 inputs or users with different types
        if (phiType != type) {
            phi->SetAssumedAnyType(AnyBaseType::UNDEFINED_TYPE);
            return;
        }
        return;
    }
    phi->SetAssumedAnyType(type);
    for (auto &user : phi->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetOpcode() == Opcode::Phi) {
            MarkedPhiRec(userInst->CastToPhi(), type);
        }
    }
}

void TypesAnalysis::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    auto self = static_cast<TypesAnalysis *>(v);
    auto type = inst->CastToCastValueToAnyType()->GetAnyType();
    ASSERT(type != AnyBaseType::UNDEFINED_TYPE);
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetOpcode() == Opcode::Phi) {
            self->MarkedPhiRec(userInst->CastToPhi(), type);
        }
    }
}

void TypesAnalysis::VisitAnyTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto self = static_cast<TypesAnalysis *>(v);
    auto type = inst->CastToAnyTypeCheck()->GetAnyType();
    if (type == AnyBaseType::UNDEFINED_TYPE) {
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() == Opcode::Phi) {
        self->MarkedPhiRec(input->CastToPhi(), type);
    }
}
}  // namespace ark::compiler
