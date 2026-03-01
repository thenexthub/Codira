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

#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/types_analysis.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/phi_type_resolving.h"

namespace ark::compiler {
PhiTypeResolving::PhiTypeResolving(Graph *graph) : Optimization(graph), phis_ {graph->GetLocalAllocator()->Adapter()} {}

void PhiTypeResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool PhiTypeResolving::RunImpl()
{
    bool isApplied = false;
    GetGraph()->RunPass<DominatorsTree>();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        // We use reverse iter because new instrucion is inserted after last phi and forward iteratoris broken.
        for (auto inst : bb->PhiInstsSafeReverse()) {
            if (inst->GetType() != DataType::ANY) {
                continue;
            }
            phis_.clear();
            anyType_ = AnyBaseType::UNDEFINED_TYPE;
            if (!CheckInputsAnyTypesRec(inst)) {
                continue;
            }
            PropagateTypeToPhi();
            ASSERT(inst->GetType() != DataType::ANY);
            isApplied = true;
        }
    }
    return isApplied;
}

bool PhiTypeResolving::CheckInputsAnyTypesRec(Inst *phi)
{
    ASSERT(phi->IsPhi());
    if (std::find(phis_.begin(), phis_.end(), phi) != phis_.end()) {
        return true;
    }

    if (GetGraph()->IsOsrMode()) {
        for (auto &user : phi->GetUsers()) {
            if (user.GetInst()->GetOpcode() == Opcode::SaveStateOsr) {
                // Find way to enable it in OSR mode.
                return false;
            }
        }
    }

    phis_.push_back(phi);
    int32_t inputNum = -1;
    for (auto &input : phi->GetInputs()) {
        ++inputNum;
        auto inputInst = phi->GetDataFlowInput(input.GetInst());
        if (GetGraph()->IsOsrMode()) {
            if (HasOsrEntryBetween<BasicBlock>(inputInst->GetBasicBlock(), phi->CastToPhi()->GetPhiInputBb(inputNum))) {
                return false;
            }
        }
        if (inputInst->GetOpcode() == Opcode::Phi) {
            if (!CheckInputsAnyTypesRec(inputInst)) {
                return false;
            }
            continue;
        }
        if (inputInst->GetOpcode() != Opcode::CastValueToAnyType) {
            return false;
        }
        auto type = inputInst->CastToCastValueToAnyType()->GetAnyType();
        ASSERT(type != AnyBaseType::UNDEFINED_TYPE);
        if (anyType_ == AnyBaseType::UNDEFINED_TYPE) {
            // We can't propogate opject, because GC can move it
            if (AnyBaseTypeToDataType(type) == DataType::REFERENCE) {
                return false;
            }
            anyType_ = type;
            continue;
        }
        if (anyType_ != type) {
            return false;
        }
    }
    return AnyBaseTypeToDataType(anyType_) != DataType::ANY;
}

void PhiTypeResolving::PropagateTypeToPhi()
{
    auto newType = AnyBaseTypeToDataType(anyType_);
    for (auto phi : phis_) {
        phi->SetType(newType);
        size_t inputsCount = phi->GetInputsCount();
        for (size_t idx = 0; idx < inputsCount; ++idx) {
            auto inputInst = phi->GetDataFlowInput(idx);
            if (inputInst->GetOpcode() == Opcode::CastValueToAnyType) {
                phi->SetInput(idx, inputInst->GetInput(0).GetInst());
            } else if (phi->GetInput(idx).GetInst() != inputInst) {
                ASSERT(std::find(phis_.begin(), phis_.end(), phi) != phis_.end());
                // case:
                // 2.any Phi v1(bb1), v3(bb3) -> v3
                // 3.any AnyTypeCheck v2 - > v2
                ASSERT(phi->GetInput(idx).GetInst()->GetOpcode() == Opcode::AnyTypeCheck);
                phi->SetInput(idx, inputInst);
            } else {
                ASSERT(std::find(phis_.begin(), phis_.end(), phi) != phis_.end());
            }
        }
        auto *castToAnyInst = GetGraph()->CreateInstCastValueToAnyType(phi->GetPc(), anyType_, nullptr);
        auto *bb = phi->GetBasicBlock();
        auto *first = bb->GetFirstInst();
        if (first == nullptr) {
            bb->AppendInst(castToAnyInst);
        } else if (first->IsSaveState()) {
            bb->InsertAfter(castToAnyInst, first);
        } else {
            bb->InsertBefore(castToAnyInst, first);
        }

        for (auto it = phi->GetUsers().begin(); it != phi->GetUsers().end();) {
            auto userInst = it->GetInst();
            if ((userInst->IsPhi() && userInst->GetType() != DataType::ANY) ||
                (userInst == first && userInst->IsSaveState())) {
                ++it;
                continue;
            }
            userInst->SetInput(it->GetIndex(), castToAnyInst);
            it = phi->GetUsers().begin();
        }
        castToAnyInst->SetInput(0, phi);
    }
}
}  // namespace ark::compiler
