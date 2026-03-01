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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/optimizations/const_folding.h"
#include "libarkbase/utils/math_helpers.h"

#include <cmath>
#include <map>

namespace ark::compiler {
template <class T>
uint64_t ConvertIntToInt(T value, DataType::Type targetType)
{
    switch (targetType) {
        case DataType::BOOL:
            return static_cast<uint64_t>(static_cast<bool>(value));
        case DataType::UINT8:
            return static_cast<uint64_t>(static_cast<uint8_t>(value));
        case DataType::INT8:
            return static_cast<uint64_t>(static_cast<int8_t>(value));
        case DataType::UINT16:
            return static_cast<uint64_t>(static_cast<uint16_t>(value));
        case DataType::INT16:
            return static_cast<uint64_t>(static_cast<int16_t>(value));
        case DataType::UINT32:
            return static_cast<uint64_t>(static_cast<uint32_t>(value));
        case DataType::INT32:
            return static_cast<uint64_t>(static_cast<int32_t>(value));
        case DataType::UINT64:
            return static_cast<uint64_t>(value);
        case DataType::INT64:
            return static_cast<uint64_t>(static_cast<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class T>
T ConvertIntToFloat(uint64_t value, DataType::Type sourceType)
{
    switch (sourceType) {
        case DataType::BOOL:
            return static_cast<T>(static_cast<bool>(value));
        case DataType::UINT8:
            return static_cast<T>(static_cast<uint8_t>(value));
        case DataType::INT8:
            return static_cast<T>(static_cast<int8_t>(value));
        case DataType::UINT16:
            return static_cast<T>(static_cast<uint16_t>(value));
        case DataType::INT16:
            return static_cast<T>(static_cast<int16_t>(value));
        case DataType::UINT32:
            return static_cast<T>(static_cast<uint32_t>(value));
        case DataType::INT32:
            return static_cast<T>(static_cast<int32_t>(value));
        case DataType::UINT64:
            return static_cast<T>(value);
        case DataType::INT64:
            return static_cast<T>(static_cast<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class To, class From>
To ConvertFloatToInt(From value)
{
    To res;

    constexpr To MIN_INT = std::numeric_limits<To>::min();
    constexpr To MAX_INT = std::numeric_limits<To>::max();
    const auto floatMinInt = static_cast<From>(MIN_INT);
    const auto floatMaxInt = static_cast<From>(MAX_INT);

    if (value > floatMinInt) {
        if (value < floatMaxInt) {
            res = static_cast<To>(value);
        } else {
            res = MAX_INT;
        }
    } else if (std::isnan(value)) {
        res = 0;
    } else {
        res = MIN_INT;
    }

    return static_cast<To>(res);
}

template <class From>
uint64_t ConvertFloatToInt(From value, DataType::Type targetType)
{
    ASSERT(DataType::GetCommonType(targetType) == DataType::INT64);
    switch (targetType) {
        case DataType::BOOL:
            return static_cast<uint64_t>(ConvertFloatToInt<bool>(value));
        case DataType::UINT8:
            return static_cast<uint64_t>(ConvertFloatToInt<uint8_t>(value));
        case DataType::INT8:
            return static_cast<uint64_t>(ConvertFloatToInt<int8_t>(value));
        case DataType::UINT16:
            return static_cast<uint64_t>(ConvertFloatToInt<uint16_t>(value));
        case DataType::INT16:
            return static_cast<uint64_t>(ConvertFloatToInt<int16_t>(value));
        case DataType::UINT32:
            return static_cast<uint64_t>(ConvertFloatToInt<uint32_t>(value));
        case DataType::INT32:
            return static_cast<uint64_t>(ConvertFloatToInt<int32_t>(value));
        case DataType::UINT64:
            return ConvertFloatToInt<uint64_t>(value);
        case DataType::INT64:
            return static_cast<uint64_t>(ConvertFloatToInt<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class From>
uint64_t ConvertFloatToIntDyn(From value, RuntimeInterface *runtime, size_t bits)
{
    return runtime->DynamicCastDoubleToInt(static_cast<double>(value), bits);
}

ConstantInst *ConstFoldingCreateIntConst(Inst *inst, uint64_t value, bool isLiteralData)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType()) && !isLiteralData) {
        return graph->FindOrCreateConstant<uint32_t>(value);
    }
    return graph->FindOrCreateConstant(value);
}

template <typename T>
ConstantInst *ConstFoldingCreateConst(Inst *inst, ConstantInst *cnst, bool isLiteralData = false)
{
    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(static_cast<T>(cnst->GetIntValue()), inst->GetType()),
                                      isLiteralData);
}

ConstantInst *ConstFoldingCastInt2Int(Inst *inst, ConstantInst *cnst)
{
    switch (inst->GetInputType(0)) {
        case DataType::BOOL:
            return ConstFoldingCreateConst<bool>(inst, cnst);
        case DataType::UINT8:
            return ConstFoldingCreateConst<uint8_t>(inst, cnst);
        case DataType::INT8:
            return ConstFoldingCreateConst<int8_t>(inst, cnst);
        case DataType::UINT16:
            return ConstFoldingCreateConst<uint16_t>(inst, cnst);
        case DataType::INT16:
            return ConstFoldingCreateConst<int16_t>(inst, cnst);
        case DataType::UINT32:
            return ConstFoldingCreateConst<uint32_t>(inst, cnst);
        case DataType::INT32:
            return ConstFoldingCreateConst<int32_t>(inst, cnst);
        case DataType::UINT64:
            return ConstFoldingCreateConst<uint64_t>(inst, cnst);
        case DataType::INT64:
            return ConstFoldingCreateConst<int64_t>(inst, cnst);
        default:
            return nullptr;
    }
}

ConstantInst *ConstFoldingCastIntConst(Graph *graph, Inst *inst, ConstantInst *cnst, bool isLiteralData = false)
{
    auto instType = DataType::GetCommonType(inst->GetType());
    if (instType == DataType::INT64) {
        // INT -> INT
        return ConstFoldingCastInt2Int(inst, cnst);
    }
    if (instType == DataType::FLOAT32) {
        // INT -> FLOAT
        if (graph->IsBytecodeOptimizer() && !isLiteralData) {
            return nullptr;
        }
        return graph->FindOrCreateConstant(ConvertIntToFloat<float>(cnst->GetIntValue(), inst->GetInputType(0)));
    }
    if (instType == DataType::FLOAT64) {
        // INT -> DOUBLE
        return graph->FindOrCreateConstant(ConvertIntToFloat<double>(cnst->GetIntValue(), inst->GetInputType(0)));
    }
    return nullptr;
}

ConstantInst *ConstFoldingCastConst(Inst *inst, Inst *input, bool isLiteralData)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cnst = static_cast<ConstantInst *>(input);
    auto instType = DataType::GetCommonType(inst->GetType());
    if (cnst->GetType() == DataType::INT32 || cnst->GetType() == DataType::INT64) {
        return ConstFoldingCastIntConst(graph, inst, cnst);
    }
    if (cnst->GetType() == DataType::FLOAT32) {
        if (graph->IsBytecodeOptimizer() && !isLiteralData) {
            return nullptr;
        }
        if (instType == DataType::INT64) {
            // FLOAT->INT
            return graph->FindOrCreateConstant(ConvertFloatToInt(cnst->GetFloatValue(), inst->GetType()));
        }
        if (instType == DataType::FLOAT32) {
            // FLOAT -> FLOAT
            return cnst;
        }
        if (instType == DataType::FLOAT64) {
            // FLOAT -> DOUBLE
            return graph->FindOrCreateConstant(static_cast<double>(cnst->GetFloatValue()));
        }
    } else if (cnst->GetType() == DataType::FLOAT64) {
        if (instType == DataType::INT64) {
            // DOUBLE->INT/LONG
            uint64_t val = graph->IsDynamicMethod()
                               ? ConvertFloatToIntDyn(cnst->GetDoubleValue(), graph->GetRuntime(),
                                                      DataType::GetTypeSize(inst->GetType(), graph->GetArch()))
                               : ConvertFloatToInt(cnst->GetDoubleValue(), inst->GetType());
            return ConstFoldingCreateIntConst(inst, val, isLiteralData);
        }
        if (instType == DataType::FLOAT32) {
            // DOUBLE -> FLOAT
            if (graph->IsBytecodeOptimizer() && !isLiteralData) {
                return nullptr;
            }
            return graph->FindOrCreateConstant(static_cast<float>(cnst->GetDoubleValue()));
        }
        if (instType == DataType::FLOAT64) {
            // DOUBLE -> DOUBLE
            return cnst;
        }
    }
    return nullptr;
}

bool ConstFoldingCast(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Cast);
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst()) {
        ConstantInst *nwCnst = ConstFoldingCastConst(inst, input);
        if (nwCnst != nullptr) {
            inst->ReplaceUsers(nwCnst);
            return true;
        }
    }
    return false;
}

bool ConstFoldingNeg(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Neg);
    auto input = inst->GetInput(0);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(-cnst->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(-cnst->GetFloatValue());
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(-cnst->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingAbs(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Abs);
    auto input = inst->GetInput(0);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64: {
                ASSERT(DataType::IsTypeSigned(inst->GetType()));
                auto value = static_cast<int64_t>(cnst->GetIntValue());
                if (value == INT64_MIN) {
                    newCnst = cnst;
                    break;
                }
                auto uvalue = static_cast<uint64_t>((value < 0) ? -value : value);
                newCnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(uvalue, inst->GetType()));
                break;
            }
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(std::abs(cnst->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(std::abs(cnst->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingNot(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Not);
    auto input = inst->GetInput(0);
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        auto newCnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(~cnst->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingAdd(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() + cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() + cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() + cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return ConstFoldingBinaryMathWithNan(inst);
}

bool ConstFoldingSub(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Sub);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        ConstantInst *newCnst = nullptr;
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() - cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() - cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() - cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    if (input0.GetInst() == input1.GetInst() && DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        // for floating point values 'x-x -> 0' optimization is not applicable because of NaN/Infinity values
        auto newCnst = ConstFoldingCreateIntConst(inst, 0);
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return ConstFoldingBinaryMathWithNan(inst);
}

bool ConstFoldingMul(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Mul);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    auto graph = inst->GetBasicBlock()->GetGraph();
    ConstantInst *newCnst = nullptr;
    if (input0->IsConst() && input1->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0);
        auto cnst1 = static_cast<ConstantInst *>(input1);
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() * cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() * cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() * cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    if (ConstFoldingBinaryMathWithNan(inst)) {
        return true;
    }
    // Const is always in input1
    if (input0->IsConst()) {
        std::swap(input0, input1);
    }
    if (input1->IsConst() && input1->CastToConstant()->IsEqualConst(0, graph->IsBytecodeOptimizer())) {
        inst->ReplaceUsers(input1);
        return true;
    }
    return false;
}

bool ConstFoldingBinaryMathWithNan(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 2U);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    ASSERT(!input0->IsConst() || !input1->IsConst());
    if (!DataType::IsFloatType(inst->GetType())) {
        return false;
    }
    if (input0->IsConst()) {
        std::swap(input0, input1);
    }
    if (!input1->IsConst()) {
        return false;
    }
    if (!input1->CastToConstant()->IsNaNConst()) {
        return false;
    }
    inst->ReplaceUsers(input1);
    return true;
}

ConstantInst *ConstFoldingDivInt2Int(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (cnst1->GetIntValue() == 0) {
        return nullptr;
    }
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            if (static_cast<int32_t>(cnst0->GetIntValue()) == INT32_MIN &&
                static_cast<int32_t>(cnst1->GetIntValue()) == -1) {
                return graph->FindOrCreateConstant<uint32_t>(INT32_MIN);
            }
            return graph->FindOrCreateConstant<uint32_t>(
                ConvertIntToInt(static_cast<int32_t>(cnst0->GetIntValue()) / static_cast<int32_t>(cnst1->GetIntValue()),
                                inst->GetType()));
        }
        if (static_cast<int64_t>(cnst0->GetIntValue()) == INT64_MIN &&
            static_cast<int64_t>(cnst1->GetIntValue()) == -1) {
            return graph->FindOrCreateConstant<uint64_t>(INT64_MIN);
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            static_cast<int64_t>(cnst0->GetIntValue()) / static_cast<int64_t>(cnst1->GetIntValue()), inst->GetType()));
    }

    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(cnst0->GetIntValue(), inst->GetType()) /
                                                ConvertIntToInt(cnst1->GetIntValue(), inst->GetType()));
}

bool ConstFoldingDiv(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Div);
    auto input0 = inst->GetDataFlowInput(0);
    auto input1 = inst->GetDataFlowInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!input0->IsConst() || !input1->IsConst()) {
        return ConstFoldingBinaryMathWithNan(inst);
    }
    auto cnst0 = input0->CastToConstant();
    auto cnst1 = input1->CastToConstant();
    ConstantInst *newCnst = nullptr;
    switch (DataType::GetCommonType(inst->GetType())) {
        case DataType::INT64:
            newCnst = ConstFoldingDivInt2Int(inst, graph, cnst0, cnst1);
            if (newCnst == nullptr) {
                return false;
            }
            break;
        case DataType::FLOAT32:
            newCnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() / cnst1->GetFloatValue());
            break;
        case DataType::FLOAT64:
            newCnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() / cnst1->GetDoubleValue());
            break;
        default:
            UNREACHABLE();
    }
    inst->ReplaceUsers(newCnst);
    return true;
}

