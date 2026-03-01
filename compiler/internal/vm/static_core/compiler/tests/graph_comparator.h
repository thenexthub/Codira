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

#include "optimizer/ir/ir_constructor.h"
#include "optimizer/analysis/rpo.h"

namespace ark::compiler {

class GraphComparator {
public:
    bool Compare(Graph *graph1, Graph *graph2)
    {
        graph1->InvalidateAnalysis<Rpo>();
        graph2->InvalidateAnalysis<Rpo>();
        if (graph1->GetBlocksRPO().size() != graph2->GetBlocksRPO().size()) {
            std::cerr << "Different number of blocks\n";
            return false;
        }
        for (auto it1 = graph1->GetBlocksRPO().begin(), it2 = graph2->GetBlocksRPO().begin();
             it1 != graph1->GetBlocksRPO().end(); it1++, it2++) {
            auto it = bbMap_.insert({*it1, *it2});
            if (!it.second) {
                return false;
            }
        }
        return std::equal(graph1->GetBlocksRPO().begin(), graph1->GetBlocksRPO().end(), graph2->GetBlocksRPO().begin(),
                          graph2->GetBlocksRPO().end(), [this](auto bb1, auto bb2) { return Compare(bb1, bb2); });
    }

    bool Compare(BasicBlock *block1, BasicBlock *block2)
    {
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
        auto instCmp = [this](auto inst1, auto inst2) {
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
                          block2->AllInsts().end(), instCmp);
    }

    bool InstInitialCompare(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetOpcode() != inst2->GetOpcode() || inst1->GetType() != inst2->GetType() ||
            inst1->GetInputsCount() != inst2->GetInputsCount()) {
            instCompareMap_.erase(inst1);
            return false;
        }
        if (inst1->GetFlagsMask() != inst2->GetFlagsMask()) {
            instCompareMap_.erase(inst1);
            return false;
        }
        if (inst1->GetOpcode() != Opcode::Phi) {
            auto inst1Begin = inst1->GetInputs().begin();
            auto inst1End = inst1->GetInputs().end();
            auto inst2Begin = inst2->GetInputs().begin();
            auto eqLambda = [this](Input input1, Input input2) { return Compare(input1.GetInst(), input2.GetInst()); };
            if (!std::equal(inst1Begin, inst1End, inst2Begin, eqLambda)) {
                instCompareMap_.erase(inst1);
                return false;
            }
        } else {
            if (inst1->GetInputsCount() != inst2->GetInputsCount()) {
                instCompareMap_.erase(inst1);
                return false;
            }
            for (size_t index1 = 0; index1 < inst1->GetInputsCount(); index1++) {
                auto input1 = inst1->GetInput(index1).GetInst();
                auto bb1 = inst1->CastToPhi()->GetPhiInputBb(index1);
                if (bbMap_.count(bb1) == 0) {
                    instCompareMap_.erase(inst1);
                    return false;
                }
                auto bb2 = bbMap_.at(bb1);
                auto input2 = inst2->CastToPhi()->GetPhiInput(bb2);
                if (!Compare(input1, input2)) {
                    instCompareMap_.erase(inst1);
                    return false;
                }
            }
        }
        return true;
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage
#define CAST(Opc) CastTo##Opc()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage
#define CHECK_OR_RETURN(Opc, Getter) /* CC-OFFNXT(G.PRE.02) namespace member */                          \
    if (inst1->GetOpcode() == Opcode::Opc && inst1->CAST(Opc)->Getter() != inst2->CAST(Opc)->Getter()) { \
        instCompareMap_.erase(inst1);                                                                    \
        return false;                                                                                    \
    }

