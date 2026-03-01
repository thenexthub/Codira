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

#include "../../../include/libabckit/cpp/abckit_cpp.h"
#include "../check_mock.h"
#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitCppMockTest : public ::testing::Test {};

// Test: test-kind=mock, api=File::WriteAbc, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTest, CppTestMockFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File file("abckit.abc");
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        file.WriteAbc("abckit.abc");
        ASSERT_TRUE(CheckMockedApi("WriteAbc"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::EnumerateModules, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTest, CppTestMockFileEnumerateModules)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File file("abckit.abc");
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));

        std::vector<abckit::core::Module> modules;

        file.EnumerateModules([&](const abckit::core::Module &md) -> bool {
            modules.push_back(md);
            return true;
        });

        ASSERT_TRUE(CheckMockedApi("FileEnumerateModules"));

        file.WriteAbc("abckit.abc");
        ASSERT_TRUE(CheckMockedApi("WriteAbc"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::GetModules, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTest, CppTestGetModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File file("abckit.abc");
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::core::Module mdl = file.GetModules()[0];
        ASSERT_TRUE(CheckMockedApi("FileEnumerateModules"));
        abckit::core::Class cls = mdl.GetClasses()[0];
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test
