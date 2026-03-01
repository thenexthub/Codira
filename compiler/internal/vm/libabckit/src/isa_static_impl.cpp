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

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"

#include "libabckit/c/metadata_core.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/ir_static.h"
#include "scoped_timer.h"

#include "libabckit/src/macros.h"

#include <iostream>

namespace libabckit {

extern "C" AbckitInst *IcreateCmp(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateCmpStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateLoadString(AbckitGraph *graph, AbckitString *str)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr)
    LIBABCKIT_BAD_ARGUMENT(str, nullptr)
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateLoadStringStatic(graph, str);
}

extern "C" AbckitInst *IcreateReturn(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr)
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr)

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateReturnStatic(graph, input0);
}

extern "C" AbckitInst *IcreateIf(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                 AbckitIsaApiStaticConditionCode cc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr)
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr)
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr)

    if (cc == ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateIfStaticStatic(graph, input0, input1, cc);
}

extern "C" AbckitInst *IcreateNeg(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateNegStatic(graph, input0);
}

extern "C" AbckitInst *IcreateNot(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateNotStatic(graph, input0);
}

extern "C" AbckitInst *IcreateAdd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAddStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateSub(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateSubStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateMul(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateMulStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateDiv(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateDivStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateMod(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateModStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateShl(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);

    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateShlStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateShrStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateAShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAShrStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateAnd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAndStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateOr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateOrStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateXor(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateXorStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateCast(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    if (targetType == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateCastStatic(graph, input0, targetType);
}

extern "C" AbckitInst *GcreateNullPtr(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return GcreateNullPtrStatic(graph);
}

extern "C" AbckitInst *IcreateNewArray(AbckitGraph *graph, AbckitCoreClass *inputClass, AbckitInst *inputSize)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputClass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputSize, nullptr);

    LIBABCKIT_WRONG_CTX(graph, inputSize->graph, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputClass->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, inputClass->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateNewArrayStatic(graph, inputClass, inputSize);
}

extern "C" AbckitInst *IcreateNewObject(AbckitGraph *graph, AbckitCoreClass *inputClass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputClass, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputClass->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, inputClass->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateNewObjectStatic(graph, inputClass);
}

extern "C" AbckitInst *IcreateInitObject(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                         ... /* function params */)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputFunction, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputFunction->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, inputFunction->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_list args;
    va_start(args, argCount);

    auto *inst = IcreateInitObjectStatic(graph, inputFunction, argCount, args);

    va_end(args);

    return inst;
}

extern "C" AbckitInst *IcreateLoadObject(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field,
                                         AbckitTypeId returnTypeId)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (returnTypeId == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateLoadObjectStatic(graph, inputObj, field, returnTypeId);
}

extern "C" AbckitInst *IcreateStoreObject(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                          AbckitInst *value, AbckitTypeId typeId)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);
    LIBABCKIT_BAD_ARGUMENT(fieldId, nullptr);
    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, value->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateStoreObjectStatic(graph, inputObj, fieldId, value, typeId);
}

extern "C" AbckitInst *IcreateLoadArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                        AbckitTypeId returnTypeId)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(arrayRef, nullptr);
    LIBABCKIT_BAD_ARGUMENT(idx, nullptr);
    if (returnTypeId == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, arrayRef->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, idx->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateLoadArrayStatic(graph, arrayRef, idx, returnTypeId);
}

extern "C" AbckitInst *IcreateStoreArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                         AbckitTypeId valueTypeId)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(arrayRef, nullptr);
    LIBABCKIT_BAD_ARGUMENT(idx, nullptr);
    LIBABCKIT_BAD_ARGUMENT(value, nullptr);
    if (valueTypeId == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, arrayRef->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, idx->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, value->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateStoreArrayStatic(graph, arrayRef, idx, value, valueTypeId);
}

extern "C" AbckitInst *IcreateStoreArrayWide(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                             AbckitInst *value, AbckitTypeId valueTypeId)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(arrayRef, nullptr);
    LIBABCKIT_BAD_ARGUMENT(idx, nullptr);
    LIBABCKIT_BAD_ARGUMENT(value, nullptr);
    if (valueTypeId == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, arrayRef->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, idx->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, value->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateStoreArrayWideStatic(graph, arrayRef, idx, value, valueTypeId);
}

extern "C" AbckitInst *IcreateLenArray(AbckitGraph *graph, AbckitInst *arr)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(arr, nullptr);

    LIBABCKIT_WRONG_CTX(graph, arr->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateLenArrayStatic(graph, arr);
}

extern "C" AbckitInst *IcreateLoadConstArray(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateLoadConstArrayStatic(graph, literalArray);
}

extern "C" AbckitInst *IcreateCheckCast(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);
    LIBABCKIT_BAD_ARGUMENT(targetType, nullptr);
    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateCheckCastStatic(graph, inputObj, targetType);
}

extern "C" AbckitInst *IcreateIsInstance(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);
    LIBABCKIT_BAD_ARGUMENT(targetType, nullptr);

    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateIsInstanceStatic(graph, inputObj, targetType);
}

extern "C" AbckitInst *IcreateLoadNullValue(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateLoadNullValueStatic(graph);
}

