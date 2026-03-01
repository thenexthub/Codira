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
const std::string ETSIMPLEMENTS_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "etsImplements.abc";
const std::string ETS_IMPLEMENTS_MESSAGE =
    "L<packagename>/src/main/ets/<filepath>/I1;,L<packagename>/src/main/ets/<filepath>/I2;";
constexpr uint8_t SIZE_OF_LITERAL_ARRAY_TABLE = 3;
constexpr uint8_t TOTAL_NUM_OF_ETSIMPLEMENTS_LITERALS = 10;

class Abc2ProgramEtsImplementsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(ETSIMPLEMENTS_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

/**
 * @tc.name: abc2program_ets_implements_test_literal_array
 * @tc.desc: get and check the ets implements in literal array
 * @tc.type: FUNC
 * @tc.require: IC9M4H
 */
HWTEST_F(Abc2ProgramEtsImplementsTest, abc2program_ets_implements_test_literal_array, TestSize.Level1)
{
    auto &literal_array_table = prog_->literalarray_table;
    EXPECT_EQ(literal_array_table.size(), SIZE_OF_LITERAL_ARRAY_TABLE);
    panda::pandasm::LiteralArray ets_implements_literal_array;

    for (auto &item : literal_array_table) {
        for (auto &literal : item.second.literals_) {
            if (literal.tag_ == panda_file::LiteralTag::ETS_IMPLEMENTS) {
                ets_implements_literal_array = item.second;
                break;
            }
        }
        if (ets_implements_literal_array.literals_.size() != 0) {
            break;
        }
    }

    EXPECT_EQ(ets_implements_literal_array.literals_.size(), TOTAL_NUM_OF_ETSIMPLEMENTS_LITERALS);
    auto it = ets_implements_literal_array.literals_.begin();
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::STRING);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::TAGVALUE);
    ++it;
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::METHOD);
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
    EXPECT_EQ(it->tag_, panda_file::LiteralTag::ETS_IMPLEMENTS);
    EXPECT_EQ(std::get<std::string>(it->value_), ETS_IMPLEMENTS_MESSAGE);
}
}; // panda::abc2program
