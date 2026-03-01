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

#include "inst_builder.h"
#include "phi_resolver.h"
#include "optimizer/code_generator/encode.h"
#include "compiler_logger.h"
#ifndef PANDA_TARGET_WINDOWS
#include "callconv.h"
#endif

namespace ark::compiler {

InstBuilder::InstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *callerInst, uint32_t inliningDepth)
    : graph_(graph),
      runtime_(graph->GetRuntime()),
      defs_(graph->GetLocalAllocator()->Adapter()),
      method_(method),
      vregsAndArgsCount_(graph->GetRuntime()->GetMethodRegistersCount(method) +
                         graph->GetRuntime()->GetMethodTotalArgumentsCount(method)),
      instructionsBuf_(GetGraph()->GetRuntime()->GetMethodCode(GetGraph()->GetMethod())),
      callerInst_(callerInst),
      inliningDepth_(inliningDepth),
      classId_ {runtime_->GetClassIdForMethod(method_)}
{
    noTypeMarker_ = GetGraph()->NewMarker();
    visitedBlockMarker_ = GetGraph()->NewMarker();

    defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph->GetLocalAllocator()->Adapter()));
    for (auto &v : defs_) {
        v.resize(GetVRegsCount());
    }

    for (auto bb : graph->GetBlocksRPO()) {
        if (bb->IsCatchBegin()) {
            for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
                auto catchPhi = GetGraph()->CreateInstCatchPhi(DataType::NO_TYPE, bb->GetGuestPc());
                catchPhi->SetMarker(GetNoTypeMarker());
                bb->AppendInst(catchPhi);
                COMPILER_LOG(DEBUG, IR_BUILDER)
                    << "Creat catchphi " << catchPhi->GetId() << " for bb(" << bb->GetId() << ")";
                if (vreg == vregsAndArgsCount_) {
                    catchPhi->SetIsAcc();
                } else if (vreg > vregsAndArgsCount_) {
                    catchPhi->SetType(DataType::ANY);
                }
            }
        }
    }
}

void InstBuilder::InitEnv(BasicBlock *bb)
{
    auto thisFunc = GetGraph()->FindParameter(0);
    auto cp = GetGraph()->CreateInstLoadConstantPool(DataType::ANY, INVALID_PC, thisFunc);
    bb->AppendInst(cp);

    auto lexEnv = GetGraph()->CreateInstLoadLexicalEnv(DataType::ANY, INVALID_PC, thisFunc);
    bb->AppendInst(lexEnv);

    defs_[bb->GetId()][vregsAndArgsCount_ + 1 + THIS_FUNC_IDX] = thisFunc;
    defs_[bb->GetId()][vregsAndArgsCount_ + 1 + CONST_POOL_IDX] = cp;
    defs_[bb->GetId()][vregsAndArgsCount_ + 1 + LEX_ENV_IDX] = lexEnv;
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Init environment this_func = " << thisFunc->GetId()
                                    << ", const_pool = " << cp->GetId() << ", lex_env = " << lexEnv->GetId();
}

void InstBuilder::SetCurrentBlock(BasicBlock *bb)
{
    if (GetGraph()->IsDynamicMethod() && !GetGraph()->IsBytecodeOptimizer() &&
        currentBb_ != GetGraph()->GetStartBlock() && currentDefs_ != nullptr) {
        ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + THIS_FUNC_IDX] != nullptr);
        ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + CONST_POOL_IDX] != nullptr);
        ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] != nullptr);
    }
    currentBb_ = bb;
    currentDefs_ = &defs_[bb->GetId()];
}

