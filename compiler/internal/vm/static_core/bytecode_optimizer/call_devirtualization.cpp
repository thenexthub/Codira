/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "call_devirtualization.h"

namespace ark::bytecodeopt {
bool CallDevirtualization::RunImpl()
{
    VisitGraph();
    return IsApplied();
}

RuntimeInterface::ClassPtr CallDevirtualization::GetClassPtr(Inst *inst, size_t inputNum)
{
    auto objInst = inst->GetDataFlowInput(inputNum);
    if (objInst->GetOpcode() != Opcode::NewObject) {
        return nullptr;
    }

    ASSERT(objInst->GetInputsCount() > 0);
    auto instInput = objInst->GetDataFlowInput(0);
    if (instInput->IsClassInst()) {
        return static_cast<compiler::ClassInst *>(instInput)->GetClass();
    }
    if (instInput->GetOpcode() == Opcode::LoadImmediate) {
        return instInput->CastToLoadImmediate()->GetClass();
    }
    if (instInput->GetOpcode() == Opcode::LoadRuntimeClass) {
        return instInput->CastToLoadRuntimeClass()->GetClass();
    }
    if (instInput->IsPhi()) {
        auto graph = instInput->GetBasicBlock()->GetGraph();
        graph->RunPass<compiler::ObjectTypePropagation>();
        auto typeInfo = instInput->GetObjectTypeInfo();
        if (typeInfo.IsValid() && typeInfo.IsExact()) {
            return typeInfo.GetClass();
        }
        return nullptr;
    }
    UNREACHABLE();
    return nullptr;
}

bool CallDevirtualization::TryDevirtualize(CallInst *callVirtual)
{
    auto runtime = GetGraph()->GetRuntime();
    auto method = callVirtual->GetCallMethod();
    auto objKlass = GetClassPtr(callVirtual, callVirtual->GetObjectIndex());
    auto methodKlass = runtime->GetClass(method);
    if (!objKlass) {
        return false;
    }

    auto objKlassAddr = reinterpret_cast<uintptr_t>(objKlass);
    auto methodKlassAddr = reinterpret_cast<uintptr_t>(methodKlass);
    if (objKlassAddr != methodKlassAddr) {
        return false;
    }

    callVirtual->SetOpcode(Opcode::CallStatic);
    return true;
}

void CallDevirtualization::VisitCallVirtual(GraphVisitor *v, Inst *inst)
{
    if (!inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }

    auto callVirtual = inst->CastToCallVirtual();
    auto self = static_cast<CallDevirtualization *>(v);
    if (self->TryDevirtualize(callVirtual)) {
        self->SetIsApplied();
    }
}

}  // namespace ark::bytecodeopt