ConstantInst *ConstFoldingMinInt(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            return graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                std::min(static_cast<int32_t>(cnst0->GetIntValue()), static_cast<int32_t>(cnst1->GetIntValue())),
                inst->GetType()));
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            std::min(static_cast<int64_t>(cnst0->GetIntValue()), static_cast<int64_t>(cnst1->GetIntValue())),
            inst->GetType()));
    }
    return ConstFoldingCreateIntConst(
        inst, ConvertIntToInt(std::min(cnst0->GetIntValue(), cnst1->GetIntValue()), inst->GetType()));
}

bool ConstFoldingMin(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Min);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingMinInt(inst, graph, cnst0, cnst1);
                ASSERT(newCnst != nullptr);
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(
                    ark::helpers::math::Min(cnst0->GetFloatValue(), cnst1->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(
                    ark::helpers::math::Min(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return ConstFoldingBinaryMathWithNan(inst);
}

ConstantInst *ConstFoldingMaxInt(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            return graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                std::max(static_cast<int32_t>(cnst0->GetIntValue()), static_cast<int32_t>(cnst1->GetIntValue())),
                inst->GetType()));
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            std::max(static_cast<int64_t>(cnst0->GetIntValue()), static_cast<int64_t>(cnst1->GetIntValue())),
            inst->GetType()));
    }
    return ConstFoldingCreateIntConst(
        inst, ConvertIntToInt(std::max(cnst0->GetIntValue(), cnst1->GetIntValue()), inst->GetType()));
}

