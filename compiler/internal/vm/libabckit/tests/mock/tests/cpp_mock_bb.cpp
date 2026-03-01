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
#include "include/libabckit/cpp/headers/basic_block.h"

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitCppMockTestBasicBlock : public ::testing::Test {};

// Test: test-kind=mock, api=BasicBlock::VisitSuccBlocks, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_VisitSuccBlocks)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).VisitSuccBlocks(
            []([[maybe_unused]] const abckit::BasicBlock &bb) {});
        ASSERT_TRUE(CheckMockedApi("BBVisitSuccBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::VisitPredBlocks, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_VisitPredBlocks)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).VisitPredBlocks(
            []([[maybe_unused]] const abckit::BasicBlock &bb) { return false; });
        ASSERT_TRUE(CheckMockedApi("BBVisitPredBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetSuccCount, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetSuccCount)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetSuccCount();
        ASSERT_TRUE(CheckMockedApi("BBGetSuccBlockCount"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetSuccByIdx, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetSuccByIdx)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetSuccByIdx(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("BBGetSuccBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetSuccs, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetSuccs)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetSuccs();
        ASSERT_TRUE(CheckMockedApi("BBVisitSuccBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetPreds, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetPreds)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetPreds();
        ASSERT_TRUE(CheckMockedApi("BBVisitPredBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::AddInstFront, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_AddInstFront)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).AddInstFront(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("BBAddInstFront"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::AddInstBack, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_AddInstBack)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).AddInstBack(abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("BBAddInstBack"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetFirstInst, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetFirstInst)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetFirstInst();
        ASSERT_TRUE(CheckMockedApi("BBGetFirstInst"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetLastInst, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetLastInst)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetLastInst();
        ASSERT_TRUE(CheckMockedApi("BBGetLastInst"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetId, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetId)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetId();
        ASSERT_TRUE(CheckMockedApi("BBGetId"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetGraph, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetGraph)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetGraph();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetPredBlockCount, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetPredBlockCount)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetPredBlockCount();
        ASSERT_TRUE(CheckMockedApi("BBGetPredBlockCount"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetPredBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetPredBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetPredBlock(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("BBGetPredBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetSuccBlockCount, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetSuccBlockCount)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetSuccBlockCount();
        ASSERT_TRUE(CheckMockedApi("BBGetSuccBlockCount"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetSuccBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetSuccBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetSuccBlock(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("BBGetSuccBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::InsertSuccBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_InsertSuccBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto bb = abckit::mock::helpers::GetMockBasicBlock(f);
        abckit::mock::helpers::GetMockBasicBlock(f).InsertSuccBlock(bb, DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("BBInsertSuccBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::AppendSuccBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_AppendSuccBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto bb = abckit::mock::helpers::GetMockBasicBlock(f);
        abckit::mock::helpers::GetMockBasicBlock(f).AppendSuccBlock(bb);
        ASSERT_TRUE(CheckMockedApi("BBAppendSuccBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::EraseSuccBlock, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_EraseSuccBlock)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).EraseSuccBlock(DEFAULT_U32);
        ASSERT_TRUE(CheckMockedApi("BBdisconnectSuccBlock"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetTrueBranch, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetTrueBranch)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetTrueBranch();
        ASSERT_TRUE(CheckMockedApi("BBGetTrueBranch"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetFalseBranch, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetFalseBranch)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetFalseBranch();
        ASSERT_TRUE(CheckMockedApi("BBGetFalseBranch"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::RemoveAllInsts, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_RemoveAllInsts)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).RemoveAllInsts();
        ASSERT_TRUE(CheckMockedApi("BBRemoveAllInsts"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetNumberOfInstructions, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetNumberOfInstructions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetNumberOfInstructions();
        ASSERT_TRUE(CheckMockedApi("BBGetNumberOfInstructions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::GetImmediateDominator, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_GetImmediateDominator)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).GetImmediateDominator();
        ASSERT_TRUE(CheckMockedApi("BBGetImmediateDominator"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::CheckDominance, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_CheckDominance)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto bb = abckit::mock::helpers::GetMockBasicBlock(f);
        abckit::mock::helpers::GetMockBasicBlock(f).CheckDominance(bb);
        ASSERT_TRUE(CheckMockedApi("BBCheckDominance"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::VisitDominatedBlocks, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_VisitDominatedBlocks)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).VisitDominatedBlocks(
            []([[maybe_unused]] const abckit::BasicBlock &bb) { return false; });
        ASSERT_TRUE(CheckMockedApi("BBVisitDominatedBlocks"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsStart, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsStart)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsStart();
        ASSERT_TRUE(CheckMockedApi("BBIsStart"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsEnd, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsEnd)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsEnd();
        ASSERT_TRUE(CheckMockedApi("BBIsEnd"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsLoopHead, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsLoopHead)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsLoopHead();
        ASSERT_TRUE(CheckMockedApi("BBIsLoopHead"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsLoopPrehead, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsLoopPrehead)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsLoopPrehead();
        ASSERT_TRUE(CheckMockedApi("BBIsLoopPrehead"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsTryBegin, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsTryBegin)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsTryBegin();
        ASSERT_TRUE(CheckMockedApi("BBIsTryBegin"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsTry, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsTry)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsTry();
        ASSERT_TRUE(CheckMockedApi("BBIsTry"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsTryEnd, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsTryEnd)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsTryEnd();
        ASSERT_TRUE(CheckMockedApi("BBIsTryEnd"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsCatchBegin, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsCatchBegin)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsCatchBegin();
        ASSERT_TRUE(CheckMockedApi("BBIsCatchBegin"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::IsCatch, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_IsCatch)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).IsCatch();
        ASSERT_TRUE(CheckMockedApi("BBIsCatch"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::Dump, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_Dump)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).Dump(DEFAULT_I32);
        ASSERT_TRUE(CheckMockedApi("BBDump"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::SplitBlockAfterInstruction, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_SplitBlockAfterInstruction)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).SplitBlockAfterInstruction(
            abckit::mock::helpers::GetMockInstruction(f), DEFAULT_BOOL);
        ASSERT_TRUE(CheckMockedApi("BBSplitBlockAfterInstruction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::CreatePhi, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_CreatePhi)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).CreatePhi(abckit::mock::helpers::GetMockInstruction(f),
                                                              abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("BBCreatePhi"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=BasicBlock::CreateCatchPhi, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockTestBasicBlock, BasicBlock_CreateCatchPhi)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockBasicBlock(f).CreateCatchPhi(abckit::mock::helpers::GetMockInstruction(f),
                                                                   abckit::mock::helpers::GetMockInstruction(f));
        ASSERT_TRUE(CheckMockedApi("BBCreateCatchPhi"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::test