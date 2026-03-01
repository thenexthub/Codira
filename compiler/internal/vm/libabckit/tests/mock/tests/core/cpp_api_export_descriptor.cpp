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

class LibAbcKitCppMockCoreTestExportDescriptor : public ::testing::Test {};

// Test: test-kind=mock, api=ExportDescriptor::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestExportDescriptor, ExportDescriptor_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreExportDescriptor(f).GetFile();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ExportDescriptor::GetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestExportDescriptor, ExportDescriptor_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreExportDescriptor(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("ExportDescriptorGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ExportDescriptor::GetExportingModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestExportDescriptor, ExportDescriptor_GetExportingModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreExportDescriptor(f).GetExportingModule();
        ASSERT_TRUE(CheckMockedApi("ExportDescriptorGetExportingModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ExportDescriptor::GetExportedModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestExportDescriptor, ExportDescriptor_GetExportedModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreExportDescriptor(f).GetExportedModule();
        ASSERT_TRUE(CheckMockedApi("ExportDescriptorGetExportedModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=ExportDescriptor::GetAlias, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestExportDescriptor, ExportDescriptor_GetAlias)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreExportDescriptor(f).GetAlias();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("ExportDescriptorGetAlias"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test