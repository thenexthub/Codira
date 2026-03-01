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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "libarkbase/utils/utils.h"
#include "optimizer/optimizations/const_folding.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/code_generator/codegen.h"

#include <optional>

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class ConstFoldingTest : public CommonTest {
public:
    ConstFoldingTest() : graph_(CreateGraphStartEndBlocks()) {}
    ~ConstFoldingTest() override = default;

    NO_COPY_SEMANTIC(ConstFoldingTest);
    NO_MOVE_SEMANTIC(ConstFoldingTest);

    using ConstFoldingFunc = bool (*)(Inst *inst);

    Graph *GetGraph()
    {
        return graph_;
    }

    template <class T>
    void CmpTest(T l, T r, int64_t result, DataType::Type srcType, bool fcmpg = false)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, l);
            CONSTANT(1U, r);
            BASIC_BLOCK(2U, 1U)
            {
                INST(2U, Opcode::Cmp).s32().SrcType(srcType).Inputs(0U, 1U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        if (DataType::IsFloatType(srcType)) {
            INS(2U).CastToCmp()->SetFcmpg(fcmpg);
            ASSERT_EQ(INS(2U).CastToCmp()->IsFcmpg(), fcmpg);
        }
        ASSERT_EQ(ConstFoldingCmp(&INS(2U)), true);
        GraphChecker(graph).Check();

        ConstantInst *inst = graph->FindConstant(DataType::INT64, result);
        ASSERT(inst != nullptr);
        ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    }

    template <class From, class To>
    void CastTest(From src, To dst, DataType::Type dstType)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, src);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Cast).SrcType(INS(0U).GetType()).Inputs(0U);
                INS(1U).SetType(dstType);
                INST(2U, Opcode::Return).Inputs(1U);
                INS(2U).SetType(dstType);
            }
        }
        ASSERT_EQ(ConstFoldingCast(&INS(1U)), true);
        GraphChecker(graph).Check();

        ConstantInst *inst = nullptr;
        if (DataType::GetCommonType(dstType) == DataType::INT64) {
            inst = graph->FindConstant(DataType::INT64, dst);
        } else if (dstType == DataType::FLOAT32) {
            inst = graph->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(dst));
        } else if (dstType == DataType::FLOAT64) {
            inst = graph->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(dst));
        }
        ASSERT(inst != nullptr);
        ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    }

    void CheckCompareEqualInputs(DataType::Type paramType, ConditionCode cc, std::optional<uint64_t> result)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(paramType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Compare).b().CC(cc).Inputs(0U, 0U);
                INST(2U, Opcode::Return).b().Inputs(1U);
            }
        }
        ASSERT_EQ(ConstFoldingCompare(&INS(1U)), result.has_value());
        if (result.has_value()) {
            auto inst = graph->FindConstant(DataType::INT64, *result);
            ASSERT(inst != nullptr);
            ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
        }
        GraphChecker(graph).Check();
    }

    void CheckCompareLoadImmediate(RuntimeInterface::ClassPtr class1, RuntimeInterface::ClassPtr class2,
                                   ConditionCode cc, uint64_t result)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            BASIC_BLOCK(2U, 1U)
            {
                INST(0U, Opcode::LoadImmediate).ref().Class(class1);
                INST(1U, Opcode::LoadImmediate).ref().Class(class2);
                INST(2U, Opcode::Compare).b().SrcType(DataType::Type::REFERENCE).CC(cc).Inputs(0U, 1U);
                INST(3U, Opcode::Return).b().Inputs(2U);
            }
        }
        ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
        auto inst = graph->FindConstant(DataType::INT64, result);
        ASSERT(inst != nullptr);
        ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
        GraphChecker(graph).Check();
    }

    enum InputValue : char {
        NAN1_F = 0,
        NAN2_F,
        NAN1_D,
        NAN2_D,
        NUMBER_F,
        NUMBER_D,
    };

    void CheckCompareWithNan(DataType::Type sourceType, InputValue input0, InputValue input1, ConditionCode cc,
                             bool result)
    {
        // Check that position is not changed
        static_assert(InputValue::NAN1_F == 0U && InputValue::NAN2_F == 1U && InputValue::NAN1_D == 2U &&
                      InputValue::NAN2_D == 3U && InputValue::NUMBER_F == 4U && InputValue::NUMBER_D == 5U);
        const auto nan1ValueF = bit_cast<float>(NAN_FLOAT);
        const auto nan2ValueF = bit_cast<float>(NAN_FLOAT | 0x1U);
        const auto nan1ValueD = bit_cast<double>(NAN_DOUBLE);
        const auto nan2ValueD = bit_cast<double>(NAN_DOUBLE | 0x1U);

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, nan1ValueF).f32();
            CONSTANT(1U, nan2ValueF).f32();
            CONSTANT(2U, nan1ValueD).f64();
            CONSTANT(3U, nan2ValueD).f64();
            PARAMETER(4U, 1.0F).f32();
            PARAMETER(5U, 2.0_D).f64();
            CONSTANT(8U, 1U);  // It is bool True
            CONSTANT(9U, 0U);  // It is bool False

            BASIC_BLOCK(2U, 1U)
            {
                // Indexes in InputValue is equal id in graph. Assert at the begin of the function is check that
                INST(6U, Opcode::Compare)
                    .b()
                    .SrcType(sourceType)
                    .CC(cc)
                    .Inputs(static_cast<size_t>(input0), static_cast<size_t>(input1));
                INST(7U, Opcode::Return).b().Inputs(6U);
            }
        }

        ConstFoldingCompare(&INS(6U));
        ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), result ? &INS(8U) : &INS(9U));
    }

    void CheckNegNan(InputValue input, bool isOptimized)
    {
        // Check that position is not changed
        static_assert(InputValue::NAN1_F == 0U && InputValue::NAN2_F == 1U && InputValue::NAN1_D == 2U &&
                      InputValue::NAN2_D == 3U && InputValue::NUMBER_F == 4U && InputValue::NUMBER_D == 5U);

        const auto nan1ValueF = bit_cast<float>(NAN_FLOAT);
        const auto nan2ValueF = bit_cast<float>(NAN_FLOAT | 0x1U);
        const auto nan1ValueD = bit_cast<double>(NAN_DOUBLE);
        const auto nan2ValueD = bit_cast<double>(NAN_DOUBLE | 0x1U);

        bool isFloat32 = IsFloat32InputValue(input);
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, nan1ValueF).f32();
            CONSTANT(1U, nan2ValueF).f32();
            CONSTANT(2U, nan1ValueD).f64();
            CONSTANT(3U, nan2ValueD).f64();
            PARAMETER(4U, 1.0F).f32();
            PARAMETER(5U, 2.0_D).f64();

            BASIC_BLOCK(2U, 1U)
            {
                // Indexes in InputValue is equal id in graph. Assert at the begin of the function is check that
                if (isFloat32) {
                    INST(6U, Opcode::Neg).f32().Inputs(static_cast<size_t>(input));
                    INST(7U, Opcode::Return).f32().Inputs(6U);
                } else {
                    INST(6U, Opcode::Neg).f64().Inputs(static_cast<size_t>(input));
                    INST(7U, Opcode::Return).f64().Inputs(6U);
                }
            }
        }

        ConstFoldingNeg(&INS(6U));
        ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), isOptimized ? &INS(input) : &INS(6U));
    }

    void CheckBinaryMathWithNan(Opcode opc, InputValue input0, InputValue input1, bool isOptimized)
    {
        // Check that position is not changed
        static_assert(InputValue::NAN1_F == 0U && InputValue::NAN2_F == 1U && InputValue::NAN1_D == 2U &&
                      InputValue::NAN2_D == 3U && InputValue::NUMBER_F == 4U && InputValue::NUMBER_D == 5U);

        const auto nan1ValueF = bit_cast<float>(NAN_FLOAT);
        const auto nan2ValueF = bit_cast<float>(NAN_FLOAT | 0x1U);
        const auto nan1ValueD = bit_cast<double>(NAN_DOUBLE);
        const auto nan2ValueD = bit_cast<double>(NAN_DOUBLE | 0x1U);

        bool isFloat32 = IsFloat32InputValue(input0);
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, nan1ValueF).f32();
            CONSTANT(1U, nan2ValueF).f32();
            CONSTANT(2U, nan1ValueD).f64();
            CONSTANT(3U, nan2ValueD).f64();
            PARAMETER(4U, 1.0F).f32();
            PARAMETER(5U, 2.0_D).f64();

            BASIC_BLOCK(2U, 1U)
            {
                // Indexes in InputValue is equal id in graph. Assert at the begin of the function is check that
                if (isFloat32) {
                    INST(6U, opc).f32().Inputs(static_cast<size_t>(input0), static_cast<size_t>(input1));
                    INST(7U, Opcode::Return).f32().Inputs(6U);
                } else {
                    INST(6U, opc).f64().Inputs(static_cast<size_t>(input0), static_cast<size_t>(input1));
                    INST(7U, Opcode::Return).f64().Inputs(6U);
                }
            }
        }
        auto func = GetConstFoldingFunc(opc);
        ASSERT_EQ(func(&INS(6U)), isOptimized);
        if (isOptimized) {
            ASSERT_EQ(INS(7U).GetInput(0U).GetInst()->IsConst(), true);
            ASSERT_EQ(INS(7U).GetInput(0U).GetInst()->CastToConstant()->IsNaNConst(), true);
        } else {
            ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), &INS(6U));
        }
    }

    void CheckNanBinaryMathManyCases(Opcode opc)
    {
        ASSERT(opc == Opcode::Mul || opc == Opcode::Div || opc == Opcode::Mod || opc == Opcode::Add ||
               opc == Opcode::Sub || opc == Opcode::Min || opc == Opcode::Max);

        // opc number and Nan
        CheckBinaryMathWithNan(opc, InputValue::NAN1_F, InputValue::NUMBER_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_F, InputValue::NUMBER_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN1_D, InputValue::NUMBER_D, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_D, InputValue::NUMBER_D, true);

        // opc Nan and number
        CheckBinaryMathWithNan(opc, InputValue::NUMBER_F, InputValue::NAN1_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NUMBER_F, InputValue::NAN2_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NUMBER_D, InputValue::NAN1_D, true);
        CheckBinaryMathWithNan(opc, InputValue::NUMBER_D, InputValue::NAN2_D, true);

        // opc with same Nan
        CheckBinaryMathWithNan(opc, InputValue::NAN1_F, InputValue::NAN1_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_F, InputValue::NAN2_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN1_D, InputValue::NAN1_D, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_D, InputValue::NAN2_D, true);

        // opc with different Nan
        CheckBinaryMathWithNan(opc, InputValue::NAN1_F, InputValue::NAN2_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_F, InputValue::NAN1_F, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN1_D, InputValue::NAN2_D, true);
        CheckBinaryMathWithNan(opc, InputValue::NAN2_D, InputValue::NAN1_D, true);

        CheckBinaryMathWithNan(opc, InputValue::NUMBER_F, InputValue::NUMBER_F, false);
        CheckBinaryMathWithNan(opc, InputValue::NUMBER_D, InputValue::NUMBER_D, false);
    }

