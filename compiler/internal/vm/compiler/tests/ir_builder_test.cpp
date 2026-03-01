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

#include "optimizer/ir/datatype.h"
#include "unit_test.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"

namespace panda::compiler {
class IrBuilderTest : public AsmTest {
public:
    IrBuilderTest()
        : default_compiler_non_optimizing_(options.IsCompilerNonOptimizing()),
          default_compiler_use_safe_point_(options.IsCompilerUseSafepoint())
    {
        options.SetCompilerNonOptimizing(false);
        options.SetCompilerUseSafepoint(false);
    }

    ~IrBuilderTest() override
    {
        options.SetCompilerNonOptimizing(default_compiler_non_optimizing_);
        options.SetCompilerUseSafepoint(default_compiler_use_safe_point_);
    }

    void CheckSimple(std::string inst_name, DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "mov" || inst_name == "lda" || inst_name == "sta");
        std::string curr_type;
        if (data_type == DataType::Type::REFERENCE) {
            curr_type = "i64[]";
        } else {
            curr_type = ToString(data_type);
        }

        std::string source = ".function " + curr_type + " main(";
        source += curr_type + " a0){\n";
        if (inst_name == "mov") {
            source += "mov" + inst_type + " v0, a0\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "lda") {
            source += "lda" + inst_type + " a0\n";
        } else if (inst_name == "sta") {
            source += "lda" + inst_type + " a0\n";
            source += "sta" + inst_type + " v0\n";
            source += "lda" + inst_type + " v0\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + inst_type + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(data_type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckSimpleWithImm(std::string inst_name, DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "mov" || inst_name == "fmov" || inst_name == "lda" || inst_name == "flda");
        std::string curr_type = ToString(data_type);

        std::string source = ".function " + curr_type + " main(){\n";
        if (inst_name == "mov") {
            source += "movi" + inst_type + " v0, 0\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "fmov") {
            source += "fmovi" + inst_type + " v0, 0.\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "lda") {
            source += "ldai" + inst_type + " 0\n";
        } else if (inst_name == "flda") {
            source += "fldai" + inst_type + " 0.\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + inst_type + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto constant_type = GetCommonType(data_type);
        auto graph = CreateGraphWithDefaultRuntime();

        GRAPH(graph)
        {
            CONSTANT(0, 0);
            INS(0).SetType(constant_type);

            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(data_type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckCmp(std::string inst_name, DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "cmp" || inst_name == "ucmp" || inst_name == "fcmpl" || inst_name == "fcmpg");
        std::string curr_type;
        if (data_type == DataType::Type::REFERENCE) {
            curr_type = "i64[]";
        } else {
            curr_type = ToString(data_type);
        }
        std::string source = ".function i32 main(";
        source += curr_type + " a0, ";
        source += curr_type + " a1){\n";
        source += "lda" + inst_type + " a0\n";
        source += inst_name + inst_type + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);
            PARAMETER(1, 1);
            INS(1).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Cmp).s32().Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckFloatCmp(std::string inst_name, DataType::Type data_type, std::string inst_type, bool fcmpg)
    {
        ASSERT(inst_name == "fcmpl" || inst_name == "fcmpg");
        std::string curr_type = ToString(data_type);

        std::string source = ".function i32 main(";
        source += curr_type + " a0, ";
        source += curr_type + " a1){\n";
        source += "lda" + inst_type + " a0\n";
        source += inst_name + inst_type + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);
            PARAMETER(1, 1);
            INS(1).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Cmp).s32().SrcType(data_type).Fcmpg(fcmpg).Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool is_obj>
    void CheckCondJump(ConditionCode CC)
    {
        std::string cmd;
        switch (CC) {
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

        std::string inst_postfix = "";
        std::string param_type = "i32";
        auto type = DataType::INT32;
        if constexpr (is_obj) {
            inst_postfix = ".obj";
            param_type = "i64[]";
            type = DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += param_type + " a0, " + param_type + " a1) {\n";
        source += "lda" + inst_postfix + " a0\n";
        source += cmd + inst_postfix + " a1, label\n";
        source += "label:\n";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            PARAMETER(1, 1);
            INS(1).SetType(type);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::Compare).b().CC(CC).Inputs(0, 1);
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool is_obj>
    void CheckCondJumpWithZero(ConditionCode CC)
    {
        std::string cmd;
        switch (CC) {
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

        std::string inst_postfix = "";
        std::string param_type = "i32";
        auto type = DataType::INT32;
        if constexpr (is_obj) {
            inst_postfix = ".obj";
            param_type = "i64[]";
            type = DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += param_type + " a0) {\n";
        source += "lda" + inst_postfix + " a0\n";
        source += cmd + inst_postfix + " label\n";
        source += "label:\n";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source.c_str(), "main"));

        auto graph = CreateGraphWithDefaultRuntime();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(2, 0).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(1, Opcode::Compare).b().CC(CC).Inputs(0, 2);
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

private:
    bool default_compiler_non_optimizing_;
    bool default_compiler_use_safe_point_;
};

TEST_F(IrBuilderTest, LoadArrayType64)
{
    auto source = R"(
    .function void main(i64[] a0, i32[] a1){
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
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 2, 3);
            INST(7, Opcode::LoadArray).s64().Inputs(4, 6);
            INST(8, Opcode::SaveState).Inputs(2, 0, 1, 7).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::NullCheck).ref().Inputs(1, 8);
            INST(10, Opcode::LenArray).s32().Inputs(9);
            INST(11, Opcode::BoundsCheck).s32().Inputs(10, 2, 8);
            INST(12, Opcode::StoreArray).s32().Inputs(9, 11, 7);
            INST(13, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicPrintU64)
{
    auto source = R"(
    .record IO <external>
    .function void IO.printU64(u64 a0) <external>
    .function void main(u64 a0){
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
        PARAMETER(1, 13).u64();
        CONSTANT(0, 23).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s64().Inputs(0, 1);
            INST(4, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::UINT64, 2}})
                .SetFlag(compiler::inst_flags::NO_HOIST)
                .SetFlag(compiler::inst_flags::NO_DCE)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::BARRIER)
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, BuiltinIsInf)
{
    auto source = R"(
        .record Double <external>
        .function u1 Double.isInfinite(f64 a0) <external>
        .function u1 main(f64 a0){
            call.short Double.isInfinite, a0
            return
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Intrinsic)
                .b()
                .Inputs({{DataType::FLOAT64, 0}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::Return).b().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicAbs)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.absF64(f64 a0) <external>
    .function f64 main(f64 a0){
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
        PARAMETER(1, 13).f64();
        CONSTANT(0, 1.23).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f64().Inputs(0, 1);
            INST(3, Opcode::Abs).f64().Inputs(2);
            INST(4, Opcode::Return).f64().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathSqrt)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.sqrt(f64 a0) <external>
    .function f64 main(f64 a0){
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
        PARAMETER(1, 0).f64();
        CONSTANT(0, 3.14).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f64().Inputs(0, 1);
            INST(3, Opcode::Sqrt).f64().Inputs(2);
            INST(4, Opcode::Return).f64().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, IntrinsicMathFsqrt)
{
    auto source = R"(
    .record Math <external>
    .function f32 Math.fsqrt(f32 a0) <external>
    .function f32 main(f32 a0){
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
        PARAMETER(1, 0).f32();
        CONSTANT(0, 3.14f).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f32().Inputs(0, 1);
            INST(3, Opcode::Sqrt).f32().Inputs(2);
            INST(4, Opcode::Return).f32().Inputs(3);
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
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Min).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
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
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Min).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
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
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Min).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
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
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Min).f32().Inputs(0, 1);
            INST(6, Opcode::Return).f32().Inputs(4);
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
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Max).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
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
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Max).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
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
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Max).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
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
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Max).f32().Inputs(0, 1);
            INST(6, Opcode::Return).f32().Inputs(4);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, NoCheckForFloatDiv)
{
    auto source = R"(
    .function f64 main(f64 a0){
        fldai.64 23.0
        fdiv2.64 a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(1, 0).f64();
        CONSTANT(0, 23.0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Div).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, MultipleThrow)
{
    auto source = R"(
    .record array <external>
    .function void main(array a0){
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
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::Throw).Inputs(0, 1);
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
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::Sub).s32().Inputs(0, 1);
            INST(5, Opcode::SaveState).Inputs(4, 0, 1, 4).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::CallStatic).s64().Inputs({{DataType::INT32, 4}, {DataType::NO_TYPE, 5}});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::SaveState).Inputs(1, 0).SrcVregs({2, 1});
            INST(8, Opcode::CallStatic).s64().Inputs({{DataType::NO_TYPE, 7}});
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mov instruction with integer parameters
TEST_F(IrBuilderTest, MovInt)
{
    DataType::Type data_type = DataType::Type::INT32;
    std::string inst_type = "";
    CheckSimple("mov", data_type, inst_type);
}

