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

#include "operand_types_print.h"
#include "mangling.h"

#include <gtest/gtest.h>
#include <string>

namespace ark::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark::pandasm;

TEST(parsertests, test1)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("mov v1, v2}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 1) << "1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(1), 2) << "2 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test2)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("label:}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].Label(), "label") << "label expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].HasLabel(), true) << "true expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::INVALID) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test3)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jlt v10, lab123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT) << "ERR_BAD_LABEL_EXT expected";
}

TEST(parsertests, test4)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("11111111}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION_NAME expected";
}

TEST(parsertests, test5)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 1}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::ADDI) << "IMM expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(1))) << "1 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test6)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 12345}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::ADDI) << "IMM expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(12345)))
        << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test7)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 11.3}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME) << "ERR_NONE expected";
}

TEST(parsertests, test8)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashdjbf iashudbfiun as}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

TEST(parsertests, test9)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("lda v1").first);
    v.push_back(l.TokenizeString("movi v10, 1001}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::LDA) << "V expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 1) << "1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].opcode, Opcode::MOVI) << "V_IMM expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].GetReg(0), 10) << "10 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].GetImm(0U), Ins::IType(int64_t(1001)))
        << "1001 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test10)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 main(){").first);
    v.push_back(l.TokenizeString("call.short nain, v1, v2}").first);
    v.push_back(l.TokenizeString(".function u1 nain(){}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});
    const auto sigNain = GetFunctionSignatureFromName("nain", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::CALL_SHORT) << "V_V_ID expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetID(0), sigNain) << "nain expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 1) << "1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(1), 2) << "2 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test11)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("i64tof64}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::I64TOF64) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test12)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp l123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT) << "ERR_BAD_LABEL_EXT expected";
}

TEST(parsertests, test13)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("l123: jmp l123}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::JMP) << "ID expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetID(0), "l123") << "l123 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE";
}

TEST(parsertests, test14)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp 123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_ID) << "ERR_BAD_NAME_ID expected";
}

TEST(parsertests, test15)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("shli 12 asd}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS) << "ERR_BAD_NUMBER_OPERANDS expected";
}

TEST(parsertests, test17)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ldarr.8 v120}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::LDARR_8) << "V expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 120) << "120 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test18)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::RETURN) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test19)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

TEST(parsertests, test20)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return 1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS) << "ERR_BAD_NUMBER_OPERANDS expected";
}

TEST(parsertests, test21)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashr2.64 1234}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG) << "ERR_BAD_NAME_REG expected";
}

TEST(parsertests, test22)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashr2.64 v12}").first);
    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::ASHR2_64) << "V expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 12) << "12 expected";
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test23)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("label1:").first);
    v.push_back(l.TokenizeString("jle v0, label2").first);
    v.push_back(l.TokenizeString("movi v15, 26").first);
    v.push_back(l.TokenizeString("label2: mov v0, v1").first);
    v.push_back(l.TokenizeString("call m123, v2, v6, v3, v4").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function f64 m123(u1 a0, f32 a1){").first);
    v.push_back(l.TokenizeString("lda v10").first);
    v.push_back(l.TokenizeString("sta a0").first);
    v.push_back(l.TokenizeString("la1:").first);
    v.push_back(l.TokenizeString("jle a1, la1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"f32", 0}, language);
    const auto sigM123 = GetFunctionSignatureFromName("m123", params);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).name, sigMain);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).name, sigM123);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).GetParamsNum(), 0U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).GetParamsNum(), 2U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).params[0].type.GetId(), ark::panda_file::Type::TypeId::U1);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).params[1].type.GetId(), ark::panda_file::Type::TypeId::F32);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).returnType.GetId(), ark::panda_file::Type::TypeId::U8);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).returnType.GetId(), ark::panda_file::Type::TypeId::F64);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).labelTable.at("label1").fileLocation.lineNumber, 2U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).labelTable.at("label1").fileLocation.isDefined, true);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).labelTable.at("label2").fileLocation.lineNumber, 3U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).labelTable.at("label2").fileLocation.isDefined, true);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).labelTable.at("la1").fileLocation.lineNumber, 11U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).labelTable.at("la1").fileLocation.isDefined, true);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].opcode, Opcode::INVALID);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].Label(), "label1");
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].opcode, Opcode::JLE);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].GetReg(0), 0);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].GetID(0), "label2");
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[2].opcode, Opcode::MOVI);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[2].GetReg(0), 15);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[2].GetImm(0U), Ins::IType(int64_t(26)));
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[2].HasLabel(), false);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].GetReg(0), 0);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].GetReg(1), 1);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].Label(), "label2");
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].HasLabel(), true);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].opcode, Opcode::CALL);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetReg(0), 2);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetReg(1), 6);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetReg(2), 3);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetReg(3), 4);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetID(0), sigM123);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[0].opcode, Opcode::LDA);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[0].GetReg(0), 10);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[1].opcode, Opcode::STA);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[1].GetReg(0), 11);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[2].opcode, Opcode::INVALID);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[2].Label(), "la1");
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[2].HasLabel(), true);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[3].opcode, Opcode::JLE);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[3].GetReg(0), 12);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigM123).ins[3].GetID(0), "la1");
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test24_functions)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void main()").first);
    v.push_back(l.TokenizeString("{").first);
    v.push_back(l.TokenizeString("movi v0, 0x100").first);
    v.push_back(l.TokenizeString("movi v15, 0xffffffff").first);
    v.push_back(l.TokenizeString("movi v15, 0xf").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 1e3").first);
    v.push_back(l.TokenizeString("movi v15, 0xE994").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 1.1").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 1.").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, .1").first);
    v.push_back(l.TokenizeString("movi v15, 0").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 0.1").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 00.1").first);
    v.push_back(l.TokenizeString("fmovi.64 v15, 00.").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u8 niam(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigMain = GetFunctionSignatureFromName("main", {});
    const auto sigNiam = GetFunctionSignatureFromName("niam", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).returnType.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(256)))
        << "256 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[1].GetImm(0U), Ins::IType(int64_t(4294967295)))
        << "4294967295 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[2].GetImm(0U), Ins::IType(int64_t(15))) << "15 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[3].GetImm(0U), Ins::IType(1000.0)) << "1000.0 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[4].GetImm(0U), Ins::IType(int64_t(59796)))
        << "59796 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[5].GetImm(0U), Ins::IType(1.1)) << "1.1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[7].GetImm(0U), Ins::IType(.1)) << ".1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[8].GetImm(0U), Ins::IType(int64_t(0))) << "0 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[9].GetImm(0U), Ins::IType(0.1)) << "0.1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[10].GetImm(0U), Ins::IType(00.1)) << "00.1 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[11].GetImm(0U), Ins::IType(00.)) << "00. expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam).ins[0].GetImm(0U), Ins::IType(int64_t(-1))) << "-1 expected";
}

TEST(parsertests, test25_record_alone)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".record Asm {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().recordTable.at("Asm").name, "Asm");
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);
}

TEST(parsertests, test26_records)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    ASSERT_EQ(item.Value().recordTable.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").name, "Asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").name, "Asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().recordTable.at("Asm4").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm4").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().recordTable.at("Asm5").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm5").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I32);
}

TEST(parsertests, test27_record_and_function)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    const auto sigNiam = GetFunctionSignatureFromName("niam", {});

    ASSERT_EQ(item.Value().recordTable.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam).ins[0].GetImm(0U), Ins::IType(int64_t(-1))) << "-1 expected";
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test28_records_and_functions)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record Asm2 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".record Asm3 {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam3(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigNiam1 = GetFunctionSignatureFromName("niam1", {});
    const auto sigNiam2 = GetFunctionSignatureFromName("niam2", {});
    const auto sigNiam3 = GetFunctionSignatureFromName("niam3", {});

    ASSERT_EQ(item.Value().recordTable.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam1).ins[0].GetImm(0U), Ins::IType(int64_t(-1)))
        << "-1 expected";

    ASSERT_EQ(item.Value().recordTable.at("Asm2").name, "Asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam2).ins[0].GetImm(0U), Ins::IType(int64_t(-1)))
        << "-1 expected";

    ASSERT_EQ(item.Value().recordTable.at("Asm3").name, "Asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].name, "asm1");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[1].name, "asm2");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[2].name, "asm3");
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam3).ins[0].GetImm(0U), Ins::IType(int64_t(-1)))
        << "-1 expected";
}

