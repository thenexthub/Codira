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

#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include "abc2program_driver.h"
#include "abc2program_test_utils.h"
#include "common/abc_file_utils.h"
#include "dump_utils.h"
#include "modifiers.h"

using namespace testing::ext;

namespace panda::abc2program {

struct FuncName {
    std::string hello_world = ".#~@0=#HelloWorld";
    std::string foo = ".#*#foo";
    std::string goo = ".#*#goo";
    std::string hoo = ".#*#hoo";
    std::string main = ".func_main_0";
    std::string method = ".#*@3*#method";
    std::string lit = ".#~@1>#lit";
    std::string add = ".#*#add";
    std::string generate = ".#*#generateFunc";
    std::string async_generate = ".#*#asyncGenerateFunc";
    std::string async_arrow = ".#*#asyncArrowFunc";
    std::string nested_literal_array = ".#~@2>#NestedLiteralArray";
    std::string class_with_default_annotation = ".#~A=#A";
    std::string class_with_spec_annotation = ".#~B=#B";
};

struct RecordName {
    std::string annotation = "Anno";
};

const FuncName FUNC_NAME;
const RecordName RECORD_NAME;
const std::string HELLO_WORLD_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "HelloWorld.abc";
const std::string HELLO_WORLD_DEBUG_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "HelloWorldDebug.abc";
const std::string JSON_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "JsonTest.abc";
const std::string JSON_TEST_FILE_NAME = "JsonTest";
const bool REMOVE_DUMP_RESULT_FILES = true;
const std::string HELLO_WORLD_DUMP_RESULT_FILE_NAME = GRAPH_TEST_ABC_DUMP_DIR "HelloWorldDumpResult.txt";
const std::string HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME = GRAPH_TEST_ABC_DUMP_DIR "HelloWorldDebugDumpResult.txt";
const std::string HELLO_WORLD_DUMP_EXPECTED_FILE_NAME = GRAPH_TEST_ABC_DUMP_DIR "HelloWorldDumpExpected.txt";
const std::string HELLO_WORLD_DEBUG_DUMP_EXPECTED_FILE_NAME = GRAPH_TEST_ABC_DUMP_DIR "HelloWorldDebugDumpExpected.txt";
constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_INS = 85;
constexpr uint8_t INS_SIZE_OF_FUNCTION_HOO = 4;
constexpr uint8_t IMMS_SIZE_OF_OPCODE_FLDAI = 1;
constexpr uint8_t SIZE_OF_LITERAL_ARRAY_TABLE = 7;
constexpr uint8_t TOTAL_NUM_OF_ASYNC_METHOD_LITERALS = 6;
constexpr uint8_t TOTAL_NUM_OF_MODULE_LITERALS = 21;
constexpr uint8_t TOTAL_NUM_OF_NESTED_LITERALS = 10;
constexpr uint8_t NUM_OF_MODULE_REQUESTS = 4;
constexpr uint8_t NUM_OF_REGULAR_IMPORTS = 1;
constexpr uint8_t NUM_OF_NAMESPACE_IMPORTS = 1;
constexpr uint8_t NUM_OF_LOCAL_EXPORTS = 1;
constexpr uint8_t NUM_OF_INDIRECT_EXPORTS = 1;
constexpr uint8_t NUM_OF_STAR_EXPORTS = 1;
const std::string ANNOTATIONS_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "Annotations.abc";


static const pandasm::Function *GetFunction(const std::string &name,
                                            const pandasm::Program &program)
{
    ASSERT(program.function_table.find(name) != program.function_table.end());
    return &(program.function_table.find(name)->second);
}

static const pandasm::Record *GetRecord(const std::string &name,
                                        const pandasm::Program &program)
{
    ASSERT(program.record_table.find(name) != program.record_table.end());
    return &(program.record_table.find(name)->second);
}

class Abc2ProgramHelloWorldTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(HELLO_WORLD_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
        hello_world_function_ = GetFunction(FUNC_NAME.hello_world, *prog_);
        foo_function_ = GetFunction(FUNC_NAME.foo, *prog_);
        goo_function_ = GetFunction(FUNC_NAME.goo, *prog_);
        hoo_function_ = GetFunction(FUNC_NAME.hoo, *prog_);
        main_function_ = GetFunction(FUNC_NAME.main, *prog_);
        method_function_ = GetFunction(FUNC_NAME.method, *prog_);
        lit_function_ = GetFunction(FUNC_NAME.lit, *prog_);
        add_function_ = GetFunction(FUNC_NAME.add, *prog_);
        generate_function_ = GetFunction(FUNC_NAME.generate, *prog_);
        async_generate_function_ = GetFunction(FUNC_NAME.async_generate, *prog_);
        async_arrow_function_ = GetFunction(FUNC_NAME.async_arrow, *prog_);
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
    const pandasm::Function *hello_world_function_ = nullptr;
    const pandasm::Function *foo_function_ = nullptr;
    const pandasm::Function *goo_function_ = nullptr;
    const pandasm::Function *hoo_function_ = nullptr;
    const pandasm::Function *main_function_ = nullptr;
    const pandasm::Function *method_function_ = nullptr;
    const pandasm::Function *lit_function_ = nullptr;
    const pandasm::Function *add_function_ = nullptr;
    const pandasm::Function *generate_function_ = nullptr;
    const pandasm::Function *async_generate_function_ = nullptr;
    const pandasm::Function *async_arrow_function_ = nullptr;
};

class Abc2ProgramHelloWorldDebugTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(HELLO_WORLD_DEBUG_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
        foo_function_ = GetFunction(FUNC_NAME.foo, *prog_);
        main_function_ = GetFunction(FUNC_NAME.main, *prog_);
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
    const pandasm::Function *foo_function_ = nullptr;
    const pandasm::Function *main_function_ = nullptr;
};

class Abc2ProgramJsonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(JSON_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

class Abc2ProgramAnnotationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(ANNOTATIONS_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
        a_function_ = GetFunction(FUNC_NAME.class_with_default_annotation, *prog_);
        b_function_ = GetFunction(FUNC_NAME.class_with_spec_annotation, *prog_);
        anno_record_ = GetRecord(RECORD_NAME.annotation, *prog_);
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
    const pandasm::Function *a_function_ = nullptr;
    const pandasm::Function *b_function_ = nullptr;
    const pandasm::Record *anno_record_ = nullptr;
};

/*------------------------------------- Cases of release mode below -------------------------------------*/

/**
 * @tc.name: abc2program_hello_world_test_dump
 * @tc.desc: check dump result in release mode.
 * @tc.type: FUNC
 * @tc.require: IADG92
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_dump, TestSize.Level1)
{
    driver_.Dump(HELLO_WORLD_DUMP_RESULT_FILE_NAME);
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateDumpResult(HELLO_WORLD_DUMP_RESULT_FILE_NAME,
                                                         HELLO_WORLD_DUMP_EXPECTED_FILE_NAME));
    if (REMOVE_DUMP_RESULT_FILES) {
        Abc2ProgramTestUtils::RemoveDumpResultFile(HELLO_WORLD_DUMP_RESULT_FILE_NAME);
    }
}

/**
 * @tc.name: abc2program_hello_world_test_func_annotation
 * @tc.desc: get program function annotation.
 * @tc.type: FUNC
 * @tc.require: #I9AQ3K
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_func_annotation, TestSize.Level1)
{
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_HELLO_WORLD_SLOTS_NUM = 2;
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_FOO_SLOTS_NUM = 24;
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_GOO_SLOTS_NUM = 0;
    EXPECT_TRUE(hello_world_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_HELLO_WORLD_SLOTS_NUM);
    EXPECT_TRUE(foo_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_FOO_SLOTS_NUM);
    EXPECT_TRUE(goo_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_GOO_SLOTS_NUM);
    EXPECT_TRUE(hello_world_function_->concurrent_module_requests.empty());
    EXPECT_TRUE(foo_function_->concurrent_module_requests.empty());
    EXPECT_TRUE(goo_function_->concurrent_module_requests.empty());
}

/**
 * @tc.name: abc2program_hello_world_test_field_metadata
 * @tc.desc: get program field metadata.
 * @tc.type: FUNC
 * @tc.require: issueI98NGN
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_field_metadata, TestSize.Level1)
{
    for (const auto &it : prog_->record_table) {
        if (it.first == "_ESModuleRecord") {
            const pandasm::Record &record = it.second;
            const std::vector<pandasm::Field> &field_list = record.field_list;
            const pandasm::Field &field = field_list[0];
            EXPECT_TRUE(field.type.GetPandasmName() == "u32");
            EXPECT_TRUE(field.name.find("HelloWorld.ts") != std::string::npos);
        }
    }
}

/**
 * @tc.name: abc2program_hello_world_test_lang
 * @tc.desc: get program language.
 * @tc.type: FUNC
 * @tc.require: issueI96G2J
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_lang, TestSize.Level1)
{
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    bool lang_matched = (prog_->lang == expected_lang);
    EXPECT_TRUE(lang_matched);
}

/**
 * @tc.name: abc2program_hello_world_test_literalarray_table
 * @tc.desc: get program literalarray_table.
 * @tc.type: FUNC
 * @tc.require: issueI9DK5D
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_literalarray_table, TestSize.Level1)
{
    std::set<size_t> literals_sizes;
    std::vector<std::string> literal_array_keys;
    for (const auto &it : prog_->literalarray_table) {
        const pandasm::LiteralArray &literal_array = it.second;
        size_t literals_size = literal_array.literals_.size();
        literals_sizes.insert(literals_size);
        literal_array_keys.emplace_back(it.first);
    }
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateLiteralsSizes(literals_sizes));
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateLiteralArrayKeys(literal_array_keys));
}

/**
 * @tc.name: abc2program_async_method_literals
 * @tc.desc: get and check literals of async generator method.
 * @tc.type: FUNC
 * @tc.require: issueI9SLHH
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_async_method_literals, TestSize.Level1)
{
    auto &literal_array_table = prog_->literalarray_table;
    EXPECT_EQ(literal_array_table.size(), SIZE_OF_LITERAL_ARRAY_TABLE);
    panda::pandasm::LiteralArray async_literals;
    for (auto &item : literal_array_table) {
        for (auto &literal : item.second.literals_) {
            if (literal.tag_ == panda_file::LiteralTag::ASYNCGENERATORMETHOD) {
                async_literals = item.second;
                break;
            }
        }
        if (async_literals.literals_.size() != 0) {
            break;
        }
    }
    EXPECT_EQ(async_literals.literals_.size(), TOTAL_NUM_OF_ASYNC_METHOD_LITERALS);
    auto it = async_literals.literals_.begin();
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::STRING);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::ASYNCGENERATORMETHOD);
    EXPECT_EQ(std::get<std::string>(it->value_), FUNC_NAME.method);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::METHODAFFILIATE);
}

/**
 * @tc.name: abc2program_hello_world_test_record_table
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: issueI96G2J
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_record_table, TestSize.Level1)
{
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    std::vector<std::string> record_names;
    for (const auto &it : prog_->record_table) {
        EXPECT_TRUE(it.second.language == expected_lang);
        EXPECT_TRUE(it.second.source_file == "");
        EXPECT_TRUE(it.first == it.second.name);
        record_names.emplace_back(it.first);
    }
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateRecordNames(record_names));
}

/**
 * @tc.name: abc2program_hello_world_test_fields
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: issueI98NGN
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_fields, TestSize.Level1)
{
    for (const auto &it : prog_->record_table) {
        if (it.first == "_ESModuleRecord" || it.first == "_ESScopeNamesRecord") {
            const pandasm::Record &record = it.second;
            const std::vector<pandasm::Field> &field_list = record.field_list;
            EXPECT_EQ(field_list.size(), 1);
            const pandasm::Field &field = field_list[0];
            EXPECT_EQ(field.type.GetPandasmName(), "u32");
            EXPECT_NE(field.name.find("HelloWorld.ts"), std::string::npos);
        } else {
            EXPECT_EQ(it.second.field_list.size(), 0);
        }
    }
}

/**
 * @tc.name: abc2program_hello_world_test_strings
 * @tc.desc: get existed string.
 * @tc.type: FUNC
 * @tc.require: issueI96G2J
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_strings, TestSize.Level1)
{
    bool string_matched = Abc2ProgramTestUtils::ValidateProgramStrings(prog_->strings);
    EXPECT_TRUE(string_matched);
}

/**
 * @tc.name: abc2program_hello_world_test_function_kind_access_flags
 * @tc.desc: get existed function_kind.
 * @tc.type: FUNC
 * @tc.require: issueI9BPIO
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_function_kind_access_flags, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    panda_file::FunctionKind function_kind = function.function_kind;
    EXPECT_TRUE(function_kind == panda_file::FunctionKind::FUNCTION);
    uint32_t access_flags = function.metadata->GetAccessFlags();
    EXPECT_TRUE((access_flags & ACC_STATIC) != 0);
    EXPECT_TRUE(lit_function_->GetFunctionKind() == panda::panda_file::FunctionKind::NONE);
    EXPECT_TRUE(foo_function_->GetFunctionKind() == panda::panda_file::FunctionKind::FUNCTION);
    EXPECT_TRUE(add_function_->GetFunctionKind() == panda::panda_file::FunctionKind::NC_FUNCTION);
    EXPECT_TRUE(generate_function_->GetFunctionKind() == panda::panda_file::FunctionKind::GENERATOR_FUNCTION);
    EXPECT_TRUE(method_function_->GetFunctionKind() == panda::panda_file::FunctionKind::ASYNC_FUNCTION);
    EXPECT_TRUE(async_generate_function_->GetFunctionKind() ==
        panda::panda_file::FunctionKind::ASYNC_GENERATOR_FUNCTION);
    EXPECT_TRUE(async_arrow_function_->GetFunctionKind() == panda::panda_file::FunctionKind::ASYNC_NC_FUNCTION);
}

/**
 * @tc.name: abc2program_code_test_function_foo_part1
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: issueI989S6
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part1, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    constexpr size_t NUM_OF_ARGS_FOR_FOO_METHOD = 3;
    EXPECT_TRUE(function.params.size() == NUM_OF_ARGS_FOR_FOO_METHOD);
    for (size_t i = 0; i < function.params.size(); ++i) {
        EXPECT_TRUE(function.params[i].type.GetPandasmName() == ANY_TYPE_NAME);
    }
    EXPECT_TRUE(function.name == FUNC_NAME.foo);
    EXPECT_TRUE(function.ins.size() == NUM_OF_CODE_TEST_UT_FOO_METHOD_INS);
    // check ins[0]
    const pandasm::InsPtr &ins0 = function.ins[0];
    std::string pa_ins_str0 = ins0->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str0 == "nop");
    EXPECT_TRUE(ins0->Label() == "");
    EXPECT_FALSE(ins0->IsLabel());
    // check ins[3]
    const pandasm::InsPtr &ins3 = function.ins[3];
    std::string pa_ins_str3 = ins3->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str3 == "label@3: ");
    EXPECT_TRUE(ins3->Label() == "label@3");
    EXPECT_TRUE(ins3->IsLabel());
    // check ins[10]
    const pandasm::InsPtr &ins10 = function.ins[10];
    std::string pa_ins_str10 = ins10->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str10 == "label@9: ");
    EXPECT_TRUE(ins10->Label() == "label@9");
    EXPECT_TRUE(ins10->IsLabel());
    // check ins[13]
    const pandasm::InsPtr &ins13 = function.ins[13];
    std::string pa_ins_str13 = ins13->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str13 == "label@11: ");
    EXPECT_TRUE(ins13->Label() == "label@11");
    EXPECT_TRUE(ins13->IsLabel());
}

/**
 * @tc.name: abc2program_code_test_function_foo_part2
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: issueI989S6
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part2, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;

    // check ins[15]
    const pandasm::InsPtr &ins15 = function.ins[15];
    std::string pa_ins_str15 = ins15->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str15 == "label@12: ");
    EXPECT_TRUE(ins15->Label() == "label@12");
    EXPECT_TRUE(ins15->IsLabel());
    // check ins[27]
    const pandasm::InsPtr &ins27 = function.ins[27];
    std::string pa_ins_str27 = ins27->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str27 == "tryldglobalbyname 0x8, varA");
    EXPECT_TRUE(ins27->Label() == "");
    EXPECT_FALSE(ins27->IsLabel());
    // check ins[31]
    const pandasm::InsPtr &ins31 = function.ins[31];
    std::string pa_ins_str31 = ins31->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str31 == "jeqz label@29");
    EXPECT_TRUE(ins31->Label() == "");
    EXPECT_FALSE(ins31->IsLabel());
    // check ins[34]
    const pandasm::InsPtr &ins34 = function.ins[34];
    std::string pa_ins_str34 = ins34->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str34 == "label@29: ");
    EXPECT_TRUE(ins34->Label() == "label@29");
    EXPECT_TRUE(ins34->IsLabel());
    // check ins[39]
    const pandasm::InsPtr &ins39 = function.ins[39];
    std::string pa_ins_str39 = ins39->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str39 == "jeqz label@36");
    EXPECT_TRUE(ins39->Label() == "");
    EXPECT_FALSE(ins39->IsLabel());
    // check ins[42]
    const pandasm::InsPtr &ins42 = function.ins[42];
    std::string pa_ins_str42 = ins42->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str42 == "label@36: ");
    EXPECT_TRUE(ins42->Label() == "label@36");
    EXPECT_TRUE(ins42->IsLabel());
}

/**
 * @tc.name: abc2program_code_test_function_foo_part3
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: issueI989S6
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part3, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    // check ins[45]
    const pandasm::InsPtr &ins45 = function.ins[45];
    std::string pa_ins_str45 = ins45->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str45 == "label@38: ");
    EXPECT_TRUE(ins45->Label() == "label@38");
    EXPECT_TRUE(ins45->IsLabel());
    // check ins[52]
    const pandasm::InsPtr &ins52 = function.ins[52];
    std::string pa_ins_str52 = ins52->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str52 == "sta v4");
    EXPECT_TRUE(ins52->Label() == "");
    EXPECT_FALSE(ins52->IsLabel());
    // check ins[55]
    const pandasm::InsPtr &ins55 = function.ins[55];
    std::string pa_ins_str55 = ins55->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str55 == "label@47: ");
    EXPECT_TRUE(ins55->Label() == "label@47");
    EXPECT_TRUE(ins55->IsLabel());
    // check ins[60]
    const pandasm::InsPtr &ins60 = function.ins[60];
    std::string pa_ins_str60 = ins60->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str60 == "jmp label@53");
    EXPECT_TRUE(ins60->Label() == "");
    EXPECT_FALSE(ins60->IsLabel());
    // check ins[61]
    const pandasm::InsPtr &ins61 = function.ins[61];
    std::string pa_ins_str61 = ins61->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str61 == "label@52: ");
    EXPECT_TRUE(ins61->Label() == "label@52");
    EXPECT_TRUE(ins61->IsLabel());
    // check ins[63]
    const pandasm::InsPtr &ins63 = function.ins[63];
    std::string pa_ins_str63 = ins63->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str63 == "label@53: ");
    EXPECT_TRUE(ins63->Label() == "label@53");
    EXPECT_TRUE(ins63->IsLabel());
    // check ins[66]
    const pandasm::InsPtr &ins66 = function.ins[66];
    std::string pa_ins_str66 = ins66->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str66 == "jeqz label@64");
    EXPECT_TRUE(ins66->Label() == "");
    EXPECT_FALSE(ins66->IsLabel());
    // check ins[75]
    const pandasm::InsPtr &ins75 = function.ins[75];
    std::string pa_ins_str75 = ins75->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str75 == "label@64: ");
    EXPECT_TRUE(ins75->Label() == "label@64");
    EXPECT_TRUE(ins75->IsLabel());
}

/**
 * @tc.name: abc2program_code_test_function_foo_part4
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: issueI989S6
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part4, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    // check ins[80]
    const pandasm::InsPtr &ins80 = function.ins[80];
    std::string pa_ins_str80 = ins80->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str80 == "jeqz label@71");
    EXPECT_TRUE(ins80->Label() == "");
    EXPECT_FALSE(ins80->IsLabel());
    // check ins[83]
    const pandasm::InsPtr &ins83 = function.ins[83];
    std::string pa_ins_str83 = ins83->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str83 == "label@71: ");
    EXPECT_TRUE(ins83->Label() == "label@71");
    EXPECT_TRUE(ins83->IsLabel());
    // check ins[84]
    const pandasm::InsPtr &ins84 = function.ins[84];
    std::string pa_ins_str84 = ins84->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str84 == "returnundefined");
    EXPECT_TRUE(ins84->Label() == "");
    EXPECT_FALSE(ins84->IsLabel());
    // check catch blocks
    constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_CATCH_BLOCKS = 3;
    EXPECT_TRUE(function.catch_blocks.size() == NUM_OF_CODE_TEST_UT_FOO_METHOD_CATCH_BLOCKS);
    // catch_blocks[0]
    const pandasm::Function::CatchBlock &pa_catch_block0 = function.catch_blocks[0];
    EXPECT_TRUE(pa_catch_block0.try_begin_label == "label@9");
    EXPECT_TRUE(pa_catch_block0.try_end_label == "label@11");
    EXPECT_TRUE(pa_catch_block0.catch_begin_label == "label@12");
    EXPECT_TRUE(pa_catch_block0.catch_end_label == "label@12");
    // catch_blocks[1]
    const pandasm::Function::CatchBlock &pa_catch_block1 = function.catch_blocks[1];
    EXPECT_TRUE(pa_catch_block1.try_begin_label == "label@3");
    EXPECT_TRUE(pa_catch_block1.try_end_label == "label@38");
    EXPECT_TRUE(pa_catch_block1.catch_begin_label == "label@38");
    EXPECT_TRUE(pa_catch_block1.catch_end_label == "label@38");
    // catch_blocks[2]
    const pandasm::Function::CatchBlock &pa_catch_block2 = function.catch_blocks[2];
    EXPECT_TRUE(pa_catch_block2.try_begin_label == "label@3");
    EXPECT_TRUE(pa_catch_block2.try_end_label == "label@47");
    EXPECT_TRUE(pa_catch_block2.catch_begin_label == "label@52");
    EXPECT_TRUE(pa_catch_block2.catch_end_label == "label@52");
}

/**
 * @tc.name: abc2program_code_test_function_goo
 * @tc.desc: get program fuction goo.
 * @tc.type: FUNC
 * @tc.require: issueI989S6
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_goo, TestSize.Level1)
{
    const pandasm::Function &function = *goo_function_;
    size_t regs_num = function.regs_num;
    constexpr uint32_t NUM_OF_CODE_TEST_UT_GOO_METHOD_INS = 1;
    EXPECT_TRUE(function.name == FUNC_NAME.goo);
    EXPECT_TRUE(function.ins.size() == NUM_OF_CODE_TEST_UT_GOO_METHOD_INS);
    // check ins[0]
    constexpr uint32_t INDEX_OF_FUNC_RETURNUNDEFINED = 0;
    const pandasm::InsPtr &ins1 = function.ins[INDEX_OF_FUNC_RETURNUNDEFINED];
    std::string pa_ins_str1 = ins1->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str1 == "returnundefined");
    EXPECT_TRUE(ins1->Label() == "");
    EXPECT_FALSE(ins1->IsLabel());
    // check catch blocks
    EXPECT_TRUE(function.catch_blocks.size() == 0);
}

/**
 * @tc.name: abc2program_code_imm_of_FLDAI
 * @tc.desc: get and check immediate number of opcode FLDAI.
 * @tc.type: FUNC
 * @tc.require: issueI9E5ZM
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_imm_of_FLDAI, TestSize.Level1)
{
    const pandasm::Function &hoo = *hoo_function_;
    EXPECT_EQ(hoo.ins.size(), INS_SIZE_OF_FUNCTION_HOO);
    pandasm::Ins *ins_fldai;
    for (auto &ins : hoo.ins) {
        if (ins->opcode == pandasm::Opcode::FLDAI) {
            ins_fldai = ins.get();
            break;
        }
    }
    EXPECT_TRUE(ins_fldai->opcode == pandasm::Opcode::FLDAI);
    // check imm of FLDAI
    EXPECT_EQ(ins_fldai->Imms().size(), IMMS_SIZE_OF_OPCODE_FLDAI);
    auto imm = ins_fldai->GetImm(0);
    auto p = std::get_if<double>(&(imm));
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(*p, 1.23);
}

/**
 * @tc.name: abc2program_module_literal_entry_test
 * @tc.desc: get and check number of modules.
 * @tc.type: FUNC
 * @tc.require: issueI9E5ZM
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_module_literal_entry_test, TestSize.Level1)
{
    auto &mod_table = prog_->literalarray_table;
    EXPECT_EQ(mod_table.size(), SIZE_OF_LITERAL_ARRAY_TABLE);
    auto &module_literals = mod_table.begin()->second.literals_;
    EXPECT_EQ(module_literals.size(), TOTAL_NUM_OF_MODULE_LITERALS);
    auto check_entry = [&module_literals](size_t idx, panda::panda_file::LiteralTag expect_tag, uint32_t expect_value) {
        auto *literal = &(module_literals[idx]);
        EXPECT_EQ(literal->tag_, expect_tag);
        auto p = std::get_if<uint32_t>(&literal->value_);
        EXPECT_TRUE(p != nullptr && *p == expect_value);
    };
    size_t idx = 0;
    // check ModuleRequests
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_MODULE_REQUESTS);
    // Each constant value '1' below stands for each entry. Each entry is also in the literal vector and shall be
    // considered while calculating the position (index) of next entry.
    // check RegularImportEntry
    idx += NUM_OF_MODULE_REQUESTS + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_REGULAR_IMPORTS);
    // check NamespaceImportEntry
    idx += NUM_OF_REGULAR_IMPORTS * LITERAL_NUM_OF_REGULAR_IMPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_NAMESPACE_IMPORTS);
    // check LocalExportEntry
    idx += NUM_OF_NAMESPACE_IMPORTS * LITERAL_NUM_OF_NAMESPACE_IMPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_LOCAL_EXPORTS);
    // check IndirectExportEntry
    idx += NUM_OF_LOCAL_EXPORTS * LITERAL_NUM_OF_LOCAL_EXPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_INDIRECT_EXPORTS);
    // check StarExportEntry
    idx += NUM_OF_INDIRECT_EXPORTS * LITERAL_NUM_OF_INDIRECT_EXPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_STAR_EXPORTS);
    // check idx
    idx += NUM_OF_STAR_EXPORTS * LITERAL_NUM_OF_STAR_EXPORT + 1;
    EXPECT_EQ(idx, TOTAL_NUM_OF_MODULE_LITERALS);
}

/**
 * @tc.name: abc2program_hello_world_test_nested_literal_array
 * @tc.desc: check contents of nested literal array
 * @tc.type: FUNC
 * @tc.require: issueIA7D9J
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_nested_literal_array, TestSize.Level1)
{
    auto &literal_array_table = prog_->literalarray_table;
    EXPECT_EQ(literal_array_table.size(), SIZE_OF_LITERAL_ARRAY_TABLE);
    /** current literal array table should be similar to below, each line indicates a literal array, which is
     *  formated as 'id_name : { literals }':
     *    _1 : { ... }                   // some other literal arrays
     *    _2 : { ... }
     *    _3 : { ... }                   // literal array that be nested, which id name is, for example, '_3'.
     *    _4 : { ..., literal_array:_3}  // target literal array we are checking.
     */
    panda::pandasm::LiteralArray nested_literal_array;
    for (auto &item : literal_array_table) {
        for (auto &literal : item.second.literals_) {
            if (literal.tag_ == panda_file::LiteralTag::LITERALARRAY) {
                nested_literal_array = item.second;
                break;
            }
        }
        if (nested_literal_array.literals_.size() != 0) {
            break;
        }
    }
    EXPECT_EQ(nested_literal_array.literals_.size(), TOTAL_NUM_OF_NESTED_LITERALS);
    auto it = nested_literal_array.literals_.begin();
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::STRING);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::METHOD);
    EXPECT_EQ(std::get<std::string>(it->value_), FUNC_NAME.nested_literal_array);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::METHODAFFILIATE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::INTEGER);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::LITERALARRAY);
    EXPECT_NE(literal_array_table.find(std::get<std::string>(it->value_)), literal_array_table.end());
}

