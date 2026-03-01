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

// Test: test-kind=mock, api=DynamicIsa::CreateSupercallspread, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSupercallspread)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSupercallspread(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSupercallspread"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateApply, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateApply)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateApply(abckit::mock::helpers::GetMockInstruction(f),
                                                                    abckit::mock::helpers::GetMockInstruction(f),
                                                                    abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateApply"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallthis0, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallthis0)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallthis0(abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallthis0"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallthis1, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallthis1)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallthis1(abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f),
                                                                        abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallthis1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallthis2, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallthis2)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallthis2(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallthis2"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallthis3, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallthis3)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallthis3(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallthis3"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCallthisrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCallthisrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCallthisrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCallthisrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideCallthisrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideCallthisrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideCallthisrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideCallthisrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSupercallthisrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSupercallthisrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSupercallthisrange(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSupercallthisrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideSupercallthisrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideSupercallthisrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideSupercallthisrange(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideSupercallthisrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSupercallarrowrange, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSupercallarrowrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSupercallarrowrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSupercallarrowrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideSupercallarrowrange, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideSupercallarrowrange)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideSupercallarrowrange(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideSupercallarrowrange"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefinegettersetterbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefinegettersetterbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefinegettersetterbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefinefunc, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefinefunc)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefinefunc(abckit::mock::helpers::GetMockCoreFunction(f),
                                                                         DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefinefunc"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefinemethod, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefinemethod)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefinemethod(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockCoreFunction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefinemethod"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDefineclasswithbuffer, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDefineclasswithbuffer)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDefineclasswithbuffer(
            abckit::mock::helpers::GetMockCoreFunction(f), abckit::mock::helpers::GetMockLiteralArray(f), DEFAULT_U64,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDefineclasswithbuffer"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateResumegenerator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateResumegenerator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateResumegenerator(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateResumegenerator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetresumemode, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetresumemode)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetresumemode(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetresumemode"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGettemplateobject, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGettemplateobject)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGettemplateobject(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGettemplateobject"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetnextpropname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetnextpropname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetnextpropname(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetnextpropname"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDelobjprop, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDelobjprop)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDelobjprop(abckit::mock::helpers::GetMockInstruction(f),
                                                                         abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDelobjprop"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSuspendgenerator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSuspendgenerator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSuspendgenerator(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSuspendgenerator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncfunctionawaituncaught, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncfunctionawaituncaught)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncfunctionawaituncaught(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncfunctionawaituncaught"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCopydataproperties, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCopydataproperties)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCopydataproperties(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCopydataproperties"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStarrayspread, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStarrayspread)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStarrayspread(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStarrayspread"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSetobjectwithproto, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSetobjectwithproto)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSetobjectwithproto(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSetobjectwithproto"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdobjbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdobjbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdobjbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdobjbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStobjbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStobjbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStobjbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStobjbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStownbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStownbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStownbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStownbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdsuperbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdsuperbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdsuperbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdsuperbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStsuperbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStsuperbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStsuperbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStsuperbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdobjbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdobjbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdobjbyindex(abckit::mock::helpers::GetMockInstruction(f),
                                                                           DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdobjbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideLdobjbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideLdobjbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideLdobjbyindex(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideLdobjbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStobjbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStobjbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStobjbyindex(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStobjbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideStobjbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideStobjbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideStobjbyindex(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideStobjbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStownbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStownbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStownbyindex(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStownbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideStownbyindex, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideStownbyindex)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideStownbyindex(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideStownbyindex"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncfunctionresolve, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncfunctionresolve)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncfunctionresolve(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncfunctionresolve"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncfunctionreject, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncfunctionreject)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncfunctionreject(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncfunctionreject"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateCopyrestargs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateCopyrestargs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateCopyrestargs(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateCopyrestargs"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideCopyrestargs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideCopyrestargs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideCopyrestargs(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideCopyrestargs"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdlexvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdlexvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdlexvar(DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdlexvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideLdlexvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideLdlexvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideLdlexvar(DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideLdlexvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStlexvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStlexvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStlexvar(abckit::mock::helpers::GetMockInstruction(f),
                                                                       DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStlexvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideStlexvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideStlexvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideStlexvar(abckit::mock::helpers::GetMockInstruction(f),
                                                                           DEFAULT_U64, DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideStlexvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateGetmodulenamespace, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateGetmodulenamespace)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateGetmodulenamespace(
            abckit::mock::helpers::GetMockCoreModule(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateGetmodulenamespace"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideGetmodulenamespace, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideGetmodulenamespace)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideGetmodulenamespace(
            abckit::mock::helpers::GetMockCoreModule(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideGetmodulenamespace"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStmodulevar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStmodulevar(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockCoreExportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideStmodulevar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideStmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideStmodulevar(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockCoreExportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideStmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTryldglobalbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTryldglobalbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTryldglobalbyname(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTryldglobalbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateTrystglobalbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateTrystglobalbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateTrystglobalbyname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateTrystglobalbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdglobalvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdglobalvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdglobalvar(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdglobalvar"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStglobalvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStglobalvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStglobalvar(abckit::mock::helpers::GetMockInstruction(f),
                                                                          DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStglobalvar"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdobjbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdobjbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdobjbyname(abckit::mock::helpers::GetMockInstruction(f),
                                                                          DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdobjbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStobjbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStobjbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStobjbyname(abckit::mock::helpers::GetMockInstruction(f),
                                                                          DEFAULT_CONST_CHAR,
                                                                          abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStobjbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStownbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStownbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStownbyname(abckit::mock::helpers::GetMockInstruction(f),
                                                                          DEFAULT_CONST_CHAR,
                                                                          abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStownbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdsuperbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdsuperbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdsuperbyname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdsuperbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStsuperbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStsuperbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStsuperbyname(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStsuperbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdlocalmodulevar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdlocalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdlocalmodulevar(
            abckit::mock::helpers::GetMockCoreExportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdlocalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideLdlocalmodulevar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideLdlocalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideLdlocalmodulevar(
            abckit::mock::helpers::GetMockCoreExportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideLdlocalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdexternalmodulevar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdexternalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdexternalmodulevar(
            abckit::mock::helpers::GetMockCoreImportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdexternalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideLdexternalmodulevar, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideLdexternalmodulevar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideLdexternalmodulevar(
            abckit::mock::helpers::GetMockCoreImportDescriptor(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideLdexternalmodulevar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStownbyvaluewithnameset, abc-kind=ArkTS1, category=internal,
// extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStownbyvaluewithnameset)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStownbyvaluewithnameset(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f),
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStownbyvaluewithnameset"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStownbynamewithnameset, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStownbynamewithnameset)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStownbynamewithnameset(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_CONST_CHAR,
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStownbynamewithnameset"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdbigint, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdbigint)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdbigint(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdbigint"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdthisbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdthisbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdthisbyname(DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdthisbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStthisbyname, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStthisbyname)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStthisbyname(abckit::mock::helpers::GetMockInstruction(f),
                                                                           DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStthisbyname"));
        ASSERT_TRUE(CheckMockedApi("CreateString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateLdthisbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateLdthisbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateLdthisbyvalue(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateLdthisbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateStthisbyvalue, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateStthisbyvalue)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateStthisbyvalue(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateStthisbyvalue"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideLdpatchvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideLdpatchvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideLdpatchvar(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideLdpatchvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateWideStpatchvar, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateWideStpatchvar)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateWideStpatchvar(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateWideStpatchvar"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateDynamicimport, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateDynamicimport)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateDynamicimport(
            abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateDynamicimport"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateAsyncgeneratorreject, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateAsyncgeneratorreject)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateAsyncgeneratorreject(
            abckit::mock::helpers::GetMockInstruction(f), abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateAsyncgeneratorreject"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateSetgeneratorstate, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateSetgeneratorstate)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateSetgeneratorstate(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateSetgeneratorstate"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateReturn, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateReturn)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateReturn(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateReturn"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateReturnundefined, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateReturnundefined)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateReturnundefined();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateReturnundefined"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=DynamicIsa::CreateIf, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestDynIsa, DynamicIsa_CreateIf)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).DynIsa().CreateIf(abckit::mock::helpers::GetMockInstruction(f),
                                                                 DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("IcreateIf"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test