// Checks the build of the mov instruction with real parameters
TEST_F(IrBuilderTest, MovReal)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckSimple("mov", data_type, inst_type);
}

// Checks the build of the mov.64 instruction with integer parameters
TEST_F(IrBuilderTest, Mov64Int)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckSimple("mov", data_type, inst_type);
}

// Checks the build of the mov.64 instruction with real parameters
TEST_F(IrBuilderTest, Mov64Real)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckSimple("mov", data_type, inst_type);
}

// Checks the build of the mov.obj instruction
TEST_F(IrBuilderTest, MovObj)
{
    DataType::Type data_type = DataType::Type::REFERENCE;
    std::string inst_type = ".obj";
    CheckSimple("mov", data_type, inst_type);
}

// Checks the build of the mov.null instruction
TEST_F(IrBuilderTest, MovNull)
{
    auto source = R"(
        .record panda.String <external>
        .function panda.String main(){
            mov.null v0
            lda v0
            return
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0, nullptr);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the movi instruction with integer parameters
TEST_F(IrBuilderTest, MoviInt)
{
    DataType::Type data_type = DataType::Type::INT32;
    std::string inst_type = "";
    CheckSimpleWithImm("mov", data_type, inst_type);
}

// Checks the build of the fmovi instruction with real parameters
TEST_F(IrBuilderTest, FmoviReal)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckSimpleWithImm("fmov", data_type, inst_type);
}

// Checks the build of the movi.64 instruction with integer parameters
TEST_F(IrBuilderTest, Movi64Int)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckSimpleWithImm("mov", data_type, inst_type);
}

// Checks the build of the movi.64 instruction with real parameters
TEST_F(IrBuilderTest, Fmovi64Real)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckSimpleWithImm("fmov", data_type, inst_type);
}

// Checks the build of the lda instruction with integer parameters
TEST_F(IrBuilderTest, LdaInt)
{
    DataType::Type data_type = DataType::Type::INT32;
    std::string inst_type = "";
    CheckSimple("lda", data_type, inst_type);
}

// Checks the build of the lda instruction with real parameters
TEST_F(IrBuilderTest, LdaReal)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckSimple("lda", data_type, inst_type);
}

// Checks the build of the lda.64 instruction with integer parameters
TEST_F(IrBuilderTest, Lda64Int)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckSimple("lda", data_type, inst_type);
}

// Checks the build of the lda.64 instruction with real parameters
TEST_F(IrBuilderTest, Lda64Real)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckSimple("lda", data_type, inst_type);
}

// Checks the build of the lda.obj instruction
TEST_F(IrBuilderTest, LdaObj)
{
    DataType::Type data_type = DataType::Type::REFERENCE;
    std::string inst_type = ".obj";
    CheckSimple("lda", data_type, inst_type);
}

// Checks the build of the lda.obj instruction
TEST_F(IrBuilderTest, LdaNull)
{
    auto source = R"(
        .record panda.String <external>
        .function panda.String main(){
            lda.null
            return.obj
        }
        )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(0, nullptr);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldai instruction with integer parameters
TEST_F(IrBuilderTest, LdaiInt)
{
    DataType::Type data_type = DataType::Type::INT32;
    std::string inst_type = "";
    CheckSimpleWithImm("lda", data_type, inst_type);
}

// Checks the build of the ldai instruction with real parameters
TEST_F(IrBuilderTest, FldaiReal)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckSimpleWithImm("flda", data_type, inst_type);
}

// Checks the build of the ldai.64 instruction with integer parameters
TEST_F(IrBuilderTest, Ldai64Int)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckSimpleWithImm("lda", data_type, inst_type);
}

// Checks the build of the ldai.64 instruction with real parameters
TEST_F(IrBuilderTest, Fldai64Real)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckSimpleWithImm("flda", data_type, inst_type);
}