TEST(parsertests, test29_instructions_def_lines)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    const auto sigNiam1 = GetFunctionSignatureFromName("niam1", {});
    const auto sigNiam2 = GetFunctionSignatureFromName("niam2", {});
    const auto sigNiam3 = GetFunctionSignatureFromName("niam3", {});
    const auto sigNiam4 = GetFunctionSignatureFromName("niam4", {});
    const auto sigNiam5 = GetFunctionSignatureFromName("niam5", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam1).ins[0].insDebug.LineNumber(), 2U) << "2 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam2).ins[0].insDebug.LineNumber(), 5U) << "5 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam3).ins[0].insDebug.LineNumber(), 9U) << "9 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam4).ins[0].insDebug.LineNumber(), 11U) << "11 expected";
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam5).ins[0].insDebug.LineNumber(), 12U) << "12 expected";
}

TEST(parsertests, test30_fields_def_lines)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].lineOfDef, 2U);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].lineOfDef, 3U);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].lineOfDef, 4U);

    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].lineOfDef, 7U);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[1].lineOfDef, 8U);
    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[2].lineOfDef, 9U);

    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].lineOfDef, 12U);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[1].lineOfDef, 13U);
    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[2].lineOfDef, 14U);

    ASSERT_EQ(item.Value().recordTable.at("Asm4").fieldList[0].lineOfDef, 16U);

    ASSERT_EQ(item.Value().recordTable.at("Asm5").fieldList[0].lineOfDef, 17U);
}

TEST(parsertests, test31_own_types)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[0].type.GetName(), "Asm");
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[1].type.GetId(), ark::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().recordTable.at("Asm1").fieldList[2].type.GetId(), ark::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().recordTable.at("Asm2").fieldList[0].type.GetName(), "Asm1");

    ASSERT_EQ(item.Value().recordTable.at("Asm3").fieldList[0].type.GetName(), "Asm2");
}

TEST(parsertests, test32_names)
{
    ASSERT_EQ(GetOwnerName("Asm.main"), "Asm");

    ASSERT_EQ(GetOwnerName("main"), "");

    ASSERT_EQ(GetItemName("Asm.main"), "main");

    ASSERT_EQ(GetItemName("main"), "main");
}

TEST(parsertests, test33_params_number)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(u1 a0, i64 a1, i32 a2){").first);
    v.push_back(l.TokenizeString("mov v0, v3").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigNiam1 = GetFunctionSignatureFromName("niam1", {});
    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"i64", 0}, language);
    params.emplace_back(Type {"i32", 0}, language);
    const auto sigNiam2 = GetFunctionSignatureFromName("niam2", params);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam1).GetParamsNum(), 0U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam1).valueOfFirstParam + 1, 0);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam2).GetParamsNum(), 3U);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam2).valueOfFirstParam + 1, 4);
}

TEST(parsertests, test34_vregs_number)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(u1 a0, i64 a1, i32 a2){").first);
    v.push_back(l.TokenizeString("mov v0, v5").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigNiam1 = GetFunctionSignatureFromName("niam1", {});
    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"i64", 0}, language);
    params.emplace_back(Type {"i32", 0}, language);
    const auto sigNiam2 = GetFunctionSignatureFromName("niam2", params);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam1).regsNum, 0U);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNiam2).regsNum, 6U);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test35_functions_bracket)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i64", 0}, language);
    const auto sigNain1 = GetFunctionSignatureFromName("nain1", params);
    const auto sigNain2 = GetFunctionSignatureFromName("nain2", params);
    const auto sigNain3 = GetFunctionSignatureFromName("nain3", params);
    const auto sigNain4 = GetFunctionSignatureFromName("nain4", params);
    const auto sigNain5 = GetFunctionSignatureFromName("nain5", params);
    const auto sigNain6 = GetFunctionSignatureFromName("nain6", params);
    const auto sigNain7 = GetFunctionSignatureFromName("nain7", params);
    const auto sigNain8 = GetFunctionSignatureFromName("nain8", params);
    const auto sigNain9 = GetFunctionSignatureFromName("nain9", params);
    const auto sigNain10 = GetFunctionSignatureFromName("nain10", params);
    const auto sigNain11 = GetFunctionSignatureFromName("nain11", params);
    const auto sigNain12 = GetFunctionSignatureFromName("nain12", params);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain1).name, sigNain1);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain12).name, sigNain12);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain3).name, sigNain3);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain2).name, sigNain2);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain4).name, sigNain4);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain5).name, sigNain5);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain6).name, sigNain6);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain7).name, sigNain7);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain8).name, sigNain8);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain9).name, sigNain9);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain10).name, sigNain10);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain11).name, sigNain11);

    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain1).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain2).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain3).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain4).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain5).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain6).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain7).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain8).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain9).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain10).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain11).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigNain12).ins[0].opcode, Opcode::MOV);
}

TEST(parsertests, test36_records_bracket)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    ASSERT_EQ(item.Value().recordTable.at("rec1").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec2").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec3").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec4").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec5").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec6").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().recordTable.at("rec7").fieldList[0].type.GetId(), ark::panda_file::Type::TypeId::I64);
}

TEST(parsertests, test37_operand_type_print)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain1(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("movi v0, 0").first);
    v.push_back(l.TokenizeString("jmp L").first);
    v.push_back(l.TokenizeString("sta a0").first);
    v.push_back(l.TokenizeString("neg").first);
    v.push_back(l.TokenizeString("call.short nain1, v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i64", 0}, language);
    const auto sigNain1 = GetFunctionSignatureFromName("nain1", params);

    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[0].opcode), "reg_reg");
    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[1].opcode), "reg_imm");
    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[2].opcode), "label");
    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[3].opcode), "reg");
    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[4].opcode), "none");
    ASSERT_EQ(OperandTypePrint(item.Value().functionStaticTable.at(sigNain1).ins[5].opcode), "call_reg_reg");
}

TEST(parsertests, test38_record_invalid_field)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string f = "T";

        v.push_back(l.TokenizeString(".record Rec {").first);
        v.push_back(l.TokenizeString(f).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_FIELD_MISSING_NAME);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, f.length());
        ASSERT_EQ(e.message, "Expected field name.");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string f = "T f <";

        v.push_back(l.TokenizeString(".record Rec {").first);
        v.push_back(l.TokenizeString(f).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_METADATA_BOUND);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, f.length());
        ASSERT_EQ(e.message, "Expected '>'.");
    }
}

TEST(parsertests, test39_parse_operand_string)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str 123";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find(' ') + 1);
        ASSERT_EQ(e.message, "Expected string literal");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str a\"bcd";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find(' ') + 1);
        ASSERT_EQ(e.message, "Expected string literal");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("lda.str \" abc123 \"").first);
        v.push_back(l.TokenizeString("lda.str \"zxcvb\"").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        Program::StringT strings = {" abc123 ", "zxcvb"};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

TEST(parsertests, test40_parse_operand_string_escape_seq)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\z")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_UNKNOWN_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Unknown escape sequence");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str " \" \' \\ \a \b \f \n \r \t \v ")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        Error e = p.ShowError();

        auto item = p.Parse(v);

        Program::StringT strings = {" \" ' \\ \a \b \f \n \r \t \v "};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test41_parse_operand_string_hex_escape_seq)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\x")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\xZZ")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\xAZ")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\xZA")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.lineNumber, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = R"(lda.str "123\xaa\x65")";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        Program::StringT strings = {"123\xaa\x65"};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