private:
    bool IsFloat32InputValue(InputValue input)
    {
        switch (input) {
            case NAN1_F:
            case NAN2_F:
            case NUMBER_F:
                return true;
            case NAN1_D:
            case NAN2_D:
            case NUMBER_D:
                return false;
            default:
                UNREACHABLE();
        }
    }

    ConstFoldingFunc GetConstFoldingFunc(Opcode opc)
    {
        switch (opc) {
            case Opcode::Mul:
                return ConstFoldingMul;
            case Opcode::Div:
                return ConstFoldingDiv;
            case Opcode::Mod:
                return ConstFoldingMod;
            case Opcode::Add:
                return ConstFoldingAdd;
            case Opcode::Sub:
                return ConstFoldingSub;
            case Opcode::Min:
                return ConstFoldingMin;
            case Opcode::Max:
                return ConstFoldingMax;
            default:
                UNREACHABLE();
        }
    }

private:
    Graph *graph_;
};

TEST_F(ConstFoldingTest, NegInt64Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, UINT64_MAX);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Neg).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingNeg(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, NegInt32Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Neg).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    int32_t result = -1;
    ASSERT_EQ(ConstFoldingNeg(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32NegIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Neg).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    int32_t result = -1;
    ASSERT_EQ(ConstFoldingNeg(&INS(1U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, NegFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, static_cast<float>(12U));
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Neg).f32().Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    float result = -12.0;
    ASSERT_EQ(ConstFoldingNeg(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, NegDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Neg).f64().Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    double result = -12.0_D;
    ASSERT_EQ(ConstFoldingNeg(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, AbsIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Abs).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    int64_t result = 1;
    ASSERT_EQ(ConstFoldingAbs(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AbsIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -1L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Abs).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    int64_t result = 1;
    ASSERT_EQ(ConstFoldingAbs(&INS(1U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AbsFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(-12.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Abs).f32().Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    float result = 12.0F;
    ASSERT_EQ(ConstFoldingAbs(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, AbsDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -12.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Abs).f64().Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    double result = 12.0_D;
    ASSERT_EQ(ConstFoldingAbs(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, NotIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -12L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Not).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    int result = 11;
    ASSERT_EQ(ConstFoldingNot(&INS(1U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32NotIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -12L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Not).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    int result = 11;
    ASSERT_EQ(ConstFoldingNot(&INS(1U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AddIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AddIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AddInt8Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<uint8_t>(0xffffffffU));
        CONSTANT(1U, static_cast<uint8_t>(1U));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).u8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u8().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AddInt8Test)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, (uint8_t)0xffffffffU);
        CONSTANT(1U, (uint8_t)1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).u8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u8().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AddFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(-2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 1.0F;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, AddDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, -2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Add).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 1.0;
    ASSERT_EQ(ConstFoldingAdd(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, SubIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).s8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s8().Inputs(2U);
        }
    }
    int result = 0xffffffff;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32SubIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).s8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s8().Inputs(2U);
        }
    }
    int result = 0xffffffff;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, SubUIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).u8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u8().Inputs(2U);
        }
    }
    int result = 0xff;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32SubUIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).u8().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u8().Inputs(2U);
        }
    }
    int result = 0xff;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, SubFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 1.0F;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, SubDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sub).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 1.0;
    ASSERT_EQ(ConstFoldingSub(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, SubTestIntXsubX)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Sub).u64().Inputs(0U, 0U);
            INST(2U, Opcode::Return).u64().Inputs(1U);
        }
    }
    ASSERT_EQ(ConstFoldingSub(&INS(1U)), true);
    int result = 0;
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32SubTestIntXsubX)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Sub).u32().Inputs(0U, 0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }
    ASSERT_EQ(ConstFoldingSub(&INS(1U)), true);
    int result = 0;
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, SubTestDoubleXsubX)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).f64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Sub).f64().Inputs(0U, 0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    // the optimization "x-x -> 0" is not applicable for floating point values
    ASSERT_EQ(ConstFoldingSub(&INS(1U)), false);
}

