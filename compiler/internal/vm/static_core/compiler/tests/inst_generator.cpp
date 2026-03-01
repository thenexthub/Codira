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

#include "inst_generator.h"

namespace ark::compiler {
// CC-OFFNXT(huge_method, huge_cyclomatic_complexity, G.FUN.01) big switch-case
Graph *GraphCreator::GenerateGraph(Inst *inst)
{
    Graph *graph;
    SetNumVRegsArgs(0U, 0U);
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
        case Opcode::StoreObject:
        case Opcode::SelectImm:
        case Opcode::Select:
        case Opcode::ReturnInlined:
        case Opcode::LoadArrayPair:
        case Opcode::LoadArrayPairI:
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI:
        case Opcode::NewArray:
        case Opcode::NewObject:
            // -1 means special processing
            graph = GenerateOperation(inst, -1L);
            break;
        case Opcode::ReturnVoid:
        case Opcode::NullPtr:
        case Opcode::Constant:
        case Opcode::Parameter:
        case Opcode::SpillFill:
        case Opcode::ReturnI:
            graph = GenerateOperation(inst, 0U);
            break;
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::LenArray:
        case Opcode::Return:
        case Opcode::IfImm:
        case Opcode::SaveState:
        case Opcode::SafePoint:
        case Opcode::Cast:
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
        case Opcode::LoadObject:
        case Opcode::LoadStatic:
        case Opcode::Monitor:
        case Opcode::NegSR:
            graph = GenerateOperation(inst, 1U);
            break;
        case Opcode::Add:
        case Opcode::Sub:
        case Opcode::Mul:
        case Opcode::Div:
        case Opcode::Mod:
        case Opcode::Min:
        case Opcode::Max:
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
        case Opcode::Compare:
        case Opcode::Cmp:
        case Opcode::If:
        case Opcode::StoreStatic:
        case Opcode::AndNot:
        case Opcode::OrNot:
        case Opcode::XorNot:
        case Opcode::MNeg:
        case Opcode::AddSR:
        case Opcode::SubSR:
        case Opcode::AndSR:
        case Opcode::OrSR:
        case Opcode::XorSR:
        case Opcode::AndNotSR:
        case Opcode::OrNotSR:
        case Opcode::XorNotSR:
        case Opcode::IsInstance:
            graph = GenerateOperation(inst, 2U);
            break;
        case Opcode::MAdd:
        case Opcode::MSub:
            graph = GenerateOperation(inst, 3U);
            break;
        case Opcode::BoundsCheck:
        case Opcode::BoundsCheckI:
            graph = GenerateBoundaryCheckOperation(inst);
            break;
        case Opcode::NullCheck:
        case Opcode::CheckCast:
        case Opcode::ZeroCheck:
        case Opcode::NegativeCheck:
            graph = GenerateCheckOperation(inst);
            break;
        case Opcode::Phi:
            graph = GeneratePhiOperation(inst);
            break;
        case Opcode::Throw:
            graph = GenerateThrowOperation(inst);
            break;
        case Opcode::MultiArray:
            graph = GenerateMultiArrayOperation(inst);
            break;
        case Opcode::Intrinsic:
            graph = GenerateIntrinsicOperation(inst);
            break;
        default:
            ASSERT_DO(0U, inst->Dump(&std::cerr));
            graph = nullptr;
            break;
    }
    if (graph != nullptr) {
        auto id = graph->GetCurrentInstructionId();
        inst->SetId(id);
        graph->SetCurrentInstructionId(++id);
        graph->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
        graph->ResetParameterInfo();
        for (auto param : graph->GetStartBlock()->Insts()) {
            if (param->GetOpcode() == Opcode::Parameter) {
                param->CastToParameter()->SetLocationData(graph->GetDataForNativeParam(param->GetType()));
                runtime_.argTypes_->push_back(param->GetType());
            }
        }
    }
    if (inst->GetOpcode() == Opcode::CheckCast) {
        runtime_.returnType_ = DataType::REFERENCE;
    } else if (inst->GetType() == DataType::NO_TYPE || inst->IsStore()) {
        runtime_.returnType_ = DataType::VOID;
    } else {
        runtime_.returnType_ = inst->GetType();
    }
#ifndef NDEBUG
    if (graph != nullptr) {
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        graph->SetLowLevelInstructionsEnabled();
    }
#endif
    return graph;
}

Graph *GraphCreator::CreateGraph()
{
    Graph *graph = allocator_.New<Graph>(&allocator_, &localAllocator_, arch_);
    runtime_.argTypes_ = allocator_.New<ArenaVector<DataType::Type>>(allocator_.Adapter());
    graph->SetRuntime(&runtime_);
    graph->SetStackSlotsCount(3U);
    return graph;
}

// NOLINTNEXTLINE(readability-function-size)
Graph *GraphCreator::GenerateOperation(Inst *inst, int32_t n)
{
    Graph *graph;
    auto opc = inst->GetOpcode();
    if (opc == Opcode::If || opc == Opcode::IfImm) {
        graph = CreateGraphWithThreeBasicBlock();
    } else {
        graph = CreateGraphWithOneBasicBlock();
    }
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    PopulateGraph(graph, inst, n);
    return graph;
}