/* static */
DataType::Type InstBuilder::ConvertPbcType(panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return DataType::VOID;
        case panda_file::Type::TypeId::U1:
            return DataType::BOOL;
        case panda_file::Type::TypeId::I8:
            return DataType::INT8;
        case panda_file::Type::TypeId::U8:
            return DataType::UINT8;
        case panda_file::Type::TypeId::I16:
            return DataType::INT16;
        case panda_file::Type::TypeId::U16:
            return DataType::UINT16;
        case panda_file::Type::TypeId::I32:
            return DataType::INT32;
        case panda_file::Type::TypeId::U32:
            return DataType::UINT32;
        case panda_file::Type::TypeId::I64:
            return DataType::INT64;
        case panda_file::Type::TypeId::U64:
            return DataType::UINT64;
        case panda_file::Type::TypeId::F32:
            return DataType::FLOAT32;
        case panda_file::Type::TypeId::F64:
            return DataType::FLOAT64;
        case panda_file::Type::TypeId::REFERENCE:
            return DataType::REFERENCE;
        case panda_file::Type::TypeId::TAGGED:
        case panda_file::Type::TypeId::INVALID:
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

void InstBuilder::Prepare(bool isInlinedGraph)
{
    SetCurrentBlock(GetGraph()->GetStartBlock());
#ifndef PANDA_TARGET_WINDOWS
    GetGraph()->ResetParameterInfo();
#endif
    // Create parameter for actual num args
    if (!GetGraph()->IsBytecodeOptimizer() && GetGraph()->IsDynamicMethod() && !GetGraph()->GetMode().IsDynamicStub()) {
        auto paramInst = GetGraph()->AddNewParameter(ParameterInst::DYNAMIC_NUM_ARGS);
        paramInst->SetType(DataType::UINT32);
        paramInst->SetLocationData(GetGraph()->GetDataForNativeParam(DataType::UINT32));
    }
    size_t argRefNum = 0;
    if (GetRuntime()->GetMethodReturnType(GetMethod()) == DataType::REFERENCE) {
        argRefNum = 1;
    }
    auto numArgs = GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
    bool isStatic = GetRuntime()->IsMethodStatic(GetMethod());
    // Create Parameter instructions for all arguments
    for (size_t i = 0; i < numArgs; i++) {
        auto paramInst = GetGraph()->AddNewParameter(i);
        if (GetGraph()->IsAbcKit()) {
            paramInst->SetFlag(inst_flags::NO_DCE);
        }
        auto type = GetCurrentMethodArgumentType(i);
        auto regNum = GetRuntime()->GetMethodRegistersCount(GetMethod()) + i;
        ASSERT(!GetGraph()->IsBytecodeOptimizer() || regNum != GetInvalidReg());

        paramInst->SetType(type);
        // This parameter in virtual method is implicit, so skipped
        if (type == DataType::REFERENCE && (isStatic || i > 0)) {
            paramInst->SetArgRefNumber(argRefNum++);
        }
        SetParamSpillFill(GetGraph(), paramInst, numArgs, i, type);

        UpdateDefinition(regNum, paramInst);
    }

    // We don't need to create SafePoint at the beginning of the callee graph
    if (g_options.IsCompilerUseSafepoint() && !isInlinedGraph) {
        GetGraph()->GetStartBlock()->AppendInst(CreateSafePoint(GetGraph()->GetStartBlock()));
    }
    methodProfile_ = GetRuntime()->GetMethodProfile(GetMethod(), !GetGraph()->IsAotMode());
}

void InstBuilder::UpdateDefsForCatch()
{
    Inst *catchPhi = currentBb_->GetFirstInst();
    ASSERT(catchPhi != nullptr);
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        ASSERT(catchPhi->IsCatchPhi());
        defs_[currentBb_->GetId()][vreg] = catchPhi;
        catchPhi = catchPhi->GetNext();
    }
}

