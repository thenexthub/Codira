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

#include "../check_mock.h"
#include "src/mock/mock_values.h"
#include "../cpp_helpers_mock.h"

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitCppMockTestValue : public ::testing::Test {};

// Test: test-kind=mock, api=Value::GetU1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetU1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueU1(f).GetU1();
        ASSERT_TRUE(CheckMockedApi("ValueGetU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Value::GetInt, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetInt)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueInt(f).GetInt();
        ASSERT_TRUE(CheckMockedApi("ValueGetInt"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Value::GetDouble, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetDouble)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueDouble(f).GetDouble();
        ASSERT_TRUE(CheckMockedApi("ValueGetDouble"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Value::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueU1(f).GetFile();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Value::GetType, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueU1(f).GetType();
        ASSERT_TRUE(CheckMockedApi("ValueGetType"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Value::GetLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_ValueGetLiteralArray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto litArr = abckit::mock::helpers::GetMockValueLiteralArray(f);
        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
        ASSERT_TRUE(litArr.GetLiteralArray() == abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("ArrayValueGetLiteralArray"));
    }

    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}
// Test: test-kind=mock, api=Value::GetString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestValue, Value_GetString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockValueString(f).GetString();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("ValueGetString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test