TEST_F(ConstFoldingTest, MulIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 6;
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MulWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Mul);
}

TEST_F(ConstFoldingTest, DivWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Div);
}

TEST_F(ConstFoldingTest, ModWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Mod);
}

TEST_F(ConstFoldingTest, AddWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Div);
}

TEST_F(ConstFoldingTest, SubWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Mod);
}

TEST_F(ConstFoldingTest, MinWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Min);
}

TEST_F(ConstFoldingTest, MaxWithNan)
{
    CheckNanBinaryMathManyCases(Opcode::Max);
}

TEST_F(ConstFoldingTest, Constant32MulIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 6;
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, MulFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 6.0F;
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MulDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 6.0_D;
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, DivIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32DivIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, DivIntTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 0xffffffffffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int32_t result = 0x80000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32DivIntTest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 0xffffffffffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int32_t result = 0x80000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, DivIntTest2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x8000000000000000U);
        CONSTANT(1U, 0xffffffffffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int64_t result = 0x8000000000000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32DivIntTest2)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0x80000000U);
        CONSTANT(1U, 0xffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int32_t result = 0x80000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, UDivIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int64_t result = 0x80000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32UDivIntTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int64_t result = 0x80000000;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, DivFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 1.5F;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, DivDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 1.5_D;
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 2;
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 2.0F;
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinFloatNegativeZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -0.0F);
        CONSTANT(1U, +0.0F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Min).f32().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f32().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(-0.0));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMin(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinFloatNaNTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, std::numeric_limits<float>::quiet_NaN());
        CONSTANT(1U, 1.3F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Min).f32().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f32().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst =
        GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(std::numeric_limits<float>::quiet_NaN()));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMin(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 2.0_D;
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinDoubleNegativeZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -0.0);
        CONSTANT(1U, +0.0);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Min).f64().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f64().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(-0.0));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMin(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinDoubleNaNTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, std::numeric_limits<double>::quiet_NaN());
        CONSTANT(1U, 1.3_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Min).f64().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f64().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64,
                                         bit_cast<uint64_t, double>(std::numeric_limits<double>::quiet_NaN()));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMin(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CompareFloatNan)
{
    // Not equal number and NaN
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NUMBER_F, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NUMBER_F, ConditionCode::CC_NE, true);
    // Not equal NaN and number
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NUMBER_F, InputValue::NAN1_F, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NUMBER_F, InputValue::NAN2_F, ConditionCode::CC_NE, true);
    // Equal with different NaN
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NAN2_F, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NAN1_F, ConditionCode::CC_NE, true);
    // Not equal with same NaN
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NAN1_F, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NAN2_F, ConditionCode::CC_NE, true);

    // Cases CC_EQ, CC_LT, CC_LE, CC_GT, CC_GE always return false
    std::array<ConditionCode, 5U> allCcWithFalse = {CC_EQ, CC_LT, CC_LE, CC_GT, CC_GE};
    for (auto cc : allCcWithFalse) {
        // Equal with different NaN
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NAN2_F, cc, false);
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NAN1_F, cc, false);
        // Equal with same NaN
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NAN1_F, cc, false);
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NAN2_F, cc, false);

        CheckCompareWithNan(DataType::FLOAT32, InputValue::NUMBER_F, InputValue::NAN1_F, cc, false);
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NUMBER_F, InputValue::NAN2_F, cc, false);
        // Equal with NaN and number
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN1_F, InputValue::NUMBER_F, cc, false);
        CheckCompareWithNan(DataType::FLOAT32, InputValue::NAN2_F, InputValue::NUMBER_F, cc, false);
    }
}

