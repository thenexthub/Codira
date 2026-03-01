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

// This file should be included inside each Visitor class
public:
void VisitGraph() override
{
    for (auto &bb : GetBlocksToVisit()) {
        for (auto inst : bb->AllInsts()) {
            TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
        }
    }
}

uint64_t VisitGraphAndCount() override
{
    uint64_t instsNumber = 0;
    for (auto &bb : GetBlocksToVisit()) {
        for (auto inst : bb->AllInsts()) {
            instsNumber++;
            TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
        }
    }
    return instsNumber;
}

void VisitBlock(BasicBlock *bb) override
{
    for (auto inst : bb->AllInsts()) {
        TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
    }
}
void VisitInstruction(Inst *inst) override
{
    TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
}

private:
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr VisitFunc TABLE[static_cast<unsigned>(Opcode::NUM_OPCODES)] = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, ...) Visit##opcode,
    OPCODE_LIST(INST_DEF)
#undef INST_DEF
};
