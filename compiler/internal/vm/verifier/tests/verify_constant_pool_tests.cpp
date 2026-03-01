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

#include "verifier.h"

#include <algorithm>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "file.h"
#include "utils.h"

using namespace testing::ext;

namespace panda::verifier {
class VerifierConstantPool : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
    // The known method id in the abc file
    std::vector<uint8_t> method_id_in_test_constant_pool = {0x0e, 0x00};
    // The known string id in the abc file
    std::vector<uint8_t> string_id_in_test_constant_pool = {0x0c, 0x00};
    // The known literal id in the abc file
    std::vector<uint8_t> literal_id_in_test_constant_pool = {0x0f, 0x00};
    // The known jump instruction in the abc file
    std::vector<uint8_t> jump_ins_id_in_test_constant_pool = {0x4f, 0x0c};

    void TestVerifierWithAbcVersionTampered(const std::string &file_name,
                                            std::array<uint8_t, panda_file::File::VERSION_SIZE> &original_version)
    {
        panda::verifier::Verifier ver {file_name};

        ver.CollectIdInfos();
        if (panda_file::IsVersionLessOrEqual(original_version, panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION)) {
            EXPECT_TRUE(ver.VerifyConstantPoolIndex());
            EXPECT_TRUE(ver.VerifyConstantPool());
        } else {
            EXPECT_FALSE(ver.VerifyConstantPoolIndex());
            EXPECT_FALSE(ver.VerifyConstantPool());
        }
    }
};

