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
#include "../../../src/mock/mock_values.h"
#include "../cpp_helpers_mock.h"

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitCppMockTestInstruction : public ::testing::Test {};

// Test: test-kind=mock, api=Instruction::GetImmediateSize, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetImmediateSize)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetImmediateSize(DEFAULT_SIZE_T);
        ASSERT_TRUE(CheckMockedApi("IgetImmediateSize"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::InsertAfter, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_InsertAfter)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).InsertAfter(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("IinsertAfter"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::InsertBefore, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_InsertBefore)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).InsertBefore(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("IinsertBefore"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::Remove, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_Remove)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).Remove();
        ASSERT_TRUE(CheckMockedApi("Iremove"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetConstantValueI64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetConstantValueI64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetConstantValueI64();
        ASSERT_TRUE(CheckMockedApi("IgetConstantValueI64"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetString, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetString)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetString();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("IgetString"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetNext, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetNext)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetNext();
        ASSERT_TRUE(CheckMockedApi("IgetNext"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetPrev, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetPrev)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetPrev();
        ASSERT_TRUE(CheckMockedApi("IgetPrev"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetFunction, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetFunction)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetFunction();
        ASSERT_TRUE(CheckMockedApi("IgetFunction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetBasicBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetBasicBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetBasicBlock();
        ASSERT_TRUE(CheckMockedApi("IgetBasicBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetInputCount, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetInputCount)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetInputCount();
        ASSERT_TRUE(CheckMockedApi("IgetInputCount"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetInput, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetInput)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetInput(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("IgetInput"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::SetInput, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_SetInput)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).SetInput(DEFAULT_U32,
                                                              abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("IsetInput"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::SetInputs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_SetInputs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).SetInputs(abckit::mock::helpers::GetMockInstruction(f),
                                                               abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("IsetInputs"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::VisitUsers, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_VisitUsers)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).VisitUsers(
            []([[maybe_unused]] const abckit::Instruction &inst) { return false; });
        ASSERT_TRUE(CheckMockedApi("IvisitUsers"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetUserCount, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetUserCount)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetUserCount();
        ASSERT_TRUE(CheckMockedApi("IgetUserCount"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetLiteralArray)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetLiteralArray();
        ASSERT_TRUE(CheckMockedApi("IgetLiteralArray"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetGraph, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetGraph)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetGraph();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Instruction::GetType, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestInstruction, Instruction_GetType)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockInstruction(f).GetType();
        ASSERT_TRUE(CheckMockedApi("IgetType"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test
