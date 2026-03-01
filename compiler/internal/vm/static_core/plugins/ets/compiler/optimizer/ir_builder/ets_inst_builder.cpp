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

#include <cstdint>
#include "libarkbase/utils/utils.h"
#include "compiler_logger.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/bytecode_instruction-inl.h"

namespace ark::compiler {

static RuntimeInterface::IntrinsicId GetIntrinsicId(DataType::Type type)
{
    switch (type) {
        case DataType::REFERENCE:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_OBJ;
        case DataType::FLOAT64:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F64;
        case DataType::FLOAT32:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F32;
        case DataType::UINT64:
        case DataType::INT64:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I64;
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I32;
        default:
            UNREACHABLE();
    }
}

#ifdef ENABLE_LIBABCKIT
static RuntimeInterface::IntrinsicId GetAbcKitIntrinsicId(DataType::Type type)
{
    switch (type) {
        case DataType::REFERENCE:
            return RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_ETS_LD_OBJ_BY_NAME_OBJECT;
        case DataType::UINT64:
        case DataType::INT64:
        case DataType::FLOAT64:
            return RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_ETS_LD_OBJ_BY_NAME_I64;
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
        case DataType::FLOAT32:
            return RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_ETS_LD_OBJ_BY_NAME_I32;
        default:
            UNREACHABLE();
    }
}
#endif  // ENABLE_LIBABCKIT

template <bool IS_ABC_KIT>
void InstBuilder::BuildLdObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    RuntimeInterface::IntrinsicId intrinsicId;
    if constexpr (IS_ABC_KIT) {
#ifdef ENABLE_LIBABCKIT
        intrinsicId = GetAbcKitIntrinsicId(type);
#else
        UNREACHABLE();
#endif  // ENABLE_LIBABCKIT
    } else {
        intrinsicId = GetIntrinsicId(type);
    }

    auto intrinsic = GetGraph()->CreateInstIntrinsic(type, pc, intrinsicId);
    if constexpr (IS_ABC_KIT) {
        // AbcKit mode: add object reference as input and fieldId as imm
        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 1_I);
        intrinsic->AppendInput(GetDefinition(bcInst->GetVReg(0)));
        intrinsic->AddInputType(DataType::REFERENCE);
        intrinsic->AddImm(GetGraph()->GetAllocator(), fieldId);
        intrinsic->SetMethod(GetMethod());
    } else {
        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);

        // Create SaveState instruction
        auto saveState = CreateSaveState(Opcode::SaveState, pc);

        // Create NullCheck instruction
        auto nullCheck =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);

        intrinsic->AppendInput(nullCheck);
        intrinsic->AddInputType(DataType::REFERENCE);
        intrinsic->AppendInput(saveState);
        intrinsic->AddInputType(DataType::NO_TYPE);

        AddInstruction(saveState);
        AddInstruction(nullCheck);

        intrinsic->AddImm(GetGraph()->GetAllocator(), fieldId);
        intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

        intrinsic->SetMethodFirstInput();
        intrinsic->SetMethod(GetMethod());
    }

    AddInstruction(intrinsic);

    UpdateDefinitionAcc(intrinsic);
}

template void InstBuilder::BuildLdObjByName<true>(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template void InstBuilder::BuildLdObjByName<false>(const BytecodeInstruction *bcInst, compiler::DataType::Type type);

static std::pair<RuntimeInterface::IntrinsicId, DataType::Type> ExtractIntrinsicIdByType(DataType::Type type)
{
    switch (type) {
        case DataType::REFERENCE:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_OBJ, type};
        case DataType::FLOAT64:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F64, type};
        case DataType::FLOAT32:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F32, type};
        case DataType::UINT64:
        case DataType::INT64:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I64, DataType::INT64};
        case DataType::UINT8:
        case DataType::INT8:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I8, DataType::INT8};
        case DataType::UINT16:
        case DataType::INT16:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I16, DataType::INT16};
        case DataType::UINT32:
        case DataType::INT32:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I32, DataType::INT32};
        default:
            UNREACHABLE();
            break;
    }
    UNREACHABLE();
}

template <bool IS_ABC_KIT>
IntrinsicInst *InstBuilder::CreateStObjByNameIntrinsic(size_t pc, compiler::DataType::Type type)
{
    auto [id, extractedType] = ExtractIntrinsicIdByType(type);
    type = extractedType;
    auto *intrinsic = GetGraph()->CreateInstIntrinsic(DataType::VOID, pc, id);

    if (intrinsic->RequireState()) {
        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 3_I);
        intrinsic->AddInputType(DataType::REFERENCE);
        intrinsic->AddInputType(type);
        intrinsic->AddInputType(DataType::NO_TYPE);
    } else {
        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 1_I);
        intrinsic->AddInputType(type);
    }
    return intrinsic;
}

template IntrinsicInst *InstBuilder::CreateStObjByNameIntrinsic<true>(size_t pc, compiler::DataType::Type type);
template IntrinsicInst *InstBuilder::CreateStObjByNameIntrinsic<false>(size_t pc, compiler::DataType::Type type);

