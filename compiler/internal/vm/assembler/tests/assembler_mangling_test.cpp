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

#include "mangling.h"
#include "assembly-function.h"
#include "extensions/extensions.h"

using namespace panda::pandasm;
using namespace testing::ext;

namespace panda::pandasm {
class ManglingTest : public testing::Test {
};

/**
 * @tc.name: mangling_test_001
 * @tc.desc: Verify the MangleFunctionName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ManglingTest, mangling_test_001, TestSize.Level1)
{
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"type1", 0}, language);
    params.emplace_back(Type {"type2", 0}, language);
    params.emplace_back(Type {"type3", 0}, language);

    auto return_type = Type("type4", 0);

    std::string name = "Asm.main";
    std::string ret = MangleFunctionName(name, std::move(params), return_type);
    EXPECT_EQ(ret, "Asm.main:type1;type2;type3;type4;");
}

/**
 * @tc.name: mangling_test_002
 * @tc.desc: Verify the DeMangleName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ManglingTest, mangling_test_002, TestSize.Level1)
{
    std::string name = "Asm.main:type1;type2;type3;type4;";
    std::string ret = DeMangleName(name);
    EXPECT_EQ(ret, "Asm.main");
}

/**
 * @tc.name: mangling_test_003
 * @tc.desc: Verify the GetFunctionSignatureFromName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ManglingTest, mangling_test_003, TestSize.Level1)
{
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"type1", 0}, language);
    params.emplace_back(Type {"type2", 0}, language);
    params.emplace_back(Type {"type3", 0}, language);

    std::string name = "Asm.main";
    std::string ret = GetFunctionSignatureFromName(name, std::move(params));
    EXPECT_EQ(ret, "Asm.main:(type1,type2,type3)");
}

/**
 * @tc.name: mangling_test_004
 * @tc.desc: Verify the GetFunctionNameFromSignature function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(ManglingTest, mangling_test_004, TestSize.Level1)
{
    std::string name = "Asm.main:(type1,type2,type3,type4)";
    std::string ret = GetFunctionNameFromSignature(name);
    IsSignatureOrMangled(ret);
    EXPECT_EQ(ret, "Asm.main");
}
}
