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

#ifndef ABCKIT_INST_BUILDER_INL_H
#define ABCKIT_INST_BUILDER_INL_H

#include "abckit_inst_builder.h"
#include <inst_builder_abckit_intrinsics.inc>

namespace ark::compiler {

template <bool IS_ACC_WRITE>
void AbcKitInstBuilder::AbcKitBuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadObjectIntrinsic<IS_ACC_WRITE>(bcInst, type);
}

template <bool IS_ACC_READ>
void AbcKitInstBuilder::AbcKitBuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreObjectIntrinsic<IS_ACC_READ>(bcInst, type);
}

void AbcKitInstBuilder::BuildNullcheck(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_NULL_CHECK);
}

void AbcKitInstBuilder::BuildIstrue(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinition(bcInst->GetVReg(0));

    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::INT32, pc,
                                                     RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_ETS_ISTRUE);

    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 1U);
    intrinsic->AppendInput(obj);
    intrinsic->AddInputType(DataType::REFERENCE);

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

void AbcKitInstBuilder::BuildTypeof(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_ETS_TYPEOF);
}

void AbcKitInstBuilder::BuildIsNullValue(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_IS_NULL_VALUE);
}

void AbcKitInstBuilder::BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadStaticIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreStaticIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadArrayIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildLoadConstArray(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_CONST_ARRAY);
}

void AbcKitInstBuilder::BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreArrayIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildNewArray(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_NEW_ARRAY);
}

void AbcKitInstBuilder::BuildNewObject(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_NEW_OBJECT);
}

void AbcKitInstBuilder::BuildInitObject(const BytecodeInstruction *bcInst, bool isRange)
{
    BuildAbcKitInitObjectIntrinsic(bcInst, isRange);
}

void AbcKitInstBuilder::BuildCheckCast(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_CHECK_CAST);
}

void AbcKitInstBuilder::BuildIsInstance(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_IS_INSTANCE);
}

void AbcKitInstBuilder::BuildThrow(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_THROW);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
void AbcKitInstBuilder::AbcKitBuildLoadFromPool(const BytecodeInstruction *bcInst)
{
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (OPCODE == Opcode::LoadType) {
        BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_TYPE);
        return;
    }

    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::LoadString) {
        BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING);
        return;
    }
    UNREACHABLE();
}

}  // namespace ark::compiler

#endif  // ABCKIT_INST_BUILDER_INL_H