bool ConstFoldingMax(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Max);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingMaxInt(inst, graph, cnst0, cnst1);
                ASSERT(newCnst != nullptr);
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(
                    ark::helpers::math::Max(cnst0->GetFloatValue(), cnst1->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(
                    ark::helpers::math::Max(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return ConstFoldingBinaryMathWithNan(inst);
}

ConstantInst *ConstFoldingModIntConst(Graph *graph, Inst *inst, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            if (static_cast<int32_t>(cnst0->GetIntValue()) == INT32_MIN &&
                static_cast<int32_t>(cnst1->GetIntValue()) == -1) {
                return graph->FindOrCreateConstant<uint32_t>(0);
            }
            return graph->FindOrCreateConstant<uint32_t>(
                ConvertIntToInt(static_cast<int32_t>(cnst0->GetIntValue()) % static_cast<int32_t>(cnst1->GetIntValue()),
                                inst->GetType()));
        }
        if (static_cast<int64_t>(cnst0->GetIntValue()) == INT64_MIN &&
            static_cast<int64_t>(cnst1->GetIntValue()) == -1) {
            return graph->FindOrCreateConstant<uint64_t>(0);
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            static_cast<int64_t>(cnst0->GetIntValue()) % static_cast<int64_t>(cnst1->GetIntValue()), inst->GetType()));
    }
    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(cnst0->GetIntValue(), inst->GetType()) %
                                                ConvertIntToInt(cnst1->GetIntValue(), inst->GetType()));
}

