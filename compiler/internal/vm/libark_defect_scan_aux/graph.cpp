/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "graph.h"

namespace panda::defect_scan_aux {
using Opcode = compiler::Opcode;
using IntrinsicId = compiler::RuntimeInterface::IntrinsicId;

static std::unordered_map<Opcode, InstType> OPCODE_INSTTYPE_MAP_TABLE = {
    OPCODE_INSTTYPE_MAP_TABLE(BUILD_OPCODE_MAP_TABLE)};

static std::unordered_map<IntrinsicId, InstType> INTRINSIC_INSTTYPE_MAP_TABLE = {
    INTRINSIC_INSTTYPE_MAP_TABLE(BUILD_INTRINSIC_MAP_TABLE)};

bool Inst::operator==(const Inst &inst) const
{
    return inst_ == inst.inst_;
}

bool Inst::operator!=(const Inst &inst) const
{
    return inst_ != inst.inst_;
}

InstType Inst::GetType() const
{
    return type_;
}

bool Inst::IsInstStLexVar() const
{
    return type_ == InstType::STLEXVAR_IMM4_IMM4 || type_ == InstType::STLEXVAR_IMM8_IMM8 ||
           type_ == InstType::WIDE_STLEXVAR_PREF_IMM16_IMM16;
}

bool Inst::IsInstLdLexVar() const
{
    return type_ == InstType::LDLEXVAR_IMM4_IMM4 || type_ == InstType::LDLEXVAR_IMM8_IMM8 ||
           type_ == InstType::WIDE_LDLEXVAR_PREF_IMM16_IMM16;
}

bool Inst::IsInstStGlobal() const
{
    return type_ == InstType::TRYSTGLOBALBYNAME_IMM8_ID16 || type_ == InstType::TRYSTGLOBALBYNAME_IMM16_ID16 ||
           type_ == InstType::STGLOBALVAR_IMM16_ID16 || type_ == InstType::STCONSTTOGLOBALRECORD_IMM16_ID16 ||
           type_ == InstType::STTOGLOBALRECORD_IMM16_ID16;
}

bool Inst::IsInstLdGlobal() const
{
    return type_ == InstType::LDGLOBALVAR_IMM16_ID16 || type_ == InstType::TRYLDGLOBALBYNAME_IMM8_ID16 ||
           type_ == InstType::TRYLDGLOBALBYNAME_IMM16_ID16;
}

uint16_t Inst::GetArgIndex() const
{
    ASSERT(inst_->IsParameter());
    return inst_->CastToParameter()->GetArgNumber();
}

uint32_t Inst::GetPc() const
{
    return inst_->GetPc();
}

uint32_t Inst::GetInstId() const
{
    return inst_->GetId();
}

BasicBlock Inst::GetBasicBlock() const
{
    return BasicBlock(inst_->GetBasicBlock());
}

Graph Inst::GetGraph() const
{
    return Graph(GetBasicBlock().GetGraph());
}

std::vector<Inst> Inst::GetInputInsts() const
{
    std::vector<Inst> inputs;
    for (auto &input : inst_->GetInputs()) {
        if (!input.GetInst()->IsSaveState()) {
            inputs.emplace_back(input.GetInst());
        }
    }
    return inputs;
}

std::vector<Inst> Inst::GetUserInsts() const
{
    std::vector<Inst> users;
    auto user_list = inst_->GetUsers();
    for (auto &user : user_list) {
        if (!user.GetInst()->IsSaveState()) {
            users.emplace_back(user.GetInst());
        }
    }
    return users;
}

std::vector<uint32_t> Inst::GetImms() const
{
    ASSERT(inst_->IsIntrinsic());
    std::vector<uint32_t> imms;
    auto &intrinsic_imms = inst_->CastToIntrinsic()->GetImms();
    for (auto imm : intrinsic_imms) {
        imms.push_back(imm);
    }
    return imms;
}

InstType Inst::GetInstType(const compiler::Inst *inst)
{
    if (inst->IsIntrinsic()) {
        return INTRINSIC_INSTTYPE_MAP_TABLE[inst->CastToIntrinsic()->GetIntrinsicId()];
    }
    return OPCODE_INSTTYPE_MAP_TABLE[inst->GetOpcode()];
}

bool BasicBlock::operator==(const BasicBlock &bb) const
{
    return bb_ == bb.bb_;
}

bool BasicBlock::operator!=(const BasicBlock &bb) const
{
    return bb_ != bb.bb_;
}

Graph BasicBlock::GetGraph() const
{
    return Graph(bb_->GetGraph());
}

std::vector<BasicBlock> BasicBlock::GetPredBlocks() const
{
    std::vector<BasicBlock> pred_blocks;
    for (auto &bb : bb_->GetPredsBlocks()) {
        pred_blocks.emplace_back(bb);
    }
    return pred_blocks;
}

std::vector<BasicBlock> BasicBlock::GetSuccBlocks() const
{
    std::vector<BasicBlock> succ_blocks;
    for (auto &bb : bb_->GetSuccsBlocks()) {
        succ_blocks.emplace_back(bb);
    }
    return succ_blocks;
}

std::vector<Inst> BasicBlock::GetInstList() const
{
    std::vector<Inst> inst_list;
    for (auto inst : bb_->AllInsts()) {
        if (!inst->IsSaveState()) {
            inst_list.emplace_back(inst);
        }
    }
    return inst_list;
}

BasicBlock Graph::GetStartBasicBlock() const
{
    return BasicBlock(graph_->GetStartBlock());
}

BasicBlock Graph::GetEndBasicBlock() const
{
    return BasicBlock(graph_->GetEndBlock());
}

std::vector<BasicBlock> Graph::GetBasicBlockList() const
{
    std::vector<BasicBlock> bb_list;
    auto &blocks = graph_->GetBlocksRPO();
    for (auto &bb : blocks) {
        bb_list.emplace_back(bb);
    }
    return bb_list;
}

void Graph::VisitAllInstructions(const InstVisitor visitor) const
{
    for (auto &bb : graph_->GetBlocksRPO()) {
        std::vector<Inst> inst_list = BasicBlock(bb).GetInstList();
        for (auto &inst : inst_list) {
            visitor(inst);
        }
    }
}
}  // namespace panda::defect_scan_aux