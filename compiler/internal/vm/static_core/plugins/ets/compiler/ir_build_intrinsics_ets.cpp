/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "optimizer/code_generator/encode.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "runtime/include/coretypes/string.h"

namespace ark::compiler {
/*
    FMOV: f -> n
    return CMP(AND(SHR(n, FP_FRACT_SIZE), FP_EXP_MASK), FP_EXP_MASK)

    1. bitcast f1
    2. shr v1, FP_FRACT_SIZE
    3. and v2, FP_EXP_MASK
    4. compare v3, FP_EXP_MASK

    fraction size is 23 bits for floats and 52 bits for doubles
    exponent mask is 0xff (8 bits) for floats and 0x7ff (11 bits) for doubles
 */
void InstBuilder::BuildIsFiniteIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto type = GetMethodArgumentType(methodId, 0);
    auto itype = type == DataType::FLOAT32 ? DataType::INT32 : DataType::INT64;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fpFractSize = type == DataType::FLOAT32 ? 23 : 52;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fpExpMask = type == DataType::FLOAT32 ? 0xff : 0x7ff;

    auto bitcast =
        GetGraph()->CreateInstBitcast(itype, GetPc(bcInst->GetAddress()), GetArgDefinition(bcInst, 0, accRead), type);
    auto shift =
        GetGraph()->CreateInstShr(itype, GetPc(bcInst->GetAddress()), bitcast, FindOrCreateConstant(fpFractSize));
    auto mask = GetGraph()->CreateInstAnd(itype, GetPc(bcInst->GetAddress()), shift, FindOrCreateConstant(fpExpMask));
    auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), mask,
                                             FindOrCreateConstant(fpExpMask), itype, ConditionCode::CC_NE);

    AddInstruction(bitcast, shift, mask, cmp);
    UpdateDefinitionAcc(cmp);
}

void InstBuilder::BuildStdRuntimeEquals(const BytecodeInstruction *bcInst, bool accRead)
{
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), GetArgDefinition(bcInst, 0, accRead),
                                      GetArgDefinition(bcInst, 1, accRead), DataType::REFERENCE, ConditionCode::CC_EQ);
    AddInstruction(cmp);
    UpdateDefinitionAcc(cmp);
}

void InstBuilder::BuildSignbitIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bitcast = GetGraph()->CreateInstBitcast(DataType::INT64, GetPc(bcInst->GetAddress()),
                                                 GetArgDefinition(bcInst, 0, accRead), DataType::FLOAT64);
    constexpr auto SHIFT = 63;
    auto res =
        GetGraph()->CreateInstShr(DataType::INT64, GetPc(bcInst->GetAddress()), bitcast, FindOrCreateConstant(SHIFT));
    AddInstruction(bitcast, res);
    UpdateDefinitionAcc(res);
}

void InstBuilder::BuildEscompatArrayGetUnsafeIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);
    auto *pos = GetArgDefinition(bcInst, 1, accRead);

    auto *runtime = GetGraph()->GetRuntime();
    auto *arrayClass = runtime->GetEscompatArrayClass();
    auto *bufferField = runtime->GetEscompatArrayBuffer(arrayClass);

    auto buffer = GetGraph()->CreateInstLoadObject(
        DataType::REFERENCE, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(bufferField), GetGraph()->GetMethod()},
        bufferField, runtime->IsFieldVolatile(bufferField));
    AddInstruction(buffer);
    auto *result = GetGraph()->CreateInstLoadArray(DataType::REFERENCE, bcAddr, buffer, pos);
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

void InstBuilder::BuildEscompatArraySetUnsafeIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);
    auto *pos = GetArgDefinition(bcInst, 1, accRead);
    auto *value = GetArgDefinition(bcInst, 2, accRead);

    auto *runtime = GetGraph()->GetRuntime();
    auto *arrayClass = runtime->GetEscompatArrayClass();
    auto *bufferField = runtime->GetEscompatArrayBuffer(arrayClass);

    auto buffer = GetGraph()->CreateInstLoadObject(
        DataType::REFERENCE, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(bufferField), GetGraph()->GetMethod()},
        bufferField, runtime->IsFieldVolatile(bufferField));
    AddInstruction(buffer);
    auto *result = GetGraph()->CreateInstStoreArray(DataType::REFERENCE, bcAddr, buffer, pos, value, true);
    AddInstruction(result);
}

void InstBuilder::BuildEscompatArrayGetBufferIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);

    auto *runtime = GetGraph()->GetRuntime();
    auto *arrayClass = runtime->GetEscompatArrayClass();
    auto *bufferField = runtime->GetEscompatArrayBuffer(arrayClass);

    auto buffer = GetGraph()->CreateInstLoadObject(
        DataType::REFERENCE, bcAddr, obj, TypeIdMixin {runtime->GetFieldId(bufferField), GetGraph()->GetMethod()},
        bufferField, runtime->IsFieldVolatile(bufferField));
    AddInstruction(buffer);
    UpdateDefinitionAcc(buffer);
}

