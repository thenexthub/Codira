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
#include "../check_mock.h"
#include "src/mock/mock_values.h"
#include "../cpp_helpers_mock.h"

#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockTestLiteral : public ::testing::Test {};

// Test: test-kind=mock, api=Literal::GetLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetLiteralArray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralLiteralArray(f);
        ASSERT_TRUE(lit.GetLiteralArray() == abckit::mock::helpers::GetMockLiteralArray(f));

        ASSERT_TRUE(CheckMockedApi("LiteralGetLiteralArray"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteral(f);
        ASSERT_TRUE(*(lit.GetFile()) == f);
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetBool, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetBool)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralBool(f);
        ASSERT_TRUE(lit.GetBool() == DEFAULT_BOOL);
        ASSERT_TRUE(CheckMockedApi("LiteralGetBool"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetU8, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetU8)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralU8(f);
        ASSERT_TRUE(lit.GetU8() == DEFAULT_U8);
        ASSERT_TRUE(CheckMockedApi("LiteralGetU8"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetU16, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetU16)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralU16(f);
        ASSERT_TRUE(lit.GetU16() == DEFAULT_U16);
        ASSERT_TRUE(CheckMockedApi("LiteralGetU16"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetU32, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetU32)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralU32(f);
        ASSERT_TRUE(lit.GetU32() == DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("LiteralGetU32"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetU64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetU64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralU64(f);
        ASSERT_TRUE(lit.GetU64() == DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("LiteralGetU64"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetMethodAffiliate, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetMethodAffiliate)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralMethodAffiliate(f);
        ASSERT_TRUE(lit.GetMethodAffiliate() == DEFAULT_U16);
        ASSERT_TRUE(CheckMockedApi("LiteralGetMethodAffiliate"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetFloat, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetFloat)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralFloat(f);
        ASSERT_TRUE(lit.GetFloat() == DEFAULT_FLOAT);
        ASSERT_TRUE(CheckMockedApi("LiteralGetFloat"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetDouble, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetDouble)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralDouble(f);
        ASSERT_TRUE(lit.GetDouble() == DEFAULT_DOUBLE);
        ASSERT_TRUE(CheckMockedApi("LiteralGetDouble"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralString(f);
        ASSERT_TRUE(lit.GetString() == DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("LiteralGetString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetMethod, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetMethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralString(f);
        ASSERT_TRUE(lit.GetMethod() == DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("LiteralGetMethod"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Literal::GetTag, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestLiteral, Literal_GetTag)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto lit = abckit::mock::helpers::GetMockLiteralString(f);
        ASSERT_TRUE(lit.GetTag() == DEFAULT_LITERAL_TAG);
        ASSERT_TRUE(CheckMockedApi("LiteralGetTag"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test