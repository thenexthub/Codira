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
#include "libarkbase/utils/utils.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "tests/graph_comparator.h"
#include "unit_test.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class IrBuilderTest : public AsmTest {
public:
    IrBuilderTest()
        : defaultCompilerNonOptimizing_(g_options.IsCompilerNonOptimizing()),
          defaultCompilerUseSafePoint_(g_options.IsCompilerUseSafepoint())
    {
        g_options.SetCompilerNonOptimizing(false);
        g_options.SetCompilerUseSafepoint(false);
    }

    ~IrBuilderTest() override
    {
        g_options.SetCompilerNonOptimizing(defaultCompilerNonOptimizing_);
        g_options.SetCompilerUseSafepoint(defaultCompilerUseSafePoint_);
    }

    NO_COPY_SEMANTIC(IrBuilderTest);
    NO_MOVE_SEMANTIC(IrBuilderTest);

    void CheckSimple(const std::string &instName, DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "mov" || instName == "lda" || instName == "sta");
        std::string currType;
        if (dataType == DataType::Type::REFERENCE) {
            currType = "i64[]";
        } else {
            currType = ToString(dataType);
        }

        std::string source = ".function " + currType + " main(";
        source += currType + " a0) {\n";
        if (instName == "mov") {
            source += "mov" + instType + " v0, a0\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "lda") {
            source += "lda" + instType + " a0\n";
        } else if (instName == "sta") {
            source += "lda" + instType + " a0\n";
            source += "sta" + instType + " v0\n";
            source += "lda" + instType + " v0\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + instType + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(dataType);

            BASIC_BLOCK(2U, -1L)
            {
                INST(1U, Opcode::Return).Inputs(0U);
                INS(1U).SetType(dataType);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckSimpleWithImm(const std::string &instName, DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "mov" || instName == "fmov" || instName == "lda" || instName == "flda");
        std::string currType = ToString(dataType);

        std::string source = ".function " + currType + " main() {\n";
        if (instName == "mov") {
            source += "movi" + instType + " v0, 0\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "fmov") {
            source += "fmovi" + instType + " v0, 0.\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "lda") {
            source += "ldai" + instType + " 0\n";
        } else if (instName == "flda") {
            source += "fldai" + instType + " 0.\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + instType + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto constantType = GetCommonType(dataType);
        auto graph = CreateGraphWithDefaultRuntime();

        GRAPH(graph)
        {
            CONSTANT(0U, 0U);
            INS(0U).SetType(constantType);

            BASIC_BLOCK(2U, -1L)
            {
                INST(1U, Opcode::Return).Inputs(0U);
                INS(1U).SetType(dataType);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckCmp(const std::string &instName, DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "cmp" || instName == "ucmp" || instName == "fcmpl" || instName == "fcmpg");
        std::string currType;
        if (dataType == DataType::Type::REFERENCE) {
            currType = "i64[]";
        } else {
            currType = ToString(dataType);
        }
        std::string source = ".function i32 main(";
        source += currType + " a0, ";
        source += currType + " a1) {\n";
        source += "lda" + instType + " a0\n";
        source += instName + instType + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(dataType);
            PARAMETER(1U, 1U);
            INS(1U).SetType(dataType);

            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Cmp).s32().Inputs(0U, 1U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckFloatCmp(const std::string &instName, DataType::Type dataType, const std::string &instType, bool fcmpg)
    {
        ASSERT(instName == "fcmpl" || instName == "fcmpg");
        std::string currType = ToString(dataType);

        std::string source = ".function i32 main(";
        source += currType + " a0, ";
        source += currType + " a1) {\n";
        source += "lda" + instType + " a0\n";
        source += instName + instType + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(dataType);
            PARAMETER(1U, 1U);
            INS(1U).SetType(dataType);

            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Cmp).s32().SrcType(dataType).Fcmpg(fcmpg).Inputs(0U, 1U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    Graph *ExpectedGraphCheckCondJump(DataType::Type type, ConditionCode cc)
    {
        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            PARAMETER(1U, 1U);
            INS(1U).SetType(type);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }
        return graph;
    }

    template <bool IS_OBJ>
    void CheckCondJump(ConditionCode cc)
    {
        std::string cmd;
        switch (cc) {
            case ConditionCode::CC_EQ:
                cmd = "jeq";
                break;
            case ConditionCode::CC_NE:
                cmd = "jne";
                break;
            case ConditionCode::CC_LT:
                cmd = "jlt";
                break;
            case ConditionCode::CC_GT:
                cmd = "jgt";
                break;
            case ConditionCode::CC_LE:
                cmd = "jle";
                break;
            case ConditionCode::CC_GE:
                cmd = "jge";
                break;
            default:
                UNREACHABLE();
        }

        std::string instPostfix;
        std::string paramType = "i32";
        auto type = DataType::INT32;
        if constexpr (IS_OBJ) {
            instPostfix = ".obj";
            paramType = "i64[]";
            type = DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += paramType + " a0, " + paramType + " a1) {\n";
        source += "lda" + instPostfix + " a0\n";
        source += cmd + instPostfix + " a1, label\n";
        source += "label:\n";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));
        auto graph = ExpectedGraphCheckCondJump(type, cc);
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    std::string GetCcString(ConditionCode cc)
    {
        std::string cmd;
        switch (cc) {
            case ConditionCode::CC_EQ:
                cmd = "jeqz";
                break;
            case ConditionCode::CC_NE:
                cmd = "jnez";
                break;
            case ConditionCode::CC_LT:
                cmd = "jltz";
                break;
            case ConditionCode::CC_GT:
                cmd = "jgtz";
                break;
            case ConditionCode::CC_LE:
                cmd = "jlez";
                break;
            case ConditionCode::CC_GE:
                cmd = "jgez";
                break;
            default:
                UNREACHABLE();
        }
        return cmd;
    }

    template <bool IS_OBJ>
    void CheckCondJumpWithZero(ConditionCode cc)
    {
        std::string cmd = GetCcString(cc);
        std::string instPostfix;
        std::string paramType = "i32";
        auto type = DataType::INT32;
        if constexpr (IS_OBJ) {
            instPostfix = ".obj";
            paramType = "i64[]";
            type = DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += paramType + " a0) {\n";
        source += "lda" + instPostfix + " a0\n";
        source += cmd + instPostfix + " label\n";
        source += "label:\n";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            if constexpr (IS_OBJ) {
                NULLPTR(2U);
            } else {
                CONSTANT(2U, 0U).s64();
            }

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(1U, Opcode::Compare).b().CC(cc).Inputs(0U, 2U);
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

private:
    bool defaultCompilerNonOptimizing_;
    bool defaultCompilerUseSafePoint_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(IrBuilderTest, LoadArrayType64)
{
    auto source = R"(
    .function void main(i64[] a0, i32[] a1) {
        ldai 0
        ldarr.64 a0
        movi v0, 0
        starr a1, v0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 3U);
            INST(7U, Opcode::LoadArray).s64().Inputs(4U, 6U);
            INST(8U, Opcode::SaveState).Inputs(2U, 7U, 1U).SrcVregs({0U, 3U, 2U});
            INST(9U, Opcode::NullCheck).ref().Inputs(1U, 8U);
            INST(10U, Opcode::LenArray).s32().Inputs(9U);
            INST(11U, Opcode::BoundsCheck).s32().Inputs(10U, 2U, 8U);
            INST(12U, Opcode::StoreArray).s32().Inputs(9U, 11U, 7U);
            INST(13U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicPrintU64)
{
    auto source = R"(
    .record IO <external>
    .function void IO.printU64(u64 a0) <external>
    .function void main(u64 a0) {
        ldai.64 23
        sub2.64 a0
        sta.64 a0
        call.short IO.printU64, a0, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1U, 13U).u64();
        CONSTANT(0U, 23U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Intrinsic)
                .v0id()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_U64)
                .Inputs({{DataType::UINT64, 2U}});
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, BuiltinIsInf)
{
    auto source = R"(
        .record Double <external>
        .function u1 Double.isInfinite(f64 a0) <external>
        .function u1 main(f64 a0) {
            call.short Double.isInfinite, a0
            return
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_DOUBLE_IS_INF)
                .b()
                .Inputs({{DataType::FLOAT64, 0U}});
            INST(2U, Opcode::Return).b().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicAbs)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.absF64(f64 a0) <external>
    .function f64 main(f64 a0) {
        fldai.64 1.23
        fsub2.64 a0
        sta.64 v5
        call.short Math.absF64, v5, v5
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1U, 13U).f64();
        CONSTANT(0U, 1.23_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Abs).f64().Inputs(2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathSqrt)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.sqrt(f64 a0) <external>
    .function f64 main(f64 a0) {
        fldai.64 3.14
        fsub2.64 a0
        sta.64 v1
        call.short Math.sqrt, v1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1U, 0U).f64();
        CONSTANT(0U, 3.14_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Sqrt).f64().Inputs(2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathFsqrt)
{
    auto source = R"(
    .record Math <external>
    .function f32 Math.fsqrt(f32 a0) <external>
    .function f32 main(f32 a0) {
        fldai 3.14
        fsub2 a0
        sta v1
        call.short Math.fsqrt, v1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1U, 0U).f32();
        CONSTANT(0U, 3.14F).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Sqrt).f32().Inputs(2U);
            INST(4U, Opcode::Return).f32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMinI32)
{
    auto source = R"(
    .record Math <external>
    .function i32 Math.minI32(i32 a0, i32 a1) <external>
    .function i32 main(i32 a0, i32 a1) {
        call.short Math.minI32, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Min).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMinI64)
{
    auto source = R"(
    .record Math <external>
    .function i64 Math.minI64(i64 a0, i64 a1) <external>
    .function i64 main(i64 a0, i64 a1) {
        call.short Math.minI64, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Min).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMinF64)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.minF64(f64 a0, f64 a1) <external>
    .function f64 main(f64 a0, f64 a1) {
        call.short Math.minF64, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMinF32)
{
    auto source = R"(
    .record Math <external>
    .function f32 Math.minF32(f32 a0, f32 a1) <external>
    .function f32 main(f32 a0, f32 a1) {
        call.short Math.minF32, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Min).f32().Inputs(0U, 1U);
            INST(6U, Opcode::Return).f32().Inputs(4U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMaxI32)
{
    auto source = R"(
    .record Math <external>
    .function i32 Math.maxI32(i32 a0, i32 a1) <external>
    .function i32 main(i32 a0, i32 a1) {
        call.short Math.maxI32, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Max).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMaxI64)
{
    auto source = R"(
    .record Math <external>
    .function i64 Math.maxI64(i64 a0, i64 a1) <external>
    .function i64 main(i64 a0, i64 a1) {
        call.short Math.maxI64, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Max).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMaxF64)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.maxF64(f64 a0, f64 a1) <external>
    .function f64 main(f64 a0, f64 a1) {
        call.short Math.maxF64, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathMaxF32)
{
    auto source = R"(
    .record Math <external>
    .function f32 Math.maxF32(f32 a0, f32 a1) <external>
    .function f32 main(f32 a0, f32 a1) {
        call.short Math.maxF32, a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Max).f32().Inputs(0U, 1U);
            INST(6U, Opcode::Return).f32().Inputs(4U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, NoCheckForFloatDiv)
{
    auto source = R"(
    .function f64 main(f64 a0) {
        fldai.64 23.0
        fdiv2.64 a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1U, 0U).f64();
        CONSTANT(0U, 23.0_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, MultipleThrow)
{
    auto source = R"(
    .record array <external>
    .function void main(array a0) {
        throw a0
        throw a0
        throw a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::Throw).Inputs(0U, 1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks if not dominate inputs are removed from SaveStateInst
TEST_F(IrBuilderTest, RemoveNotDominateInputs)
{
    auto source = R"(
    .function void main(i32 a0, i32 a1) {
        lda a0
        jlt a1, label

        sub2 a1
        sta v0
        call foo1, v0
    label:
        call foo2
        return.void
    }

    .function i64 foo1(i32 a0) {
        ldai.64 1
        return.64
    }

    .function i64 foo2() {
        ldai.64 1
        return.64
    }
    )";

    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::Sub).s32().Inputs(0U, 1U);
            INST(5U, Opcode::SaveState).Inputs(4U, 4U).SrcVregs({0U, 3U});
            INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::INT32, 4U}, {DataType::NO_TYPE, 5U}});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(8U, Opcode::CallStatic).s64().Inputs({{DataType::NO_TYPE, 7U}});
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mov instruction with integer parameters
TEST_F(IrBuilderTest, MovInt)
{
    DataType::Type dataType = DataType::Type::INT32;
    std::string instType;
    CheckSimple("mov", dataType, instType);
}

// Checks the build of the mov instruction with real parameters
TEST_F(IrBuilderTest, MovReal)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckSimple("mov", dataType, instType);
}

// Checks the build of the mov.64 instruction with integer parameters
TEST_F(IrBuilderTest, Mov64Int)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckSimple("mov", dataType, instType);
}

// Checks the build of the mov.64 instruction with real parameters
TEST_F(IrBuilderTest, Mov64Real)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckSimple("mov", dataType, instType);
}

// Checks the build of the mov.obj instruction
TEST_F(IrBuilderTest, MovObj)
{
    DataType::Type dataType = DataType::Type::REFERENCE;
    std::string instType = ".obj";
    CheckSimple("mov", dataType, instType);
}

// Checks the build of the mov.null instruction
TEST_F(IrBuilderTest, MovNull)
{
    auto source = R"(
        .record panda.String <external>
        .function panda.String main() {
            mov.null v0
            lda v0
            return
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, nullptr);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the movi instruction with integer parameters
TEST_F(IrBuilderTest, MoviInt)
{
    DataType::Type dataType = DataType::Type::INT32;
    std::string instType;
    CheckSimpleWithImm("mov", dataType, instType);
}

// Checks the build of the fmovi instruction with real parameters
TEST_F(IrBuilderTest, FmoviReal)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckSimpleWithImm("fmov", dataType, instType);
}

// Checks the build of the movi.64 instruction with integer parameters
TEST_F(IrBuilderTest, Movi64Int)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckSimpleWithImm("mov", dataType, instType);
}

// Checks the build of the movi.64 instruction with real parameters
TEST_F(IrBuilderTest, Fmovi64Real)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckSimpleWithImm("fmov", dataType, instType);
}

// Checks the build of the lda instruction with integer parameters
TEST_F(IrBuilderTest, LdaInt)
{
    DataType::Type dataType = DataType::Type::INT32;
    std::string instType;
    CheckSimple("lda", dataType, instType);
}

// Checks the build of the lda instruction with real parameters
TEST_F(IrBuilderTest, LdaReal)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckSimple("lda", dataType, instType);
}

// Checks the build of the lda.64 instruction with integer parameters
TEST_F(IrBuilderTest, Lda64Int)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckSimple("lda", dataType, instType);
}

