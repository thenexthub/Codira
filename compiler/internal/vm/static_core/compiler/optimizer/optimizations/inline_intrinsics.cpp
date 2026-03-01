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
#include "compiler_options.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/types_analysis.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/inline_intrinsics.h"
#ifdef PANDA_WITH_IRTOC
#ifndef __clang_analyzer__
#include "irtoc_ir_inline.h"
#endif
#endif

namespace ark::compiler {
InlineIntrinsics::InlineIntrinsics(Graph *graph)
    : Optimization(graph),
      types_ {graph->GetLocalAllocator()->Adapter()},
      savedInputs_ {graph->GetLocalAllocator()->Adapter()},
      namedAccessProfile_ {graph->GetLocalAllocator()->Adapter()}
{
}

void InlineIntrinsics::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool InlineIntrinsics::RunImpl()
{
    bool isApplied = false;
    GetGraph()->RunPass<TypesAnalysis>();
    bool clearBbs = false;
    // We can't replace intrinsics on Deoptimize in OSR mode, because we can get incorrect value
    bool isReplaceOnDeopt = !GetGraph()->IsAotMode() && !GetGraph()->IsOsrMode();
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr || bb->IsEmpty()) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }
            if (inst->GetOpcode() == Opcode::CallDynamic) {
                isApplied |= TryInline(inst->CastToCallDynamic());
                continue;
            }
            if (inst->GetOpcode() != Opcode::Intrinsic) {
                continue;
            }
            auto intrinsicsInst = inst->CastToIntrinsic();
            if (g_options.IsCompilerInlineFullIntrinsics() && !intrinsicsInst->CanBeInlined()) {
                // skip intrinsics built inside inlined irtoc handler
                continue;
            }
            if (TryInline(intrinsicsInst)) {
                isApplied = true;
                continue;
            }
            if (isReplaceOnDeopt && intrinsicsInst->IsReplaceOnDeoptimize()) {
                inst->GetBasicBlock()->ReplaceInstByDeoptimize(inst);
                clearBbs = true;
                break;
            }
        }
    }
    if (clearBbs) {
        GetGraph()->RemoveUnreachableBlocks();
        if (GetGraph()->IsOsrMode()) {
            CleanupGraphSaveStateOSR(GetGraph());
        }
        isApplied = true;
    }
    return isApplied;
}

AnyBaseType InlineIntrinsics::GetAssumedAnyType(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Phi:
            return inst->CastToPhi()->GetAssumedAnyType();
        case Opcode::CastValueToAnyType:
            return inst->CastToCastValueToAnyType()->GetAnyType();
        case Opcode::AnyTypeCheck: {
            auto type = inst->CastToAnyTypeCheck()->GetAnyType();
            if (type == AnyBaseType::UNDEFINED_TYPE) {
                return GetAssumedAnyType(inst->GetInput(0).GetInst());
            }
            return type;
        }
        case Opcode::Constant: {
            if (inst->GetType() != DataType::ANY) {
                // NOTE(dkofanov): Probably, we should resolve const's type based on `inst->GetType()`:
                return AnyBaseType::UNDEFINED_TYPE;
            }
            coretypes::TaggedValue anyConst(inst->CastToConstant()->GetRawValue());
            auto type = GetGraph()->GetRuntime()->ResolveSpecialAnyTypeByConstant(anyConst);
            ASSERT(type != AnyBaseType::UNDEFINED_TYPE);
            return type;
        }
        case Opcode::LoadFromConstantPool: {
            if (inst->CastToLoadFromConstantPool()->IsString()) {
                auto language = GetGraph()->GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod());
                return GetAnyStringType(language);
            }
            return AnyBaseType::UNDEFINED_TYPE;
        }
        default:
            return AnyBaseType::UNDEFINED_TYPE;
    }
}

bool InlineIntrinsics::DoInline(IntrinsicInst *intrinsic)
{
    // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
    switch (intrinsic->GetIntrinsicId()) {
#ifndef __clang_analyzer__
#include "intrinsics_inline.inl"
#endif
        default: {
            return false;
        }
    }
}

bool InlineIntrinsics::TryInline(CallInst *callInst)
{
    if ((callInst->GetCallMethod() == nullptr)) {
        return false;
    }
    // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
    switch (GetGraph()->GetRuntime()->GetMethodSourceLanguage(callInst->GetCallMethod())) {
#include "intrinsics_inline_native_method.inl"
        default: {
            return false;
        }
    }
}

bool InlineIntrinsics::TryInline(IntrinsicInst *intrinsic)
{
    if (!IsIntrinsicInlinedByInputTypes(intrinsic->GetIntrinsicId())) {
        return DoInline(intrinsic);
    }
    types_.clear();
    savedInputs_.clear();
    AnyBaseType type = AnyBaseType::UNDEFINED_TYPE;
    for (auto &input : intrinsic->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->IsSaveState()) {
            continue;
        }
        auto inputType = GetAssumedAnyType(inputInst);
        if (inputType != AnyBaseType::UNDEFINED_TYPE) {
            type = inputType;
        } else if (inputInst->GetOpcode() == Opcode::AnyTypeCheck &&
                   inputInst->CastToAnyTypeCheck()->IsTypeWasProfiled()) {
            // Type is mixed and compiler cannot make optimization for that case.
            // Any deduced type will also cause deoptimization. So avoid intrinsic inline here.
            // Revise it in the future optimizations.
            return false;
        }
        types_.emplace_back(inputType);
        savedInputs_.emplace_back(inputInst);
    }
    // last input is SaveState
    ASSERT(types_.size() + 1 == intrinsic->GetInputsCount());

    if (!GetGraph()->GetRuntime()->IsDestroyed(GetGraph()->GetMethod()) && type != AnyBaseType::UNDEFINED_TYPE &&
        AnyBaseTypeToDataType(type) != DataType::ANY) {
        // Set known type to undefined input types.
        // Do not set type based on special input types like Undefined or Null
        for (auto &currType : types_) {
            if (currType == AnyBaseType::UNDEFINED_TYPE) {
                currType = type;
            }
        }
    }
    if (DoInline(intrinsic)) {
        for (size_t i = 0; i < savedInputs_.size(); i++) {
            ASSERT(!savedInputs_[i]->IsSaveState());
            if (savedInputs_[i]->GetOpcode() == Opcode::AnyTypeCheck) {
                savedInputs_[i]->CastToAnyTypeCheck()->SetAnyType(types_[i]);
            }
        }
        return true;
    }
    return false;
}

}  // namespace ark::compiler
