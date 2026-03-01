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

#include "include/libabckit/cpp/abckit_cpp.h"
#include "include/libabckit/cpp/headers/core/namespace.h"
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockCoreTestNamespace : public ::testing::Test {};

// Test: test-kind=mock, api=Namespace::GetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("NamespaceGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetParentNamespace, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetParentNamespace)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetParentNamespace();
        ASSERT_TRUE(CheckMockedApi("NamespaceGetParentNamespace"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetNamespaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetNamespaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetNamespaces();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateNamespaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetClasses, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetClasses();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetInterfaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetInterfaces();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetEnums, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetEnums)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetEnums();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateEnums"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetTopLevelFunctions, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetTopLevelFunctions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetTopLevelFunctions();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateTopLevelFunctions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::GetFields, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_GetFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).GetFields();
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::EnumerateNamespaces, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_EnumerateNamespaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).EnumerateNamespaces(DEFAULT_LAMBDA(abckit::core::Namespace));
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateNamespaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::EnumerateClasses, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_EnumerateClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).EnumerateClasses(DEFAULT_LAMBDA(abckit::core::Class));
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Namespace::EnumerateTopLevelFunctions, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestNamespace, Namespace_EnumerateTopLevelFunctions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreNamespace(f).EnumerateTopLevelFunctions(
            DEFAULT_LAMBDA(abckit::core::Function));
        ASSERT_TRUE(CheckMockedApi("NamespaceEnumerateTopLevelFunctions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test