// Checks the build of the lda.str instruction
TEST_F(IrBuilderTest, LdaStr)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(){
        lda.str "lda_test"
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(0, Opcode::LoadString).ref().Inputs(2);
            INST(1, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the lda.type instruction
TEST_F(IrBuilderTest, LdaType)
{
    auto source = R"(
    .record R {}
    .function R main(){
        lda.type R
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(0, Opcode::LoadType).ref().Inputs(2);
            INST(1, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sta instruction with integer parameters
TEST_F(IrBuilderTest, StaInt)
{
    DataType::Type data_type = DataType::Type::INT32;
    std::string inst_type = "";
    CheckSimple("sta", data_type, inst_type);
}

// Checks the build of the sta instruction with real parameters
TEST_F(IrBuilderTest, StaReal)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckSimple("sta", data_type, inst_type);
}

// Checks the build of the sta.64 instruction with integer parameters
TEST_F(IrBuilderTest, Sta64Int)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckSimple("sta", data_type, inst_type);
}

// Checks the build of the sta.64 instruction with real parameters
TEST_F(IrBuilderTest, Sta64Real)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckSimple("sta", data_type, inst_type);
}

// Checks the build of the sta.obj instruction
TEST_F(IrBuilderTest, StaObj)
{
    DataType::Type data_type = DataType::Type::REFERENCE;
    std::string inst_type = ".obj";
    CheckSimple("sta", data_type, inst_type);
}

// Checks the build of the jmp instruction
TEST_F(IrBuilderTest, Jmp)
{
    auto source = R"(
    .function void main(){
        jmp label
    label:
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, -1)
        {
            INST(1, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the cmp.64 instruction
TEST_F(IrBuilderTest, Cmp64)
{
    DataType::Type data_type = DataType::Type::INT64;
    std::string inst_type = ".64";
    CheckCmp("cmp", data_type, inst_type);
}

// Checks the build of the ucmp instruction
TEST_F(IrBuilderTest, Ucmp)
{
    DataType::Type data_type = DataType::Type::UINT32;
    std::string inst_type = "";
    CheckCmp("ucmp", data_type, inst_type);
}

// Checks the build of the ucmp.64 instruction
TEST_F(IrBuilderTest, Ucmp64)
{
    DataType::Type data_type = DataType::Type::UINT64;
    std::string inst_type = ".64";
    CheckCmp("ucmp", data_type, inst_type);
}

// Checks the build of the fcmpl instruction
TEST_F(IrBuilderTest, Fcmpl)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckFloatCmp("fcmpl", data_type, inst_type, false);
}

// Checks the build of the fcmpl.64 instruction
TEST_F(IrBuilderTest, Fcmpl64)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckFloatCmp("fcmpl", data_type, inst_type, false);
}

// Checks the build of the fcmpg instruction
TEST_F(IrBuilderTest, Fcmpg)
{
    DataType::Type data_type = DataType::Type::FLOAT32;
    std::string inst_type = "";
    CheckFloatCmp("fcmpg", data_type, inst_type, true);
}

// Checks the build of the fcmpg.64 instruction
TEST_F(IrBuilderTest, Fcmpg64)
{
    DataType::Type data_type = DataType::Type::FLOAT64;
    std::string inst_type = ".64";
    CheckFloatCmp("fcmpg", data_type, inst_type, true);
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
    .function f32 main(f32 a0, f32 a1){
        lda a0
        fadd2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).f32().Inputs(0, 1);
            INST(3, Opcode::Return).f32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fadd2.64 instruction
TEST_F(IrBuilderTest, Fadd2_64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1){
        lda.64 a0
        fadd2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fsub2 instruction
TEST_F(IrBuilderTest, Fsub2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1){
        lda a0
        fsub2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f32().Inputs(0, 1);
            INST(3, Opcode::Return).f32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fsub2.64 instruction
TEST_F(IrBuilderTest, Fsub2_64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1){
        lda.64 a0
        fsub2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmul2 instruction
TEST_F(IrBuilderTest, Fmul2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1){
        lda a0
        fmul2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).f32().Inputs(0, 1);
            INST(3, Opcode::Return).f32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmul2.64 instruction
TEST_F(IrBuilderTest, Fmul2_64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1){
        lda.64 a0
        fmul2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fdiv2 instruction
TEST_F(IrBuilderTest, Fdiv2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1){
        lda a0
        fdiv2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Div).f32().Inputs(0, 1);
            INST(3, Opcode::Return).f32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fdiv2.64 instruction
TEST_F(IrBuilderTest, Fdiv2_64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1){
        lda.64 a0
        fdiv2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Div).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmod2 instruction
TEST_F(IrBuilderTest, Fmod2)
{
    auto source = R"(
    .function f32 main(f32 a0, f32 a1){
        lda a0
        fmod2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mod).f32().Inputs(0, 1);
            INST(3, Opcode::Return).f32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fmod2.64 instruction
TEST_F(IrBuilderTest, Fmod2_64)
{
    auto source = R"(
    .function f64 main(f64 a0, f64 a1){
        lda.64 a0
        fmod2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mod).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add2 instruction
TEST_F(IrBuilderTest, Add2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        add2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add2.64 instruction
TEST_F(IrBuilderTest, Add2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        add2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub2 instruction
TEST_F(IrBuilderTest, Sub2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        sub2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub2.64 instruction
TEST_F(IrBuilderTest, Sub2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        sub2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul2 instruction
TEST_F(IrBuilderTest, Mul2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        mul2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul2.64 instruction
TEST_F(IrBuilderTest, Mul2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        mul2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and2 instruction
TEST_F(IrBuilderTest, And2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        and2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and2.64 instruction
TEST_F(IrBuilderTest, And2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        and2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or2 instruction
TEST_F(IrBuilderTest, Or2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        or2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or2.64 instruction
TEST_F(IrBuilderTest, Or2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        or2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the Xor2 instruction
TEST_F(IrBuilderTest, Xor2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        xor2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Xor).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xor2.64 instruction
TEST_F(IrBuilderTest, Xor2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        xor2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Xor).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl2 instruction
TEST_F(IrBuilderTest, Shl2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        shl2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl2.64 instruction
TEST_F(IrBuilderTest, Shl2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        shl2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr2 instruction
TEST_F(IrBuilderTest, Shr2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        shr2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shr).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr2.64 instruction
TEST_F(IrBuilderTest, Shr2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        shr2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shr).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr2 instruction
TEST_F(IrBuilderTest, Ashr2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        ashr2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AShr).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr2.64 instruction
TEST_F(IrBuilderTest, Ashr2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        ashr2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AShr).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div2 instruction
TEST_F(IrBuilderTest, Div2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        div2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div2.64 instruction
TEST_F(IrBuilderTest, Div2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        div2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).s64().Inputs(1, 2);
            INST(4, Opcode::Div).s64().Inputs(0, 3);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod2 instruction
TEST_F(IrBuilderTest, Mod2)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        lda a0
        mod2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Mod).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod2.64 instruction
TEST_F(IrBuilderTest, Mod2_64)
{
    auto source = R"(
    .function i64 main(i64 a0, i64 a1){
        lda.64 a0
        mod2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).s64().Inputs(1, 2);
            INST(4, Opcode::Mod).s64().Inputs(0, 3);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divu2 instruction
TEST_F(IrBuilderTest, Divu2)
{
    auto source = R"(
    .function u32 main(u32 a0, u32 a1){
        lda a0
        divu2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).u32().Inputs(1, 2);
            INST(4, Opcode::Div).u32().Inputs(0, 3);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divu2.64 instruction
TEST_F(IrBuilderTest, Divu2_64)
{
    auto source = R"(
    .function u64 main(u64 a0, u64 a1){
        lda.64 a0
        divu2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).u64().Inputs(1, 2);
            INST(4, Opcode::Div).u64().Inputs(0, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modu2 instruction
TEST_F(IrBuilderTest, Modu2)
{
    auto source = R"(
    .function u32 main(u32 a0, u32 a1){
        lda a0
        modu2 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).u32().Inputs(1, 2);
            INST(4, Opcode::Mod).u32().Inputs(0, 3);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modu2.64 instruction
TEST_F(IrBuilderTest, Modu2_64)
{
    auto source = R"(
    .function u64 main(u64 a0, u64 a1){
        lda.64 a0
        modu2.64 a1
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::ZeroCheck).u64().Inputs(1, 2);
            INST(4, Opcode::Mod).u64().Inputs(0, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the add instruction
TEST_F(IrBuilderTest, Add)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        add a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the sub instruction
TEST_F(IrBuilderTest, Sub)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        sub a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mul instruction
TEST_F(IrBuilderTest, Mul)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        mul a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the and instruction
TEST_F(IrBuilderTest, And)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        and a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the or instruction
TEST_F(IrBuilderTest, Or)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        or a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xor instruction
TEST_F(IrBuilderTest, Xor)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        xor a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Xor).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shl instruction
TEST_F(IrBuilderTest, Shl)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        shl a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shr instruction
TEST_F(IrBuilderTest, Shr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        shr a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shr).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashr instruction
TEST_F(IrBuilderTest, Ashr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        ashr a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AShr).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the div instruction
TEST_F(IrBuilderTest, Div)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        div a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the mod instruction
TEST_F(IrBuilderTest, Mod)
{
    auto source = R"(
    .function i32 main(i32 a0, i32 a1){
        mod a0, a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Mod).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the addi instruction
TEST_F(IrBuilderTest, Addi)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        addi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Add).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the subi instruction
TEST_F(IrBuilderTest, Subi)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        subi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Sub).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the muli instruction
TEST_F(IrBuilderTest, Muli)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        muli 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Mul).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the andi instruction
TEST_F(IrBuilderTest, Andi)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        andi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::And).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ori instruction
TEST_F(IrBuilderTest, Ori)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        ori 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Or).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the xori instruction
TEST_F(IrBuilderTest, Xori)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        xori 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Xor).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shli instruction
TEST_F(IrBuilderTest, Shli)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        shli 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Shl).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the shri instruction
TEST_F(IrBuilderTest, Shri)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        shri 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Shr).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ashri instruction
TEST_F(IrBuilderTest, Ashri)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        ashri 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::AShr).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the divi instruction
TEST_F(IrBuilderTest, Divi)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        divi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 0).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the modi instruction
TEST_F(IrBuilderTest, Modi)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        modi 1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 0).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Mod).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fneg instruction
TEST_F(IrBuilderTest, Fneg)
{
    auto source = R"(
    .function f32 main(f32 a0){
        lda a0
        fneg
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Neg).f32().Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fneg.64 instruction
TEST_F(IrBuilderTest, Fneg64)
{
    auto source = R"(
    .function f64 main(f64 a0){
        lda a0
        fneg.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Neg).f64().Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the neg instruction
TEST_F(IrBuilderTest, Neg)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        neg
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Neg).s32().Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the neg.64 instruction
TEST_F(IrBuilderTest, Neg64)
{
    auto source = R"(
    .function i64 main(i64 a0){
        lda a0
        neg.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Neg).s64().Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the not instruction
TEST_F(IrBuilderTest, Not)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        not
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Not).s32().Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the not.64 instruction
TEST_F(IrBuilderTest, Not64)
{
    auto source = R"(
    .function i64 main(i64 a0){
        lda a0
        not.64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Not).s64().Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the inci instruction
TEST_F(IrBuilderTest, Inci)
{
    auto source = R"(
    .function i32 main(i32 a0){
        inci a0, 1
        lda a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Add).s32().Inputs(0, 2);
            INST(3, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32tof32 instruction
TEST_F(IrBuilderTest, I32tof32)
{
    auto source = R"(
    .function f32 main(i32 a0){
        lda a0
        i32tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32tof64 instruction
TEST_F(IrBuilderTest, I32tof64)
{
    auto source = R"(
    .function f64 main(i32 a0){
        lda a0
        i32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32tof32 instruction
TEST_F(IrBuilderTest, U32tof32)
{
    auto source = R"(
    .function f32 main(u32 a0){
        lda a0
        u32tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::UINT32).Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32tof64 instruction
TEST_F(IrBuilderTest, U32tof64)
{
    auto source = R"(
    .function f64 main(u32 a0){
        lda a0
        u32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(DataType::UINT32).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64tof32 instruction
TEST_F(IrBuilderTest, I64tof32)
{
    auto source = R"(
    .function f32 main(i64 a0){
        lda.64 a0
        i64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64tof64 instruction
TEST_F(IrBuilderTest, I64tof64)
{
    auto source = R"(
    .function f64 main(i64 a0){
        lda.64 a0
        i64tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u64tof32 instruction
TEST_F(IrBuilderTest, U64tof32)
{
    auto source = R"(
    .function f32 main(u64 a0){
        lda.64 a0
        u64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::UINT64).Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u64tof64 instruction
TEST_F(IrBuilderTest, U64tof64)
{
    auto source = R"(
    .function f64 main(u64 a0){
        lda.64 a0
        u64tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(DataType::UINT64).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32toi32 instruction
TEST_F(IrBuilderTest, F32toi32)
{
    auto source = R"(
    .function i32 main(f32 a0){
        lda a0
        f32toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(DataType::FLOAT32).Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32toi64 instruction
TEST_F(IrBuilderTest, F32toi64)
{
    auto source = R"(
    .function i64 main(f32 a0){
        lda a0
        f32toi64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s64().SrcType(DataType::FLOAT32).Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tou32 instruction
TEST_F(IrBuilderTest, F32tou32)
{
    auto source = R"(
    .function u32 main(f32 a0){
        lda a0
        f32tou32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u32().SrcType(DataType::FLOAT32).Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tou64 instruction
TEST_F(IrBuilderTest, F32tou64)
{
    auto source = R"(
    .function u64 main(f32 a0){
        lda a0
        f32tou64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(0);
            INST(2, Opcode::Return).u64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f32tof64 instruction
TEST_F(IrBuilderTest, F32tof64)
{
    auto source = R"(
    .function f64 main(f32 a0){
        lda a0
        f32tof64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(DataType::FLOAT32).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64toi32 instruction
TEST_F(IrBuilderTest, F64toi32)
{
    auto source = R"(
    .function i32 main(f64 a0){
        lda.64 a0
        f64toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64toi64 instruction
TEST_F(IrBuilderTest, F64toi64)
{
    auto source = R"(
    .function i64 main(f64 a0){
        lda.64 a0
        f64toi64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s64().SrcType(DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tou32 instruction
TEST_F(IrBuilderTest, F64tou32)
{
    auto source = R"(
    .function u32 main(f64 a0){
        lda.64 a0
        f64tou32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tou64 instruction
TEST_F(IrBuilderTest, F64tou64)
{
    auto source = R"(
    .function u64 main(f64 a0){
        lda.64 a0
        f64tou64
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).u64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the f64tof32 instruction
TEST_F(IrBuilderTest, F64tof32)
{
    auto source = R"(
    .function f32 main(f64 a0){
        lda.64 a0
        f64tof32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).f32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i32toi64 instruction
TEST_F(IrBuilderTest, I32toi64)
{
    auto source = R"(
    .function i64 main(i32 a0){
        lda a0
        i32toi64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s64().SrcType(DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the i64toi32 instruction
TEST_F(IrBuilderTest, I64toi32)
{
    auto source = R"(
    .function i32 main(i64 a0){
        lda.64 a0
        i64toi32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the u32toi64 instruction
TEST_F(IrBuilderTest, U32toi64)
{
    auto source = R"(
    .function i64 main(u32 a0){
        lda a0
        u32toi64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s64().SrcType(DataType::UINT32).Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.8 instruction
TEST_F(IrBuilderTest, Ldarr8)
{
    auto source = R"(
    .function i8 main(i32 a0, i8[] a1){
        lda a0
        ldarr.8 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s8().Inputs(3, 5);
            INST(7, Opcode::Return).s8().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarru.8 instruction
TEST_F(IrBuilderTest, Ldarru8)
{
    auto source = R"(
    .function u8 main(i32 a0, u8[] a1){
        lda a0
        ldarru.8 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).u8().Inputs(3, 5);
            INST(7, Opcode::Return).u8().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.16 instruction
TEST_F(IrBuilderTest, Ldarr16)
{
    auto source = R"(
    .function i16 main(i32 a0, i16[] a1){
        lda a0
        ldarr.16 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s16().Inputs(3, 5);
            INST(7, Opcode::Return).s16().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarru.16 instruction
TEST_F(IrBuilderTest, Ldarru16)
{
    auto source = R"(
    .function u16 main(i32 a0, u16[] a1){
        lda a0
        ldarru.16 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).u16().Inputs(3, 5);
            INST(7, Opcode::Return).u16().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr instruction
TEST_F(IrBuilderTest, Ldarr)
{
    auto source = R"(
    .function i32 main(i32 a0, i32[] a1){
        lda a0
        ldarr a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s32().Inputs(3, 5);
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fldarr.32 instruction
TEST_F(IrBuilderTest, Fldarr32)
{
    auto source = R"(
    .function f32 main(i32 a0, f32[] a1){
        lda a0
        fldarr.32 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).f32().Inputs(3, 5);
            INST(8, Opcode::Return).f32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.64 instruction
TEST_F(IrBuilderTest, Ldarr64)
{
    auto source = R"(
    .function u64 main(i32 a0, u64[] a1){
        lda a0
        ldarr.64 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s64().Inputs(3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fldarr.64 instruction
TEST_F(IrBuilderTest, Fldarr64)
{
    auto source = R"(
    .function f64 main(i32 a0, f64[] a1){
        lda a0
        fldarr.64 a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).f64().Inputs(3, 5);
            INST(7, Opcode::Return).f64().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the ldarr.obj instruction
TEST_F(IrBuilderTest, LdarrObj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(i32 a0, panda.String[] a1){
        lda a0
        ldarr.obj a1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).ref().Inputs(3, 5);
            INST(7, Opcode::Return).ref().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.8 instruction
TEST_F(IrBuilderTest, Starr8)
{
    auto source = R"(
    .function void main(i32 a0, u8[] a1, u8 a2){
        lda a2
        starr.8 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).u8();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(7, Opcode::StoreArray).s8().Inputs(4, 6, 2);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.16 instruction
TEST_F(IrBuilderTest, Starr16)
{
    auto source = R"(
    .function void main(i32 a0, u16[] a1, u16 a2){
        lda a2
        starr.16 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).u16();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(7, Opcode::StoreArray).s16().Inputs(4, 6, 2);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr instruction
TEST_F(IrBuilderTest, Starr)
{
    auto source = R"(
    .function void main(i32 a0, i32[] a1, i32 a2){
        lda a2
        starr a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(7, Opcode::StoreArray).s32().Inputs(4, 6, 2);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fstarr.32 instruction
TEST_F(IrBuilderTest, Fstarr32)
{
    auto source = R"(
    .function void main(i32 a0, f32[] a1, f32 a2){
        lda a2
        fstarr.32 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(8, Opcode::StoreArray).f32().Inputs(4, 6, 2);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.64 instruction
TEST_F(IrBuilderTest, Starr64)
{
    auto source = R"(
    .function void main(i32 a0, i64[] a1, i64 a2){
        lda.64 a2
        starr.64 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(7, Opcode::StoreArray).s64().Inputs(4, 6, 2);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the fstarr.64 instruction
TEST_F(IrBuilderTest, Fstarr64)
{
    auto source = R"(
    .function void main(i32 a0, f64[] a1, f64 a2){
        lda.64 a2
        fstarr.64 a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(7, Opcode::StoreArray).f64().Inputs(4, 6, 2);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the starr.obj instruction
TEST_F(IrBuilderTest, StarrObj)
{
    auto source = R"(
    .record panda.String <external>
    .function void main(i32 a0, panda.String[] a1, panda.String a2){
        lda.obj a2
        starr.obj a1, a0
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({0, 1, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(1, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 0, 3);
            INST(9, Opcode::RefTypeCheck).ref().Inputs(4, 2, 3);
            INST(7, Opcode::StoreArray).ref().Inputs(4, 6, 9);
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the lenarr instruction
TEST_F(IrBuilderTest, Lenarr)
{
    auto source = R"(
    .function i32 main(i32[] a0){
        lenarr a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LenArray).s32().Inputs(2);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the newarr instruction
TEST_F(IrBuilderTest, Newarr)
{
    auto source = R"(
    .function i32[] main(i32 a0){
        newarr a0, a0, i32[]
        lda.obj a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(2, Opcode::NegativeCheck).s32().Inputs(0, 1);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
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
    auto default_option = options.GetCompilerUnfoldConstArrayMaxSize();
    options.SetCompilerUnfoldConstArrayMaxSize(2);
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(40, 3).s64();
        BASIC_BLOCK(2, -1)
        {
            // string array
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadConstArray).ref().Inputs(0);

            // bool array
            INST(12, Opcode::SaveState).Inputs(1).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(12).TypeId(68);
            INST(13, Opcode::NegativeCheck).s32().Inputs(40, 12);
            INST(14, Opcode::NewArray).ref().Inputs(44, 13, 12);

            INST(15, Opcode::SaveState).Inputs(1, 14).SrcVregs({0, 1});
            INST(16, Opcode::FillConstArray).s8().Inputs(14, 15);

            // int array
            INST(22, Opcode::SaveState).Inputs(1, 14).SrcVregs({0, 1});
            INST(45, Opcode::LoadAndInitClass).ref().Inputs(22).TypeId(68);
            INST(23, Opcode::NegativeCheck).s32().Inputs(40, 22);
            INST(24, Opcode::NewArray).ref().Inputs(45, 23, 22);

            INST(25, Opcode::SaveState).Inputs(1, 14, 24).SrcVregs({0, 1, 2});
            INST(26, Opcode::FillConstArray).s32().Inputs(24, 25);

            // float array
            INST(32, Opcode::SaveState).Inputs(1, 14, 24).SrcVregs({0, 1, 2});
            INST(46, Opcode::LoadAndInitClass).ref().Inputs(32).TypeId(68);
            INST(33, Opcode::NegativeCheck).s32().Inputs(40, 32);
            INST(34, Opcode::NewArray).ref().Inputs(46, 33, 32);

            INST(35, Opcode::SaveState).Inputs(1, 14, 24, 34).SrcVregs({0, 1, 2, 3});
            INST(36, Opcode::FillConstArray).f32().Inputs(34, 35);

            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    options.SetCompilerUnfoldConstArrayMaxSize(default_option);
}

// Checks the build of unfolded lda.const instruction
TEST_F(IrBuilderTest, LdaConstUnfold)
{
    auto source = R"(
.array array0 panda.String 2 { "a" "ab" }
.array array1 u1 2 { 0 1 }
.array array2 i32 2 { 2 3 }
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
        CONSTANT(0, 2).s64();
        CONSTANT(4, 0).s64();
        CONSTANT(13, 1).s64();
        CONSTANT(43, 3).s64();
        CONSTANT(52, 4.0f).f32();
        CONSTANT(59, 5.0f).f32();

        BASIC_BLOCK(2, -1)
        {
            // string array
            INST(1, Opcode::SaveState).NoVregs();
            INST(444, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(2, Opcode::NegativeCheck).s32().Inputs(0, 1);
            INST(3, Opcode::NewArray).ref().Inputs(444, 2, 1);

            INST(5, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(6, Opcode::LoadString).ref().Inputs(5);
            INST(7, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(9, Opcode::LenArray).s32().Inputs(8);
            INST(10, Opcode::BoundsCheck).s32().Inputs(9, 4, 7);
            INST(11, Opcode::StoreArray).ref().Inputs(8, 10, 6);

            INST(14, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(15, Opcode::LoadString).ref().Inputs(14);
            INST(16, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(17, Opcode::NullCheck).ref().Inputs(3, 16);
            INST(18, Opcode::LenArray).s32().Inputs(17);
            INST(19, Opcode::BoundsCheck).s32().Inputs(18, 13, 16);
            INST(20, Opcode::StoreArray).ref().Inputs(17, 19, 15);

            // bool array
            INST(22, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(445, Opcode::LoadAndInitClass).ref().Inputs(22).TypeId(68);
            INST(23, Opcode::NegativeCheck).s32().Inputs(0, 22);
            INST(24, Opcode::NewArray).ref().Inputs(445, 23, 22);

            INST(25, Opcode::SaveState).Inputs(3, 24).SrcVregs({0, 1});
            INST(26, Opcode::NullCheck).ref().Inputs(24, 25);
            INST(27, Opcode::LenArray).s32().Inputs(26);
            INST(28, Opcode::BoundsCheck).s32().Inputs(27, 4, 25);
            INST(29, Opcode::StoreArray).s8().Inputs(26, 28, 4);

            INST(30, Opcode::SaveState).Inputs(3, 24).SrcVregs({0, 1});
            INST(31, Opcode::NullCheck).ref().Inputs(24, 30);
            INST(32, Opcode::LenArray).s32().Inputs(31);
            INST(33, Opcode::BoundsCheck).s32().Inputs(32, 13, 30);
            INST(34, Opcode::StoreArray).s8().Inputs(31, 33, 13);

            // int array
            INST(35, Opcode::SaveState).Inputs(3, 24).SrcVregs({0, 1});
            INST(446, Opcode::LoadAndInitClass).ref().Inputs(35).TypeId(68);
            INST(36, Opcode::NegativeCheck).s32().Inputs(0, 35);
            INST(37, Opcode::NewArray).ref().Inputs(446, 36, 35);

            INST(38, Opcode::SaveState).Inputs(3, 24, 37).SrcVregs({0, 1, 2});
            INST(39, Opcode::NullCheck).ref().Inputs(37, 38);
            INST(40, Opcode::LenArray).s32().Inputs(39);
            INST(41, Opcode::BoundsCheck).s32().Inputs(40, 4, 38);
            INST(42, Opcode::StoreArray).s32().Inputs(39, 41, 0);

            INST(44, Opcode::SaveState).Inputs(3, 24, 37).SrcVregs({0, 1, 2});
            INST(45, Opcode::NullCheck).ref().Inputs(37, 44);
            INST(46, Opcode::LenArray).s32().Inputs(45);
            INST(47, Opcode::BoundsCheck).s32().Inputs(46, 13, 44);
            INST(48, Opcode::StoreArray).s32().Inputs(45, 47, 43);

            // float array
            INST(49, Opcode::SaveState).Inputs(3, 24, 37).SrcVregs({0, 1, 2});
            INST(447, Opcode::LoadAndInitClass).ref().Inputs(49).TypeId(68);
            INST(50, Opcode::NegativeCheck).s32().Inputs(0, 49);
            INST(51, Opcode::NewArray).ref().Inputs(447, 50, 49);

            INST(53, Opcode::SaveState).Inputs(3, 24, 37, 51).SrcVregs({0, 1, 2, 3});
            INST(54, Opcode::NullCheck).ref().Inputs(51, 53);
            INST(55, Opcode::LenArray).s32().Inputs(54);
            INST(56, Opcode::BoundsCheck).s32().Inputs(55, 4, 53);
            INST(57, Opcode::StoreArray).f32().Inputs(54, 56, 52);

            INST(60, Opcode::SaveState).Inputs(3, 24, 37, 51).SrcVregs({0, 1, 2, 3});
            INST(61, Opcode::NullCheck).ref().Inputs(51, 60);
            INST(62, Opcode::LenArray).s32().Inputs(61);
            INST(63, Opcode::BoundsCheck).s32().Inputs(62, 13, 60);
            INST(64, Opcode::StoreArray).f32().Inputs(61, 63, 59);

            INST(66, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the newobj instruction
TEST_F(IrBuilderTest, Newobj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(){
        newobj v0, panda.String
        lda.obj v0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(0, Opcode::NewObject).ref().Inputs(3, 2);
            INST(1, Opcode::Return).ref().Inputs(0);
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
    .function R main(){
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
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).Inputs().SrcVregs({});
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0);
            INST(4, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(3, Opcode::CallStatic).v0id().Inputs({{DataType::REFERENCE, 2}, {DataType::NO_TYPE, 4}});
            INST(5, Opcode::Return).ref().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Enable after supporting MultiArray in panda assembly
TEST_F(IrBuilderTest, DISABLED_MultiArray)
{
    // TODO(pishin): fix ctor before enabling
    auto source = R"(
    .record __I <external>
    .function ___I _init__i32_i32_(i32 a0, i32 a1) <ctor, external>
    .record panda.Array <external>
    .function panda.Array main(i32 a0){
        movi v0, 0x1
        initobj _init__i32_i32_ v0, a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({1, 0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(4, Opcode::MultiArray)
                .ref()
                .Inputs({{DataType::REFERENCE, 3}, {DataType::INT32, 1}, {DataType::INT32, 0}, {DataType::NO_TYPE, 2}});
            INST(5, Opcode::Return).ref().Inputs(4);
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
    .function i32 main(R a0){
        ldobj a0, R.v_i32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s32().Inputs(2);
            INST(4, Opcode::Return).s32().Inputs(3);
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
    .function i64 main(R a0){
        ldobj.64 a0, R.v_i64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s64().Inputs(2);
            INST(4, Opcode::Return).s64().Inputs(3);
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
    .function panda.String main(R a0){
        ldobj.obj a0, R.v_string
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).ref().Inputs(2);
            INST(4, Opcode::Return).ref().Inputs(3);
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
    .function void main(R a0, i32 a1){
        lda a1
        stobj a0, R.v_i32
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 1).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).s32().Inputs(3, 1);
            INST(5, Opcode::ReturnVoid).v0id();
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
    .function void main(R a0, i64 a1){
        lda.64 a1
        stobj.64 a0, R.v_i64
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 1).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).s64().Inputs(3, 1);
            INST(5, Opcode::ReturnVoid).v0id();
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
    .function void main(R a0, panda.String a1){
        lda.obj a1
        stobj.obj a0, R.v_string
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 1).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).ref().Inputs(3, 1);
            INST(5, Opcode::ReturnVoid).v0id();
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
    .function i32 main(){
        ldstatic R.v_i32
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).SrcVregs({});
            INST(0, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(1, Opcode::LoadStatic).s32().Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
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
    .function i64 main(){
        ldstatic.64 R.v_i64
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).SrcVregs({});
            INST(0, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(1, Opcode::LoadStatic).s64().Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
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
    .function panda.String main(){
        ldstatic.obj R.v_string
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).SrcVregs({});
            INST(0, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(1, Opcode::LoadStatic).ref().Inputs(0);
            INST(2, Opcode::Return).ref().Inputs(1);
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
    .function void main(i32 a0){
        lda a0
        ststatic R.v_i32
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 0).SrcVregs({0, 1});
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(2, Opcode::StoreStatic).s32().Inputs(1, 0);
            INST(3, Opcode::ReturnVoid).v0id();
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
    .function void main(i64 a0){
        lda.64 a0
        ststatic.64 R.v_i64
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 0).SrcVregs({0, 1});
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(2, Opcode::StoreStatic).s64().Inputs(1, 0);
            INST(3, Opcode::ReturnVoid).v0id();
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
    .function void main(panda.String a0){
        lda.obj a0
        ststatic.obj R.v_string
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 0).SrcVregs({0, 1});
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(2, Opcode::StoreStatic).ref().Inputs(1, 0);
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return instruction
TEST_F(IrBuilderTest, Return)
{
    auto source = R"(
    .function i32 main(i32 a0){
        lda a0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).s32().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.64 instruction
TEST_F(IrBuilderTest, Return64)
{
    auto source = R"(
    .function i64 main(i64 a0){
        lda.64 a0
        return.64
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).s64().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.obj instruction
TEST_F(IrBuilderTest, ReturnObj)
{
    auto source = R"(
    .record panda.String <external>
    .function panda.String main(panda.String a0){
        lda.obj a0
        return.obj
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the return.void instruction
TEST_F(IrBuilderTest, ReturnVoid)
{
    auto source = R"(
    .function void main(){
        return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the throw instruction
TEST_F(IrBuilderTest, Throw)
{
    auto source = R"(
    .record panda.String <external>
    .function void main(panda.String a0){
        throw a0
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::Throw).Inputs(0, 1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call.short instruction
TEST_F(IrBuilderTest, CallShort)
{
    auto source = R"(
    .function i32 main(){
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
        CONSTANT(0, 1);

        BASIC_BLOCK(2, -1)
        {
            using namespace DataType;
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::CallStatic).s32().Inputs({{INT64, 0}, {NO_TYPE, 1}});
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({0, 1});
            INST(4, Opcode::CallStatic).s32().Inputs({{INT64, 0}, {INT64, 0}, {NO_TYPE, 3}});
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call instruction
TEST_F(IrBuilderTest, Call)
{
    auto source = R"(
    .function i64 main(){
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
        CONSTANT(0, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            using namespace DataType;
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::CallStatic).s64().Inputs({{INT32, 0}, {NO_TYPE, 1}});
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({0, 1});
            INST(4, Opcode::CallStatic).s64().Inputs({{INT32, 0}, {INT32, 0}, {NO_TYPE, 3}});
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checks the build of the call.range instruction
TEST_F(IrBuilderTest, CallRange)
{
    auto source = R"(
    .function i64 main(){
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
        CONSTANT(0, 1);
        CONSTANT(1, 2);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::CallStatic).s64().Inputs({{DataType::INT32, 0}, {DataType::NO_TYPE, 2}});
            INST(4, Opcode::Return).s64().Inputs(3);
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
    .function void main(){
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
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::SaveState).Inputs().SrcVregs({});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(1, Opcode::NewObject).ref().Inputs(6, 5);
            INST(2, Opcode::SaveState).Inputs(1, 1).SrcVregs({0, 1});
            INST(7, Opcode::LoadClass).ref().Inputs(2);
            INST(3, Opcode::CheckCast).Inputs(1, 7, 2);
            INST(4, Opcode::ReturnVoid).v0id();
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
    .function u1 main(){
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
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(1, Opcode::NewObject).ref().Inputs(5, 4);
            INST(6, Opcode::SaveState).Inputs(1, 1).SrcVregs({0, 1});
            INST(7, Opcode::LoadClass).ref().Inputs(6);
            INST(2, Opcode::IsInstance).b().Inputs(1, 7, 6);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(0, 2);
        CONSTANT(1, 1);
        CONSTANT(2, 0);

        BASIC_BLOCK(2, 3, 5, 6)
        {
            INST(5, Opcode::Try).CatchTypeIds({0x0, 0xE1});
        }
        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState).Inputs().SrcVregs({});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(14);
        }
        BASIC_BLOCK(4, 7, 5, 6) {}  // Try-end
        BASIC_BLOCK(7, -1)
        {
            INST(12, Opcode::Return).b().Inputs(0);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(13, Opcode::Return).b().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(0, 1);
        CONSTANT(1, 3);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 7, 4, 5)
        {
            INST(3, Opcode::Try).CatchTypeIds({0x0, 0xE1});
        }
        BASIC_BLOCK(7, 3)
        {
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(4);
        }
        BASIC_BLOCK(3, 6, 4, 5) {}  // Try-end
        BASIC_BLOCK(4, 6) {}
        BASIC_BLOCK(5, 6) {}
        BASIC_BLOCK(6, -1)
        {
            INST(11, Opcode::Phi).s32().Inputs({{3, 0}, {4, 2}, {5, 1}});
            INST(12, Opcode::Sub).s32().Inputs(11, 0);
            INST(13, Opcode::Return).b().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s64();
        CONSTANT(1, 100);

        BASIC_BLOCK(2, 3, 4, 5)
        {
            INST(2, Opcode::Try).CatchTypeIds({0xE1, 0x0});
        }
        BASIC_BLOCK(3, 6)
        {
            INST(3, Opcode::SaveState).Inputs(1, 0, 1).SrcVregs({0, 1, 2});
            INST(4, Opcode::ZeroCheck).s64().Inputs(0, 3);
            INST(5, Opcode::Div).s64().Inputs(1, 4);
            INST(6, Opcode::SaveState).Inputs(5, 0, 5).SrcVregs({0, 1, 2});
            INST(7, Opcode::ZeroCheck).s64().Inputs(0, 6);
            INST(8, Opcode::Div).s64().Inputs(5, 7);
        }
        BASIC_BLOCK(6, 7, 4, 5) {}  // Try-end
        BASIC_BLOCK(7, -1)
        {
            INST(9, Opcode::Return).s64().Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::CatchPhi).s64().Inputs(1, 5);
            INST(11, Opcode::Return).s64().Inputs(10);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::CatchPhi).s64().Inputs(1, 5);
            INST(13, Opcode::Return).s64().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(3, 10)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 0).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::ZeroCheck).s32().Inputs(1, 5);
            INST(7, Opcode::Div).s32().Inputs(0, 6);
        }
        BASIC_BLOCK(10, 9, 4) {}  // Try-end

        BASIC_BLOCK(4, 5) {}  // Catch-begin
        BASIC_BLOCK(5, 6, 7)
        {
            INST(11, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(6, 11)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1, 2, 0).SrcVregs({0, 1, 2, 3});
            INST(13, Opcode::ZeroCheck).s32().Inputs(2, 12);
            INST(14, Opcode::Div).s32().Inputs(0, 13);
        }
        BASIC_BLOCK(11, 9, 7) {}  // Try-end

        BASIC_BLOCK(7, 9)  // Catch-begin
        {
            INST(16, Opcode::Add).s32().Inputs(0, 3);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(17, Opcode::Phi).s32().Inputs(7, 14, 16);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(3, 6)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(6, Opcode::ZeroCheck).s32().Inputs(1, 5);
            INST(7, Opcode::Div).s32().Inputs(0, 6);
        }
        BASIC_BLOCK(6, 5, 4) {}  // Try-end
        BASIC_BLOCK(4, 5) {}     // Catch-begin
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).s32().Inputs(7, 0);
            INST(10, Opcode::Add).s32().Inputs(9, 2);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(2, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(3, 8)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(6, Opcode::ZeroCheck).s32().Inputs(1, 5);
            INST(7, Opcode::Div).s32().Inputs(0, 6);
        }
        BASIC_BLOCK(8, 5, 4) {}  // Try-end
        BASIC_BLOCK(4, 6)
        {
            INST(16, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
        }
        BASIC_BLOCK(6, 5, 7)
        {
            INST(9, Opcode::Phi).s32().Inputs(0, 12);
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(9, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(7, 6)
        {
            INST(12, Opcode::Sub).s32().Inputs(9, 3);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::Phi).s32().Inputs(7, 9);
            INST(14, Opcode::Return).s32().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(11, 1);
        CONSTANT(13, 0);

        BASIC_BLOCK(5, 6)
        {
            INST(9, Opcode::SaveState).Inputs().SrcVregs({});
            INST(10, Opcode::LoadAndInitClass).ref().Inputs(9);
            INST(2, Opcode::NewObject).ref().Inputs(10, 9);
        }
        BASIC_BLOCK(6, 2, 11)
        {
            INST(1, Opcode::Try).CatchTypeIds({0xE1});
        }
        BASIC_BLOCK(2, 7)  // try block
        {
            INST(3, Opcode::SaveState).Inputs(2).SrcVregs({1});
            INST(4, Opcode::Throw).Inputs(2, 3);
        }
        BASIC_BLOCK(7, -1, 11) {}  // try_end block

        BASIC_BLOCK(11, 3, 4)
        {
            INST(6, Opcode::CatchPhi).ref().Inputs();  // catch-phi for acc, which holds exception-object
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(6, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::Return).s32().Inputs(11);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::Return).s32().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 3).s32();
        CONSTANT(3, 0);
        CONSTANT(4, 1);

        BASIC_BLOCK(2, 3, 10)
        {
            INST(5, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).Inputs(0, 1, 2, 0).SrcVregs({0, 1, 2, 3});
            INST(7, Opcode::ZeroCheck).s32().Inputs(1, 6);
            INST(8, Opcode::Div).s32().Inputs(0, 7);
        }
        BASIC_BLOCK(4, 5, 10) {}  // Try-end
        BASIC_BLOCK(5, 6, 12)
        {
            INST(9, Opcode::Compare).b().CC(CC_EQ).Inputs(8, 3);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(6, 7, 10)
        {
            INST(11, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(7, 8)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1, 2, 0).SrcVregs({0, 1, 2, 3});
            INST(13, Opcode::ZeroCheck).s32().Inputs(2, 12);
            INST(14, Opcode::Div).s32().Inputs(0, 13);
        }
        BASIC_BLOCK(8, 9, 10) {}  // Try-end
        BASIC_BLOCK(9, 11, 12)
        {
            INST(15, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 3);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(11, 12) {}
        BASIC_BLOCK(10, 12)  // Catch-block
        {
            INST(18, Opcode::Add).s32().Inputs(0, 4);
        }
        BASIC_BLOCK(12, -1)
        {
            INST(19, Opcode::Phi).s32().Inputs(8, 14, 4, 18);
            INST(20, Opcode::Return).s32().Inputs(19);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Try).CatchTypeIds({0xE1});
        }
        BASIC_BLOCK(3, 9)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(6, Opcode::ZeroCheck).s32().Inputs(1, 5);
            INST(7, Opcode::Div).s32().Inputs(0, 6);
        }
        BASIC_BLOCK(9, 8, 4) {}  // Try-end
        BASIC_BLOCK(4, 6, 7)
        {
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).s32().Inputs(2);
        }
        BASIC_BLOCK(7, 8)
        {
            INST(13, Opcode::Sub).s32().Inputs(0, 3);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(14, Opcode::Phi).s32().Inputs(7, 13);
            INST(15, Opcode::Return).s32().Inputs(14);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(IrBuilderTest, OsrMode)
{
    auto source = R"(
    .function u32 foo(u32 a0){
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
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto expected_graph = CreateGraphOsrWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 5, 4)
        {
            INST(3, Opcode::Phi).s32().Inputs(0, 7);
            INST(4, Opcode::SaveStateOsr).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::Sub).s32().Inputs(3, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).u32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(IrBuilderTest, TestSaveStateDeoptimize)
{
    auto source = R"(
    .function u32 foo(u32 a0){
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
    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(3, Opcode::Phi).s32().Inputs(0, 7);
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::Sub).s32().Inputs(3, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).u32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(IrBuilderTest, InfiniteLoop)
{
    auto source = R"(
    .function u32 foo_inf(i32 a0){
    loop:
        inci a0, 1
        jmp loop
    }
    )";
    auto graph = CreateGraph();
    ASSERT_TRUE(ParseToGraph(source, "foo_inf", graph));
    ASSERT_FALSE(graph->HasEndBlock());
    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 1);
        BASIC_BLOCK(3, 2)
        {
            INST(4, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
        }
        BASIC_BLOCK(2, 2)
        {
            INST(2, Opcode::Phi).s32().Inputs(0, 3);
            INST(3, Opcode::Add).s32().Inputs(2, 1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(IrBuilderTest, TestSaveStateDeoptimizeAuxiliaryBlock)
{
    auto source = R"(
    .function u32 foo(i32 a0){
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
    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0);
        CONSTANT(20, 10);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 10, 11)
        {
            INST(10, Opcode::Compare).b().CC(CC_NE).Inputs(1, 0);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(11, 10) {}
        BASIC_BLOCK(10, 3)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 20}, {11, 0}});
            INST(4, Opcode::SaveStateDeoptimize).Inputs(0, 12).SrcVregs({0, 1});
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(3, Opcode::Phi).s32().Inputs(12, 7);
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::Sub).s32().Inputs(3, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).u32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
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
    EXPECT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(0, 1);
        CONSTANT(1, 0);
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1).SrcType(DataType::Type::INT32);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Return).s32().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0);
        CONSTANT(2, 2);
        CONSTANT(3, 1);

        BASIC_BLOCK(6, 2, 13, 14)
        {
            INST(4, Opcode::Try).CatchTypeIds({0xE1, 0xE2});
        }
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1).SrcType(DataType::Type::INT32);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::SaveState).Inputs(0, 0).SrcVregs({1, 2});
            INST(10, Opcode::LoadAndInitClass).ref().Inputs(9);
            INST(11, Opcode::NewObject).ref().Inputs(10, 9);
            INST(12, Opcode::SaveState).Inputs(11, 0, 0).SrcVregs({0, 1, 2});
            INST(13, Opcode::Throw).Inputs(11, 12);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(14, Opcode::SaveState).Inputs(0, 0).SrcVregs({1, 2});
            INST(15, Opcode::LoadAndInitClass).ref().Inputs(14);
            INST(16, Opcode::NewObject).ref().Inputs(15, 14);
            INST(17, Opcode::SaveState).Inputs(16, 0, 0).SrcVregs({0, 1, 2});
            INST(18, Opcode::Throw).Inputs(16, 17);
        }
        BASIC_BLOCK(7, -1, 13, 14) {}  // try end
        BASIC_BLOCK(14, 5) {}          // catch1
        BASIC_BLOCK(13, 5) {}          // catch2
        BASIC_BLOCK(5, -1)
        {
            INST(20, Opcode::Phi).s32().Inputs(2, 3);
            INST(21, Opcode::Return).s32().Inputs(20);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
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

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        PARAMETER(6, 0).s32();
        CONSTANT(8, 0x0).s64();

        BASIC_BLOCK(5, 2, 12)
        {
            INST(0, Opcode::Try).CatchTypeIds({0xE0});
        }
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(6, 8).SrcType(DataType::Type::INT32);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(7);
        }
        BASIC_BLOCK(4, 7)
        {
            INST(10, Opcode::SaveState).Inputs(6, 6).SrcVregs({2, 3});
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(10);
        }
        BASIC_BLOCK(7, 9, 12)
        {
            INST(1, Opcode::Try).CatchTypeIds({0xE1});
        }
        BASIC_BLOCK(9, 8)
        {
            INST(12, Opcode::SaveState).Inputs(6).SrcVregs({2});
            INST(13, Opcode::LoadAndInitClass).ref().Inputs(12);
            INST(14, Opcode::NewObject).ref().Inputs(13, 12);
            INST(15, Opcode::SaveState).Inputs(14, 6).SrcVregs({1, 2});
            INST(16, Opcode::Throw).Inputs(14, 15);
        }
        BASIC_BLOCK(8, 6, 12) {}   // try_end
        BASIC_BLOCK(6, -1, 12) {}  // try_end
        BASIC_BLOCK(12, 3) {}      // catch_begin, catch
        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).s32().Inputs(8);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
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
            auto try_bb = bb->GetSuccessor(0);
            EXPECT_TRUE(try_bb->IsTry());

            auto first_real_inst = try_bb->GetFirstInst();
            while (first_real_inst->IsSaveState()) {
                first_real_inst = first_real_inst->GetNext();
            }
            EXPECT_TRUE(GetGraph()->IsInstThrowable(first_real_inst));
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
    GRAPH(expected)
    {
        CONSTANT(7, 0);
        CONSTANT(10, 2);
        CONSTANT(14, 1);

        BASIC_BLOCK(2, 4)
        {
            INST(6, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(4, 3, 9)  // try_begin, loop
        {
            INST(9, Opcode::Phi).s32().Inputs(7, 13);
            INST(0, Opcode::Try).CatchTypeIds({0xE});
        }
        BASIC_BLOCK(3, 5)  // try, loop
        {
            INST(11, Opcode::SaveState).Inputs(10, 9).SrcVregs({0, 2});
            INST(12, Opcode::CallStatic).v0id().InputsAutoType(11);
            INST(13, Opcode::Add).s32().Inputs(9, 14);
        }
        BASIC_BLOCK(5, 4, 9) {}  // try_end, loop
        BASIC_BLOCK(9, -1)       // catch
        {
            INST(3, Opcode::CatchPhi).b().Inputs(9);
            INST(15, Opcode::Return).b().Inputs(3);
        }
    }
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
        BASIC_BLOCK(3, 2, 8)  // try_begin
        {
            INST(0, Opcode::Try).CatchTypeIds({0xE});
        }
        BASIC_BLOCK(2, 4)  // try
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::CallStatic).v0id().InputsAutoType(2);
            INST(4, Opcode::ReturnVoid).v0id();
        }
        BASIC_BLOCK(4, -1, 8) {}  // try_end
        BASIC_BLOCK(8, -1)
        {
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }

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
        BASIC_BLOCK(4, 2, 9)  // try_begin
        {
            INST(0, Opcode::Try).CatchTypeIds({0xE});
        }
        BASIC_BLOCK(2, 5)  // try
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::CallStatic).v0id().InputsAutoType(2);
        }
        BASIC_BLOCK(5, 3, 9) {}  // try_end
        BASIC_BLOCK(9, 3)        // catch
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(7, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(IrBuilderTest, CompareAnyType)
{
    // no crash.
    auto graph = CreateGraphWithDefaultRuntime();
    graph->SetDynamicMethod();
    GRAPH(graph)
    {
        PARAMETER(0, 0);
        INS(0).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::CompareAnyType).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }

    const CompareAnyTypeInst *cati = INS(2).CastToCompareAnyType();

    ASSERT_TRUE(cati != nullptr);
    EXPECT_TRUE(cati->GetInputType(0) == DataType::Type::ANY);
    EXPECT_TRUE(cati->GetType() == DataType::BOOL);
    EXPECT_TRUE(cati->GetAnyType() == AnyBaseType::UNDEFINED_TYPE);
}

TEST_F(IrBuilderTest, CastAnyTypeValue)
{
    // no crash.
    auto graph = CreateGraphWithDefaultRuntime();
    graph->SetDynamicMethod();
    GRAPH(graph)
    {
        PARAMETER(0, 0);
        INS(0).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::CastAnyTypeValue).AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0).u64();
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }

    const CastAnyTypeValueInst *catvi = INS(2).CastToCastAnyTypeValue();

    ASSERT_TRUE(catvi != nullptr);
    EXPECT_TRUE(catvi->GetInputType(0) == DataType::Type::ANY);
    EXPECT_TRUE(catvi->GetDeducedType() == DataType::Type::ANY);
    EXPECT_TRUE(catvi->GetAnyType() == AnyBaseType::UNDEFINED_TYPE);
}

TEST_F(IrBuilderTest, PhiWithIdenticalInputs)
{
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();

        BASIC_BLOCK(2, 3, 5)
        {
            INST(4, Opcode::Try).CatchTypeIds({0xE1});
        }
        BASIC_BLOCK(3, 4)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(4, 5) {}  // try-end
        BASIC_BLOCK(5, 6)
        {  // catch
            INST(7, Opcode::CatchPhi).s32().Inputs(0, 0);
        }
        BASIC_BLOCK(6, 7, 8)
        {
            INST(8, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1, 2);
        }
        BASIC_BLOCK(7, 8) {}
        BASIC_BLOCK(8, -1)
        {
            INST(10, Opcode::Phi).s32().Inputs(7, 7);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
    }
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    ASSERT_TRUE(RegAllocResolver(graph).ResolveCatchPhis());

    EXPECT_EQ(INS(11).GetInput(0).GetInst(), &INS(0));
}

}  // namespace panda::compiler