// Checks the build of the lda.64 instruction with real parameters
TEST_F(IrBuilderTest, Lda64Real)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckSimple("lda", dataType, instType);
}

// Checks the build of the lda.obj instruction
TEST_F(IrBuilderTest, LdaObj)
{
    DataType::Type dataType = DataType::Type::REFERENCE;
    std::string instType = ".obj";
    CheckSimple("lda", dataType, instType);
}

// Checks the build of the lda.obj instruction
TEST_F(IrBuilderTest, LdaNull)
{
    auto source = R"(
        .record panda.String <external>
        .function panda.String main() {
            lda.null
            return.obj
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, nullptr);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldai instruction with integer parameters
TEST_F(IrBuilderTest, LdaiInt)
{
    DataType::Type dataType = DataType::Type::INT32;
    std::string instType;
    CheckSimpleWithImm("lda", dataType, instType);
}

// Checks the build of the ldai instruction with real parameters
TEST_F(IrBuilderTest, FldaiReal)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckSimpleWithImm("flda", dataType, instType);
}

// Checks the build of the ldai.64 instruction with integer parameters
TEST_F(IrBuilderTest, Ldai64Int)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckSimpleWithImm("lda", dataType, instType);
}

// Checks the build of the ldai.64 instruction with real parameters
TEST_F(IrBuilderTest, Fldai64Real)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckSimpleWithImm("flda", dataType, instType);
}

// Checks the build of the lda.str instruction
TEST_F(IrBuilderTest, LdaStr)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main() {
        lda.str "lda_test"
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::LoadString).ref().Inputs(2U);
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the lda.type instruction
TEST_F(IrBuilderTest, LdaType)
{
    auto source = R"(
    .record R {}
    .function R main() {
        lda.type R
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::LoadType).ref().Inputs(2U);
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sta instruction with integer parameters
TEST_F(IrBuilderTest, StaInt)
{
    DataType::Type dataType = DataType::Type::INT32;
    std::string instType;
    CheckSimple("sta", dataType, instType);
}

// Checks the build of the sta instruction with real parameters
TEST_F(IrBuilderTest, StaReal)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckSimple("sta", dataType, instType);
}

// Checks the build of the sta.64 instruction with integer parameters
TEST_F(IrBuilderTest, Sta64Int)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckSimple("sta", dataType, instType);
}

// Checks the build of the sta.64 instruction with real parameters
TEST_F(IrBuilderTest, Sta64Real)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckSimple("sta", dataType, instType);
}

// Checks the build of the sta.obj instruction
TEST_F(IrBuilderTest, StaObj)
{
    DataType::Type dataType = DataType::Type::REFERENCE;
    std::string instType = ".obj";
    CheckSimple("sta", dataType, instType);
}

// Checks the build of the jmp instruction
TEST_F(IrBuilderTest, Jmp)
{
    auto source = R"(
    .function void main() {
        jmp label
    label:
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, -1L)
        {
            INST(1U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the cmp.64 instruction
TEST_F(IrBuilderTest, Cmp64)
{
    DataType::Type dataType = DataType::Type::INT64;
    std::string instType = ".64";
    CheckCmp("cmp", dataType, instType);
}

// Checks the build of the ucmp instruction
TEST_F(IrBuilderTest, Ucmp)
{
    DataType::Type dataType = DataType::Type::UINT32;
    std::string instType;
    CheckCmp("ucmp", dataType, instType);
}

// Checks the build of the ucmp.64 instruction
TEST_F(IrBuilderTest, Ucmp64)
{
    DataType::Type dataType = DataType::Type::UINT64;
    std::string instType = ".64";
    CheckCmp("ucmp", dataType, instType);
}

// Checks the build of the fcmpl instruction
TEST_F(IrBuilderTest, Fcmpl)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckFloatCmp("fcmpl", dataType, instType, false);
}

// Checks the build of the fcmpl.64 instruction
TEST_F(IrBuilderTest, Fcmpl64)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckFloatCmp("fcmpl", dataType, instType, false);
}

// Checks the build of the fcmpg instruction
TEST_F(IrBuilderTest, Fcmpg)
{
    DataType::Type dataType = DataType::Type::FLOAT32;
    std::string instType;
    CheckFloatCmp("fcmpg", dataType, instType, true);
}

// Checks the build of the fcmpg.64 instruction
TEST_F(IrBuilderTest, Fcmpg64)
{
    DataType::Type dataType = DataType::Type::FLOAT64;
    std::string instType = ".64";
    CheckFloatCmp("fcmpg", dataType, instType, true);
}

// Checks the build of the jeqz.obj instruction
TEST_F(IrBuilderTest, JeqzObj)
{
    CheckCondJumpWithZero<true>(ConditionCode::CC_EQ);
}

// Checks the build of the jnez.obj instruction
TEST_F(IrBuilderTest, JnezObj)
{
    CheckCondJumpWithZero<true>(ConditionCode::CC_NE);
}

// Checks the build of the jeqz instruction
TEST_F(IrBuilderTest, Jeqz)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_EQ);
}

// Checks the build of the jnez instruction
TEST_F(IrBuilderTest, Jnez)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_NE);
}

// Checks the build of the jltz instruction
TEST_F(IrBuilderTest, Jltz)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_LT);
}

// Checks the build of the jgtz instruction
TEST_F(IrBuilderTest, Jgtz)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_GT);
}

// Checks the build of the jlez instruction
TEST_F(IrBuilderTest, Jlez)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_LE);
}

// Checks the build of the jgez instruction
TEST_F(IrBuilderTest, Jgez)
{
    CheckCondJumpWithZero<false>(ConditionCode::CC_GE);
}

// Checks the build of the jeq.obj instruction
TEST_F(IrBuilderTest, JeqObj)
{
    CheckCondJump<true>(ConditionCode::CC_EQ);
}

// Checks the build of the jne.obj instruction
TEST_F(IrBuilderTest, JneObj)
{
    CheckCondJump<true>(ConditionCode::CC_NE);
}

// Checks the build of the jeq instruction
TEST_F(IrBuilderTest, Jeq)
{
    CheckCondJump<false>(ConditionCode::CC_EQ);
}

// Checks the build of the jne instruction
TEST_F(IrBuilderTest, Jne)
{
    CheckCondJump<false>(ConditionCode::CC_NE);
}

