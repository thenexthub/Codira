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

using namespace panda::pandasm;

TEST(parsertests, test1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("mov v1, v2}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[0], 1) << "1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[1], 2) << "2 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test2)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("label:}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].label, "label") << "label expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].set_label, true) << "true expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::INVALID) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test3)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jlt v10, lab123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT) << "ERR_BAD_LABEL_EXT expected";
}

TEST(parsertests, test4)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("11111111}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION_NAME expected";
}

TEST(parsertests, test5)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 1}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::ADDI) << "IMM expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(1))) << "1 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test6)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 12345}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::ADDI) << "IMM expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(12345))) << "12345 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test7)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("addi 11.3}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME) << "ERR_NONE expected";
}

TEST(parsertests, test8)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashdjbf iashudbfiun as}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

TEST(parsertests, test9)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("lda v1").first);
    v.push_back(l.TokenizeString("movi v10, 1001}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::LDA) << "V expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[0], 1) << "1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].opcode, Opcode::MOVI) << "V_IMM expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].regs[0], 10) << "10 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].imms[0], Ins::IType(int64_t(1001))) << "1001 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test10)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 main(){").first);
    v.push_back(l.TokenizeString("call.short nain, v1, v2}").first);
    v.push_back(l.TokenizeString(".function u1 nain(){}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});
    const auto sig_nain = GetFunctionSignatureFromName("nain", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::CALL_SHORT) << "V_V_ID expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].ids[0], sig_nain) << "nain expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[0], 1) << "1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[1], 2) << "2 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test11)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("i64tof64}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::I64TOF64) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test12)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp l123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_LABEL_EXT) << "ERR_BAD_LABEL_EXT expected";
}

TEST(parsertests, test13)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("l123: jmp l123}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::JMP) << "ID expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].ids[0], "l123") << "l123 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE";
}

TEST(parsertests, test14)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("jmp 123}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_ID) << "ERR_BAD_NAME_ID expected";
}

TEST(parsertests, test15)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("shli 12 asd}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS) << "ERR_BAD_NUMBER_OPERANDS expected";
}

TEST(parsertests, test17)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ldarr.8 v120}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::LDARR_8) << "V expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[0], 120) << "120 expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test18)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::RETURN) << "NONE expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test19)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_OPERATION_NAME) << "ERR_BAD_OPERATION expected";
}

TEST(parsertests, test20)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("return 1}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS) << "ERR_BAD_NUMBER_OPERANDS expected";
}

TEST(parsertests, test21)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashr2.64 1234}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG) << "ERR_BAD_NAME_REG expected";
}

TEST(parsertests, test22)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u8 main(){").first);
    v.push_back(l.TokenizeString("ashr2.64 v12}").first);
    auto item = p.Parse(v);

    const auto sig_main = GetFunctionSignatureFromName("main", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::ASHR2_64) << "V expected";
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].regs[0], 12) << "12 expected";
}

TEST(parsertests, test23)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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

    const auto sig_main = GetFunctionSignatureFromName("main", {});
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"f32", 0}, language);
    const auto sig_m123 = GetFunctionSignatureFromName("m123", params);

    ASSERT_EQ(item.Value().function_table.at(sig_main).name, sig_main);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).name, sig_m123);
    ASSERT_EQ(item.Value().function_table.at(sig_main).GetParamsNum(), 0U);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).GetParamsNum(), 2U);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).params[0].type.GetId(), panda::panda_file::Type::TypeId::U1);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).params[1].type.GetId(), panda::panda_file::Type::TypeId::F32);
    ASSERT_EQ(item.Value().function_table.at(sig_main).return_type.GetId(), panda::panda_file::Type::TypeId::U8);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).return_type.GetId(), panda::panda_file::Type::TypeId::F64);
    ASSERT_EQ(item.Value().function_table.at(sig_main).label_table.at("label1").file_location->line_number, 2U);
    ASSERT_EQ(item.Value().function_table.at(sig_main).label_table.at("label1").file_location->is_defined, true);
    ASSERT_EQ(item.Value().function_table.at(sig_main).label_table.at("label2").file_location->line_number, 3U);
    ASSERT_EQ(item.Value().function_table.at(sig_main).label_table.at("label2").file_location->is_defined, true);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).label_table.at("la1").file_location->line_number, 11U);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).label_table.at("la1").file_location->is_defined, true);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].opcode, Opcode::INVALID);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].label, "label1");
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].opcode, Opcode::JLE);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].regs[0], 0);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].ids[0], "label2");
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[2].opcode, Opcode::MOVI);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[2].regs[0], 15);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[2].imms[0], Ins::IType(int64_t(26)));
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[2].set_label, false);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].regs[0], 0);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].regs[1], 1);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].label, "label2");
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].set_label, true);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].opcode, Opcode::CALL);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].regs[0], 2);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].regs[1], 6);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].regs[2], 3);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].regs[3], 4);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].ids[0], sig_m123);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[0].opcode, Opcode::LDA);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[0].regs[0], 10);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[1].opcode, Opcode::STA);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[1].regs[0], 11);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[2].opcode, Opcode::INVALID);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[2].label, "la1");
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[2].set_label, true);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[3].opcode, Opcode::JLE);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[3].regs[0], 12);
    ASSERT_EQ(item.Value().function_table.at(sig_m123).ins[3].ids[0], "la1");
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