TEST_F(ConstFoldingTest, CompareDoubleNan)
{
    // Not equal number and NaN
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NUMBER_D, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NUMBER_D, ConditionCode::CC_NE, true);
    // Not equal NaN and number
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NUMBER_D, InputValue::NAN1_D, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NUMBER_D, InputValue::NAN2_D, ConditionCode::CC_NE, true);
    // Equal with different NaN
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NAN2_D, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NAN1_D, ConditionCode::CC_NE, true);
    // Not equal with same NaN
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NAN1_D, ConditionCode::CC_NE, true);
    CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NAN2_D, ConditionCode::CC_NE, true);

    // Cases CC_EQ, CC_LT, CC_LE, CC_GT, CC_GE always return false
    std::array<ConditionCode, 5U> allCcWithFalse = {CC_EQ, CC_LT, CC_LE, CC_GT, CC_GE};
    for (auto cc : allCcWithFalse) {
        // Equal with different NaN
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NAN2_D, cc, false);
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NAN1_D, cc, false);
        // Equal with same NaN
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NAN1_D, cc, false);
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NAN2_D, cc, false);

        CheckCompareWithNan(DataType::FLOAT64, InputValue::NUMBER_D, InputValue::NAN1_D, cc, false);
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NUMBER_D, InputValue::NAN2_D, cc, false);
        // Equal with NaN and number
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN1_D, InputValue::NUMBER_D, cc, false);
        CheckCompareWithNan(DataType::FLOAT64, InputValue::NAN2_D, InputValue::NUMBER_D, cc, false);
    }
}