TEST(parsertests, test42_parse_operand_string_octal_escape_seq)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string op = R"(lda.str "123\1\02\00123")";

    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString(op).first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    Program::StringT strings = {"123\1\02\00123"};

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_TRUE(item.HasValue());
    ASSERT_EQ(item.Value().strings, strings);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test43_call_short)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].RegSize(), 0U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0").first);
        v.push_back(l.TokenizeString("call.virt.short A.f, v0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0, v1").first);
        v.push_back(l.TokenizeString("call.virt.short A.f, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(1U), 1U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.virt.short f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test44_call)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].RegSize(), 0U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0").first);
        v.push_back(l.TokenizeString("call.virt A.f, v0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1").first);
        v.push_back(l.TokenizeString("call.virt A.f, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(1U), 1U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("call.virt A.f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(2U), 2U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(2U), 2U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1, v2, v3").first);
        v.push_back(l.TokenizeString("call.virt A.f, v0, v1, v2, v3").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sigF = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(2U), 2U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetReg(3U), 3U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(0U), 0U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(1U), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(2U), 2U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetReg(3U), 3U);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0, v1, v2, v3, v4").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record A {}").first);
        v.push_back(l.TokenizeString(".function void A.f(A a0) {").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.virt.short A.f, v0, v1, v2, v3, v4").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }
}

TEST(parsertests, function_argument_mismatch)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("call.short nain, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("call.range nain, v0").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("call nain, v0").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_FUNCTION_ARGUMENT_MISMATCH);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("call nain, v0, v1, v2, v3").first);
        v.push_back(l.TokenizeString("}").first);
        v.push_back(l.TokenizeString(".function u8 nain(i32 a0, i32 a1){").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, test45_argument_width_mov)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("mov v67000, v0").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

TEST(parsertests, test45_argument_width_call)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("call.range f, v256").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

TEST(parsertests, test_argument_width_call_param)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function void g(u1 a0, u1 a1) {").first);
    v.push_back(l.TokenizeString("call.range f, v256").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("movi v5, 0").first);
    v.push_back(l.TokenizeString("call g, a1, v15").first);
    v.push_back(l.TokenizeString("return").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

TEST(parsertests, test_argument_virt_call_param)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".record A {}").first);
    v.push_back(l.TokenizeString(".function void A.constructor(A a0) <ctor> {").first);
    v.push_back(l.TokenizeString("return.void").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function void A.func(A a0) <static> {").first);
    v.push_back(l.TokenizeString("return.void").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function i32 main() {").first);
    v.push_back(l.TokenizeString("initobj.short A.constructor").first);
    v.push_back(l.TokenizeString("sta.obj v0").first);
    v.push_back(l.TokenizeString("call.virt.range A.func, v0").first);
    v.push_back(l.TokenizeString("ldai 0").first);
    v.push_back(l.TokenizeString("return").first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERAND);
}

TEST(parsertests, Naming_function_function)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, Naming_function_function_static)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_function_static2)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_function_static3)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, Naming_function_function_static4)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, Naming_function_function_static5)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 foo(A a0) {").first);
    v.push_back(l.TokenizeString("L: call.short A.nain, v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_function_static6)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1, i32 a2) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 foo(A a0) {").first);
    v.push_back(l.TokenizeString("L: call A.nain:(A,i64,i32), v0, v1, v2").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_function_static7)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1, i32 a2) {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 A.nain(A a0, i64 a1) <static> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function u1 foo(A a0) {").first);
    v.push_back(l.TokenizeString("L: call.short A.nain:(A,i64), v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_label_label)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("SAME: mov v0, a0").first);
    v.push_back(l.TokenizeString("SAME: sta v0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT);
}

TEST(parsertests, Naming_function_label)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("nain: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_operation)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("L: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_label_operation)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("mov: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_function_label_operation)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("mov: mov v0, a0").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_jump_label)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("jmp mov").first);
    v.push_back(l.TokenizeString("mov:").first);
    v.push_back(l.TokenizeString("return").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, Naming_call_function)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("call.short mov, v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, register_naming_incorr)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("sta a01").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta q0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta vy1").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v01").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
}
TEST(parsertests, register_naming_corr)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("sta a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("mov v0, a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, array_type)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record R {").first);
        v.push_back(l.TokenizeString("R[][] f").first);
        v.push_back(l.TokenizeString("}").first);

        v.push_back(l.TokenizeString(".function R[] f(i8[ ] a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[  ][]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        std::vector<Function::Parameter> params;
        ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"i8", 1}, language);
        const auto sigF = GetFunctionSignatureFromName("f", params);

        ASSERT_TRUE(item.HasValue());

        ASSERT_EQ(item.Value().recordTable.at("R").fieldList.size(), 1U);
        ASSERT_TRUE(item.Value().recordTable.at("R").fieldList[0].type.IsArray());
        ASSERT_TRUE(item.Value().recordTable.at("R").fieldList[0].type.IsObject());
        ASSERT_EQ(item.Value().recordTable.at("R").fieldList[0].type.GetName(), "R[][]");
        ASSERT_EQ(item.Value().recordTable.at("R").fieldList[0].type.GetComponentName(), "R");
        ASSERT_EQ(item.Value().recordTable.at("R").fieldList[0].type.GetDescriptor(), "[[LR;");

        ASSERT_TRUE(item.Value().functionStaticTable.at(sigF).returnType.IsArray());
        ASSERT_TRUE(item.Value().functionStaticTable.at(sigF).returnType.IsObject());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).returnType.GetName(), "R[]");
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).returnType.GetComponentName(), "R");
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).returnType.GetDescriptor(), "[LR;");

        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).params.size(), 1U);
        ASSERT_TRUE(item.Value().functionStaticTable.at(sigF).params[0].type.IsArray());
        ASSERT_TRUE(item.Value().functionStaticTable.at(sigF).params[0].type.IsObject());
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).params[0].type.GetName(), "i8[]");
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).params[0].type.GetComponentName(), "i8");
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).params[0].type.GetDescriptor(), "[B");

        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].IDSize(), 1U);
        ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetID(0), "i32[][]");
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[][").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function f64[ f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32[][][ a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record R {").first);
        v.push_back(l.TokenizeString("R[][ f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }
}

TEST(parsertests, undefined_type)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void main() <> {").first);
        v.push_back(l.TokenizeString("movi v0, 5").first);
        v.push_back(l.TokenizeString("newarr v0, v0, panda.String[]").first);
        v.push_back(l.TokenizeString("return.void").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".record panda.String <external>").first);
        v.push_back(l.TokenizeString(".function void main() <> {").first);
        v.push_back(l.TokenizeString("movi v0, 5").first);
        v.push_back(l.TokenizeString("newarr v0, v0, panda.String[]").first);
        v.push_back(l.TokenizeString("return.void").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void main() <> {").first);
        v.push_back(l.TokenizeString("movi v0, 5").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("return.void").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, parse_undefined_record_field)
{
    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                newobj v0, ObjWrongType
                lda.obj v0
                return
            }

            .record ObjType {}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
        ASSERT_EQ(e.lineNumber, 3);
        ASSERT_EQ(e.pos, 27);
        ASSERT_EQ(e.message, "Record 'ObjWrongType' does not exist.");
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                newobj v0, ObjType
                lda.obj v0
                return
            }

            .record ObjType {}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                ldobj v0, ObjWrongType.fld
                return
            }

            .record ObjType {
                i32 fld
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
        ASSERT_EQ(e.lineNumber, 3);
        ASSERT_EQ(e.pos, 26);
        ASSERT_EQ(e.message, "Record 'ObjWrongType' does not exist.");
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                ldobj v0, ObjType.fldwrong
                return
            }

            .record ObjType {
                i32 fld
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_FIELD);
        ASSERT_EQ(e.lineNumber, 3);
        ASSERT_EQ(e.pos, 34);
        ASSERT_EQ(e.message, "This field does not exist.");
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                ldobj v0, ObjType.fld
                return
            }

            .record ObjType {
                i32 fld
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                lda.type i64[]
                return
            }

            .record ObjType {
                i32 fld
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .record panda.String <external>

            .function panda.String panda.NullPointerException.getMessage(panda.NullPointerException a0) {
                ldobj.obj a0, panda.NullPointerException.messagewrong
                return.obj
            }

            .record panda.NullPointerException {
                panda.String message
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_FIELD);
        ASSERT_EQ(e.lineNumber, 5);
        ASSERT_EQ(e.pos, 57);
        ASSERT_EQ(e.message, "This field does not exist.");
    }

    {
        Parser p;
        std::string source = R"(
            .record panda.String <external>

            .function panda.String panda.NullPointerException.getMessage(panda.NullPointerException a0) {
                ldobj.obj a0, panda.NullPointerException.message
                return.obj
            }

            .record panda.NullPointerException {
                panda.String message
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main(u1 a0) {
                newarr a0, a0, ObjWrongType[]
                return
            }

            .record ObjType {}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
        ASSERT_EQ(e.lineNumber, 3);
        ASSERT_EQ(e.pos, 44);
        ASSERT_EQ(e.message, "Record 'ObjWrongType' does not exist.");
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main(u1 a0) {
                newarr a0, a0, ObjType[]
                return
            }

            .record ObjType {}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, Vreg_width)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
        v.push_back(l.TokenizeString("mov v999, a0").first);
        v.push_back(l.TokenizeString("movi a0, 0").first);
        v.push_back(l.TokenizeString("lda a0").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("mov a0, v999").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }

    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u1 nain(i64 a0) <> {").first);
        v.push_back(l.TokenizeString("movi.64 v15, 222").first);
        v.push_back(l.TokenizeString("call bar, a0, v0").first);
        v.push_back(l.TokenizeString("return").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, Num_vregs)
{
    {
        Parser p;
        std::string source = R"(
            .record KKK{}

            .function u1 main(u1 a0) {
                movi v1, 1
                mov v0, a0

                return
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        auto &program = res.Value();

        std::vector<Function::Parameter> params;
        ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"u1", 0}, language);
        const auto sigMain = GetFunctionSignatureFromName("main", params);

        auto itFunc = program.functionStaticTable.find(sigMain);

        ASSERT_TRUE(itFunc != program.functionStaticTable.end());
        ASSERT_EQ(itFunc->second.regsNum, 2);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main(u1 a0) {
                movi v1, 1
                mov v0, a0

                return
            }

            .record KKK{}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        std::vector<Function::Parameter> params;
        ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"u1", 0}, language);
        const auto sigMain = GetFunctionSignatureFromName("main", params);

        auto &program = res.Value();
        auto itFunc = program.functionStaticTable.find(sigMain);

        ASSERT_TRUE(itFunc != program.functionStaticTable.end());
        ASSERT_EQ(itFunc->second.regsNum, 2);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                movi v0, 1

                return
            }

            .record KKK{}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        const auto sigMain = GetFunctionSignatureFromName("main", {});

        auto &program = res.Value();
        auto itFunc = program.functionStaticTable.find(sigMain);

        ASSERT_TRUE(itFunc != program.functionStaticTable.end());
        ASSERT_EQ(itFunc->second.regsNum, 1);
    }

    {
        Parser p;
        std::string source = R"(
            .function u1 main() {
                movi v1, 1
                movi v0, 0
                movi v2, 2
                movi v3, 3
                movi v4, 4

                return
            }

            .record KKK{}
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        const auto sigMain = GetFunctionSignatureFromName("main", {});

        auto &program = res.Value();
        auto itFunc = program.functionStaticTable.find(sigMain);

        ASSERT_TRUE(itFunc != program.functionStaticTable.end());
        ASSERT_EQ(itFunc->second.regsNum, 5);
    }
}

