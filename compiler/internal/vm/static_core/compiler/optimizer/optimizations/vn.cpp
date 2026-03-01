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

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "vn.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "compiler_logger.h"

namespace ark::compiler {
class BasicBlock;

inline bool IsAddressArithmeticInst(Inst *inst)
{
    return inst->GetType() == DataType::POINTER &&
           (inst->GetOpcode() == Opcode::AddI || inst->GetOpcode() == Opcode::SubI);
}

static bool IsIrreducibleClassInst(Inst *inst, Inst *equivInst)
{
    return equivInst->GetOpcode() == Opcode::LoadClass &&
           (inst->GetOpcode() == Opcode::InitClass || inst->GetOpcode() == Opcode::LoadAndInitClass);
}

void VnObject::Set(Inst *inst)
{
    inst->SetVnObject(this);
}

void VnObject::Add(HalfObjType obj1, HalfObjType obj2)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    static constexpr HalfObjType SHIFT16 = 16;
    ObjType obj = static_cast<ObjType>(obj1) << SHIFT16;
    obj |= static_cast<ObjType>(obj2);
    objs_[sizeObjs_++] = obj;
}

void VnObject::Add(ObjType obj)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    objs_[sizeObjs_++] = obj;
}

void VnObject::Add(DoubleObjType obj)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    static constexpr DoubleObjType MASK32 = std::numeric_limits<uint32_t>::max();
    static constexpr DoubleObjType SHIFT32 = 32;
    objs_[sizeObjs_++] = static_cast<ObjType>(obj & MASK32);
    objs_[sizeObjs_++] = static_cast<ObjType>(obj >> SHIFT32);
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

ValNum::ValNum(Graph *graph) : Optimization(graph), mapInsts_(GetGraph()->GetLocalAllocator()->Adapter()) {}

inline void ValNum::SetInstValNum(Inst *inst)
{
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << currVn_ << " for inst " << inst->GetId();
    inst->SetVN(currVn_++);
}

void ValNum::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool ValNum::TryToApplyCse(Inst *inst, InstVector *equivInsts)
{
    ASSERT(!equivInsts->empty());
    inst->SetVN((*equivInsts)[0]->GetVN());
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << inst->GetVN() << " for inst " << inst->GetId();
    for (auto equivInst : *equivInsts) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are found, id " << equivInst->GetId();
        if (IsIrreducibleClassInst(inst, equivInst)) {
            continue;
        }
        if (IsInstInDifferentBlocks(inst, equivInst)) {
            if (!equivInst->IsDominate(inst) || (inst->IsResolver() && HasTryBlockBetween(equivInst, inst)) ||
                HasOsrEntryBetween(equivInst, inst)) {
                continue;
            }
        }
        if (IsAddressArithmeticInst(inst) && HasSaveStateBetween<IsSaveStateCanTriggerGc>(equivInst, inst)) {
            continue;
        }
        // Check bridges
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), equivInst, inst);
        } else {
            // check result can be moved by GC, but checks have no_cse flag
            ASSERT(!inst->IsCheck());
            COMPILER_LOG(DEBUG, VN_OPT) << " Don't need to do bridge for opcode " << *inst;
        }

        COMPILER_LOG(DEBUG, VN_OPT) << " CSE is applied for inst with id " << inst->GetId();
        GetGraph()->GetEventWriter().EventGvn(inst->GetId(), inst->GetPc(), equivInst->GetId(), equivInst->GetPc());
        inst->ReplaceUsers(equivInst);
        if (equivInst->GetOpcode() == Opcode::InitClass &&
            (inst->GetOpcode() == Opcode::LoadClass || inst->GetOpcode() == Opcode::LoadAndInitClass)) {
            equivInst->SetOpcode(Opcode::LoadAndInitClass);
            equivInst->SetType(DataType::REFERENCE);
        }

        // IsInstance, InitClass, LoadClass and LoadAndInitClass have attribute NO_DCE,
        // so they can't be removed by DCE pass. But we can remove the instructions after VN
        // because there is dominate equal instruction.
        inst->ClearFlag(compiler::inst_flags::NO_DCE);
        cseIsApplied_ = true;
        return true;
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

    // If there will be LoopUnroll pass, avoid preheader modification:
    if (inst->GetOpcode() == Opcode::Compare && !GetGraph()->IsBytecodeOptimizer() &&
        g_options.IsCompilerDeferPreheaderTransform() && !GetGraph()->IsUnrollComplete()) {
        auto bb = inst->GetBasicBlock();
        if (bb->IsLoopPreHeader() && inst->HasSingleUser() && inst->GetFirstUser()->GetInst() == bb->GetLastInst() &&
            bb->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            COMPILER_LOG(DEBUG, VN_OPT) << " The inst with id " << inst->GetId()
                                        << " is in preheader and thus deferred";
            SetInstValNum(inst);
            return;
        }
    }

    auto obj = GetGraph()->GetLocalAllocator()->New<VnObject>();
    ASSERT(obj != nullptr);
    obj->Set(inst);
    COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are searched for inst with id " << inst->GetId();
    auto it = mapInsts_.find(obj);
    if (it == mapInsts_.cend()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions aren't found";
        SetInstValNum(inst);
        InstVector equivInsts(GetGraph()->GetLocalAllocator()->Adapter());
        equivInsts.push_back(inst);
        mapInsts_.insert({obj, std::move(equivInsts)});
        return;
    }

    auto &equivInsts = it->second;
    if (!TryToApplyCse(inst, &equivInsts)) {
        equivInsts.push_back(inst);
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
    return cseIsApplied_;
}
}  // namespace ark::compiler