auto DataTypeByOpcode(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::IsInstance:
        case Opcode::LenArray:
        case Opcode::SaveState:
        case Opcode::SafePoint:
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
        case Opcode::NewArray:
        case Opcode::LoadObject:
        case Opcode::Monitor:
            return DataType::REFERENCE;
        case Opcode::IfImm:
            return inst->CastToIfImm()->GetOperandsType();
        case Opcode::If:
            return inst->CastToIf()->GetOperandsType();
        case Opcode::Compare:
            return inst->CastToCompare()->GetOperandsType();
        case Opcode::Cmp:
            return inst->CastToCmp()->GetOperandsType();
        default:
            return inst->GetType();
    }
}

Inst *GraphCreator::PopulateLoadArrayPair(Graph *graph, BasicBlock *block, Inst *inst, Opcode opc)
{
    auto array = CreateParamInst(graph, DataType::REFERENCE, 0U);
    Inst *index = nullptr;
    if (opc == Opcode::LoadArrayPair) {
        index = CreateParamInst(graph, DataType::INT32, 1U);
    }
    inst->SetInput(0U, array);
    if (opc == Opcode::LoadArrayPair) {
        inst->SetInput(1U, index);
    }
    block->AppendInst(inst);
    auto loadPairPart0 = graph->CreateInstLoadPairPart(inst->GetType(), INVALID_PC, inst, 0U);
    auto loadPairPart1 = graph->CreateInstLoadPairPart(inst->GetType(), INVALID_PC, inst, 1U);
    block->AppendInst(loadPairPart0);
    return loadPairPart1;
}

void GraphCreator::PopulateStoreArrayPair(Graph *graph, Inst *inst, Opcode opc)
{
    int stackSlot = 0;
    auto array = CreateParamInst(graph, DataType::REFERENCE, stackSlot++);
    Inst *index = nullptr;
    if (opc == Opcode::StoreArrayPair) {
        index = CreateParamInst(graph, DataType::INT32, stackSlot++);
    }
    auto val1 = CreateParamInst(graph, inst->GetType(), stackSlot++);
    auto val2 = CreateParamInst(graph, inst->GetType(), stackSlot++);
    int idx = 0;
    inst->SetInput(idx++, array);
    if (opc == Opcode::StoreArrayPair) {
        inst->SetInput(idx++, index);
    }
    inst->SetInput(idx++, val1);
    inst->SetInput(idx++, val2);
}

void GraphCreator::PopulateReturnInlined(Graph *graph, BasicBlock *block, Inst *inst, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    block->AppendInst(saveState);

    auto callInst = static_cast<CallInst *>(graph->CreateInstCallStatic());
    callInst->SetType(DataType::VOID);
    callInst->SetInlined(true);
    callInst->SetInputs(&allocator_, {{saveState, DataType::NO_TYPE}});
    block->AppendInst(callInst);

    inst->SetInput(0U, saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateCall(Graph *graph, BasicBlock *block, Inst *inst, DataType::Type type, int32_t n)
{
    ASSERT(n >= 0);
    auto callInst = static_cast<CallInst *>(inst);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    callInst->SetCallMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    block->PrependInst(saveState);
    callInst->AllocateInputTypes(&allocator_, n + 1);
    for (int32_t i = 0; i < n; ++i) {
        auto param = CreateParamInst(graph, type, i);
        callInst->AppendInput(param, type);
        saveState->AppendInput(param);
    }
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    callInst->AppendInput(saveState, DataType::NO_TYPE);
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
}

void GraphCreator::PopulateLoadStoreArray(Graph *graph, Inst *inst, DataType::Type type, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);  // array
    auto param2 = CreateParamInst(graph, DataType::INT32, 1U);      // index
    inst->SetInput(0U, param1);
    inst->SetInput(1U, param2);
    if (inst->GetOpcode() == Opcode::StoreArray) {
        auto param3 = CreateParamInst(graph, type, 2U);
        inst->SetInput(2U, param3);
    }
}

void GraphCreator::PopulateLoadStoreArrayI(Graph *graph, Inst *inst, DataType::Type type, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);  // array/object
    inst->SetInput(0U, param1);
    if (inst->GetOpcode() != Opcode::LoadArrayI) {
        auto param2 = CreateParamInst(graph, type, 1U);
        inst->SetInput(1U, param2);
    }
}

void GraphCreator::PopulateSelect(Graph *graph, Inst *inst, DataType::Type type, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto cmpType = inst->CastToSelect()->GetOperandsType();
    auto param0 = CreateParamInst(graph, type, 0U);
    auto param1 = CreateParamInst(graph, type, 1U);
    auto param2 = CreateParamInst(graph, cmpType, 2U);
    auto param3 = CreateParamInst(graph, cmpType, 3U);
    inst->SetInput(0U, param0);
    inst->SetInput(1U, param1);
    inst->SetInput(2U, param2);
    inst->SetInput(3U, param3);
}

void GraphCreator::PopulateSelectI(Graph *graph, Inst *inst, DataType::Type type, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto cmpType = inst->CastToSelectImm()->GetOperandsType();
    auto param0 = CreateParamInst(graph, type, 0U);
    auto param1 = CreateParamInst(graph, type, 1U);
    auto param2 = CreateParamInst(graph, cmpType, 2U);
    inst->SetInput(0U, param0);
    inst->SetInput(1U, param1);
    inst->SetInput(2U, param2);
}

