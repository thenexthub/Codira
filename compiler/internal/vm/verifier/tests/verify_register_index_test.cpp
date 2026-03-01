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
#include "utils.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>
#include <fstream>

#include "file.h"
#include "utils/logger.h"
#include "code_data_accessor-inl.h"
#include "method_data_accessor-inl.h"

using namespace testing::ext;

namespace panda::verifier {
class VerifierRegisterTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: verifier_test_001
* @tc.desc: Verify the abc file register index function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierRegisterTest, verifier_register_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_register_index.abc";
    panda::verifier::Verifier ver {file_name};
    ver.CollectIdInfos();
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.Verify());
}

/**
* @tc.name: verifier_test_002
* @tc.desc: Verify the modified abc file register index function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierRegisterTest, verifier_register_002, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_register_index.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyRegisterIndex());
    }
    // the new register index in the abc file
    const uint8_t new_reg_id = 0x09;

    std::ifstream base_file(base_file_name, std::ios::binary);
    if (!base_file.is_open()) {
        LOG(ERROR, VERIFIER) << "Failed to open file " << base_file_name;
        EXPECT_TRUE(base_file.is_open());
    }
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});
    // the known instruction which contains register index in the abc file
    const std::vector<uint8_t> op_code = {0x60, 0x03};

    for (size_t i = 0; i < buffer.size() - 2; i++) {
        if (buffer[i] == op_code[0] && buffer[i+1] == op_code[1]) {
            buffer[i + 1] = static_cast<unsigned char>(new_reg_id);
        }
    }

    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_register_002.abc";
    GenerateModifiedAbc(buffer, tar_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {tar_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyRegisterIndex());
    }
}

/**
* @tc.name: verifier_test_003
* @tc.desc: Verify the range register index of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierRegisterTest, verifier_register_003, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_register_index.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        ver.CollectIdInfos();
        EXPECT_TRUE(ver.VerifyRegisterIndex());
    }
    // the new range register number
    const uint8_t new_range_reg_num = 0x0c;

    std::ifstream base_file(base_file_name, std::ios::binary);
    if (!base_file.is_open()) {
        LOG(ERROR, VERIFIER) << "Failed to open file " << base_file_name;
        EXPECT_TRUE(base_file.is_open());
    }
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});
    // the known range instruction in the abc file
    const std::vector<uint8_t> inst = {0x08, 0x09, 0x03, 0x03};

    for (size_t i = 0; i < buffer.size() - 4; i++) {
        if (buffer[i] == inst[0] && buffer[i+1] == inst[1] &&
            buffer[i+2] == inst[2] && buffer[i+3] == inst[3]) {
            buffer[i + 2] = static_cast<unsigned char>(new_range_reg_num);
        }
    }

    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_register_003.abc";
    GenerateModifiedAbc(buffer, tar_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {tar_file_name};
        ver.CollectIdInfos();
        EXPECT_FALSE(ver.VerifyRegisterIndex());
    }
}
} // namespace panda::verifier