TEST(parsertests, test24_functions)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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

    const auto sig_main = GetFunctionSignatureFromName("main", {});
    const auto sig_niam = GetFunctionSignatureFromName("niam", {});

    ASSERT_EQ(item.Value().function_table.at(sig_main).return_type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(256))) << "256 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[1].imms[0], Ins::IType(int64_t(4294967295)))
        << "4294967295 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[2].imms[0], Ins::IType(int64_t(15))) << "15 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[3].imms[0], Ins::IType(1000.0)) << "1000.0 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[4].imms[0], Ins::IType(int64_t(59796))) << "59796 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[5].imms[0], Ins::IType(1.1)) << "1.1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[7].imms[0], Ins::IType(.1)) << ".1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[8].imms[0], Ins::IType(int64_t(0))) << "0 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[9].imms[0], Ins::IType(0.1)) << "0.1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[10].imms[0], Ins::IType(00.1)) << "00.1 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_main).ins[11].imms[0], Ins::IType(00.)) << "00. expected";
    ASSERT_EQ(item.Value().function_table.at(sig_niam).ins[0].imms[0], Ins::IType(int64_t(-1))) << "-1 expected";
}

TEST(parsertests, test25_record_alone)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".record Asm {").first);
    v.push_back(l.TokenizeString("i64 asm1").first);
    v.push_back(l.TokenizeString("void asm2").first);
    v.push_back(l.TokenizeString("i32 asm3").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);
    ASSERT_EQ(item.Value().record_table.at("Asm").name, "Asm");
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
}

TEST(parsertests, test26_records)
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

    ASSERT_EQ(item.Value().record_table.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().record_table.at("Asm2").name, "Asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().record_table.at("Asm3").name, "Asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().record_table.at("Asm4").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm4").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().record_table.at("Asm5").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm5").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I32);
}

TEST(parsertests, test27_record_and_function)
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

    ASSERT_EQ(item.Value().record_table.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);
    ASSERT_EQ(item.Value().function_table.at(sig_niam).ins[0].imms[0], Ins::IType(int64_t(-1))) << "-1 expected";
}

TEST(parsertests, test28_records_and_functions)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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

    const auto sig_niam1 = GetFunctionSignatureFromName("niam1", {});
    const auto sig_niam2 = GetFunctionSignatureFromName("niam2", {});
    const auto sig_niam3 = GetFunctionSignatureFromName("niam3", {});

    ASSERT_EQ(item.Value().record_table.at("Asm1").name, "Asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().function_table.at(sig_niam1).ins[0].imms[0], Ins::IType(int64_t(-1))) << "-1 expected";

    ASSERT_EQ(item.Value().record_table.at("Asm2").name, "Asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().function_table.at(sig_niam2).ins[0].imms[0], Ins::IType(int64_t(-1))) << "-1 expected";

    ASSERT_EQ(item.Value().record_table.at("Asm3").name, "Asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].name, "asm1");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[1].name, "asm2");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[2].name, "asm3");
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().function_table.at(sig_niam3).ins[0].imms[0], Ins::IType(int64_t(-1))) << "-1 expected";
}

TEST(parsertests, test29_instructions_def_lines)
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

    ASSERT_EQ(item.Value().function_table.at(sig_niam1).ins[0].ins_debug.line_number, 2U) << "2 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_niam2).ins[0].ins_debug.line_number, 5U) << "5 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_niam3).ins[0].ins_debug.line_number, 9U) << "9 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_niam4).ins[0].ins_debug.line_number, 11U) << "11 expected";
    ASSERT_EQ(item.Value().function_table.at(sig_niam5).ins[0].ins_debug.line_number, 12U) << "12 expected";
}

TEST(parsertests, test30_fields_def_lines)
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

    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].line_of_def, 2U);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].line_of_def, 3U);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].line_of_def, 4U);

    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].line_of_def, 7U);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[1].line_of_def, 8U);
    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[2].line_of_def, 9U);

    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].line_of_def, 12U);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[1].line_of_def, 13U);
    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[2].line_of_def, 14U);

    ASSERT_EQ(item.Value().record_table.at("Asm4").field_list[0].line_of_def, 16U);

    ASSERT_EQ(item.Value().record_table.at("Asm5").field_list[0].line_of_def, 17U);
}

TEST(parsertests, test31_own_types)
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

    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetName(), "Asm");
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    ASSERT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    ASSERT_EQ(item.Value().record_table.at("Asm2").field_list[0].type.GetName(), "Asm1");

    ASSERT_EQ(item.Value().record_table.at("Asm3").field_list[0].type.GetName(), "Asm2");
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
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"i64", 0}, language);
    params.emplace_back(Type {"i32", 0}, language);
    const auto sig_niam2 = GetFunctionSignatureFromName("niam2", params);

    ASSERT_EQ(item.Value().function_table.at(sig_niam1).GetParamsNum(), 0U);
    ASSERT_EQ(item.Value().function_table.at(sig_niam1).value_of_first_param + 1, 0);

    ASSERT_EQ(item.Value().function_table.at(sig_niam2).GetParamsNum(), 3U);
    ASSERT_EQ(item.Value().function_table.at(sig_niam2).value_of_first_param + 1, 4);
}

TEST(parsertests, test34_vregs_number)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    v.push_back(l.TokenizeString(".function u8 niam1(){").first);
    v.push_back(l.TokenizeString("ldai -1").first);
    v.push_back(l.TokenizeString("}").first);

    v.push_back(l.TokenizeString(".function u8 niam2(u1 a0, i64 a1, i32 a2){").first);
    v.push_back(l.TokenizeString("mov v0, v5").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sig_niam1 = GetFunctionSignatureFromName("niam1", {});
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    params.emplace_back(Type {"i64", 0}, language);
    params.emplace_back(Type {"i32", 0}, language);
    const auto sig_niam2 = GetFunctionSignatureFromName("niam2", params);

    ASSERT_EQ(item.Value().function_table.at(sig_niam1).regs_num, 0U);

    ASSERT_EQ(item.Value().function_table.at(sig_niam2).regs_num, 6U);
}

