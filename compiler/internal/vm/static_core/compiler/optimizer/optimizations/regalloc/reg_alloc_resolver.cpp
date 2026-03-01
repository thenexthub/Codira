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

#include "reg_alloc_resolver.h"
#include "reg_type.h"
#include "compiler/optimizer/code_generator/codegen.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
/*
 * For each instruction set destination register if it is assigned,
 * Pop inputs from stack and push result on stack if stack slot is assigned.
 */
void RegAllocResolver::Resolve()
{
    // We use RPO order because we need to calculate Caller's roots mask before its inlined callees
    // (see PropagateCallerMasks).
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInstsSafe()) {
            if (inst->IsSaveState()) {
                ResolveSaveState(inst);
                continue;
            }
            ResolveInputs(inst);
            ResolveOutput(inst);
            if (GetGraph()->IsInstThrowable(inst)) {
                AddCatchPhiMoves(inst);
            }
        }
    }
}

void RegAllocResolver::AddCatchPhiMoves(Inst *inst)
{
    auto spillFillInst = GetGraph()->CreateInstSpillFill(SpillFillType::INPUT_FILL);
    auto handlers = GetGraph()->GetThrowableInstHandlers(inst);

    for (auto catchHandler : handlers) {
        for (auto catchInst : catchHandler->AllInsts()) {
            if (!catchInst->IsCatchPhi() || catchInst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catchPhi = catchInst->CastToCatchPhi();
            const auto &throwableInsts = catchPhi->GetThrowableInsts();
            auto it = std::find(throwableInsts->begin(), throwableInsts->end(), inst);
            if (it == throwableInsts->end()) {
                continue;
            }
            int index = std::distance(throwableInsts->begin(), it);
            auto catchInput = catchPhi->GetDataFlowInput(index);
            auto inputInterval = liveness_->GetInstLifeIntervals(catchInput);
            ASSERT(inputInterval->GetSibling() == nullptr);
            auto catchPhiInterval = liveness_->GetInstLifeIntervals(catchPhi);
            if (inputInterval->GetLocation() != catchPhiInterval->GetLocation()) {
                ConnectIntervals(spillFillInst, inputInterval, catchPhiInterval);
            }
        }
    }
    if (!spillFillInst->GetSpillFills().empty()) {
        inst->InsertBefore(spillFillInst);
    }
}

void RegAllocResolver::ResolveInputs(Inst *inst)
{
    if (inst->IsPhi() || inst->IsCatchPhi() || IsPseudoUserOfMultiOutput(inst) || InstHasPseudoInputs(inst)) {
        return;
    }
    // Life-position before instruction to analyze intervals, that were splitted directly before it
    auto insLn = liveness_->GetInstLifeIntervals(inst)->GetBegin();
    auto preInsLn = insLn - 1U;

    inputLocations_.clear();
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto inputInterval = liveness_->GetInstLifeIntervals(inst->GetDataFlowInput(i));
        auto sibling = inputInterval->FindSiblingAt(insLn);
        ASSERT(sibling != nullptr);
        inputLocations_.push_back(sibling->GetLocation());
    }

    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto location = inst->GetLocation(i);
        auto inputInterval = liveness_->GetInstLifeIntervals(inst->GetDataFlowInput(i));
        if (CanReadFromAccumulator(inst, i) || inputInterval->NoDest() || location.IsInvalid()) {
            continue;
        }

        // Interval with fixed register can be splitted before `inst`: we don't need any extra moves in that case,
        // since fixed register can't be overwrite
        auto sibling = inputInterval->FindSiblingAt(preInsLn);
        ASSERT(sibling != nullptr);
        if (location.IsFixedRegister() && sibling->GetLocation() == location) {
            auto it = std::find(inputLocations_.begin(), inputLocations_.end(), location);
            // If some other instruction reside in the location then we can't reuse a split ending
            // before the inst as a corresponding location will be overridden
            if (it == inputLocations_.end() || static_cast<size_t>(it - inputLocations_.begin()) == i) {
                continue;
            }
        }

        // Otherwise use sibling covering `inst`
        if (sibling->GetEnd() == preInsLn) {
            sibling = sibling->GetSibling();
        }

        // Input's location required any register: specify the allocated one
        if (location.IsUnallocatedRegister()) {
            ASSERT(sibling->HasReg());
            inst->SetLocation(i, sibling->GetLocation());
            continue;
        }

        // Finally, if input's location is not equal to the required one, add spill-fill
        if (sibling->GetLocation() != location) {
            AddMoveToFixedLocation(inst, sibling->GetLocation(), i);
        }
    }

    if (inst->RequireTmpReg()) {
        auto interval = liveness_->GetTmpRegInterval(inst);
        ASSERT(interval != nullptr);
        ASSERT(interval->HasReg());
        auto regLocation = Location::MakeRegister(interval->GetReg(), interval->GetType());
        inst->SetTmpLocation(regLocation);
        GetGraph()->SetRegUsage(regLocation);
    }
}

