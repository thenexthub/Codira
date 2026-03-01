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

#include <gtest/gtest.h>

#include "assembler/assembly-emitter.h"
#include "bytecode_optimizer/constant_propagation/constant_propagation.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/tests/graph_test.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/cleanup.h"

using namespace testing::ext;

namespace panda::bytecodeopt {

enum class ExpectType {
    EXPECT_LDTRUE,
    EXPECT_LDFALSE,
    EXPECT_BOTH_TRUE_FALSE,
    EXPECT_CONST,
    EXPECT_PHI,
    EXPECT_OTHER,
};

static void VisitBlock(compiler::BasicBlock *bb, ExpectType type);

class ConstantProgagationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    compiler::GraphTest graph_test_;

    bool CheckFunction(const std::string &file, const char *test_method_name, ExpectType before, ExpectType after,
                       std::unordered_map<uint32_t, std::string> *strings = nullptr)
    {
        bool status = false;

        graph_test_.TestBuildGraphFromFile(file, [test_method_name, before, after, strings, &status]
            (compiler::Graph *graph, std::string &method_name) {
            if (test_method_name != method_name) {
                return;
            }
            status = true;
            EXPECT_NE(graph, nullptr);
            EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());
            EXPECT_TRUE(graph->RunPass<compiler::DominatorsTree>());
            auto &bbs = graph->GetVectorBlocks();
            for (auto bb : bbs) {
                VisitBlock(bb, before);
            }

            pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
            pandasm::Program *prog = nullptr;
            if (strings != nullptr) {
                maps.strings = *strings;
            }
            BytecodeOptIrInterface interface(&maps, prog);
            EXPECT_TRUE(graph->RunPass<ConstantPropagation>(&interface));
            auto modified = graph->RunPass<compiler::Cleanup>();
            if (before != after) {
                EXPECT_TRUE(modified);
            }
            for (auto bb : bbs) {
                VisitBlock(bb, after);
            }
        });
        return status;
    }
};

static void VisitBlock(BasicBlock *bb, ExpectType type)
{
    if (!bb) {
        return;
    }
    for (auto inst : bb->Insts()) {
        if (!inst->IsIntrinsic() ||
            inst->CastToIntrinsic()->GetIntrinsicId() != RuntimeInterface::IntrinsicId::CALLARG1_IMM8_V8) {
            continue;
        }
        auto input = inst->GetInput(0).GetInst();

        switch (type) {
            case ExpectType::EXPECT_CONST:
                ASSERT_TRUE(input->IsConst());
                break;
            case ExpectType::EXPECT_PHI:
                ASSERT_TRUE(input->IsPhi());
                break;
            case ExpectType::EXPECT_LDTRUE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
            case ExpectType::EXPECT_LDFALSE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE);
                break;
            }
            case ExpectType::EXPECT_BOTH_TRUE_FALSE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE ||
                            id == RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
            case ExpectType::EXPECT_OTHER: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id != RuntimeInterface::IntrinsicId::LDFALSE &&
                            id != RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
        }
    }
}

static void VisitBlockCheckIf(BasicBlock *bb, bool optimize = false)
{
    if (!bb) {
        return;
    }
    for (auto inst : bb->Insts()) {
        if (inst->GetOpcode() != Opcode::IfImm) {
            continue;
        }
        auto input = inst->GetInput(0).GetInst();
        if (!optimize) {
            EXPECT_TRUE(input->GetOpcode() == Opcode::Compare);
        } else {
            auto id = input->CastToIntrinsic()->GetIntrinsicId();
            EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE || id == RuntimeInterface::IntrinsicId::LDTRUE);
        }
    }
}

/**
 * @tc.name: constant_progagation_test_001
 * @tc.desc: Verify the pass to fold greater.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_001, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreater";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_002
 * @tc.desc: Verify the pass to fold greater but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_002, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_003
 * @tc.desc: Verify the pass to fold greatereq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_003, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_004
 * @tc.desc: Verify the pass to fold greatereq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_004, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterEqNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_005
 * @tc.desc: Verify the pass to fold less.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_005, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLess";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_006
 * @tc.desc: Verify the pass to fold less but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_006, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_007
 * @tc.desc: Verify the pass to fold lesseq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_007, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_008
 * @tc.desc: Verify the pass to fold lesseq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_008, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessEqNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_009
 * @tc.desc: Verify the pass to fold eq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_009, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_010
 * @tc.desc: Verify the pass to fold eq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_010, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldEqNoEffect";
    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_011
 * @tc.desc: Verify the pass to fold stricteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_011, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_012
 * @tc.desc: Verify the pass to fold eq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_012, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEqNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_013
 * @tc.desc: Verify the pass to fold noteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_013, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_014
 * @tc.desc: Verify the pass to fold noteq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_014, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEqNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_015
 * @tc.desc: Verify the pass to fold strictnoteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_015, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictNotEq";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_016
 * @tc.desc: Verify the pass to fold strictnoteq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_016, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictNotEqNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_017
 * @tc.desc: Verify the pass without any effect with nan.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_017, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";

    EXPECT_TRUE(CheckFunction(file, "sccpNoEffectNanWithInt",
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
    EXPECT_TRUE(CheckFunction(file, "sccpNoEffectNanWithBoolAndDouble",
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_018
 * @tc.desc: Verify the optimizion to Compare.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_018, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpIfCheck";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](compiler::Graph *graph,
                                                                         std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());
        EXPECT_TRUE(graph->RunPass<compiler::DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlockCheckIf(bb, false);
        }
        pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
        pandasm::Program *prog = nullptr;
        BytecodeOptIrInterface interface(&maps, prog);
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>(&interface));
        EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());
        for (auto bb : bbs) {
            VisitBlockCheckIf(bb, true);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_019
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_019, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhi";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_PHI, ExpectType::EXPECT_BOTH_TRUE_FALSE));
}

/**
 * @tc.name: constant_progagation_test_020
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_020, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhiNoEffect";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_PHI, ExpectType::EXPECT_PHI));
}

/**
 * @tc.name: constant_progagation_test_021
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_021, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhi2";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_PHI, ExpectType::EXPECT_CONST));
}

/**
 * @tc.name: constant_progagation_test_022
 * @tc.desc: Verify the pass without any effect with inf.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_022, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";

    EXPECT_TRUE(CheckFunction(file, "sccpNoEffectInfinityWithInt",
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
    EXPECT_TRUE(CheckFunction(file, "sccpNoEffectInfinityWithBoolAndDouble",
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}

/**
 * @tc.name: constant_progagation_test_023
 * @tc.desc: Verify the pass without any effect with math calculation.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_023, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpNoEffectArithmetic";

    EXPECT_TRUE(CheckFunction(file, test_method_name,
                              ExpectType::EXPECT_OTHER, ExpectType::EXPECT_OTHER));
}
}  // namespace panda::bytecodeopt

