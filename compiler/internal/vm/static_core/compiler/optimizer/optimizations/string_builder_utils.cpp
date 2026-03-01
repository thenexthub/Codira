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

#include "string_builder_utils.h"

namespace ark::compiler {

bool IsStringBuilderInstance(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::NewObject) {
        return false;
    }

    auto klass = GetObjectClass(inst->CastToNewObject());
    if (klass == nullptr) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsClassStringBuilder(klass);
}

bool IsMethodStringBuilderGetStringLength(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderGetStringLength(call->GetCallMethod());
}

bool IsMethodStringConcat(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic && inst->GetOpcode() != Opcode::CallVirtual) {
        return false;
    }

    auto call = static_cast<const CallInst *>(inst);
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringConcat(call->GetCallMethod());
}

bool IsMethodStringBuilderConstructorWithStringArg(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderConstructorWithStringArg(call->GetCallMethod());
}

bool IsMethodStringBuilderConstructorWithCharArrayArg(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderConstructorWithCharArrayArg(call->GetCallMethod());
}

bool IsStringBuilderToString(const Inst *inst)
{
    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    if (inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual) {
        auto callInst = static_cast<const CallInst *>(inst);
        return !callInst->IsInlined() && runtime->IsMethodStringBuilderToString(callInst->GetCallMethod());
    }
    if (inst->IsIntrinsic()) {
        auto intrinsic = inst->CastToIntrinsic();
        return runtime->IsIntrinsicStringBuilderToString(intrinsic->GetIntrinsicId());
    }
    return false;
}

bool IsMethodStringBuilderDefaultConstructor(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderDefaultConstructor(call->GetCallMethod());
}

static bool IsMethodOrIntrinsicCall(const Inst *inst, const Inst *self)
{
    const Inst *actualSelf = nullptr;
    if (inst->GetOpcode() == Opcode::CallStatic) {
        auto *call = inst->CastToCallStatic();
        if (call->IsInlined()) {
            return false;
        }
        actualSelf = call->GetObjectInst();
    } else if (inst->IsIntrinsic()) {
        auto *intrinsic = inst->CastToIntrinsic();
        actualSelf = intrinsic->GetInput(0).GetInst();
    } else {
        return false;
    }

    if (self == nullptr) {
        return true;
    }

    // Skip NullChecks
    while (actualSelf->IsNullCheck()) {
        actualSelf = actualSelf->CastToNullCheck()->GetInput(0).GetInst();
    }
    return actualSelf == self;
}

bool IsStringBuilderCtorCall(const Inst *inst, const Inst *self)
{
    if (!IsMethodOrIntrinsicCall(inst, self)) {
        return false;
    }

    return IsMethodStringBuilderDefaultConstructor(inst) || IsMethodStringBuilderConstructorWithStringArg(inst) ||
           IsMethodStringBuilderConstructorWithCharArrayArg(inst);
}

bool IsStringBuilderMethod(const Inst *inst, const Inst *self)
{
    if (!IsMethodOrIntrinsicCall(inst, self)) {
        return false;
    }

    return IsStringBuilderCtorCall(inst, self) || IsStringBuilderAppend(inst) || IsStringBuilderToString(inst);
}

bool IsNullCheck(const Inst *inst, const Inst *self)
{
    return inst->IsNullCheck() && (self == nullptr || inst->CastToNullCheck()->GetInput(0) == self);
}

bool IsIntrinsicStringConcat(const Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsIntrinsicStringConcat(inst->CastToIntrinsic()->GetIntrinsicId());
}

void InsertBeforeWithSaveState(Inst *inst, Inst *before)
{
    ASSERT(before != nullptr);
    if (inst->RequireState()) {
        before->InsertBefore(inst->GetSaveState());
    }
    before->InsertBefore(inst);
}

void InsertAfterWithSaveState(Inst *inst, Inst *after)
{
    after->InsertAfter(inst);
    if (inst->RequireState()) {
        after->InsertAfter(inst->GetSaveState());
    }
}

void InsertBeforeWithInputs(Inst *inst, Inst *before)
{
    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->GetBasicBlock() == nullptr) {
            InsertBeforeWithInputs(inputInst, before);
        }
    }

    if (inst->GetBasicBlock() == nullptr) {
        ASSERT(before != nullptr);
        before->InsertBefore(inst);
    }
}

bool HasInput(Inst *inst, const FindInputPredicate &predicate)
{
    // Check if any instruction input satisfy predicate

    auto found = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(), predicate);
    return found != inst->GetInputs().end();
}

bool HasInputPhiRecursively(Inst *inst, Marker visited, const FindInputPredicate &predicate)
{
    // Check if any instruction input satisfy predicate
    // All Phi-instruction inputs are checked recursively

    if (HasInput(inst, predicate)) {
        return true;
    }

    inst->SetMarker(visited);

    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (!inputInst->IsPhi()) {
            continue;
        }
        if (inputInst->IsMarked(visited)) {
            continue;
        }
        if (HasInputPhiRecursively(inputInst, visited, predicate)) {
            return true;
        }
    }

    return false;
}