bool ConstFoldingMod(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Mod);
    auto input0 = inst->GetDataFlowInput(0);
    auto input1 = inst->GetDataFlowInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input1->IsConst() && !DataType::IsFloatType(inst->GetType()) && input1->CastToConstant()->GetIntValue() == 1) {
        ConstantInst *cnst = ConstFoldingCreateIntConst(inst, 0);
        inst->ReplaceUsers(cnst);
        return true;
    }
    if (!input0->IsConst() || !input1->IsConst()) {
        return ConstFoldingBinaryMathWithNan(inst);
    }
    ConstantInst *newCnst = nullptr;
    auto cnst0 = input0->CastToConstant();
    auto cnst1 = input1->CastToConstant();
    if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        if (cnst1->GetIntValue() == 0) {
            return false;
        }
        newCnst = ConstFoldingModIntConst(graph, inst, cnst0, cnst1);
    } else if (inst->GetType() == DataType::FLOAT32) {
        if (cnst1->GetFloatValue() == 0) {
            return false;
        }
        newCnst =
            graph->FindOrCreateConstant(static_cast<float>(fmodf(cnst0->GetFloatValue(), cnst1->GetFloatValue())));
    } else if (inst->GetType() == DataType::FLOAT64) {
        if (cnst1->GetDoubleValue() == 0) {
            return false;
        }
        newCnst = graph->FindOrCreateConstant(fmod(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
    }
    inst->ReplaceUsers(newCnst);
    return true;
}

bool ConstFoldingShl(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Shl);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        ConstantInst *newCnst = nullptr;
        uint64_t sizeMask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            newCnst = graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                static_cast<uint32_t>(cnst0) << (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(sizeMask)),
                inst->GetType()));
        } else {
            newCnst = graph->FindOrCreateConstant(ConvertIntToInt(cnst0 << (cnst1 & sizeMask), inst->GetType()));
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingShr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Shr);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        uint64_t sizeMask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        // zerod high part of the constant
        if (sizeMask < DataType::GetTypeSize(DataType::INT32, graph->GetArch())) {
            // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
            uint64_t typeMask = (1ULL << (sizeMask + 1)) - 1;
            cnst0 = cnst0 & typeMask;
        }
        ConstantInst *newCnst = nullptr;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            newCnst = graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                static_cast<uint32_t>(cnst0) >> (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(sizeMask)),
                inst->GetType()));
        } else {
            newCnst = graph->FindOrCreateConstant(ConvertIntToInt(cnst0 >> (cnst1 & sizeMask), inst->GetType()));
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingAShr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::AShr);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        int64_t cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        uint64_t sizeMask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        ConstantInst *newCnst = nullptr;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            newCnst = graph->FindOrCreateConstant<uint32_t>(
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                ConvertIntToInt(static_cast<int32_t>(cnst0) >>
                                    (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(sizeMask)),
                                inst->GetType()));
        } else {
            newCnst = graph->FindOrCreateConstant(
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                ConvertIntToInt(cnst0 >> (cnst1 & sizeMask), inst->GetType()));
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingAnd(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::And);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    ConstantInst *newCnst = nullptr;
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        newCnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() & cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(newCnst);
        return true;
    }
    if (input0.GetInst()->IsConst()) {
        newCnst = static_cast<ConstantInst *>(input0.GetInst());
    } else if (input1.GetInst()->IsConst()) {
        newCnst = static_cast<ConstantInst *>(input1.GetInst());
    }
    if (newCnst != nullptr && newCnst->GetIntValue() == 0) {
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingOr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Or);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    ConstantInst *newCnst = nullptr;
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        newCnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() | cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(newCnst);
        return true;
    }
    if (input0.GetInst()->IsConst()) {
        newCnst = static_cast<ConstantInst *>(input0.GetInst());
    } else if (input1.GetInst()->IsConst()) {
        newCnst = static_cast<ConstantInst *>(input1.GetInst());
    }
    if (newCnst != nullptr && newCnst->GetIntValue() == static_cast<uint64_t>(-1)) {
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingXor(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Xor);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0->IsConst() && input1->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0);
        auto cnst1 = static_cast<ConstantInst *>(input1);
        ConstantInst *newCnst = nullptr;
        newCnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() ^ cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(newCnst);
        return true;
    }
    // A xor A = 0
    if (input0 == input1) {
        inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
        return true;
    }
    return false;
}

