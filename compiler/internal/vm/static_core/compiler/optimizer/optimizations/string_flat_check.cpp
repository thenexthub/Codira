/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "optimizer/optimizations/string_flat_check.h"
#include "optimizer/ir/runtime_interface.h"
#include "intrinsic_string_flat_check.inl"
#include "optimizer/ir/analysis.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/linear_order.h"

namespace ark::compiler {
StringFlatCheck::StringFlatCheck(Graph *graph) : Optimization(graph), users_(graph->GetAllocator()->Adapter()) {}

const ArenaVector<BasicBlock *> &StringFlatCheck::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

bool StringFlatCheck::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    if (GetGraph()->IsOsrMode()) {
        GetGraph()->RunPass<LoopAnalyzer>();
    }

    VisitGraph();
    if (applied_) {
        FixSaveStates();
    }
    return applied_;
}

bool StringFlatCheck::IsEnable() const
{
    return IsEnable(GetGraph()->GetRuntime());
}

bool StringFlatCheck::IsEnable(RuntimeInterface *runtime)
{
    return g_options.IsCompilerStringFlatCheck() && runtime->CanUseStringFlatCheck();
}

void StringFlatCheck::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    auto *intrinsic = inst->CastToIntrinsic();
    auto stringFlatCheckArgMask = GetStringFlatCheckArgMask(intrinsic->GetIntrinsicId());
    if (stringFlatCheckArgMask == 0) {
        return;
    }

    auto *visitor = static_cast<StringFlatCheck *>(v);
    visitor->InsertStringFlatCheck(intrinsic, stringFlatCheckArgMask);
}

void StringFlatCheck::InsertStringFlatCheck(IntrinsicInst *intrinsic, uint32_t stringFlatCheckArgMask)
{
    auto argsCount = intrinsic->RequireState() ? intrinsic->GetInputsCount() - 1 : intrinsic->GetInputsCount();

    for (size_t i = 0; i < argsCount; i++) {
        if ((stringFlatCheckArgMask & (1 << i)) == 0) {  // NOLINT(hicpp-signed-bitwise)
            continue;
        }

        auto *inputInst = intrinsic->GetInput(i).GetInst();
        if (Inst::GetDataFlowInput(inputInst)->GetFlag(inst_flags::FLAT_STRING)) {
            continue;
        }

        auto *flatCheck = GetStringFlatCheckUser(intrinsic, inputInst);
        if (flatCheck == nullptr) {
            flatCheck = InsertInputStringFlatCheck(intrinsic, inputInst);
            applied_ = true;
        } else {
            // StringFlatCheck was already inserted for another Intrinsic
            ASSERT(!flatCheck->IsDominate(intrinsic));
            if (!MoveThroughDominationTree(flatCheck, intrinsic)) {
                flatCheck = InsertInputStringFlatCheck(intrinsic, inputInst);
            }
        }

        ReplaceUsers(inputInst, flatCheck);
    }
}

Inst *StringFlatCheck::GetStringFlatCheckUser(IntrinsicInst *intrinsic, Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        auto *userInst = user.GetInst();
        if (!user.GetInst()->Is(Opcode::StringFlatCheck)) {
            continue;
        }
        if (!GetGraph()->IsOsrMode() || !SeparatedByOsrEntry(userInst->GetBasicBlock(), intrinsic->GetBasicBlock())) {
            return userInst;
        }
    }
    return nullptr;
}

bool StringFlatCheck::MoveThroughDominationTree(Inst *flatCheck, IntrinsicInst *intrinsic)
{
    ASSERT(flatCheck->Is(Opcode::StringFlatCheck));
    ASSERT(!flatCheck->IsDominate(intrinsic));

    auto *bb = flatCheck->GetBasicBlock();
    auto *intrinsicBasicBlock = intrinsic->GetBasicBlock();
    while (!bb->IsDominate(intrinsicBasicBlock)) {
        bb = bb->GetDominator();
        ASSERT(bb != nullptr);
        ASSERT(!GetGraph()->IsOsrMode() || !SeparatedByOsrEntry(bb, intrinsicBasicBlock));
    }
    ASSERT(!bb->IsStartBlock());

    auto *foundSaveState = bb->FindSaveState();
    if (foundSaveState == nullptr) {
        return false;
    }

    auto *flatCheckInputInst = flatCheck->GetInput(0).GetInst();
    if (foundSaveState->IsDominate(flatCheckInputInst) && !flatCheckInputInst->IsNullCheck()) {
        return false;
    }

    flatCheck->GetSaveState()->RemoveInputs();
    flatCheck->GetBasicBlock()->EraseInst(flatCheck->GetSaveState());
    flatCheck->GetBasicBlock()->EraseInst(flatCheck);

    if (foundSaveState->IsDominate(flatCheckInputInst)) {
        ASSERT(flatCheckInputInst->IsNullCheck());
        flatCheckInputInst->InsertAfter(flatCheck);
    } else {
        foundSaveState->InsertBefore(flatCheck);
    }

    auto *saveState = CopySaveState(GetGraph(), foundSaveState);
    InsertStringFlatCheckSaveState(flatCheck, saveState);

    return true;
}