void GraphCreator::PopulateStoreStatic(Graph *graph, BasicBlock *block, Inst *inst, DataType::Type type)
{
    auto param0 = CreateParamInst(graph, type, 0U);
    inst->SetInput(1U, param0);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param0);
    saveState->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
    auto initInst =
        graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, saveState,
                                          TypeIdMixin {inst->CastToStoreStatic()->GetTypeId(), nullptr}, nullptr);
    inst->SetInput(0U, initInst);
    block->PrependInst(initInst);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateLoadStatic(Graph *graph, BasicBlock *block, Inst *inst)
{
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    auto initInst =
        graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, saveState,
                                          TypeIdMixin {inst->CastToLoadStatic()->GetTypeId(), nullptr}, nullptr);
    inst->SetInput(0U, initInst);
    block->PrependInst(initInst);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateMonitor(Graph *graph, BasicBlock *block, Inst *inst)
{
    auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0U);
    inst->SetInput(0U, param0);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param0);
    saveState->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
    inst->SetInput(1U, saveState);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateLoadType(Graph *graph, BasicBlock *block, Inst *inst)
{
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    inst->SetInput(0U, saveState);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateIsInstance(Graph *graph, BasicBlock *block, Inst *inst)
{
    auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0U);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param0);
    saveState->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
    auto loadClass = graph->CreateInstLoadClass(DataType::REFERENCE, INVALID_PC, saveState, TypeIdMixin {0, nullptr},
                                                reinterpret_cast<RuntimeInterface::ClassPtr>(1));
    inst->SetInput(0U, param0);
    inst->SetInput(1U, loadClass);
    inst->SetSaveState(saveState);
    block->PrependInst(loadClass);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateNewArray(Graph *graph, BasicBlock *block, Inst *inst, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto initInst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC);
    inst->SetInput(NewArrayInst::INDEX_CLASS, initInst);

    auto param0 = CreateParamInst(graph, DataType::INT32, 0U);
    inst->SetInput(NewArrayInst::INDEX_SIZE, param0);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param0);
    saveState->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));

    initInst->SetTypeId(inst->CastToNewArray()->GetTypeId());
    initInst->SetInput(0U, saveState);

    inst->SetInput(NewArrayInst::INDEX_SAVE_STATE, saveState);
    block->PrependInst(initInst);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateNewObject(Graph *graph, BasicBlock *block, Inst *inst, [[maybe_unused]] int32_t n)
{
    ASSERT(n == -1L);
    auto saveState = graph->CreateInstSaveState()->CastToSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    auto initInst =
        graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, saveState,
                                          TypeIdMixin {inst->CastToNewObject()->GetTypeId(), nullptr}, nullptr);
    inst->SetInput(0U, initInst);
    inst->SetInput(0U, initInst);
    inst->SetInput(1U, saveState);
    block->PrependInst(initInst);
    block->PrependInst(saveState);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
}

void GraphCreator::PopulateDefault(Graph *graph, Inst *inst, DataType::Type type, int32_t n)
{
    ASSERT(n >= 0);
    for (int32_t i = 0; i < n; ++i) {
        auto param = CreateParamInst(graph, type, i);
        if (!inst->IsOperandsDynamic()) {
            inst->SetInput(i, param);
        } else {
            inst->AppendInput(param);
        }
    }
}

// CC-OFFNXT(huge_cyclomatic_complexity, huge_cca_cyclomatic_complexity) solid logic
void GraphCreator::PopulateGraph(Graph *graph, Inst *inst, int32_t n)
{
    auto type = DataTypeByOpcode(inst);
    auto block = graph->GetVectorBlocks()[2U];
    auto opc = inst->GetOpcode();
    if (opc == Opcode::LoadArrayPair || opc == Opcode::LoadArrayPairI) {
        inst = PopulateLoadArrayPair(graph, block, inst, opc);
    } else if (opc == Opcode::StoreArrayPairI || opc == Opcode::StoreArrayPair) {
        PopulateStoreArrayPair(graph, inst, opc);
    } else if (opc == Opcode::ReturnInlined) {
        PopulateReturnInlined(graph, block, inst, n);
    } else if (opc == Opcode::CallStatic || opc == Opcode::CallVirtual) {
        PopulateCall(graph, block, inst, type, n);
    } else if (opc == Opcode::LoadArray || opc == Opcode::StoreArray) {
        PopulateLoadStoreArray(graph, inst, type, n);
    } else if (opc == Opcode::LoadArrayI || opc == Opcode::StoreArrayI || opc == Opcode::StoreObject) {
        PopulateLoadStoreArrayI(graph, inst, type, n);
    } else if (opc == Opcode::Select) {
        PopulateSelect(graph, inst, type, n);
    } else if (opc == Opcode::SelectImm) {
        PopulateSelectI(graph, inst, type, n);
    } else if (opc == Opcode::StoreStatic) {
        PopulateStoreStatic(graph, block, inst, type);
    } else if (opc == Opcode::LoadStatic) {
        PopulateLoadStatic(graph, block, inst);
    } else if (opc == Opcode::Monitor) {
        PopulateMonitor(graph, block, inst);
    } else if (opc == Opcode::LoadType || opc == Opcode::LoadString) {
        PopulateLoadType(graph, block, inst);
    } else if (opc == Opcode::IsInstance) {
        PopulateIsInstance(graph, block, inst);
    } else if (opc == Opcode::NewArray) {
        PopulateNewArray(graph, block, inst, n);
    } else if (opc == Opcode::NewObject) {
        PopulateNewObject(graph, block, inst, n);
    } else {
        PopulateDefault(graph, inst, type, n);
    }

    Finalize(graph, block, inst);
}