TEST(parsertests, test35_functions_bracket)
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

    ASSERT_EQ(item.Value().function_table.at(sig_nain1).name, sig_nain1);
    ASSERT_EQ(item.Value().function_table.at(sig_nain12).name, sig_nain12);
    ASSERT_EQ(item.Value().function_table.at(sig_nain3).name, sig_nain3);
    ASSERT_EQ(item.Value().function_table.at(sig_nain2).name, sig_nain2);
    ASSERT_EQ(item.Value().function_table.at(sig_nain4).name, sig_nain4);
    ASSERT_EQ(item.Value().function_table.at(sig_nain5).name, sig_nain5);
    ASSERT_EQ(item.Value().function_table.at(sig_nain6).name, sig_nain6);
    ASSERT_EQ(item.Value().function_table.at(sig_nain7).name, sig_nain7);
    ASSERT_EQ(item.Value().function_table.at(sig_nain8).name, sig_nain8);
    ASSERT_EQ(item.Value().function_table.at(sig_nain9).name, sig_nain9);
    ASSERT_EQ(item.Value().function_table.at(sig_nain10).name, sig_nain10);
    ASSERT_EQ(item.Value().function_table.at(sig_nain11).name, sig_nain11);

    ASSERT_EQ(item.Value().function_table.at(sig_nain1).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain2).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain3).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain4).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain5).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain6).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain7).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain8).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain9).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain10).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain11).ins[0].opcode, Opcode::MOV);
    ASSERT_EQ(item.Value().function_table.at(sig_nain12).ins[0].opcode, Opcode::MOV);
}

TEST(parsertests, test36_records_bracket)
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

    ASSERT_EQ(item.Value().record_table.at("rec1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec2").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec3").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec4").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec5").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec6").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    ASSERT_EQ(item.Value().record_table.at("rec7").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
}

TEST(parsertests, test37_operand_type_print)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i64", 0}, language);
    const auto sig_nain1 = GetFunctionSignatureFromName("nain1", params);

    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[0].opcode), "reg_reg");
    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[1].opcode), "reg_imm");
    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[2].opcode), "label");
    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[3].opcode), "reg");
    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[4].opcode), "none");
    ASSERT_EQ(OperandTypePrint(item.Value().function_table.at(sig_nain1).ins[5].opcode), "call_reg_reg");
}

TEST(parsertests, test38_record_invalid_field)
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_FIELD_MISSING_NAME);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, f.length());
        ASSERT_EQ(e.message, "Expected field name.");
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_METADATA_BOUND);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, f.length());
        ASSERT_EQ(e.message, "Expected '>'.");
    }
}

TEST(parsertests, test39_parse_operand_string)
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find(' ') + 1);
        ASSERT_EQ(e.message, "Expected string literal");
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_OPERAND);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find(' ') + 1);
        ASSERT_EQ(e.message, "Expected string literal");
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

        std::unordered_set<std::string> strings = {" abc123 ", "zxcvb"};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

TEST(parsertests, test40_parse_operand_string_escape_seq)
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_UNKNOWN_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Unknown escape sequence");
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

        std::unordered_set<std::string> strings = {" \" ' \\ \a \b \f \n \r \t \v "};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

TEST(parsertests, test41_parse_operand_string_hex_escape_seq)
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

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\xZZ\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\xAZ\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string op = "lda.str \"123\\xZA\"";

        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString(op).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE);
        ASSERT_EQ(e.line_number, 2U);
        ASSERT_EQ(e.pos, op.find('\\'));
        ASSERT_EQ(e.message, "Invalid \\x escape sequence");
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

        std::unordered_set<std::string> strings = {"123\xaa\x65"};

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        ASSERT_EQ(item.Value().strings, strings);
    }
}

TEST(parsertests, test42_parse_operand_string_octal_escape_seq)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;

    std::string op = "lda.str \"123\\1\\02\\00123\"";

    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString(op).first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    std::unordered_set<std::string> strings = {"123\1\02\00123"};

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_TRUE(item.HasValue());
    ASSERT_EQ(item.Value().strings, strings);
}

TEST(parsertests, test43_call_short)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f").first);
        v.push_back(l.TokenizeString("call.virt.short f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0").first);
        v.push_back(l.TokenizeString("call.virt.short f, v0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.short f, v0, v1").first);
        v.push_back(l.TokenizeString("call.virt.short f, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0, 1};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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

TEST(parsertests, test44_call)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f").first);
        v.push_back(l.TokenizeString("call.virt f").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0").first);
        v.push_back(l.TokenizeString("call.virt f, v0").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1").first);
        v.push_back(l.TokenizeString("call.virt f, v0, v1").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0, 1};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("call.virt f, v0, v1, v2").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0, 1, 2};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call f, v0, v1, v2, v3").first);
        v.push_back(l.TokenizeString("call.virt f, v0, v1, v2, v3").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        const auto sig_f = GetFunctionSignatureFromName("f", {});

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
        ASSERT_TRUE(item.HasValue());
        std::vector<uint16_t> regs {0, 1, 2, 3};
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].regs, regs);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].regs, regs);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("call.virt.short f, v0, v1, v2, v3, v4").first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
    }
}

TEST(parsertests, function_argument_mismatch)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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

TEST(parsertests, Naming_function_function)
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
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ID_FUNCTION);
}

TEST(parsertests, Naming_label_label)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".function u1 mov(i64 a0) <> {").first);
    v.push_back(l.TokenizeString("call.short mov, v0, v1").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
}

