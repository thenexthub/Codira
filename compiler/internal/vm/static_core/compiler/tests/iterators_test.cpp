/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "unit_test.h"
#include <algorithm>
#include <vector>

namespace ark::compiler {
class IteratorsTest : public GraphTest {
public:
    static constexpr size_t INST_COUNT = 10;

public:
    void CheckInstForwardIterator(BasicBlock *block, std::vector<Inst *> &result)
    {
        result.clear();
        for (auto inst : block->PhiInsts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectPhis_);

        result.clear();
        for (auto inst : block->Insts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectInsts_);

        result.clear();
        for (auto inst : block->AllInsts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectAll_);
    }

    void CheckInstForwardValidIterator(BasicBlock *block, std::vector<Inst *> &result)
    {
        result.clear();
        for (auto inst : block->PhiInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectPhis_);

        result.clear();
        for (auto inst : block->InstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectInsts_);

        result.clear();
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expectAll_);
    }

    void CheckInstBackwardValidIterator(BasicBlock *block, std::vector<Inst *> &result)
    {
        result.clear();
        for (auto inst : block->PhiInstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectPhis_);

        result.clear();
        for (auto inst : block->InstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectInsts_);

        result.clear();
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectAll_);
    }

    void CheckInstForwardValidIteratorWithErasing(BasicBlock *block, std::vector<Inst *> &result,
                                                  std::vector<Inst *> &testedInstructions)
    {
        result.clear();
        for (auto inst : block->PhiInstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expectPhis_);

        result.clear();
        for (auto inst : block->InstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expectInsts_);

        result.clear();
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result.size(), 0U);

        PopulateBlock(block, testedInstructions);
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expectAll_);
    }

    void CheckInstBackwardValidIteratorWithErasing(BasicBlock *block, std::vector<Inst *> &result,
                                                   std::vector<Inst *> &testedInstructions)
    {
        PopulateBlock(block, testedInstructions);
        result.clear();
        for (auto inst : block->PhiInstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectPhis_);

        result.clear();
        for (auto inst : block->InstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectInsts_);

        result.clear();
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result.size(), 0U);

        PopulateBlock(block, testedInstructions);
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expectAll_);
    }

    void Check(std::vector<Inst *> &testedInstructions)
    {
        auto block = &BB(0U);
        PopulateBlock(block, testedInstructions);
        InitExpectData(testedInstructions);

        std::vector<Inst *> result;
        CheckInstForwardIterator(block, result);
        CheckInstForwardValidIterator(block, result);
        CheckInstBackwardValidIterator(block, result);
        // Check InstForwardValidIterator with erasing instructions
        CheckInstForwardValidIteratorWithErasing(block, result, testedInstructions);
        // Check InstBackwardValidIterator with erasing instructions
        CheckInstBackwardValidIteratorWithErasing(block, result, testedInstructions);
    }

private:
    void InitExpectData(std::vector<Inst *> &instructions)
    {
        expectPhis_.clear();
        expectInsts_.clear();
        expectAll_.clear();

        for (auto inst : instructions) {
            if (inst->IsPhi()) {
                expectPhis_.push_back(inst);
            } else {
                expectInsts_.push_back(inst);
            }
        }
        expectAll_.insert(expectAll_.end(), expectPhis_.begin(), expectPhis_.end());
        expectAll_.insert(expectAll_.end(), expectInsts_.begin(), expectInsts_.end());
    }

    void PopulateBlock(BasicBlock *block, std::vector<Inst *> &instructions)
    {
        for (auto inst : instructions) {
            if (inst->IsPhi()) {
                block->AppendPhi(inst);
            } else {
                block->AppendInst(inst);
            }
        }
    }

private:
    std::vector<Inst *> expectPhis_;
    std::vector<Inst *> expectInsts_;
    std::vector<Inst *> expectAll_;
};

TEST_F(IteratorsTest, EmptyBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions;
    Check(instructions);
}

TEST_F(IteratorsTest, BlockPhisInstructions)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions(IteratorsTest::INST_COUNT);
    for (auto &inst : instructions) {
        inst = GetGraph()->CreateInst(Opcode::Phi);
    }
    Check(instructions);
}

TEST_F(IteratorsTest, BlockNotPhisInstructions)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions(IteratorsTest::INST_COUNT);
    for (auto &inst : instructions) {
        inst = GetGraph()->CreateInst(Opcode::Add);
    }
    Check(instructions);
}

TEST_F(IteratorsTest, BlockAllInstructions)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions(IteratorsTest::INST_COUNT);

    // first instruction is phi
    size_t i = 0;
    for (auto &inst : instructions) {
        if ((++i % 2U) != 0U) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);
    // first instruction is not phi
    i = 1;
    for (auto &inst : instructions) {
        if ((++i % 2U) != 0U) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);

    // first instructions are phi
    i = 0;
    for (auto &inst : instructions) {
        if (i < IteratorsTest::INST_COUNT / 2U) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);

    // first instructions are not phi
    i = 0;
    for (auto &inst : instructions) {
        if (i >= IteratorsTest::INST_COUNT / 2U) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);
}
}  // namespace ark::compiler