void ResetInputMarkersRecursively(Inst *inst, Marker visited)
{
    // Reset marker for an instruction and all it's inputs recursively

    if (inst->IsMarked(visited)) {
        inst->ResetMarker(visited);

        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (inputInst->IsMarked(visited)) {
                ResetInputMarkersRecursively(inputInst, visited);
            }
        }
    }
}

bool HasUser(Inst *inst, const FindUserPredicate &predicate)
{
    // Check if instruction is used in a context defined by predicate

    auto found = std::find_if(inst->GetUsers().begin(), inst->GetUsers().end(), predicate);
    return found != inst->GetUsers().end();
}

bool HasUserPhiRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate)
{
    // Check if instruction is used in a context defined by predicate
    // All Phi-instruction users are checked recursively

    if (HasUser(inst, predicate)) {
        return true;
    }

    inst->SetMarker(visited);

    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!userInst->IsPhi()) {
            continue;
        }
        if (userInst->IsMarked(visited)) {
            continue;
        }
        if (HasUserPhiRecursively(userInst, visited, predicate)) {
            return true;
        }
    }

    return false;
}

bool HasUserRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate)
{
    // Check if instruction is used in a context defined by predicate
    // All Check-instruction users are checked recursively

    if (HasUser(inst, predicate)) {
        return true;
    }

    inst->SetMarker(visited);

    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!userInst->IsCheck()) {
            continue;
        }
        if (userInst->IsMarked(visited)) {
            continue;
        }
        if (HasUserRecursively(userInst, visited, predicate)) {
            return true;
        }
    }

    return false;
}

size_t CountUsers(Inst *inst, const FindUserPredicate &predicate)
{
    size_t count = 0;
    for (auto &user : inst->GetUsers()) {
        if (predicate(user)) {
            ++count;
        }

        auto userInst = user.GetInst();
        if (userInst->IsCheck()) {
            count += CountUsers(userInst, predicate);
        }
    }

    return count;
}

void ResetUserMarkersRecursively(Inst *inst, Marker visited)
{
    // Reset marker for an instruction and all it's users recursively

    if (inst->IsMarked(visited)) {
        inst->ResetMarker(visited);

        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsMarked(visited)) {
                ResetUserMarkersRecursively(userInst, visited);
            }
        }
    }
}

Inst *SkipSingleUserCheckInstruction(Inst *inst)
{
    if (inst->IsCheck() && inst->HasSingleUser()) {
        inst = inst->GetUsers().Front().GetInst();
    }
    return inst;
}

const Inst *SkipSingleUserCheckInstruction(const Inst *inst)
{
    if (inst->IsCheck() && inst->HasSingleUser()) {
        inst = inst->GetUsers().Front().GetInst();
    }
    return inst;
}

bool IsMethodStringGetLength(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringGetLength(call->GetCallMethod());
}

bool IsStringLength(const Inst *inst)
{
    return IsStringLengthAccessor(inst) || IsMethodStringGetLength(inst);
}

bool IsStringLengthAccessor(const Inst *inst)
{
    return inst->GetOpcode() == compiler::Opcode::LenArray;
}

bool IsStringLengthAccessorChain(const Inst *inst)
{
    inst = SkipSingleUserCheckInstruction(inst);
    return IsStringLength(inst);
}

Inst *GetStringLengthCompressedShr(Inst *lenArrayCall)
{
    // Note: ark::coretypes::String::GetLength at
    // static_core/runtime/include/coretypes/string.h
    // Getting string length is followed by Shr caused by optional string compression

    if (!lenArrayCall->HasSingleUser()) {
        return nullptr;
    }

    auto *maybeShift = lenArrayCall->GetUsers().Front().GetInst();
    return (maybeShift->GetOpcode() == Opcode::Shr) ? maybeShift : nullptr;
}

bool IsIntrinsicStringBuilderAppendString(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsIntrinsicStringBuilderAppendString(inst->CastToIntrinsic()->GetIntrinsicId());
}

bool IsUsedOutsideBasicBlock(Inst *inst, BasicBlock *bb)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsCheck()) {
            if (!userInst->HasUsers()) {
                continue;
            }
            if (!userInst->HasSingleUser()) {
                // In case of multi user check-instruction we assume it is used outside current basic block without
                // actually testing it.
                return true;
            }
            // In case of single user check-instruction we test its the only user.
            userInst = userInst->GetUsers().Front().GetInst();
        }
        if (userInst->GetBasicBlock() != bb) {
            return true;
        }
    }
    return false;
}

SaveStateInst *FindFirstSaveState(BasicBlock *block)
{
    if (block->IsEmpty()) {
        return nullptr;
    }

    for (auto inst : block->Insts()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            return inst->CastToSaveState();
        }
    }

    return nullptr;
}