TEST(parsertests, Bad_imm_value)
{
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,.").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,%").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
    {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,;").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
}

TEST(parsertests, parse_catch_directive_1)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".record Exception {}").first);
    v.push_back(l.TokenizeString(".catch Exception, try_begin, try_end, catch_begin").first);

    p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
    ASSERT_EQ(e.lineNumber, 2);
    ASSERT_EQ(e.message, ".catch directive is located outside of a function body.");
}

TEST(parsertests, parse_catch_directive_2)
{
    std::vector<std::string> directives {
        ".catch",        ".catch R",         ".catch R,",         ".catch R, t1",
        ".catch R, t1,", ".catch R, t1, t2", ".catch R, t1, t2,", ".catch R, t1, t2, c,"};

    for (const auto &s : directives) {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record Exception {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(s).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        ASSERT_EQ(e.lineNumber, 3);
        ASSERT_EQ(e.pos, 0);
        ASSERT_EQ(e.message,
                  "Incorrect catch block declaration. Must be in the format: .catch <exception_record>, "
                  "<try_begin_label>, <try_end_label>, <catch_begin_label>[, <catch_end_label>]");
    }
}

TEST(parsertests, parse_catch_directive_3)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catch $Exception, try_begin, try_end, catch_begin";

    v.push_back(l.TokenizeString(".record $Exception {}").first);
    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString("try_begin:").first);
    v.push_back(l.TokenizeString("try_end:").first);
    v.push_back(l.TokenizeString("catch_begin:").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    Error e = p.ShowError();
    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, parse_catch_directive_4)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catch 4Exception, try_begin, try_end, catch_begin";

    v.push_back(l.TokenizeString(".record Exception {}").first);
    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    Error e = p.ShowError();
    ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_RECORD_NAME);
    ASSERT_EQ(e.lineNumber, 3);
    ASSERT_EQ(e.pos, s.find('4'));
    ASSERT_EQ(e.message, "Invalid name of the exception record.");
}

TEST(parsertests, parse_catch_directive_5)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catch Exception-record, try_begin, try_end, catch_begin";

    v.push_back(l.TokenizeString(".record Exception-record {}").first);
    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString("try_begin:").first);
    v.push_back(l.TokenizeString("try_end:").first);
    v.push_back(l.TokenizeString("catch_begin:").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    p.Parse(v);

    Error e = p.ShowError();
    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, parse_catch_directive_6)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};

    for (size_t i = 0; i < labels.size(); i++) {
        std::string directive = ".catch Exception";
        for (size_t j = 0; j < labels.size(); j++) {
            directive += i == j ? " $ " : " , ";
            directive += labels[j];
        }

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record Exception {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(directive).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();
        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION) << "Test " << directive;
        ASSERT_EQ(e.lineNumber, 3) << "Test " << directive;
        ASSERT_EQ(e.pos, directive.find('$')) << "Test " << directive;
        ASSERT_EQ(e.message, "Expected comma.") << "Test " << directive;
    }
}

TEST(parsertests, parse_catch_directive_7)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};

    for (size_t i = 0; i < labels.size(); i++) {
        std::stringstream ss;
        ss << "Test " << labels[i] << " does not exists";

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string catchTable = ".catch Exception, try_begin, try_end, catch_begin";

        v.push_back(l.TokenizeString(".record Exception {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        for (size_t j = 0; j < labels.size(); j++) {
            if (i != j) {
                v.push_back(l.TokenizeString(labels[j] + ":").first);
            }
        }
        v.push_back(l.TokenizeString(catchTable).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL_EXT) << ss.str();
        ASSERT_EQ(e.pos, catchTable.find(labels[i])) << ss.str();
        ASSERT_EQ(e.message, "This label does not exist.") << ss.str();
    }
}

TEST(parsertests, parse_catch_directive_8)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catch Exception, try_begin, try_end, catch_begin";

    v.push_back(l.TokenizeString(".record Exception {}").first);
    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString("try_begin:").first);
    v.push_back(l.TokenizeString("try_end:").first);
    v.push_back(l.TokenizeString("catch_begin:").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();
    auto &function = program.functionStaticTable.find(sigMain)->second;

    ASSERT_EQ(function.catchBlocks.size(), 1);
    ASSERT_EQ(function.catchBlocks[0].exceptionRecord, "Exception");
    ASSERT_EQ(function.catchBlocks[0].tryBeginLabel, "try_begin");
    ASSERT_EQ(function.catchBlocks[0].tryEndLabel, "try_end");
    ASSERT_EQ(function.catchBlocks[0].catchBeginLabel, "catch_begin");
    ASSERT_EQ(function.catchBlocks[0].catchEndLabel, "catch_begin");
}