template <class T>
int64_t GetResult(T l, T r, [[maybe_unused]] const CmpInst *cmp)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (std::is_same<T, float>() || std::is_same<T, double>()) {
        ASSERT(DataType::IsFloatType(cmp->GetInputType(0)));
        if (std::isnan(l) || std::isnan(r)) {
            if (cmp->IsFcmpg()) {
                return 1;
            }
            return -1;
        }
    }
    if (l > r) {
        return 1;
    }
    if (l < r) {
        return -1;
    }
    return 0;
}

int64_t GetIntResult(ConstantInst *cnst0, ConstantInst *cnst1, DataType::Type inputType, const CmpInst *cmp)
{
    auto l = ConvertIntToInt(cnst0->GetIntValue(), inputType);
    auto r = ConvertIntToInt(cnst1->GetIntValue(), inputType);
    auto graph = cnst0->GetBasicBlock()->GetGraph();
    if (DataType::IsTypeSigned(inputType)) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inputType)) {
            return GetResult(static_cast<int32_t>(l), static_cast<int32_t>(r), cmp);
        }
        return GetResult(static_cast<int64_t>(l), static_cast<int64_t>(r), cmp);
    }
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(inputType)) {
        return GetResult(static_cast<uint32_t>(l), static_cast<uint32_t>(r), cmp);
    }
    return GetResult(l, r, cmp);
}

