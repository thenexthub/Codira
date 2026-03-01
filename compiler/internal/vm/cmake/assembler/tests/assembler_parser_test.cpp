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

#include <gtest/gtest.h>
#include <string>

#include "assembly-parser.h"
#include "operand_types_print.h"
#include "mangling.h"

using namespace testing::ext;

namespace panda::pandasm {

using namespace panda::pandasm;

class ParserTest : public testing::Test {
};

/**
 * @tc.name: parser_test_001
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_001, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("mov v1, v2}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->GetReg(0), 1) << "1 expected";
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->GetReg(1), 2) << "2 expected";
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_002
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_002, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("label:}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->Label(), "label") <<
        "label expected";
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->IsLabel(), true) << "true expected";
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->opcode, Opcode::INVALID) << "NONE expected";
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_003
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_003, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jlt v10, lab123}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_LABEL_EXT expected";
}

/**
 * @tc.name: parser_test_004
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_004, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("11111111}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION_NAME expected";
}

/**
 * @tc.name: parser_test_005
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_005, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 1}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_006
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_006, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 12345}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_007
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_007, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 11.3}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_008
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_008, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashdjbf iashudbfiun as}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

/**
 * @tc.name: parser_test_009
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_009, TestSize.Level1)
{
    const auto source = R"(
        .function any foo(any a0, any a1, any a2) {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            sta v4
            lda v4
            callarg0 0x1
            return
        }
    )";
    panda::pandasm::Parser p;
    auto res = p.Parse(source, "source");
    auto &program = res.Value();
    const std::string fun_name = "foo:(any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    auto &func = it->second;
    EXPECT_NE(func.ins[0]->opcode, Opcode::NOP);
    EXPECT_EQ(func.ins[3]->GetReg(0), 4U);
    EXPECT_EQ(func.ins[1]->GetReg(0), 1U);
    EXPECT_EQ(func.ins[5]->GetImm(0), IType(int64_t(0x1)));
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_010
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_010, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any func(any a0, any a1, any a2) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            ldglobalvar 0x0, "foo"
            sta v4
            lda v4
            callarg0 0x1
            return
        }

        .function any foo(any a0, any a1, any a2) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            ldai 0x1
            sta v3
            ldundefined
            returnundefined
        }
    )";
    auto item = p.Parse(source);
    GetFunctionSignatureFromName("func", {});
    const std::string func_name = "func:(any,any,any)";
    auto it = item.Value().function_table.find(func_name);
    EXPECT_NE(it, item.Value().function_table.end());
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3]->GetId(0), "foo") << "nain expected";
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[0]->GetReg(0), 0) << "1 expected";
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[0]->GetReg(1), 5) << "2 expected";
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_011
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_011, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("i64tof64}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME);
}

/**
 * @tc.name: parser_test_012
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_012, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp l123}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT) << "ERR_BAD_LABEL_EXT expected";
}

/**
 * @tc.name: parser_test_013
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_013, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("l123:").first);
    v.push_back(l.TokenizeString("jmp l123}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[1]->opcode, Opcode::JMP) << "ID expected";
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[1]->GetId(0), "l123") << "l123 expected";
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE";
}

/**
 * @tc.name: parser_test_014
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_014, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp 123}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_ID) << "ERR_BAD_NAME_ID expected";
}

/**
 * @tc.name: parser_test_015
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_015, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("shli 12 asd}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME);
}

/**
 * @tc.name: parser_test_016
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_016, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("l123: jmp l123}").first);

    auto item = p.Parse(v);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_017
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_017, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[0]->opcode, Opcode::RETURN) << "NONE expected";
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_018
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_018, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return1}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

/**
 * @tc.name: parser_test_019
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_019, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return 1}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS) << "ERR_BAD_NUMBER_OPERANDS expected";
}

/**
 * @tc.name: parser_test_020
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_020, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashr2.12 1234}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_NAME_REG expected";
}

/**
 * @tc.name: parser_test_021
 * @tc.desc: Verify the ERR_BAD_ARRAY_SIZE function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_021, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record Asm }{").first);
        v.push_back(l.TokenizeString("i64 asm1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_DEFINITION_METADATA);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record asm1 { i64 asm1{}").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_KEYWORD);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record asm1 { i64 asm1}").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".array array u8 3 { 1 2 }").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_SIZE);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".array array u8 3 { 1 2 3 4").first);
        v.push_back(l.TokenizeString("").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_END);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".array array u8 3 { i32:0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_INTEGER);
    }
}

/**
 * @tc.name: parser_test_022
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_022, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
    .function i32 main() {
            ldai -1
            sta v0
            jeq v0, label
            ldai 1
            return
        label:
            ldai 0
            return
        }
    .function f64 m123(u1 a0, f32 a1){
        la1:
            jeq v0, la1
            ldai 0
            return
        }
    )";
    auto item = p.Parse(source);

    std::vector<Function::Parameter> params1;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params1.emplace_back(Type {"u1", 0}, language);
    params1.emplace_back(Type {"f32", 0}, language);
    const auto sig_m123 = GetFunctionSignatureFromName("m123", params1);
    const std::string sig_main = "main:()";
    auto it = item.Value().function_table.find(sig_main);
    size_t num_size = 2U;

    auto &func_m123_at = item.Value().function_table.at(sig_m123).ins;
    auto &func_main_at = item.Value().function_table.at(sig_main).ins;

    EXPECT_NE(it, item.Value().function_table.end());
    EXPECT_EQ(item.Value().function_table.at(sig_main).name, sig_main);
    EXPECT_EQ(item.Value().function_table.at(sig_m123).name, sig_m123);
    EXPECT_EQ(item.Value().function_table.at(sig_main).GetParamsNum(), 0U);
    EXPECT_EQ(item.Value().function_table.at(sig_m123).GetParamsNum(), num_size);
    EXPECT_EQ(item.Value().function_table.at(sig_m123).params[0].type.GetId(), panda::panda_file::Type::TypeId::U1);
    EXPECT_EQ(item.Value().function_table.at(sig_m123).params[1].type.GetId(), panda::panda_file::Type::TypeId::F32);
    EXPECT_EQ(item.Value().function_table.at(sig_main).return_type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().function_table.at(sig_m123).return_type.GetId(), panda::panda_file::Type::TypeId::F64);

    auto func_m123_table = item.Value().function_table.at(sig_m123).label_table.at("la1").file_location;
    auto func_main_table = item.Value().function_table.at(sig_main).label_table.at("label").file_location;

    EXPECT_EQ(func_main_table->line_number, 5U);
    EXPECT_EQ(func_main_table->is_defined, true);
    EXPECT_EQ(func_m123_table->line_number, 13U);
    EXPECT_EQ(func_m123_table->is_defined, true);
    EXPECT_EQ(func_main_at[5]->opcode, Opcode::INVALID);
    EXPECT_EQ(func_main_at[5]->Label(), "label");
    EXPECT_EQ(func_main_at[1]->GetReg(0), 0U);
    EXPECT_EQ(func_main_at[2]->GetReg(0), 0U);
    EXPECT_EQ(func_main_at[3]->IsLabel(), false);
    EXPECT_EQ(func_m123_at[2]->opcode, Opcode::LDAI);
    EXPECT_EQ(func_m123_at[0]->opcode, Opcode::INVALID);
    EXPECT_EQ(func_m123_at[0]->Label(), "la1");
    EXPECT_EQ(func_m123_at[1]->IsLabel(), false);
    EXPECT_EQ(func_m123_at[1]->GetId(0), "la1");
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_023
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_023, TestSize.Level1)
{
    const auto source = R"(
        .function i32 main() {
        begin:
            ldai 2
            return
        end:
        catch:
            ldai 0
            return
        catch_all:
            ldai 1
            return
        .catch panda.ExceptionInInitializerError, begin, end, catch
        .catchall begin, end, catch_all
        }
    )";
    Parser p;
    auto item = p.Parse(source);
    const std::string sig_main = "main:()";
    auto it = item.Value().function_table.find(sig_main);
    EXPECT_NE(it, item.Value().function_table.end());
    EXPECT_EQ(item.Value().function_table.at(sig_main).name, sig_main);
    EXPECT_EQ(item.Value().function_table.at(sig_main).GetParamsNum(), 0U);
    EXPECT_EQ(item.Value().function_table.at(sig_main).return_type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[1]->opcode, Opcode::LDAI);
    EXPECT_EQ(item.Value().function_table.at(sig_main).ins[3]->IsLabel(), true);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: parser_test_024
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_024, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".record Asm2 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3 }").first);
    v.push_back(l.TokenizeString(".record Asm3").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".record Asm4 { i32 asm1 }").first);
    v.push_back(l.TokenizeString(".record Asm5 { i32 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().record_table.at("Asm1").name, "Asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].name, "asm2");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].name, "asm3");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().record_table.at("Asm2").name, "Asm2");
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[1].name, "asm2");
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[2].name, "asm3");
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().record_table.at("Asm3").name, "Asm3");
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[1].name, "asm2");
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[2].name, "asm3");
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().record_table.at("Asm4").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm4").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I32);
    EXPECT_EQ(item.Value().record_table.at("Asm5").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm5").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I32);
}

/**
 * @tc.name: parser_test_026
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_026, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u8 niam(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sig_niam = GetFunctionSignatureFromName("niam", {});
    EXPECT_EQ(item.Value().function_table.at(sig_niam).ins[0]->GetImm(0), IType(int64_t(-1))) << "-1 expected";
}

/**
 * @tc.name: parser_test_027
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_027, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam3()").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam4(){ldai -1}").first);

    v.push_back(l.TokenizeString(".function u8 niam5(){ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sig_niam1 = GetFunctionSignatureFromName("niam1", {});
    const auto sig_niam2 = GetFunctionSignatureFromName("niam2", {});
    const auto sig_niam3 = GetFunctionSignatureFromName("niam3", {});
    const auto sig_niam4 = GetFunctionSignatureFromName("niam4", {});
    const auto sig_niam5 = GetFunctionSignatureFromName("niam5", {});

    EXPECT_EQ(item.Value().function_table.at(sig_niam1).ins[0]->ins_debug.line_number, 2U) << "2 expected";
    EXPECT_EQ(item.Value().function_table.at(sig_niam2).ins[0]->ins_debug.line_number, 5U) << "5 expected";
    EXPECT_EQ(item.Value().function_table.at(sig_niam3).ins[0]->ins_debug.line_number, 9U) << "9 expected";
    EXPECT_EQ(item.Value().function_table.at(sig_niam4).ins[0]->ins_debug.line_number, 11U) << "11 expected";
    EXPECT_EQ(item.Value().function_table.at(sig_niam5).ins[0]->ins_debug.line_number, 12U) << "12 expected";
}

/**
 * @tc.name: parser_test_028
 * @tc.desc: Verify the record line_of_def function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_028, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record Asm2 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3 }").first);

    v.push_back(l.TokenizeString(".record Asm3").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record Asm4 { i32 asm1 }").first);

    v.push_back(l.TokenizeString(".record Asm5 { i32 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].line_of_def, 2U);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].line_of_def, 3U);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].line_of_def, 4U);

    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[0].line_of_def, 7U);
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[1].line_of_def, 8U);
    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[2].line_of_def, 9U);

    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[0].line_of_def, 12U);
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[1].line_of_def, 13U);
    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[2].line_of_def, 14U);

    EXPECT_EQ(item.Value().record_table.at("Asm4").field_list[0].line_of_def, 16U);

    EXPECT_EQ(item.Value().record_table.at("Asm5").field_list[0].line_of_def, 17U);
}

/**
 * @tc.name: parser_test_029
 * @tc.desc: Verify the record GetName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_029, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Asm {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("Asm asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3 }").first);

    v.push_back(l.TokenizeString(".record Asm2 { Asm1 asm1 }").first);

    v.push_back(l.TokenizeString(".record Asm3 { Asm2 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetName(), "Asm");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    EXPECT_EQ(item.Value().record_table.at("Asm2").field_list[0].type.GetName(), "Asm1");

    EXPECT_EQ(item.Value().record_table.at("Asm3").field_list[0].type.GetName(), "Asm2");
}

/**
 * @tc.name: parser_test_030
 * @tc.desc: Verify the GetItemName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_030, TestSize.Level1)
{
    EXPECT_EQ(GetOwnerName("Asm.main"), "Asm");

    EXPECT_EQ(GetOwnerName("main"), "");

    EXPECT_EQ(GetItemName("Asm.main"), "main");

    EXPECT_EQ(GetItemName("main"), "main");
}

/**
 * @tc.name: parser_test_031
 * @tc.desc: Verify the GetParamsNum function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_031, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(u1 a0, i64 a1, i32 a2){").first);
    v.push_back(l.TokenizeString("mov v0, v3").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sig_niam1 = GetFunctionSignatureFromName("niam1", {});
    int64_t value_param = 4;
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"i64", 0}, language);
    params.emplace_back(Type {"i32", 0}, language);
    const auto sig_niam2 = GetFunctionSignatureFromName("niam2", params);

    EXPECT_EQ(item.Value().function_table.at(sig_niam1).GetParamsNum(), 0U);
    EXPECT_EQ(item.Value().function_table.at(sig_niam1).value_of_first_param + 1, 0);

    EXPECT_EQ(item.Value().function_table.at(sig_niam2).GetParamsNum(), 3U);
    EXPECT_EQ(item.Value().function_table.at(sig_niam2).value_of_first_param + 1, value_param);
}

/**
 * @tc.name: parser_test_032
 * @tc.desc: Verify the Opcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_032, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain1(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain2(i64 a0) <> {   mov v0, a0}").first);
    v.push_back(l.TokenizeString(".function u1 nain3(i64 a0) <> {    mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain4(i64 a0) ").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain5(i64 a0) <>{").first);
    v.push_back(l.TokenizeString("mov v0, a0}").first);

    v.push_back(l.TokenizeString(".function u1 nain6(i64 a0) <>").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("mov v0, a0}").first);

    v.push_back(l.TokenizeString(".function u1 nain7(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain8(i64 a0) {   mov v0, a0}").first);
    v.push_back(l.TokenizeString(".function u1 nain9(i64 a0) {    mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain10(i64 a0) <>").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u1 nain11(i64 a0) {").first);
    v.push_back(l.TokenizeString("mov v0, a0}").first);

    v.push_back(l.TokenizeString(".function u1 nain12(i64 a0)").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("mov v0, a0}").first);

    auto item = p.Parse(v);

    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i64", 0}, language);
    const auto sig_nain1 = GetFunctionSignatureFromName("nain1", params);
    const auto sig_nain2 = GetFunctionSignatureFromName("nain2", params);
    const auto sig_nain3 = GetFunctionSignatureFromName("nain3", params);
    const auto sig_nain4 = GetFunctionSignatureFromName("nain4", params);
    const auto sig_nain5 = GetFunctionSignatureFromName("nain5", params);
    const auto sig_nain6 = GetFunctionSignatureFromName("nain6", params);
    const auto sig_nain7 = GetFunctionSignatureFromName("nain7", params);
    const auto sig_nain8 = GetFunctionSignatureFromName("nain8", params);
    const auto sig_nain9 = GetFunctionSignatureFromName("nain9", params);
    const auto sig_nain10 = GetFunctionSignatureFromName("nain10", params);
    const auto sig_nain11 = GetFunctionSignatureFromName("nain11", params);
    const auto sig_nain12 = GetFunctionSignatureFromName("nain12", params);

    EXPECT_EQ(item.Value().function_table.at(sig_nain1).name, sig_nain1);
    EXPECT_EQ(item.Value().function_table.at(sig_nain12).name, sig_nain12);
    EXPECT_EQ(item.Value().function_table.at(sig_nain3).name, sig_nain3);
    EXPECT_EQ(item.Value().function_table.at(sig_nain2).name, sig_nain2);
    EXPECT_EQ(item.Value().function_table.at(sig_nain4).name, sig_nain4);
    EXPECT_EQ(item.Value().function_table.at(sig_nain5).name, sig_nain5);
    EXPECT_EQ(item.Value().function_table.at(sig_nain6).name, sig_nain6);
    EXPECT_EQ(item.Value().function_table.at(sig_nain7).name, sig_nain7);
    EXPECT_EQ(item.Value().function_table.at(sig_nain8).name, sig_nain8);
    EXPECT_EQ(item.Value().function_table.at(sig_nain9).name, sig_nain9);
    EXPECT_EQ(item.Value().function_table.at(sig_nain10).name, sig_nain10);
    EXPECT_EQ(item.Value().function_table.at(sig_nain11).name, sig_nain11);

    EXPECT_EQ(item.Value().function_table.at(sig_nain1).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain2).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain3).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain4).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain5).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain6).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain7).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain8).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain9).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain10).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain11).ins[0]->opcode, Opcode::MOV);
    EXPECT_EQ(item.Value().function_table.at(sig_nain12).ins[0]->opcode, Opcode::MOV);
}

/**
 * @tc.name: parser_test_033
 * @tc.desc: Verify the GetId function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_033, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".record rec1 <> {").first);
    v.push_back(l.TokenizeString("i64 asm1 <>").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record rec2 <> {   i64 asm1}").first);
    v.push_back(l.TokenizeString(".record rec3 <> {    i64 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record rec4").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record rec5{").first);
    v.push_back(l.TokenizeString("i64 asm1}").first);

    v.push_back(l.TokenizeString(".record rec6").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("i64 asm1}").first);

    v.push_back(l.TokenizeString(".record rec7{").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().record_table.at("rec1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec2").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec3").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec4").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec5").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec6").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("rec7").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
}

/**
 * @tc.name: parser_test_034
 * @tc.desc: Verify the ins->opcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_034, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain1(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("sta a0").first);
    v.push_back(l.TokenizeString("L:").first);
    v.push_back(l.TokenizeString("mov v0, a0").first);
    v.push_back(l.TokenizeString("sta v4").first);
    v.push_back(l.TokenizeString("jmp L").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i64", 0}, language);
    const auto sig_nain1 = GetFunctionSignatureFromName("nain1", params);
    auto it = item.Value().function_table.find(sig_nain1);
    EXPECT_NE(it, item.Value().function_table.end());
    auto &func_table = item.Value().function_table.at(sig_nain1).ins;
    EXPECT_EQ(OperandTypePrint(func_table[0]->opcode), "reg");
    EXPECT_EQ(func_table[1]->opcode, Opcode::INVALID);
    EXPECT_EQ(OperandTypePrint(func_table[2]->opcode), "reg_reg");
    EXPECT_EQ(OperandTypePrint(func_table[3]->opcode), "reg");
    EXPECT_EQ(OperandTypePrint(func_table[4]->opcode), "label");
}

/**
 * @tc.name: parser_test_035
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_035, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string f = "T";

        v.push_back(l.TokenizeString(".record Rec {").first);
        v.push_back(l.TokenizeString(f).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_FIELD_MISSING_NAME);
        EXPECT_EQ(e.line_number, 2U);
        EXPECT_EQ(static_cast<uint32_t>(e.pos), f.length());
        EXPECT_EQ(e.message, "Expected field name.");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string f = "T f <";

        v.push_back(l.TokenizeString(".record Rec {").first);
        v.push_back(l.TokenizeString(f).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_METADATA_BOUND);
        EXPECT_EQ(e.line_number, 2U);
        EXPECT_EQ(static_cast<uint32_t>(e.pos), f.length());
        EXPECT_EQ(e.message, "Expected '>'.");
    }
}

/**
 * @tc.name: parser_test_036
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_036, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\z\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_UNKNOWN_ESCAPE_SEQUENCE);
        EXPECT_EQ(e.line_number, 2U);
        EXPECT_EQ(e.message, "Unknown escape sequence");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \" \\\" \\' \\\\ \\a \\b \\f \\n \\r \\t \\v \"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        Error e = p.ShowError();

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());
    }
}

/**
 * @tc.name: parser_test_037
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_037, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str 123";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        EXPECT_EQ(e.line_number, 2U);
        EXPECT_EQ(e.message, "Expected string literal");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str a\"bcd";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        EXPECT_EQ(e.line_number, 2U);
        EXPECT_EQ(e.message, "Expected string literal");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("lda.str \" abc123 \"").first);
        v.push_back(l.TokenizeString("lda.str \"zxcvb\"").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());

        auto str1 = item.Value().strings.find(" abc123 ");
        auto str2 = item.Value().strings.find("zxcvb");
        EXPECT_NE(str1, item.Value().strings.end());
        EXPECT_NE(str2, item.Value().strings.end());
    }
}

/**
 * @tc.name: parser_test_038
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_038, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\x\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();
        size_t line_nubmer = 2U;
        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        EXPECT_EQ(e.line_number, line_nubmer);
        size_t ret = op.find('\\');
        EXPECT_EQ(e.pos, ret);
        EXPECT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\xaa\\x65\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());
    }
}

/**
 * @tc.name: parser_test_039
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_039, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString("ldglobalvar 0x0, \"f\"").first);
        v.push_back(l.TokenizeString("callarg0 0x1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("main", {});

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {};
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[0]->Regs(), regs);
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[1]->Regs(), regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("callarg1 0x1, v0").first);
        v.push_back(l.TokenizeString("callarg1 0x1, v0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0};
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[0]->Regs(), regs);
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[1]->Regs(), regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("callargs2 0x1, v0, v1").first);
        v.push_back(l.TokenizeString("callargs2 0x1, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        EXPECT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0, 1};
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[0]->Regs(), regs);
        EXPECT_EQ(item.Value().function_table.at(sig_f).ins[1]->Regs(), regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("callargs3 0x1, v0; v1, v2").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        EXPECT_EQ(e.err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
}

/**
 * @tc.name: parser_test_040
 * @tc.desc: Verify the ERR_BAD_NUMBER_OPERANDS function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_040, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("stobjbyname 0xd, \"nain\", v0, v1").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }
}

/**
 * @tc.name: parser_test_041
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_041, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("stobjbyname 0x43, \"nain\", v0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_042
 * @tc.desc: Verify the ERR_BAD_OPERAND function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_042, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("mov v67000, v0").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

/**
 * @tc.name: parser_test_043
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_043, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("stobjbyname 0x43, \"f\", v256").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

/**
 * @tc.name: parser_test_044
 * @tc.desc: Verify the ERR_BAD_OPERATION_NAME function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_044, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("movi v5, 0").first);
    v.push_back(l.TokenizeString("call.this1 g, a1, v15").first);
    v.push_back(l.TokenizeString("return").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME);
}

/**
 * @tc.name: parser_test_045
 * @tc.desc: Verify the ERR_BAD_ID_FUNCTION function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_045, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

/**
 * @tc.name: parser_test_046
 * @tc.desc: Verify the ERR_BAD_LABEL_EXT function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_046, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("SAME: mov v0, a0").first);
    v.push_back(l.TokenizeString("SAME: sta v0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT);
}

/**
 * @tc.name: parser_test_047
 * @tc.desc: Verify the lable function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_047, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("nain: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_048
 * @tc.desc: Verify the lable function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_048, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_049
 * @tc.desc: Verify the mov lable function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_049, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("jmp mov").first);
    v.push_back(l.TokenizeString("mov:").first);
    v.push_back(l.TokenizeString("return").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_050
 * @tc.desc: Verify the callargs2 function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_050, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("callargs2 0x0c, v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_051
 * @tc.desc: Verify the ERR_BAD_NAME_REG function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_051, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("sta a01").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta q0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta vy1").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v01").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
}

/**
 * @tc.name: parser_test_052
 * @tc.desc: Verify the ERR_BAD_ARRAY_TYPE_BOUND function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_052, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record R {").first);
        v.push_back(l.TokenizeString("R[][] f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        std::vector<Function::Parameter> params;
        panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"i8", 1}, language);
        const auto sig_f = GetFunctionSignatureFromName("f", params);

        EXPECT_TRUE(item.HasValue());

        EXPECT_EQ(item.Value().record_table.at("R").field_list.size(), 1U);
        EXPECT_TRUE(item.Value().record_table.at("R").field_list[0].type.IsArray());
        EXPECT_TRUE(item.Value().record_table.at("R").field_list[0].type.IsObject());
        EXPECT_EQ(item.Value().record_table.at("R").field_list[0].type.GetName(), "R[][]");
        EXPECT_EQ(item.Value().record_table.at("R").field_list[0].type.GetComponentName(), "R");
        EXPECT_EQ(item.Value().record_table.at("R").field_list[0].type.GetDescriptor(), "[[LR;");
        EXPECT_EQ(item.Value().record_table.at("R").field_list[0].type.GetDescriptor(true), "[[LR;");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[][").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function f64[ f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32[][][ a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record R {").first);
        v.push_back(l.TokenizeString("R[][ f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }
}

/**
 * @tc.name: parser_test_053
 * @tc.desc: Verify the ERR_BAD_OPERATION_NAME function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_053, TestSize.Level1)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void main() <> {").first);
        v.push_back(l.TokenizeString("movi v0, 5").first);
        v.push_back(l.TokenizeString("newarr v0, v0, panda.String[]").first);
        v.push_back(l.TokenizeString("return.void").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void main() <> {").first);
        v.push_back(l.TokenizeString("mov v0, 5").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("return.void").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
}

/**
 * @tc.name: parser_test_054
 * @tc.desc: Verify the array function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_054, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("u1 1").first);
    v.push_back(l.TokenizeString("u8 2").first);
    v.push_back(l.TokenizeString("i8 -30").first);
    v.push_back(l.TokenizeString("u16 400").first);
    v.push_back(l.TokenizeString("i16 -5000").first);
    v.push_back(l.TokenizeString("u32 60000").first);
    v.push_back(l.TokenizeString("i32 -700000").first);
    v.push_back(l.TokenizeString("u64 8000000").first);
    v.push_back(l.TokenizeString("i64 -90000000").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    EXPECT_EQ(
        static_cast<uint64_t>(std::get<bool>(item.Value().literalarray_table.at("array").literals_[0].value_)), true);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::ARRAY_U8);
    int8_t u8 = 2;
    int8_t i8 = -30;
    EXPECT_EQ(
        static_cast<uint8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[1].value_)), u8);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_, panda::panda_file::LiteralTag::ARRAY_I8);
    EXPECT_EQ(static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[2].value_)),
              i8);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_, panda::panda_file::LiteralTag::ARRAY_U16);
    uint16_t u16 = 400;
    int16_t i16 = -5000;
    EXPECT_EQ(
        static_cast<uint16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array").literals_[3].value_)),
        u16);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_, panda::panda_file::LiteralTag::ARRAY_I16);
    EXPECT_EQ(static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array").literals_[4].value_)),
              i16);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[5].tag_, panda::panda_file::LiteralTag::ARRAY_U32);
    uint32_t ui32 = 60000;
    int32_t i32 = -700000;
    EXPECT_EQ(
        static_cast<uint32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[5].value_)),
       ui32);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[6].tag_, panda::panda_file::LiteralTag::ARRAY_I32);
    EXPECT_EQ(static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[6].value_)),
             i32);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[7].tag_, panda::panda_file::LiteralTag::ARRAY_U64);
    uint64_t ui64 = 8000000;
    int64_t i64 = -90000000;
    EXPECT_EQ(
        static_cast<uint64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array").literals_[7].value_)),
        ui64);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[8].tag_, panda::panda_file::LiteralTag::ARRAY_I64);
    EXPECT_EQ(static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array").literals_[8].value_)),
              i64);
}

/**
 * @tc.name: parser_test_055
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_055, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("f32 -123.4").first);
    v.push_back(l.TokenizeString("f64 -1234.5").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);
    double f32 = -123.4;
    double f64 = -1234.5;
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::ARRAY_F32);
    EXPECT_NEAR(std::get<float>(item.Value().literalarray_table.at("array").literals_[0].value_), f32, 0.01f);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::ARRAY_F64);
    EXPECT_NEAR(std::get<double>(item.Value().literalarray_table.at("array").literals_[1].value_), f64, 0.01f);
}

/**
 * @tc.name: parser_test_056
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_056, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("panda.String \"a\"").first);
    v.push_back(l.TokenizeString("panda.String \"ab\"").first);
    v.push_back(l.TokenizeString("panda.String \"abc\"").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".array array_static panda.String 3 { \"a\" \"ab\" \"abc\" }").first);
    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[0].value_), "a");
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[1].value_), "ab");
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[2].value_), "abc");

    // string intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_static").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_static").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_STRING));
    EXPECT_EQ(item.Value().literalarray_table.at("array_static").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_static").literals_[1].value_), 3);

    // string array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_static").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[2].value_), "a");
    EXPECT_EQ(item.Value().literalarray_table.at("array_static").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[3].value_), "ab");
    EXPECT_EQ(item.Value().literalarray_table.at("array_static").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[4].value_), "abc");
}

/**
 * @tc.name: parser_test_057
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_057, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array u1 3 { 1 0 1 }").first);
    auto item = p.Parse(v);

    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U1));
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[1].value_), 3);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    EXPECT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[2].value_), true);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    EXPECT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[3].value_), false);
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    EXPECT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[4].value_), true);
}

/**
 * @tc.name: parser_test_058
 * @tc.desc: Verify the array_unsigned_byte function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_058, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array_unsigned_byte u8 3 { 1 2 3 }").first);
    v.push_back(l.TokenizeString(".array array_byte i8 3 { -1 2 -3 }").first);
    v.push_back(l.TokenizeString(".array array_unsigned_short u16 3 { 100 200 300 }").first);
    v.push_back(l.TokenizeString(".array array_short i16 3 { 100 -200 300 }").first);
    v.push_back(l.TokenizeString(".array array_unsigned_int u32 3 { 1000 2000 3000 }").first);
    v.push_back(l.TokenizeString(".array array_int i32 3 { -1000 2000 -3000 }").first);
    v.push_back(l.TokenizeString(".array array_unsigned_long u64 3 { 10000 20000 30000 }").first);
    v.push_back(l.TokenizeString(".array array_long i64 3 { 10000 -20000 30000 }").first);
    auto item = p.Parse(v);

    // unsigned byte intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U8));
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    uint32_t u8_value = 3;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[1].value_),
              u8_value);

    // unsigned byte array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[2].value_), 1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    uint8_t u8_value1 = 2;
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[3].value_),
              u8_value1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    uint8_t u8_value2 = 3;
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[4].value_),
              u8_value2);

    // byte intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_byte").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I8));
    EXPECT_EQ(item.Value().literalarray_table.at("array_byte").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_byte").literals_[1].value_), u8_value);

    // byte array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_byte").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    EXPECT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[2].value_)),
        -1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_byte").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    EXPECT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[3].value_)),
        2);
    EXPECT_EQ(item.Value().literalarray_table.at("array_byte").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    EXPECT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[4].value_)),
        -3);

    // unsigned short intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U16));
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[1].value_),
              u8_value);

    // unsigned short array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    uint16_t u16_value = 100;
    EXPECT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[2].value_),
              u16_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    u16_value = 200;
    EXPECT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[3].value_),
              u16_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    u16_value = 300;
    EXPECT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[4].value_),
              u16_value);

    // short intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_short").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_short").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I16));
    EXPECT_EQ(item.Value().literalarray_table.at("array_short").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    uint32_t u32_value = 3;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_short").literals_[1].value_), u32_value);

    // short array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_short").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    int16_t i16_value = 100;
    EXPECT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[2].value_)),
        i16_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_short").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    i16_value = -200;
    EXPECT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[3].value_)),
        i16_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_short").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    i16_value = 300;
    EXPECT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[4].value_)),
        i16_value);

    // unsigned int intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U32));
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[1].value_), 3);

    // unsigned int array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    uint32_t lit_value = 1000;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[2].value_),
              lit_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    lit_value = 2000;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[3].value_),
              lit_value);
    EXPECT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    lit_value = 3000;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[4].value_),
              lit_value);

    // int intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_int").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_int").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I32));
    EXPECT_EQ(item.Value().literalarray_table.at("array_int").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[1].value_), 3);

    // int array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_int").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    int32_t lit_value1 = -1000;
    EXPECT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[2].value_)),
        lit_value1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_int").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    lit_value1 = 2000;
    EXPECT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[3].value_)),
        lit_value1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_int").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    lit_value1 = -3000;
    EXPECT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[4].value_)),
        lit_value1);

    // unsigned long intro literals
    auto literals = item.Value().literalarray_table.at("array_unsigned_long").literals_;
    EXPECT_EQ(literals[0].tag_, panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(literals[0].value_), static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U64));
    EXPECT_EQ(literals[1].tag_, panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(literals[1].value_), 3);

    // unsigned long array elements
    EXPECT_EQ(literals[2].tag_, panda::panda_file::LiteralTag::ARRAY_U64);
    EXPECT_EQ(static_cast<int64_t>(std::get<uint64_t>(literals[2].value_)), 10000);
    EXPECT_EQ(literals[3].tag_, panda::panda_file::LiteralTag::ARRAY_U64);
    EXPECT_EQ(static_cast<int64_t>(std::get<uint64_t>(literals[3].value_)), 20000);
    EXPECT_EQ(literals[4].tag_, panda::panda_file::LiteralTag::ARRAY_U64);
    EXPECT_EQ(static_cast<int64_t>(std::get<uint64_t>(literals[4].value_)), 30000);

    // long intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_long").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_long").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I64));
    EXPECT_EQ(item.Value().literalarray_table.at("array_long").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    uint32_t lit_value2 = 3;
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_long").literals_[1].value_), lit_value2);

    // long array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_long").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    int64_t i64_value1 = 10000;
    EXPECT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[2].value_)),
        i64_value1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_long").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    i64_value1 = -20000;
    EXPECT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[3].value_)),
        i64_value1);
    EXPECT_EQ(item.Value().literalarray_table.at("array_long").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    i64_value1 = 30000;
    EXPECT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[4].value_)),
        i64_value1);
}

/**
 * @tc.name: parser_test_059
 * @tc.desc: Verify the array_float function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_059, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array_float f32 3 { 12.3 -12.34 12.345 }").first);
    v.push_back(l.TokenizeString(".array array_double f64 3 { -120.3 120.34 -120.345 }").first);
    auto item = p.Parse(v);

    // float intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_float").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_float").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_F32));
    EXPECT_EQ(item.Value().literalarray_table.at("array_float").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_float").literals_[1].value_), 3);

    // float array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_float").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    float f_value = 12.3;
    double d_value = -12.34;
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[2].value_), f_value, 0.01f);
    EXPECT_EQ(item.Value().literalarray_table.at("array_float").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[3].value_), d_value,
                0.001f);
    EXPECT_EQ(item.Value().literalarray_table.at("array_float").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    f_value = 12.345;
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[4].value_), f_value,
                0.0001f);

    // double intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array_double").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_double").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_F64));
    EXPECT_EQ(item.Value().literalarray_table.at("array_double").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_double").literals_[1].value_), 3);

    // double array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array_double").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    d_value = -120.3;
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[2].value_), d_value,
                0.01f);
    EXPECT_EQ(item.Value().literalarray_table.at("array_double").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    d_value = 120.34;
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[3].value_), d_value,
                0.001f);
    EXPECT_EQ(item.Value().literalarray_table.at("array_double").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    d_value = -120.345;
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[4].value_), d_value,
                0.0001f);
}

/**
 * @tc.name: parser_test_060
 * @tc.desc: Verify the panda.String function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_060, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array panda.String 3 { \"a\" \"ab\" \"abc\" }").first);
    auto item = p.Parse(v);

    // string intro literals
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::TAGVALUE);
    EXPECT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_STRING));
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[1].value_), 3);

    // string array elements
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[2].value_), "a");
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[3].value_), "ab");
    EXPECT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    EXPECT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[4].value_), "abc");
}

/**
 * @tc.name: parser_test_061
 * @tc.desc: Verify the GetFunctionSignatureFromName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_061, TestSize.Level1)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}
        )";

    auto res = p.Parse(source);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";

    auto &program = res.Value();

    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i8", 0}, language);
    const auto sig_f = GetFunctionSignatureFromName("f", {});
    const auto sig_fi8 = GetFunctionSignatureFromName("f", params);

    EXPECT_TRUE(program.function_table.find(sig_f) != program.function_table.end());
    EXPECT_TRUE(program.function_table.find(sig_fi8) != program.function_table.end());
}

/**
 * @tc.name: parser_test_062
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_062, TestSize.Level1)
{
    Parser p;
    std::string source = R"(
            .function u1 f(i8 a0) {}
            .function i8 f(i8 a0) {}
        )";

    auto res = p.Parse(source);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

/**
 * @tc.name: parser_test_063
 * @tc.desc: Verify the ldglobalvar function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_063, TestSize.Level1)
{
    Parser p;
    std::string source = R"(
        .function void gg(u1 a0) {}
        .function void f() {
            ldglobalvar 0x9, "gg"
            sta v4
            lda v4
            callarg0 0x1
        }
    )";

    auto res = p.Parse(source);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

/**
 * @tc.name: parser_test_064
 * @tc.desc: Verify the ShowError function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ParserTest, parser_test_064, TestSize.Level1)
{
    Parser p;
    std::string source = R"(
        .function u1 f() {}
        .function u1 f(i8 a0) {}

        .function void main(u1 a0) {
            ldglobalvar 0x9, "f"
            sta v4
            ldai 0x1
            sta v5
            lda v4
            callarg1 0x4, v5
        }
    )";

    auto res = p.Parse(source);

    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}
}