TEST(parsertests, parse_catch_directive_9)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catch Exception, try_begin, try_end, catch_begin, catch_end";

    v.push_back(l.TokenizeString(".record Exception {}").first);
    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString("try_begin:").first);
    v.push_back(l.TokenizeString("try_end:").first);
    v.push_back(l.TokenizeString("catch_begin:").first);
    v.push_back(l.TokenizeString("catch_end:").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();
    auto &function = program.functionStaticTable.find(sigMain)->second;

    ASSERT_EQ(function.catchBlocks.size(), 1);
    ASSERT_EQ(function.catchBlocks[0].exceptionRecord, "Exception");
    ASSERT_EQ(function.catchBlocks[0].tryBeginLabel, "try_begin");
    ASSERT_EQ(function.catchBlocks[0].tryEndLabel, "try_end");
    ASSERT_EQ(function.catchBlocks[0].catchBeginLabel, "catch_begin");
    ASSERT_EQ(function.catchBlocks[0].catchEndLabel, "catch_end");
}

TEST(parsertests, parse_catchall_directive1)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".catchall try_begin, try_end, catch_begin").first);

    p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
    ASSERT_EQ(e.lineNumber, 1);
    ASSERT_EQ(e.message, ".catchall directive is located outside of a function body.");
}

TEST(parsertests, parse_catchall_directive2)
{
    std::vector<std::string> directives {".catchall",        ".catchall t1",      ".catchall t1,",
                                         ".catchall t1, t2", ".catchall t1, t2,", ".catchall t1, t2, c,"};

    for (const auto &s : directives) {
        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(s).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        ASSERT_EQ(e.lineNumber, 2);
        ASSERT_EQ(e.pos, 0);
        ASSERT_EQ(e.message,
                  "Incorrect catch block declaration. Must be in the format: .catchall <try_begin_label>, "
                  "<try_end_label>, <catch_begin_label>[, <catch_end_label>]");
    }
}

TEST(parsertests, parse_catchall_directive3)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};

    for (size_t i = 1; i < labels.size(); i++) {
        std::string directive = ".catchall ";

        for (size_t j = 0; j < labels.size(); j++) {
            if (j != 0) {
                directive += i == j ? " $ " : " , ";
            }
            directive += labels[j];
        }

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(directive).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION) << "Test " << directive;
        ASSERT_EQ(e.lineNumber, 2) << "Test " << directive;
        ASSERT_EQ(e.pos, directive.find('$')) << "Test " << directive;
        ASSERT_EQ(e.message, "Expected comma.") << "Test " << directive;
    }
}

TEST(parsertests, parse_catchall_directive4)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};

    for (size_t i = 0; i < labels.size(); i++) {
        std::string directive = ".catchall ";
        for (size_t j = 0; j < labels.size(); j++) {
            if (j != 0) {
                directive += " , ";
            }
            directive += i == j ? "$Exception" : labels[j];
        }

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record $Exception {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString("$Exception:").first);
        v.push_back(l.TokenizeString("try_begin:").first);
        v.push_back(l.TokenizeString("try_end:").first);
        v.push_back(l.TokenizeString("catch_begin:").first);
        v.push_back(l.TokenizeString(directive).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();
        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, parse_catchall_directive5)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};
    std::vector<std::string> labelNames {"try block begin", "try block end", "catch block begin"};

    for (size_t i = 0; i < labels.size(); i++) {
        std::string directive = ".catchall ";
        for (size_t j = 0; j < labels.size(); j++) {
            if (j != 0) {
                directive += " , ";
            }
            directive += i == j ? "1Exception" : labels[j];
        }

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(directive).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();
        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL) << "Test " << directive;
        ASSERT_EQ(e.lineNumber, 2) << "Test " << directive;
        ASSERT_EQ(e.pos, directive.find('1')) << "Test " << directive;
        ASSERT_EQ(e.message, std::string("Invalid name of the ") + labelNames[i] + " label.") << "Test " << directive;
    }
}

TEST(parsertests, parse_catchall_directive6)
{
    std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};

    for (size_t i = 0; i < labels.size(); i++) {
        std::stringstream ss;
        ss << "Test " << labels[i] << " does not exists";

        std::vector<std::vector<ark::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string catchTable = ".catchall try_begin, try_end, catch_begin";

        v.push_back(l.TokenizeString(".function void main() {").first);
        for (size_t j = 0; j < labels.size(); j++) {
            if (i != j) {
                v.push_back(l.TokenizeString(labels[j] + ":").first);
            }
        }
        v.push_back(l.TokenizeString(catchTable).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL_EXT) << ss.str();
        ASSERT_EQ(e.pos, catchTable.find(labels[i])) << ss.str();
        ASSERT_EQ(e.message, "This label does not exist.") << ss.str();
    }
}

TEST(parsertests, parse_catchall_directive7)

{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = ".catchall try_begin, try_end, catch_begin";

    v.push_back(l.TokenizeString(".function void main() {").first);
    v.push_back(l.TokenizeString("try_begin:").first);
    v.push_back(l.TokenizeString("try_end:").first);
    v.push_back(l.TokenizeString("catch_begin:").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    const auto sigMain = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();
    auto &function = program.functionStaticTable.find(sigMain)->second;

    ASSERT_EQ(function.catchBlocks.size(), 1);
    ASSERT_EQ(function.catchBlocks[0].exceptionRecord, "");
    ASSERT_EQ(function.catchBlocks[0].tryBeginLabel, "try_begin");
    ASSERT_EQ(function.catchBlocks[0].tryEndLabel, "try_end");
    ASSERT_EQ(function.catchBlocks[0].catchBeginLabel, "catch_begin");
}

TEST(parsertests, parse_numbers0)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, 12345}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(12345)))
        << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers1)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, 0xFEFfe}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(0xFEFfe)))
        << "0xFEFfe expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers2)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, 01237}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(01237)))
        << "01237 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers3)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, 0b10101}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(0b10101)))
        << "0b10101 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers4)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, -12345}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(-12345)))
        << "-12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers5)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, -0xFEFfe}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(-0xFEFfe)))
        << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers6)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, -01237}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(-01237)))
        << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers7)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("movi v0, -0b10101}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(int64_t(-0b10101)))
        << "-0b10101 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers8)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.0}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.0)) << "1.0 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers9)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.)) << "1. expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers10)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, .1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(.1)) << ".0 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers11)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1e10)) << "1e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers12)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1e+10)) << "1e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers13)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1e-10)) << "1e-10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers14)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.0e10)) << "1.0e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers15)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.0e+10))
        << "1.0e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers16)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.0e-10)) << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers17)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.e10)) << "1.e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers18)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.e+10)) << "1.e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers19)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, 1.e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(1.e-10)) << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers20)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.0}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.0)) << "-1.0 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers21)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.)) << "-1. expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers22)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -.1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-.1)) << "-.0 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers23)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1e10)) << "-1e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers24)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1e+10)) << "-1e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers25)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1e-10)) << "-1e-10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers26)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.0e10))
        << "-1.0e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers27)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.0e+10))
        << "-1.0e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers28)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.0e-10))
        << "-1.0e-10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers29)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.e10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.e10)) << "-1.e10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers30)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.e+10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.e+10))
        << "-1.e+10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, parse_numbers31)
{
    const auto sigMain = GetFunctionSignatureFromName("main", {});
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("fmovi.64 v0, -1.e-10}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigMain).ins[0].GetImm(0U), Ins::IType(-1.e-10))
        << "-1.e-10 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, field_value1)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = "i32 f <value=A>";

    v.push_back(l.TokenizeString(".record A {").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_METADATA_INVALID_VALUE);
    ASSERT_EQ(e.lineNumber, 2);
    ASSERT_EQ(e.pos, s.find('A'));
    ASSERT_EQ(e.message, "Excepted integer literal");
}

TEST(parsertests, field_value2)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = "i32 f <value=10>";

    v.push_back(l.TokenizeString(".record A {").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE) << e.message;

    auto &program = res.Value();
    auto &record = program.recordTable.find("A")->second;
    auto &field = record.fieldList[0];

    ASSERT_EQ(field.metadata->GetFieldType().GetName(), "i32");
    ASSERT_TRUE(field.metadata->GetValue());
    ASSERT_EQ(field.metadata->GetValue()->GetType(), Value::Type::I32);
    ASSERT_EQ(field.metadata->GetValue()->GetValue<int32_t>(), 10);
}

