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

#ifndef PANDA_PLUGINS_ETS_COMPILER_GTEST_H
#define PANDA_PLUGINS_ETS_COMPILER_GTEST_H

#include <gtest/gtest.h>
#include <cstdlib>
#include <optional>
#include <vector>

namespace ark::ets::compiler::testing {

class CompilerTest : public ::testing::Test {
public:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        // Create boot-panda-files options
        bootFileString_ = "--boot-panda-files=" + std::string(stdlib);
        const char *abcPath = std::getenv("COMPILER_GTEST_ABC_PATH");
        ASSERT_NE(abcPath, nullptr);
        abcFile_ = std::string(abcPath);
    }

protected:
    std::string bootFileString_;  // NOLINT(misc-non-private-member-variables-in-classes)
    std::string abcFile_;         // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::compiler::testing

#endif  // PANDA_PLUGINS_ETS_COMPILER_GTEST_H