void GraphCreator::Finalize(Graph *graph, BasicBlock *block, Inst *inst)
{
    auto opc = inst->GetOpcode();
    if (opc == Opcode::Constant || opc == Opcode::Parameter || opc == Opcode::NullPtr) {
        graph->GetStartBlock()->AppendInst(inst);
    } else {
        block->AppendInst(inst);
    }
    if (!inst->IsControlFlow()) {
        if (!inst->NoDest() || IsPseudoUserOfMultiOutput(inst)) {
            auto ret = graph->CreateInstReturn(inst->GetType(), INVALID_PC, inst);
            block->AppendInst(ret);
        } else {
            auto ret = graph->CreateInstReturnVoid();
            block->AppendInst(ret);
        }
    }

    if (opc == Opcode::SaveState || opc == Opcode::SafePoint) {
        auto *saveState = static_cast<SaveStateInst *>(inst);
        for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
            saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
        }
        SetNumVRegsArgs(0U, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    }
    if (inst->GetType() == DataType::REFERENCE) {
        if (inst->GetOpcode() == Opcode::StoreArray) {
            inst->CastToStoreArray()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayI) {
            inst->CastToStoreArrayI()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreStatic) {
            inst->CastToStoreStatic()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreObject) {
            inst->CastToStoreObject()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayPair) {
            inst->CastToStoreArrayPair()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
            inst->CastToStoreArrayPairI()->SetNeedBarrier(true);
        }
    }
}

Inst *GraphCreator::CreateCheckInstByPackArgs(const PackArgsForCkeckInst &pack)
{
    Inst *ret = nullptr;
    DataType::Type type = pack.type;
    if (pack.inst->GetOpcode() == Opcode::CheckCast) {
        pack.inst->SetType(DataType::NO_TYPE);
        ret = pack.graph->CreateInstReturn(type, INVALID_PC, pack.param1);
    } else {
        auto newInst = pack.graph->CreateInst(pack.opcode);
        if (pack.opcode == Opcode::NewArray || pack.opcode == Opcode::NewObject) {
            auto initInst = pack.graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC);
            initInst->SetSaveState(pack.saveState);
            pack.block->AppendInst(initInst);
            if (pack.opcode == Opcode::NewArray) {
                newInst->SetInput(NewArrayInst::INDEX_CLASS, initInst);
                newInst->SetInput(NewArrayInst::INDEX_SIZE, pack.inst);
            } else {
                newInst->SetInput(0U, initInst);
            }
            newInst->SetSaveState(pack.saveState);
            type = DataType::REFERENCE;
        } else if (pack.opcode == Opcode::LoadArray) {
            newInst->SetInput(0U, pack.param1);
            newInst->SetInput(1U, pack.param2);
            type = DataType::REFERENCE;
        } else {
            newInst->SetInput(0U, pack.param1);
            newInst->SetInput(1U, pack.inst);
            type = DataType::UINT64;
        }
        newInst->SetType(type);
        pack.block->AppendInst(newInst);

        ret = pack.graph->CreateInstReturn();
        if (pack.opcode == Opcode::NewArray) {
            ret->SetType(DataType::UINT32);
            ret->SetInput(0U, pack.param2);
        } else {
            ret->SetType(type);
            ret->SetInput(0U, newInst);
        }
    }
    return ret;
}

Graph *GraphCreator::GenerateCheckOperation(Inst *inst)
{
    Opcode opcode;
    DataType::Type type;
    if (inst->GetOpcode() == Opcode::ZeroCheck) {
        opcode = Opcode::Div;
        type = DataType::UINT64;
    } else if (inst->GetOpcode() == Opcode::NegativeCheck) {
        opcode = Opcode::NewArray;
        type = DataType::INT32;
    } else if (inst->GetOpcode() == Opcode::NullCheck) {
        opcode = Opcode::NewObject;
        type = DataType::REFERENCE;
    } else {
        opcode = Opcode::LoadArray;
        type = DataType::REFERENCE;
    }
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, type, 0U);
    auto param2 = CreateParamInst(graph, DataType::UINT32, 1U);
    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    block->AppendInst(saveState);
    inst->SetInput(0U, param1);
    inst->SetSaveState(saveState);
    inst->SetType(type);
    if (inst->GetOpcode() == Opcode::CheckCast) {
        auto loadClass = graph->CreateInstLoadClass(DataType::REFERENCE, INVALID_PC, nullptr, TypeIdMixin {0, nullptr},
                                                    reinterpret_cast<RuntimeInterface::ClassPtr>(1));
        loadClass->SetSaveState(saveState);
        block->AppendInst(loadClass);
        inst->SetInput(1U, loadClass);
    }
    block->AppendInst(inst);

    Inst *ret = CreateCheckInstByPackArgs({opcode, type, inst, param1, param2, saveState, block, graph});
    block->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::GenerateSSOperation(Inst *inst)
{
    DataType::Type type = DataType::UINT64;

    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, type, 0U);
    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    block->AppendInst(saveState);
    if (!inst->IsOperandsDynamic()) {
        inst->SetInput(0U, saveState);
    } else {
        (static_cast<DynamicInputsInst *>(inst))->AppendInput(saveState);
    }
    inst->SetType(type);
    block->AppendInst(inst);

    auto ret = graph->CreateInstReturn(type, INVALID_PC, inst);
    block->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::GenerateBoundaryCheckOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);
    auto param2 = CreateParamInst(graph, DataType::UINT32, 1U);

    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param1);
    saveState->AppendInput(param2);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    block->AppendInst(saveState);

    auto lenArr = graph->CreateInstLenArray(DataType::INT32, INVALID_PC, param1);
    block->AppendInst(lenArr);
    auto boundsCheck = static_cast<FixedInputsInst3 *>(inst);
    boundsCheck->SetInput(0U, lenArr);
    boundsCheck->SetType(DataType::INT32);
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        boundsCheck->SetInput(1U, param2);
        boundsCheck->SetInput(2U, saveState);
    } else {
        boundsCheck->SetInput(1U, saveState);
    }
    block->AppendInst(boundsCheck);

    Inst *ldArr = nullptr;
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        ldArr = graph->CreateInstLoadArray(DataType::UINT32, INVALID_PC, param1, boundsCheck);
    } else {
        ldArr = graph->CreateInstLoadArrayI(DataType::UINT32, INVALID_PC, param1, 1U);
    }
    block->AppendInst(ldArr);

    auto ret = graph->CreateInstReturn(DataType::UINT32, INVALID_PC, ldArr);
    block->AppendInst(ret);
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    return graph;
}

