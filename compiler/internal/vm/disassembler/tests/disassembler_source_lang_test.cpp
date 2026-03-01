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
#include <iostream>

using namespace testing::ext;

namespace panda::disasm {

static const std::string CHECK_ECMASCRIPT = ".language ECMAScript\n.record default {";
static const std::string CHECK_JAVASCRIPT = ".language JavaScript\n.record js {";
static const std::string CHECK_ARKTSSCRIPT = ".language ArkTS\n.record ets {";
static const std::string CHECK_TYPESCRIPT = ".language TypeScript\n.record ts {";

static const std::string CHECK_ETS_STR_ANNO = ".language ArkTS\n.record ets.EtsStringAnno {";
static const std::string CHECK_TS_ANNO = ".language TypeScript\n.record ts.TsAnno {";
static const std::string CHECK_PANDA_STR = ".language ECMAScript\n.record panda.String <external>";

class DisasmSourceLangTest : public testing::Test {
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
* @tc.name: disassembler_source_lang_test_001
* @tc.desc: Check source files types in abc.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmSourceLangTest, disassembler_source_lang_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "sourceLang.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    std::stringstream ss {};
    disasm.Serialize(ss);
    EXPECT_TRUE(Find(ss, CHECK_ECMASCRIPT));
    EXPECT_TRUE(Find(ss, CHECK_JAVASCRIPT));
    EXPECT_TRUE(Find(ss, CHECK_ARKTSSCRIPT));
    EXPECT_TRUE(Find(ss, CHECK_TYPESCRIPT));

    EXPECT_TRUE(Find(ss, CHECK_ETS_STR_ANNO));
    EXPECT_TRUE(Find(ss, CHECK_TS_ANNO));
    EXPECT_TRUE(Find(ss, CHECK_PANDA_STR));
}
}
