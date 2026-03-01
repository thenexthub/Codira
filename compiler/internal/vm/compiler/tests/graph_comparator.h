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

#ifndef COMPILER_TESTS_GRAPH_COMPARATOR_H
#define COMPILER_TESTS_GRAPH_COMPARATOR_H

#include <algorithm>
#include <iostream>
#include "optimizer/analysis/rpo.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"

namespace panda::compiler {

class GraphComparator {
public:
    bool Compare(Graph *graph1, Graph *graph2)
    {
        ASSERT(graph1 != nullptr);
        ASSERT(graph2 != nullptr);
        graph1->InvalidateAnalysis<Rpo>();
        graph2->InvalidateAnalysis<Rpo>();
        if (graph1->GetBlocksRPO().size() != graph2->GetBlocksRPO().size()) {
            std::cerr << "Different number of blocks\n";
            return false;
        }
        for (auto it1 = graph1->GetBlocksRPO().begin(), it2 = graph2->GetBlocksRPO().begin();
             it1 != graph1->GetBlocksRPO().end(); it1++, it2++) {
            auto it = bb_map_.insert({*it1, *it2});
            if (!it.second) {
                return false;
            }
        }
        return std::equal(graph1->GetBlocksRPO().begin(), graph1->GetBlocksRPO().end(), graph2->GetBlocksRPO().begin(),
                          graph2->GetBlocksRPO().end(), [this](auto bb1, auto bb2) { return Compare(bb1, bb2); });
    }

    bool Compare(BasicBlock *block1, BasicBlock *block2)
    {
        ASSERT(block1 != nullptr);
        ASSERT(block2 != nullptr);
        if (block1->GetPredsBlocks().size() != block2->GetPredsBlocks().size()) {
            std::cerr << "Different number of preds blocks\n";
            block1->Dump(&std::cerr);
            block2->Dump(&std::cerr);
            return false;
        }
        if (block1->GetSuccsBlocks().size() != block2->GetSuccsBlocks().size()) {
            std::cerr << "Different number of succs blocks\n";
            block1->Dump(&std::cerr);
            block2->Dump(&std::cerr);
            return false;
        }
        auto inst_cmp = [this](auto inst1, auto inst2) {
            ASSERT(inst2 != nullptr);
            bool t = Compare(inst1, inst2);
            if (!t) {
                std::cerr << "Different instructions:\n";
                inst1->Dump(&std::cerr);
                inst2->Dump(&std::cerr);
            }
            return t;
        };
        return std::equal(block1->AllInsts().begin(), block1->AllInsts().end(), block2->AllInsts().begin(),
                          block2->AllInsts().end(), inst_cmp);
    }

    bool Compare(Inst *inst1, Inst *inst2)
    {
        ASSERT(inst1 != nullptr);
        ASSERT(inst2 != nullptr);
        if (auto it = inst_compare_map_.insert({inst1, inst2}); !it.second) {
            if (inst2 == it.first->second) {
                return true;
            }
            inst_compare_map_.erase(inst1);
            return false;
        }

        if (inst1->GetOpcode() != inst2->GetOpcode() || inst1->GetType() != inst2->GetType() ||
            inst1->GetInputsCount() != inst2->GetInputsCount()) {
                inst_compare_map_.erase(inst1);
                return false;
        }

        bool result = (inst1->GetOpcode() != Opcode::Phi) ?
                      CompareNonPhiInputs(inst1, inst2) : ComparePhiInputs(inst1, inst2);
        if (!result) {
            inst_compare_map_.erase(inst1);
            return false;
        }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage
#define CAST(Opc) CastTo##Opc()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage
#define CHECK_INST(Opc, Getter)                                                                               \
    if (inst1->GetOpcode() == Opcode::Opc && inst1->CAST(Opc)->Getter() != inst2->CAST(Opc)->Getter()) { \
        inst_compare_map_.erase(inst1);                                                                  \
        return false;                                                                                    \
    }
        CHECK_INST(CastAnyTypeValue, GetDeducedType)

        CHECK_INST(Cmp, GetOperandsType)

        CHECK_INST(Compare, GetCc)
        CHECK_INST(Compare, GetOperandsType)

        CHECK_INST(If, GetCc)
        CHECK_INST(If, GetOperandsType)

        CHECK_INST(IfImm, GetCc)
        CHECK_INST(IfImm, GetImm)
        CHECK_INST(IfImm, GetOperandsType)

        CHECK_INST(LoadString, GetNeedBarrier)

        CHECK_INST(CompareAnyType, GetAnyType)
        CHECK_INST(CastValueToAnyType, GetAnyType)
        CHECK_INST(CastAnyTypeValue, GetAnyType)

        // Those below can fail because unit test Graph don't have proper Runtime links
        // CHECK_INST(Intrinsic, GetEntrypointId)
        // CHECK_INST(CallStatic, GetCallMethodId)
        // CHECK_INST(CallVirtual, GetCallMethodId)

        // CHECK_INST(InitClass, GetTypeId)
        // CHECK_INST(LoadAndInitClass, GetTypeId)
        // CHECK_INST(LoadStatic, GetTypeId)
        // CHECK_INST(StoreStatic, GetTypeId)
        // CHECK_INST(LoadObject, GetTypeId)
        // CHECK_INST(StoreObject, GetTypeId)
        // CHECK_INST(NewObject, GetTypeId)
        // CHECK_INST(InitObject, GetTypeId)
        // CHECK_INST(NewArray, GetTypeId)
        // CHECK_INST(LoadConstArray, GetTypeId)
        // CHECK_INST(CHECK_INSTCast, GetTypeId)
        // CHECK_INST(IsInstance, GetTypeId)
        // CHECK_INST(LoadString, GetTypeId)
        // CHECK_INST(LoadType, GetTypeId)
#undef CHECK_INST
#undef CAST
        if (!CompareInputTypes(inst1, inst2)
            || !CompareIntrinsicInst(inst1, inst2) || !CompareConstantInst(inst1, inst2)
            || !CompareFcmpgInst(inst1, inst2) || !CompareSaveStateInst(inst1, inst2)) {
            inst_compare_map_.erase(inst1);
            return false;
        }

        return true;
    }
private:
    std::unordered_map<Inst *, Inst *> inst_compare_map_;
    std::unordered_map<BasicBlock *, BasicBlock *> bb_map_;