bool ConstFoldingCmpFloatNan(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Cmp);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (!input0->IsConst() && !input1->IsConst()) {
        return false;
    }

    if (!DataType::IsFloatType(inst->CastToCmp()->GetOperandsType())) {
        return false;
    }

    // One of the constant always will be in input1
    if (input0->IsConst()) {
        std::swap(input0, input1);
    }

    // For Float constant is applied only NaN cases
    if (!input1->CastToConstant()->IsNaNConst()) {
        return false;
    }
    // Result related with Fcmpg as wrote in spec
    int64_t res {-1};
    if (inst->CastToCmp()->IsFcmpg()) {
        res = 1;
    }
    inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, res));
    return true;
}

bool ConstFoldingCmp(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Cmp);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    auto cmp = inst->CastToCmp();
    auto inputType = cmp->GetInputType(0);
    if (ConstFoldingCmpFloatNan(inst)) {
        return true;
    }
    if (input0->IsConst() && input1->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0);
        auto cnst1 = static_cast<ConstantInst *>(input1);
        int64_t result = 0;
        switch (DataType::GetCommonType(inputType)) {
            case DataType::INT64: {
                result = GetIntResult(cnst0, cnst1, inputType, cmp);
                break;
            }
            case DataType::FLOAT32:
                result = GetResult(cnst0->GetFloatValue(), cnst1->GetFloatValue(), cmp);
                break;
            case DataType::FLOAT64:
                result = GetResult(cnst0->GetDoubleValue(), cnst1->GetDoubleValue(), cmp);
                break;
            default:
                break;
        }
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto newCnst = graph->IsBytecodeOptimizer() ? graph->FindOrCreateConstant<int32_t>(result)
                                                    : graph->FindOrCreateConstant(result);
        inst->ReplaceUsers(newCnst);
        return true;
    }
    if (input0 == input1 && DataType::GetCommonType(inputType) == DataType::INT64) {
        // for floating point values result may be non-zero if x is NaN
        inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
        return true;
    }
    return false;
}

