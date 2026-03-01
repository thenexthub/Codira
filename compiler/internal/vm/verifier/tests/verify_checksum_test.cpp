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

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

#include "file.h"
#include "utils.h"

using namespace testing::ext;

namespace panda::verifier {
class VerifierTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: verifier_checksum_001
* @tc.desc: Verify the abc file checksum value function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierTest, verifier_checksum_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    panda::verifier::Verifier ver {file_name};
    EXPECT_TRUE(ver.VerifyChecksum());
}

/**
* @tc.name: verifier_checksum_002
* @tc.desc: Verify the modified abc file checksum value function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierTest, verifier_checksum_002, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        EXPECT_TRUE(ver.VerifyChecksum());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_checksum = {0x01, 0x01, 0x01, 0x01};

    // The 8~11 elements in the buffer of the abc file hold the checksum
    buffer[8] = new_checksum[0];
    buffer[9] = new_checksum[1];
    buffer[10] = new_checksum[2];
    buffer[11] = new_checksum[3];

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_checksum_002.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        EXPECT_FALSE(ver.VerifyChecksum());
    }
}

/**
* @tc.name: verifier_checksum_003
* @tc.desc: Verify the modified abc file content function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierTest, verifier_checksum_003, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    {
        panda::verifier::Verifier ver {base_file_name};
        EXPECT_TRUE(ver.VerifyChecksum());
    }
    std::ifstream base_file(base_file_name, std::ios::binary);
    EXPECT_TRUE(base_file.is_open());

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_content = {0x01, 0x01};

    // The checksum calculation starts with the 12th element
    buffer[12] = new_content[0];
    buffer[13] = new_content[1];

    const std::string target_file_name = GRAPH_TEST_ABC_DIR "verifier_checksum_003.abc";
    GenerateModifiedAbc(buffer, target_file_name);
    base_file.close();

    {
        panda::verifier::Verifier ver {target_file_name};
        EXPECT_FALSE(ver.VerifyChecksum());
        EXPECT_FALSE(ver.Verify());
    }
}

}; // namespace panda::verifier
