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

#include "locations_builder.h"
#include "compiler/optimizer/code_generator/callconv.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/locations.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOCATIONS_BUILDER(type) \
    template <Arch arch>        \
    type LocationsBuilder<arch>

namespace ark::compiler {
bool RunLocationsBuilder(Graph *graph)
{
    if (graph->GetCallingConvention() == nullptr) {
        return true;
    }
    auto pinfo = graph->GetCallingConvention()->GetParameterInfo(0);
    switch (graph->GetArch()) {
        case Arch::X86_64: {
            return graph->RunPass<LocationsBuilder<Arch::X86_64>>(pinfo);
        }
        case Arch::AARCH64: {
            return graph->RunPass<LocationsBuilder<Arch::AARCH64>>(pinfo);
        }
        case Arch::AARCH32: {
            return graph->RunPass<LocationsBuilder<Arch::AARCH32>>(pinfo);
        }
        default:
            break;
    }
    LOG(ERROR, COMPILER) << "Unknown target architecture";
    return false;
}

LOCATIONS_BUILDER()::LocationsBuilder(Graph *graph, ParameterInfo *pinfo) : Optimization(graph), paramsInfo_(pinfo)
{
    if (graph->SupportManagedCode()) {
        /* We skip first parameter location in managed mode, because calling convention for Panda methods requires
           pointer to the called method as a first parameter. So we reserve one native parameter for it. */
        GetParameterInfo()->GetNextLocation(DataType::POINTER);
    }
}

LOCATIONS_BUILDER(const ArenaVector<BasicBlock *> &)::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

LOCATIONS_BUILDER(bool)::RunImpl()
{
    VisitGraph();
    return true;
}

LOCATIONS_BUILDER(void)::ProcessManagedCall(Inst *inst, ParameterInfo *pinfo)
{
    ArenaAllocator *allocator = GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    if (pinfo == nullptr) {
        pinfo = GetResetParameterInfo();
        if (inst->GetOpcode() != Opcode::CallResolvedVirtual && inst->GetOpcode() != Opcode::CallResolvedStatic) {
            pinfo->GetNextLocation(GetWordType());
        }
    }

    size_t inputsCount = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t stackArgs = 0;
    for (size_t i = 0; i < inputsCount; i++) {
        ASSERT(inst->GetInputType(i) != DataType::NO_TYPE);
        if (i == 0U && inst->GetOpcode() == Opcode::CallNative) {
            // NOTE: this is a hack for workarounding regalloc bug (i.e. this input can get some parameter register
            // location by mistake)!
            ASSERT(locations != nullptr);
            locations->SetLocation(0U, Location::MakeRegister(GetFirstCalleeReg(GetGraph()->GetArch(), false)));
            continue;
        }
        auto param = pinfo->GetNextLocation(inst->GetInputType(i));
        if (i == 0 && inst->IsIntrinsic() && inst->CastToIntrinsic()->HasIdInput()) {
            param = Location::MakeRegister(GetTarget().GetParamRegId(0));  // place id input before imms
        }
        locations->SetLocation(i, param);
        if (param.IsStackArgument()) {
            stackArgs++;
        }
    }
    if (inst->IsIntrinsic() && stackArgs > 0) {
        inst->CastToIntrinsic()->SetArgumentsOnStack();
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }

    GetGraph()->UpdateStackSlotsCount(stackArgs);
}

LOCATIONS_BUILDER(void)::ProcessManagedCallStackRange(Inst *inst, size_t rangeStart, ParameterInfo *pinfo)
{
    ArenaAllocator *allocator = GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    /* Reserve first parameter for the callee method. It will be set by codegen. */
    if (pinfo == nullptr) {
        pinfo = GetResetParameterInfo();
        pinfo->GetNextLocation(GetWordType());
    }

    size_t inputsCount = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t stackArgs = 0;
    ASSERT(inputsCount >= rangeStart);
    for (size_t i = 0; i < rangeStart; i++) {
        ASSERT(inst->GetInputType(i) != DataType::NO_TYPE);
        auto param = pinfo->GetNextLocation(inst->GetInputType(i));
        ASSERT(locations != nullptr);
        locations->SetLocation(i, param);
        if (param.IsStackArgument()) {
            stackArgs++;
        }
    }
    if (inst->IsIntrinsic() && stackArgs > 0) {
        inst->CastToIntrinsic()->SetArgumentsOnStack();
    }
    for (size_t i = rangeStart; i < inputsCount; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(stackArgs++));
    }
    locations->SetDstLocation(GetLocationForReturn(inst));
    GetGraph()->UpdateStackSlotsCount(stackArgs);
}