/*------------------------------------- Cases of release mode above -------------------------------------*/
/*-------------------------------------- Cases of debug mode below --------------------------------------*/

/**
 * @tc.name: abc2program_hello_world_debug_test_dump
 * @tc.desc: check dump result in debug mode.
 * @tc.type: FUNC
 * @tc.require: IADG92
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_debug_test_dump, TestSize.Level1)
{
    driver_.Dump(HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME);
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateDumpResult(HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME,
                                                         HELLO_WORLD_DEBUG_DUMP_EXPECTED_FILE_NAME));
    if (REMOVE_DUMP_RESULT_FILES) {
        Abc2ProgramTestUtils::RemoveDumpResultFile(HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME);
    }
}

/**
 * @tc.name: abc2program_hello_world_test_source_file
 * @tc.desc: Verify the source file.
 * @tc.type: FUNC
 * @tc.require: issueI9CL5Z
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_source_file, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    std::string source_file = function.source_file;
    EXPECT_TRUE(source_file.find("HelloWorld.ts") != std::string::npos);
}

/**
 * @tc.name: abc2program_hello_world_test_source_code
 * @tc.desc: Verify the source code
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_source_code, TestSize.Level1)
{
    const pandasm::Function &function = *main_function_;
    std::string source_code = function.source_code;
    EXPECT_TRUE(source_code.find("not supported") != std::string::npos);
}

/**
 * @tc.name: abc2program_hello_world_test_local_variable
 * @tc.desc: get local variables
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_local_variable, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    EXPECT_FALSE(function.local_variable_debug.empty());
    EXPECT_TRUE(function.local_variable_debug[0].name.find("4funcObj") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].signature.find("any") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].signature_type.find("any") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].reg == 0);
    EXPECT_TRUE(function.local_variable_debug[0].start == 3);
    EXPECT_TRUE(function.local_variable_debug[0].length == 88);
}

/**
 * @tc.name: abc2program_hello_world_test_ins_debug
 * @tc.desc: get ins_debug line number and column number
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_ins_debug, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    EXPECT_FALSE(function.local_variable_debug.empty());

    const pandasm::InsPtr &ins0 = function.ins[0];
    std::string pa_ins_str0 = ins0->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str0 == "mov v0, a0");
    EXPECT_TRUE(ins0->ins_debug.line_number == -1);
    EXPECT_TRUE(ins0->ins_debug.column_number == -1);

    const pandasm::InsPtr &ins3 = function.ins[3];
    std::string pa_ins_str3 = ins3->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str3 == "ldundefined");
    EXPECT_TRUE(ins3->ins_debug.line_number == 40);
    EXPECT_TRUE(ins3->ins_debug.column_number == 2);

    const pandasm::InsPtr &ins6 = function.ins[6];
    std::string pa_ins_str6 = ins6->ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str6 == "ldai 0xb");
    EXPECT_TRUE(ins6->ins_debug.line_number == 41);
    EXPECT_TRUE(ins6->ins_debug.column_number == 11);
}

/**
 * @tc.name: abc2program_hello_world_test_open_abc_file
 * @tc.desc: open a non-existent abc file
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_open_abc_file, TestSize.Level1)
{
    std::string file_path = "invalid_file_path";
    EXPECT_FALSE(driver_.Compile(file_path));
}

/**
 * @tc.name: abc2program_hello_world_test_driver_run
 * @tc.desc: driver run different parameters
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_driver_run, TestSize.Level1)
{
    EXPECT_TRUE(driver_.Run(0, nullptr));
    int argc1 = 2;
    const char* args1[] = {"--param1", "--param2"};
    EXPECT_TRUE(driver_.Run(argc1, args1));
    int argc2 = 2;
    const char* args2[] = {"abc2program_options", "input_file_path"};
    EXPECT_TRUE(driver_.Run(argc2, args2));
    int argc3 = 3;
    const char* args3[] = {"abc2program_options", "input_file_path", "output_file_path"};
    EXPECT_TRUE(driver_.Run(argc3, args3));
    int argc4 = 2;
    const char* args4[] = {"abc2program_options", "--debug"};
    EXPECT_TRUE(driver_.Run(argc4, args4));
    int argc5 = 4;
    const char* args5[] = {"abc2program_options", "--debug", "--debug-file", "debug_file_path"};
    EXPECT_TRUE(driver_.Run(argc5, args5));
    int argc6 = 3;
    const char* args6[] = {"abc2program_options", HELLO_WORLD_DEBUG_ABC_TEST_FILE_NAME.c_str(),
                            HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME.c_str()};
    EXPECT_FALSE(driver_.Run(argc6, args6));
    if (REMOVE_DUMP_RESULT_FILES) {
        Abc2ProgramTestUtils::RemoveDumpResultFile(HELLO_WORLD_DEBUG_DUMP_RESULT_FILE_NAME);
    }
}

/**
 * @tc.name: abc2program_hello_world_test_Label
 * @tc.desc: label found in the map
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldDebugTest, abc2program_hello_world_test_Label, TestSize.Level1)
{
    LabelMap label_map = {{"first_label", "first_mapped_label"}, {"second_label", "second_mapped_label"}};
    std::string label1 = "second_label";
    std::string label2 = "third_label";
    PandasmDumperUtils test = PandasmDumperUtils();
    std::string result_found = test.GetMappedLabel(label1, label_map);
    std::string result_null = test.GetMappedLabel(label2, label_map);
    EXPECT_EQ(result_found, "second_mapped_label");
    EXPECT_EQ(result_null, "");
}

/*-------------------------------------- Cases of debug mode above --------------------------------------*/
/*-------------------------------------- Case of json file below ----------------------------------------*/