void InstBuilder::UpdateDefsForLoopHead()
{
    // If current block is a loop header, then propagate all definitions from preheader's predecessors to
    // current block.
    ASSERT(currentBb_->GetLoop()->GetPreHeader());
    auto predDefs = defs_[currentBb_->GetLoop()->GetPreHeader()->GetId()];
    COMPILER_LOG(DEBUG, IR_BUILDER) << "basic block is loop header";
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        auto defInst = predDefs[vreg];
        if (defInst != nullptr) {
            auto phi = GetGraph()->CreateInstPhi();
            if (vreg > vregsAndArgsCount_) {
                phi->SetType(DataType::ANY);
            }
            phi->SetMarker(GetNoTypeMarker());
            phi->SetLinearNumber(vreg);
            currentBb_->AppendPhi(phi);
            (*currentDefs_)[vreg] = phi;
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg
                                            << "(def id=" << predDefs[vreg]->GetId() << ")";
        }
    }
}

void InstBuilder::UpdateDefinition(size_t vreg, Inst *inst)
{
    ASSERT(vreg < currentDefs_->size());
    COMPILER_LOG(DEBUG, IR_BUILDER) << "update def for r" << vreg << " from "
                                    << ((*currentDefs_)[vreg] != nullptr
                                            ? std::to_string((*currentDefs_)[vreg]->GetId())
                                            : "null")
                                    << " to " << inst->GetId();
    (*currentDefs_)[vreg] = inst;
}

void InstBuilder::UpdateDefinitionAcc(Inst *inst)
{
    if (inst == nullptr) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "reset accumulator definition";
    } else {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "update accumulator from "
                                        << ((*currentDefs_)[vregsAndArgsCount_] != nullptr
                                                ? std::to_string((*currentDefs_)[vregsAndArgsCount_]->GetId())
                                                : "null")
                                        << " to " << inst->GetId();
    }
    (*currentDefs_)[vregsAndArgsCount_] = inst;
}

void InstBuilder::UpdateDefinitionLexEnv(Inst *inst)
{
    ASSERT(inst != nullptr);
    ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] != nullptr);
    COMPILER_LOG(DEBUG, IR_BUILDER) << "update lexical environment from "
                                    << std::to_string((*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX]->GetId())
                                    << " to " << inst->GetId();
    (*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] = inst;
}

Inst *InstBuilder::GetDefinition(size_t vreg)
{
    ASSERT(vreg < currentDefs_->size());
    ASSERT((*currentDefs_)[vreg] != nullptr);

    if (vreg >= currentDefs_->size() || (*currentDefs_)[vreg] == nullptr) {
        failed_ = true;
        COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinition failed for verg " << vreg;
        return nullptr;
    }
    return (*currentDefs_)[vreg];
}

Inst *InstBuilder::GetDefinitionAcc()
{
    auto *accInst = (*currentDefs_)[vregsAndArgsCount_];
    ASSERT(accInst != nullptr);

    if (accInst == nullptr) {
        failed_ = true;
        COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinitionAcc failed";
    }
    return accInst;
}

Inst *InstBuilder::GetEnvDefinition(uint8_t envIdx)
{
    auto *inst = (*currentDefs_)[vregsAndArgsCount_ + 1 + envIdx];
    ASSERT(inst != nullptr);

    if (inst == nullptr) {
        failed_ = true;
    }
    return inst;
}

ConstantInst *InstBuilder::FindOrCreate32BitConstant(uint32_t value)
{
    auto inst = GetGraph()->FindOrCreateConstant<uint32_t>(value);
    if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
    }
    return inst;
}

ConstantInst *InstBuilder::FindOrCreateConstant(uint64_t value)
{
    auto inst = GetGraph()->FindOrCreateConstant<uint64_t>(value);
    if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
    }
    return inst;
}

ConstantInst *InstBuilder::FindOrCreateAnyConstant(DataType::Any value)
{
    auto inst = GetGraph()->FindOrCreateConstant(value);
    if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value.Raw() << ", inst=" << inst->GetId();
    }
    return inst;
}

ConstantInst *InstBuilder::FindOrCreateDoubleConstant(double value)
{
    auto inst = GetGraph()->FindOrCreateConstant<double>(value);
    if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
    }
    return inst;
}