extern "C" AbckitInst *IcreateReturnVoid(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    return IcreateReturnVoidStatic(graph);
}

extern "C" AbckitInst *IcreateEquals(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateEqualsStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateStrictEquals(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateStrictEqualsStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateCallStatic(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                         ... /* inputFunction params */)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputFunction, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputFunction->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, inputFunction->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_list args;
    va_start(args, argCount);

    auto *inst = IcreateCallStaticStatic(graph, inputFunction, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateCallVirtual(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreFunction *inputFunction,
                                          size_t argCount, ... /* inputFunction params */)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputFunction, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);
    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputFunction->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, inputFunction->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_list args;
    va_start(args, argCount);

    auto *inst = IcreateCallVirtualStatic(graph, inputObj, inputFunction, argCount, args);
    va_end(args);

    return inst;
}

extern "C" AbckitInst *IcreateAddI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAddIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateSubI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateSubIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateMulI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateMulIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateDivI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateDivIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateModI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateModIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateShlI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateShlIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateShrIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateAShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAShrIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateAndI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateAndIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateOrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateOrIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateXorI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateXorIStatic(graph, input0, imm);
}

extern "C" AbckitInst *IcreateThrow(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateThrowStatic(graph, acc);
}

extern "C" AbckitIsaApiStaticOpcode IgetOpcode(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, ABCKIT_ISA_API_STATIC_OPCODE_INVALID);

    return IgetOpcodeStaticStatic(inst);
}

extern "C" AbckitCoreClass *IgetClass(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetClassStatic(inst);
}

extern "C" void IsetClass(AbckitInst *inst, AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(klass);

    IsetClassStatic(inst, klass);
}

extern "C" AbckitIsaApiStaticConditionCode IgetConditionCode(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);

    if (IgetOpcode(inst) != ABCKIT_ISA_API_STATIC_OPCODE_IF) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Trying to get condition code not from 'If' instruction\n";
        return ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE;
    }

    return IgetConditionCodeStaticStatic(inst);
}

extern "C" void IsetConditionCode(AbckitInst *inst, AbckitIsaApiStaticConditionCode cc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    if (cc == ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    if (IgetOpcode(inst) != ABCKIT_ISA_API_STATIC_OPCODE_IF) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Trying to get condition code not from 'If' instruction\n";
        return;
    }

    bool ccDynamicResitiction =
        !((cc == ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE) || (cc == ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ));
    if (IsDynamic(inst->graph->function->owningModule->target) && ccDynamicResitiction) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Wrong condition code set for dynamic if\n";
        return;
    }

    IsetConditionCodeStaticStatic(inst, cc);
}

extern "C" void IsetTargetType(AbckitInst *inst, AbckitTypeId t)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    if (t == ABCKIT_TYPE_ID_INVALID) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    IsetTargetTypeStatic(inst, t);
}

extern "C" AbckitTypeId IgetTargetType(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, AbckitTypeId::ABCKIT_TYPE_ID_INVALID);

    return IgetTargetTypeStatic(inst);
}

extern "C" AbckitInst *IcreateIsUndefined(AbckitGraph *graph, AbckitInst *inputObj)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);

    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateIsUndefinedStatic(graph, inputObj);
}

extern "C" AbckitInst *IcreateNullCheck(AbckitGraph *graph, AbckitInst *inputObj)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(inputObj, nullptr);

    LIBABCKIT_WRONG_CTX(graph, inputObj->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::STATIC, nullptr);
    return IcreateNullCheckStatic(graph, inputObj);
}

AbckitIsaApiStatic g_isaApiStaticImpl = {

    IgetClass,
    IsetClass,
    IgetConditionCode,
    IsetConditionCode,
    IgetOpcode,
    IsetTargetType,
    IgetTargetType,

    IcreateCmp,
    IcreateLoadString,
    IcreateReturn,
    IcreateIf,
    IcreateNeg,
    IcreateNot,
    IcreateAdd,
    IcreateSub,
    IcreateMul,
    IcreateDiv,
    IcreateMod,
    IcreateShl,
    IcreateShr,
    IcreateAShr,
    IcreateAnd,
    IcreateOr,
    IcreateXor,
    IcreateCast,
    GcreateNullPtr,
    IcreateNewArray,
    IcreateNewObject,
    IcreateInitObject,
    IcreateLoadObject,
    IcreateStoreObject,
    IcreateLoadArray,
    IcreateStoreArray,
    IcreateStoreArrayWide,
    IcreateLenArray,
    IcreateLoadConstArray,
    IcreateCheckCast,
    IcreateIsInstance,
    IcreateLoadNullValue,
    IcreateReturnVoid,
    IcreateEquals,
    IcreateStrictEquals,
    IcreateCallStatic,
    IcreateCallVirtual,
    IcreateAddI,
    IcreateSubI,
    IcreateMulI,
    IcreateDivI,
    IcreateModI,
    IcreateShlI,
    IcreateShrI,
    IcreateAShrI,
    IcreateAndI,
    IcreateOrI,
    IcreateXorI,
    IcreateThrow,
    IcreateIsUndefined,
    IcreateNullCheck,
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitIsaApiStatic const *AbckitGetIsaApiStaticImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockIsaApiStaticImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_isaApiStaticImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