ConstantInst *ConstFoldingCompareCreateConst(Inst *inst, bool value)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
        return graph->FindOrCreateConstant(static_cast<uint32_t>(value));
    }
    return graph->FindOrCreateConstant(static_cast<uint64_t>(value));
}

ConstantInst *ConstFoldingCompareCreateNewConst(Inst *inst, uint64_t cnstVal0, uint64_t cnstVal1)
{
    switch (inst->CastToCompare()->GetCc()) {
        case ConditionCode::CC_EQ:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 == cnstVal1));
        case ConditionCode::CC_NE:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 != cnstVal1));
        case ConditionCode::CC_LT:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnstVal0) < static_cast<int64_t>(cnstVal1)));
        case ConditionCode::CC_B:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 < cnstVal1));
        case ConditionCode::CC_LE:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnstVal0) <= static_cast<int64_t>(cnstVal1)));
        case ConditionCode::CC_BE:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 <= cnstVal1));
        case ConditionCode::CC_GT:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnstVal0) > static_cast<int64_t>(cnstVal1)));
        case ConditionCode::CC_A:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 > cnstVal1));
        case ConditionCode::CC_GE:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnstVal0) >= static_cast<int64_t>(cnstVal1)));
        case ConditionCode::CC_AE:
            return ConstFoldingCompareCreateConst(inst, (cnstVal0 >= cnstVal1));
        case ConditionCode::CC_TST_EQ:
            return ConstFoldingCompareCreateConst(inst, ((cnstVal0 & cnstVal1) == 0));
        case ConditionCode::CC_TST_NE:
            return ConstFoldingCompareCreateConst(inst, ((cnstVal0 & cnstVal1) != 0));
        default:
            UNREACHABLE();
    }
}

bool ConstFoldingCompareEqualInputs(Inst *inst, Inst *input0, Inst *input1)
{
    if (input0 != input1) {
        return false;
    }
    auto cmpInst = inst->CastToCompare();
    auto commonType = DataType::GetCommonType(input0->GetType());
    switch (cmpInst->GetCc()) {
        case ConditionCode::CC_EQ:
        case ConditionCode::CC_LE:
        case ConditionCode::CC_GE:
        case ConditionCode::CC_BE:
        case ConditionCode::CC_AE:
            // for floating point values result may be non-zero if x is NaN
            if (commonType == DataType::INT64 || commonType == DataType::POINTER) {
                inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 1));
                return true;
            }
            break;
        case ConditionCode::CC_NE:
            if (commonType == DataType::INT64 || commonType == DataType::POINTER) {
                inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
                return true;
            }
            break;
        case ConditionCode::CC_LT:
        case ConditionCode::CC_GT:
        case ConditionCode::CC_B:
        case ConditionCode::CC_A:
            // x<x is false even for x=NaN
            inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
            return true;
        default:
            return false;
    }
    return false;
}

static bool IsUniqueRef(Inst *inst)
{
    return inst->IsAllocation() || inst->GetOpcode() == Opcode::NullPtr ||
           inst->GetOpcode() == Opcode::LoadUniqueObject;
}

bool ConstFoldingCompareFloatNan(Inst *inst)
{
    ASSERT(DataType::IsFloatType(inst->CastToCompare()->GetOperandsType()));
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (!input0->IsConst() && !input1->IsConst()) {
        return false;
    }

    // One of the constant always will be in input1
    if (input0->IsConst()) {
        std::swap(input0, input1);
    }

    // For Float constant is applied only NaN cases
    if (!input1->CastToConstant()->IsNaNConst()) {
        return false;
    }

    // If both operands is NaN constant - it is OK, all optimization will be applied anyway
    bool resultConst {};
    // We shouldn't reverse ConditionCode, because the results is not related to order of inputs
    switch (inst->CastToCompare()->GetCc()) {
        case ConditionCode::CC_NE:
            // NaN != number is true
            resultConst = true;
            break;
        case ConditionCode::CC_EQ:  // ==
        case ConditionCode::CC_LT:  // <
        case ConditionCode::CC_LE:  // <=
        case ConditionCode::CC_GT:  // >
        case ConditionCode::CC_GE:  // >=
            // All these CC with NaN give false
            resultConst = false;
            break;
        default:
            UNREACHABLE();
    }
    inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, resultConst));
    return true;
}