// Checks the build of the jlt instruction
TEST_F(IrBuilderTest, Jlt)
{
    CheckCondJump<false>(ConditionCode::CC_LT);
}

// Checks the build of the jgt instruction
TEST_F(IrBuilderTest, Jgt)
{
    CheckCondJump<false>(ConditionCode::CC_GT);
}

// Checks the build of the jle instruction
TEST_F(IrBuilderTest, Jle)
{
    CheckCondJump<false>(ConditionCode::CC_LE);
}

// Checks the build of the jge instruction
TEST_F(IrBuilderTest, Jge)
{
    CheckCondJump<false>(ConditionCode::CC_GE);
}

// Checks the build of the fadd2 instruction
TEST_F(IrBuilderTest, Fadd2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1) {
        lda a0
        fadd2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fadd2.64 instruction
TEST_F(IrBuilderTest, Fadd2f64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1) {
        lda.64 a0
        fadd2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fsub2 instruction
TEST_F(IrBuilderTest, Fsub2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1) {
        lda a0
        fsub2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fsub2.64 instruction
TEST_F(IrBuilderTest, Fsub2f64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1) {
        lda.64 a0
        fsub2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmul2 instruction
TEST_F(IrBuilderTest, Fmul2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1) {
        lda a0
        fmul2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmul2.64 instruction
TEST_F(IrBuilderTest, Fmul2f64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1) {
        lda.64 a0
        fmul2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fdiv2 instruction
TEST_F(IrBuilderTest, Fdiv2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1) {
        lda a0
        fdiv2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fdiv2.64 instruction
TEST_F(IrBuilderTest, Fdiv2f64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1) {
        lda.64 a0
        fdiv2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmod2 instruction
TEST_F(IrBuilderTest, Fmod2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1) {
        lda a0
        fmod2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mod).f32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmod2.64 instruction
TEST_F(IrBuilderTest, Fmod2f64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1) {
        lda.64 a0
        fmod2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mod).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add2 instruction
TEST_F(IrBuilderTest, Add2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        add2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add2.64 instruction
TEST_F(IrBuilderTest, Add2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        add2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub2 instruction
TEST_F(IrBuilderTest, Sub2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        sub2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub2.64 instruction
TEST_F(IrBuilderTest, Sub2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        sub2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul2 instruction
TEST_F(IrBuilderTest, Mul2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        mul2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul2.64 instruction
TEST_F(IrBuilderTest, Mul2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        mul2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and2 instruction
TEST_F(IrBuilderTest, And2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        and2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and2.64 instruction
TEST_F(IrBuilderTest, And2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        and2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or2 instruction
TEST_F(IrBuilderTest, Or2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        or2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or2.64 instruction
TEST_F(IrBuilderTest, Or2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        or2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the Xor2 instruction
TEST_F(IrBuilderTest, Xor2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        xor2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xor2.64 instruction
TEST_F(IrBuilderTest, Xor2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        xor2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl2 instruction
TEST_F(IrBuilderTest, Shl2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        shl2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl2.64 instruction
TEST_F(IrBuilderTest, Shl2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        shl2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr2 instruction
TEST_F(IrBuilderTest, Shr2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        shr2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr2.64 instruction
TEST_F(IrBuilderTest, Shr2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        shr2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shr).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr2 instruction
TEST_F(IrBuilderTest, Ashr2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        ashr2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AShr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr2.64 instruction
TEST_F(IrBuilderTest, Ashr2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        ashr2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AShr).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div2 instruction
TEST_F(IrBuilderTest, Div2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        div2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div2.64 instruction
TEST_F(IrBuilderTest, Div2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        div2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).s64().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod2 instruction
TEST_F(IrBuilderTest, Mod2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        lda a0
        mod2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod2.64 instruction
TEST_F(IrBuilderTest, Mod2i64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1) {
        lda.64 a0
        mod2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).s64().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).s64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divu2 instruction
TEST_F(IrBuilderTest, Divu2)
{
    auto source = R"(
    .function u32 main(u32 a0, u32 a1) {
        lda a0
        divu2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).u32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).u32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divu2.64 instruction
TEST_F(IrBuilderTest, Divu2u64)
{
    auto source = R"(
    .function u64 main(u64 a0, u64 a1) {
        lda.64 a0
        divu2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).u64().Inputs(1U, 2U);
            INST(4U, Opcode::Div).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modu2 instruction
TEST_F(IrBuilderTest, Modu2)
{
    auto source = R"(
    .function u32 main(u32 a0, u32 a1) {
        lda a0
        modu2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).u32().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).u32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modu2.64 instruction
TEST_F(IrBuilderTest, Modu2u64)
{
    auto source = R"(
    .function u64 main(u64 a0, u64 a1) {
        lda.64 a0
        modu2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::ZeroCheck).u64().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add instruction
TEST_F(IrBuilderTest, Add)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        add a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub instruction
TEST_F(IrBuilderTest, Sub)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        sub a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul instruction
TEST_F(IrBuilderTest, Mul)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        mul a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and instruction
TEST_F(IrBuilderTest, And)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        and a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or instruction
TEST_F(IrBuilderTest, Or)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        or a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xor instruction
TEST_F(IrBuilderTest, Xor)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        xor a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl instruction
TEST_F(IrBuilderTest, Shl)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        shl a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr instruction
TEST_F(IrBuilderTest, Shr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        shr a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr instruction
TEST_F(IrBuilderTest, Ashr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        ashr a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AShr).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div instruction
TEST_F(IrBuilderTest, Div)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        div a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod instruction
TEST_F(IrBuilderTest, Mod)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1) {
        mod a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the addi instruction
TEST_F(IrBuilderTest, Addi)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        addi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the subi instruction
TEST_F(IrBuilderTest, Subi)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        subi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the muli instruction
TEST_F(IrBuilderTest, Muli)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        muli 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Mul).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the andi instruction
TEST_F(IrBuilderTest, Andi)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        andi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::And).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ori instruction
TEST_F(IrBuilderTest, Ori)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        ori 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Or).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xori instruction
TEST_F(IrBuilderTest, Xori)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        xori 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Xor).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shli instruction
TEST_F(IrBuilderTest, Shli)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        shli 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Shl).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shri instruction
TEST_F(IrBuilderTest, Shri)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        shri 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Shr).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashri instruction
TEST_F(IrBuilderTest, Ashri)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        ashri 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::AShr).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divi instruction
TEST_F(IrBuilderTest, Divi)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        divi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 0U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modi instruction
TEST_F(IrBuilderTest, Modi)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        modi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 0U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Mod).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fneg instruction
TEST_F(IrBuilderTest, Fneg)
{
    auto source = R"(
    .function f32 main(f32 a0) {
        lda a0
        fneg
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Neg).f32().Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fneg.64 instruction
TEST_F(IrBuilderTest, Fneg64)
{
    auto source = R"(
    .function f64 main(f64 a0) {
        lda a0
        fneg.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Neg).f64().Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the neg instruction
TEST_F(IrBuilderTest, Neg)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        neg
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Neg).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the neg.64 instruction
TEST_F(IrBuilderTest, Neg64)
{
    auto source = R"(
    .function i64 main(i64 a0) {
        lda a0
        neg.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Neg).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the not instruction
TEST_F(IrBuilderTest, Not)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        not
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Not).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the not.64 instruction
TEST_F(IrBuilderTest, Not64)
{
    auto source = R"(
    .function i64 main(i64 a0) {
        lda a0
        not.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Not).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the inci instruction
TEST_F(IrBuilderTest, Inci)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        inci a0, 1
        lda a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32tof32 instruction
TEST_F(IrBuilderTest, I32tof32)
{
    auto source = R"(
    .function f32 main(i32 a0) {
        lda a0
        i32tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32tof64 instruction
TEST_F(IrBuilderTest, I32tof64)
{
    auto source = R"(
    .function f64 main(i32 a0) {
        lda a0
        i32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32tof32 instruction
TEST_F(IrBuilderTest, U32tof32)
{
    auto source = R"(
    .function f32 main(u32 a0) {
        lda a0
        u32tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::UINT32).Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32tof64 instruction
TEST_F(IrBuilderTest, U32tof64)
{
    auto source = R"(
    .function f64 main(u32 a0) {
        lda a0
        u32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(DataType::UINT32).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64tof32 instruction
TEST_F(IrBuilderTest, I64tof32)
{
    auto source = R"(
    .function f32 main(i64 a0) {
        lda.64 a0
        i64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64tof64 instruction
TEST_F(IrBuilderTest, I64tof64)
{
    auto source = R"(
    .function f64 main(i64 a0) {
        lda.64 a0
        i64tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u64tof32 instruction
TEST_F(IrBuilderTest, U64tof32)
{
    auto source = R"(
    .function f32 main(u64 a0) {
        lda.64 a0
        u64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::UINT64).Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u64tof64 instruction
TEST_F(IrBuilderTest, U64tof64)
{
    auto source = R"(
    .function f64 main(u64 a0) {
        lda.64 a0
        u64tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(DataType::UINT64).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32toi32 instruction
TEST_F(IrBuilderTest, F32toi32)
{
    auto source = R"(
    .function i32 main(f32 a0) {
        lda a0
        f32toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(DataType::FLOAT32).Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32toi64 instruction
TEST_F(IrBuilderTest, F32toi64)
{
    auto source = R"(
    .function i64 main(f32 a0) {
        lda a0
        f32toi64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s64().SrcType(DataType::FLOAT32).Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tou32 instruction
TEST_F(IrBuilderTest, F32tou32)
{
    auto source = R"(
    .function u32 main(f32 a0) {
        lda a0
        f32tou32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u32().SrcType(DataType::FLOAT32).Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tou64 instruction
TEST_F(IrBuilderTest, F32tou64)
{
    auto source = R"(
    .function u64 main(f32 a0) {
        lda a0
        f32tou64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(0U);
            INST(2U, Opcode::Return).u64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tof64 instruction
TEST_F(IrBuilderTest, F32tof64)
{
    auto source = R"(
    .function f64 main(f32 a0) {
        lda a0
        f32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(DataType::FLOAT32).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64toi32 instruction
TEST_F(IrBuilderTest, F64toi32)
{
    auto source = R"(
    .function i32 main(f64 a0) {
        lda.64 a0
        f64toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64toi64 instruction
TEST_F(IrBuilderTest, F64toi64)
{
    auto source = R"(
    .function i64 main(f64 a0) {
        lda.64 a0
        f64toi64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s64().SrcType(DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tou32 instruction
TEST_F(IrBuilderTest, F64tou32)
{
    auto source = R"(
    .function u32 main(f64 a0) {
        lda.64 a0
        f64tou32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tou64 instruction
TEST_F(IrBuilderTest, F64tou64)
{
    auto source = R"(
    .function u64 main(f64 a0) {
        lda.64 a0
        f64tou64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).u64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tof32 instruction
TEST_F(IrBuilderTest, F64tof32)
{
    auto source = R"(
    .function f32 main(f64 a0) {
        lda.64 a0
        f64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).f32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32toi64 instruction
TEST_F(IrBuilderTest, I32toi64)
{
    auto source = R"(
    .function i64 main(i32 a0) {
        lda a0
        i32toi64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s64().SrcType(DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64toi32 instruction
TEST_F(IrBuilderTest, I64toi32)
{
    auto source = R"(
    .function i32 main(i64 a0) {
        lda.64 a0
        i64toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32toi64 instruction
TEST_F(IrBuilderTest, U32toi64)
{
    auto source = R"(
    .function i64 main(u32 a0) {
        lda a0
        u32toi64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s64().SrcType(DataType::UINT32).Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.8 instruction
TEST_F(IrBuilderTest, Ldarr8)
{
    auto source = R"(
    .function i8 main(i32 a0, i8[] a1) {
        lda a0
        ldarr.8 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s8().Inputs(3U, 5U);
            INST(7U, Opcode::Return).s8().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarru.8 instruction
TEST_F(IrBuilderTest, Ldarru8)
{
    auto source = R"(
    .function u8 main(i32 a0, u8[] a1) {
        lda a0
        ldarru.8 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).u8().Inputs(3U, 5U);
            INST(7U, Opcode::Return).u8().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.16 instruction
TEST_F(IrBuilderTest, Ldarr16)
{
    auto source = R"(
    .function i16 main(i32 a0, i16[] a1) {
        lda a0
        ldarr.16 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s16().Inputs(3U, 5U);
            INST(7U, Opcode::Return).s16().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarru.16 instruction
TEST_F(IrBuilderTest, Ldarru16)
{
    auto source = R"(
    .function u16 main(i32 a0, u16[] a1) {
        lda a0
        ldarru.16 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).u16().Inputs(3U, 5U);
            INST(7U, Opcode::Return).u16().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr instruction
TEST_F(IrBuilderTest, Ldarr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32[] a1) {
        lda a0
        ldarr a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s32().Inputs(3U, 5U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fldarr.32 instruction
TEST_F(IrBuilderTest, Fldarr32)
{
    auto source = R"(
    .function f32 main(i32 a0, f32[] a1) {
        lda a0
        fldarr.32 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).f32().Inputs(3U, 5U);
            INST(8U, Opcode::Return).f32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.64 instruction
TEST_F(IrBuilderTest, Ldarr64)
{
    auto source = R"(
    .function u64 main(i32 a0, u64[] a1) {
        lda a0
        ldarr.64 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s64().Inputs(3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fldarr.64 instruction
TEST_F(IrBuilderTest, Fldarr64)
{
    auto source = R"(
    .function f64 main(i32 a0, f64[] a1) {
        lda a0
        fldarr.64 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).f64().Inputs(3U, 5U);
            INST(7U, Opcode::Return).f64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.obj instruction
TEST_F(IrBuilderTest, LdarrObj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(i32 a0, panda.String[] a1) {
        lda a0
        ldarr.obj a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).ref().Inputs(3U, 5U);
            INST(7U, Opcode::Return).ref().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.8 instruction
TEST_F(IrBuilderTest, Starr8)
{
    auto source = R"(
    .function void main(i32 a0, u8[] a1, u8 a2) {
        lda a2
        starr.8 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).u8();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(7U, Opcode::StoreArray).s8().Inputs(4U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.16 instruction
TEST_F(IrBuilderTest, Starr16)
{
    auto source = R"(
    .function void main(i32 a0, u16[] a1, u16 a2) {
        lda a2
        starr.16 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).u16();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(7U, Opcode::StoreArray).s16().Inputs(4U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr instruction
TEST_F(IrBuilderTest, Starr)
{
    auto source = R"(
    .function void main(i32 a0, i32[] a1, i32 a2) {
        lda a2
        starr a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(7U, Opcode::StoreArray).s32().Inputs(4U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fstarr.32 instruction
TEST_F(IrBuilderTest, Fstarr32)
{
    auto source = R"(
    .function void main(i32 a0, f32[] a1, f32 a2) {
        lda a2
        fstarr.32 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(8U, Opcode::StoreArray).f32().Inputs(4U, 6U, 2U);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.64 instruction
TEST_F(IrBuilderTest, Starr64)
{
    auto source = R"(
    .function void main(i32 a0, i64[] a1, i64 a2) {
        lda.64 a2
        starr.64 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(7U, Opcode::StoreArray).s64().Inputs(4U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fstarr.64 instruction
TEST_F(IrBuilderTest, Fstarr64)
{
    auto source = R"(
    .function void main(i32 a0, f64[] a1, f64 a2) {
        lda.64 a2
        fstarr.64 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(7U, Opcode::StoreArray).f64().Inputs(4U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.obj instruction
TEST_F(IrBuilderTest, StarrObj)
{
    auto source = R"(
    .record panda.String <external>
    .function void main(i32 a0, panda.String[] a1, panda.String a2) {
        lda.obj a2
        starr.obj a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(1U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 0U, 3U);
            INST(9U, Opcode::RefTypeCheck).ref().Inputs(4U, 2U, 3U);
            INST(7U, Opcode::StoreArray).ref().Inputs(4U, 6U, 9U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the lenarr instruction
TEST_F(IrBuilderTest, Lenarr)
{
    auto source = R"(
    .function i32 main(i32[] a0) {
        lenarr a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LenArray).s32().Inputs(2U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the newarr instruction
TEST_F(IrBuilderTest, Newarr)
{
    auto source = R"(
    .function i32[] main(i32 a0) {
        newarr a0, a0, i32[]
        lda.obj a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(2U, Opcode::NegativeCheck).s32().Inputs(0U, 1U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of lda.const instruction
TEST_F(IrBuilderTest, LdaConst)
{
    auto source = R"(
.array array0 panda.String 3 { "a" "ab" "abc"}
.array array1 u1 3 { 0 1 0}
.array array2 i32 3 { 2 3 4}
.array array3 f32 3 { 5.0 6.0 7.0 }

.function void main() {
lda.const v0, array0
lda.const v1, array1
lda.const v2, array2
lda.const v3, array3
return.void
}
)";
    auto defaultOption = g_options.GetCompilerUnfoldConstArrayMaxSize();
    g_options.SetCompilerUnfoldConstArrayMaxSize(2U);
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(40U, 3U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            // string array
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadConstArray).ref().Inputs(0U);

            // bool array
            INST(12U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(12U).TypeId(68U);
            INST(13U, Opcode::NegativeCheck).s32().Inputs(40U, 12U);
            INST(14U, Opcode::NewArray).ref().Inputs(44U, 13U, 12U);

            INST(15U, Opcode::SaveState).Inputs(14U).SrcVregs({1U});
            INST(16U, Opcode::FillConstArray).s8().Inputs(14U, 15U);

            // int array
            INST(22U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(45U, Opcode::LoadAndInitClass).ref().Inputs(22U).TypeId(68U);
            INST(23U, Opcode::NegativeCheck).s32().Inputs(40U, 22U);
            INST(24U, Opcode::NewArray).ref().Inputs(45U, 23U, 22U);

            INST(25U, Opcode::SaveState).Inputs(24U).SrcVregs({2U});
            INST(26U, Opcode::FillConstArray).s32().Inputs(24U, 25U);

            // float array
            INST(32U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(46U, Opcode::LoadAndInitClass).ref().Inputs(32U).TypeId(68U);
            INST(33U, Opcode::NegativeCheck).s32().Inputs(40U, 32U);
            INST(34U, Opcode::NewArray).ref().Inputs(46U, 33U, 32U);

            INST(35U, Opcode::SaveState).Inputs(34U).SrcVregs({3U});
            INST(36U, Opcode::FillConstArray).f32().Inputs(34U, 35U);

            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    g_options.SetCompilerUnfoldConstArrayMaxSize(defaultOption);
}

// Checks the build of unfolded lda.const instruction
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(IrBuilderTest, LdaConstUnfold)
{
    auto source = R"(
.array array0 panda.String 2 { "a" "ab" }
.array array1 u1 2 { 0 1 }
.array array2 i32 3 { 2 3 -4}
.array array3 f32 2 { 4.0 5.0 }

.function void main() {
lda.const v0, array0
lda.const v1, array1
lda.const v2, array2
lda.const v3, array3
return.void
}
)";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, 2U).s64();
        CONSTANT(4U, 0U).s64();
        CONSTANT(13U, 1U).s64();
        CONSTANT(43U, 3U).s64();
        CONSTANT(590U, -4L).s64();
        CONSTANT(52U, 4.0F).f32();
        CONSTANT(59U, 5.0F).f32();

        BASIC_BLOCK(2U, -1L)
        {
            // string array
            INST(1U, Opcode::SaveState).NoVregs();
            INST(444U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(2U, Opcode::NegativeCheck).s32().Inputs(0U, 1U);
            INST(3U, Opcode::NewArray).ref().Inputs(444U, 2U, 1U);

            INST(5U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(6U, Opcode::LoadString).ref().Inputs(5U);
            INST(7U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(9U, Opcode::LenArray).s32().Inputs(8U);
            INST(10U, Opcode::BoundsCheck).s32().Inputs(9U, 4U, 7U);
            INST(11U, Opcode::StoreArray).ref().Inputs(8U, 10U, 6U);

            INST(14U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(15U, Opcode::LoadString).ref().Inputs(14U);
            INST(16U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(17U, Opcode::NullCheck).ref().Inputs(3U, 16U);
            INST(18U, Opcode::LenArray).s32().Inputs(17U);
            INST(19U, Opcode::BoundsCheck).s32().Inputs(18U, 13U, 16U);
            INST(20U, Opcode::StoreArray).ref().Inputs(17U, 19U, 15U);

            // bool array
            INST(22U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(445U, Opcode::LoadAndInitClass).ref().Inputs(22U).TypeId(68U);
            INST(23U, Opcode::NegativeCheck).s32().Inputs(0U, 22U);
            INST(24U, Opcode::NewArray).ref().Inputs(445U, 23U, 22U);

            INST(25U, Opcode::SaveState).Inputs(24U).SrcVregs({1U});
            INST(26U, Opcode::NullCheck).ref().Inputs(24U, 25U);
            INST(27U, Opcode::LenArray).s32().Inputs(26U);
            INST(28U, Opcode::BoundsCheck).s32().Inputs(27U, 4U, 25U);
            INST(29U, Opcode::StoreArray).s8().Inputs(26U, 28U, 4U);

            INST(30U, Opcode::SaveState).Inputs(24U).SrcVregs({1U});
            INST(31U, Opcode::NullCheck).ref().Inputs(24U, 30U);
            INST(32U, Opcode::LenArray).s32().Inputs(31U);
            INST(33U, Opcode::BoundsCheck).s32().Inputs(32U, 13U, 30U);
            INST(34U, Opcode::StoreArray).s8().Inputs(31U, 33U, 13U);

            // int array
            INST(35U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(446U, Opcode::LoadAndInitClass).ref().Inputs(35U).TypeId(68U);
            INST(36U, Opcode::NegativeCheck).s32().Inputs(43U, 35U);
            INST(37U, Opcode::NewArray).ref().Inputs(446U, 36U, 35U);

            INST(38U, Opcode::SaveState).Inputs(37U).SrcVregs({2U});
            INST(39U, Opcode::NullCheck).ref().Inputs(37U, 38U);
            INST(40U, Opcode::LenArray).s32().Inputs(39U);
            INST(41U, Opcode::BoundsCheck).s32().Inputs(40U, 4U, 38U);
            INST(42U, Opcode::StoreArray).s32().Inputs(39U, 41U, 0U);

            INST(44U, Opcode::SaveState).Inputs(37U).SrcVregs({2U});
            INST(45U, Opcode::NullCheck).ref().Inputs(37U, 44U);
            INST(46U, Opcode::LenArray).s32().Inputs(45U);
            INST(47U, Opcode::BoundsCheck).s32().Inputs(46U, 13U, 44U);
            INST(48U, Opcode::StoreArray).s32().Inputs(45U, 47U, 43U);

            INST(440U, Opcode::SaveState).Inputs(37U).SrcVregs({2U});
            INST(450U, Opcode::NullCheck).ref().Inputs(37U, 440U);
            INST(460U, Opcode::LenArray).s32().Inputs(450U);
            INST(470U, Opcode::BoundsCheck).s32().Inputs(460U, 0U, 440U);
            INST(480U, Opcode::StoreArray).s32().Inputs(450U, 470U, 590U);

            // float array
            INST(49U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(447U, Opcode::LoadAndInitClass).ref().Inputs(49U).TypeId(68U);
            INST(50U, Opcode::NegativeCheck).s32().Inputs(0U, 49U);
            INST(51U, Opcode::NewArray).ref().Inputs(447U, 50U, 49U);

            INST(53U, Opcode::SaveState).Inputs(51U).SrcVregs({3U});
            INST(54U, Opcode::NullCheck).ref().Inputs(51U, 53U);
            INST(55U, Opcode::LenArray).s32().Inputs(54U);
            INST(56U, Opcode::BoundsCheck).s32().Inputs(55U, 4U, 53U);
            INST(57U, Opcode::StoreArray).f32().Inputs(54U, 56U, 52U);

            INST(60U, Opcode::SaveState).Inputs(51U).SrcVregs({3U});
            INST(61U, Opcode::NullCheck).ref().Inputs(51U, 60U);
            INST(62U, Opcode::LenArray).s32().Inputs(61U);
            INST(63U, Opcode::BoundsCheck).s32().Inputs(62U, 13U, 60U);
            INST(64U, Opcode::StoreArray).f32().Inputs(61U, 63U, 59U);

            INST(66U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the newobj instruction
TEST_F(IrBuilderTest, Newobj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main() {
        newobj v0, panda.String
        lda.obj v0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(0U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, Initobj)
{
    auto source = R"(
    .record R{
        i32 f
    }
    .function R main() {
        initobj R.ctor
        return
    }
    .function void R.ctor() <ctor> {
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U);
            INST(4U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(3U, Opcode::CallStatic).v0id().Inputs({{DataType::REFERENCE, 2U}, {DataType::NO_TYPE, 4U}});
            INST(5U, Opcode::Return).ref().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Enable after supporting MultiArray in panda assembly
TEST_F(IrBuilderTest, DISABLED_MultiArray)
{
    // NOTE(pishin): fix ctor before enabling
    auto source = R"(
    .record __I <external>
    .function ___I _init__i32_i32_(i32 a0, i32 a1) <ctor, external>
    .record panda.Array <external>
    .function panda.Array main(i32 a0) {
        movi v0, 0x1
        initobj _init__i32_i32_ v0, a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(4U, Opcode::MultiArray)
                .ref()
                .Inputs(
                    // CC-OFFNXT(G.FMT.06) project code style
                    {{DataType::REFERENCE, 3U}, {DataType::INT32, 1U}, {DataType::INT32, 0U}, {DataType::NO_TYPE, 2U}});
            INST(5U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldobj instruction
TEST_F(IrBuilderTest, Ldobj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32
        panda.String v_string
    }
    .function i32 main(R a0) {
        ldobj a0, R.v_i32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s32().Inputs(2U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldobj.64 instruction
TEST_F(IrBuilderTest, Ldobj64)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i64 v_i64
        panda.String v_string
    }
    .function i64 main(R a0) {
        ldobj.64 a0, R.v_i64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s64().Inputs(2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldobj.obj instruction
TEST_F(IrBuilderTest, LdobjObj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32
        panda.String v_string
    }
    .function panda.String main(R a0) {
        ldobj.obj a0, R.v_string
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).ref().Inputs(2U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the stobj instruction
TEST_F(IrBuilderTest, Stobj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32
        panda.String v_string
    }
    .function void main(R a0, i32 a1) {
        lda a1
        stobj a0, R.v_i32
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 1U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).s32().Inputs(3U, 1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the stobj instruction
TEST_F(IrBuilderTest, Stobj64)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i64 v_i64
        panda.String v_string
    }
    .function void main(R a0, i64 a1) {
        lda.64 a1
        stobj.64 a0, R.v_i64
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 1U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).s64().Inputs(3U, 1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the stobj.obj instruction
TEST_F(IrBuilderTest, StobjObj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32
        panda.String v_string
    }
    .function void main(R a0, panda.String a1) {
        lda.obj a1
        stobj.obj a0, R.v_string
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 1U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).ref().Inputs(3U, 1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldstatic instruction
TEST_F(IrBuilderTest, Ldstatic)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32             <static>
        panda.String v_string <static>
    }
    .function i32 main() {
        ldstatic R.v_i32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).SrcVregs({});
            INST(0U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(1U, Opcode::LoadStatic).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldstatic.64 instruction
TEST_F(IrBuilderTest, Ldstatic64)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i64 v_i64             <static>
        panda.String v_string <static>
    }
    .function i64 main() {
        ldstatic.64 R.v_i64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).SrcVregs({});
            INST(0U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(1U, Opcode::LoadStatic).s64().Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldstatic.obj instruction
TEST_F(IrBuilderTest, LdstaticObj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32             <static>
        panda.String v_string <static>
    }
    .function panda.String main() {
        ldstatic.obj R.v_string
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).SrcVregs({});
            INST(0U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(1U, Opcode::LoadStatic).ref().Inputs(0U);
            INST(2U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ststatic instruction
TEST_F(IrBuilderTest, Ststatic)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32             <static>
        panda.String v_string <static>
    }
    .function void main(i32 a0) {
        lda a0
        ststatic R.v_i32
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 0U).SrcVregs({0U, 1U});
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(2U, Opcode::StoreStatic).s32().Inputs(1U, 0U);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ststatic.64 instruction
TEST_F(IrBuilderTest, Ststatic64)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i64 v_i64             <static>
        panda.String v_string <static>
    }
    .function void main(i64 a0) {
        lda.64 a0
        ststatic.64 R.v_i64
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 0U).SrcVregs({0U, 1U});
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(2U, Opcode::StoreStatic).s64().Inputs(1U, 0U);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ststatic.obj instruction
TEST_F(IrBuilderTest, StstaticObj)
{
    auto source = R"(
    .record panda.String <external>
    .record R {
        i32 v_i32             <static>
        panda.String v_string <static>
    }
    .function void main(panda.String a0) {
        lda.obj a0
        ststatic.obj R.v_string
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 0U).SrcVregs({0U, 1U});
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(2U, Opcode::StoreStatic).ref().Inputs(1U, 0U);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return instruction
TEST_F(IrBuilderTest, Return)
{
    auto source = R"(
    .function i32 main(i32 a0) {
        lda a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.64 instruction
TEST_F(IrBuilderTest, Return64)
{
    auto source = R"(
    .function i64 main(i64 a0) {
        lda.64 a0
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).s64().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.obj instruction
TEST_F(IrBuilderTest, ReturnObj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(panda.String a0) {
        lda.obj a0
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.void instruction
TEST_F(IrBuilderTest, ReturnVoid)
{
    auto source = R"(
    .function void main() {
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the throw instruction
TEST_F(IrBuilderTest, Throw)
{
    auto source = R"(
    .record panda.String <external>
    .function void main(panda.String a0) {
        throw a0
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::Throw).Inputs(0U, 1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call.short instruction
TEST_F(IrBuilderTest, CallShort)
{
    auto source = R"(
    .function i32 main() {
        movi.64 v0, 1
        call.short foo1, v0, v0
        call.short foo2, v0, v0
        return
    }
    .function i32 foo1(i64 a0) {
        ldai 0
        return
    }
    .function i32 foo2(i64 a0, i64 a1) {
        ldai 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            using namespace DataType;  // NOLINT(*-build-using-namespace)
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::CallStatic).s32().Inputs({{INT64, 0U}, {NO_TYPE, 1U}});
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::CallStatic).s32().Inputs({{INT64, 0U}, {INT64, 0U}, {NO_TYPE, 3U}});
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call instruction
TEST_F(IrBuilderTest, Call)
{
    auto source = R"(
    .function i64 main() {
        movi.64 v0, 1
        call foo1, v0, v0, v0, v0
        call foo2, v0, v0, v0, v0
        return
    }
    .function i64 foo1(i32 a0) {
        ldai.64 0
        return
    }
    .function i64 foo2(i32 a0, i32 a1) {
        ldai.64 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            using namespace DataType;  // NOLINT(*-build-using-namespace)
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::CallStatic).s64().Inputs({{INT32, 0U}, {NO_TYPE, 1U}});
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::CallStatic).s64().Inputs({{INT32, 0U}, {INT32, 0U}, {NO_TYPE, 3U}});
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call.range instruction
TEST_F(IrBuilderTest, CallRange)
{
    auto source = R"(
    .function i64 main() {
        movi.64 v0, 1
        movi.64 v1, 2
        call.range foo, v0
        return
    }
    .function i64 foo(i32 a0) {
        ldai.64 0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::CallStatic).s64().Inputs({{DataType::INT32, 0U}, {DataType::NO_TYPE, 2U}});
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, Checkcast)
{
    auto source = R"(
    .record Asm{
        i32 asm1
        i64 asm2
    }
    .function void main() {
        newobj v0, Asm
        lda.obj v0
        checkcast Asm
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(1U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(2U, Opcode::SaveState).Inputs(1U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::LoadClass).ref().Inputs(2U);
            INST(3U, Opcode::CheckCast).Inputs(1U, 7U, 2U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, Isinstance)
{
    auto source = R"(
    .record Asm{
        i32 asm1
        i64 asm2
    }
    .function u1 main() {
        newobj v0, Asm
        lda.obj v0
        isinstance Asm
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(1U, Opcode::NewObject).ref().Inputs(5U, 4U);
            INST(6U, Opcode::SaveState).Inputs(1U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::LoadClass).ref().Inputs(6U);
            INST(2U, Opcode::IsInstance).b().Inputs(1U, 7U, 6U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

OUT_GRAPH(SimpleTryCatch, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 2U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0U);

        BASIC_BLOCK(2U, 3U, 15U, 16U)  // Try-begin
        {
            INST(5U, Opcode::Try).CatchTypeIds({0x0U, 0xE1U});
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(14U);
        }
        BASIC_BLOCK(4U, 7U, 15U, 16U) {}  // Try-end
        BASIC_BLOCK(7U, -1L)
        {
            INST(12U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(15U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(16U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(13U, Opcode::Return).b().Inputs(1U);
        }
    }
}

TEST_F(IrBuilderTest, SimpleTryCatch)
{
    auto source = R"(
    .record E1 {}

    .function void foo() {
    }

    .function u1 main() {
    try_begin:
        call foo
        ldai 2
    try_end:
        jmp exit

    catch_block1_begin:
        ldai 0
        return

    catch_block2_begin:
        ldai 1
        return

    exit:
        return

    .catchall try_begin, try_end, catch_block1_begin
    .catch E1, try_begin, try_end, catch_block2_begin
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::SimpleTryCatch::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(TryCatchFinally, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 3U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 7U, 4U, 5U)
        {
            INST(3U, Opcode::Try).CatchTypeIds({0x0U, 0xE1U});
        }
        BASIC_BLOCK(7U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(4U);
        }
        BASIC_BLOCK(3U, 6U, 4U, 5U) {}  // Try-end
        BASIC_BLOCK(4U, 6U) {}
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{3U, 0U}, {4U, 2U}, {5U, 1U}});
            INST(12U, Opcode::Sub).s32().Inputs(11U, 0U);
            INST(13U, Opcode::Return).b().Inputs(12U);
        }
    }
}

TEST_F(IrBuilderTest, TryCatchFinally)
{
    auto source = R"(
    .record E1 {}

    .function void foo() {
    }

    .function u1 main() {
    try_begin:
        call foo
        ldai 1
    try_end:
        jmp label

    catch_block1_begin:
        ldai 2
        jmp label

    catch_block2_begin:
        ldai 3

    label:
        subi 1
        return

    .catchall try_begin, try_end, catch_block1_begin
    .catch E1, try_begin, try_end, catch_block2_begin
    }

    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::TryCatchFinally::CREATE(expectedGraph);
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(CatchPhis, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 100U);

        BASIC_BLOCK(2U, 3U, 14U, 15U)
        {
            INST(2U, Opcode::Try).CatchTypeIds({0xE1U, 0x0U});
        }
        BASIC_BLOCK(3U, 6U)
        {
            INST(3U, Opcode::SaveState).Inputs(1U, 0U, 1U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::ZeroCheck).s64().Inputs(0U, 3U);
            INST(5U, Opcode::Div).s64().Inputs(1U, 4U);
            INST(6U, Opcode::SaveState).Inputs(5U, 0U, 5U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::ZeroCheck).s64().Inputs(0U, 6U);
            INST(8U, Opcode::Div).s64().Inputs(5U, 7U);
        }
        BASIC_BLOCK(6U, 7U, 14U, 15U) {}  // Try-end
        BASIC_BLOCK(7U, -1L)
        {
            INST(9U, Opcode::Return).s64().Inputs(8U);
        }
        BASIC_BLOCK(14U, 24U)  // Catch-begin
        {
            INST(10U, Opcode::CatchPhi).s64().Inputs(1U, 1U, 5U, 5U);
        }
        BASIC_BLOCK(24U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(10U);
        }
        BASIC_BLOCK(15U, 25U)  // Catch-begin
        {
            INST(12U, Opcode::CatchPhi).s64().Inputs(1U, 1U, 5U, 5U);
        }
        BASIC_BLOCK(25U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(12U);
        }
    }
}

TEST_F(IrBuilderTest, CatchPhis)
{
    auto source = R"(
    .record E1 {}
    .record A {}

    .function i64 main(i64 a0) {
    try_begin:
        movi.64 v0, 100
        lda.64 v0
        div2.64 a0
        sta.64 v0
        div2.64 a0
    try_end:
        jmp exit

    catch_block1_begin:
        lda.64 v0
        return

    catch_block2_begin:
        lda.64 v0
        return

    exit:
        return

    .catch E1, try_begin, try_end, catch_block1_begin
    .catchall try_begin, try_end, catch_block2_begin
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::CatchPhis::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(NestedTryCatch, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)  // Try-begin
        {
            INST(4U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 10U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 0U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::ZeroCheck).s32().Inputs(1U, 5U);
            INST(7U, Opcode::Div).s32().Inputs(0U, 6U);
        }
        BASIC_BLOCK(10U, 9U, 4U) {}  // Try-end
        BASIC_BLOCK(4U, 5U) {}       // Catch-begin
        BASIC_BLOCK(5U, 6U, 17U)     // Try-begin
        {
            INST(11U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(6U, 11U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 0U, 2U).SrcVregs({0U, 2U, 3U});
            INST(13U, Opcode::ZeroCheck).s32().Inputs(2U, 12U);
            INST(14U, Opcode::Div).s32().Inputs(0U, 13U);
        }
        BASIC_BLOCK(11U, 9U, 17U) {}  // Try-end
        BASIC_BLOCK(17U, 7U) {}
        BASIC_BLOCK(7U, 9U)
        {
            INST(16U, Opcode::Add).s32().Inputs(0U, 3U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(17U, Opcode::Phi).s32().Inputs(7U, 14U, 16U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }
}

TEST_F(IrBuilderTest, NestedTryCatch)
{
    auto source = R"(
    .record panda.ArithmeticException <external>

    .function i32 main(i32 a0, i32 a1, i32 a2) {
    try_begin:
        lda a0
        div2 a1
    try_end:
        jmp lbl

    catch_block:
    try_begin1:
        lda a0
        div2 a2
    try_end1:
        jmp lbl

    catch_block1:
        lda a0
        addi 1

    lbl:
        return

    .catch panda.ArithmeticException, try_begin, try_end, catch_block
    .catch panda.ArithmeticException, try_begin1, try_end1, catch_block1
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::NestedTryCatch::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

TEST_F(IrBuilderTest, EmptyCatchBlock)
{
    auto source = R"(
    .record panda.ArithmeticException <external>

    .function i32 main(i32 a0, i32 a1) {
    try_begin:
        lda a0
        div2 a1
        sta a0
    try_end:
        jmp lbl

    catch_block:
    lbl:
        lda a0
        addi 1
        return

    .catch panda.ArithmeticException, try_begin, try_end, catch_block
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));

    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        PARAMETER(4U, 0U).s32();
        PARAMETER(5U, 1U).s32();
        CONSTANT(13U, 1U);

        BASIC_BLOCK(2U, 10U, 7U)
        {
            INST(10U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(10U, 3U)
        {
            INST(6U, Opcode::SaveState).Inputs(4U, 5U, 4U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::ZeroCheck).s32().Inputs(5U, 6U);
            INST(8U, Opcode::Div).s32().Inputs(4U, 7U);
        }
        BASIC_BLOCK(3U, 11U, 7U) {}  // Try-end
        BASIC_BLOCK(7U, 11U) {}
        BASIC_BLOCK(11U, -1L)
        {
            INST(9U, Opcode::Phi).s32().Inputs(8U, 4U);
            INST(12U, Opcode::Add).s32().Inputs(9U, 13U);
            INST(14U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

TEST_F(IrBuilderTest, EmptyTryBlock)
{
    auto source = R"(
    .record panda.ArithmeticException <external>

    .function i32 main(i32 a0, i32 a1) {
    try_begin1:
    try_end1:
        jmp lbl1

    catch_block1:
        lda a0
        return

    try_begin2:
    try_end2:
    lbl1:
        jmp lbl2

    catch_block2:
        lda a1
        return

    lbl2:
        ldai 0
        return

    .catchall try_begin1, try_end1, catch_block1
    .catchall try_begin2, try_end2, catch_block2
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        CONSTANT(2U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(CatchBlockWithCycle, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 14U)
        {
            INST(4U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 8U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::ZeroCheck).s32().Inputs(1U, 5U);
            INST(7U, Opcode::Div).s32().Inputs(0U, 6U);
        }
        BASIC_BLOCK(8U, 5U, 14U) {}  // Try-end
        BASIC_BLOCK(14U, 4U) {}
        BASIC_BLOCK(4U, 6U)
        {
            INST(18U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).SetFlag(inst_flags::NO_DCE);
            INST(16U, Opcode::SaveStateDeoptimize).Inputs(0U, 0U).SrcVregs({0U, 2U});
        }
        BASIC_BLOCK(6U, 5U, 7U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 12U);
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(9U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(7U, 6U)
        {
            INST(12U, Opcode::Sub).s32().Inputs(9U, 3U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::Phi).s32().Inputs(7U, 9U);
            INST(17U, Opcode::SaveState).Inputs(13U).SrcVregs({2U}).SetFlag(inst_flags::NO_DCE);
            INST(14U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

TEST_F(IrBuilderTest, CatchBlockWithCycle)
{
    auto source = R"(
    .record panda.ArithmeticException <external>

    .function i32 main(i32 a0, i32 a1) {
    try_begin:
        lda a0
        div2 a1
        sta a0
    try_end:
        jmp exit

    catch_block:
        lda a0
    loop:
        jeqz exit
        subi 1
        jmp loop

    exit:
        return

    .catch panda.ArithmeticException, try_begin, try_end, catch_block
    }
    )";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::CatchBlockWithCycle::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(DeadBlocksAfterThrow, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(11U, 1U);
        CONSTANT(13U, 0U);

        BASIC_BLOCK(5U, 6U)
        {
            INST(9U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(10U, Opcode::LoadAndInitClass).ref().Inputs(9U);
            INST(2U, Opcode::NewObject).ref().Inputs(10U, 9U);
        }
        BASIC_BLOCK(6U, 2U, 12U)
        {
            INST(1U, Opcode::Try).CatchTypeIds({0xE1U});
        }
        BASIC_BLOCK(2U, 7U)  // try block
        {
            INST(3U, Opcode::SaveState).Inputs(2U).SrcVregs({1U});
            INST(4U, Opcode::Throw).Inputs(2U, 3U);
        }
        BASIC_BLOCK(7U, -1L, 12U) {}  // try_end block
        BASIC_BLOCK(12U, 11U)
        {
            INST(6U, Opcode::CatchPhi).ref().Inputs();  // catch-phi for acc, which holds exception-object
        }
        BASIC_BLOCK(11U, 3U, 4U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(6U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::Return).s32().Inputs(11U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

TEST_F(IrBuilderTest, DeadBlocksAfterThrow)
{
    auto source = R"(
.record E1 {}

.function i32 main() {
    newobj v1, E1
begin1:
    throw v1
end1:
    ldai 0
loop:
    addi 1
    jnez loop
    return

catch1:
    jeq.obj v1, ok
    ldai 1
    return
ok:
    ldai 0
    return

.catch E1, begin1, end1, catch1
}
)";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::DeadBlocksAfterThrow::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(FallthroughBeforeTryBlockEnd, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(6U, 0U).s32();
        PARAMETER(7U, 1U).s32();
        PARAMETER(8U, 3U).s32();
        CONSTANT(13U, 0U);
        CONSTANT(19U, 1U);

        BASIC_BLOCK(4U, 90U, 14U)  // Try-begin
        {
            INST(0U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(90U, 5U)
        {
            INST(9U, Opcode::SaveState).Inputs(6U, 7U, 8U, 6U).SrcVregs({0U, 1U, 2U, 3U});
            INST(10U, Opcode::ZeroCheck).s32().Inputs(7U, 9U);
            INST(11U, Opcode::Div).s32().Inputs(6U, 10U);
        }
        BASIC_BLOCK(5U, 8U, 14U) {}  // Try-end
        BASIC_BLOCK(8U, 91U, 6U)
        {
            INST(12U, Opcode::Compare).b().CC(CC_EQ).Inputs(11U, 13U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(6U, 9U, 14U)  // Try-begin
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(9U, 7U)  // Try
        {
            INST(15U, Opcode::SaveState).Inputs(6U, 6U, 8U).SrcVregs({0U, 3U, 2U});
            INST(16U, Opcode::ZeroCheck).s32().Inputs(8U, 15U);
            INST(17U, Opcode::Div).s32().Inputs(6U, 16U);
        }
        BASIC_BLOCK(7U, 10U, 14U) {}  // Try-end
        BASIC_BLOCK(14U, 11U) {}      // Catch-begin
        BASIC_BLOCK(10U, 91U, 3U)
        {
            INST(20U, Opcode::Compare).b().CC(CC_EQ).Inputs(17U, 13U);
            INST(21U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(20U);
        }
        BASIC_BLOCK(3U, 91U) {}
        BASIC_BLOCK(11U, 91U)  // Catch
        {
            INST(18U, Opcode::Add).s32().Inputs(6U, 19U);
        }
        BASIC_BLOCK(91U, -1L)
        {
            INST(25U, Opcode::Phi).s32().Inputs(11U, 17U, 19U, 18U);
            INST(26U, Opcode::Return).s32().Inputs(25U);
        }
    }
}

TEST_F(IrBuilderTest, FallthroughBeforeTryBlockEnd)
{
    auto source = R"(
.record panda.NullPointerException {}

.function i32 main(i32 a0, i32 a1, i32 a2) {
begin1:
    lda a0
    div2 a1
end1:
    jeqz exit
    lda a0
begin2:
    div2 a2
end2:
    jeqz exit
    ldai 1
    jmp exit
catch1:
    lda a0
    addi 1
    jmp exit
exit:
    return

.catch panda.NullPointerException, begin1, end1, catch1
.catch panda.NullPointerException, begin2, end2, catch1
}
)";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::FallthroughBeforeTryBlockEnd::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(CatchWithFallthrough, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 14U)  // Try-begin
        {
            INST(4U, Opcode::Try).CatchTypeIds({0xE1U});
        }
        BASIC_BLOCK(3U, 9U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::ZeroCheck).s32().Inputs(1U, 5U);
            INST(7U, Opcode::Div).s32().Inputs(0U, 6U);
        }
        BASIC_BLOCK(9U, 8U, 14U) {}  // Try-end
        BASIC_BLOCK(14U, 4U) {}
        BASIC_BLOCK(4U, 6U, 7U)
        {
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(2U);
        }
        BASIC_BLOCK(7U, 8U)
        {
            INST(13U, Opcode::Sub).s32().Inputs(0U, 3U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(14U, Opcode::Phi).s32().Inputs(7U, 13U);
            INST(15U, Opcode::Return).s32().Inputs(14U);
        }
    }
}

TEST_F(IrBuilderTest, CatchWithFallthrough)
{
    auto source = R"(
.record E1 {}

.function i32 main(i32 a0, i32 a1) {
begin1:
    lda a0
    div2 a1
    sta a0
end1:
    jmp exit

catch1:
    lda a0
    jeqz label
    subi 1
    jmp exit
label:
    ldai 0
    return

exit:
    return

.catch E1, begin1, end1, catch1
}
)";

    // build IR with try-catch
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::CatchWithFallthrough::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

TEST_F(IrBuilderTest, OsrMode)
{
    auto source = R"(
    .function u32 foo(u32 a0) {
        lda a0
    loop:
        jlez exit
        subi 1
        jmp loop
    exit:
        return
    }
    )";

    auto graph = CreateGraphOsr();
    ASSERT_TRUE(ParseToGraph(source, "foo", graph));
    EXPECT_TRUE(graph->RunPass<Cleanup>(false));

    auto expectedGraph = CreateGraphOsrWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(4U, Opcode::SaveStateOsr).Inputs(3U).SrcVregs({1U});
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(3U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).u32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

TEST_F(IrBuilderTest, TestSaveStateDeoptimize)
{
    auto source = R"(
    .function u32 foo(u32 a0) {
        lda a0
    loop:
        jlez exit
        subi 1
        jmp loop
    exit:
        return
    }
    )";

    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph(source, "foo", graph));
    EXPECT_TRUE(graph->RunPass<Cleanup>());
    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).SetFlag(inst_flags::NO_DCE);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(0U, 0U).SrcVregs({0U, 1U});
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(3U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(3U).SrcVregs({1U}).SetFlag(inst_flags::NO_DCE);
            INST(8U, Opcode::Return).u32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

TEST_F(IrBuilderTest, InfiniteLoop)
{
    auto source = R"(
    .function u32 foo_inf(i32 a0) {
    loop:
        inci a0, 1
        jmp loop
    }
    )";
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph(source, "foo_inf", graph));
    ASSERT_FALSE(graph->HasEndBlock());
    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(3U, 2U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).SetFlag(inst_flags::NO_DCE);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(2U, 2U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(0U, 3U);
            INST(3U, Opcode::Add).s32().Inputs(2U, 1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

OUT_GRAPH(TestSaveStateDeoptimizeAuxiliaryBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);
        CONSTANT(20U, 10U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 10U, 11U)
        {
            INST(10U, Opcode::Compare).b().CC(CC_NE).Inputs(1U, 0U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(11U, 10U) {}
        BASIC_BLOCK(10U, 3U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 20U}, {11U, 0U}});
            INST(13U, Opcode::SaveState).Inputs(0U, 12U).SrcVregs({0U, 1U}).SetFlag(inst_flags::NO_DCE);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(0U, 12U).SrcVregs({0U, 1U});
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(12U, 7U);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(3U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(3U).SrcVregs({1U}).SetFlag(inst_flags::NO_DCE);
            INST(8U, Opcode::Return).u32().Inputs(3U);
        }
    }
}

TEST_F(IrBuilderTest, TestSaveStateDeoptimizeAuxiliaryBlock)
{
    auto source = R"(
    .function u32 foo(i32 a0) {
        ldai 0
        jne a0, test
        jmp end_test
    test:
        ldai 10
        jmp loop
    end_test:
        lda a0
    loop:
        jlez exit
        subi 1
        jmp loop
    exit:
        return
    }
    )";

    ASSERT_TRUE(ParseToGraph(source, "foo"));
    EXPECT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::TestSaveStateDeoptimizeAuxiliaryBlock::CREATE(expectedGraph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(IrBuilderTest, TestEmptyLoop)
{
    auto source = R"(
    .function i32 main() {
        ldai 1
    loop:
        jeqz loop
        ldai 0
        return
    }
    )";

    ASSERT_TRUE(ParseToGraph(source, "main"));
    // IrBuilder construct dead PHI:
    //   2p.i32  Phi                        v1(bb0), v2p(bb1) -> (v2p, v3)
    // GraphComparator failed on the instruction
    EXPECT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(7U, Opcode::SaveState).SetFlag(inst_flags::NO_DCE);
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U).SrcType(DataType::Type::INT32);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).SetFlag(inst_flags::NO_DCE);
            INST(5U, Opcode::Return).s32().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

OUT_GRAPH(MultiThrowTryBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(6U, 2U, 13U, 14U)
        {
            INST(4U, Opcode::Try).CatchTypeIds({0xE1U, 0xE2U});
        }
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U).SrcType(DataType::Type::INT32);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(10U, Opcode::LoadAndInitClass).ref().Inputs(9U);
            INST(11U, Opcode::NewObject).ref().Inputs(10U, 9U);
            INST(12U, Opcode::SaveState).Inputs(11U).SrcVregs({0U});
            INST(13U, Opcode::Throw).Inputs(11U, 12U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(14U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(15U, Opcode::LoadAndInitClass).ref().Inputs(14U);
            INST(16U, Opcode::NewObject).ref().Inputs(15U, 14U);
            INST(17U, Opcode::SaveState).Inputs(16U).SrcVregs({0U});
            INST(18U, Opcode::Throw).Inputs(16U, 17U);
        }
        BASIC_BLOCK(7U, -1L, 13U, 14U) {}  // try end
        BASIC_BLOCK(14U, 5U) {}            // catch1
        BASIC_BLOCK(13U, 5U) {}            // catch2
        BASIC_BLOCK(5U, -1L)
        {
            INST(20U, Opcode::Phi).s32().Inputs(2U, 3U);
            INST(21U, Opcode::Return).s32().Inputs(20U);
        }
    }
}

TEST_F(IrBuilderTest, MultiThrowTryBlock)
{
    auto source = R"(
    .record E1 {}
    .record E2 {}

    .function i32 main(i32 a0) {
    try_begin:
        lda a0
        jeqz label
        newobj v0, E1
        throw v0
    label:
        newobj v0, E2
        throw v0
    try_end:
        ldai 0
        jmp exit
    catch1:
        ldai 1
        jmp exit
    catch2:
        ldai 2
        jmp exit

    exit:
        return

    .catch E1, try_begin, try_end, catch1
    .catch E2, try_begin, try_end, catch2
    }
    )";

    ASSERT_TRUE(ParseToGraph<true>(source, "main"));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::MultiThrowTryBlock::CREATE(expectedGraph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

OUT_GRAPH(JumpToCatchHandler, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(6U, 0U).s32();
        CONSTANT(8U, 0x0U).s64();

        BASIC_BLOCK(5U, 2U, 12U)
        {
            INST(0U, Opcode::Try).CatchTypeIds({0xE0U});
        }
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(6U, 8U).SrcType(DataType::Type::INT32);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 7U)
        {
            INST(10U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(10U);
        }
        BASIC_BLOCK(7U, 9U, 12U)
        {
            INST(1U, Opcode::Try).CatchTypeIds({0xE1U});
        }
        BASIC_BLOCK(9U, 8U)
        {
            INST(12U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(13U, Opcode::LoadAndInitClass).ref().Inputs(12U);
            INST(14U, Opcode::NewObject).ref().Inputs(13U, 12U);
            INST(15U, Opcode::SaveState).Inputs(14U).SrcVregs({1U});
            INST(16U, Opcode::Throw).Inputs(14U, 15U);
        }
        BASIC_BLOCK(8U, 6U, 12U) {}   // try_end
        BASIC_BLOCK(6U, -1L, 12U) {}  // try_end
        BASIC_BLOCK(12U, 3U) {}       // catch_begin, catch
        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).s32().Inputs(8U);
        }
    }
}

TEST_F(IrBuilderTest, JumpToCatchHandler)
{
    auto source = R"(
    .record E0 {}
    .record E1 {}

    .function void foo() {
        return.void
    }

    .function i32 main(i32 a0) {
    try_begin1:
        lda a0
        jeqz try_end
        call.short foo
    try_begin2:
        newobj v1, E1
        throw v1
    try_end:
    catch_begin:
        ldai 0
        return

    .catch E0, try_begin1, try_end, catch_begin
    .catch E1, try_begin2, try_end, catch_begin
    }
    )";

    ASSERT_TRUE(ParseToGraph<true>(source, "main"));

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::JumpToCatchHandler::CREATE(expectedGraph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(IrBuilderTest, GroupOfThrowableInstructions)
{
    auto source = R"(
    .record E1 {}
    .record Obj {}

    .function void Obj.ctor(Obj a0) <ctor> {
        return.void
    }

    .function i32 Obj.foo(Obj a0) {
        ldai 42
        return
    }

    .function u1 main() {
        try_begin:
            initobj.short Obj.ctor
            sta.obj v0
            call.virt Obj.foo, v0
            jmp exit
        try_end:
            lda v1
        exit:
            return

        .catch E1, try_begin, try_end, try_end
    }

    )";

    ASSERT_TRUE(ParseToGraph<true>(source, "main"));
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsTryBegin()) {
            auto tryBb = bb->GetSuccessor(0U);
            EXPECT_TRUE(tryBb->IsTry());

            auto firstRealInst = tryBb->GetFirstInst();
            while (firstRealInst->IsSaveState()) {
                firstRealInst = firstRealInst->GetNext();
            }
            EXPECT_TRUE(GetGraph()->IsInstThrowable(firstRealInst));
        }
    }
}

OUT_GRAPH(InfiniteLoopInsideTryBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(7U, 0U);
        CONSTANT(14U, 1U);

        BASIC_BLOCK(2U, 4U)
        {
            INST(17U, Opcode::SaveState).SetFlag(inst_flags::NO_DCE);
            INST(6U, Opcode::SaveStateDeoptimize).Inputs(7U, 7U).SrcVregs({0U, 2U});
        }
        BASIC_BLOCK(4U, 3U, 19U)  // try_begin, loop
        {
            INST(9U, Opcode::Phi).s32().Inputs(7U, 13U);
            INST(0U, Opcode::Try).CatchTypeIds({0xEU});
        }
        BASIC_BLOCK(3U, 5U)  // try, loop
        {
            INST(11U, Opcode::SaveState).Inputs(9U).SrcVregs({2U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(11U);
            INST(13U, Opcode::Add).s32().Inputs(9U, 14U);
        }
        BASIC_BLOCK(5U, 4U, 19U) {}  // try_end, loop
        BASIC_BLOCK(19U, 9U)
        {
            INST(3U, Opcode::CatchPhi).b().Inputs(9U);
            INST(16U, Opcode::SaveState).Inputs(3U).SrcVregs({2U}).SetFlag(inst_flags::NO_DCE);
        }
        BASIC_BLOCK(9U, -1L)  // catch
        {
            INST(15U, Opcode::Return).b().Inputs(3U);
        }
    }
}

TEST_F(IrBuilderTest, InfiniteLoopInsideTryBlock)
{
    auto source = R"(
    .record E {}

    .function void foo() {
        return.void
    }

    .function u1 main() {
        movi v0, 0x0
        mov v2, v0
    try_begin:
        movi v0, 0x2
        call.short foo
        mov.obj v3, v0
        inci v2, 0x1
        jmp try_begin
    try_end:
        lda v2
        return

    .catch E, try_begin, try_end, try_end
    }
    )";
    ASSERT_TRUE(ParseToGraph<true>(source, "main"));
    auto expected = CreateGraphWithDefaultRuntime();
    out_graph::InfiniteLoopInsideTryBlock::CREATE(expected);
    GraphChecker(expected).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(IrBuilderTest, ReturnInsideTryBlock)
{
    auto source = R"(
    .record E {}

    .function void foo() {
        return.void
    }

    .function void main() {
    try_begin:
        call.short foo
        return.void
    try_end:
        return.void

    .catch E, try_begin, try_end, try_end
    }

    )";

    ASSERT_TRUE(ParseToGraph<true>(source, "main"));

    auto expected = CreateGraphWithDefaultRuntime();
    GRAPH(expected)
    {
        BASIC_BLOCK(3U, 2U, 18U)  // try_begin
        {
            INST(0U, Opcode::Try).CatchTypeIds({0xEU});
        }
        BASIC_BLOCK(2U, 4U)  // try
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(2U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
        BASIC_BLOCK(4U, -1L, 18U) {}  // try_end
        BASIC_BLOCK(18U, 8U) {}
        BASIC_BLOCK(8U, -1L)
        {
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    GraphChecker(expected).Check();

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(IrBuilderTest, JumpInsideTryBlock)
{
    auto source = R"(
    .record E {}

    .function void foo() {
        return.void
    }

    .function void main() {
    try_begin:
        call.short foo
        jmp label
    try_end:
        call.short foo
    label:
        ldai 0x0
        return.void

    .catch E, try_begin, try_end, try_end
    }

    )";

    ASSERT_TRUE(ParseToGraph<true>(source, "main"));

    auto expected = CreateGraphWithDefaultRuntime();
    GRAPH(expected)
    {
        BASIC_BLOCK(4U, 2U, 19U)  // try_begin
        {
            INST(0U, Opcode::Try).CatchTypeIds({0xEU});
        }
        BASIC_BLOCK(2U, 5U)  // try
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(2U);
        }
        BASIC_BLOCK(5U, 3U, 19U) {}  // try_end
        BASIC_BLOCK(19U, 9U) {}
        BASIC_BLOCK(9U, 3U)  // catch
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::ReturnVoid).v0id();
        }
    }
    GraphChecker(expected).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(IrBuilderTest, CompareAnyType)
{
    // no crash.
    auto graph = CreateGraphDynWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CompareAnyType).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }

    const CompareAnyTypeInst *cati = INS(2U).CastToCompareAnyType();

    ASSERT_TRUE(cati != nullptr);
    EXPECT_TRUE(cati->GetInputType(0U) == DataType::Type::ANY);
    EXPECT_TRUE(cati->GetType() == DataType::BOOL);
    EXPECT_TRUE(cati->GetAnyType() == AnyBaseType::UNDEFINED_TYPE);
}

TEST_F(IrBuilderTest, CastAnyTypeValue)
{
    // no crash.
    auto graph = CreateGraphDynWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CastAnyTypeValue).AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U).u64();
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }

    const CastAnyTypeValueInst *catvi = INS(2U).CastToCastAnyTypeValue();

    ASSERT_TRUE(catvi != nullptr);
    EXPECT_TRUE(catvi->GetInputType(0U) == DataType::Type::ANY);
    EXPECT_TRUE(catvi->GetDeducedType() == DataType::Type::ANY);
    EXPECT_TRUE(catvi->GetAnyType() == AnyBaseType::UNDEFINED_TYPE);
}

TEST_F(IrBuilderTest, PhiWithIdenticalInputs)
{
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(4U, Opcode::Try).CatchTypeIds({0xE1U});
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(4U, 5U) {}  // try-end
        BASIC_BLOCK(5U, 6U)
        {  // catch
            INST(7U, Opcode::CatchPhi).s32().Inputs(0U, 0U);
        }
        BASIC_BLOCK(6U, 7U, 8U)
        {
            INST(8U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1U, 2U);
        }
        BASIC_BLOCK(7U, 8U) {}
        BASIC_BLOCK(8U, -1L)
        {
            INST(10U, Opcode::Phi).s32().Inputs(7U, 7U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
    }
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    EXPECT_EQ(INS(11U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(IrBuilderTest, CallVirtShort)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0) <noimpl>
        .function i32 B.foo(B a0) {
            ldai 0
            return
        }

        .function u1 main() {
            newobj v0, B
            call.virt.short A.foo, v0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(*-build-using-namespace)
            INST(1U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {NO_TYPE, 4U}});
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirt)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0) <noimpl>
        .function i32 B.foo(B a0) {
            ldai 0
            return
        }

        .function u1 main() {
            newobj v0, B
            call.virt A.foo, v0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(*-build-using-namespace)
            INST(1U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {NO_TYPE, 4U}});
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirtRange)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0) <noimpl>
        .function i32 B.foo(B a0) {
            ldai 0
            return
        }

        .function u1 main() {
            newobj v0, B
            call.virt.range A.foo, v0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(*-build-using-namespace)
            INST(1U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {NO_TYPE, 4U}});
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