/**
* @tc.name: verifier_constant_pool_001
* @tc.desc: Verify abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    panda::verifier::Verifier ver {file_name};
    ver.CollectIdInfos();
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.Verify());
}

/**
* @tc.name: verifier_constant_pool_002
* @tc.desc: Verify the method id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_002, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolIndex());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_method_id = string_id_in_test_constant_pool;
    std::vector<uint8_t> method_id = method_id_in_test_constant_pool;

    for (size_t i = buffer.size() - 1; i >= 0; --i) {
        if (buffer[i] == method_id[0] && buffer[i + 1] == method_id[1]) {
            buffer[i] = static_cast<unsigned char>(new_method_id[0]);
            buffer[i + 1] = static_cast<unsigned char>(new_method_id[1]);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_002.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.include_literal_array_ids = false;
        ver.CollectIdInfos();
        ver.literal_ids_ = literal_ids;
        EXPECT_FALSE(ver.VerifyConstantPoolIndex());
    }

    std::ifstream base_file_02(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file_02.is_open());
    std::vector<unsigned char> buffer_0(std::istreambuf_iterator<char>(base_file_02), {});
    std::array<uint8_t, panda_file::File::VERSION_SIZE> version {buffer_0[12], buffer_0[13],
                                                                 buffer_0[14], buffer_0[15]};
    buffer_0[12] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[0]);
    buffer_0[13] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[1]);
    buffer_0[14] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[2]);
    buffer_0[15] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[3]);
    const std::string target_file_name_02 = GRAPH_TEST_ABC_DIR "verifier_constant_pool_002_2.abc";
    GenerateModifiedAbc(buffer_0, target_file_name_02);
    base_file_02.close();

    TestVerifierWithAbcVersionTampered(target_file_name_02, version);
}

/**
* @tc.name: verifier_constant_pool_003
* @tc.desc: Verify the literal id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_003, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolIndex());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_literal_id = method_id_in_test_constant_pool;
    std::vector<uint8_t> literal_id = literal_id_in_test_constant_pool;

    for (size_t i = 0; i < buffer.size(); ++i) {
        if (buffer[i] == literal_id[0] && buffer[i + 1] == literal_id[1]) {
            buffer[i] = static_cast<unsigned char>(new_literal_id[0]);
            buffer[i + 1] = static_cast<unsigned char>(new_literal_id[1]);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_003.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.include_literal_array_ids = false;
        ver.CollectIdInfos();
        ver.literal_ids_ = literal_ids;
        EXPECT_FALSE(ver.VerifyConstantPoolIndex());
    }

    std::ifstream base_file_02(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file_02.is_open());
    std::vector<unsigned char> buffer_0(std::istreambuf_iterator<char>(base_file_02), {});
    std::array<uint8_t, panda_file::File::VERSION_SIZE> version {buffer_0[12], buffer_0[13],
                                                                 buffer_0[14], buffer_0[15]};
    buffer_0[12] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[0]);
    buffer_0[13] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[1]);
    buffer_0[14] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[2]);
    buffer_0[15] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[3]);
    const std::string target_file_name_02 = GRAPH_TEST_ABC_DIR "verifier_constant_pool_003_2.abc";
    GenerateModifiedAbc(buffer_0, target_file_name_02);
    base_file_02.close();

    TestVerifierWithAbcVersionTampered(target_file_name_02, version);
}

/**
* @tc.name: verifier_constant_pool_004
* @tc.desc: Verify the string id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_004, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolIndex());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_string_id = literal_id_in_test_constant_pool;
    std::vector<uint8_t> string_id = string_id_in_test_constant_pool;

    for (size_t i = sizeof(panda_file::File::Header); i < buffer.size(); ++i) {
        if (buffer[i] == string_id[0] && buffer[i + 1] == string_id[1]) {
            buffer[i] = static_cast<unsigned char>(new_string_id[0]);
            buffer[i + 1] = static_cast<unsigned char>(new_string_id[1]);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_004.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.include_literal_array_ids = false;
        ver.CollectIdInfos();
        ver.literal_ids_ = literal_ids;
        EXPECT_FALSE(ver.VerifyConstantPoolIndex());
    }

    std::ifstream base_file_02(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file_02.is_open());
    std::vector<unsigned char> buffer_0(std::istreambuf_iterator<char>(base_file_02), {});
    std::array<uint8_t, panda_file::File::VERSION_SIZE> version {buffer_0[12], buffer_0[13],
                                                                 buffer_0[14], buffer_0[15]};
    buffer_0[12] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[0]);
    buffer_0[13] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[1]);
    buffer_0[14] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[2]);
    buffer_0[15] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[3]);
    const std::string target_file_name_02 = GRAPH_TEST_ABC_DIR "verifier_constant_pool_004_2.abc";
    GenerateModifiedAbc(buffer_0, target_file_name_02);
    base_file_02.close();

    TestVerifierWithAbcVersionTampered(target_file_name_02, version);
}

/**
* @tc.name: verifier_constant_pool_006
* @tc.desc: Verify the format of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_006, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char new_opcode = 0xff;
    std::vector<unsigned char> opcode_imm8 =
        jump_ins_id_in_test_constant_pool;
    for (size_t i = 0; i < buffer.size() - opcode_imm8.size(); ++i) {
        if (buffer[i] == opcode_imm8[0] && buffer[i + 1] == opcode_imm8[1]) {
            buffer[i] = new_opcode;
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_006.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.include_literal_array_ids = false;
        ver.CollectIdInfos();
        ver.literal_ids_ = literal_ids;
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }

    std::ifstream base_file_02(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file_02.is_open());
    std::vector<unsigned char> buffer_0(std::istreambuf_iterator<char>(base_file_02), {});
    std::array<uint8_t, panda_file::File::VERSION_SIZE> version {buffer_0[12], buffer_0[13],
                                                                 buffer_0[14], buffer_0[15]};
    buffer_0[12] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[0]);
    buffer_0[13] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[1]);
    buffer_0[14] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[2]);
    buffer_0[15] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[3]);
    const std::string target_file_name_02 = GRAPH_TEST_ABC_DIR "verifier_constant_pool_006_2.abc";
    GenerateModifiedAbc(buffer_0, target_file_name_02);
    base_file_02.close();

    TestVerifierWithAbcVersionTampered(target_file_name_02, version);
}

/**
* @tc.name: verifier_constant_pool_007
* @tc.desc: Verify the jump instruction of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_007, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char new_imm8 = 0x4f;
    std::vector<unsigned char> opcode_imm8 = jump_ins_id_in_test_constant_pool;
    for (size_t i = 0; i < buffer.size() - opcode_imm8.size(); ++i) {
        if (buffer[i] == opcode_imm8[0] && buffer[i + 1] == opcode_imm8[1]) {
            buffer[i + 1] = new_imm8;
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_007.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
* @tc.name: verifier_constant_pool_008
* @tc.desc: Verify the literal tag of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_008, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<uint32_t> literal_ids;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        std::copy(ver.literal_ids_.begin(), ver.literal_ids_.end(), std::back_inserter(literal_ids));
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char invalid_tag = 0x5c; // a invalid tag

    for (const auto &literal_id : literal_ids) {
        size_t tag_off = static_cast<size_t>(literal_id) + sizeof(uint32_t);
        buffer[tag_off] = invalid_tag;
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_008.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.include_literal_array_ids = false;
        ver.CollectIdInfos();
        ver.literal_ids_ = literal_ids;
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }

    std::ifstream base_file_02(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file_02.is_open());
    std::vector<unsigned char> buffer_0(std::istreambuf_iterator<char>(base_file_02), {});
    std::array<uint8_t, panda_file::File::VERSION_SIZE> version {buffer_0[12], buffer_0[13],
                                                                 buffer_0[14], buffer_0[15]};
    buffer_0[12] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[0]);
    buffer_0[13] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[1]);
    buffer_0[14] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[2]);
    buffer_0[15] = static_cast<unsigned char>(panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION[3]);
    const std::string target_file_name_02 = GRAPH_TEST_ABC_DIR "verifier_constant_pool_008_2.abc";
    GenerateModifiedAbc(buffer_0, target_file_name_02);
    base_file_02.close();

    {
        panda::verifier::Verifier ver {target_file_name_02};
        ver.CollectIdInfos();
        if (panda_file::IsVersionLessOrEqual(version, panda_file::LAST_CONTAINS_LITERAL_IN_HEADER_VERSION)) {
            EXPECT_TRUE(ver.VerifyConstantPoolIndex());
        } else {
            EXPECT_FALSE(ver.VerifyConstantPoolIndex());
        }
    }
}

/**
* @tc.name: verifier_constant_pool_010
* @tc.desc: Verify the literal id in the literal array of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_010, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_literal_array.abc";
    std::unordered_map<uint32_t, uint32_t> inner_literal_map;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        inner_literal_map.insert(ver.inner_literal_map_.begin(), ver.inner_literal_map_.end());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    ModifyBuffer(inner_literal_map, buffer);

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_010.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
}

/**
* @tc.name: verifier_constant_pool_011
* @tc.desc: Verify the method id in the literal array of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_011, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_literal_array.abc";
    std::unordered_map<uint32_t, uint32_t> inner_method_map;
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        inner_method_map.insert(ver.inner_method_map_.begin(), ver.inner_method_map_.end());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    ModifyBuffer(inner_method_map, buffer);

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_011.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
* @tc.name: verifier_null_file
* @tc.desc: verify not-exist abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_not_exist, TestSize.Level1)
{
    const std::string not_exist_file_name = GRAPH_TEST_ABC_DIR "test_not_exist_file.abc";
    panda::verifier::Verifier ver {not_exist_file_name};
    ver.CollectIdInfos();
    EXPECT_FALSE(ver.VerifyConstantPool());
    EXPECT_FALSE(ver.VerifyConstantPoolIndex());
    EXPECT_FALSE(ver.VerifyConstantPoolContent());
    EXPECT_FALSE(ver.Verify());
}

/**
 * @tc.name: verifier_constant_pool_012
 * @tc.desc: Verify the try blocks in the abc file.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(VerifierConstantPool, verifier_constant_pool_012, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char try_start_pc = 0x01; // The known try.start_pc in the abc file
    unsigned char try_length = 0x0f;   // The known try.length in the abc file
    unsigned char code_size = 0x35;    // The known code_size of the try block in the abc file

    for (size_t i = 0; i < buffer.size() - 1; ++i) {
        if (buffer[i] == try_start_pc && buffer[i + 1] == try_length) {
            auto invalid_try_start_pc = try_start_pc + code_size;
            buffer[i] = static_cast<unsigned char>(invalid_try_start_pc);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_012.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
 * @tc.name: verifier_constant_pool_013
 * @tc.desc: Verify the catch blocks in the abc file.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(VerifierConstantPool, verifier_constant_pool_013, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char catch_type_idx = 0x00;      // The known catch.type_idx in the abc file
    unsigned char catch_handler_pc = 0x12;    // The known catch.handler_pc in the abc file
    unsigned char code_size = 0x00;           // The known code_size of the catch block in the abc file
    unsigned char method_code_size = 0x35;    // The known code_size of the method in the abc file

    for (size_t i = 0; i < buffer.size() - 1; ++i) {
        if (buffer[i] == catch_type_idx && buffer[i + 1] == catch_handler_pc && buffer[i + 2] == code_size) {
            auto invalid_catch_handler_pc = catch_handler_pc + method_code_size;
            buffer[i + 1] = static_cast<unsigned char>(invalid_catch_handler_pc);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_013.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
 * @tc.name: verifier_constant_pool_014
 * @tc.desc: Verify the code size in the abc file.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(VerifierConstantPool, verifier_constant_pool_014, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    unsigned char tries_size = 0x00;           // The known tries_size of the method in the abc file
    unsigned char method_code_size = 0x2D;     // The known code_size of the method in the abc file

    for (size_t i = 0; i < buffer.size() - 1; ++i) {
        if (buffer[i] == method_code_size && buffer[i + 1] == tries_size) {
            auto invalid_code_size = 0;
            buffer[i] = static_cast<unsigned char>(invalid_code_size);
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_014.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
 * @tc.name: verifier_constant_pool_015
 * @tc.desc: Verify the literal tag is double value  in the abc file.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(VerifierConstantPool, verifier_constant_pool_015, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<unsigned char> double_literal_value = {0xae, 0x47,
                                                       0xe1, 0x7a,
                                                       0x14, 0xae,
                                                       0x28, 0x40}; // The known double literal
    std::vector<unsigned char> invalid_double_literal_value = {0x00, 0x00,
                                                               0x00, 0x00,
                                                               0x00, 0x00,
                                                               0xff, 0xff}; // a invalid double tag

    for (size_t i = 0; i <= buffer.size() - double_literal_value.size(); ++i) {
        if (buffer[i] == double_literal_value[0] && buffer[i + 1] == double_literal_value[1]) {
            for (size_t j = 0; j < invalid_double_literal_value.size(); ++j) {
                buffer[i + j] = invalid_double_literal_value[j];
            }
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_015.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

/**
 * @tc.name: verifier_constant_pool_016
 * @tc.desc: Verify the jump in the abc file.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(VerifierConstantPool, verifier_constant_pool_016, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<unsigned char> opcode_imm8 = {0x4f, 0x13}; // The known jump instruction in the abc file

    for (size_t i = 0; i < buffer.size() - opcode_imm8.size(); ++i) {
        if (buffer[i] == opcode_imm8[0] && buffer[i + 1] == opcode_imm8[1]) {
            buffer[i + 1] = opcode_imm8[1] + 1;   // jump instruction middle
            break;
        }
    }

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_constant_pool_016.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyConstantPoolContent());
    }
}

}; // namespace panda::verifier