void InstBuilder::BuildUint8ClampedArraySetIntrinsic(const BytecodeInstruction *bcInst,
                                                     ark::compiler::DataType::Type type, bool accRead)
{
    constexpr auto MAX_VALUE = 255;
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    auto *graph = GetGraph();
    auto *clamped0 = graph->CreateInstMax(DataType::INT32, bcAddr);
    clamped0->SetInput(0, value);
    clamped0->SetInput(1, graph->FindOrCreateConstant(0));
    AddInstruction(clamped0);
    auto *clamped = graph->CreateInstMin(DataType::INT32, bcAddr);
    clamped->SetInput(0, clamped0);
    clamped->SetInput(1, graph->FindOrCreateConstant(MAX_VALUE));
    AddInstruction(clamped);
    BuildTypedUnsignedArraySetIntrinsic(bcInst, clamped, type, accRead);
}

void InstBuilder::BuildTypedArraySetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type, bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset(bcInst, type, accRead, true);
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    BuildTypedArraySet(bcInst, value, type, loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildTypedUnsignedArraySetIntrinsic(const BytecodeInstruction *bcInst, Inst *value,
                                                      DataType::Type type, bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset<true>(bcInst, type, accRead, true);
    BuildTypedArraySet(bcInst, value, type, loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildTypedUnsignedArraySetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type,
                                                      bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    BuildTypedUnsignedArraySetIntrinsic(bcInst, value, type, accRead);
}

void InstBuilder::BuildTypedArraySet(const BytecodeInstruction *bcInst, Inst *value, ark::compiler::DataType::Type type,
                                     Inst *loadDataInst, Inst *dataOffsetInst)
{
    const size_t valueIndex = 2;
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *storeInst = GetGraph()->CreateInstStoreNative(type, bcAddr);
    storeInst->SetInput(0, loadDataInst);
    storeInst->SetInput(1, dataOffsetInst);
    storeInst->SetInput(valueIndex, value);
    AddInstruction(storeInst);
}

void InstBuilder::BuildBigInt64ArraySetIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset(bcInst, DataType::INT64, accRead, true);
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    if (value->GetType() != DataType::INT64) {
        auto bcAddr = GetPc(bcInst->GetAddress());
        auto *cast = GetGraph()->CreateInstCast(DataType::INT64, bcAddr, value, value->GetType());
        AddInstruction(cast);
        value = cast;
    }
    BuildTypedArraySet(bcInst, value, DataType::INT64, loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildBigUint64ArraySetIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto [loadDataInst, dataOffsetInst] =
        BuildTypedArrayLoadDataAndOffset<true>(bcInst, DataType::INT64, accRead, true);
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    if (value->GetType() != DataType::INT64) {
        auto bcAddr = GetPc(bcInst->GetAddress());
        auto *cast = GetGraph()->CreateInstCast(DataType::INT64, bcAddr, value, value->GetType());
        AddInstruction(cast);
        value = cast;
    }
    BuildTypedArraySet(bcInst, value, DataType::INT64, loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildTypedArrayGetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type, bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset(bcInst, type, accRead, true);
    auto *loadInst = BuildTypedArrayGet(bcInst, type, loadDataInst, dataOffsetInst);
    auto result = GetGraph()->CreateInstCast(DataType::FLOAT64, bcAddr, loadInst, loadInst->GetType());
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

void InstBuilder::BuildTypedArrayGetUnsafeIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type,
                                                    bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset(bcInst, type, accRead, false);
    auto *loadInst = BuildTypedArrayGet(bcInst, type, loadDataInst, dataOffsetInst);
    UpdateDefinitionAcc(loadInst);
}

void InstBuilder::BuildTypedUnsignedArrayGetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type,
                                                      bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset<true>(bcInst, type, accRead, true);
    auto *loadInst = BuildTypedArrayGet(bcInst, type, loadDataInst, dataOffsetInst);
    auto result = GetGraph()->CreateInstCast(DataType::FLOAT64, bcAddr, loadInst, loadInst->GetType());
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

void InstBuilder::BuildTypedUnsignedArrayGetUnsafeIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type,
                                                            bool accRead)
{
    ASSERT(type != DataType::INT64);
    auto [loadDataInst, dataOffsetInst] = BuildTypedArrayLoadDataAndOffset<true>(bcInst, type, accRead, false);
    auto *loadInst = BuildTypedArrayGet(bcInst, type, loadDataInst, dataOffsetInst);
    UpdateDefinitionAcc(loadInst);
}

void InstBuilder::BuildUint32ArrayGetUnsafeIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto [loadDataInst, dataOffsetInst] =
        BuildTypedArrayLoadDataAndOffset<true>(bcInst, DataType::UINT32, accRead, false);
    auto *loadInst = BuildTypedArrayGet(bcInst, DataType::UINT32, loadDataInst, dataOffsetInst);
    auto result = GetGraph()->CreateInstCast(DataType::INT64, bcAddr, loadInst, loadInst->GetType());
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

Inst *InstBuilder::BuildTypedArrayGet(const BytecodeInstruction *bcInst, DataType::Type type, Inst *loadDataInst,
                                      Inst *dataOffsetInst)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *loadInst = GetGraph()->CreateInstLoadNative(type, bcAddr);
    loadInst->SetInput(0, loadDataInst);
    loadInst->SetInput(1, dataOffsetInst);
    AddInstruction(loadInst);
    return loadInst;
}

void InstBuilder::BuildBigInt64ArrayGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead, bool needBoundCheck)
{
    auto [loadDataInst, dataOffsetInst] =
        BuildTypedArrayLoadDataAndOffset(bcInst, DataType::INT64, accRead, needBoundCheck);
    auto *loadInst = BuildTypedArrayGet(bcInst, DataType::INT64, loadDataInst, dataOffsetInst);
    UpdateDefinitionAcc(loadInst);
}