TEST_F(ConstFoldingTest, CmpNan)
{
    const auto nan1ValueF = bit_cast<float>(NAN_FLOAT);
    const auto nan1ValueD = bit_cast<double>(NAN_DOUBLE);

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, nan1ValueF).f32();
        CONSTANT(2U, nan1ValueD).f64();
        PARAMETER(4U, 1.0F).f32();
        PARAMETER(5U, 2.0_D).f64();
        CONSTANT(8U, 1L);   // It is bool True
        CONSTANT(9U, -1L);  // It is bool False

        BASIC_BLOCK(2U, 1U)
        {
            // Replace with constant 1
            INST(10U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 5U);
            INST(11U, Opcode::Neg).s32().Inputs(10U);

            // Replace with constant -1
            INST(12U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(false).Inputs(2U, 5U);
            INST(13U, Opcode::Neg).s32().Inputs(12U);

            // Replace with constant 1
            INST(14U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(0U, 4U);
            INST(15U, Opcode::Neg).s32().Inputs(14U);

            // Replace with constant -1
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(0U, 4U);
            INST(17U, Opcode::Neg).s32().Inputs(16U);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }

    ConstFoldingCmp(&INS(10U));
    ConstFoldingCmp(&INS(12U));
    ConstFoldingCmp(&INS(14U));
    ConstFoldingCmp(&INS(16U));
    ASSERT_EQ(INS(11U).GetInput(0U).GetInst(), &INS(8U));
    ASSERT_EQ(INS(13U).GetInput(0U).GetInst(), &INS(9U));
    ASSERT_EQ(INS(15U).GetInput(0U).GetInst(), &INS(8U));
    ASSERT_EQ(INS(17U).GetInput(0U).GetInst(), &INS(9U));
}

TEST_F(ConstFoldingTest, NegationNan)
{
    CheckNegNan(InputValue::NAN1_F, true);
    CheckNegNan(InputValue::NAN2_F, true);
    CheckNegNan(InputValue::NUMBER_F, false);
    CheckNegNan(InputValue::NAN1_D, true);
    CheckNegNan(InputValue::NAN2_D, true);
    CheckNegNan(InputValue::NUMBER_D, false);
}

TEST_F(ConstFoldingTest, MaxIntTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 3;
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxFloatTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(2.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 3.0F;
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxFloatNegativeZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -0.0F);
        CONSTANT(1U, +0.0F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Max).f32().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f32().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(+0.0));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMax(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxFloatNaNTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, std::numeric_limits<float>::quiet_NaN());
        CONSTANT(1U, 1.3F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Max).f32().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f32().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst =
        GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(std::numeric_limits<float>::quiet_NaN()));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMax(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxDoubleTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 2.0_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = 3.0_D;
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxDoubleNegativeZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -0.0);
        CONSTANT(1U, +0.0);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Max).f64().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f64().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(+0.0));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMax(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxDoubleNaNTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, std::numeric_limits<double>::quiet_NaN());
        CONSTANT(1U, 1.3_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Max).f64().Inputs(1U, 0U);
            INST(4U, Opcode::Min).f64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).f64().Inputs(4U);
        }
    }
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64,
                                         bit_cast<uint64_t, double>(std::numeric_limits<double>::quiet_NaN()));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), inst);
    ASSERT_EQ(ConstFoldingMax(&INS(3U)), true);
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, ShlTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 4;
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ShlTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 4;
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, Shl64Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 66U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 4;
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Shl32Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 34U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 4;
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32Shl32Test)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 34U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 4;
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ShrTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 4U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ShrTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 4U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AShrUTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 4U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::AShr).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    uint64_t result = 1;
    ASSERT_EQ(ConstFoldingAShr(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AShrUTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 4U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::AShr).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    uint32_t result = 1;
    ASSERT_EQ(ConstFoldingAShr(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, AShrTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -4L);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::AShr).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    int64_t result = -1;
    ASSERT_EQ(ConstFoldingAShr(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AShrTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -4L);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::AShr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int64_t result = -1;
    ASSERT_EQ(ConstFoldingAShr(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, Shr32Test)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -4L);
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int64_t result = 0xfffffff;
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32Shr32Test)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -4L);
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int64_t result = 0xfffffff;
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ModTestInt)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ModTestInt)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ModIntTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 0xffffffffffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int32_t result = 0;
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ModIntTest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0xffffffff80000000U);
        CONSTANT(1U, 0xffffffffffffffffU);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    int32_t result = 0;
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ModTestFloat)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 7.3F);
        CONSTANT(1U, 2.9F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 1.5;
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, ModTestDouble)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 15.5_D);
        CONSTANT(1U, 2.2_D);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    double result = fmod(15.5_D, 2.2_D);
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT64, bit_cast<uint64_t, double>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Mod1Test)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    int result = 0;
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32Mod1Test)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), true);
    int result = 0;
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, Mod1TestFloat)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).f64();
        CONSTANT(1U, 1.0);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    // the optimization "x % 1 -> 0" is not applicable for floating point values
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), false);
}

