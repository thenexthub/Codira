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

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "api_modifier.h"

namespace libabckit::test {

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestParameterCheck)
{
    static constexpr auto VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
    static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "scenarios/parameter_check/dynamic/parameter_check.abc";
    static constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "scenarios/parameter_check/dynamic/parameter_check_modified.abc";
    const std::string entryPoint = "parameter_check";

    auto *impl = AbckitGetApiImpl(VERSION);
    AbckitFile *file = impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_NE(file, nullptr);

    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, entryPoint);
    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "undefined\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "undefined\n"));

    const MethodInfo method = {"modules/base", "Handler", "handle"};

    ApiModifier modifier(VERSION, file, impl, method);

    modifier.Run();

    AbckitFile *modCtxI = modifier.GetFile();
    ASSERT_NE(modCtxI, nullptr);
    impl->writeAbc(modCtxI, OUTPUT_PATH, strlen(OUTPUT_PATH));
    impl->closeFile(file);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, entryPoint);
    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "-1\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "-1\n"));
}

}  // namespace libabckit::test