void InstBuilder::BuildBigUint64ArrayGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead, bool needBoundCheck)
{
    auto [loadDataInst, dataOffsetInst] =
        BuildTypedArrayLoadDataAndOffset<true>(bcInst, DataType::INT64, accRead, needBoundCheck);
    auto *loadInst = BuildTypedArrayGet(bcInst, DataType::INT64, loadDataInst, dataOffsetInst);
    UpdateDefinitionAcc(loadInst);
}

/*
    1. typedArray       arg 0 -> (v4, v3, v2)
    2. pos              arg 1 -> (v14, v6, v3)
    3. SaveState        v0, v1 -> (v9, v6, v4)
    4. NullCheck        v0, v3 -> (v10, v7, v5)
    5. LoadObject       v4 -> (v6), TYPED_ARRAY_LENGTH_INT_OFFSET
    6. BoundsCheck D    v5, v1, v3
    7. LoadObject       v4 -> (v_8), TYPED_ARRAY_BUFFER_OFFSET
    8. LoadObject       v7 -> (v9), ARRAY_BUFFER_DATA_OFFSET
    9. NullCheck D      v_8, v3 -> (v16)
    10. LoadObject      v4 -> (v12), TYPED_ARRAY_BUFFER_BYTE_OFFSET_OFFSET
    11. Add             v11, v10 -> (v15), ARRAY_DATA_OFFSET
    12. Shl             v0, v13 -> (v15)
    13. Add             v14, v12 -> (v16)
    Return              v9, v15
*/
// CC-OFFNXT(huge_depth[C++]) solid logic
template <bool UNSIGNED>
std::tuple<Inst *, Inst *> InstBuilder::BuildTypedArrayLoadDataAndOffset(const BytecodeInstruction *bcInst,
                                                                         DataType::Type type, bool accRead,
                                                                         bool needBoundCheck)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);
    auto *pos = GetArgDefinition(bcInst, 1, accRead);
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    AddInstruction(saveState);
    auto *graph = GetGraph();
    auto *nullCheck = graph->CreateInstNullCheck(DataType::REFERENCE, bcAddr, obj, saveState);
    AddInstruction(nullCheck);

    auto *runtime = graph->GetRuntime();
    auto calleeMethodId = runtime->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto *calleeMethod = runtime->GetMethodById(GetMethod(), calleeMethodId);

    // TypedArray helpers
    auto *typedArrayClass = runtime->GetClass(calleeMethod);
    auto *bufferField = runtime->GetEscompatTypedArrayBuffer(typedArrayClass);
    auto *byteOffsetField = UNSIGNED ? runtime->GetEscompatUnsignedTypedArrayByteOffsetInt(typedArrayClass)
                                     : runtime->GetEscompatTypedArrayByteOffset(typedArrayClass);
    if (needBoundCheck) {
        auto *lengthIntField = runtime->GetEscompatTypedArrayLengthInt(typedArrayClass);
        auto *loadLengthInst = graph->CreateInstLoadObject(
            DataType::INT32, bcAddr, nullCheck, TypeIdMixin {runtime->GetFieldId(lengthIntField), calleeMethod},
            lengthIntField, runtime->IsFieldVolatile(lengthIntField));
        AddInstruction(loadLengthInst);
        BuildTypedArrayBoundsCheck(pos, loadLengthInst, bcAddr, saveState);
    }

    // ArrayBuffer helpers
    auto *arrayBufferClass = runtime->GetEscompatArrayBufferClass();
    // .data field (FixedArray<byte>)
    auto *dataField = runtime->GetEscompatArrayBufferManagedData(arrayBufferClass);
    // offset to real data inside .data (FixedArray<byte>)
    auto arrayDataOffset = runtime->GetArrayDataOffset(graph->GetArch());

    auto *loadBufferInst = graph->CreateInstLoadObject(DataType::REFERENCE, bcAddr, nullCheck,
                                                       TypeIdMixin {runtime->GetFieldId(bufferField), calleeMethod},
                                                       bufferField, runtime->IsFieldVolatile(bufferField));
    AddInstruction(loadBufferInst);

    // Load ArrayBuffer.data
    Inst *loadDataInst = graph->CreateInstLoadObject(DataType::REFERENCE, bcAddr, loadBufferInst,
                                                     TypeIdMixin {runtime->GetFieldId(dataField), calleeMethod},
                                                     dataField, runtime->IsFieldVolatile(dataField));
    AddInstruction(loadDataInst);

    BuildTypedArrayDetachedManagedDataCheck(&loadDataInst, bcAddr, saveState);

    // Load position in data
    auto *loadByteOffsetInst = graph->CreateInstLoadObject(
        DataType::INT32, bcAddr, nullCheck, TypeIdMixin {runtime->GetFieldId(byteOffsetField), calleeMethod},
        byteOffsetField, runtime->IsFieldVolatile(byteOffsetField));
    AddInstruction(loadByteOffsetInst);

    // Add offset to real data in ArrayBuffer.data
    auto *loadDataOffsetInst =
        graph->CreateInstAdd(DataType::INT32, bcAddr, graph->FindOrCreateConstant(arrayDataOffset), loadByteOffsetInst);
    AddInstruction(loadDataOffsetInst);

    // Convert index in data to correct byte offset
    auto scale = DataType::ShiftByType(type, graph->GetArch());
    auto *scaledPos = graph->CreateInstShl(DataType::INT32, bcAddr, pos, graph->FindOrCreateConstant(scale));
    AddInstruction(scaledPos);
    auto *dataOffsetInst = graph->CreateInstAdd(DataType::INT32, bcAddr, scaledPos, loadDataOffsetInst);
    AddInstruction(dataOffsetInst);

    return std::make_tuple(loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildTypedArrayDetachedDataCheck(Inst **dataInst, size_t bcAddr, SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    auto *zeroCheck = graph->CreateInstZeroCheck(DataType::INT64, bcAddr, *dataInst, saveState);
    zeroCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    AddInstruction(zeroCheck);
    *dataInst = zeroCheck;
}

void InstBuilder::BuildTypedArrayDetachedManagedDataCheck(Inst **dataInst, size_t bcAddr, SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    auto *nullCheck = graph->CreateInstNullCheck(DataType::REFERENCE, bcAddr, *dataInst, saveState);
    nullCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    AddInstruction(nullCheck);
    *dataInst = nullCheck;
}

void InstBuilder::BuildTypedArrayBoundsCheck(Inst *posInst, Inst *lengthInst, size_t bcAddr, SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    auto *boundsCheck = graph->CreateInstBoundsCheck(DataType::INT32, bcAddr, lengthInst, posInst, saveState);
    boundsCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    AddInstruction(boundsCheck);
}

template <bool LOAD>
void InstBuilder::BuildUnsafeIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    /* ensure the boot context of the caller */
    if (!IsInBootContext()) {
        failed_ = true;
        return;
    }

    auto bcAddr = GetPc(bcInst->GetAddress());
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);

    auto type = LOAD ? GetMethodReturnType(methodId) : GetMethodArgumentType(methodId, 1);
    ASSERT(type == DataType::INT8 || type == DataType::INT16 || type == DataType::INT32 || type == DataType::INT64 ||
           type == DataType::BOOL || type == DataType::FLOAT32 || type == DataType::FLOAT64);

    auto raw = GetArgDefinition(bcInst, 0, accRead);
    auto addr = GetGraph()->CreateInstCast(DataType::POINTER, bcAddr, raw, DataType::INT64);
    auto zero = FindOrCreateConstant(0);
    AddInstruction(addr);

    if (LOAD) {
        auto ld = GetGraph()->CreateInstLoadNative(type, bcAddr, addr, zero);
        AddInstruction(ld);
        UpdateDefinitionAcc(ld);
    } else {
        auto val = GetArgDefinition(bcInst, 1, accRead);
        auto st = GetGraph()->CreateInstStoreNative(type, bcAddr, addr, zero, val);
        AddInstruction(st);
    }
}

void InstBuilder::BuildUnsafeLoadIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    BuildUnsafeIntrinsic<true>(bcInst, accRead);
}

void InstBuilder::BuildUnsafeStoreIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    BuildUnsafeIntrinsic<false>(bcInst, accRead);
}

}  // namespace ark::compiler
