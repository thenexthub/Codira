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

using namespace testing::ext;

namespace panda::disasm {
static const std::string ETS_IMPLEMENTS_TAG = "ets_implements";
static const std::string ETS_IMPLEMENTS_VALUE_EXPECTED =
    "L<packagename>/src/main/ets/<filepath>/I1;,L<packagename>/src/main/ets/<filepath>/I2;";

class DisassemblerEtsImplementsLiteralTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    bool Find(std::stringstream &ss, const std::string &dst)
    {
        return ss.str().find(dst) != std::string::npos;
    }
};

/**
* @tc.name: disassembler_ets_implements_literal_test_001
* @tc.desc: get and check ets_implements in literal array of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerEtsImplementsLiteralTest, disassembler_ets_implements_literal_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "etsImplements.abc";

    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, true, true);
    std::stringstream ss {};
    disasm.Serialize(ss);
    EXPECT_TRUE(Find(ss, ETS_IMPLEMENTS_TAG));
    EXPECT_TRUE(Find(ss, ETS_IMPLEMENTS_VALUE_EXPECTED));
}
}  // namespace panda::disasm
