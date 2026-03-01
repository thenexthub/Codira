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

#include "assembly-ins.h"
#include "assembly-parser.h"
#include "ide_helpers.h"
#include "ins_to_string.cpp"
#include "operand_types_print.h"
#include "extensions/ecmascript_meta.h"
#include "meta.h"

using namespace testing::ext;

namespace panda::pandasm {
class AssemblerInsTest : public testing::Test {
};

/**
 * @tc.name: assembler_ins_test_001
 * @tc.desc: Verify the Parse function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_001, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any foo(){}
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
    )";
    auto item = p.Parse(source);
    const std::string func_name = "func:(any,any,any)";
    auto it = item.Value().function_table.find(func_name);
    EXPECT_NE(it, item.Value().function_table.end());
    const auto &func_value = item.Value().function_table.at(func_name).ins;
    size_t json_size = 280;
    EXPECT_EQ(func_value[3]->OperandListLength(), 2ULL);
    EXPECT_EQ(func_value[3]->HasFlag(InstFlags::TYPE_ID), false);
    EXPECT_EQ(func_value[3]->CanThrow(), true);
    EXPECT_EQ(func_value[3]->IsJump(), false);
    EXPECT_EQ(func_value[3]->IsConditionalJump(), false);
    EXPECT_EQ(func_value[3]->IsCall(), false);
    EXPECT_EQ(func_value[3]->IsCallRange(), false);
    EXPECT_EQ(func_value[3]->IsPseudoCall(), false);
    EXPECT_EQ(func_value[7]->IsReturn(), true);
    EXPECT_EQ(func_value[7]->MaxRegEncodingWidth(), 0);
    EXPECT_EQ(func_value[7]->HasDebugInfo(), true);
    EXPECT_EQ(func_value[7]->IsValidToEmit(), true);
    EXPECT_EQ(item.Value().JsonDump().size(), json_size);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: assembler_ins_test_002
 * @tc.desc: Verify the Parse function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_002, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any func() {
            sta v4
            lda.str "xxx"
            ldglobalvar 0x7, "oDiv"
            callarg1 0x1, v0
            return
        }
    )";
    auto item = p.Parse(source);
    const std::string func_name = "func:()";
    auto it = item.Value().function_table.find(func_name);
    EXPECT_NE(it, item.Value().function_table.end());
    const auto &function_value = item.Value().function_table.at(func_name).ins;
    std::string ret = function_value[0]->ToString("test", true, 0);
    EXPECT_EQ(ret, "sta a4test");
    ret = function_value[0]->ToString("test", false, 0);
    EXPECT_EQ(ret, "sta v4test");

    ret = function_value[1]->ToString("test", true, 0);
    EXPECT_EQ(ret, "lda.str xxxtest");
    ret = function_value[1]->ToString("test", false, 0);
    EXPECT_EQ(ret, "lda.str xxxtest");

    ret = function_value[2]->ToString("test", true, 0);
    EXPECT_EQ(ret, "ldglobalvar 0x7, oDivtest");
    ret = function_value[2]->ToString("test", false, 0);
    EXPECT_EQ(ret, "ldglobalvar 0x7, oDivtest");
}

/**
 * @tc.name: assembler_ins_test_003
 * @tc.desc: Verify the ToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_003, TestSize.Level1)
{
    uint16_t reg1 = 2U;
    uint16_t reg2 = 3U;
    auto opcode = Opcode::DEPRECATED_LDMODULEVAR;
    std::vector<uint16_t> regs;
    regs.push_back(reg1);
    regs.push_back(reg2);
    std::vector<IType> imms;
    imms.push_back(IType(int64_t(0x1)));
    std::vector<std::string> ids;
    ids.push_back("a1");
    panda::pandasm::Ins *ins = Ins::CreateIns(opcode, regs, imms, ids);

    std::string ret = ins->ToString("test", true, 0);
    EXPECT_EQ(ret, "deprecated.ldmodulevar a1, 0x1test");
    ret = ins->ToString("test", false, 0);
    EXPECT_EQ(ret, "deprecated.ldmodulevar a1, 0x1test");
    delete ins;

    opcode = Opcode::DEFINEFUNC;
    imms.push_back(IType(int64_t(0x2)));
    ins = Ins::CreateIns(opcode, regs, imms, ids);
    ret = ins->ToString("test", true, 0);
    EXPECT_EQ(ret, "definefunc 0x1, a1, 0x2test");
    ret = ins->ToString("test", false, 0);
    EXPECT_EQ(ret, "definefunc 0x1, a1, 0x2test");
    EXPECT_TRUE(ins->CanThrow());
    delete ins;

    opcode = Opcode::JEQZ;
    ins = Ins::CreateIns(opcode, regs, imms, ids);
    EXPECT_TRUE(ins->IsConditionalJump());
    delete ins;
}

/**
 * @tc.name: assembler_ins_test_004
 * @tc.desc: Verify the JsonDump function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_004, TestSize.Level1)
{
    panda::pandasm::Program pro;
    std::string ret = pro.JsonDump();
    EXPECT_EQ(ret, "{ \"functions\": [  ], \"records\": [  ] }");

    std::string_view component_name = "u8";
    size_t rank = 0;
    panda::pandasm::Type type(component_name, rank);
    bool bo = type.IsArrayContainsPrimTypes();
    EXPECT_TRUE(bo);

    ret = type.GetDescriptor(false);
    EXPECT_EQ(ret, "H");

    EXPECT_EQ(panda::pandasm::Type::GetId(component_name, true), panda_file::Type::TypeId::REFERENCE);
    component_name = "[]";
    EXPECT_EQ(panda::pandasm::Type::FromDescriptor(component_name).GetName(), component_name);
    EXPECT_EQ(panda::pandasm::Type::FromName(component_name, false).GetName(), component_name);
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};

    std::string name = "test";
    EXPECT_FALSE(panda::pandasm::Type::IsStringType(name, language));

    component_name = "test";
    rank = 0;
    panda::pandasm::Type type1(component_name, rank);
    bo = type1.IsArrayContainsPrimTypes();
    EXPECT_FALSE(bo);
}

/**
 * @tc.name: assembler_ins_test_005
 * @tc.desc: Verify the JsonSerializeProgramItems function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_005, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any func() {
            sta v4
            lda.str "xxx"
            ldglobalvar 0x7, "oDiv"
            callarg1 0x1, v0
            return
        }
    )";
    auto item = p.Parse(source);
    const std::string func_name = "func:()";
    panda::panda_file::SourceLang language1 {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    panda::pandasm::Function function("fun", language1);
    function.file_location->is_defined = false;
    panda::pandasm::Function function1("fun", language1, 0, 10, "func", false, 10);

    std::string ret = JsonSerializeItemBody(function);
    EXPECT_EQ(ret, "{ \"name\": \"fun\" }");

    std::map<std::string, panda::pandasm::Function> function_table;
    ret = JsonSerializeProgramItems(function_table);
    EXPECT_EQ(ret, "[  ]");

    std::string test = "test";
    function_table.emplace(test, std::move(function1));

    ret = JsonSerializeProgramItems(function_table);
    EXPECT_EQ(ret, "[ { \"name\": \"fun\" } ]");

    ret = item.Value().function_table.at(func_name).ins[3]->ToString("test", true, 0);
    EXPECT_EQ(ret, "callarg1 0x1, a0test");
    ret = item.Value().function_table.at(func_name).ins[3]->ToString("test", false, 0);
    EXPECT_EQ(ret, "callarg1 0x1, v0test");
}

/**
 * @tc.name: assembler_ins_test_006
 * @tc.desc: Verify the NextMask function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_006, TestSize.Level1)
{
    Context con;
    con.token = "test";

    EXPECT_GT(con.Len(), 0);
    size_t number_of_params_already_is = 65535;
    EXPECT_FALSE(con.ValidateParameterName(number_of_params_already_is));
    number_of_params_already_is = 4;
    EXPECT_FALSE(con.ValidateParameterName(number_of_params_already_is));

    panda::pandasm::Token token1;
    con.tokens.push_back(token1);
    panda::pandasm::Token token2;
    con.tokens.push_back(token2);
    EXPECT_EQ(con.Next(), Token::Type::ID_BAD);
    size_t line_size = 3;
    con.number = line_size;
    con.end = false;
    EXPECT_TRUE(con.NextMask());
    con.end = true;
    EXPECT_TRUE(con.NextMask());
}
}