/**
 * @tc.name: abc2program_json_field_test
 * @tc.desc: get json field metadata
 * @tc.type: FUNC
 * @tc.require: issueIAJKTS
 */
HWTEST_F(Abc2ProgramJsonTest, abc2program_json_field_test, TestSize.Level1)
{
    auto it = prog_->record_table.find(JSON_TEST_FILE_NAME);
    ASSERT(it != prog_->record_table.end());
    const pandasm::Record &record = it->second;
    const std::vector<pandasm::Field> &field_list = record.field_list;
    EXPECT_EQ(field_list.size(), 1);
    const pandasm::Field &field = field_list[0];
    EXPECT_EQ(field.name, JSON_FILE_CONTENT);
    std::optional<pandasm::ScalarValue> val = field.metadata->GetValue();
    EXPECT_TRUE(val.has_value());
    auto content = val.value().GetValue<std::string>();
    EXPECT_EQ(content, "{\n  \"name\" : \"Import json\"\n}");
}

/**
 * @tc.name: abc2program_annotations_test_Record
 * @tc.desc: record fields compare
 * @tc.type: FUNC
 * @tc.require: IARNAU
 */
HWTEST_F(Abc2ProgramAnnotationTest, abc2program_annotations_test_Record, TestSize.Level1)
{
    const auto &fields = anno_record_->field_list;
    for (auto& field : fields) {
        EXPECT_TRUE(field.name == "a" || field.name == "b" || field.name == "c" || field.name == "d" ||
                    field.name == "e" || field.name == "f");
        if (field.name == "a") {
            EXPECT_EQ(field.type.GetName(), "f64");
            EXPECT_EQ(field.metadata->GetValue()->GetValue<double>(), 7);
        }
        if (field.name == "b") {
            EXPECT_EQ(field.type.GetName(), "f64[]");
            auto &litarr = prog_->literalarray_table.at(field.metadata->GetValue()->GetValue<std::string>());
            EXPECT_EQ(litarr.literals_.size(), 4);
        }
        if (field.name == "c") {
            EXPECT_EQ(field.type.GetName(), "panda.String");
            EXPECT_EQ(field.metadata->GetValue()->GetValue<std::string>(), "abc");
        }
        if (field.name == "d") {
            EXPECT_EQ(field.type.GetName(), "u1");
            EXPECT_EQ(field.metadata->GetValue()->GetValue<bool>(), false);
        }
        if (field.name == "e") {
            EXPECT_EQ(field.type.GetName(), "f64[]");
            auto &litarr = prog_->literalarray_table.at(field.metadata->GetValue()->GetValue<std::string>());
            EXPECT_EQ(litarr.literals_.size(), 6);
        }
        if (field.name == "f") {
            EXPECT_EQ(field.type.GetName(), "f64");
            EXPECT_EQ(field.metadata->GetValue()->GetValue<double>(), 2);
        }
    }
}

