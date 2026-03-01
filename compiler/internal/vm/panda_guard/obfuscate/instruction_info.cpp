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

#include "instruction_info.h"
#include "program.h"
#include "configs/guard_context.h"

bool panda::guard::InstructionInfo::IsInnerReg() const
{
    // if regs[0] is less than regsNum, the register is v, otherwise, the register is a
    return this->ins_->GetReg(0) < this->function_->regsNum_;
}

void panda::guard::InstructionInfo::UpdateInsName(bool generateNewName)
{
    this->origin_ = this->ins_->GetId(0);
    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->origin_, generateNewName);
    this->ins_->GetId(0) = this->obfName_;
    this->function_->program_->prog_->strings.emplace(this->obfName_);
}

void panda::guard::InstructionInfo::UpdateInsFileName()
{
    this->origin_ = this->ins_->GetId(0);
    ReferenceFilePath filePath(this->function_->program_);
    filePath.SetFilePath(this->origin_);
    filePath.Init();
    filePath.Update();
    this->obfName_ = filePath.obfFilePath_;
    this->ins_->GetId(0) = this->obfName_;
    this->function_->program_->prog_->strings.emplace(this->obfName_);
}

void panda::guard::InstructionInfo::WriteNameCache() const
{
    GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(this->origin_, this->obfName_);
}

bool panda::guard::InstructionInfo::IsValid() const
{
    return this->ins_ != nullptr;
}

bool panda::guard::InstructionInfo::equalToOpcode(const pandasm::Opcode opcode) const
{
    return this->ins_->opcode == opcode;
}

bool panda::guard::InstructionInfo::notEqualToOpcode(const pandasm::Opcode opcode) const
{
    return !this->equalToOpcode(opcode);
}
