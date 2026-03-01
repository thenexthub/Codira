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

#include "../../cpp_helpers_mock.h"
#include "include/libabckit/cpp/abckit_cpp.h"
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockCoreTestInterface : public ::testing::Test {};

// Test: test-kind=mock, api=Interface::GetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("InterfaceGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::GetAllMethods, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetAllMethods)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetAllMethods();
        ASSERT_TRUE(CheckMockedApi("InterfaceEnumerateMethods"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::GetFields, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetFields();
        ASSERT_TRUE(CheckMockedApi("InterfaceEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::GetSuperInterfaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetSuperInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetSuperInterfaces();
        ASSERT_TRUE(CheckMockedApi("InterfaceEnumerateSuperInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::GetSubInterfaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetSubInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetSubInterfaces();
        ASSERT_TRUE(CheckMockedApi("InterfaceEnumerateSubInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::GetClasses, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestInterface, Interface_GetClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreInterface(f).GetClasses();
        ASSERT_TRUE(CheckMockedApi("InterfaceEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}
}  // namespace libabckit::cpp_test