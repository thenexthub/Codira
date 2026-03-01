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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "compiler_logger.h"
#include "vn.h"

namespace panda::compiler {
class BasicBlock;

static void AddSpecialTraits(Inst *inst, VnObject *obj)
{
    switch (inst->GetOpcode()) {
        case Opcode::Intrinsic:
            if (inst->GetFlagsMask() == 0) {
                /* Add only those intrinsics that have no flags set */
                obj->Add(static_cast<uint32_t>(inst->CastToIntrinsic()->GetIntrinsicId()));
            }
            break;
        case Opcode::CompareAnyType:
            obj->Add(static_cast<uint32_t>(inst->CastToCompareAnyType()->GetAnyType()));
            break;
        case Opcode::CastAnyTypeValue:
            obj->Add(static_cast<uint32_t>(inst->CastToCastAnyTypeValue()->GetAnyType()));
            break;
        default:
            break;
    }
}

void VnObject::Add(Inst *inst)
{
    Add(static_cast<uint32_t>(inst->GetOpcode()));
    Add(static_cast<uint32_t>(inst->GetType()));

    AddSpecialTraits(inst, this);

    for (auto input : inst->GetInputs()) {
        auto input_inst = inst->GetDataFlowInput(input.GetInst());
        auto vn = input_inst->GetVN();
        ASSERT(vn != INVALID_VN);
        Add(vn);
    }

    inst->SetVnObject(this);
}

void VnObject::Add(uint32_t obj)
{
    ASSERT(size_objs_ < MAX_ARRAY_SIZE);
    objs_[size_objs_++] = obj;
}

void VnObject::Add(uint64_t obj)
{
    ASSERT(size_objs_ < MAX_ARRAY_SIZE);
    static constexpr uint64_t MASK32 = std::numeric_limits<uint32_t>::max();
    static constexpr uint64_t SHIFT32 = 32;
    objs_[size_objs_++] = static_cast<uint32_t>(obj & MASK32);
    objs_[size_objs_++] = static_cast<uint32_t>(obj >> SHIFT32);
}

bool VnObject::Compare(VnObject *obj)
{
    uint32_t size = GetSize();
    if (size != obj->GetSize()) {
        return false;
    }
    for (uint32_t i = 0; i < size; ++i) {
        if (GetElement(i) != obj->GetElement(i)) {
            return false;
        }
    }
    return true;
}

ValNum::ValNum(Graph *graph) : Optimization(graph), map_insts_(GetGraph()->GetLocalAllocator()->Adapter()) {}

inline void ValNum::SetInstValNum(Inst *inst)
{
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << curr_vn_ << " for inst " << inst->GetId();
    inst->SetVN(curr_vn_++);
}

void ValNum::InvalidateAnalyses()
{
}

bool ValNum::TryToApplyCse(Inst *inst, InstVector *equiv_insts)
{
    ASSERT(!equiv_insts->empty());
    inst->SetVN((*equiv_insts)[0]->GetVN());
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << inst->GetVN() << " for inst " << inst->GetId();
    auto block = inst->GetBasicBlock();
    for (auto equiv_inst : *equiv_insts) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are found, id " << equiv_inst->GetId();
        if (block == equiv_inst->GetBasicBlock() || equiv_inst->IsDominate(inst)) {
            COMPILER_LOG(DEBUG, VN_OPT) << " CSE is applied for inst with id " << inst->GetId();
            GetGraph()->GetEventWriter().EventGvn(inst->GetId(), inst->GetPc(), equiv_inst->GetId(),
                                                  equiv_inst->GetPc());
            inst->ReplaceUsers(equiv_inst);
            inst->ClearFlag(compiler::inst_flags::NO_DCE);
            cse_is_appied_ = true;
            return true;
        }
    }

    return false;
}

void ValNum::FindEqualVnOrCreateNew(Inst *inst)
{
    // create new vn for instruction with the property NO_CSE
    if (inst->IsNotCseApplicable()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " The inst with id " << inst->GetId() << " has the property NO_CSE";
        SetInstValNum(inst);
        return;
    }
    auto obj = GetGraph()->GetLocalAllocator()->New<VnObject>();
    CHECK_NOT_NULL(obj);
    obj->Add(inst);
    COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are searched for inst with id " << inst->GetId();
    auto it = map_insts_.find(obj);
    if (it == map_insts_.cend()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions aren't found";
        SetInstValNum(inst);
        InstVector equiv_insts(GetGraph()->GetLocalAllocator()->Adapter());
        equiv_insts.push_back(inst);
        map_insts_.insert({obj, std::move(equiv_insts)});
        return;
    }

    auto &equiv_insts = it->second;
    if (!TryToApplyCse(inst, &equiv_insts)) {
        equiv_insts.push_back(inst);
    }
}

bool ValNum::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            inst->SetVN(INVALID_VN);
        }
    }
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FindEqualVnOrCreateNew(inst);
        }
    }
    return cse_is_appied_;
}
}  // namespace panda::compiler