void RegAllocResolver::AddMoveToFixedLocation(Inst *inst, Location inputLocation, size_t inputNum)
{
    // Create or get existing SpillFillInst
    SpillFillInst *sfInst {};
    if (inst->GetPrev() != nullptr && inst->GetPrev()->IsSpillFill()) {
        sfInst = inst->GetPrev()->CastToSpillFill();
    } else {
        sfInst = GetGraph()->CreateInstSpillFill(SpillFillType::INPUT_FILL);
        inst->InsertBefore(sfInst);
    }

    // Add move from input to fixed location
    auto type = ConvertRegType(GetGraph(), inst->GetInputType(inputNum));
    auto fixedLocation = inst->GetLocation(inputNum);
    if (fixedLocation.IsFixedRegister()) {
        GetGraph()->SetRegUsage(fixedLocation.GetValue(), type);
    }
    sfInst->AddSpillFill(inputLocation, fixedLocation, type);
}

Inst *GetFirstUserOrInst(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::ReturnInlined) {
            return user.GetInst();
        }
    }
    return inst;
}

// For implicit null check we need to find the first null check's user to
// correctly capture SaveState's input locations, because implicit null checks are fired
// when its input is accessed by its users (for example, when LoadArray instruction is loading
// value from null array reference). Some life intervals may change its location (due to spilling)
// between NullCheck and its users, so locations captured at implicit null check could be incorrect.
// While implicit NullCheck may have multiple users we can use only a user dominating all other users,
// because null check either will be fired at it, or won't be fired at all.
Inst *GetExplicitUser(Inst *inst)
{
    if (!inst->IsNullCheck() || !inst->CastToNullCheck()->IsImplicit() || inst->GetUsers().Empty()) {
        return inst;
    }
    if (inst->HasSingleUser()) {
        return inst->GetUsers().Front().GetInst();
    }

    Inst *userInst {nullptr};
    for (auto &user : inst->GetUsers()) {
        auto currInst = user.GetInst();
        if (!IsSuitableForImplicitNullCheck(currInst)) {
            continue;
        }
        if (currInst->GetInput(0) != inst) {
            continue;
        }
        if (!currInst->CanThrow()) {
            continue;
        }
        userInst = currInst;
        break;
    }
#ifndef NDEBUG
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->IsPhi()) {
            continue;
        }
        ASSERT(userInst != nullptr && userInst->IsDominate(user.GetInst()));
    }
#endif
    return userInst;
}

void RegAllocResolver::PropagateCallerMasks(SaveStateInst *saveState)
{
    saveState->CreateRootsStackMask(GetGraph()->GetAllocator());
    auto user = GetExplicitUser(GetFirstUserOrInst(saveState));
    // Get location of save state inputs at the save state user (note that at this point
    // all inputs will have the same location at all users (excluding ReturnInlined that should be skipped)).
    FillSaveStateRootsMask(saveState, user, saveState);

    for (auto callerInst = saveState->GetCallerInst(); callerInst != nullptr;) {
        auto callerSs = callerInst->GetSaveState();
        FillSaveStateRootsMask(callerSs, user, saveState);
        auto saveStateTmp = callerInst->GetSaveState();
        ASSERT(saveStateTmp != nullptr);
        callerInst = saveStateTmp->GetCallerInst();
    }
}

void RegAllocResolver::FillSaveStateRootsMask(SaveStateInst *saveState, Inst *user, SaveStateInst *targetSs)
{
    auto dstLn = liveness_->GetInstLifeIntervals(user)->GetBegin();

    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        auto inputInst = saveState->GetDataFlowInput(i);
        if (!inputInst->IsMovableObject()) {
            continue;
        }
        auto inputInterval = liveness_->GetInstLifeIntervals(inputInst);
        auto sibling = inputInterval->FindSiblingAt(dstLn);
        ASSERT(sibling != nullptr);
        bool isSplitCover;
        if (user->IsPropagateLiveness()) {
            isSplitCover = sibling->SplitCover<true>(dstLn);
        } else {
            isSplitCover = sibling->SplitCover(dstLn);
        }
        if (!isSplitCover) {
            continue;
        }
        AddLocationToRoots(sibling->GetLocation(), targetSs, GetGraph());
#ifndef NDEBUG
        for (auto &testUser : targetSs->GetUsers()) {
            if (testUser.GetInst()->GetOpcode() == Opcode::ReturnInlined ||
                testUser.GetInst()->GetId() == user->GetId()) {
                continue;
            }
            auto explicitTestUser = GetExplicitUser(testUser.GetInst());
            auto udstLn = liveness_->GetInstLifeIntervals(explicitTestUser)->GetBegin();
            ASSERT(sibling->GetLocation() == inputInterval->FindSiblingAt(udstLn)->GetLocation());
        }
#endif
    }
}

