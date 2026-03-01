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

#include "bytecodeopt_peepholes.h"

#include "libarkfile/bytecode_instruction-inl.h"

namespace ark::bytecodeopt {
bool BytecodeOptPeepholes::RunImpl()
{
    VisitGraph();
    return IsApplied();
}

CallInst *FindCtorCall(Inst *newObject)
{
    auto *graph = newObject->GetBasicBlock()->GetGraph();
    auto *adapter = graph->GetRuntime();
    for (auto &user : newObject->GetUsers()) {
        auto *inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::NullCheck) {
            return FindCtorCall(inst);
        }
        if (inst->GetOpcode() != Opcode::CallStatic) {
            continue;
        }
        auto call = inst->CastToCallStatic();
        auto callFirstArg = call->GetInput(0U).GetInst();
        if (callFirstArg->GetOpcode() == Opcode::NewObject) {
            auto newObjectAsArg = callFirstArg->CastToNewObject();
            if (newObjectAsArg != newObject) {
                continue;
            }
        }
        if (adapter->IsConstructor(call->GetCallMethod(), graph->GetLanguage())) {
            return call;
        }
    }

    return nullptr;
}

CallInst *CreateInitObject(compiler::GraphVisitor *v, compiler::ClassInst *load, const CallInst *callInit)
{
    auto *graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    auto *initObject = static_cast<CallInst *>(graph->CreateInst(compiler::Opcode::InitObject));
    initObject->SetType(compiler::DataType::REFERENCE);

    auto inputTypesCount = callInit->GetInputsCount();
    initObject->AllocateInputTypes(graph->GetAllocator(), inputTypesCount);
    initObject->AppendInput(load, compiler::DataType::REFERENCE);
    for (size_t i = 1; i < inputTypesCount; ++i) {
        auto inputInst = callInit->GetInput(i).GetInst();
        initObject->AppendInput(inputInst, inputInst->GetType());
    }

    initObject->SetCallMethodId(callInit->GetCallMethodId());
    initObject->SetCallMethod(static_cast<const CallInst *>(callInit)->GetCallMethod());
    return initObject;
}

void ReplaceNewObjectUsers(Inst *newObject, Inst *nullCheck, CallInst *initObject)
{
    for (auto it = newObject->GetUsers().begin(); it != newObject->GetUsers().end();
         it = newObject->GetUsers().begin()) {
        auto user = it->GetInst();
        if (user != nullCheck) {
            user->SetInput(it->GetIndex(), initObject);
        } else {
            newObject->RemoveUser(&(*it));
        }
    }

    // Update throwable instructions data
    auto graph = newObject->GetBasicBlock()->GetGraph();
    if (graph->IsInstThrowable(newObject)) {
        graph->ReplaceThrowableInst(newObject, initObject);
    }
}

static bool IsSafeInstBetweenNewAndCtor(const Inst *inst, const Inst *newObj)
{
    if (inst->IsSaveState()) {
        return true;
    }

    if (inst->GetOpcode() == Opcode::NullCheck) {
        return inst->GetDataFlowInput(0) == newObj;
    }

    return !inst->HasSideEffectsFlags();
}

/*
 * This peephole replaces a `newobj` followed by its constructor call with a single `InitObject` bytecode instruction.
 * It applies only when both instructions are in the same basic block and no unsafe instructions appear in between.
 * Safe instructions in between are allowed: SaveState, a single NullCheck on the new object,
 * and any instruction without forbidden flags (no throw, allocation, call, store, barrier, deopt, etc.).
 * The transformation rewires all uses of `newobj` (and NullCheck if present) to the new `InitObject`.
 * This reduces overhead by folding allocation and initialization into one safe instruction.
 */
void BytecodeOptPeepholes::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst != nullptr);
    CallInst *callInit = FindCtorCall(inst);
    if (callInit == nullptr || inst->GetBasicBlock() != callInit->GetBasicBlock()) {
        return;
    }

    const auto graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    const size_t newobjSize =
        BytecodeInstruction::Size(  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            BytecodeInstruction(graph->GetRuntime()->GetMethodCode(graph->GetMethod()) + inst->GetPc()).GetFormat());
    if (inst->GetBasicBlock()->IsTry() && callInit->GetPc() - inst->GetPc() > newobjSize) {
        return;
    }

    Inst *nullCheck = nullptr;
    for (auto *i = inst->GetNext(); i != callInit; i = i->GetNext()) {
        if (!IsSafeInstBetweenNewAndCtor(i, inst)) {
            return;
        }
        if (i->IsNullCheck() && nullCheck == nullptr) {
            nullCheck = i;
        }
    }

    auto load = static_cast<compiler::ClassInst *>(inst->GetInput(0U).GetInst());
    auto *initObject = CreateInitObject(v, load, callInit);
    callInit->InsertBefore(initObject);
    initObject->SetPc(callInit->GetPc());

    ReplaceNewObjectUsers(inst, nullCheck, initObject);
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    if (nullCheck != nullptr) {
        nullCheck->ReplaceUsers(initObject);
        nullCheck->ClearFlag(compiler::inst_flags::NO_DCE);
        nullCheck->RemoveInputs();
        nullCheck->GetBasicBlock()->ReplaceInst(nullCheck,
                                                static_cast<BytecodeOptPeepholes *>(v)->GetGraph()->CreateInstNOP());
    }
    ASSERT(!callInit->HasUsers());
    callInit->ClearFlag(compiler::inst_flags::NO_DCE);

    static_cast<BytecodeOptPeepholes *>(v)->SetIsApplied();
}

}  // namespace ark::bytecodeopt