ConstantInst *InstBuilder::FindOrCreateFloatConstant(float value)
{
    auto inst = GetGraph()->FindOrCreateConstant<float>(value);
    if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
        COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
    }
    return inst;
}

bool InstBuilder::UpdateDefsForPreds(size_t vreg, std::optional<Inst *> &value)
{
    for (auto predBb : currentBb_->GetPredsBlocks()) {
        // When irreducible loop header is visited before it's back-edge, phi should be created,
        // since we do not know if definitions are different at this point
        if (!predBb->IsMarked(visitedBlockMarker_)) {
            ASSERT(currentBb_->GetLoop()->IsIrreducible());
            return true;
        }
        if (!value.has_value()) {
            value = defs_[predBb->GetId()][vreg];
        } else if (value.value() != defs_[predBb->GetId()][vreg]) {
            return true;
        }
    }
    return false;
}

void InstBuilder::UpdateDefs()
{
    currentBb_->SetMarker(visitedBlockMarker_);
    if (currentBb_->IsCatchBegin()) {
        UpdateDefsForCatch();
    } else if (currentBb_->IsLoopHeader() && !currentBb_->GetLoop()->IsIrreducible()) {
        UpdateDefsForLoopHead();
    } else if (currentBb_->GetPredsBlocks().size() == 1) {
        // Only one predecessor - simply copy all its definitions
        auto &predDefs = defs_[currentBb_->GetPredsBlocks()[0]->GetId()];
        std::copy(predDefs.begin(), predDefs.end(), currentDefs_->begin());
    } else if (currentBb_->GetPredsBlocks().size() > 1) {
        // If there are multiple predecessors, then add phi for each register that has different definitions
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
            std::optional<Inst *> value;
            bool different = UpdateDefsForPreds(vreg, value);
            if (different) {
                auto phi = GetGraph()->CreateInstPhi();
                phi->SetMarker(GetNoTypeMarker());
                phi->SetLinearNumber(vreg);
                currentBb_->AppendPhi(phi);
                (*currentDefs_)[vreg] = phi;
                COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg;
            } else {
                (*currentDefs_)[vreg] = value.value();
            }
        }
    }
}

void InstBuilder::AddCatchPhiInputs(const ArenaUnorderedSet<BasicBlock *> &catchHandlers, const InstVector &defs,
                                    Inst *throwableInst)
{
    ASSERT(!catchHandlers.empty());
    for (auto catchBb : catchHandlers) {
        auto inst = catchBb->GetFirstInst();
        while (!inst->IsCatchPhi()) {
            inst = inst->GetNext();
        }
        ASSERT(inst != nullptr);
        GetGraph()->AppendThrowableInst(throwableInst, catchBb);
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++, inst = inst->GetNext()) {
            ASSERT(inst->GetOpcode() == Opcode::CatchPhi);
            auto catchPhi = inst->CastToCatchPhi();
            if (catchPhi->IsAcc()) {
                ASSERT(vreg == vregsAndArgsCount_);
                continue;
            }
            auto inputInst = defs[vreg];
            if (inputInst != nullptr && inputInst != catchPhi) {
                catchPhi->AppendInput(inputInst);
                catchPhi->AppendThrowableInst(throwableInst);
            }
        }
    }
}

