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

class LibAbcKitCppMockArktsTestClass : public ::testing::Test {};

// Test: test-kind=mock, api=Class::RemoveAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_RemoveAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).RemoveAnnotation(abckit::mock::helpers::GetMockArktsAnnotation(f));
        ASSERT_TRUE(CheckMockedApi("ClassRemoveAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::AddAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_AddAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).AddAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotationInterface(f));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationToCoreAnnotation"));
        ASSERT_TRUE(CheckMockedApi("ClassAddAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::AddInterface, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_AddInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).AddInterface(abckit::mock::helpers::GetMockArktsInterface(f));
        ASSERT_TRUE(CheckMockedApi("ClassAddInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::RemoveInterface, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_RemoveInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).RemoveInterface(abckit::mock::helpers::GetMockArktsInterface(f));
        ASSERT_TRUE(CheckMockedApi("ClassRemoveInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::SetSuperClass, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_SetSuperClass)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).SetSuperClass(abckit::mock::helpers::GetMockArktsClass(f));
        ASSERT_TRUE(CheckMockedApi("ClassSetSuperClass"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::RemoveMethod, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_RemoveMethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).RemoveMethod(abckit::mock::helpers::GetMockArktsFunction(f));
        ASSERT_TRUE(CheckMockedApi("ClassRemoveMethod"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::CreateClass, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_CreateClass)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::arkts::Class::CreateClass(abckit::mock::helpers::GetMockArktsModule(f), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
        ASSERT_TRUE(CheckMockedApi("ArktsClassToCoreClass"));
        ASSERT_TRUE(CheckMockedApi("CreateClass"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::RemoveField, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_RemoveField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto ae = abckit::mock::helpers::GetMockArktsClassField(f);
        abckit::mock::helpers::GetMockArktsClass(f).RemoveField(ae);
        ASSERT_TRUE(CheckMockedApi("ClassRemoveField"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::SetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("ClassSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::AddField, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_AddField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).AddField(DEFAULT_CONST_CHAR, abckit::mock::helpers::GetMockType(f),
                                                             abckit::mock::helpers::GetMockValueDouble(f));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
        ASSERT_TRUE(CheckMockedApi("ArktsClassFieldToCoreClassField"));
        ASSERT_TRUE(CheckMockedApi("ClassAddField"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
        abckit::mock::helpers::GetMockArktsClass(f).AddField(DEFAULT_CONST_CHAR, abckit::mock::helpers::GetMockType(f));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
        ASSERT_TRUE(CheckMockedApi("ArktsClassFieldToCoreClassField"));
        ASSERT_TRUE(CheckMockedApi("ClassAddField"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Class::SetOwningModule, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestClass, Class_SetOwningModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClass(f).SetOwningModule(abckit::mock::helpers::GetMockArktsModule(f));
        ASSERT_TRUE(CheckMockedApi("ClassSetOwningModule"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}
}  // namespace libabckit::cpp_test