Graph *GraphCreator::GenerateMultiArrayOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::INT32, 0U);
    auto param2 = CreateParamInst(graph, DataType::INT32, 1U);

    auto saveState = graph->CreateInstSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    block->AppendInst(saveState);

    auto initInst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, saveState,
                                                      TypeIdMixin {0U, nullptr}, nullptr);
    auto arraysInst = inst->CastToMultiArray();
    arraysInst->SetInputs(&allocator_, {{initInst, DataType::REFERENCE},
                                        {param1, DataType::INT32},
                                        {param2, DataType::INT32},
                                        {saveState, DataType::NO_TYPE}});

    block->AppendInst(initInst);
    block->AppendInst(inst);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    return graph;
}

Graph *GraphCreator::GenerateThrowOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);

    auto saveState = graph->CreateInstSaveState();
    saveState->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1U);
    block->AppendInst(saveState);

    inst->SetInput(0U, param1);
    inst->SetInput(1U, saveState);
    block->AppendInst(inst);
    return graph;
}

Graph *GraphCreator::GeneratePhiOperation(Inst *inst)
{
    auto *phi = static_cast<PhiInst *>(inst);
    auto graph = CreateGraphWithFourBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() == 6U);
    auto param1 = CreateParamInst(graph, inst->GetType(), 0U);
    auto param2 = CreateParamInst(graph, inst->GetType(), 1U);
    auto param3 = CreateParamInst(graph, DataType::BOOL, 2U);
    auto add = graph->CreateInstAdd();
    auto sub = graph->CreateInstSub();
    auto ifInst = graph->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, param3, 0U, DataType::BOOL, CC_NE);
    graph->GetVectorBlocks()[2U]->AppendInst(ifInst);
    if (inst->GetType() != DataType::REFERENCE) {
        add->SetInput(0U, param1);
        add->SetInput(1U, param2);
        add->SetType(inst->GetType());
        graph->GetVectorBlocks()[3U]->AppendInst(add);

        sub->SetInput(0U, param1);
        sub->SetInput(1U, param2);
        sub->SetType(inst->GetType());
        graph->GetVectorBlocks()[4U]->AppendInst(sub);

        phi->AppendInput(add);
        phi->AppendInput(sub);
    } else {
        phi->AppendInput(param1);
        phi->AppendInput(param2);
    }
    graph->GetVectorBlocks()[5U]->AppendPhi(phi);
    auto ret = graph->CreateInstReturn(phi->GetType(), INVALID_PC, phi);
    graph->GetVectorBlocks()[5U]->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::CreateGraphWithOneBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithTwoBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block1 = graph->CreateEmptyBlock();
    auto block2 = graph->CreateEmptyBlock();
    entry->AddSucc(block1);
    block1->AddSucc(block2);
    block2->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithThreeBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto blockMain = graph->CreateEmptyBlock();
    auto blockTrue = graph->CreateEmptyBlock();
    auto blockFalse = graph->CreateEmptyBlock();
    auto ret1 = graph->CreateInstReturnVoid();
    auto ret2 = graph->CreateInstReturnVoid();
    blockTrue->AppendInst(ret1);
    blockFalse->AppendInst(ret2);
    entry->AddSucc(blockMain);
    blockMain->AddSucc(blockTrue);
    blockMain->AddSucc(blockFalse);
    blockTrue->AddSucc(exit);
    blockFalse->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithFourBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto blockMain = graph->CreateEmptyBlock();
    auto blockTrue = graph->CreateEmptyBlock();
    auto blockFalse = graph->CreateEmptyBlock();
    auto blockPhi = graph->CreateEmptyBlock();

    entry->AddSucc(blockMain);
    blockMain->AddSucc(blockTrue);
    blockMain->AddSucc(blockFalse);
    blockTrue->AddSucc(blockPhi);
    blockFalse->AddSucc(blockPhi);
    blockPhi->AddSucc(exit);
    return graph;
}

