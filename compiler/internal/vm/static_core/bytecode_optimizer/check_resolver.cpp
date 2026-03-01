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

#include "check_resolver.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/inst.h"

namespace ark::bytecodeopt {

static void ReplaceCheck(compiler::Inst *inst)
{
    auto op = inst->GetOpcode();
    size_t i = (op == compiler::Opcode::BoundsCheck || op == compiler::Opcode::RefTypeCheck) ? 1U : 0U;
    auto input = inst->GetInput(i).GetInst();
    for (auto &user : inst->GetUsers()) {
        user.GetInst()->SetFlag(compiler::inst_flags::CAN_THROW);
    }
    inst->ReplaceUsers(input);
    inst->ClearFlag(compiler::inst_flags::NO_DCE);  // DCE will remove the check inst
}

static void MarkLenArray(compiler::Inst *inst)
{
    bool noDce = !inst->HasUsers();
    for (const auto &usr : inst->GetUsers()) {
        if (!CheckResolver::IsCheck(usr.GetInst())) {
            noDce = true;
            break;
        }
    }
    if (noDce) {
        inst->SetFlag(compiler::inst_flags::NO_DCE);
    }
    inst->SetFlag(compiler::inst_flags::NO_HOIST);
}

bool CheckResolver::RunImpl()
{
    bool applied = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            // replace check
            if (IsCheck(inst)) {
                ReplaceCheck(inst);
                applied = true;
            }

            // set NO_DCE for Div, Mod, LoadArray, LoadStatic and LoadObject
            if (NeedSetNoDCE(inst)) {
                inst->SetFlag(compiler::inst_flags::NO_DCE);
                applied = true;
            }

            // mark LenArray whose users are not all check as no_dce
            // mark LenArray as no_hoist in all cases
            if (inst->GetOpcode() == compiler::Opcode::LenArray) {
                MarkLenArray(inst);
            }
        }
    }
    return applied;
}

bool CheckResolver::NeedSetNoDCE(const compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case compiler::Opcode::Div:
        case compiler::Opcode::Mod:
        case compiler::Opcode::LoadArray:
        case compiler::Opcode::LoadStatic:
        case compiler::Opcode::LoadObject:
            return true;
        default:
            return false;
    }
}

bool CheckResolver::IsCheck(const compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case compiler::Opcode::BoundsCheck:
        case compiler::Opcode::NullCheck:
        case compiler::Opcode::NegativeCheck:
        case compiler::Opcode::ZeroCheck:
        case compiler::Opcode::RefTypeCheck:
        case compiler::Opcode::BoundsCheckI:
            return true;
        default:
            return false;
    }
}

}  // namespace ark::bytecodeopt