void RemoveFromInstructionInputs(ArenaVector<InputDesc> &inputDescriptors, bool doMarkSaveStates)
{
    // Inputs must be walked in reverse order for removal
    std::sort(inputDescriptors.begin(), inputDescriptors.end(),
              [](auto inputDescX, auto inputDescY) { return inputDescX.second > inputDescY.second; });

    for (auto inputDesc : inputDescriptors) {
        auto inst = inputDesc.first;
        auto index = inputDesc.second;
        inst->RemoveInput(index);
        if (inst->IsSaveState() && doMarkSaveStates) {
            auto *saveState = static_cast<SaveStateInst *>(inst);
            saveState->SetInputsWereDeleted();
#ifndef NDEBUG
            if (!saveState->CanRemoveInputs()) {
                saveState->SetInputsWereDeletedSafely();  // assuming this is safe
            }
#endif
        }
    }
}

bool BreakStringBuilderAppendChains(BasicBlock *block)
{
    // StringBuilder append-call returns 'this' (instance)
    // Replace all users of append-call with instance itself to support chain calls
    // like: sb.append(s0).append(s1)...
    bool isApplied = false;
    for (auto inst : block->Insts()) {
        if (!IsStringBuilderAppend(inst) && !IsStringBuilderToString(inst)) {
            continue;
        }

        auto instance = inst->GetDataFlowInput(0);
        for (auto &user : instance->GetUsers()) {
            auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
            if (IsStringBuilderAppend(userInst)) {
                userInst->ReplaceUsers(instance);
                isApplied = true;
            }
        }
    }
    return isApplied;
}

Inst *GetStoreArrayIndexConstant(Inst *storeArray)
{
    ASSERT(storeArray->GetOpcode() == Opcode::StoreArray);
    ASSERT(storeArray->GetInputsCount() > 1);

    auto inputInst1 = storeArray->GetDataFlowInput(1U);
    if (inputInst1->IsConst()) {
        return inputInst1;
    }

    return nullptr;
}

bool FillArrayElement(Inst *inst, InstVector &arrayElements)
{
    if (inst->GetOpcode() == Opcode::StoreArray) {
        auto indexInst = GetStoreArrayIndexConstant(inst);
        if (indexInst == nullptr) {
            return false;
        }

        ASSERT(indexInst->IsConst());
        auto indexValue = indexInst->CastToConstant()->GetIntValue();
        if (arrayElements[indexValue] != nullptr) {
            return false;
        }

        auto element = inst->GetDataFlowInput(2U);
        arrayElements[indexValue] = element;
    }
    return true;
}

bool FillArrayElements(Inst *inst, InstVector &arrayElements)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!FillArrayElement(userInst, arrayElements)) {
            return false;
        }
        if (userInst->GetOpcode() == Opcode::NullCheck) {
            if (!FillArrayElements(userInst, arrayElements)) {
                return false;
            }
        }
    }
    return true;
}

Inst *GetArrayLengthConstant(Inst *newArray)
{
    if (newArray->GetOpcode() != Opcode::NewArray) {
        return nullptr;
    }
    ASSERT(newArray->GetInputsCount() > 1);

    auto inputInst1 = newArray->GetDataFlowInput(1U);
    if (inputInst1->IsConst()) {
        return inputInst1;
    }

    return nullptr;
}

bool CollectArrayElements(Inst *newArray, InstVector &arrayElements)
{
    /*
        Collect instructions stored to a given array

        This functions used to find all the arguments of the calls like:
            str.concat(a, b, c)
        IR builder generates the following IR for it:

        bb_start:
            v0  Constant 0x0
            v1  Constant 0x1
            v2  Constant 0x2
            v3  Constant 0x3
        bb1:
            v9  NewArray class, v3, save_state
            v10 StoreArray v9, v0, a
            v11 StoreArray v9, v1, b
            v12 StoreArray v9, v2, c
            v20 CallStatic String::concat str, v9, save_state

        Conditions:
            - array size is constant (3 in the sample code above)
            - every StoreArray instruction stores value by constant index (0, 1 and 2 in the sample code above)
            - every element stored only once
            - array filled completely

        If any of the above is false, this functions returns false and clears array.
        If all the above conditions true, this function returns true and fills array.
    */

    ASSERT(newArray->GetOpcode() == Opcode::NewArray);
    arrayElements.clear();

    auto lengthInst = GetArrayLengthConstant(newArray);
    if (lengthInst == nullptr) {
        return false;
    }
    ASSERT(lengthInst->IsConst());

    auto length = lengthInst->CastToConstant()->GetIntValue();
    arrayElements.resize(length);

    if (!FillArrayElements(newArray, arrayElements)) {
        arrayElements.clear();
        return false;
    }

    // Check if array is filled completely
    auto foundNull =
        std::find_if(arrayElements.begin(), arrayElements.end(), [](auto &element) { return element == nullptr; });
    if (foundNull != arrayElements.end()) {
        arrayElements.clear();
        return false;
    }

    return true;
}

void CleanupStoreArrayInstructions(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetOpcode() == Opcode::StoreArray) {
            userInst->ClearFlag(inst_flags::NO_DCE);
        }
        if (userInst->IsCheck()) {
            userInst->ClearFlag(inst_flags::NO_DCE);
            CleanupStoreArrayInstructions(userInst);
        }
    }
}

}  // namespace ark::compiler