TEST(parsertests, field_value3)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = "panda.String f <value=\"10\">";

    v.push_back(l.TokenizeString(".record A {").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE) << e.message;

    auto &program = res.Value();
    auto &record = program.recordTable.find("A")->second;
    auto &field = record.fieldList[0];

    ASSERT_EQ(field.metadata->GetFieldType().GetName(), "panda.String");
    ASSERT_TRUE(field.metadata->GetValue());
    ASSERT_EQ(field.metadata->GetValue()->GetType(), Value::Type::STRING);
    ASSERT_EQ(field.metadata->GetValue()->GetValue<std::string>(), "10");
}
TEST(parsertests, field_value4)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string s = "panda.String f";

    v.push_back(l.TokenizeString(".record A {").first);
    v.push_back(l.TokenizeString(s).first);
    v.push_back(l.TokenizeString("}").first);

    auto res = p.Parse(v);

    Error e = p.ShowError();

    ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE) << e.message;

    auto &program = res.Value();
    auto &record = program.recordTable.find("A")->second;
    auto &field = record.fieldList[0];

    ASSERT_EQ(field.metadata->GetFieldType().GetName(), "panda.String");
    ASSERT_FALSE(field.metadata->GetValue());
}

TEST(parsertests, call_short_0args)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                call.short
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }
}

TEST(parsertests, type_id_tests_lda)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                lda.type a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .function void f() {
                lda.type a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                lda.type a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, type_id_tests_newarr)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                newarr v0, v0, a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .function void f() {
                newarr v0, v0, a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                newarr v0, v0, a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                newarr v0, v0, a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();
        Error w = p.ShowWarnings()[0];

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
        ASSERT_EQ(w.err, Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
    }
}

TEST(parsertests, type_id_tests_newobj)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                newobj v0, a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .function void f() {
                newobj v0, a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                newobj v0, a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                newobj v0, a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();
        Error w = p.ShowWarnings()[0];

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
        ASSERT_EQ(w.err, Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
    }
}

TEST(parsertests, type_id_tests_checkcast)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                checkcast a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .function void f() {
                checkcast a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                checkcast a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, type_id_tests_isinstance)
{
    {
        Parser p;
        std::string source = R"(
            .function void f() {
                isinstance a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .function void f() {
                isinstance a[]
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_ID_RECORD);
    }

    {
        Parser p;
        std::string source = R"(
            .record a {}
            .function void f() {
                isinstance a
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, test_fields_same_name)
{
    {
        Parser p;
        std::string source = R"(
            .record A {
                i16 aaa
                u8  aaa
                i32 aaa
            }
        )";

        auto res = p.Parse(source);
        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_REPEATING_FIELD_NAME);
    }

    {
        Parser p;
        std::string source = R"(
            .function i32 main() {
                ldobj.64 v0, A.aaa
                ldai 0
                return
            }
            .record A {
                i16 aaa
            }
        )";

        auto res = p.Parse(source);
        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test_array_integer_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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

    auto counter = 0;
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::BOOL));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::BOOL);
    ASSERT_EQ(std::get<bool>(item.Value().literalarrayTable.at("array").literals[counter++].value), true);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value), 2);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value)),
        -30);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value), 400);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value)),
        -5000);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value), 60000);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::INTEGER));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[counter++].value)),
        -700000);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::BIGINT));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::BIGINT);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarrayTable.at("array").literals[counter++].value), 8000000);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::BIGINT));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::BIGINT);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarrayTable.at("array").literals[counter++].value)),
        -90000000);
}

TEST(parsertests, test_array_float_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("f32 -123.4").first);
    v.push_back(l.TokenizeString("f64 -1234.5").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);

    int counter = 0;
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::FLOAT));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::FLOAT);
    ASSERT_NEAR(std::get<float>(item.Value().literalarrayTable.at("array").literals[counter++].value), -123.4, 0.01F);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::DOUBLE));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::DOUBLE);
    ASSERT_NEAR(std::get<double>(item.Value().literalarrayTable.at("array").literals[counter++].value), -1234.5, 0.01F);
}

TEST(parsertests, test_array_string_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("panda.String \"a\"").first);
    v.push_back(l.TokenizeString("panda.String \"ab\"").first);
    v.push_back(l.TokenizeString("panda.String \"abc\"").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(R"(.array array_static panda.String 3 { "a" "ab" "abc" })").first);
    auto item = p.Parse(v);

    auto counter = 0;
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::STRING));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[counter++].value), "a");

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::STRING));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[counter++].value), "ab");

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[counter++].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::STRING));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[counter].tag, ark::panda_file::LiteralTag::STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[counter++].value), "abc");

    // string intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_static").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_static").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_STRING));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_static").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_static").literals[1].value), 3);

    // string array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_static").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array_static").literals[2].value), "a");
    ASSERT_EQ(item.Value().literalarrayTable.at("array_static").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array_static").literals[3].value), "ab");
    ASSERT_EQ(item.Value().literalarrayTable.at("array_static").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array_static").literals[4].value), "abc");
}

TEST(parsertests, test_array_static_bool_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array u1 3 { 1 0 1 }").first);
    auto item = p.Parse(v);

    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_U1));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[1].value), 3);
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarrayTable.at("array").literals[2].value), true);
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarrayTable.at("array").literals[3].value), false);
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarrayTable.at("array").literals[4].value), true);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test_array_static_integer_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
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
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_byte").literals[0].tag,
              ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_byte").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_U8));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_byte").literals[1].tag,
              ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_byte").literals[1].value), 3);

    // unsigned byte array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_byte").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_byte").literals[2].value), 1);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_byte").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_byte").literals[3].value), 2);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_byte").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_byte").literals[4].value), 3);

    // byte intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_byte").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_byte").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_I8));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_byte").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_byte").literals[1].value), 3);

    // byte array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_byte").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarrayTable.at("array_byte").literals[2].value)),
              -1);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_byte").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarrayTable.at("array_byte").literals[3].value)),
              2);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_byte").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarrayTable.at("array_byte").literals[4].value)),
              -3);

    // unsigned short intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_short").literals[0].tag,
              ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_short").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_U16));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_short").literals[1].tag,
              ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_short").literals[1].value), 3);

    // unsigned short array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_short").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarrayTable.at("array_unsigned_short").literals[2].value), 100);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_short").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarrayTable.at("array_unsigned_short").literals[3].value), 200);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_short").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarrayTable.at("array_unsigned_short").literals[4].value), 300);

    // short intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_short").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_short").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_I16));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_short").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_short").literals[1].value), 3);

    // short array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_short").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarrayTable.at("array_short").literals[2].value)),
        100);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_short").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarrayTable.at("array_short").literals[3].value)),
        -200);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_short").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarrayTable.at("array_short").literals[4].value)),
        300);

    // unsigned int intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_int").literals[0].tag,
              ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_int").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_U32));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_int").literals[1].tag,
              ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_int").literals[1].value), 3);

    // unsigned int array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_int").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_int").literals[2].value), 1000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_int").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_int").literals[3].value), 2000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_int").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_int").literals[4].value), 3000);

    // int intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_int").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_int").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_I32));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_int").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_int").literals[1].value), 3);

    // int array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_int").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array_int").literals[2].value)),
        -1000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_int").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array_int").literals[3].value)),
        2000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_int").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarrayTable.at("array_int").literals[4].value)),
        -3000);

    // unsigned long intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_long").literals[0].tag,
              ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_unsigned_long").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_U64));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_long").literals[1].tag,
              ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_unsigned_long").literals[1].value), 3);

    // unsigned long array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_long").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarrayTable.at("array_unsigned_long").literals[2].value), 10000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_long").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarrayTable.at("array_unsigned_long").literals[3].value), 20000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_unsigned_long").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarrayTable.at("array_unsigned_long").literals[4].value), 30000);

    // long intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_long").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_long").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_I64));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_long").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_long").literals[1].value), 3);

    // long array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_long").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarrayTable.at("array_long").literals[2].value)),
        10000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_long").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarrayTable.at("array_long").literals[3].value)),
        -20000);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_long").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarrayTable.at("array_long").literals[4].value)),
        30000);
}