namespace {
bool HasSameLocation(LifeIntervals *interval, LifeNumber pos1, LifeNumber pos2)
{
    auto sibling1 = interval->FindSiblingAt(pos1);
    auto sibling2 = interval->FindSiblingAt(pos2);
    ASSERT(sibling1 != nullptr);
    ASSERT(sibling2 != nullptr);
    return sibling1->SplitCover(pos1) && sibling1->SplitCover(pos2) &&
           sibling1->GetLocation() == sibling2->GetLocation();
}

bool SaveStateCopyRequired(Inst *inst, User *currUser, User *prevUser, const LivenessAnalyzer *la)
{
    ASSERT(inst->IsSaveState());
    auto currUserLn = la->GetInstLifeIntervals(GetExplicitUser(currUser->GetInst()))->GetBegin();
    auto prevUserLn = la->GetInstLifeIntervals(GetExplicitUser(prevUser->GetInst()))->GetBegin();
    bool needCopy = false;
    // If current save state is part of inlined method then we have to check location for all
    // parent save states.
    for (auto ss = static_cast<SaveStateInst *>(inst); ss != nullptr && !needCopy;) {
        for (size_t inputIdx = 0; inputIdx < ss->GetInputsCount() && !needCopy; inputIdx++) {
            auto inputInterval = la->GetInstLifeIntervals(ss->GetDataFlowInput(inputIdx));
            needCopy = !HasSameLocation(inputInterval, currUserLn, prevUserLn);
        }
        auto caller = ss->GetCallerInst();
        if (caller == nullptr) {
            ss = nullptr;
        } else {
            ss = caller->GetSaveState();
        }
    }
    return needCopy;
}
}  // namespace

void RegAllocResolver::ResolveSaveState(Inst *inst)
{
    if (GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }
    ASSERT(inst->IsSaveState());

    bool handledAllUsers = inst->HasSingleUser() || !inst->HasUsers();
    while (!handledAllUsers) {
        size_t copyUsers = 0;
        auto userIt = inst->GetUsers().begin();
        User *prevUser = &*userIt;
        ++userIt;
        bool needCopy = false;

        // Find first user having different location for some of the save state inputs and use SaveState's
        // copy for all preceding users.
        for (; userIt != inst->GetUsers().end() && !needCopy; ++userIt, copyUsers++) {
            auto &currUser = *userIt;
            // ReturnInline's SaveState is required only for SaveState's inputs life range propagation,
            // so it does not actually matter which interval will be actually used.
            if (prevUser->GetInst()->GetOpcode() == Opcode::ReturnInlined) {
                prevUser = &*userIt;
                continue;
            }
            if (currUser.GetInst()->GetOpcode() == Opcode::ReturnInlined) {
                continue;
            }
            needCopy = SaveStateCopyRequired(inst, &currUser, prevUser, liveness_);
            prevUser = &*userIt;
        }
        if (needCopy) {
            auto copy = CopySaveState(GetGraph(), static_cast<SaveStateInst *>(inst));
            // Replace original SaveState with the copy for first N users (N = `copy_users` ).
            while (copyUsers > 0) {
                auto userInst = inst->GetUsers().Front().GetInst();
                userInst->ReplaceInput(inst, copy);
                copyUsers--;
            }
            inst->GetBasicBlock()->InsertAfter(copy, inst);
            PropagateCallerMasks(copy);
            handledAllUsers = inst->HasSingleUser();
        } else {
            handledAllUsers = !(userIt != inst->GetUsers().end());
        }
    }
    // At this point inst either has single user or all its inputs have the same location at all users.
    PropagateCallerMasks(static_cast<SaveStateInst *>(inst));
}

/*
 * Pop output on stack from reserved register
 */