TEST(parsertests, register_naming_incorr)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("sta a01").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta 123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta q0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta vy1").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_NAME_REG);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v123").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f() {").first);
        v.push_back(l.TokenizeString("sta v0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("sta a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("mov v0, a0").first);
        v.push_back(l.TokenizeString("}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    }
}

TEST(parsertests, array_type)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"i8", 1}, language);
        const auto sig_f = GetFunctionSignatureFromName("f", params);

        ASSERT_TRUE(item.HasValue());

        ASSERT_EQ(item.Value().record_table.at("R").field_list.size(), 1U);
        ASSERT_TRUE(item.Value().record_table.at("R").field_list[0].type.IsArray());
        ASSERT_TRUE(item.Value().record_table.at("R").field_list[0].type.IsObject());
        ASSERT_EQ(item.Value().record_table.at("R").field_list[0].type.GetName(), "R[][]");
        ASSERT_EQ(item.Value().record_table.at("R").field_list[0].type.GetComponentName(), "R");
        ASSERT_EQ(item.Value().record_table.at("R").field_list[0].type.GetDescriptor(), "[[LR;");

        ASSERT_TRUE(item.Value().function_table.at(sig_f).return_type.IsArray());
        ASSERT_TRUE(item.Value().function_table.at(sig_f).return_type.IsObject());
        ASSERT_EQ(item.Value().function_table.at(sig_f).return_type.GetName(), "R[]");
        ASSERT_EQ(item.Value().function_table.at(sig_f).return_type.GetComponentName(), "R");
        ASSERT_EQ(item.Value().function_table.at(sig_f).return_type.GetDescriptor(), "[LR;");

        ASSERT_EQ(item.Value().function_table.at(sig_f).params.size(), 1U);
        ASSERT_TRUE(item.Value().function_table.at(sig_f).params[0].type.IsArray());
        ASSERT_TRUE(item.Value().function_table.at(sig_f).params[0].type.IsObject());
        ASSERT_EQ(item.Value().function_table.at(sig_f).params[0].type.GetName(), "i8[]");
        ASSERT_EQ(item.Value().function_table.at(sig_f).params[0].type.GetComponentName(), "i8");
        ASSERT_EQ(item.Value().function_table.at(sig_f).params[0].type.GetDescriptor(), "[B");

        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].ids.size(), 1U);
        ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].ids[0], "i32[][]");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[][").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function f64[ f(i32 a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function void f(i32[][][ a0) {").first);
        v.push_back(l.TokenizeString("newarr v0, v0, i32[]").first);
        v.push_back(l.TokenizeString("}").first);

        auto item = p.Parse(v);

        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        ASSERT_EQ(e.line_number, 3);
        ASSERT_EQ(e.pos, 27);
        ASSERT_EQ(e.message, "This record does not exist.");
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
        ASSERT_EQ(e.line_number, 3);
        ASSERT_EQ(e.pos, 26);
        ASSERT_EQ(e.message, "This record does not exist.");
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
        ASSERT_EQ(e.line_number, 3);
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
        ASSERT_EQ(e.line_number, 5);
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
        ASSERT_EQ(e.line_number, 3);
        ASSERT_EQ(e.pos, 44);
        ASSERT_EQ(e.message, "This record does not exist.");
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"u1", 0}, language);
        const auto sig_main = GetFunctionSignatureFromName("main", params);

        auto it_func = program.function_table.find(sig_main);

        ASSERT_TRUE(it_func != program.function_table.end());
        ASSERT_EQ(it_func->second.regs_num, 2);
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
        panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
        params.emplace_back(Type {"u1", 0}, language);
        const auto sig_main = GetFunctionSignatureFromName("main", params);

        auto &program = res.Value();
        auto it_func = program.function_table.find(sig_main);

        ASSERT_TRUE(it_func != program.function_table.end());
        ASSERT_EQ(it_func->second.regs_num, 2);
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

        const auto sig_main = GetFunctionSignatureFromName("main", {});

        auto &program = res.Value();
        auto it_func = program.function_table.find(sig_main);

        ASSERT_TRUE(it_func != program.function_table.end());
        ASSERT_EQ(it_func->second.regs_num, 1);
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

        const auto sig_main = GetFunctionSignatureFromName("main", {});

        auto &program = res.Value();
        auto it_func = program.function_table.find(sig_main);

        ASSERT_TRUE(it_func != program.function_table.end());
        ASSERT_EQ(it_func->second.regs_num, 5);
    }
}

TEST(parsertests, Bad_imm_value)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,.").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,%").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u n(){movi v0,;").first);
        auto item = p.Parse(v);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_BAD_INTEGER_NAME);
    }
}