    bool CompareNonPhiInputs(Inst *inst1, Inst *inst2)
    {
        auto inst1_begin = inst1->GetInputs().begin();
        auto inst1_end = inst1->GetInputs().end();
        auto inst2_begin = inst2->GetInputs().begin();
        auto eq_lambda = [this](Input input1, Input input2) {
            return Compare(input1.GetInst(), input2.GetInst());
        };
        return std::equal(inst1_begin, inst1_end, inst2_begin, eq_lambda);
    }

    bool ComparePhiInputs(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetInputsCount() != inst2->GetInputsCount()) {
            return false;
        }

        for (size_t index1 = 0; index1 < inst1->GetInputsCount(); index1++) {
            auto input1 = inst1->GetInput(index1).GetInst();
            auto bb1 = inst1->CastToPhi()->GetPhiInputBb(index1);
            if (bb_map_.count(bb1) == 0) {
                return false;
            }
            auto bb2 = bb_map_.at(bb1);
            auto input2 = inst2->CastToPhi()->GetPhiInput(bb2);
            if (!Compare(input1, input2)) {
                return false;
            }
        }
        return true;
    }

    bool CompareIntrinsicInst(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetOpcode() != Opcode::Intrinsic) {
            return true;
        }

        auto intrinsic1 = inst1->CastToIntrinsic();
        auto intrinsic2 = inst2->CastToIntrinsic();
        auto same = intrinsic1->GetIntrinsicId() == intrinsic2->GetIntrinsicId();
        if (intrinsic1->HasImms()) {
            auto imms1 = intrinsic1->GetImms();
            auto imms2 = intrinsic2->GetImms();
            same = same && std::equal(imms1.begin(), imms1.end(), imms2.begin(), imms2.end());
        }
        return same;
    }

    bool CompareConstantInst(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetOpcode() != Opcode::Constant) {
            return true;
        }

        auto c1 = inst1->CastToConstant();
        auto c2 = inst2->CastToConstant();
        bool same = false;
        switch (inst1->GetType()) {
            case DataType::FLOAT32:
            case DataType::INT32:
                same = static_cast<uint32_t>(c1->GetRawValue()) == static_cast<uint32_t>(c2->GetRawValue());
                break;
            default:
                same = c1->GetRawValue() == c2->GetRawValue();
                break;
        }
        return same;
    }

    bool CompareFcmpgInst(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetOpcode() != Opcode::Cmp || !IsFloatType(inst1->GetInput(0).GetInst()->GetType())) {
            return true;
        }

        auto cmp1 = static_cast<CmpInst *>(inst1);
        auto cmp2 = static_cast<CmpInst *>(inst2);
        return cmp1->IsFcmpg() == cmp2->IsFcmpg();
    }

    bool CompareInputTypes(Inst *inst1, Inst *inst2)
    {
        for (size_t i = 0; i < inst2->GetInputsCount(); i++) {
            if (inst1->GetInputType(i) != inst2->GetInputType(i)) {
                return false;
            }
        }
        return true;
    }

    bool CompareSaveStateInst(Inst *inst1, Inst *inst2)
    {
        if (!inst1->IsSaveState()) {
            return true;
        }

        auto *sv_st1 = static_cast<SaveStateInst *>(inst1);
        auto *sv_st2 = static_cast<SaveStateInst *>(inst2);
        if (sv_st1->GetImmediatesCount() != sv_st2->GetImmediatesCount()) {
            return false;
        }

        std::vector<VirtualRegister::ValueType> regs1;
        std::vector<VirtualRegister::ValueType> regs2;
        regs1.reserve(sv_st1->GetInputsCount());
        regs2.reserve(sv_st2->GetInputsCount());
        for (size_t i {0}; i < sv_st1->GetInputsCount(); ++i) {
            regs1.emplace_back(sv_st1->GetVirtualRegister(i).Value());
            regs2.emplace_back(sv_st2->GetVirtualRegister(i).Value());
        }
        std::sort(regs1.begin(), regs1.end());
        std::sort(regs2.begin(), regs2.end());
        if (regs1 != regs2) {
            return false;
        }
        if (sv_st1->GetImmediatesCount() != 0) {
            auto eq_lambda = [](SaveStateImm i1, SaveStateImm i2) {
                return i1.value == i2.value && i1.vreg == i2.vreg && i1.is_acc == i2.is_acc;
            };
            if (!std::equal(sv_st1->GetImmediates()->begin(), sv_st1->GetImmediates()->end(),
                            sv_st2->GetImmediates()->begin(), eq_lambda)) {
                return false;
            }
        }
        return true;
    }
};
}  // namespace panda::compiler

#endif  // COMPILER_TESTS_GRAPH_COMPARATOR_H
