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
#include "disassembler.h"
#include "file.h"
#include "utils.h"

using namespace testing::ext;

namespace panda::disasm {
class DisassemblerModuleLiteralTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    bool ValidateModuleLiteral(const std::vector<std::string> &module_literals,
                               const std::vector<std::string> &expected_module_literals)
    {
        for (const auto &expected_module_literal : expected_module_literals) {
            const auto module_iter = std::find(module_literals.begin(), module_literals.end(), expected_module_literal);
            if (module_iter == module_literals.end()) {
                return false;
            }
        }

        return true;
    }
};

/**
* @tc.name: disassembler_module_literal_test_001
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-regular-import.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-export-file.js,\n\t}",
        "\tModuleTag: REGULAR_IMPORT, local_name: a, import_name: a, module_request: ./module-export-file.js"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
* @tc.name: disassembler_module_literal_test_002
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_002, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-namespace-import.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-export-file.js,\n\t}",
        "\tModuleTag: NAMESPACE_IMPORT, local_name: ns, module_request: ./module-export-file.js"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
* @tc.name: disassembler_module_literal_test_003
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_003, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-local-export.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t}",
        "\tModuleTag: LOCAL_EXPORT, local_name: c, export_name: c"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
* @tc.name: disassembler_module_literal_test_004
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_004, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-indirect-export.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-import-file.js,\n\t}",
        "\tModuleTag: INDIRECT_EXPORT, export_name: a, import_name: a, module_request: ./module-import-file.js"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
* @tc.name: disassembler_module_literal_test_005
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_005, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-start-export.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-import-file.js,\n\t}",
        "\tModuleTag: STAR_EXPORT, module_request: ./module-import-file.js"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
* @tc.name: disassembler_module_literal_test_005
* @tc.desc: get module literal of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_006, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "module-regular-import-local-export.abc";
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-indirect-export.js,\n\t}",
        "\tModuleTag: REGULAR_IMPORT, local_name: a, import_name: a, module_request: ./module-indirect-export.js",
        "\tModuleTag: LOCAL_EXPORT, local_name: b, export_name: b"
    };
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_TRUE(has_module_literal);
}

/**
 * @tc.name: disassembler_module_literal_test_007
 * @tc.desc: test module reuqire index.
 * @tc.type: FUNC
 * @tc.require: file path
 */
HWTEST_F(DisassemblerModuleLiteralTest, disassembler_module_literal_test_007, TestSize.Level1)
{
    std::vector<std::string> expected_module_literals = {
        "\tMODULE_REQUEST_ARRAY: {\n\t\t0 : ./module-export-file.js,\n\t}",
        "\tModuleTag: REGULAR_IMPORT, local_name: a, import_name: a, module_request: ./module-export-file.js"};

    const std::string file_name = GRAPH_TEST_ABC_DIR "module-regular-import.abc";
    std::ifstream baseFile(file_name, std::ios::binary);
    EXPECT_TRUE(baseFile.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(baseFile), {});
    // change module request idx
    buffer[680] = 0xff;
    buffer[681] = 0xff;

    const std::string targetFileName = GRAPH_TEST_ABC_DIR "module-regular-import-001.abc";
    GenerateModifiedAbc(buffer, targetFileName);
    baseFile.close();

    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(targetFileName, false, false);
    std::vector<std::string> module_literals = disasm.GetModuleLiterals();
    bool has_module_literal = ValidateModuleLiteral(module_literals, expected_module_literals);
    EXPECT_FALSE(has_module_literal);
}
}