TEST(parsertests, parse_catch_directive)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".record Exception {}").first);
        v.push_back(l.TokenizeString(".catch Exception, try_begin, try_end, catch_begin").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        ASSERT_EQ(e.line_number, 2);
        ASSERT_EQ(e.message, ".catch directive is located outside of a function body.");
    }

    {
        std::vector<std::string> directives {
            ".catch",        ".catch R",         ".catch R,",         ".catch R, t1",
            ".catch R, t1,", ".catch R, t1, t2", ".catch R, t1, t2,", ".catch R, t1, t2, c,"};

        for (auto s : directives) {
            std::vector<std::vector<panda::pandasm::Token>> v;
            Lexer l;
            Parser p;

            v.push_back(l.TokenizeString(".record Exception {}").first);
            v.push_back(l.TokenizeString(".function void main() {").first);
            v.push_back(l.TokenizeString(s).first);
            v.push_back(l.TokenizeString("}").first);

            p.Parse(v);

            Error e = p.ShowError();

            ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
            ASSERT_EQ(e.line_number, 3);
            ASSERT_EQ(e.pos, 0);
            ASSERT_EQ(e.message,
                      "Incorrect catch block declaration. Must be in the format: .catch <exception_record>, "
                      "<try_begin_label>, <try_end_label>, <catch_begin_label>[, <catch_end_label>]");
        }
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string s = ".catch $Exception, try_begin, try_end, catch_begin";

        v.push_back(l.TokenizeString(".record Exception {}").first);
        v.push_back(l.TokenizeString(".function void main() {").first);
        v.push_back(l.TokenizeString(s).first);
        v.push_back(l.TokenizeString("}").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_RECORD_NAME);
        ASSERT_EQ(e.line_number, 3);
        ASSERT_EQ(e.pos, s.find("$"));
        ASSERT_EQ(e.message, "Invalid name of the exception record.");
    }

    {
        std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};
        std::vector<std::string> label_names {"try block begin", "try block end", "catch block begin"};

        for (size_t i = 0; i < labels.size(); i++) {
            std::string s = ".catch Exception";

            {
                std::string directive = s;
                for (size_t j = 0; j < labels.size(); j++) {
                    directive += i == j ? " $ " : " , ";
                    directive += labels[j];
                }

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                v.push_back(l.TokenizeString(".record Exception {}").first);
                v.push_back(l.TokenizeString(".function void main() {").first);
                v.push_back(l.TokenizeString(directive).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION) << "Test " << directive;
                ASSERT_EQ(e.line_number, 3) << "Test " << directive;
                ASSERT_EQ(e.pos, directive.find("$")) << "Test " << directive;
                ASSERT_EQ(e.message, "Expected comma.") << "Test " << directive;
            }

            {
                std::string directive = s;
                for (size_t j = 0; j < labels.size(); j++) {
                    directive += " , ";
                    directive += i == j ? "$" : labels[j];
                }

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                v.push_back(l.TokenizeString(".record Exception {}").first);
                v.push_back(l.TokenizeString(".function void main() {").first);
                v.push_back(l.TokenizeString(directive).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL) << "Test " << directive;
                ASSERT_EQ(e.line_number, 3) << "Test " << directive;
                ASSERT_EQ(e.pos, directive.find("$")) << "Test " << directive;
                ASSERT_EQ(e.message, std::string("Invalid name of the ") + label_names[i] + " label.")
                    << "Test " << directive;
            }

            {
                std::stringstream ss;
                ss << "Test " << labels[i] << " does not exists";

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                std::string catch_table = ".catch Exception, try_begin, try_end, catch_begin";

                v.push_back(l.TokenizeString(".record Exception {}").first);
                v.push_back(l.TokenizeString(".function void main() {").first);
                for (size_t j = 0; j < labels.size(); j++) {
                    if (i != j) {
                        v.push_back(l.TokenizeString(labels[j] + ":").first);
                    }
                }
                v.push_back(l.TokenizeString(catch_table).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL_EXT) << ss.str();
                ASSERT_EQ(e.pos, catch_table.find(labels[i])) << ss.str();
                ASSERT_EQ(e.message, "This label does not exist.") << ss.str();
            }
        }
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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

        const auto sig_main = GetFunctionSignatureFromName("main", {});

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        auto &program = res.Value();
        auto &function = program.function_table.find(sig_main)->second;

        ASSERT_EQ(function.catch_blocks.size(), 1);
        ASSERT_EQ(function.catch_blocks[0].exception_record, "Exception");
        ASSERT_EQ(function.catch_blocks[0].try_begin_label, "try_begin");
        ASSERT_EQ(function.catch_blocks[0].try_end_label, "try_end");
        ASSERT_EQ(function.catch_blocks[0].catch_begin_label, "catch_begin");
        ASSERT_EQ(function.catch_blocks[0].catch_end_label, "catch_begin");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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

        const auto sig_main = GetFunctionSignatureFromName("main", {});

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        auto &program = res.Value();
        auto &function = program.function_table.find(sig_main)->second;

        ASSERT_EQ(function.catch_blocks.size(), 1);
        ASSERT_EQ(function.catch_blocks[0].exception_record, "Exception");
        ASSERT_EQ(function.catch_blocks[0].try_begin_label, "try_begin");
        ASSERT_EQ(function.catch_blocks[0].try_end_label, "try_end");
        ASSERT_EQ(function.catch_blocks[0].catch_begin_label, "catch_begin");
        ASSERT_EQ(function.catch_blocks[0].catch_end_label, "catch_end");
    }
}

TEST(parsertests, parse_catchall_directive)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        v.push_back(l.TokenizeString(".catchall try_begin, try_end, catch_begin").first);

        p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        ASSERT_EQ(e.line_number, 1);
        ASSERT_EQ(e.message, ".catchall directive is located outside of a function body.");
    }

    {
        std::vector<std::string> directives {".catchall",        ".catchall t1",      ".catchall t1,",
                                             ".catchall t1, t2", ".catchall t1, t2,", ".catchall t1, t2, c,"};

        for (auto s : directives) {
            std::vector<std::vector<panda::pandasm::Token>> v;
            Lexer l;
            Parser p;

            v.push_back(l.TokenizeString(".function void main() {").first);
            v.push_back(l.TokenizeString(s).first);
            v.push_back(l.TokenizeString("}").first);

            p.Parse(v);

            Error e = p.ShowError();

            ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
            ASSERT_EQ(e.line_number, 2);
            ASSERT_EQ(e.pos, 0);
            ASSERT_EQ(e.message,
                      "Incorrect catch block declaration. Must be in the format: .catchall <try_begin_label>, "
                      "<try_end_label>, <catch_begin_label>[, <catch_end_label>]");
        }
    }

    {
        std::vector<std::string> labels {"try_begin", "try_end", "catch_begin"};
        std::vector<std::string> label_names {"try block begin", "try block end", "catch block begin"};

        for (size_t i = 0; i < labels.size(); i++) {
            std::string s = ".catchall ";

            if (i != 0) {
                std::string directive = s;
                for (size_t j = 0; j < labels.size(); j++) {
                    if (j != 0) {
                        directive += i == j ? " $ " : " , ";
                    }
                    directive += labels[j];
                }

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                v.push_back(l.TokenizeString(".function void main() {").first);
                v.push_back(l.TokenizeString(directive).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION) << "Test " << directive;
                ASSERT_EQ(e.line_number, 2) << "Test " << directive;
                ASSERT_EQ(e.pos, directive.find("$")) << "Test " << directive;
                ASSERT_EQ(e.message, "Expected comma.") << "Test " << directive;
            }

            {
                std::string directive = s;
                for (size_t j = 0; j < labels.size(); j++) {
                    if (j != 0) {
                        directive += " , ";
                    }
                    directive += i == j ? "$" : labels[j];
                }

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                v.push_back(l.TokenizeString(".function void main() {").first);
                v.push_back(l.TokenizeString(directive).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL) << "Test " << directive;
                ASSERT_EQ(e.line_number, 2) << "Test " << directive;
                ASSERT_EQ(e.pos, directive.find("$")) << "Test " << directive;
                ASSERT_EQ(e.message, std::string("Invalid name of the ") + label_names[i] + " label.")
                    << "Test " << directive;
            }

            {
                std::stringstream ss;
                ss << "Test " << labels[i] << " does not exists";

                std::vector<std::vector<panda::pandasm::Token>> v;
                Lexer l;
                Parser p;

                std::string catch_table = ".catchall try_begin, try_end, catch_begin";

                v.push_back(l.TokenizeString(".function void main() {").first);
                for (size_t j = 0; j < labels.size(); j++) {
                    if (i != j) {
                        v.push_back(l.TokenizeString(labels[j] + ":").first);
                    }
                }
                v.push_back(l.TokenizeString(catch_table).first);
                v.push_back(l.TokenizeString("}").first);

                p.Parse(v);

                Error e = p.ShowError();

                ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_LABEL_EXT) << ss.str();
                ASSERT_EQ(e.pos, catch_table.find(labels[i])) << ss.str();
                ASSERT_EQ(e.message, "This label does not exist.") << ss.str();
            }
        }
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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

        const auto sig_main = GetFunctionSignatureFromName("main", {});

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);

        auto &program = res.Value();
        auto &function = program.function_table.find(sig_main)->second;

        ASSERT_EQ(function.catch_blocks.size(), 1);
        ASSERT_EQ(function.catch_blocks[0].exception_record, "");
        ASSERT_EQ(function.catch_blocks[0].try_begin_label, "try_begin");
        ASSERT_EQ(function.catch_blocks[0].try_end_label, "try_end");
        ASSERT_EQ(function.catch_blocks[0].catch_begin_label, "catch_begin");
    }
}

