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

class LibAbcKitCppMockCoreTestAnnotationInterface : public ::testing::Test {};

// Test: test-kind=mock, api=AnnotationInterface::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestAnnotationInterface, AnnotationInterface_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreAnnotationInterface(f).GetFile();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=AnnotationInterface::GetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestAnnotationInterface, AnnotationInterface_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreAnnotationInterface(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=AnnotationInterface::GetFields, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestAnnotationInterface, AnnotationInterface_GetFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreAnnotationInterface(f).GetFields();
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=AnnotationInterface::GetModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestAnnotationInterface, AnnotationInterface_GetModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreAnnotationInterface(f).GetModule();
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceGetModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=AnnotationInterface::EnumerateFields, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestAnnotationInterface, AnnotationInterface_EnumerateFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreAnnotationInterface(f).EnumerateFields(
            DEFAULT_LAMBDA(abckit::core::AnnotationInterfaceField));
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test