void InstBuilder::SetParamSpillFill(Graph *graph, ParameterInst *paramInst, size_t numArgs, size_t i,
                                    DataType::Type type)
{
    if (graph->IsBytecodeOptimizer()) {
        auto regSrc = static_cast<Register>(GetFrameSize() - numArgs + i);
        DataType::Type regType;
        if (DataType::IsReference(type)) {
            regType = DataType::REFERENCE;
        } else if (DataType::Is64Bits(type, graph->GetArch())) {
            regType = DataType::UINT64;
        } else {
            regType = DataType::UINT32;
        }

        paramInst->SetLocationData({LocationType::REGISTER, LocationType::REGISTER, regSrc, regSrc, regType});
    } else {
#ifndef PANDA_TARGET_WINDOWS
        if (graph->IsDynamicMethod() && !graph->GetMode().IsDynamicStub()) {
            ASSERT(type == DataType::ANY);
            uint16_t slot = i + CallConvDynInfo::FIXED_SLOT_COUNT;
            ASSERT(slot <= UINT8_MAX);
            paramInst->SetLocationData(
                {LocationType::STACK_PARAMETER, LocationType::INVALID, slot, GetInvalidReg(), DataType::UINT64});
        } else {
            paramInst->SetLocationData(graph->GetDataForNativeParam(type));
        }
#endif
    }
}

/// Set type of instruction, then recursively set type to its inputs.
void InstBuilder::SetTypeRec(Inst *inst, DataType::Type type)
{
    inst->SetType(type);
    inst->ResetMarker(GetNoTypeMarker());
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            SetTypeRec(input.GetInst(), type);
        }
    }
}

/**
 * Remove vreg from SaveState for the case
 * BB 1
 *   ....
 * succs: [bb 2, bb 3]
 *
 * BB 2: preds: [bb 1]
 *   89.i64  Sub                        v85, v88 -> (v119, v90)
 *   90.f64  Cast                       v89 -> (v96, v92)
 * succs: [bb 3]
 *
 * BB 3: preds: [bb 1, bb 2]
 *   .....
 *   119.     SaveState                  v105(vr0), v106(vr1), v94(vr4), v89(vr8), v0(vr10), v1(vr11) -> (v120)
 *
 * v89(vr8) used only in BB 2, so we need to remove its from "119.     SaveState"
 */
/* static */
void InstBuilder::RemoveNotDominateInputs(SaveStateInst *saveState)
{
    size_t idx = 0;
    size_t inputsCount = saveState->GetInputsCount();
    while (idx < inputsCount) {
        auto inputInst = saveState->GetInput(idx).GetInst();
        // We can don't call IsDominate, if save_state and input_inst in one basic block.
        // It's reduce number of IsDominate calls.
        if (!inputInst->InSameBlockOrDominate(saveState)) {
            saveState->RemoveInput(idx);
            inputsCount--;
        } else {
            ASSERT(inputInst->GetBasicBlock() != saveState->GetBasicBlock() || inputInst->IsDominate(saveState));
            idx++;
        }
    }
}

// Remove dead Phi and set types to phi which have not type.
// Phi may not have type if all it users are pseudo instructions, like SaveState
void InstBuilder::FixType(PhiInst *inst, BasicBlock *bb)
{
    inst->ReserveInputs(bb->GetPredsBlocks().size());
    for (auto &predBb : bb->GetPredsBlocks()) {
        if (inst->GetLinearNumber() == INVALID_LINEAR_NUM) {
            continue;
        }
        auto pred = defs_[predBb->GetId()][inst->GetLinearNumber()];
        if (pred == nullptr) {
            // If any input of phi instruction is not defined then we assume that phi is dead. DCE should
            // remove it.
            continue;
        }
        inst->AppendInput(pred);
    }
}

// Check all instructions that have no type and fix it. Type is got from instructions with known input types.
void InstBuilder::FixType(Inst *inst)
{
    if (inst->IsSaveState()) {
        RemoveNotDominateInputs(static_cast<SaveStateInst *>(inst));
        return;
    }
    auto inputIdx = 0;
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            auto inputType = inst->GetInputType(inputIdx);
            if (inputType != DataType::NO_TYPE) {
                SetTypeRec(input.GetInst(), inputType);
            }
        }
        inputIdx++;
    }
}