TEST(parsertests, parse_numbers)
{
    const auto sig_main = GetFunctionSignatureFromName("main", {});

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, 12345}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(12345)))
            << "12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, 0xFEFfe}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(0xFEFfe)))
            << "0xFEFfe expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, 01237}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(01237)))
            << "01237 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, 0b10101}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(0b10101)))
            << "0b10101 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, -12345}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(-12345)))
            << "-12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, -0xFEFfe}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(-0xFEFfe)))
            << "12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, -01237}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(-01237)))
            << "12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("movi v0, -0b10101}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(int64_t(-0b10101)))
            << "-0b10101 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.0}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.0)) << "1.0 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.)) << "1. expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, .1}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(.1)) << ".0 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1e10)) << "1e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1e+10)) << "1e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1e-10)) << "1e-10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.0e10)) << "1.0e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.0e+10)) << "1.0e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.0e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.0e-10)) << "12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.e10)) << "1.e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.e+10)) << "1.e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, 1.e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(1.e-10)) << "12345 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.0}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.0)) << "-1.0 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.)) << "-1. expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -.1}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-.1)) << "-.0 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1e10)) << "-1e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1e+10)) << "-1e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1e-10)) << "-1e-10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.0e10)) << "-1.0e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.0e+10)) << "-1.0e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.0e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.0e-10)) << "-1.0e-10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.e10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.e10)) << "-1.e10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.e+10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.e+10)) << "-1.e+10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;
        v.push_back(l.TokenizeString(".function u8 main(){").first);
        v.push_back(l.TokenizeString("fmovi.64 v0, -1.e-10}").first);
        auto item = p.Parse(v);
        ASSERT_EQ(item.Value().function_table.at(sig_main).ins[0].imms[0], Ins::IType(-1.e-10)) << "-1.e-10 expected";
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
    }
}

TEST(parsertests, field_value)
{
    {
        std::vector<std::vector<panda::pandasm::Token>> v;
        Lexer l;
        Parser p;

        std::string s = "i32 f <value=A>";

        v.push_back(l.TokenizeString(".record A {").first);
        v.push_back(l.TokenizeString(s).first);
        v.push_back(l.TokenizeString("}").first);

        auto res = p.Parse(v);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_BAD_METADATA_INVALID_VALUE);
        ASSERT_EQ(e.line_number, 2);
        ASSERT_EQ(e.pos, s.find("A"));
        ASSERT_EQ(e.message, "Excepted integer literal");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        auto &record = program.record_table.find("A")->second;
        auto &field = record.field_list[0];

        ASSERT_EQ(field.metadata->GetFieldType().GetName(), "i32");
        ASSERT_TRUE(field.metadata->GetValue());
        ASSERT_EQ(field.metadata->GetValue()->GetType(), Value::Type::I32);
        ASSERT_EQ(field.metadata->GetValue()->GetValue<int32_t>(), 10);
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        auto &record = program.record_table.find("A")->second;
        auto &field = record.field_list[0];

        ASSERT_EQ(field.metadata->GetFieldType().GetName(), "panda.String");
        ASSERT_TRUE(field.metadata->GetValue());
        ASSERT_EQ(field.metadata->GetValue()->GetType(), Value::Type::STRING);
        ASSERT_EQ(field.metadata->GetValue()->GetValue<std::string>(), "10");
    }

    {
        std::vector<std::vector<panda::pandasm::Token>> v;
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
        auto &record = program.record_table.find("A")->second;
        auto &field = record.field_list[0];

        ASSERT_EQ(field.metadata->GetFieldType().GetName(), "panda.String");
        ASSERT_FALSE(field.metadata->GetValue());
    }
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