template <bool IS_ABC_KIT>
void InstBuilder::BuildStObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Get a value to store
    Inst *storeVal = nullptr;
    storeVal = GetDefinitionAcc();

    auto *intrinsic = CreateStObjByNameIntrinsic<IS_ABC_KIT>(pc, type);

    if constexpr (!IS_ABC_KIT) {
        // Create SaveState instruction
        auto saveState = CreateSaveState(Opcode::SaveState, pc);
        // Create NullCheck instruction
        auto nullCheck =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);

        intrinsic->AppendInput(nullCheck);
        intrinsic->AppendInput(storeVal);
        intrinsic->AppendInput(saveState);
        AddInstruction(saveState);
        AddInstruction(nullCheck);
    } else {
        intrinsic->AppendInput(storeVal);
    }

    intrinsic->AddImm(GetGraph()->GetAllocator(), fieldId);
    intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

    intrinsic->SetMethodFirstInput();
    intrinsic->SetMethod(GetMethod());

    AddInstruction(intrinsic);
}

template void InstBuilder::BuildStObjByName<true>(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template void InstBuilder::BuildStObjByName<false>(const BytecodeInstruction *bcInst, compiler::DataType::Type type);

void InstBuilder::BuildIsNullValue(const BytecodeInstruction *bcInst)
{
    auto uniqueObjInst = graph_->GetOrCreateUniqueObjectInst();
    auto cmpInst = graph_->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), GetDefinitionAcc(),
                                             uniqueObjInst, DataType::REFERENCE, ConditionCode::CC_EQ);
    AddInstruction(cmpInst);
    UpdateDefinitionAcc(cmpInst);
}

template <bool IS_STRICT>
void InstBuilder::BuildEquals(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());

    Inst *obj1 = GetDefinition(bcInst->GetVReg(0));
    Inst *obj2 = GetDefinition(bcInst->GetVReg(1));

    auto intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_EQUALS;
    if constexpr (IS_STRICT) {
        intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRICT_EQUALS;
    }
#if defined(ENABLE_LIBABCKIT)
    if (GetGraph()->IsAbcKit()) {
        if constexpr (IS_STRICT) {
            intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_STRICT_EQUALS;
        } else {
            intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_EQUALS;
        }
    }
#else
#endif
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), intrinsic->RequireState() ? 3_I : 2_I);

    intrinsic->AppendInput(obj1);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AppendInput(obj2);
    intrinsic->AddInputType(DataType::REFERENCE);

    if (intrinsic->RequireState()) {
        // Create SaveState instruction
        auto *saveState = CreateSaveState(Opcode::SaveState, pc);
        intrinsic->AppendInput(saveState);
        intrinsic->AddInputType(DataType::NO_TYPE);
        AddInstruction(saveState);
    }

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

template void InstBuilder::BuildEquals<true>(const BytecodeInstruction *bcInst);
template void InstBuilder::BuildEquals<false>(const BytecodeInstruction *bcInst);

void InstBuilder::BuildTypeof(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinition(bcInst->GetVReg(0));

    RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_TYPEOF;
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc, intrinsicId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    intrinsic->AppendInput(obj);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AppendInput(saveState);
    intrinsic->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

void InstBuilder::BuildIstrue(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinition(bcInst->GetVReg(0));

    RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ISTRUE;
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 1_I);
    intrinsic->AppendInput(obj);
    intrinsic->AddInputType(DataType::REFERENCE);

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

template <bool IS_RANGE>
void InstBuilder::BuildCallByName(const BytecodeInstruction *bcInst)
{
    // Suppress BytecodeOptimizer if call.name was encountered
    if (GetGraph()->IsBytecodeOptimizer()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto startReg = bcInst->GetVReg(0);
    auto objRef = GetDefinition(startReg++);
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto argsCount = GetMethodArgumentsCount(methodId);
    auto retType = GetMethodReturnType(methodId);

    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto nullCheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc, objRef, saveState);

    ASSERT(saveState != nullptr);
    ASSERT(nullCheck != nullptr);

    ResolveVirtualInst *resolver = GetGraph()->CreateInstResolveByName(DataType::POINTER, pc, methodId, GetMethod());

    ASSERT(resolver != nullptr);

    resolver->SetInput(0, nullCheck);
    resolver->SetInput(1, saveState);

    CallInst *call = GetGraph()->CreateInstCallResolvedVirtual(retType, pc, methodId, nullptr);

    // + 1 because the first param is objRef
    // + 1 because of resolver
    // + 1 because CallResolvedVirtual has saveState as input (last)
    call->ReserveInputs(argsCount + 3_I);
    call->AllocateInputTypes(GetGraph()->GetAllocator(), argsCount + 3_I);
    call->AppendInput(resolver, DataType::POINTER);
    call->AppendInput(nullCheck, DataType::REFERENCE);

    if constexpr (IS_RANGE) {
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            call->AppendInput(GetDefinition(startReg), GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            call->AppendInput(GetDefinition(bcInst->GetVReg(i + 1)), GetMethodArgumentType(methodId, i));
        }
    }
    call->AppendInput(saveState, DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(resolver);
    AddInstruction(call);

    if (retType != DataType::VOID) {
        UpdateDefinitionAcc(call);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
}

template void InstBuilder::BuildCallByName<true>(const BytecodeInstruction *bcInst);
template void InstBuilder::BuildCallByName<false>(const BytecodeInstruction *bcInst);

void InstBuilder::BuildNullcheck(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinitionAcc();

    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(saveState);

    if (GetGraph()->IsBytecodeOptimizer()) {
        RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_NULLCHECK;
        auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);

        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
        intrinsic->AppendInput(obj);
        intrinsic->AddInputType(DataType::REFERENCE);
        intrinsic->AppendInput(saveState);
        intrinsic->AddInputType(DataType::NO_TYPE);

        AddInstruction(intrinsic);
    } else {
        auto nullCheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc, obj, saveState);
        AddInstruction(nullCheck);
        UpdateDefinitionAcc(nullCheck);
    }
}

}  // namespace ark::compiler
