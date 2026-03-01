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

#include "../../../include/libabckit/cpp/abckit_cpp.h"
#include "../check_mock.h"
#include "../../../src/mock/mock_values.h"
#include "../cpp_helpers_mock.h"

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitCppMockTestDynIsa : public ::testing::Test {};

// Test: test-kind=mock, api=DynamicIsa::CreateLoadString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLoadString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLoadString(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLoadString"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSub2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSub2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSub2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSub2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::GetModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_GetModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().GetModule(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IgetModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::SetModule, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_SetModule)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().SetModule(abckit::mock::helpers::GetMockInstruction(f),
                                                                  abckit::mock::helpers::GetMockCoreModule(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IsetModule"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::GetConditionCode, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_GetConditionCode)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().GetConditionCode(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IgetConditionCode"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::SetConditionCode, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_SetConditionCode)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().SetConditionCode(abckit::mock::helpers::GetMockInstruction(f),
                                                                         DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IsetConditionCode"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::GetOpcode, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_GetOpcode)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().GetOpcode(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IgetOpcode"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::GetImportDescriptor, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_GetImportDescriptor)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().GetImportDescriptor(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IgetImportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::SetImportDescriptor, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_SetImportDescriptor)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().SetImportDescriptor(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockCoreImportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IsetImportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::GetExportDescriptor, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_GetExportDescriptor)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().GetExportDescriptor(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IgetExportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::SetExportDescriptor, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_SetExportDescriptor)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().SetExportDescriptor(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockCoreExportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IsetExportDescriptor"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdnan, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdnan)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdnan();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdnan"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdinfinity, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdinfinity)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdinfinity();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdinfinity"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdundefined, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdundefined)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdundefined();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdundefined"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdnull, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdnull)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdnull();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdnull"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdsymbol, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdsymbol)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdsymbol();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdsymbol"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdglobal, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdglobal)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdglobal();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdglobal"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdtrue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdtrue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdtrue();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdtrue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdfalse, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdfalse)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdfalse();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdfalse"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdhole, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdhole)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdhole();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdhole"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdnewtarget, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdnewtarget)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdnewtarget();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdnewtarget"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdthis, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdthis)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdthis();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdthis"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreatePoplexenv, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreatePoplexenv)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreatePoplexenv();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreatePoplexenv"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetunmappedargs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetunmappedargs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetunmappedargs();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetunmappedargs"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncfunctionenter, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncfunctionenter)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncfunctionenter();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncfunctionenter"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdfunction, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdfunction)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdfunction();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdfunction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDebugger, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDebugger)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDebugger();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDebugger"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetpropiterator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetpropiterator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetpropiterator(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetpropiterator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetiterator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetiterator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetiterator(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetiterator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetasynciterator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetasynciterator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetasynciterator(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetasynciterator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdprivateproperty, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdprivateproperty)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdprivateproperty(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdprivateproperty"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStprivateproperty, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStprivateproperty)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStprivateproperty(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStprivateproperty"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTestin, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTestin)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTestin(abckit::mock::helpers::GetMockInstruction(f),
                                                                     DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTestin"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefinefieldbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefinefieldbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefinefieldbyname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefinefieldbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefinepropertybyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefinepropertybyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefinepropertybyname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefinepropertybyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateemptyobject, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateemptyobject)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateemptyobject();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateemptyobject"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateemptyarray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateemptyarray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateemptyarray();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateemptyarray"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreategeneratorobj, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreategeneratorobj)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreategeneratorobj(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreategeneratorobj"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateiterresultobj, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateiterresultobj)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateiterresultobj(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateiterresultobj"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateobjectwithexcludedkeys, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateobjectwithexcludedkeys)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateobjectwithexcludedkeys(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateobjectwithexcludedkeys"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideCreateobjectwithexcludedkeys, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideCreateobjectwithexcludedkeys)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideCreateobjectwithexcludedkeys(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideCreateobjectwithexcludedkeys"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreatearraywithbuffer, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreatearraywithbuffer)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreatearraywithbuffer(
            abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreatearraywithbuffer"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateobjectwithbuffer)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateobjectwithbuffer(
            abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateobjectwithbuffer"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNewobjapply, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNewobjapply)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNewobjapply(abckit::mock::helpers::GetMockInstruction(f),
                                                                          abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNewobjapply"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNewobjrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNewobjrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNewobjrange(abckit::mock::helpers::GetMockInstruction(f),
                                                                          abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNewobjrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideNewobjrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideNewobjrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideNewobjrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideNewobjrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNewlexenv, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNewlexenv)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNewlexenv(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNewlexenv"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideNewlexenv, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideNewlexenv)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideNewlexenv(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideNewlexenv"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNewlexenvwithname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNewlexenvwithname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNewlexenvwithname(
            DEFAULT_U64, abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNewlexenvwithname"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideNewlexenvwithname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideNewlexenvwithname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideNewlexenvwithname(
            DEFAULT_U64, abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideNewlexenvwithname"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCreateasyncgeneratorobj, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCreateasyncgeneratorobj)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCreateasyncgeneratorobj(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCreateasyncgeneratorobj"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncgeneratorresolve, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncgeneratorresolve)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncgeneratorresolve(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncgeneratorresolve"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAdd2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAdd2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAdd2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAdd2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateMul2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateMul2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateMul2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateMul2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDiv2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDiv2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDiv2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDiv2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateMod2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateMod2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateMod2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateMod2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateEq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateEq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateEq(abckit::mock::helpers::GetMockInstruction(f),
                                                                 abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateEq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNoteq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNoteq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNoteq(abckit::mock::helpers::GetMockInstruction(f),
                                                                    abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNoteq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLess, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLess)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLess(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLess"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLesseq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLesseq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLesseq(abckit::mock::helpers::GetMockInstruction(f),
                                                                     abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLesseq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGreater, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGreater)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGreater(abckit::mock::helpers::GetMockInstruction(f),
                                                                      abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGreater"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGreatereq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGreatereq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGreatereq(abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGreatereq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateShl2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateShl2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateShl2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateShl2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateShr2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateShr2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateShr2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateShr2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAshr2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAshr2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAshr2(abckit::mock::helpers::GetMockInstruction(f),
                                                                    abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAshr2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAnd2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAnd2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAnd2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAnd2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateOr2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateOr2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateOr2(abckit::mock::helpers::GetMockInstruction(f),
                                                                  abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateOr2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateXor2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateXor2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateXor2(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateXor2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateExp, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateExp)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateExp(abckit::mock::helpers::GetMockInstruction(f),
                                                                  abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateExp"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTypeof, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTypeof)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTypeof(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTypeof"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTonumber, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTonumber)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTonumber(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTonumber"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTonumeric, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTonumeric)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTonumeric(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTonumeric"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNeg, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNeg)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNeg(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNeg"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateNot, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateNot)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateNot(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateNot"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateInc, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateInc)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateInc(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateInc"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDec, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDec)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDec(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDec"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateIstrue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateIstrue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateIstrue(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateIstrue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateIsfalse, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateIsfalse)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateIsfalse(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateIsfalse"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateIsin, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateIsin)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateIsin(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateIsin"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateInstanceof, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateInstanceof)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateInstanceof(abckit::mock::helpers::GetMockInstruction(f),
                                                                         abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateInstanceof"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStrictnoteq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStrictnoteq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStrictnoteq(abckit::mock::helpers::GetMockInstruction(f),
                                                                          abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStrictnoteq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStricteq, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStricteq)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStricteq(abckit::mock::helpers::GetMockInstruction(f),
                                                                       abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStricteq"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeNotifyconcurrentresult, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeNotifyconcurrentresult)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeNotifyconcurrentresult(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeNotifyconcurrentresult"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeDefinefieldbyvalue, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeDefinefieldbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeDefinefieldbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeDefinefieldbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeDefinefieldbyindex, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeDefinefieldbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeDefinefieldbyindex(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeDefinefieldbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeTopropertykey, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeTopropertykey)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeTopropertykey(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeTopropertykey"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeCreateprivateproperty, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeCreateprivateproperty)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeCreateprivateproperty(
            DEFAULT_U64, abckit::mock::helpers::GetMockLiteralArray(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeCreateprivateproperty"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeDefineprivateproperty, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeDefineprivateproperty)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeDefineprivateproperty(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeDefineprivateproperty"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeCallinit, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeCallinit)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeCallinit(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeCallinit"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeDefinesendableclass, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeDefinesendableclass)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeDefinesendableclass(
            abckit::mock::helpers::GetMockCoreFunction(f), abckit::mock::helpers::GetMockLiteralArray(f), DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeDefinesendableclass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeLdsendableclass, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeLdsendableclass)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeLdsendableclass(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeLdsendableclass"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeLdsendableexternalmodulevar, abc-kind=ArkTS1,
// category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeLdsendableexternalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeLdsendableexternalmodulevar(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeLdsendableexternalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeWideldsendableexternalmodulevar, abc-kind=ArkTS1,
// category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeWideldsendableexternalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeWideldsendableexternalmodulevar(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeWideldsendableexternalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeNewsendableenv, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeNewsendableenv)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeNewsendableenv(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeNewsendableenv"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeWidenewsendableenv, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeWidenewsendableenv)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeWidenewsendableenv(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeWidenewsendableenv"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeStsendablevar, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeStsendablevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeStsendablevar(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeStsendablevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeWidestsendablevar, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeWidestsendablevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeWidestsendablevar(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeWidestsendablevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeLdsendablevar, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeLdsendablevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeLdsendablevar(DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeLdsendablevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeWideldsendablevar, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeWideldsendablevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeWideldsendablevar(DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeWideldsendablevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeIstrue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeIstrue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeIstrue(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeIstrue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallruntimeIsfalse, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallruntimeIsfalse)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallruntimeIsfalse(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallruntimeIsfalse"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrow, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrow)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrow(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrow"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowNotexists, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowNotexists)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowNotexists();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowNotexists"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowPatternnoncoercible, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowPatternnoncoercible)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowPatternnoncoercible();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowPatternnoncoercible"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowDeletesuperproperty, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowDeletesuperproperty)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowDeletesuperproperty();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowDeletesuperproperty"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowConstassignment, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowConstassignment)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowConstassignment(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowConstassignment"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowIfnotobject, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowIfnotobject)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowIfnotobject(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowIfnotobject"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowUndefinedifhole, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowUndefinedifhole)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowUndefinedifhole(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowUndefinedifhole"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowIfsupernotcorrectcall, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowIfsupernotcorrectcall)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowIfsupernotcorrectcall(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowIfsupernotcorrectcall"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateThrowUndefinedifholewithname, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateThrowUndefinedifholewithname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateThrowUndefinedifholewithname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateThrowUndefinedifholewithname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallarg0, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallarg0)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallarg0(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallarg0"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallarg1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallarg1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallarg1(abckit::mock::helpers::GetMockInstruction(f),
                                                                       abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallarg1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallargs2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallargs2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallargs2(abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallargs2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallargs3, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallargs3)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallargs3(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallargs3"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallrange(abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideCallrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideCallrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideCallrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideCallrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test
