/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

bool EmitArrayCopyTo(Inst *inst);
bool EmitArrayCopyWithin(Inst *inst);
bool EmitStdStringSubstring(Inst *inst);
bool EmitStringBuilderAppendBool(Inst *inst);
bool EmitStringBuilderAppendChar(Inst *inst);
bool EmitStringBuilderAppendByte(Inst *inst);
bool EmitStringBuilderAppendShort(Inst *inst);
bool EmitStringBuilderAppendInt(Inst *inst);
bool EmitStringBuilderAppendLong(Inst *inst);
bool EmitStringBuilderAppendString(Inst *inst);
bool EmitStringBuilderAppendStrings(Inst *inst);
bool EmitStringBuilderToString(Inst *inst);

llvm::Value *CreateStringBuilderAppendLong(Inst *inst);
llvm::Value *CreateStringBuilderAppendString(Inst *inst);
llvm::Value *CreateStringBuilderAppendStrings(Inst *inst);

void StringBuilderAppendStringNull(Inst *inst, llvm::PHINode *result, llvm::BasicBlock *contBb);
void StringBuilderAppendStringMain(Inst *inst, llvm::Value *sbIndexOffset, llvm::Value *sbIndex, llvm::Value *sbBuffer,
                                   llvm::BasicBlock *contBb);
bool EmitDoubleToStringDecimal(Inst *inst);
bool EmitStringTrim(Inst *inst);
bool EmitStringTrimLeft(Inst *inst);
bool EmitStringTrimRight(Inst *inst);
bool EmitCharIsWhiteSpace(Inst *inst);
bool EmitStringStartsWith(Inst *inst);
bool EmitStringEndsWith(Inst *inst);
bool EmitStringGetBytesTlab(Inst *inst);
bool EmitStringIndexOf(Inst *inst);
bool EmitStringIndexOfAfter(Inst *inst);
bool EmitStringLastIndexOf(Inst *inst);
bool EmitStringFromCharCode(Inst *inst);
bool EmitStringFromCharCodeSingle(Inst *inst);
bool EmitStringRepeat(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_REPEAT, 2U);
}
bool EmitStringCharAt(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_CHAR_AT, 2U);
}
bool EmitInt8ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT8_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitInt16ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT16_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitInt32ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT32_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitBigInt64ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_INT64_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitFloat32ArrayFillInternal(Inst *inst);
bool EmitFloat64ArrayFillInternal(Inst *inst);
bool EmitFloatArrayFillInternal(Inst *inst, RuntimeInterface::EntrypointId eid, llvm::IntegerType *bitcastType);
bool EmitUInt8ClampedArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT8_CLAMPED_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitUInt8ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT8_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitUInt16ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT16_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitUInt32ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT32_ARRAY_FILL_INTERNAL, 4U);
}
bool EmitBigUInt64ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_U_INT64_ARRAY_FILL_INTERNAL, 4U);
}

bool EmitInt8ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT8_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitInt16ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT16_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitInt32ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT32_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitBigInt64ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_INT64_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitFloat32ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::FLOAT32_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitFloat64ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::FLOAT64_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitUint8ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::UINT8_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitUint8ClampedArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::UINT8_CLAMPED_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitUint16ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::UINT16_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitUint32ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::UINT32_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}
bool EmitBigUint64ArraySetValuesFromArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_UINT64_ARRAY_SET_VALUES_FROM_ARRAY, 2U);
}

bool EmitGetHashCodeByValue(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::GET_HASH_CODE_BY_VALUE_FAST_PATH, 1U);
}

bool EmitEscompatArrayIsPlatformArray(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::ESCOMPAT_ARRAY_IS_PLATFORM_ARRAY_FAST, 1U);
}

bool EmitArrayFastCopyToRef(Inst *inst);

bool EmitEscompatArrayReverse(Inst *inst);

bool EmitSameValueZero(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::SAME_VALUE_ZERO_FAST_PATH, 2U);
}

bool EmitStringConcat2(Inst *inst);
bool EmitStringConcat3(Inst *inst);
bool EmitStringConcat4(Inst *inst);

bool EmitEtsStringEquals(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::ETS_STRING_EQUALS, 2U);
}

llvm::Value *CreateStringToCase(Inst *inst, RuntimeInterface::EntrypointId fastPathEntrypointId,
                                RuntimeInterface::EntrypointId slowPathEntrypointId);

bool EmitStringToLowerCase(Inst *inst);

bool EmitStringToUpperCase(Inst *inst);

bool EmitStringToLocaleLowerCase(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_TO_LOCALE_LOWER_CASE_TLAB, 2U);
}

bool EmitStringToLocaleUpperCase(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_TO_LOCALE_UPPER_CASE_TLAB, 2U);
}

bool EmitDefaultLocaleSupportsFastLatinCaseConversion(Inst *inst);
