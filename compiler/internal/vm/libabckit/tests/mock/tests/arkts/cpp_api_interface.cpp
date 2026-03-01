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

#include "include/libabckit/cpp/abckit_cpp.h"
#include "src/mock/mock_values.h"
#include "tests/mock/check_mock.h"
#include "tests/mock/cpp_helpers_mock.h"

namespace libabckit::cpp_test {

class LibAbcKitCppMockArktsTestInterface : public ::testing::Test {};

// Test: test-kind=mock, api=Interface::RemoveField, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_RemoveField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto ae = abckit::mock::helpers::GetMockArktsInterfaceField(f);
        abckit::mock::helpers::GetMockArktsInterface(f).RemoveField(ae);
        ASSERT_TRUE(CheckMockedApi("InterfaceRemoveField"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::CreateInterface, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_CreateInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::arkts::Interface::CreateInterface(abckit::mock::helpers::GetMockArktsModule(f), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
        ASSERT_TRUE(CheckMockedApi("ArktsInterfaceToCoreInterface"));
        ASSERT_TRUE(CheckMockedApi("CreateInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::SetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_SetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).SetName(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("InterfaceSetName"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::AddField, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_AddField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).AddField(DEFAULT_CONST_CHAR,
                                                                 abckit::mock::helpers::GetMockType(f));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceFieldToArktsInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("ArktsInterfaceFieldToCoreInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("InterfaceAddField"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::AddMethod, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_AddMethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto ae = abckit::mock::helpers::GetMockArktsInterfaceMethod(f);
        abckit::mock::helpers::GetMockArktsInterface(f).AddMethod(ae);
        ASSERT_TRUE(CheckMockedApi("InterfaceAddMethod"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::AddSuperInterface, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_AddSuperInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).AddSuperInterface(
            abckit::mock::helpers::GetMockArktsInterface(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceAddSuperInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::RemoveSuperInterface, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_RemoveSuperInterface)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).RemoveSuperInterface(
            abckit::mock::helpers::GetMockArktsInterface(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceRemoveSuperInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::RemoveMethod, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_RemoveMethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).RemoveMethod(abckit::mock::helpers::GetMockArktsFunction(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceRemoveMethod"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Interface::SetOwningModule, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestInterface, Interface_SetOwningModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsInterface(f).SetOwningModule(abckit::mock::helpers::GetMockArktsModule(f));
        ASSERT_TRUE(CheckMockedApi("InterfaceSetOwningModule"));
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("CoreInterfaceToArktsInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test