TEST(parsertests, test_array_integer_def)
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
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[0].value_), true);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[1].value_), 2);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_, panda::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[2].value_)),
              -30);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_, panda::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array").literals_[3].value_), 400);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_, panda::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array").literals_[4].value_)),
              -5000);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[5].tag_, panda::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[5].value_), 60000);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[6].tag_, panda::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[6].value_)),
              -700000);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[7].tag_, panda::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarray_table.at("array").literals_[7].value_), 8000000);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[8].tag_, panda::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array").literals_[8].value_)),
              -90000000);
}

TEST(parsertests, test_array_float_def)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("f32 -123.4").first);
    v.push_back(l.TokenizeString("f64 -1234.5").first);
    v.push_back(l.TokenizeString("}").first);
    auto item = p.Parse(v);

    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array").literals_[0].value_), -123.4, 0.01f);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array").literals_[1].value_), -1234.5, 0.01f);
}

TEST(parsertests, test_array_string_def)
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

    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[0].value_), "a");
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[1].value_), "ab");
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[2].value_), "abc");

    // string intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_static").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_static").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_STRING));
    ASSERT_EQ(item.Value().literalarray_table.at("array_static").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_static").literals_[1].value_), 3);

    // string array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_static").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[2].value_), "a");
    ASSERT_EQ(item.Value().literalarray_table.at("array_static").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[3].value_), "ab");
    ASSERT_EQ(item.Value().literalarray_table.at("array_static").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array_static").literals_[4].value_), "abc");
}

TEST(parsertests, test_array_static_bool_def)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array u1 3 { 1 0 1 }").first);
    auto item = p.Parse(v);

    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U1));
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[1].value_), 3);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[2].value_), true);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[3].value_), false);
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_, panda::panda_file::LiteralTag::ARRAY_U1);
    ASSERT_EQ(std::get<bool>(item.Value().literalarray_table.at("array").literals_[4].value_), true);
}

TEST(parsertests, test_array_static_integer_def)
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
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U8));
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[1].value_), 3);

    // unsigned byte array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[2].value_), 1);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[3].value_), 2);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_byte").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U8);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_byte").literals_[4].value_), 3);

    // byte intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_byte").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I8));
    ASSERT_EQ(item.Value().literalarray_table.at("array_byte").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_byte").literals_[1].value_), 3);

    // byte array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_byte").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[2].value_)),
        -1);
    ASSERT_EQ(item.Value().literalarray_table.at("array_byte").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[3].value_)),
        2);
    ASSERT_EQ(item.Value().literalarray_table.at("array_byte").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I8);
    ASSERT_EQ(
        static_cast<int8_t>(std::get<uint8_t>(item.Value().literalarray_table.at("array_byte").literals_[4].value_)),
        -3);

    // unsigned short intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U16));
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[1].value_), 3);

    // unsigned short array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[2].value_), 100);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[3].value_), 200);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_short").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U16);
    ASSERT_EQ(std::get<uint16_t>(item.Value().literalarray_table.at("array_unsigned_short").literals_[4].value_), 300);

    // short intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_short").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_short").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I16));
    ASSERT_EQ(item.Value().literalarray_table.at("array_short").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_short").literals_[1].value_), 3);

    // short array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_short").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[2].value_)),
        100);
    ASSERT_EQ(item.Value().literalarray_table.at("array_short").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[3].value_)),
        -200);
    ASSERT_EQ(item.Value().literalarray_table.at("array_short").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I16);
    ASSERT_EQ(
        static_cast<int16_t>(std::get<uint16_t>(item.Value().literalarray_table.at("array_short").literals_[4].value_)),
        300);

    // unsigned int intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U32));
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[1].value_), 3);

    // unsigned int array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[2].value_), 1000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[3].value_), 2000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_int").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U32);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_int").literals_[4].value_), 3000);

    // int intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_int").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_int").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I32));
    ASSERT_EQ(item.Value().literalarray_table.at("array_int").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[1].value_), 3);

    // int array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_int").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[2].value_)),
        -1000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_int").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[3].value_)),
        2000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_int").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I32);
    ASSERT_EQ(
        static_cast<int32_t>(std::get<uint32_t>(item.Value().literalarray_table.at("array_int").literals_[4].value_)),
        -3000);

    // unsigned long intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_long").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_unsigned_long").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_U64));
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_long").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_unsigned_long").literals_[1].value_), 3);

    // unsigned long array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_long").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarray_table.at("array_unsigned_long").literals_[2].value_), 10000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_long").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarray_table.at("array_unsigned_long").literals_[3].value_), 20000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_unsigned_long").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_U64);
    ASSERT_EQ(std::get<uint64_t>(item.Value().literalarray_table.at("array_unsigned_long").literals_[4].value_), 30000);

    // long intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_long").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_long").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_I64));
    ASSERT_EQ(item.Value().literalarray_table.at("array_long").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_long").literals_[1].value_), 3);

    // long array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_long").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[2].value_)),
        10000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_long").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[3].value_)),
        -20000);
    ASSERT_EQ(item.Value().literalarray_table.at("array_long").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_I64);
    ASSERT_EQ(
        static_cast<int64_t>(std::get<uint64_t>(item.Value().literalarray_table.at("array_long").literals_[4].value_)),
        30000);
}

