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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace ark::compiler {

bool IsStringBuilderInstance(Inst *inst);
bool IsMethodStringConcat(const Inst *inst);
bool IsMethodStringBuilderGetStringLength(Inst *inst);
bool IsMethodStringBuilderConstructorWithStringArg(const Inst *inst);
bool IsMethodStringBuilderConstructorWithCharArrayArg(const Inst *inst);
bool IsStringBuilderToString(const Inst *inst);
bool IsMethodStringBuilderDefaultConstructor(const Inst *inst);
bool IsMethodStringGetLength(const Inst *inst);
bool IsStringLength(const Inst *inst);
bool IsStringLengthAccessor(const Inst *inst);
bool IsStringLengthAccessorChain(const Inst *inst);
Inst *GetStringLengthCompressedShr(Inst *lenArrayCall);

bool IsStringBuilderCtorCall(const Inst *inst, const Inst *self = nullptr);
bool IsStringBuilderMethod(const Inst *inst, const Inst *self = nullptr);
bool IsNullCheck(const Inst *inst, const Inst *self = nullptr);

bool IsIntrinsicStringConcat(const Inst *inst);
void InsertBeforeWithSaveState(Inst *inst, Inst *before);
void InsertAfterWithSaveState(Inst *inst, Inst *after);
void InsertBeforeWithInputs(Inst *inst, Inst *before);
using FindInputPredicate = std::function<bool(Input &input)>;
bool HasInput(Inst *inst, const FindInputPredicate &predicate);
bool HasInputPhiRecursively(Inst *inst, Marker visited, const FindInputPredicate &predicate);
void ResetInputMarkersRecursively(Inst *inst, Marker visited);
using FindUserPredicate = std::function<bool(User &user)>;
bool HasUser(Inst *inst, const FindUserPredicate &predicate);
bool HasUserPhiRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate);
bool HasUserRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate);
size_t CountUsers(Inst *inst, const FindUserPredicate &predicate);
void ResetUserMarkersRecursively(Inst *inst, Marker visited);
Inst *SkipSingleUserCheckInstruction(Inst *inst);
const Inst *SkipSingleUserCheckInstruction(const Inst *inst);
bool IsUsedOutsideBasicBlock(Inst *inst, BasicBlock *bb);
SaveStateInst *FindFirstSaveState(BasicBlock *block);

template <bool ALLOW_INLINED = false>
bool IsStringBuilderAppend(const Inst *inst, RuntimeInterface *runtime = nullptr)
{
    if (runtime == nullptr) {
        runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    }
    if (inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual) {
        auto callInst = static_cast<const CallInst *>(inst);
        return (ALLOW_INLINED || !callInst->IsInlined()) &&
               runtime->IsMethodStringBuilderAppend(callInst->GetCallMethod());
    }
    if (inst->IsIntrinsic()) {
        auto intrinsic = inst->CastToIntrinsic();
        return runtime->IsIntrinsicStringBuilderAppend(intrinsic->GetIntrinsicId());
    }
    return false;
}

bool IsIntrinsicStringBuilderAppendString(Inst *inst);

using InputDesc = std::pair<Inst *, unsigned>;
void RemoveFromInstructionInputs(ArenaVector<InputDesc> &inputDescriptors, bool doMarkSaveStates = false);
bool BreakStringBuilderAppendChains(BasicBlock *block);

Inst *GetArrayLengthConstant(Inst *newArray);
bool CollectArrayElements(Inst *newArray, InstVector &arrayElements);
void CleanupStoreArrayInstructions(Inst *inst);
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H
