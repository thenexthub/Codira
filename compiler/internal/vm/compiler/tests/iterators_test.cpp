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

#include <algorithm>
#include <vector>
#include "unit_test.h"

namespace panda::compiler {
class IteratorsTest : public GraphTest {
public:
    static constexpr size_t INST_COUNT = 10;

public:
    void Check(std::vector<Inst *> &tested_instructions)
    {
        auto block = &BB(0);
        PopulateBlock(block, tested_instructions);
        InitExpectData(tested_instructions);

        // Check InstForwardIterator
        std::vector<Inst *> result;
        for (auto inst : block->PhiInsts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_phis_);

        result.clear();
        for (auto inst : block->Insts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_insts_);

        result.clear();
        for (auto inst : block->AllInsts()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_all_);

        // Check InstForwardValidIterator
        result.clear();
        for (auto inst : block->PhiInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_phis_);

        result.clear();
        for (auto inst : block->InstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_insts_);

        result.clear();
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result, expect_all_);

        // Check InstBackwardValidIterator
        result.clear();
        for (auto inst : block->PhiInstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_phis_);

        result.clear();
        for (auto inst : block->InstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_insts_);

        result.clear();
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_all_);

        // Check InstForwardValidIterator with erasing instructions
        result.clear();
        for (auto inst : block->PhiInstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expect_phis_);

        result.clear();
        for (auto inst : block->InstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expect_insts_);

        result.clear();
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result.size(), 0U);

        PopulateBlock(block, tested_instructions);
        for (auto inst : block->AllInstsSafe()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        EXPECT_EQ(result, expect_all_);

        // Check InstBackwardValidIterator with erasing instructions
        PopulateBlock(block, tested_instructions);
        result.clear();
        for (auto inst : block->PhiInstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_phis_);

        result.clear();
        for (auto inst : block->InstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_insts_);

        result.clear();
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
        }
        EXPECT_EQ(result.size(), 0U);

        PopulateBlock(block, tested_instructions);
        for (auto inst : block->AllInstsSafeReverse()) {
            result.push_back(inst);
            block->EraseInst(inst);
        }
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expect_all_);
    }

private:
    void InitExpectData(std::vector<Inst *> &instructions)
    {
        expect_phis_.clear();
        expect_insts_.clear();
        expect_all_.clear();

        for (auto inst : instructions) {
            if (inst->IsPhi()) {
                expect_phis_.push_back(inst);
            } else {
                expect_insts_.push_back(inst);
            }
        }
        expect_all_.insert(expect_all_.end(), expect_phis_.begin(), expect_phis_.end());
        expect_all_.insert(expect_all_.end(), expect_insts_.begin(), expect_insts_.end());
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
    std::vector<Inst *> expect_phis_;
    std::vector<Inst *> expect_insts_;
    std::vector<Inst *> expect_all_;
};

TEST_F(IteratorsTest, EmptyBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions;
    Check(instructions);
}

TEST_F(IteratorsTest, BlockPhisInstructions)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
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
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
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
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    std::vector<Inst *> instructions(IteratorsTest::INST_COUNT);

    // first instruction is phi
    size_t i = 0;
    for (auto &inst : instructions) {
        if (++i % 2) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);
    // first instruction is not phi
    i = 1;
    for (auto &inst : instructions) {
        if (++i % 2) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);

    // first instructions are phi
    i = 0;
    for (auto &inst : instructions) {
        if (i < IteratorsTest::INST_COUNT / 2) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);

    // first instructions are not phi
    i = 0;
    for (auto &inst : instructions) {
        if (i >= IteratorsTest::INST_COUNT / 2) {
            inst = GetGraph()->CreateInst(Opcode::Phi);
        } else {
            inst = GetGraph()->CreateInst(Opcode::Add);
        }
    }
    Check(instructions);
}
}  // namespace panda::compiler
