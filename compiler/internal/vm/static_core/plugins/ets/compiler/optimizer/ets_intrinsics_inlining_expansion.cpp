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

#include "compiler/optimizer/optimizations/inlining.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {
/*
1. Let actualLength = LoadObject obj (std.core.Array.actualLength)
2. BoundsCheck actualLength index (forced de-opt if check fails)
3. Let buffer = LoadObject obj (std.core.Array.buffer)
4. Let value = LoadArray buffer, index
Returns value
*/
void Inlining::ExpandIntrinsicEscompatArrayGet(CallInst *callInst)
{
    int virtualIndex = callInst->GetOpcode() == Opcode::CallResolvedVirtual ? 1 : 0;
    auto bcAddr = callInst->GetPc();
    auto *obj = callInst->GetInput(0 + virtualIndex).GetInst()->CastToNullCheck();
    auto *index = callInst->GetInput(1 + virtualIndex).GetInst();
    auto *saveState = callInst->GetInput(2 + virtualIndex).GetInst()->CastToSaveState();

    auto *runtime = GetGraph()->GetRuntime();
    auto *arrayClass = runtime->GetEscompatArrayClass();
    auto *actualLengthField = runtime->GetEscompatArrayActualLength(arrayClass);
    auto *bufferField = runtime->GetEscompatArrayBuffer(arrayClass);

    auto *actualLength = GetGraph()->CreateInstLoadObject(
        DataType::INT32, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(actualLengthField), callInst->GetCallMethod()},
        actualLengthField, runtime->IsFieldVolatile(actualLengthField));
    BoundsCheckInst *boundsCheck =
        GetGraph()->CreateInstBoundsCheck(DataType::INT32, bcAddr, actualLength, index, saveState);
    boundsCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);

    auto buffer = GetGraph()->CreateInstLoadObject(
        DataType::REFERENCE, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(bufferField), callInst->GetCallMethod()},
        bufferField, runtime->IsFieldVolatile(bufferField));
    auto *result = GetGraph()->CreateInstLoadArray(DataType::REFERENCE, bcAddr, buffer, boundsCheck);

    callInst->InsertAfter(actualLength);
    actualLength->InsertAfter(boundsCheck);
    boundsCheck->InsertAfter(buffer);
    buffer->InsertAfter(result);

    callInst->ReplaceUsers(result);
}

/*
1. Let actualLength = LoadObject obj (std.core.Array.actualLength)
2. BoundsCheck actualLength index (forced de-opt if check fails)
3. Let buffer = LoadObject obj (std.core.Array.buffer)
4. StoreArray buffer, index, value
*/
void Inlining::ExpandIntrinsicEscompatArraySet(CallInst *callInst)
{
    auto bcAddr = callInst->GetPc();
    auto *obj = callInst->GetInput(0).GetInst()->CastToNullCheck();
    auto *index = callInst->GetInput(1).GetInst();
    auto *value = callInst->GetInput(2).GetInst();
    auto *saveState = callInst->GetInput(3).GetInst()->CastToSaveState();

    auto *runtime = GetGraph()->GetRuntime();
    auto *arrayClass = runtime->GetEscompatArrayClass();
    auto *actualLengthField = runtime->GetEscompatArrayActualLength(arrayClass);
    auto *bufferField = runtime->GetEscompatArrayBuffer(arrayClass);

    auto *actualLength = GetGraph()->CreateInstLoadObject(
        DataType::INT32, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(actualLengthField), callInst->GetCallMethod()},
        actualLengthField, runtime->IsFieldVolatile(actualLengthField));
    auto *boundsCheck = GetGraph()->CreateInstBoundsCheck(DataType::INT32, bcAddr, actualLength, index, saveState);
    boundsCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);

    auto buffer = GetGraph()->CreateInstLoadObject(
        DataType::REFERENCE, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(bufferField), callInst->GetCallMethod()},
        bufferField, runtime->IsFieldVolatile(bufferField));
    // true : needBarrier
    auto *storeValue = GetGraph()->CreateInstStoreArray(DataType::REFERENCE, bcAddr, buffer, boundsCheck, value, true);

    callInst->InsertAfter(actualLength);
    actualLength->InsertAfter(boundsCheck);
    boundsCheck->InsertAfter(buffer);
    buffer->InsertAfter(storeValue);
}
}  // namespace ark::compiler