    bool InstPropertiesCompare(Inst *inst1, Inst *inst2)
    {
        CHECK_OR_RETURN(Cast, GetOperandsType)
        CHECK_OR_RETURN(Cmp, GetOperandsType)

        CHECK_OR_RETURN(Compare, GetCc)
        CHECK_OR_RETURN(Compare, GetOperandsType)

        CHECK_OR_RETURN(If, GetCc)
        CHECK_OR_RETURN(If, GetOperandsType)

        CHECK_OR_RETURN(IfImm, GetCc)
        CHECK_OR_RETURN(IfImm, GetImm)
        CHECK_OR_RETURN(IfImm, GetOperandsType)

        CHECK_OR_RETURN(Select, GetCc)
        CHECK_OR_RETURN(Select, GetOperandsType)

        CHECK_OR_RETURN(SelectImm, GetCc)
        CHECK_OR_RETURN(SelectImm, GetImm)
        CHECK_OR_RETURN(SelectImm, GetOperandsType)

        CHECK_OR_RETURN(LoadArrayI, GetImm)
        CHECK_OR_RETURN(LoadArrayPairI, GetImm)
        CHECK_OR_RETURN(LoadPairPart, GetImm)
        CHECK_OR_RETURN(StoreArrayI, GetImm)
        CHECK_OR_RETURN(StoreArrayPairI, GetImm)
        CHECK_OR_RETURN(LoadArrayPair, GetImm)
        CHECK_OR_RETURN(StoreArrayPair, GetImm)
        CHECK_OR_RETURN(BoundsCheckI, GetImm)
        CHECK_OR_RETURN(ReturnI, GetImm)
        CHECK_OR_RETURN(AddI, GetImm)
        CHECK_OR_RETURN(SubI, GetImm)
        CHECK_OR_RETURN(ShlI, GetImm)
        CHECK_OR_RETURN(ShrI, GetImm)
        CHECK_OR_RETURN(AShrI, GetImm)
        CHECK_OR_RETURN(AndI, GetImm)
        CHECK_OR_RETURN(OrI, GetImm)
        CHECK_OR_RETURN(XorI, GetImm)

        return true;
    }

    bool InstAdditionalPropertiesCompare(Inst *inst1, Inst *inst2)
    {
        CHECK_OR_RETURN(LoadArray, GetNeedBarrier)
        CHECK_OR_RETURN(LoadArrayPair, GetNeedBarrier)
        CHECK_OR_RETURN(StoreArray, GetNeedBarrier)
        CHECK_OR_RETURN(StoreArrayPair, GetNeedBarrier)
        CHECK_OR_RETURN(LoadArrayI, GetNeedBarrier)
        CHECK_OR_RETURN(LoadArrayPairI, GetNeedBarrier)
        CHECK_OR_RETURN(StoreArrayI, GetNeedBarrier)
        CHECK_OR_RETURN(StoreArrayPairI, GetNeedBarrier)
        CHECK_OR_RETURN(LoadStatic, GetNeedBarrier)
        CHECK_OR_RETURN(StoreStatic, GetNeedBarrier)
        CHECK_OR_RETURN(LoadObject, GetNeedBarrier)
        CHECK_OR_RETURN(StoreObject, GetNeedBarrier)
        CHECK_OR_RETURN(LoadStatic, GetVolatile)
        CHECK_OR_RETURN(StoreStatic, GetVolatile)
        CHECK_OR_RETURN(LoadObject, GetVolatile)
        CHECK_OR_RETURN(StoreObject, GetVolatile)
        CHECK_OR_RETURN(NewObject, GetNeedBarrier)
        CHECK_OR_RETURN(NewArray, GetNeedBarrier)
        CHECK_OR_RETURN(CheckCast, GetNeedBarrier)
        CHECK_OR_RETURN(IsInstance, GetNeedBarrier)
        CHECK_OR_RETURN(LoadString, GetNeedBarrier)
        CHECK_OR_RETURN(LoadConstArray, GetNeedBarrier)
        CHECK_OR_RETURN(LoadType, GetNeedBarrier)

        CHECK_OR_RETURN(CallStatic, IsInlined)
        CHECK_OR_RETURN(CallVirtual, IsInlined)

        CHECK_OR_RETURN(LoadArray, IsArray)
        CHECK_OR_RETURN(LenArray, IsArray)

        CHECK_OR_RETURN(Deoptimize, GetDeoptimizeType)
        CHECK_OR_RETURN(DeoptimizeIf, GetDeoptimizeType)

        CHECK_OR_RETURN(CompareAnyType, GetAnyType)
        CHECK_OR_RETURN(CastValueToAnyType, GetAnyType)
        CHECK_OR_RETURN(CastAnyTypeValue, GetAnyType)
        CHECK_OR_RETURN(AnyTypeCheck, GetAnyType)

        CHECK_OR_RETURN(HclassCheck, GetCheckIsFunction)
        CHECK_OR_RETURN(HclassCheck, GetCheckFunctionIsNotClassConstructor)

        // Those below can fail because unit test Graph don't have proper Runtime links
        // CHECK_OR_RETURN(Intrinsic, GetEntrypointId)
        // CHECK_OR_RETURN(CallStatic, GetCallMethodId)
        // CHECK_OR_RETURN(CallVirtual, GetCallMethodId)

        // CHECK_OR_RETURN(InitClass, GetTypeId)
        // CHECK_OR_RETURN(LoadAndInitClass, GetTypeId)
        // CHECK_OR_RETURN(LoadStatic, GetTypeId)
        // CHECK_OR_RETURN(StoreStatic, GetTypeId)
        // CHECK_OR_RETURN(LoadObject, GetTypeId)
        // CHECK_OR_RETURN(StoreObject, GetTypeId)
        // CHECK_OR_RETURN(NewObject, GetTypeId)
        // CHECK_OR_RETURN(InitObject, GetTypeId)
        // CHECK_OR_RETURN(NewArray, GetTypeId)
        // CHECK_OR_RETURN(LoadConstArray, GetTypeId)
        // CHECK_OR_RETURN(CheckCast, GetTypeId)
        // CHECK_OR_RETURN(IsInstance, GetTypeId)
        // CHECK_OR_RETURN(LoadString, GetTypeId)
        // CHECK_OR_RETURN(LoadType, GetTypeId)

        return true;
    }
#undef CHECK_OR_RETURN
#undef CAST