TEST_F(ConstFoldingTest, AndTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::And).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingAnd(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AndTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::And).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingAnd(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, OrTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 3;
    ASSERT_EQ(ConstFoldingOr(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32OrTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Or).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 3;
    ASSERT_EQ(ConstFoldingOr(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, XorTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Xor).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    int result = 3;
    ASSERT_EQ(ConstFoldingXor(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32XorTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Xor).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    int result = 3;
    ASSERT_EQ(ConstFoldingXor(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareEQTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareEQTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareNETest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareNETest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareLTTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareLTTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareLTTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -3L);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareLTTest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -3L);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareLETest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareLETest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareLETest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareLETest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareGTTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareGTTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareGTTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareGTTest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareGETest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareGETest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareGETest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareGETest1)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, -2L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareBTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_B).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareBTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_B).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareBETest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_BE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareBETest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_BE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareATest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_A).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareATest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_A).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareAETest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_AE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32CompareAETest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 3U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_AE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CompareZeroWithNullPtr)
{
    for (auto cc : {CC_EQ, CC_NE}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, 0U);
            CONSTANT(1U, nullptr);
            BASIC_BLOCK(2U, 1U)
            {
                INST(2U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(3U, Opcode::Return).b().Inputs(2U);
            }
        }
        ASSERT_TRUE(ConstFoldingCompare(&INS(2U)));
        ASSERT_TRUE(graph->RunPass<Cleanup>());
        auto expGraph = CreateEmptyGraph();
        GRAPH(expGraph)
        {
            CONSTANT(0U, (cc == CC_EQ ? 1U : 0U));
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::Return).b().Inputs(0U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, expGraph));
    }
}

