/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/method_properties.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {

MethodProperties::MethodProperties(const Graph *graph)
{
    for (auto bb : graph->GetBlocksRPO()) {
        // Calls may be in the middle of method
        for (auto inst : bb->Insts()) {
            if (inst->IsInitObject()) {
                ASSERT(g_options.IsCompilerSupportInitObjectInst());
            }
            if (inst->GetFlag(inst_flags::CAN_DEOPTIMIZE)) {
                SetHasDeopt(true);
            }
            if (inst->IsReturn()) {
                lastReturn_ = inst;
            }
            if (!GetHasParamsOnStack() && inst->GetOpcode() == Opcode::Parameter) {
                CheckAndSetStackParams(inst);
            }
            if (inst->GetOpcode() == Opcode::SafePoint) {
                SetHasSafepoints(true);
            }
            if (inst->IsCall() || inst->IsIntrinsic()) {
                SetHasCalls(true);
            }
            if (Codegen::InstEncodedWithLibCall(inst, graph->GetArch())) {
                SetHasLibCalls(true);
            }
            if (inst->IsRuntimeCall()) {
                SetHasRuntimeCalls(true);
            }
            if (inst->RequireState()) {
                SetHasRequireState(true);
            }
            if (inst->CanThrow()) {
                SetCanThrow(true);
            }
        }
    }

    /**
     * "Compact" prologue/epilogue means that unused callee-saved registers
     * are not saved in the prologue and restored in the epilogue.
     *
     * 1. We do support compact prologue/epilogue only for AARCH64.
     *    The reasons are as follows:
     *      - for X86_64 we're reserving almost all callee-saved registers for temporaries
     *      - for AARCH32 we're treating all callee-saved registers as "used".
     *    Thus there is no sense in supporting compact prologue for these targets.
     *
     * 2. We don't support compact prologue/epilogue for OSR to simplify OSR entry bridge.
     */
    SetCompactPrologueAllowed(graph->GetArch() == Arch::AARCH64 && !graph->IsOsrMode() &&
                              g_options.IsCompilerCompactPrologue());

    SetRequireFrameSetup(!IsLeaf() || graph->IsOsrMode());
}

void MethodProperties::CheckAndSetStackParams(Inst *inst)
{
    auto sf = static_cast<const ParameterInst *>(inst)->GetLocationData();
    if (sf.DstValue() != INVALID_REG && sf.SrcType() == LocationType::STACK_PARAMETER) {
        SetHasParamsOnStack(true);
    }
}
}  // namespace ark::compiler