Inst *StringFlatCheck::InsertInputStringFlatCheck(IntrinsicInst *intrinsic, Inst *inputInst)
{
    auto *graph = GetGraph();
    auto *flatCheck = graph->CreateInstStringFlatCheck(DataType::REFERENCE, intrinsic->GetPc());

    auto *foundSaveState = intrinsic->GetBasicBlock()->FindSaveState(intrinsic);
    ASSERT_PRINT(foundSaveState != nullptr, "Intrinsic BB should have SaveState for StringFlatCheck ");

    if (intrinsic->RequireState() && inputInst->GetSaveState() == intrinsic->GetSaveState()) {
        ASSERT(foundSaveState == intrinsic->GetSaveState());
        intrinsic->InsertBefore(flatCheck);
        auto *intrinsicSaveState = CopySaveState(GetGraph(), foundSaveState);
        intrinsic->SetSaveState(intrinsicSaveState);
        intrinsic->InsertBefore(intrinsicSaveState);
    } else {
        foundSaveState->InsertBefore(flatCheck);
    }

    flatCheck->SetInput(0, inputInst);

    auto *saveState = CopySaveState(GetGraph(), foundSaveState);
    InsertStringFlatCheckSaveState(flatCheck, saveState);

    return flatCheck;
}

void StringFlatCheck::ReplaceUsers(Inst *inputInst, Inst *flatCheck)
{
    users_.clear();

    for (auto *inst : {inputInst, Inst::GetDataFlowInput(inputInst)}) {
        for (auto &user : inst->GetUsers()) {
            auto *userInst = user.GetInst();
            if (CanUpdateInput(userInst, flatCheck)) {
                users_.push_back({user.GetIndex(), userInst});
            }
        }
    }

    for (auto &[index, userInst] : users_) {
        userInst->SetInput(index, flatCheck);
    }
}

bool StringFlatCheck::CanUpdateInput(Inst *userInst, Inst *flatCheck)
{
    if (!flatCheck->IsDominate(userInst) || userInst == flatCheck) {
        return false;
    }

    if (!GetGraph()->IsOsrMode()) {
        return true;
    }

    return !SeparatedByOsrEntry(userInst->GetBasicBlock(), flatCheck->GetBasicBlock());
}

void StringFlatCheck::InsertStringFlatCheckSaveState(Inst *flatCheck, SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    saveState->SetPc(flatCheck->GetPc());
    if (auto *callerInst = FindCallerInst(flatCheck->GetBasicBlock(), flatCheck)) {
        saveState->SetMethod(callerInst->GetCallMethod());
        saveState->SetCallerInst(callerInst);
        auto *callerSaveState = callerInst->GetSaveState();
        ASSERT(callerSaveState != nullptr);
        saveState->SetInliningDepth(callerSaveState->GetInliningDepth() + 1);
    } else {
        saveState->SetMethod(graph->GetMethod());
        saveState->SetCallerInst(nullptr);
        saveState->SetInliningDepth(0);
    }
    flatCheck->InsertBefore(saveState);
    flatCheck->SetSaveState(saveState);
}

void StringFlatCheck::FixSaveStates()
{
    SaveStateBridgesBuilder ssb;
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : bb->AllInsts()) {
            ssb.FixInstUsageInSS(GetGraph(), inst);
        }
    }
}

bool StringFlatCheck::SeparatedByOsrEntry(BasicBlock *bb1, BasicBlock *bb2)
{
    ASSERT(GetGraph()->IsOsrMode());
    if (bb1->GetLoop() != bb2->GetLoop()) {
        return true;
    }
    if (bb1 == bb2) {
        return false;
    }
    // to avoid additional analysis think they are separated by OSR entry if have inner loop
    return !bb1->GetLoop()->GetInnerLoops().empty();
}
}  // namespace ark::compiler