TEST(parsertests, test_array_static_float_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array_float f32 3 { 12.3 -12.34 12.345 }").first);
    v.push_back(l.TokenizeString(".array array_double f64 3 { -120.3 120.34 -120.345 }").first);
    auto item = p.Parse(v);

    // float intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_float").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_float").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_F32));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_float").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_float").literals[1].value), 3);

    // float array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_float").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarrayTable.at("array_float").literals[2].value), 12.3, 0.01F);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_float").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarrayTable.at("array_float").literals[3].value), -12.34, 0.001F);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_float").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarrayTable.at("array_float").literals[4].value), 12.345, 0.0001F);

    // double intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array_double").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array_double").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_F64));
    ASSERT_EQ(item.Value().literalarrayTable.at("array_double").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array_double").literals[1].value), 3);

    // double array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array_double").literals[2].tag,
              ark::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarrayTable.at("array_double").literals[2].value), -120.3, 0.01F);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_double").literals[3].tag,
              ark::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarrayTable.at("array_double").literals[3].value), 120.34, 0.001F);
    ASSERT_EQ(item.Value().literalarrayTable.at("array_double").literals[4].tag,
              ark::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarrayTable.at("array_double").literals[4].value), -120.345,
                0.0001F);
}

TEST(parsertests, test_array_static_string_def)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(R"(.array array panda.String 3 { "a" "ab" "abc" })").first);
    auto item = p.Parse(v);

    // string intro literals
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[0].tag, ark::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarrayTable.at("array").literals[0].value),
              static_cast<uint8_t>(ark::panda_file::LiteralTag::ARRAY_STRING));
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[1].tag, ark::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarrayTable.at("array").literals[1].value), 3);

    // string array elements
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[2].tag, ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[2].value), "a");
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[3].tag, ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[3].value), "ab");
    ASSERT_EQ(item.Value().literalarrayTable.at("array").literals[4].tag, ark::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarrayTable.at("array").literals[4].value), "abc");
}

TEST(parsertests, test_array_string_use)
{
    std::vector<std::vector<ark::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".record Asm1 {").first);
    v.push_back(l.TokenizeString("void f").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("panda.String \"a\"").first);
    v.push_back(l.TokenizeString("panda.String \"ab\"").first);
    v.push_back(l.TokenizeString("panda.String \"abc\"").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(R"(.array array_static panda.String 3 { "a" "ab" "abc" })").first);

    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("lda.const v0, array").first);
    v.push_back(l.TokenizeString("lda.const v1, array_static").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sigF = GetFunctionSignatureFromName("f", {});

    ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].opcode, Opcode::LDA_CONST);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[0].GetID(0), "array");

    ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].opcode, Opcode::LDA_CONST);
    ASSERT_EQ(item.Value().functionStaticTable.at(sigF).ins[1].GetID(0), "array_static");
}

TEST(parsertests, test_function_overloading_1)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";

    auto &program = res.Value();

    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i8", 0}, language);
    const auto sigF = GetFunctionSignatureFromName("f", {});
    const auto sigFi8 = GetFunctionSignatureFromName("f", params);

    ASSERT_TRUE(program.functionStaticTable.find(sigF) != program.functionStaticTable.end());
    ASSERT_TRUE(program.functionStaticTable.find(sigFi8) != program.functionStaticTable.end());
}

TEST(parsertests, test_function_overloading_2)
{
    Parser p;
    std::string source = R"(
            .function u1 f(i8 a0) {}
            .function i8 f(i8 a0) {}
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, test_function_overloading_3)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES);
}

TEST(parsertests, test_function_overloading_4)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:(), a0
                call f:(i8), a0
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, test_function_overloading_5)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}

            .function void main(u1 a0) {
                call f:()
                call f
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";

    std::vector<Function::Parameter> params;
    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    const auto sigF = GetFunctionSignatureFromName("f", {});
    const auto sigMain = GetFunctionSignatureFromName("main", params);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[1].GetID(0), sigF);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[0].GetID(0), sigF);
}

TEST(parsertests, test_function_overloading_6)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:(u1)
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, test_function_overloading_7)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:(u1,)
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_SIGNATURE_PARAMETERS);
}

TEST(parsertests, test_function_overloading_8)
{
    Parser p;
    std::string source = R"(
            .function u1 f(u1 a0) {}
            .function u1 f(i8 a0, u8 a1) {}

            .function void main(u1 a0) {
                call f:(i8, u8), a0
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_FUNCTION_ARGUMENT_MISMATCH);
}

TEST(parsertests, test_function_overloading_9)
{
    Parser p;
    std::string source = R"(
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:(i8), a0
                call f, v1
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";

    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::vector<Function::Parameter> paramsMain;
    paramsMain.emplace_back(Type {"u1", 0}, language);
    std::vector<Function::Parameter> paramsF;
    paramsF.emplace_back(Type {"i8", 0}, language);
    const auto sigF = GetFunctionSignatureFromName("f", paramsF);
    const auto sigMain = GetFunctionSignatureFromName("main", paramsMain);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[1].GetID(0), sigF);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[1].GetReg(0), 1);  //  v0, [v1], a0
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[0].GetID(0), sigF);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 2);  //  v0,  v1, [a0]
}

TEST(parsertests, test_function_overloading_10)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:(i8, u1
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_SIGNATURE);
}

TEST(parsertests, test_function_overloading_11)
{
    Parser p;
    std::string source = R"(
            .function u1 f() {}
            .function u1 f(i8 a0) {}

            .function void main(u1 a0) {
                call f:
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_SIGNATURE);
}

TEST(parsertests, test_function_overloading_12)
{
    Parser p;
    std::string source = R"(
            .function u1 f(i8 a0) {}
            .function u1 f() {}
            .function u1 f(u8 a0) {}
            .function u1 f(i8 a0, u8 a1) {}

            .function void main(u1 a0) {
                call f:(i8), a0
                call f, a0
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES);
}

TEST(parsertests, test_function_overloading_13)
{
    Parser p;
    std::string source = R"(
            .function u1 f(i8 a0) {}
            .function u1 f() {}
            .function u1 f(u8 a0) {}
            .function u1 f(i8 a0, u8 a1) {}

            .function void main(u1 a0) {
                call f:(i8), a0
                call f:(), v1
                call f:(u8), v2
                call f:(i8, u8), v3, v4
            }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();

    ark::panda_file::SourceLang language {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::vector<Function::Parameter> paramsMain;
    paramsMain.emplace_back(Type {"u1", 0}, language);
    std::vector<Function::Parameter> paramsFi8;
    paramsFi8.emplace_back(Type {"i8", 0}, language);
    std::vector<Function::Parameter> paramsFu8;
    paramsFu8.emplace_back(Type {"u8", 0}, language);
    std::vector<Function::Parameter> paramsFi8u8;
    paramsFi8u8.emplace_back(Type {"i8", 0}, language);
    paramsFi8u8.emplace_back(Type {"u8", 0}, language);
    const auto sigF = GetFunctionSignatureFromName("f", {});
    const auto sigFi8 = GetFunctionSignatureFromName("f", paramsFi8);
    const auto sigFu8 = GetFunctionSignatureFromName("f", paramsFu8);
    const auto sigFi8u8 = GetFunctionSignatureFromName("f", paramsFi8u8);
    const auto sigMain = GetFunctionSignatureFromName("main", paramsMain);

    ASSERT_TRUE(program.functionStaticTable.find(sigMain) != program.functionStaticTable.end());
    ASSERT_TRUE(program.functionStaticTable.find(sigF) != program.functionStaticTable.end());
    ASSERT_TRUE(program.functionStaticTable.find(sigFi8) != program.functionStaticTable.end());
    ASSERT_TRUE(program.functionStaticTable.find(sigFu8) != program.functionStaticTable.end());
    ASSERT_TRUE(program.functionStaticTable.find(sigFi8u8) != program.functionStaticTable.end());

    // f:(i8)
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[0].GetID(0), sigFi8);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[0].GetReg(0), 5);

    // f:()
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[1].GetID(0), sigF);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[1].GetReg(0), 1);

    // f:(u8)
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[2].GetID(0), sigFu8);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[2].GetReg(0), 2);

    // f:(i8u8)
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[3].GetID(0), sigFi8u8);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[3].GetReg(0), 3);
    ASSERT_EQ(res.Value().functionStaticTable.at(sigMain).ins[3].GetReg(1), 4);
}