LOCATIONS_BUILDER(void)::VisitResolveStatic([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToResolveStatic()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallResolvedStatic()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitResolveVirtual([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToResolveVirtual()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallResolvedVirtual()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallStatic()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallVirtual(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallVirtual()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallDynamic(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    if (inst->CastToCallDynamic()->IsInlined()) {
        ASSERT(inst->GetInputType(CallConvDynInfo::REG_METHOD) == DataType::ANY);
        auto param = Location::MakeRegister(GetTarget().GetParamRegId(CallConvDynInfo::REG_METHOD));
        locations->SetLocation(CallConvDynInfo::REG_METHOD, param);
        return;
    }

    auto pinfo = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();
    for (uint8_t i = 0; i < CallConvDynInfo::REG_COUNT; i++) {
        [[maybe_unused]] Location loc = pinfo->GetNextLocation(GetWordType());
        ASSERT(loc.IsRegister());
    }
    size_t inputsCount = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);

    for (size_t i = 0; i < inputsCount; ++i) {
        ASSERT(inst->GetInputType(i) == DataType::ANY);
        auto param = Location::MakeStackArgument(i + CallConvDynInfo::FIXED_SLOT_COUNT);
        locations->SetLocation(i, param);
    }
    locations->SetDstLocation(GetLocationForReturn(inst));
    static_cast<LocationsBuilder *>(visitor)->GetGraph()->UpdateStackSlotsCount(inputsCount);
}

LOCATIONS_BUILDER(void)::VisitCallIndirect(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto params = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();

    /* First input is an address of the memory to call. So it hasn't exact location and maybe be any register */
    ASSERT(locations != nullptr);
    locations->SetLocation(0, Location::RequireRegister());

    /* Inputs, starting from 1, are the native call arguments */
    for (size_t i = 1; i < inst->GetInputsCount(); i++) {
        auto param = params->GetNextLocation(inst->GetInputType(i));
        locations->SetLocation(i, param);
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }
}

LOCATIONS_BUILDER(void)::VisitCall(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto params = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        auto param = params->GetNextLocation(inst->GetInputType(i));
        ASSERT(locations != nullptr);
        locations->SetLocation(i, param);
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }
}

LOCATIONS_BUILDER(void)::VisitIntrinsic(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    auto intrinsic = inst->CastToIntrinsic();
    auto id = intrinsic->GetIntrinsicId();
    if (intrinsic->IsNativeCall() || IntrinsicNeedsParamLocations(id)) {
        auto pinfo = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();
        if (intrinsic->IsMethodFirstInput()) {
            pinfo->GetNextLocation(GetWordType());
        }
        if (intrinsic->HasImms() && graph->SupportManagedCode()) {
            for ([[maybe_unused]] auto imm : intrinsic->GetImms()) {
                pinfo->GetNextLocation(DataType::INT32);
            }
        }
        size_t explicitArgs;
        if (IsStackRangeIntrinsic(id, &explicitArgs)) {
            static_cast<LocationsBuilder *>(visitor)->ProcessManagedCallStackRange(inst, explicitArgs, pinfo);
        } else {
            static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst, pinfo);
        }
        return;
    }
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto inputsCount = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    for (size_t i = 0; i < inputsCount; i++) {
        ASSERT(locations != nullptr);
        locations->SetLocation(i, Location::RequireRegister());
    }
}

LOCATIONS_BUILDER(void)::VisitCallNative(GraphVisitor *visitor, Inst *inst)
{
    auto *pinfo = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst, pinfo);
}

LOCATIONS_BUILDER(void)::VisitNewArray([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToNewArray()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    inst->CastToNewArray()->SetLocation(1, Location::MakeRegister(GetTarget().GetParamRegId(1)));
    inst->CastToNewArray()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitNewObject([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToNewObject()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    inst->CastToNewObject()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitParameter([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    /* Currently we can't process parameters in the Locations Builder, because Parameter instructions may be removed
     * during optimizations pipeline. Thus, locations is set by IR builder before optimizations.
     * NOTE(compiler): we need to move Parameters' locations here, thereby we get rid of arch structures in the Graph,
     * such as ParameterInfo, CallingConvention, etc.
     */
}

LOCATIONS_BUILDER(void)::VisitReturn([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    auto returnReg =
        DataType::IsFloatType(inst->GetType()) ? GetTarget().GetReturnRegId() : GetTarget().GetReturnFpRegId();
    inst->CastToReturn()->SetLocation(0, Location::MakeRegister(returnReg));
}

LOCATIONS_BUILDER(void)::VisitThrow([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToThrow()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
}

LOCATIONS_BUILDER(void)::VisitLiveOut([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToLiveOut()->SetLocation(0, Location::MakeRegister(inst->GetDstReg()));
}

LOCATIONS_BUILDER(void)::VisitMultiArray(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    ASSERT(locations != nullptr);
    locations->SetLocation(0, Location::RequireRegister());

    for (size_t i = 1; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 1));
    }
    auto stackArgs = inst->GetInputsCount() - 2U;
    graph->UpdateStackSlotsCount(RoundUp(stackArgs, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitStoreStatic([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        inst->SetFlag(inst_flags::REQUIRE_TMP);
    }
}

LOCATIONS_BUILDER(void)::VisitResolveByName([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToResolveByName()->SetDstLocation(GetLocationForReturn(inst));
}

template <Arch ARCH>
Location LocationsBuilder<ARCH>::GetLocationForReturn(Inst *inst)
{
    auto returnReg =
        DataType::IsFloatType(inst->GetType()) ? GetTarget().GetReturnFpRegId() : GetTarget().GetReturnRegId();
    return Location::MakeRegister(returnReg, inst->GetType());
}

template <Arch ARCH>
ParameterInfo *LocationsBuilder<ARCH>::GetResetParameterInfo()
{
    paramsInfo_->Reset();
    return paramsInfo_;
}
}  // namespace ark::compiler