ParameterInst *GraphCreator::CreateParamInst(Graph *graph, DataType::Type type, uint8_t slot)
{
    auto param = graph->CreateInstParameter(slot, type);
    graph->GetStartBlock()->AppendInst(param);
    return param;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperations(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, opCode);
        inst->SetType(opcodeXPossibleTypes_[opCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, opCode);
        auto type = opcodeXPossibleTypes_[opCode][i];
        inst->SetType(type);
        inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsShiftedRegister(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        for (auto &shiftType : opcodeXPossibleShiftTypes_[opCode]) {
            auto inst = Inst::New<T>(&allocator_, opCode);
            auto type = opcodeXPossibleTypes_[opCode][i];
            inst->SetType(type);
            inst->SetShiftType(shiftType);
            inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CallInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto inst = Inst::New<CallInst>(&allocator_, opCode);
        inst->SetType(opcodeXPossibleTypes_[opCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CastInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto inst = Inst::New<CastInst>(&allocator_, opCode);
        inst->SetType(opcodeXPossibleTypes_[opCode][i]);
        inst->CastToCast()->SetOperandsType(opcodeXPossibleTypes_[opCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CompareInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto type = opcodeXPossibleTypes_[opCode][i];
        for (int ccInt = ConditionCode::CC_FIRST; ccInt != ConditionCode::CC_LAST; ccInt++) {
            auto cc = static_cast<ConditionCode>(ccInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            if (IsFloatType(type) && (cc == ConditionCode::CC_TST_EQ || cc == ConditionCode::CC_TST_NE)) {
                continue;
            }
            auto inst = Inst::New<CompareInst>(&allocator_, opCode);
            inst->SetType(DataType::BOOL);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CmpInst>(Opcode opCode)
{
    auto inst = Inst::New<CmpInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpg();
    insts_.push_back(inst);
    inst = Inst::New<CmpInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpl();
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<IfInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto type = opcodeXPossibleTypes_[opCode][i];
        for (int ccInt = ConditionCode::CC_FIRST; ccInt != ConditionCode::CC_LAST; ccInt++) {
            auto cc = static_cast<ConditionCode>(ccInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            auto inst = Inst::New<IfInst>(&allocator_, opCode);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<IfImmInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto type = opcodeXPossibleTypes_[opCode][i];
        for (int ccInt = ConditionCode::CC_FIRST; ccInt != ConditionCode::CC_LAST; ccInt++) {
            auto cc = static_cast<ConditionCode>(ccInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            auto inst = Inst::New<IfImmInst>(&allocator_, opCode);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

void InstGenerator::SetFlagsNoCseNoHoistIfReference(Inst *inst, DataType::Type dstType)
{
    if (dstType == DataType::REFERENCE) {
        inst->SetFlag(inst_flags::NO_CSE);
        inst->SetFlag(inst_flags::NO_HOIST);
    }
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<SelectInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto cmpType = opcodeXPossibleTypes_[opCode][i];
        for (int ccInt = ConditionCode::CC_FIRST; ccInt != ConditionCode::CC_LAST; ccInt++) {
            auto cc = static_cast<ConditionCode>(ccInt);
            if (cmpType == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcodeXPossibleTypes_[opCode].size(); ++j) {
                auto dstType = opcodeXPossibleTypes_[opCode][j];
                auto inst = Inst::New<SelectInst>(&allocator_, opCode);
                inst->SetOperandsType(cmpType);
                inst->SetType(dstType);
                inst->SetCc(cc);
                SetFlagsNoCseNoHoistIfReference(inst, dstType);
                insts_.push_back(inst);
            }
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<SelectImmInst>(Opcode opCode)
{
    for (size_t i = 0; i < opcodeXPossibleTypes_[opCode].size(); ++i) {
        auto cmpType = opcodeXPossibleTypes_[opCode][i];
        for (int ccInt = ConditionCode::CC_FIRST; ccInt != ConditionCode::CC_LAST; ccInt++) {
            auto cc = static_cast<ConditionCode>(ccInt);
            if (cmpType == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcodeXPossibleTypes_[opCode].size(); ++j) {
                auto dstType = opcodeXPossibleTypes_[opCode][j];
                auto inst = Inst::New<SelectImmInst>(&allocator_, opCode);
                inst->SetOperandsType(cmpType);
                inst->SetType(dstType);
                inst->SetCc(cc);
                inst->SetImm(cmpType == DataType::REFERENCE ? 0U : 1U);
                SetFlagsNoCseNoHoistIfReference(inst, dstType);
                insts_.push_back(inst);
            }
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<SpillFillInst>(Opcode opCode)
{
    auto inst = Inst::New<SpillFillInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->AddSpill(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->AddFill(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->AddMove(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->AddMemCopy(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<MonitorInst>(Opcode opCode)
{
    auto inst = Inst::New<MonitorInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->SetEntry();
    insts_.push_back(inst);

    inst = Inst::New<MonitorInst>(&allocator_, opCode);
    inst->SetType(opcodeXPossibleTypes_[opCode][0U]);
    inst->SetExit();
    insts_.push_back(inst);

    return insts_;
}

#include "generate_operations_intrinsic_inst.inl"

// CC-OFFNXT(huge_method, huge_cyclomatic_complexity, G.FUN.01) big switch-case
std::vector<Inst *> &InstGenerator::Generate(Opcode opCode)
{
    insts_.clear();
    switch (opCode) {
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
            return GenerateOperations<UnaryOperation>(opCode);
        case Opcode::Add:
        case Opcode::Sub:
        case Opcode::Mul:
        case Opcode::Div:
        case Opcode::Mod:
        case Opcode::Min:
        case Opcode::Max:
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
        case Opcode::AndNot:
        case Opcode::OrNot:
        case Opcode::XorNot:
        case Opcode::MNeg:
            return GenerateOperations<BinaryOperation>(opCode);
        case Opcode::Compare:
            return GenerateOperations<CompareInst>(opCode);
        case Opcode::Constant:
            return GenerateOperations<ConstantInst>(opCode);
        case Opcode::NewObject:
            return GenerateOperations<NewObjectInst>(opCode);
        case Opcode::If:
            return GenerateOperations<IfInst>(opCode);
        case Opcode::IfImm:
            return GenerateOperationsImm<IfImmInst>(opCode);
        case Opcode::IsInstance:
            return GenerateOperations<IsInstanceInst>(opCode);
        case Opcode::LenArray:
            return GenerateOperations<LengthMethodInst>(opCode);
        case Opcode::Return:
        case Opcode::ReturnInlined:
            return GenerateOperations<FixedInputsInst1>(opCode);
        case Opcode::NewArray:
            return GenerateOperations<NewArrayInst>(opCode);
        case Opcode::Cmp:
            return GenerateOperations<CmpInst>(opCode);
        case Opcode::CheckCast:
            return GenerateOperations<CheckCastInst>(opCode);
        case Opcode::NullCheck:
        case Opcode::ZeroCheck:
        case Opcode::NegativeCheck:
            return GenerateOperations<FixedInputsInst2>(opCode);
        case Opcode::Throw:
            return GenerateOperations<ThrowInst>(opCode);
        case Opcode::BoundsCheck:
        case Opcode::MAdd:
        case Opcode::MSub:
            return GenerateOperations<FixedInputsInst3>(opCode);
        case Opcode::Parameter:
            return GenerateOperations<ParameterInst>(opCode);
        case Opcode::LoadArray:
            return GenerateOperations<LoadInst>(opCode);
        case Opcode::StoreArray:
            return GenerateOperations<StoreInst>(opCode);
        case Opcode::LoadArrayI:
            return GenerateOperationsImm<LoadInstI>(opCode);
        case Opcode::StoreArrayI:
            return GenerateOperationsImm<StoreInstI>(opCode);
        case Opcode::NullPtr:
        case Opcode::ReturnVoid:
            return GenerateOperations<FixedInputsInst0>(opCode);
        case Opcode::SaveState:
        case Opcode::SafePoint:
            return GenerateOperations<SaveStateInst>(opCode);
        case Opcode::Phi:
            return GenerateOperations<PhiInst>(opCode);
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
            return GenerateOperations<CallInst>(opCode);
        case Opcode::Monitor:
            return GenerateOperations<MonitorInst>(opCode);
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
            return GenerateOperationsImm<BinaryImmOperation>(opCode);
        case Opcode::AndSR:
        case Opcode::SubSR:
        case Opcode::AddSR:
        case Opcode::OrSR:
        case Opcode::XorSR:
        case Opcode::AndNotSR:
        case Opcode::OrNotSR:
        case Opcode::XorNotSR:
            return GenerateOperationsShiftedRegister<BinaryShiftedRegisterOperation>(opCode);
        case Opcode::NegSR:
            return GenerateOperationsShiftedRegister<UnaryShiftedRegisterOperation>(opCode);
        case Opcode::SpillFill:
            return GenerateOperations<SpillFillInst>(opCode);
        case Opcode::LoadObject:
            return GenerateOperations<LoadObjectInst>(opCode);
        case Opcode::StoreObject:
            return GenerateOperations<StoreObjectInst>(opCode);
        case Opcode::LoadStatic:
            return GenerateOperations<LoadStaticInst>(opCode);
        case Opcode::StoreStatic:
            return GenerateOperations<StoreStaticInst>(opCode);
        case Opcode::LoadString:
        case Opcode::LoadType:
            return GenerateOperations<LoadFromPool>(opCode);
        case Opcode::BoundsCheckI:
            return GenerateOperationsImm<BoundsCheckInstI>(opCode);
        case Opcode::ReturnI:
            return GenerateOperationsImm<ReturnInstI>(opCode);
        case Opcode::Intrinsic:
            return GenerateOperations<IntrinsicInst>(opCode);
        case Opcode::Select:
            return GenerateOperations<SelectInst>(opCode);
        case Opcode::SelectImm:
            return GenerateOperationsImm<SelectImmInst>(opCode);
        case Opcode::LoadArrayPair:
            return GenerateOperations<LoadArrayPairInst>(opCode);
        case Opcode::LoadArrayPairI:
            return GenerateOperationsImm<LoadArrayPairInstI>(opCode);
        case Opcode::StoreArrayPair:
            return GenerateOperations<StoreArrayPairInst>(opCode);
        case Opcode::StoreArrayPairI:
            return GenerateOperationsImm<StoreArrayPairInstI>(opCode);
        case Opcode::Cast:
            return GenerateOperations<CastInst>(opCode);
        case Opcode::Builtin:
            ASSERT_DO(0U, std::cerr << "Unexpected Opcode Builtin\n");
            return insts_;
        default:
            ASSERT_DO(0U, std::cerr << GetOpcodeString(opCode) << "\n");
            return insts_;
    }
}

constexpr std::array<const char *, 15U> LABELS = {"NO_TYPE", "REF",     "BOOL",    "UINT8", "INT8",
                                                  "UINT16",  "INT16",   "UINT32",  "INT32", "UINT64",
                                                  "INT64",   "FLOAT32", "FLOAT64", "ANY",   "VOID"};

void StatisticGenerator::FillHTMLPageHeadPart(std::ofstream &htmlPage)
{
    htmlPage << "<!DOCTYPE html>\n"
             << "<html>\n"
             << "<head>\n"
             << "\t<style>table, th, td {border: 1px solid black; border-collapse: collapse;}</style>\n"
             << "\t<title>Codegen coverage statistic</title>\n"
             << "</head>\n"
             << "<body>\n"
             << "\t<header><h1>Codegen coverage statistic</h1></header>"
             << "\t<h3>Legend</h3>"
             << "\t<table style=\"width:300px%\">\n"
             << "\t\t<tr><th align=\"left\">Codegen successfully translate IR</th><td align=\"center\""
             << "bgcolor=\"#00fd00\" width=\"90px\">+</td></tr>\n"
             << "\t\t<tr><th align=\"left\">Codegen UNsuccessfully translate IR</th><td align=\"center\""
             << "bgcolor=\"#fd0000\">-</td></tr>\n"
             << "\t\t<tr><th align=\"left\">IR does't support instruction with this type </th><td></td></tr>\n"
             << "\t\t<tr><th align=\"left\">Test generator not implement for this opcode</th><td "
             << "bgcolor=\"#808080\"></td></tr>\n"
             << "\t</table>\n"
             << "\t<br>\n"
             << "\t<h3>Summary information</h3>\n"
             << "\t<table>\n"
             << "\t\t<tr><th>Positive tests</th><td>" << positiveInstNumber_ << "</td></tr>\n"
             << "\t\t<tr><th>All generated tests</th><td>" << allInstNumber_ << "</td></tr>\n"
             << "\t\t<tr><th></th><td>" << positiveInstNumber_ * 100.0F / allInstNumber_ << "%</td></tr>\n"
             << "\t</table>\n"
             << "\t<br>\n"
             << "\t<table>"
             << "\t\t<tr><th align=\"left\">Number of opcodes for which tests were generated</th><td>"
             << implementedOpcodeNumber_ << "</td></tr>"
             << "\t\t<tr><th align=\"left\">Full number of opcodes</th><td>" << allOpcodeNumber_ << "</td></tr>"
             << "\t\t<tr><th></th><td>" << implementedOpcodeNumber_ * 100.0F / allOpcodeNumber_ << "%</td></tr>"
             << "\t</table>\n"
             << "\t<h3>Detailed information</h3>"
             << "\t\t<table>"
             << "\t\t<tr><th>Opcode\\Type</th>";
}

void StatisticGenerator::FillHTMLPageOpcodeStatistic(std::ofstream &htmlPage, Opcode opc)
{
    htmlPage << "\t\t<tr>";
    htmlPage << "<th>" << GetOpcodeString(opc) << "</th>";
    if (statistic_.first.find(opc) != statistic_.first.end()) {
        auto item = statistic_.first[opc];
        int positivCount = 0;
        int negativCount = 0;
        for (auto &j : item) {
            std::string flag;
            std::string color;
            switch (j.second) {
                case 0U:
                    flag = "-";
                    color = "bgcolor=\"#fd0000\"";
                    negativCount++;
                    break;
                case 1U:
                    flag = "+";
                    color = "bgcolor=\"#00fd00\"";
                    positivCount++;
                    break;
                default:
                    break;
            }
            htmlPage << "<td align=\"center\" " << color << ">" << flag << "</td>";
        }
        if (positivCount + negativCount != 0U) {
            htmlPage << "<td align=\"right\">" << positivCount * 100.0F / (positivCount + negativCount) << "</td>";
        }
    } else {
        for (auto j = tmplt_.begin(); j != tmplt_.end(); ++j) {
            htmlPage << R"(<td align=" center " bgcolor=" #808080"></td>)";
        }
        htmlPage << "<td align=\"right\">0</td>";
    }
    htmlPage << "</tr>\n";
}

void StatisticGenerator::GenerateHTMLPage(const std::string &fileName)
{
    std::ofstream htmlPage;
    htmlPage.open(fileName);
    FillHTMLPageHeadPart(htmlPage);
    for (auto label : LABELS) {
        htmlPage << "<th style=\"width:90px\">" << label << "</th>";
    }
    htmlPage << "<th>%</th>";
    htmlPage << "<tr>\n";
    for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
        auto opc = static_cast<Opcode>(i);
        if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::LoadPairPart) {
            continue;
        }
        FillHTMLPageOpcodeStatistic(htmlPage, opc);
    }
    htmlPage << "\t</table>\n";

    htmlPage << "\t<h3>Intrinsics</h3>\n";
    htmlPage << "\t<table>\n";
    htmlPage << "\t\t<tr><th>IntrinsicId</th><th>Status</th></tr>";
    for (auto i = 0; i != static_cast<int>(RuntimeInterface::IntrinsicId::COUNT); ++i) {
        auto intrinsicId = static_cast<RuntimeInterface::IntrinsicId>(i);
        auto intrinsicName = GetIntrinsicName(intrinsicId);
        if (intrinsicName.empty()) {
            continue;
        }
        htmlPage << "<tr><th>" << intrinsicName << "</th>";
        if (statistic_.second.find(intrinsicId) != statistic_.second.end()) {
            std::string flag;
            std::string color;
            if (statistic_.second[intrinsicId]) {
                flag = "+";
                color = "bgcolor=\"#00fd00\"";
            } else {
                flag = "-";
                color = "bgcolor=\"#fd0000\"";
            }
            htmlPage << "<td align=\"center\" " << color << ">" << flag << "</td></tr>";
        } else {
            htmlPage << R"(<td align=" center " bgcolor=" #808080"></td></tr>)";
        }
        htmlPage << "\n";
    }
    htmlPage << "</table>\n";
    htmlPage << "</body>\n"
             << "</html>";
    htmlPage.close();
}
}  // namespace ark::compiler
