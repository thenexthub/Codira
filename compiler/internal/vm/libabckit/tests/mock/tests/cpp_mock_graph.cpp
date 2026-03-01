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

class LibAbcKitCppGraphMockTest : public ::testing::Test {};

// Test: test-kind=mock, api=Graph::GetIsa, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetIsa)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        AbckitIsaType type = abckit::mock::helpers::GetMockGraph(f).GetIsa();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetIsa"));
        ASSERT_TRUE(type == DEFAULT_ENUM_ISA_TYPE);
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetStartBb, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetStartBb)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::BasicBlock bb = abckit::mock::helpers::GetMockGraph(f).GetStartBb();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetStartBasicBlock"));
        ASSERT_TRUE(bb == abckit::mock::helpers::GetMockBasicBlock(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetEndBb, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetEndBb)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::BasicBlock bb = abckit::mock::helpers::GetMockGraph(f).GetEndBb();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetEndBasicBlock"));
        ASSERT_TRUE(bb == abckit::mock::helpers::GetMockBasicBlock(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetNumBbs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetNumBbs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        uint32_t numBbs = abckit::mock::helpers::GetMockGraph(f).GetNumBbs();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetNumberOfBasicBlocks"));
        ASSERT_TRUE(numBbs == DEFAULT_U32);
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetBlocksRPO, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetBlocksRPO)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto blocks = abckit::mock::helpers::GetMockGraph(f).GetBlocksRPO();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GvisitBlocksRpo"));
        ASSERT_TRUE(blocks[0] == abckit::mock::helpers::GetMockBasicBlock(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::EnumerateBasicBlocksRpo, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_EnumerateBasicBlocksRpo)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).EnumerateBasicBlocksRpo(
            []([[maybe_unused]] const abckit::BasicBlock &) { return false; });
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GvisitBlocksRpo"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetBasicBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetBasicBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::BasicBlock bb = abckit::mock::helpers::GetMockGraph(f).GetBasicBlock(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetBasicBlock"));
        ASSERT_TRUE(bb == abckit::mock::helpers::GetMockBasicBlock(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::GetNumberOfParameters, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_GetNumberOfParameters)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        uint32_t numBbs = abckit::mock::helpers::GetMockGraph(f).GetNumberOfParameters();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GgetNumberOfParameters"));
        ASSERT_TRUE(numBbs == DEFAULT_U32);
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::InsertTryCatch, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_InsertTryCatch)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::BasicBlock bb = abckit::mock::helpers::GetMockBasicBlock(f);
        abckit::mock::helpers::GetMockGraph(f).InsertTryCatch(bb, bb, bb, bb);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GinsertTryCatch"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::Dump, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_Dump)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).Dump(DEFAULT_I32);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("Gdump"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::FindOrCreateConstantI32, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_FindOrCreateConstantI32)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::Instruction inst = abckit::mock::helpers::GetMockGraph(f).FindOrCreateConstantI32(DEFAULT_I32);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GfindOrCreateConstantI32"));
        ASSERT_TRUE(inst == abckit::mock::helpers::GetMockInstruction(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::FindOrCreateConstantI64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_FindOrCreateConstantI64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::Instruction inst = abckit::mock::helpers::GetMockGraph(f).FindOrCreateConstantI64(DEFAULT_I64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GfindOrCreateConstantI64"));
        ASSERT_TRUE(inst == abckit::mock::helpers::GetMockInstruction(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::FindOrCreateConstantU64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_FindOrCreateConstantU64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::Instruction inst = abckit::mock::helpers::GetMockGraph(f).FindOrCreateConstantU64(DEFAULT_U64);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GfindOrCreateConstantU64"));
        ASSERT_TRUE(inst == abckit::mock::helpers::GetMockInstruction(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::FindOrCreateConstantF64, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_FindOrCreateConstantF64)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::Instruction inst = abckit::mock::helpers::GetMockGraph(f).FindOrCreateConstantF64(DEFAULT_DOUBLE);
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GfindOrCreateConstantF64"));
        ASSERT_TRUE(inst == abckit::mock::helpers::GetMockInstruction(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::CreateEmptyBb, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_CreateEmptyBb)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::BasicBlock bb = abckit::mock::helpers::GetMockGraph(f).CreateEmptyBb();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("BBCreateEmpty"));
        ASSERT_TRUE(bb == abckit::mock::helpers::GetMockBasicBlock(f));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Graph::RunPassRemoveUnreachableBlocks, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppGraphMockTest, Graph_RunPassRemoveUnreachableBlocks)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockGraph(f).RunPassRemoveUnreachableBlocks();
        ASSERT_TRUE(CheckMockedApi("DestroyGraph"));
        ASSERT_TRUE(CheckMockedApi("GrunPassRemoveUnreachableBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test