/// Fix instructions that can't be fully completed in building process.
void InstBuilder::FixInstructions()
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->PhiInstsSafe()) {
            FixType(inst->CastToPhi(), bb);
        }
    }

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FixType(inst);
        }
    }
    // Resolve dead and inconsistent phi instructions
    PhiResolver phiResolver(GetGraph());
    phiResolver.Run();
    ResolveConstants();
}

SaveStateInst *InstBuilder::CreateSaveState(Opcode opc, size_t pc)
{
    ASSERT(opc == Opcode::SaveState || opc == Opcode::SafePoint || opc == Opcode::SaveStateOsr ||
           opc == Opcode::SaveStateDeoptimize);
    SaveStateInst *inst;
    bool withoutNumericInputs = false;
    auto liveVergsCount =
        std::count_if(currentDefs_->begin(), currentDefs_->end(), [](Inst *p) { return p != nullptr; });
    if (opc == Opcode::SaveState) {
        inst = GetGraph()->CreateInstSaveState(pc, GetMethod(), callerInst_, inliningDepth_);
    } else if (opc == Opcode::SaveStateOsr) {
        inst = GetGraph()->CreateInstSaveStateOsr(pc, GetMethod(), callerInst_, inliningDepth_);
    } else if (opc == Opcode::SafePoint) {
        inst = GetGraph()->CreateInstSafePoint(pc, GetMethod(), callerInst_, inliningDepth_);
        withoutNumericInputs = true;
    } else {
        inst = GetGraph()->CreateInstSaveStateDeoptimize(pc, GetMethod(), callerInst_, inliningDepth_);
    }
    if (GetGraph()->IsBytecodeOptimizer()) {
        inst->ReserveInputs(0);
        return inst;
    }
    inst->ReserveInputs(liveVergsCount);

    for (VirtualRegister::ValueType regIdx = 0; regIdx < vregsAndArgsCount_; ++regIdx) {
        auto defInst = (*currentDefs_)[regIdx];
        if (defInst != nullptr && (!withoutNumericInputs || !DataType::IsTypeNumeric(defInst->GetType()))) {
            auto inputIdx = inst->AppendInput(defInst);
            inst->SetVirtualRegister(inputIdx, VirtualRegister(regIdx, VRegType::VREG));
        }
    }
    VirtualRegister::ValueType regIdx = vregsAndArgsCount_;
    auto defInst = (*currentDefs_)[regIdx];
    if (defInst != nullptr && (!withoutNumericInputs || !DataType::IsTypeNumeric(defInst->GetType()))) {
        auto inputIdx = inst->AppendInput(defInst);
        inst->SetVirtualRegister(inputIdx, VirtualRegister(regIdx, VRegType::ACC));
    }
    regIdx++;
    if (GetGraph()->IsDynamicMethod() && !GetGraph()->IsBytecodeOptimizer() && (*currentDefs_)[regIdx] != nullptr) {
        for (uint8_t envIdx = 0; envIdx < VRegInfo::ENV_COUNT; ++envIdx) {
            auto inputIdx = inst->AppendInput(GetEnvDefinition(envIdx));
            inst->SetVirtualRegister(inputIdx, VirtualRegister(regIdx++, VRegType(VRegType::ENV_BEGIN + envIdx)));
        }
        if (additionalDef_ != nullptr) {
            inst->AppendBridge(additionalDef_);
        }
    }
    return inst;
}

ClassInst *InstBuilder::CreateLoadAndInitClassGeneric(uint32_t classId, size_t pc)
{
    auto classPtr = GetRuntime()->ResolveType(GetGraph()->GetMethod(), classId);
    ClassInst *inst = nullptr;
    if (classPtr == nullptr) {
        ASSERT(!graph_->IsBytecodeOptimizer());
        inst = graph_->CreateInstUnresolvedLoadAndInitClass(DataType::REFERENCE, pc, nullptr,
                                                            TypeIdMixin {classId, GetGraph()->GetMethod()}, classPtr);
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            auto unresolvedTypes = GetRuntime()->GetUnresolvedTypes();
            ASSERT(unresolvedTypes != nullptr);
            unresolvedTypes->AddTableSlot(GetMethod(), classId, UnresolvedTypesInterface::SlotKind::CLASS);
        }
    } else {
        ASSERT(classId != 0);
        inst = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, nullptr,
                                                  TypeIdMixin {classId, GetGraph()->GetMethod()}, classPtr);
    }
    return inst;
}

