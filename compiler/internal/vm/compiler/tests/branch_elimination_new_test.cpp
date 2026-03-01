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

#include <cstdint>
#include <set>
#include "bytecode_optimizer/constant_propagation/constant_propagation.h"
#include "bytecode_optimizer/ir_interface.h"
#include "compiler/tests/graph_test.h"
#include "gtest/gtest.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/cleanup.h"

using namespace testing::ext;

namespace panda::compiler {
class BranchEliminationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    static bool IsIntrinsicConstInst(Inst *inst)
    {
        ASSERT(inst != nullptr);
        if (!inst->IsIntrinsic()) {
            return false;
        }

        auto intrinsic_id = inst->CastToIntrinsic()->GetIntrinsicId();
        return intrinsic_id == IntrinsicInst::IntrinsicId::LDTRUE ||
               intrinsic_id == IntrinsicInst::IntrinsicId::LDFALSE;
    }

    static bool IsIfWithConstInputs(Inst *inst)
    {
        ASSERT(inst != nullptr);
        if (inst->GetOpcode() != Opcode::IfImm) {
            return false;
        }

        auto input_inst = inst->GetInput(0).GetInst();
        return input_inst->IsConst() || IsIntrinsicConstInst(input_inst);
    }

    static bool GetConstValue(Inst *inst)
    {
        ASSERT(inst != nullptr);
        if (inst->IsConst()) {
            auto const_value = inst->CastToConstant()->GetIntValue();
            ASSERT(const_value <= 1);
            return static_cast<bool>(const_value);
        } else if (IsIntrinsicConstInst(inst)) {
            return inst->CastToIntrinsic()->GetIntrinsicId() == IntrinsicInst::IntrinsicId::LDTRUE;
        } else {
            UNREACHABLE();
        }
    }

    static BasicBlock* GetDeadBranch(IfImmInst *inst)
    {
        auto input_inst = inst->GetInput(0).GetInst();
        ASSERT(input_inst->IsConst() || IsIntrinsicConstInst(input_inst));

        bool const_value = GetConstValue(input_inst);
        bool cond_result = (const_value == inst->GetImm());
        if (inst->GetCc() == CC_NE) {
            cond_result = !cond_result;
        } else {
            ASSERT(inst->GetCc() == CC_EQ);
        }

        if (cond_result) {
            return inst->GetEdgeIfInputFalse();
        } else {
            return inst->GetEdgeIfInputTrue();
        }
    }

    static void CollectDominatedDeadBlocks(Graph *graph, std::set<uint32_t> &dead_blocks, IfImmInst *dead_if_inst)
    {
        ASSERT(graph != nullptr);
        ASSERT(dead_if_inst != nullptr);

        // Collect dead blocks that need to be eliminated.
        auto dead_bb = GetDeadBranch(dead_if_inst);
        dead_blocks.insert(dead_bb->GetId());
        for (auto dom_bb : graph->GetBlocksRPO()) {
            if (!dead_bb->IsDominate(dom_bb)) {
                continue;
            }

            dead_blocks.insert(dom_bb->GetId());
        }
    }

    static void CollectDeadBlocksWithIfInst(Graph *graph, std::set<uint32_t> &dead_if_insts,
                                            std::set<uint32_t> &dead_blocks)
    {
        ASSERT(graph != nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (!IsIfWithConstInputs(inst)) {
                    continue;
                }

                dead_if_insts.insert(inst->GetId());
                CollectDominatedDeadBlocks(graph, dead_blocks, inst->CastToIfImm());
            }
        }
    }

    GraphTest graph_test_;
};

/**
 * @tc.name: branch_elimination_test_001
 * @tc.desc: Verify branch elimination.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BranchEliminationTest, branch_elimination_test_001, TestSize.Level1)
{
    std::string pfile_unopt = GRAPH_TEST_ABC_DIR "branchElimination.abc";
    options.SetCompilerUseSafepoint(false);
    graph_test_.TestBuildGraphFromFile(pfile_unopt, [](Graph* graph, std::string &method_name) {
        if (method_name == "func_main_0") {
            return;
        }
        EXPECT_NE(graph, nullptr);
        pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
        pandasm::Program *prog = nullptr;
        bytecodeopt::BytecodeOptIrInterface interface(&maps, prog);
        graph->RunPass<bytecodeopt::ConstantPropagation>(&interface);

        std::set<uint32_t> dead_if_insts;
        std::set<uint32_t> dead_blocks;
        CollectDeadBlocksWithIfInst(graph, dead_if_insts, dead_blocks);

        EXPECT_FALSE(dead_if_insts.empty());
        EXPECT_FALSE(dead_blocks.empty());

        graph->RunPass<BranchElimination>();
        graph->RunPass<Cleanup>();
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_TRUE(dead_blocks.count(bb->GetId()) == 0);
            for (auto inst : bb->AllInsts()) {
                EXPECT_TRUE(dead_if_insts.count(inst->GetId()) == 0);
            }
        }
    });
}
}  // namespace panda::compiler
