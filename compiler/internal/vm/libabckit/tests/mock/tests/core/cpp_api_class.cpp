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
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockCoreTestClass : public ::testing::Test {};

// Test: test-kind=mock, api=Class::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetFile();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("ClassGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetModule();
        ASSERT_TRUE(CheckMockedApi("ClassGetModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetAllMethods, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetAllMethods)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetAllMethods();
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateMethods"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetFields, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetFields();
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetAnnotations, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetAnnotations)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetAnnotations();
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateAnnotations"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetSuperClass, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetSuperClass)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetSuperClass();
        ASSERT_TRUE(CheckMockedApi("ClassGetSuperClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetSubClasses, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetSubClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetSubClasses();
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateSubClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetInterfaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetInterfaces();
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::EnumerateMethods, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_EnumerateMethods)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).EnumerateMethods(DEFAULT_LAMBDA(abckit::core::Function));
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateMethods"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::EnumerateAnnotations, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_EnumerateAnnotations)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).EnumerateAnnotations(DEFAULT_LAMBDA(abckit::core::Annotation));
        ASSERT_TRUE(CheckMockedApi("ClassEnumerateAnnotations"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetParentFunction, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetParentFunction)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetParentFunction();
        ASSERT_TRUE(CheckMockedApi("ClassGetParentFunction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::GetParentNamespace, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestClass, Class_GetParentNamespace)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreClass(f).GetParentNamespace();
        ASSERT_TRUE(CheckMockedApi("ClassGetParentNamespace"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test