/**
 * @tc.name: abc2program_annotations_test_function_annotation
 * @tc.desc: check if annotated function has annotation
 * @tc.type: FUNC
 * @tc.require: IARNAU
 */
HWTEST_F(Abc2ProgramAnnotationTest, abc2program_annotations_test_function_annotation, TestSize.Level1)
{
    auto &annotations = a_function_->metadata->GetAnnotations();
    auto iter = std::find_if(annotations.begin(), annotations.end(), [](const pandasm::AnnotationData &anno) {
        return anno.GetName() == "Anno";
    });
    EXPECT_NE(iter, annotations.end());
    EXPECT_EQ(iter->GetName(), "Anno");
}

/**
 * @tc.name: abc2program_annotations_test_function_annotation_fields
 * @tc.desc: annotation of function fields comparison
 * @tc.type: FUNC
 * @tc.require: IARNAU
 */
HWTEST_F(Abc2ProgramAnnotationTest, abc2program_annotations_test_function_annotation_fields, TestSize.Level1)
{
    auto &annotations = b_function_->metadata->GetAnnotations();
    auto anno = std::find_if(annotations.begin(), annotations.end(), [](const pandasm::AnnotationData &anno) {
        return anno.GetName() == "Anno";
    });
    EXPECT_NE(anno, annotations.end());
    EXPECT_EQ(anno->GetName(), "Anno");
    const auto &elements = anno->GetElements();
    for (auto& elem : elements) {
        EXPECT_TRUE(elem.GetName() == "a" || elem.GetName() == "b" || elem.GetName() == "c" || elem.GetName() == "d" ||
                    elem.GetName() == "e" || elem.GetName() == "f");
        if (elem.GetName() == "a") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::F64);
            EXPECT_EQ(elem.GetValue()->GetAsScalar()->GetValue<double>(), 5);
        }
        if (elem.GetName() == "b") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::LITERALARRAY);
            auto &litarr = prog_->literalarray_table.at(elem.GetValue()->GetAsScalar()->GetValue<std::string>());
            EXPECT_EQ(litarr.literals_.size(), 6);
        }
        if (elem.GetName() == "c") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::STRING);
            EXPECT_EQ(elem.GetValue()->GetAsScalar()->GetValue<std::string>(), "def");
        }
        if (elem.GetName() == "d") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::U1);
            EXPECT_EQ(elem.GetValue()->GetAsScalar()->GetValue<bool>(), true);
        }
        if (elem.GetName() == "e") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::LITERALARRAY);
            auto &litarr = prog_->literalarray_table.at(elem.GetValue()->GetAsScalar()->GetValue<std::string>());
            EXPECT_EQ(litarr.literals_.size(), 4);
        }
        if (elem.GetName() == "f") {
            EXPECT_EQ(elem.GetValue()->GetType(), pandasm::Value::Type::F64);
            EXPECT_EQ(elem.GetValue()->GetAsScalar()->GetValue<double>(), 3);
        }
    }
}

};  // panda::abc2program