TEST_F(ConstFoldingTest, CompareTstEqTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_TST_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CompareTstEqTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_TST_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CompareTstNeTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_TST_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 0;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CompareTstNeTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_TST_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    int result = 1;
    ASSERT_EQ(ConstFoldingCompare(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CompareEqualInputsTest)
{
    for (int ccInt = CC_LT; ccInt <= CC_AE; ++ccInt) {
        auto cc = static_cast<ConditionCode>(ccInt);
        for (auto type : {DataType::INT32, DataType::INT64, DataType::FLOAT64}) {
            std::optional<bool> result;
            switch (cc) {
                case ConditionCode::CC_EQ:
                case ConditionCode::CC_LE:
                case ConditionCode::CC_GE:
                case ConditionCode::CC_BE:
                case ConditionCode::CC_AE:
                    if (!IsFloatType(type)) {
                        result = true;
                    }
                    break;
                case ConditionCode::CC_NE:
                    if (!IsFloatType(type)) {
                        result = false;
                    }
                    break;
                case ConditionCode::CC_LT:
                case ConditionCode::CC_GT:
                case ConditionCode::CC_B:
                case ConditionCode::CC_A:
                    result = false;
                    break;
                default:
                    UNREACHABLE();
            }
            CheckCompareEqualInputs(type, cc, result);
        }
    }
}

TEST_F(ConstFoldingTest, CompareLoadImmediateTest)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    auto class2 = reinterpret_cast<RuntimeInterface::ClassPtr>(2U);
    CheckCompareLoadImmediate(class1, class1, ConditionCode::CC_EQ, 1U);
    CheckCompareLoadImmediate(class1, class2, ConditionCode::CC_EQ, 0U);
    CheckCompareLoadImmediate(class1, class1, ConditionCode::CC_NE, 0U);
    CheckCompareLoadImmediate(class1, class2, ConditionCode::CC_NE, 1U);
}

TEST_F(ConstFoldingTest, DivZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), false);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32DivZeroTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Div).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingDiv(&INS(2U)), false);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ModZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), false);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ModZeroTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMod(&INS(2U)), false);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, MulIntZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32MulIntZeroTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, MulFloatZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, static_cast<float>(3.0F));
        CONSTANT(1U, static_cast<float>(0.0F));
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MulDoubleZeroTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 3.0_D);
        CONSTANT(1U, 0.0);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mul).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingMul(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, AndZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::And).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingAnd(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32AndZeroTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::And).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingAnd(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, OrMinusOneTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        CONSTANT(1U, -1L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingOr(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32OrMinusOneTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        CONSTANT(1U, -1L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Or).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingOr(&INS(2U)), true);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(1U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, XorEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Xor).u64().Inputs(0U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingXor(&INS(2U)), true);
    int result = 0;
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32XorEqualInputs)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Xor).u32().Inputs(0U, 0U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingXor(&INS(2U)), true);
    int result = 0;
    auto inst = graph->FindConstant(DataType::INT32, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ShlBigOffsetTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        CONSTANT(1U, 20U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u16().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u16().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), false);
    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ShlBigOffsetTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        CONSTANT(1U, 20U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shl).u16().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u16().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingShl(&INS(2U)), false);
    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, ShrBigOffsetTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        CONSTANT(1U, 20U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).u16().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u16().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), false);
    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, Constant32ShrBigOffsetTest)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 25U).u32();
        CONSTANT(1U, 20U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Shr).u16().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u16().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingShr(&INS(2U)), false);
    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
    GraphChecker(graph).Check();
}

TEST_F(ConstFoldingTest, CastTest)
{
    uint8_t srcU8 = 0xff;
    CastTest(srcU8, static_cast<int8_t>(srcU8), DataType::INT8);
    CastTest(srcU8, static_cast<int16_t>(srcU8), DataType::INT16);
    CastTest(srcU8, static_cast<uint16_t>(srcU8), DataType::UINT16);

    int8_t srcI8 = -1;
    CastTest(srcI8, static_cast<float>(srcI8), DataType::FLOAT32);

    uint16_t srcU16 = 0xffff;
    CastTest(srcU16, static_cast<int8_t>(srcU16), DataType::INT8);
    CastTest(srcU16, static_cast<double>(srcU16), DataType::FLOAT64);
    CastTest(srcU16, srcU16, DataType::UINT16);

    int64_t srcI64 = -1;
    CastTest(srcI64, static_cast<uint8_t>(srcI64), DataType::UINT8);

    int32_t srcI32 = -1;
    CastTest(srcI32, static_cast<int8_t>(srcI32), DataType::INT8);

    float srcF = 0.25;
    CastTest(srcF, srcF, DataType::FLOAT32);

    double srcD = 0.25;
    CastTest(srcD, srcD, DataType::FLOAT64);

    CastTest(FLT_MAX, static_cast<double>(FLT_MAX), DataType::FLOAT64);
    CastTest(FLT_MIN, static_cast<double>(FLT_MIN), DataType::FLOAT64);

    // NOTE (schernykh) : ub test? - convert from double_max to float
    // DBL_MAX, static_cast<float>(DBL_MAX), DataType::FLOAT32
    CastTest(DBL_MIN, static_cast<float>(DBL_MIN), DataType::FLOAT32);

    CastTest(0.0F, static_cast<uint64_t>(0.0F), DataType::UINT64);
    // FLOAT->INT32
    CastTest(FLT_MAX, INT32_MAX, DataType::INT32);
    CastTest(-FLT_MAX, INT32_MIN, DataType::INT32);
    CastTest(nanf(""), static_cast<int32_t>(0U), DataType::INT32);
    CastTest(32.0F, static_cast<int32_t>(32.0F), DataType::INT32);
    // FLOAT->INT64
    CastTest(FLT_MAX, INT64_MAX, DataType::INT64);
    CastTest(-FLT_MAX, INT64_MIN, DataType::INT64);
    CastTest(nanf(""), static_cast<int64_t>(0U), DataType::INT64);
    CastTest(32.0F, static_cast<int64_t>(32.0F), DataType::INT64);
    // DOUBLE->INT32
    CastTest(DBL_MAX, INT32_MAX, DataType::INT32);
    CastTest(-DBL_MAX, INT32_MIN, DataType::INT32);
    CastTest(nan(""), static_cast<int32_t>(0U), DataType::INT32);
    CastTest(64.0_D, static_cast<int32_t>(64.0_D), DataType::INT32);
    // DOUBLE->INT64
    CastTest(DBL_MAX, INT64_MAX, DataType::INT64);
    CastTest(-DBL_MAX, INT64_MIN, DataType::INT64);
    CastTest(nan(""), static_cast<int64_t>(0U), DataType::INT64);
    CastTest(64.0_D, static_cast<int64_t>(64.0_D), DataType::INT64);
}

