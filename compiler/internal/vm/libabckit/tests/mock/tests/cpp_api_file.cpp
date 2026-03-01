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
#include "include/libabckit/cpp/headers/core/module.h"
#include "src/mock/mock_values.h"
#include "../cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockTestFile : public ::testing::Test {};

// Test: test-kind=mock, api=File::CreateValueU1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateValueU1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateValueU1(DEFAULT_BOOL);

        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateValueDouble, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateValueDouble)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateValueDouble(DEFAULT_DOUBLE);

        ASSERT_TRUE(CheckMockedApi("CreateValueDouble"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralBool, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralBool)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralBool(DEFAULT_BOOL);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralBool"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralU8, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralU8)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralU8(DEFAULT_U8);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralU8"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralU16, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralU16)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralU16(DEFAULT_U16);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralU16"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralU32, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralU32)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralU32(DEFAULT_U32);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralU32"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralU64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralU64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralU64(DEFAULT_U64);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralU64"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralFloat, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralFloat)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralFloat(DEFAULT_FLOAT);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralFloat"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralDouble, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralDouble)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralDouble(DEFAULT_DOUBLE);

        ASSERT_TRUE(CheckMockedApi("CreateLiteralDouble"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralLiteralArray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralLiteralArray(abckit::mock::helpers::GetMockLiteralArray(f));

        ASSERT_TRUE(CheckMockedApi("CreateLiteralLiteralArray"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralArray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto l = abckit::mock::helpers::GetMockLiteral(f);
        f.CreateLiteralArray({l});

        ASSERT_TRUE(CheckMockedApi("CreateLiteralArray"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralString(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CreateLiteralString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralMethod, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralMethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto func = abckit::mock::helpers::GetMockCoreFunction(f);
        f.CreateLiteralMethod(func);
        ASSERT_TRUE(CheckMockedApi("CreateLiteralMethod"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::WriteAbc, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_WriteAbc)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.WriteAbc(DEFAULT_PATH);

        ASSERT_TRUE(CheckMockedApi("WriteAbc"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::GetModules, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_GetModules)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.GetModules();
        ASSERT_TRUE(CheckMockedApi("FileEnumerateModules"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::EnumerateModules, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_EnumerateModules)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.EnumerateModules(DEFAULT_LAMBDA(abckit::core::Module));
        ASSERT_TRUE(CheckMockedApi("FileEnumerateModules"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::EnumerateExternalModules, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_EnumerateExternalModules)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.EnumerateExternalModules(DEFAULT_LAMBDA(abckit::core::Module));
        ASSERT_TRUE(CheckMockedApi("FileEnumerateExternalModules"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateType, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateType(DEFAULT_ENUM_TYPE_ID);
        ASSERT_TRUE(CheckMockedApi("CreateType"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateReferenceType, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateReferenceType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto c = abckit::mock::helpers::GetMockCoreClass(f);
        f.CreateReferenceType(c);
        ASSERT_TRUE(CheckMockedApi("CreateReferenceType"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateValueString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateValueString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateValueString(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CreateValueString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralArrayValue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralArrayValue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto val = abckit::mock::helpers::GetMockValueU1(f);
        std::vector<abckit::Value> litArr = {val};
        f.CreateLiteralArrayValue(litArr, 1);
        ASSERT_TRUE(CheckMockedApi("CreateLiteralArrayValue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::CreateLiteralMethodAffiliate, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_CreateLiteralMethodAffiliate)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.CreateLiteralMethodAffiliate(DEFAULT_U16);
        ASSERT_TRUE(CheckMockedApi("CreateLiteralMethodAffiliate"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::AddExternalModuleArktsV1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_AddExternalModuleArktsV1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.AddExternalModuleArktsV1(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("ArktsModuleToCoreModule"));
        ASSERT_TRUE(CheckMockedApi("FileAddExternalModuleArktsV1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::AddExternalModuleArktsV2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_AddExternalModuleArktsV2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.AddExternalModuleArktsV2(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
        ASSERT_TRUE(CheckMockedApi("ArktsModuleToCoreModule"));
        ASSERT_TRUE(CheckMockedApi("FileAddExternalModuleArktsV2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=File::AddExternalModuleJs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestFile, File_AddExternalModuleJs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        f.AddExternalModuleJs(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreModuleToJsModule"));
        ASSERT_TRUE(CheckMockedApi("JsModuleToCoreModule"));
        ASSERT_TRUE(CheckMockedApi("FileAddExternalModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test