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

class LibAbcKitCppMockArktsTestField : public ::testing::Test {};

// Test: test-kind=mock, api=ClassField::RemoveAnnotation, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ClassField_RemoveAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClassField(f).RemoveAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotation(f));
        ASSERT_TRUE(CheckMockedApi("ClassFieldRemoveAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ClassField::AddAnnotation, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ClassField_AddAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClassField(f).AddAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotationInterface(f));
        ASSERT_TRUE(CheckMockedApi("ClassFieldAddAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ClassField::SetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ClassField_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClassField(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("ClassFieldSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ClassField::SetType, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ClassField_SetType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClassField(f).SetType(abckit::mock::helpers::GetMockType(f));
        ASSERT_TRUE(CheckMockedApi("ClassFieldSetType"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ClassField::SetValue, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ClassField_SetValue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsClassField(f).SetValue(abckit::mock::helpers::GetMockValueDouble(f));
        ASSERT_TRUE(CheckMockedApi("ClassFieldSetValue"));
        ASSERT_TRUE(CheckMockedApi("CoreClassFieldToArktsClassField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ModuleField::SetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ModuleField_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsModuleField(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("ModuleFieldSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleFieldToArktsModuleField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ModuleField::SetType, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ModuleField_SetType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsModuleField(f).SetType(abckit::mock::helpers::GetMockType(f));
        ASSERT_TRUE(CheckMockedApi("ModuleFieldSetType"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleFieldToArktsModuleField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ModuleField::SetValue, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, ModuleField_SetValue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsModuleField(f).SetValue(abckit::mock::helpers::GetMockValueDouble(f));
        ASSERT_TRUE(CheckMockedApi("ModuleFieldSetValue"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleFieldToArktsModuleField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=InterfaceField::SetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, InterfaceField_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterfaceField(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("InterfaceFieldSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=InterfaceField::SetType, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, InterfaceField_SetType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterfaceField(f).SetType(abckit::mock::helpers::GetMockType(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceFieldSetType"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=InterfaceField::RemoveAnnotation, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, InterfaceField_RemoveAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterfaceField(f).RemoveAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotation(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceFieldRemoveAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=InterfaceField::AddAnnotation, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, InterfaceField_AddAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterfaceField(f).AddAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotationInterface(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceFieldAddAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=EnumField::SetName, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestField, EnumField_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsEnumField(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("EnumFieldSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreEnumFieldToArktsEnumField"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test