bool ConstFoldingCompareIntConstant(Inst *inst)
{
    ASSERT(!DataType::IsFloatType(inst->CastToCompare()->GetOperandsType()));
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (!input0->IsConst() || !input1->IsConst()) {
        return false;
    }

    auto cnst0 = input0->CastToConstant();
    auto cnst1 = input1->CastToConstant();
    ConstantInst *newCnst = nullptr;
    auto type = inst->GetInputType(0);
    if (DataType::GetCommonType(type) == DataType::INT64) {
        uint64_t cnstVal0 = ConvertIntToInt(cnst0->GetIntValue(), type);
        uint64_t cnstVal1 = ConvertIntToInt(cnst1->GetIntValue(), type);
        newCnst = ConstFoldingCompareCreateNewConst(inst, cnstVal0, cnstVal1);
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingCompare(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Compare);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();

    if (DataType::IsFloatType(inst->CastToCompare()->GetOperandsType())) {
        if (ConstFoldingCompareFloatNan(inst)) {
            return true;
        }
    } else {
        if (ConstFoldingCompareIntConstant(inst)) {
            return true;
        }
    }
    if (input0->GetOpcode() == Opcode::LoadImmediate && input1->GetOpcode() == Opcode::LoadImmediate) {
        auto class0 = input0->CastToLoadImmediate()->GetObject();
        auto class1 = input1->CastToLoadImmediate()->GetObject();
        auto cc = inst->CastToCompare()->GetCc();
        ASSERT(cc == CC_NE || cc == CC_EQ);
        bool res {(class0 == class1) == (cc == CC_EQ)};
        inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, res));
        return true;
    }
    if (IsZeroConstantOrNullPtr(input0) && IsZeroConstantOrNullPtr(input1)) {
        auto cc = inst->CastToCompare()->GetCc();
        ASSERT(cc == CC_NE || cc == CC_EQ);
        inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, cc == CC_EQ));
        return true;
    }
    if (ConstFoldingCompareEqualInputs(inst, input0, input1)) {
        return true;
    }
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        ASSERT(input0 != input1);
        if (IsUniqueRef(input0) && IsUniqueRef(input1)) {
            auto cc = inst->CastToCompare()->GetCc();
            inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, cc == CC_NE));
            return true;
        }
    }
    return false;
}

bool ConstFoldingSqrt(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Sqrt);
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst()) {
        auto cnst = input->CastToConstant();
        Inst *newCnst = nullptr;
        if (cnst->GetType() == DataType::FLOAT32) {
            newCnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(std::sqrt(cnst->GetFloatValue()));
        } else {
            ASSERT(cnst->GetType() == DataType::FLOAT64);
            newCnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(std::sqrt(cnst->GetDoubleValue()));
        }
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

bool ConstFoldingLoadStatic(Inst *inst)
{
    auto field = inst->CastToLoadStatic()->GetObjField();
    ASSERT(field != nullptr);
    auto isFloatType = DataType::IsFloatType(inst->GetType());
    if (DataType::GetCommonType(inst->GetType()) != DataType::INT64 && !isFloatType) {
        return false;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto klass = runtime->GetClassForField(field);
    if (runtime->IsFieldReadonly(field) && runtime->IsClassInitialized(reinterpret_cast<uintptr_t>(klass))) {
        if (isFloatType) {
            auto value = runtime->GetStaticFieldFloatValue(field);
            if (inst->GetType() == DataType::FLOAT32) {
                inst->ReplaceUsers(graph->FindOrCreateConstant(static_cast<float>(value)));
            } else {
                ASSERT(inst->GetType() == DataType::FLOAT64);
                inst->ReplaceUsers(graph->FindOrCreateConstant(value));
            }
        } else {
            auto value = runtime->GetStaticFieldIntegerValue(field);
            inst->ReplaceUsers(graph->FindOrCreateConstant(ConvertIntToInt(value, inst->GetType())));
        }
        return true;
    }
    return false;
}

}  // namespace ark::compiler