TEST_F(ConstFoldingTest, CmpTest)
{
    CmpTest(0U, 1U, -1L, DataType::INT32);
    CmpTest(1U, 0U, 1U, DataType::INT32);
    CmpTest(0U, 0U, 0U, DataType::INT32);

    CmpTest(0L, -1L, -1L, DataType::UINT32);
    CmpTest(INT64_MIN, INT64_MAX, -1L, DataType::INT64);
    CmpTest(INT64_MAX, INT64_MIN, 1U, DataType::INT64);

    CmpTest(0.0F, 1.0F, -1L, DataType::FLOAT32);
    CmpTest(1.0F, 0.0F, 1U, DataType::FLOAT32);
    CmpTest(0.0F, 0.0F, 0U, DataType::FLOAT32);

    CmpTest(0.0, 1.0, -1L, DataType::FLOAT64);
    CmpTest(1.0, 0.0, 1U, DataType::FLOAT64);
    CmpTest(0.0, 0.0, 0U, DataType::FLOAT64);

    CmpTest(std::nan("0"), 1.0, -1L, DataType::FLOAT64);
    CmpTest(1.0, std::nan("0"), -1L, DataType::FLOAT64);
    CmpTest(std::nan("0"), 1.0, 1U, DataType::FLOAT64, true);
    CmpTest(1.0, std::nan("0"), 1U, DataType::FLOAT64, true);

    CmpTest(std::nanf("0"), 1.0F, -1L, DataType::FLOAT32);
    CmpTest(1.0F, std::nanf("0"), -1L, DataType::FLOAT32);
    CmpTest(std::nanf("0"), 1.0F, 1U, DataType::FLOAT32, true);
    CmpTest(1.0F, std::nanf("0"), 1U, DataType::FLOAT32, true);
}

TEST_F(ConstFoldingTest, CmpEqualInputsIntTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).u64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Cmp).i32().Inputs(0U, 0U);
            INST(3U, Opcode::Return).i32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingCmp(&INS(2U)), true);
    int result = 0;
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, CmpEqualInputsDoubleTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 25U).f64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Cmp).i32().Inputs(0U, 0U);
            INST(3U, Opcode::Return).i32().Inputs(2U);
        }
    }
    ASSERT_EQ(ConstFoldingCmp(&INS(2U)), false);
}

TEST_F(ConstFoldingTest, SqrtTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(1U, 0.78539816339744828F);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Sqrt).f32().Inputs(1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    float result = 0.88622695207595825;
    ASSERT_EQ(ConstFoldingSqrt(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::FLOAT32, bit_cast<uint32_t, float>(result));
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MaxIntTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Max).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    int result = 2;
    ASSERT_EQ(ConstFoldingMax(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}

TEST_F(ConstFoldingTest, MinIntTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -1L);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Min).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    int result = -1;
    ASSERT_EQ(ConstFoldingMin(&INS(2U)), true);
    auto inst = GetGraph()->FindConstant(DataType::INT64, result);
    ASSERT(inst != nullptr);
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), inst);
    GraphChecker(GetGraph()).Check();
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