void RegAllocResolver::ResolveOutput(Inst *inst)
{
    // Don't process LiveOut, since it is instruction with pseudo destination
    if (inst->GetOpcode() == Opcode::LiveOut) {
        return;
    }
    // Multi-output instructions' dst registers will be filled after procecssing theirs pseudo users
    if (inst->GetLinearNumber() == INVALID_LINEAR_NUM || inst->GetDstCount() > 1) {
        return;
    }

    if (CanStoreToAccumulator(inst)) {
        return;
    }

    auto instInterval = liveness_->GetInstLifeIntervals(inst);
    if (instInterval->NoDest()) {
        inst->SetDstReg(GetInvalidReg());
        return;
    }

    if (inst->GetOpcode() == Opcode::Parameter) {
        inst->CastToParameter()->GetLocationData().SetDst(instInterval->GetLocation());
    }
    // Process multi-output inst
    size_t dstMum = inst->GetSrcRegIndex();
    if (IsPseudoUserOfMultiOutput(inst)) {
        inst = inst->GetInput(0).GetInst();
    }
    // Wrtie dst
    auto regType = instInterval->GetType();
    if (instInterval->HasReg()) {
        auto reg = instInterval->GetReg();
        inst->SetDstReg(dstMum, reg);
        GetGraph()->SetRegUsage(reg, regType);
    } else {
        ASSERT(inst->IsConst() || inst->IsPhi() || inst->IsParameter());
    }
}

bool RegAllocResolver::ResolveCatchPhis()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (!block->IsCatchBegin()) {
            continue;
        }
        for (auto inst : block->AllInstsSafe()) {
            if (!inst->IsCatchPhi()) {
                break;
            }
            if (inst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            // This is the case when all throwable instructions were removed from the try-block,
            // so that catch-handler is unreachable
            if (inst->GetInputs().Empty()) {
                return false;
            }
            auto newCatchPhi = SqueezeCatchPhiInputs(inst->CastToCatchPhi());
            if (newCatchPhi != nullptr) {
                inst->ReplaceUsers(newCatchPhi);
                block->RemoveInst(inst);
            }
        }
    }
    return true;
}

/**
 * Try to remove catch phi's inputs:
 * If the input's corresponding throwable instruction dominates other throwable inst, we can remove other equal catch
 * phi's input
 *
 * CatchPhi(v1, v1, v1, v2, v2, v2) -> CatchPhi(v1, v2)
 *
 * Return nullptr if inputs count was not reduced.
 */
Inst *RegAllocResolver::SqueezeCatchPhiInputs(CatchPhiInst *catchPhi)
{
    bool inputsAreIdentical = true;
    auto firstInput = catchPhi->GetInput(0).GetInst();
    for (size_t i = 1; i < catchPhi->GetInputsCount(); ++i) {
        if (catchPhi->GetInput(i).GetInst() != firstInput) {
            inputsAreIdentical = false;
            break;
        }
    }
    if (inputsAreIdentical) {
        return firstInput;
    }

    // Create a new one and fill it with the necessary inputs
    auto newCatchPhi = GetGraph()->CreateInstCatchPhi(catchPhi->GetType(), catchPhi->GetPc());
    ASSERT(catchPhi->GetBasicBlock()->GetFirstInst()->IsCatchPhi());
    catchPhi->GetBasicBlock()->PrependInst(newCatchPhi);
    for (size_t i = 0; i < catchPhi->GetInputsCount(); i++) {
        auto inputInst = catchPhi->GetInput(i).GetInst();
        auto currentThrowableInst = catchPhi->GetThrowableInst(i);
        ASSERT(GetGraph()->IsInstThrowable(currentThrowableInst));
        bool skip = false;
        for (size_t j = 0; j < newCatchPhi->GetInputsCount(); j++) {
            auto savedInst = newCatchPhi->GetInput(j).GetInst();
            if (savedInst != inputInst) {
                continue;
            }
            auto savedThrowableInst = newCatchPhi->GetThrowableInst(j);
            if (savedThrowableInst->IsDominate(currentThrowableInst)) {
                skip = true;
            }
            if (currentThrowableInst->IsDominate(savedThrowableInst)) {
                newCatchPhi->ReplaceThrowableInst(savedThrowableInst, currentThrowableInst);
                skip = true;
            }
            if (skip) {
                break;
            }
        }
        if (!skip) {
            newCatchPhi->AppendInput(inputInst);
            newCatchPhi->AppendThrowableInst(currentThrowableInst);
        }
    }
    if (newCatchPhi->GetInputsCount() == catchPhi->GetInputsCount()) {
        newCatchPhi->GetBasicBlock()->RemoveInst(newCatchPhi);
        return nullptr;
    }
    return newCatchPhi;
}

}  // namespace ark::compiler