    bool InstSaveStateCompare(Inst *inst1, Inst *inst2)
    {
        auto *svSt1 = static_cast<SaveStateInst *>(inst1);
        auto *svSt2 = static_cast<SaveStateInst *>(inst2);
        if (svSt1->GetImmediatesCount() != svSt2->GetImmediatesCount()) {
            instCompareMap_.erase(inst1);
            return false;
        }

        std::vector<VirtualRegister::ValueType> regs1;
        std::vector<VirtualRegister::ValueType> regs2;
        regs1.reserve(svSt1->GetInputsCount());
        regs2.reserve(svSt2->GetInputsCount());
        for (size_t i {0}; i < svSt1->GetInputsCount(); ++i) {
            regs1.emplace_back(svSt1->GetVirtualRegister(i).Value());
            regs2.emplace_back(svSt2->GetVirtualRegister(i).Value());
        }
        std::sort(regs1.begin(), regs1.end());
        std::sort(regs2.begin(), regs2.end());
        if (regs1 != regs2) {
            instCompareMap_.erase(inst1);
            return false;
        }
        if (svSt1->GetImmediatesCount() != 0) {
            auto eqLambda = [](SaveStateImm i1, SaveStateImm i2) {
                return i1.value == i2.value && i1.vreg == i2.vreg && i1.vregType == i2.vregType && i1.type == i2.type;
            };
            if (!std::equal(svSt1->GetImmediates()->begin(), svSt1->GetImmediates()->end(),
                            svSt2->GetImmediates()->begin(), eqLambda)) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        if (svSt1->GetInliningDepth() != svSt2->GetInliningDepth()) {
            instCompareMap_.erase(inst1);
            return false;
        }
        if ((svSt1->GetCallerInst() == nullptr) != (svSt2->GetCallerInst() == nullptr)) {
            instCompareMap_.erase(inst1);
            return false;
        }
        if (svSt1->GetCallerInst() != nullptr && svSt2->GetCallerInst() != nullptr &&
            svSt1->GetCallerInst()->GetId() != svSt2->GetCallerInst()->GetId()) {
            instCompareMap_.erase(inst1);
            return false;
        }
        return true;
    }

private:
    bool CompareCommon(Inst *inst1, Inst *inst2)
    {
        if (auto it = instCompareMap_.insert({inst1, inst2}); !it.second) {
            if (inst2 == it.first->second) {
                return true;
            }
            instCompareMap_.erase(inst1);
            return false;
        }

        if (!InstInitialCompare(inst1, inst2)) {
            return false;
        }

        if (!InstPropertiesCompare(inst1, inst2)) {
            return false;
        }

        if (!InstAdditionalPropertiesCompare(inst1, inst2)) {
            return false;
        }
        return true;
    }

public:
    bool Compare(Inst *inst1, Inst *inst2)
    {
        if (!CompareCommon(inst1, inst2)) {
            return false;
        }

        if (inst1->GetOpcode() == Opcode::Constant) {
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
            if (!same) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        if (inst1->GetOpcode() == Opcode::Cmp && IsFloatType(inst1->GetInput(0).GetInst()->GetType())) {
            auto cmp1 = static_cast<CmpInst *>(inst1);
            auto cmp2 = static_cast<CmpInst *>(inst2);
            if (cmp1->IsFcmpg() != cmp2->IsFcmpg()) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        for (size_t i = 0; i < inst2->GetInputsCount(); i++) {
            if (inst1->GetInputType(i) != inst2->GetInputType(i)) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        if (inst1->IsSaveState()) {
            return InstSaveStateCompare(inst1, inst2);
        }
        return true;
    }

private:
    std::unordered_map<Inst *, Inst *> instCompareMap_;
    std::unordered_map<BasicBlock *, BasicBlock *> bbMap_;
};
}  // namespace ark::compiler

#endif  // COMPILER_TESTS_GRAPH_COMPARATOR_H