TEST(parsertests, test_array_static_float_def)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array_float f32 3 { 12.3 -12.34 12.345 }").first);
    v.push_back(l.TokenizeString(".array array_double f64 3 { -120.3 120.34 -120.345 }").first);
    auto item = p.Parse(v);

    // float intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_float").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_float").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_F32));
    ASSERT_EQ(item.Value().literalarray_table.at("array_float").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_float").literals_[1].value_), 3);

    // float array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_float").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[2].value_), 12.3, 0.01f);
    ASSERT_EQ(item.Value().literalarray_table.at("array_float").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[3].value_), -12.34, 0.001f);
    ASSERT_EQ(item.Value().literalarray_table.at("array_float").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_F32);
    ASSERT_NEAR(std::get<float>(item.Value().literalarray_table.at("array_float").literals_[4].value_), 12.345,
                0.0001f);

    // double intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array_double").literals_[0].tag_,
              panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array_double").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_F64));
    ASSERT_EQ(item.Value().literalarray_table.at("array_double").literals_[1].tag_,
              panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array_double").literals_[1].value_), 3);

    // double array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array_double").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[2].value_), -120.3,
                0.01f);
    ASSERT_EQ(item.Value().literalarray_table.at("array_double").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[3].value_), 120.34,
                0.001f);
    ASSERT_EQ(item.Value().literalarray_table.at("array_double").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_F64);
    ASSERT_NEAR(std::get<double>(item.Value().literalarray_table.at("array_double").literals_[4].value_), -120.345,
                0.0001f);
}

TEST(parsertests, test_array_static_string_def)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser p;
    v.push_back(l.TokenizeString(".array array panda.String 3 { \"a\" \"ab\" \"abc\" }").first);
    auto item = p.Parse(v);

    // string intro literals
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[0].tag_, panda::panda_file::LiteralTag::TAGVALUE);
    ASSERT_EQ(std::get<uint8_t>(item.Value().literalarray_table.at("array").literals_[0].value_),
              static_cast<uint8_t>(panda::panda_file::LiteralTag::ARRAY_STRING));
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[1].tag_, panda::panda_file::LiteralTag::INTEGER);
    ASSERT_EQ(std::get<uint32_t>(item.Value().literalarray_table.at("array").literals_[1].value_), 3);

    // string array elements
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[2].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[2].value_), "a");
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[3].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[3].value_), "ab");
    ASSERT_EQ(item.Value().literalarray_table.at("array").literals_[4].tag_,
              panda::panda_file::LiteralTag::ARRAY_STRING);
    ASSERT_EQ(std::get<std::string>(item.Value().literalarray_table.at("array").literals_[4].value_), "abc");
}

TEST(parsertests, test_array_string_use)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
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
    v.push_back(l.TokenizeString(".array array_static panda.String 3 { \"a\" \"ab\" \"abc\" }").first);

    v.push_back(l.TokenizeString(".function void f() {").first);
    v.push_back(l.TokenizeString("lda.const v0, array").first);
    v.push_back(l.TokenizeString("lda.const v1, array_static").first);
    v.push_back(l.TokenizeString("}").first);

    auto item = p.Parse(v);

    const auto sig_f = GetFunctionSignatureFromName("f", {});

    ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].opcode, Opcode::LDA_CONST);
    ASSERT_EQ(item.Value().function_table.at(sig_f).ins[0].ids[0], "array");

    ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].opcode, Opcode::LDA_CONST);
    ASSERT_EQ(item.Value().function_table.at(sig_f).ins[1].ids[0], "array_static");
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
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"i8", 0}, language);
    const auto sig_f = GetFunctionSignatureFromName("f", {});
    const auto sig_fi8 = GetFunctionSignatureFromName("f", params);

    ASSERT_TRUE(program.function_table.find(sig_f) != program.function_table.end());
    ASSERT_TRUE(program.function_table.find(sig_fi8) != program.function_table.end());
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
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"u1", 0}, language);
    const auto sig_f = GetFunctionSignatureFromName("f", {});
    const auto sig_main = GetFunctionSignatureFromName("main", params);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[1].ids[0], sig_f);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[0].ids[0], sig_f);
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

    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::vector<Function::Parameter> params_main;
    params_main.emplace_back(Type {"u1", 0}, language);
    std::vector<Function::Parameter> params_f;
    params_f.emplace_back(Type {"i8", 0}, language);
    const auto sig_f = GetFunctionSignatureFromName("f", params_f);
    const auto sig_main = GetFunctionSignatureFromName("main", params_main);

    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[1].ids[0], sig_f);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[1].regs[0], 1);  //  v0, [v1], a0
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[0].ids[0], sig_f);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[0].regs[0], 2);  //  v0,  v1, [a0]
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

    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::vector<Function::Parameter> params_main;
    params_main.emplace_back(Type {"u1", 0}, language);
    std::vector<Function::Parameter> params_fi8;
    params_fi8.emplace_back(Type {"i8", 0}, language);
    std::vector<Function::Parameter> params_fu8;
    params_fu8.emplace_back(Type {"u8", 0}, language);
    std::vector<Function::Parameter> params_fi8u8;
    params_fi8u8.emplace_back(Type {"i8", 0}, language);
    params_fi8u8.emplace_back(Type {"u8", 0}, language);
    const auto sig_f = GetFunctionSignatureFromName("f", {});
    const auto sig_fi8 = GetFunctionSignatureFromName("f", params_fi8);
    const auto sig_fu8 = GetFunctionSignatureFromName("f", params_fu8);
    const auto sig_fi8u8 = GetFunctionSignatureFromName("f", params_fi8u8);
    const auto sig_main = GetFunctionSignatureFromName("main", params_main);

    ASSERT_TRUE(program.function_table.find(sig_main) != program.function_table.end());
    ASSERT_TRUE(program.function_table.find(sig_f) != program.function_table.end());
    ASSERT_TRUE(program.function_table.find(sig_fi8) != program.function_table.end());
    ASSERT_TRUE(program.function_table.find(sig_fu8) != program.function_table.end());
    ASSERT_TRUE(program.function_table.find(sig_fi8u8) != program.function_table.end());

    // f:(i8)
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[0].ids[0], sig_fi8);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[0].regs[0], 5);

    // f:()
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[1].ids[0], sig_f);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[1].regs[0], 1);

    // f:(u8)
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[2].ids[0], sig_fu8);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[2].regs[0], 2);

    // f:(i8u8)
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[3].ids[0], sig_fi8u8);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[3].regs[0], 3);
    ASSERT_EQ(res.Value().function_table.at(sig_main).ins[3].regs[1], 4);
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