TEST(parsertests, test_function_doesnt_exist)
{
    Parser p;
    std::string source = R"(
        .function void gg(u1 a0) {}
        .function void f() {
            call g, v0
        }
        )";

    auto res = p.Parse(source);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, test_external_fields)
{
    {
        Parser p;
        std::string source = R"(
            .record A {
                i32 fld <external>
            }
            )";

        auto res = p.Parse(source);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A <external> {
                i32 fld <external>
                i32 fld2 <external, static>
            }
            )";

        auto res = p.Parse(source);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A <external> {
                i32 fld
            }
            )";

        auto res = p.Parse(source);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_FIELD_METADATA);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(parsertests, test_accessors)
{
    {
        Parser p;
        std::string source = R"(
            .record A {
                i32 fld1 <access.field=public>
                i32 fld2 <access.field=protected>
                i32 fld3 <access.field=private>
            }
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .function void f1() <access.function=public> { }
            .function void f2() <access.function=protected> { }
            .function void f3() <access.function=private> { }
            .function void f4() <external, access.function=private>
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A <access.record=public> { }
            .record B <access.record=protected> { }
            .record C <access.record=private> { }
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A {
                i32 fld1 <access.field>
            }
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_BAD_METADATA_MISSING_VALUE);
        ASSERT_EQ(err.lineNumber, 3);
        ASSERT_EQ(err.message, "Attribute 'access.field' must have a value");
    }
    {
        Parser p;
        std::string source = R"(
            .function void f1() <access.function=internal> { }
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_BAD_METADATA_INVALID_VALUE);
        ASSERT_EQ(err.lineNumber, 2);
        ASSERT_EQ(err.message,
                  "Attribute 'access.function' have incorrect value 'internal'. Should be one of [\"public\", "
                  "\"protected\", \"private\"]");
    }
}

TEST(parsertests, test_extends)
{
    {
        Parser p;
        std::string source = R"(
            .record A {}
            .record B <extends=A> {}
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A {}
            .record B <extends> {}
            )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_BAD_METADATA_MISSING_VALUE);
        ASSERT_EQ(err.lineNumber, 3);
        ASSERT_EQ(err.message, "Attribute 'extends' must have a value");
    }
}

TEST(parsertests, test_final)
{
    {
        Parser p;
        std::string source = R"(
            .record A {
                i32 fld <final>
            }
        )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .function void f1() <final> { }
        )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
    {
        Parser p;
        std::string source = R"(
            .record A <final> { }
        )";

        auto res = p.Parse(source);

        auto err = p.ShowError();
        ASSERT_EQ(err.err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, test_union_canonicalization)
{
    Parser p;
    std::string source =
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UC,C,B,C,B} <external>\n"
        ".record {UD,C,B,A} <external>\n"
        ".record {UA,A,A,B} <external>\n"
        ".record {UA,std.core.Double} <external>\n"
        ".record {Ustd.core.Double,std.core.Double,std.core.Long} <external>\n"
        ".record {Ustd.core.Long,std.core.Double,std.core.Int} <external>\n"
        ".record {UA,B,C,B,A} <external>\n"
        ".record {UA,D,D,D} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();

    std::map<std::string, std::string> records {
        {"{UC,C,B,C,B}", "{UB,C}"},
        {"{UD,C,B,A}", "{UA,B,C,D}"},
        {"{UA,A,A,B}", "{UA,B}"},
        {"", "{UA,std.core.Double}"},
        {"{Ustd.core.Double,std.core.Double,std.core.Long}", "{Ustd.core.Double,std.core.Long}"},
        {"{Ustd.core.Long,std.core.Double,std.core.Int}", "{Ustd.core.Double,std.core.Int,std.core.Long}"},
        {"{UA,B,C,B,A}", "{UA,B,C}"},
        {"{UA,D,D,D}", "{UA,D}"}};

    for (const auto &[notCanonRec, canonRec] : records) {
        ASSERT_EQ(program.recordTable.find(notCanonRec), program.recordTable.end());
        ASSERT_NE(program.recordTable.find(canonRec), program.recordTable.end());
    }
}

TEST(parsertests, test_union_canonicalization_arrays)
{
    Parser p;

    std::string source =
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UB,C,{UA,D,std.core.Double}[],A} <external>\n"
        ".record {U{UD,A,std.core.Int}[],{UA,D,std.core.Double}[],{Ustd.core.Long,D,A}[]} <external>\n"
        ".record {UB,C,{UA,B}[],A} <external>\n"
        ".record {UB,{UA,D}[],{UD,B}[],{UC,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[][],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[][],{UD,B}[]} <external>\n"
        ".record {U{UD,C}[],{UC,B}[]} <external>\n"
        ".record {U{UD,C}[],C,B} <external>\n"
        ".record {U{Ustd.core.Int,std.core.Double}[],std.core.Long,std.core.Double} <external>\n"
        ".record {U{Ustd.core.Int[],std.core.Double[]}[],std.core.Long[],std.core.Double[]} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto &program = res.Value();

    std::map<std::string, std::string> records {
        {"{UB,C,{UA,D,std.core.Double}[],A}", "{UA,B,C,{UA,D,std.core.Double}[]}"},
        {"{U{UD,A,std.core.Int}[],{UA,D,std.core.Double}[],{Ustd.core.Long,D,A}[]}",
         "{U{UA,D,std.core.Double}[],{UA,D,std.core.Int}[],{UA,D,std.core.Long}[]}"},
        {"{UB,C,{UA,B}[],A}", "{UA,B,C,{UA,B}[]}"},
        {"{UB,{UA,D}[],{UD,B}[],{UC,B}[],{UC,D}[]}", "{UB,{UA,D}[],{UB,C}[],{UB,D}[],{UC,D}[]}"},
        {"{U{UA,D}[],{UD,B}[],{UD,B}[],{UC,D}[]}", "{U{UA,D}[],{UB,D}[],{UC,D}[]}"},
        {"{U{UA,D}[],{UD,B}[][],{UD,B}[],{UC,D}[]}", "{U{UA,D}[],{UB,D}[],{UB,D}[][],{UC,D}[]}"},
        {"{U{UA,D}[][],{UD,B}[]}", "{U{UA,D}[][],{UB,D}[]}"},
        {"{U{UD,C}[],{UC,B}[]}", "{U{UB,C}[],{UC,D}[]}"},
        {"{U{UD,C}[],C,B}", "{UB,C,{UC,D}[]}"},
        {"{U{Ustd.core.Int,std.core.Double}[],std.core.Long,std.core.Double}",
         "{Ustd.core.Double,std.core.Long,{Ustd.core.Double,std.core.Int}[]}"},
        {"{U{Ustd.core.Int[],std.core.Double[]}[],std.core.Long[],std.core.Double[]}",
         "{Ustd.core.Double[],std.core.Long[],{Ustd.core.Double[],std.core.Int[]}[]}"}};

    for (const auto &[notCanonRec, canonRec] : records) {
        ASSERT_EQ(program.recordTable.find(notCanonRec), program.recordTable.end());
        ASSERT_NE(program.recordTable.find(canonRec), program.recordTable.end());
    }
}

}  // namespace ark::test