DataType::Type InstBuilder::GetCurrentMethodReturnType() const
{
    return GetRuntime()->GetMethodReturnType(GetMethod());
}

DataType::Type InstBuilder::GetCurrentMethodArgumentType(size_t index) const
{
    return GetRuntime()->GetMethodTotalArgumentType(GetMethod(), index);
}

size_t InstBuilder::GetCurrentMethodArgumentsCount() const
{
    return GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
}

DataType::Type InstBuilder::GetMethodReturnType(uintptr_t id) const
{
    return GetRuntime()->GetMethodReturnType(GetMethod(), id);
}

DataType::Type InstBuilder::GetMethodArgumentType(uintptr_t id, size_t index) const
{
    return GetRuntime()->GetMethodArgumentType(GetMethod(), id, index);
}

size_t InstBuilder::GetMethodArgumentsCount(uintptr_t id) const
{
    return GetRuntime()->GetMethodArgumentsCount(GetMethod(), id);
}

size_t InstBuilder::GetPc(const uint8_t *instPtr) const
{
    return instPtr - instructionsBuf_;
}

void InstBuilder::ResolveConstants()
{
    ConstantInst *currConst = GetGraph()->GetFirstConstInst();
    while (currConst != nullptr) {
        SplitConstant(currConst);
        currConst = currConst->GetNextConst();
    }
}

void InstBuilder::SplitConstant(ConstantInst *constInst)
{
    if (constInst->GetType() != DataType::INT64 || !constInst->HasUsers()) {
        return;
    }
    auto users = constInst->GetUsers();
    auto currIt = users.begin();
    while (currIt != users.end()) {
        auto user = (*currIt).GetInst();
        DataType::Type type = user->GetInputType(currIt->GetIndex());
        ++currIt;
        if (type != DataType::FLOAT32 && type != DataType::FLOAT64) {
            continue;
        }
        ConstantInst *newConst = nullptr;
        if (type == DataType::FLOAT32) {
            auto val = bit_cast<float>(static_cast<uint32_t>(constInst->GetIntValue()));
            newConst = GetGraph()->FindOrCreateConstant(val);
        } else {
            auto val = bit_cast<double, uint64_t>(constInst->GetIntValue());
            newConst = GetGraph()->FindOrCreateConstant(val);
        }
        user->ReplaceInput(constInst, newConst);
    }
}

void InstBuilder::SyncWithGraph()
{
    size_t idx = currentDefs_ - &defs_[0];
    size_t size = defs_.size();
    defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph_->GetLocalAllocator()->Adapter()));
    for (size_t i = size; i < defs_.size(); i++) {
        defs_[i].resize(vregsAndArgsCount_ + 1 + graph_->GetEnvCount());
        std::copy(defs_[idx].cbegin(), defs_[idx].cend(), defs_[i].begin());
    }
    currentDefs_ = &defs_[currentBb_->GetId()];
}

template <bool IS_STATIC>
bool InstBuilder::IsInConstructor() const
{
    for (auto graph = GetGraph(); graph != nullptr; graph = graph->GetParentGraph()) {
        auto method = graph->GetMethod();
        if (IS_STATIC ? GetRuntime()->IsMethodStaticConstructor(method) : GetRuntime()->IsInstanceConstructor(method)) {
            return true;
        }
    }
    return false;
}
template bool InstBuilder::IsInConstructor<true>() const;
template bool InstBuilder::IsInConstructor<false>() const;

}  // namespace ark::compiler
