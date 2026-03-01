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

#include "assembly-parser.h"
#include "bytecode_optimizer/ir_interface.h"
#include "bytecode_optimizer/runtime_adapter.h"
#include "libpandabase/mem/arena_allocator.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/file.h"
#include "macros.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir_builder/ir_builder.h"

namespace panda::compiler {
class IrBuilderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    IrBuilderTest()
    {
        PoolManager::Initialize(PoolType::MALLOC);
    }

    ~IrBuilderTest()
    {
        PoolManager::Finalize();
    }

    template <class Callback>
    void TestBuildGraphFromFunc(pandasm::Program &prog, const char *methodName, const Callback &cb)
    {
        pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
        auto pfile = pandasm::AsmEmitter::Emit(prog, &maps);
        for (uint32_t id : pfile->GetClasses()) {
            panda_file::File::EntityId record_id {id};
            panda_file::ClassDataAccessor cda {*pfile, record_id};

            cda.EnumerateMethods([&prog, maps, methodName, &cb](panda_file::MethodDataAccessor &mda) {
                auto ir_interface = panda::bytecodeopt::BytecodeOptIrInterface(&maps, &prog);
                auto func_name = ir_interface.GetMethodIdByOffset(mda.GetMethodId().GetOffset());
                if (func_name != methodName) {
                    return;
                }

                ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
                ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

                auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(
                    mda.GetMethodId().GetOffset());
                panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
                auto *graph = allocator.New<Graph>(&allocator, &local_allocator, Arch::NONE, method_ptr, &adapter,
                                                   false, nullptr, true, true);
                graph->RunPass<panda::compiler::IrBuilder>();
                cb(graph);
            });
        }
    }

    template <class Callback>
    void TestBuildGraphFromFile(const std::string &pFileName, const Callback &cb)
    {
        auto pfile = panda_file::OpenPandaFile(pFileName);
        for (uint32_t id : pfile->GetClasses()) {
            panda_file::File::EntityId record_id {id};

            if (pfile->IsExternal(record_id)) {
                continue;
            }

            panda_file::ClassDataAccessor cda {*pfile, record_id};
            cda.EnumerateMethods([&pfile, &cb](panda_file::MethodDataAccessor &mda) {
                if (!mda.IsExternal()) {
                    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
                    ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

                    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(
                        mda.GetMethodId().GetOffset());
                    panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
                    auto *graph = allocator.New<Graph>(&allocator, &local_allocator, Arch::NONE, method_ptr, &adapter,
                                                       false, nullptr, true, true);
                    graph->RunPass<panda::compiler::IrBuilder>();

                    auto method_name = std::string(utf::Mutf8AsCString(pfile->GetStringData(mda.GetNameId()).data));
                    auto pos = method_name.find_last_of("#");
                    if (pos != std::string::npos) {
                        method_name = method_name.substr(pos + 1);
                    }
                    cb(graph, method_name);
                }
            });
        }
    }
};

HWTEST_F(IrBuilderTest, simpleTryCatchAsm, testing::ext::TestSize.Level1)
{
    /**
     *  try {
     *      a = 1;
     *  } catch(e) {
     *      a = 2;
     *  }
     */
    const auto source = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) {
            mov v0, a0
            mov v1, a1
            mov v2, a2
        try_begin:
            ldai 0x1
            trystglobalbyname 0x0, "a"
        try_end:
            jmp catch_end
        catch_begin:
            sta v4
            tryldglobalbyname 0x1, "a"
        catch_end:
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &prog = res.Value();
    for (auto &[name, func] : prog.function_table) {
        auto &catchBlock = func.catch_blocks.emplace_back();
        catchBlock.try_begin_label = "try_begin";
        catchBlock.try_end_label = "try_end";
        catchBlock.catch_begin_label = "catch_begin";
        catchBlock.catch_end_label = "catch_end";
    }

    TestBuildGraphFromFunc(prog, "foo:(any,any,any)", [](Graph* graph) {
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetSuccessor(0)->IsTry());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());

                EXPECT_TRUE(bb->GetFirstInst()->IsTry());
                continue;
            }

            if (bb->IsTry()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::TRYSTGLOBALBYNAME_IMM8_ID16);
                continue;
            }

            if (bb->IsTryEnd()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetSuccessor(0)->GetGuestPc() == bb->GetGuestPc());
                EXPECT_TRUE(!bb->GetSuccessor(0)->IsCatch());
                EXPECT_TRUE(!bb->GetSuccessor(0)->IsCatchBegin());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());

                EXPECT_TRUE(bb->IsEmpty());
                continue;
            }

            if (bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetPredecessor(1)->IsTryEnd());
                EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());

                EXPECT_TRUE(bb->IsEmpty());
                continue;
            }

            if (bb->IsCatch() && !bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());

                EXPECT_TRUE(bb->GetSuccessor(0)->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetSuccessor(0)->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetSuccessor(0)->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::RETURNUNDEFINED);

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                continue;
            }
        }
    });
}

HWTEST_F(IrBuilderTest, nestedTryCatchAsm, testing::ext::TestSize.Level1)
{
    /**
     *  try {
     *      try {
     *          a = 1;
     *      } catch(e) {
     *          a;
     *      }
     *  } catch(e) {
     *      print(e);
     *  }
     */
    const auto source = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) {
            mov v2, a2
            mov v1, a1
            mov v0, a0
            lda.str "use strict"
        LABEL_0:
        LABEL_1:
            ldai 1
            trystglobalbyname 0, "a"
        LABEL_2:
            jmp LABEL_3
        LABEL_4:
            sta v3
            tryldglobalbyname 1, "a"
        LABEL_3:
        LABEL_5:
            jmp LABEL_6
        LABEL_7:
            sta v4
            tryldglobalbyname 2, "print"
            sta v5
            lda v4
            sta v6
            lda v5
            callarg1 3, v6
        LABEL_6:
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &prog = res.Value();
    EXPECT_TRUE(prog.function_table.size() == 1);
    for (auto &[name, func] : prog.function_table) {
        auto &catchBlock1 = func.catch_blocks.emplace_back();
        catchBlock1.try_begin_label = "LABEL_0";
        catchBlock1.try_end_label = "LABEL_1";
        catchBlock1.catch_begin_label = "LABEL_7";
        catchBlock1.catch_end_label = "LABEL_6";

        auto &catchBlock2 = func.catch_blocks.emplace_back();
        catchBlock2.try_begin_label = "LABEL_1";
        catchBlock2.try_end_label = "LABEL_2";
        catchBlock2.catch_begin_label = "LABEL_4";
        catchBlock2.catch_end_label = "LABEL_3";

        auto &catchBlock3 = func.catch_blocks.emplace_back();
        catchBlock3.try_begin_label = "LABEL_2";
        catchBlock3.try_end_label = "LABEL_5";
        catchBlock3.catch_begin_label = "LABEL_7";
        catchBlock3.catch_end_label = "LABEL_6";
    }

    TestBuildGraphFromFunc(prog, "foo:(any,any,any)", [](Graph* graph) {
        EXPECT_NE(graph, nullptr);
        int32_t numOfTry = 0;
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                numOfTry++;

                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetSuccessor(0)->IsTry());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());

                EXPECT_TRUE(bb->GetSuccessor(0)->GetTryId() == bb->GetTryId());
                continue;
            }

            if (bb->IsTry()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                if (!bb->GetPredecessor(0)->IsTryBegin()) {
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                    EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                    EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                    EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                                RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                    continue;
                }


                EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == bb->GetTryId());
                EXPECT_TRUE(bb->GetPredecessor(0)->GetGuestPc() == bb->GetGuestPc());
                if (bb->GetPredecessor(0)->GetTryId() == 1) {
                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryEnd());

                    EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                    EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                    EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                                RuntimeInterface::IntrinsicId::TRYSTGLOBALBYNAME_IMM8_ID16);
                    continue;
                }

                EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == 2);
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                EXPECT_TRUE(bb->IsEmpty());

                for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                    EXPECT_TRUE(inst->IsPhi());
                }
                continue;
            }

            if (bb->IsTryEnd()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == bb->GetTryId());

                EXPECT_TRUE(bb->IsEmpty());

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                if (bb->GetTryId() == 1) {
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryBegin());
                    EXPECT_TRUE(bb->GetSuccessor(0)->GetGuestPc() == bb->GetGuestPc());

                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());

                    for (auto inst : bb->GetSuccessor(1)->AllInsts()) {
                        EXPECT_TRUE(inst->IsCatchPhi());
                    }
                    continue;
                }

                EXPECT_TRUE(bb->GetTryId() == 2);
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                    EXPECT_TRUE(inst->IsPhi());
                }
                continue;
            }

            if (bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetPredecessor(1)->IsTryEnd());

                EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());
                continue;
            }

            if (bb->IsCatch() && !bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);

                if (bb->IsTry()) {
                    EXPECT_TRUE(bb->GetTryId() == 2);

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryEnd());
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());

                    EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                    EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                    EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                                RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                    continue;
                }

                if (bb->IsTryEnd()) {
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                    for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                        EXPECT_TRUE(inst->IsPhi());
                    }
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());

                    EXPECT_TRUE(bb->IsEmpty());
                    continue;
                }

                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());
                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                auto *successor = bb->GetSuccessor(0);
                EXPECT_TRUE(successor->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(successor->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(successor->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::RETURNUNDEFINED);
                for (auto inst : successor->GetPredecessor(0)->AllInsts()) {
                    EXPECT_TRUE(inst->IsPhi());
                }

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::CALLARG1_IMM8_V8);
            }
        }

        EXPECT_EQ(numOfTry, 2);
    });
}

HWTEST_F(IrBuilderTest, simpleTryCatchAbc, testing::ext::TestSize.Level1)
{
    std::string pFile = GRAPH_TEST_ABC_DIR "simpleTryCatch.abc";
    const char *testMethodName = "func_main_0";
    TestBuildGraphFromFile(pFile, [&testMethodName](Graph* graph, std::string &methodName) {
        if (testMethodName != methodName) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetSuccessor(0)->IsTry());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());

                EXPECT_TRUE(bb->GetFirstInst()->IsTry());
                continue;
            }

            if (bb->IsTry()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::TRYSTGLOBALBYNAME_IMM8_ID16);
                continue;
            }

            if (bb->IsTryEnd()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetSuccessor(0)->GetGuestPc() == bb->GetGuestPc());
                EXPECT_TRUE(!bb->GetSuccessor(0)->IsCatch());
                EXPECT_TRUE(!bb->GetSuccessor(0)->IsCatchBegin());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());

                EXPECT_TRUE(bb->IsEmpty());
                continue;
            }

            if (bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetPredecessor(1)->IsTryEnd());
                EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());

                EXPECT_TRUE(bb->IsEmpty());
                continue;
            }

            if (bb->IsCatch() && !bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());

                EXPECT_TRUE(bb->GetSuccessor(0)->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetSuccessor(0)->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetSuccessor(0)->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::RETURNUNDEFINED);

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                continue;
            }
        }
    });
}

HWTEST_F(IrBuilderTest, nestedTryCatchAbc, testing::ext::TestSize.Level1)
{
    std::string pFile = GRAPH_TEST_ABC_DIR "nestedTryCatch.abc";
    const char *testMethodName = "func_main_0";
    TestBuildGraphFromFile(pFile, [testMethodName](Graph* graph, std::string &methodName) {
        if (testMethodName != methodName) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        int32_t numOfTry = 0;
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                numOfTry++;

                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);

                if (bb->GetTryId() == 0) {
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTry());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetSuccessor(0)->GetTryId() == bb->GetTryId());
                }

                if (bb->GetTryId() == 1) {
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryBegin());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                }

                continue;
            }

            if (bb->IsTry()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                if (bb->GetPredecessor(0)->IsTryBegin()) {
                    EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == bb->GetTryId());
                    EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == 0);
                    EXPECT_TRUE(bb->GetPredecessor(0)->GetGuestPc() == bb->GetGuestPc());

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryEnd());

                    EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                    EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                    EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                                RuntimeInterface::IntrinsicId::TRYSTGLOBALBYNAME_IMM8_ID16);
                    continue;
                }

                if (bb->GetPredecessor(0)->IsTryEnd()) {
                    EXPECT_FALSE(bb->GetPredecessor(0)->GetTryId() == bb->GetTryId());
                    EXPECT_TRUE(bb->GetPredecessor(0)->GetGuestPc() == bb->GetGuestPc());

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                    for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                        EXPECT_TRUE(inst->IsPhi());
                    }
                    continue;
                }

                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());
                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                continue;
            }

            if (bb->IsTryEnd()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                EXPECT_TRUE(bb->GetPredecessor(0)->GetTryId() == bb->GetTryId());

                EXPECT_TRUE(bb->IsEmpty());

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                if (bb->GetTryId() == 0) {
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTry());
                    EXPECT_TRUE(bb->GetSuccessor(0)->GetGuestPc() == bb->GetGuestPc());

                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());

                    for (auto inst : bb->GetSuccessor(1)->AllInsts()) {
                        EXPECT_TRUE(inst->IsCatchPhi());
                    }
                    continue;
                }

                EXPECT_TRUE(bb->GetTryId() == 1);
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());
                for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                    EXPECT_TRUE(inst->IsPhi());
                }
                continue;
            }

            if (bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 2);
                EXPECT_TRUE(bb->GetPredecessor(0)->IsTryBegin());
                EXPECT_TRUE(bb->GetPredecessor(1)->IsTryEnd());

                EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());
                continue;
            }

            if (bb->IsCatch() && !bb->IsCatchBegin()) {
                EXPECT_TRUE(bb->GetPredsBlocks().size() == 1);

                if (bb->IsTry()) {
                    EXPECT_TRUE(bb->GetTryId() == 2);

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsTryEnd());
                    EXPECT_TRUE(bb->GetSuccessor(0)->IsCatch());

                    EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                    EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                    EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                                RuntimeInterface::IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16);
                    continue;
                }

                if (bb->IsTryEnd()) {
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsTry());
                    EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                    EXPECT_TRUE(bb->GetSuccsBlocks().size() == 2);
                    for (auto inst : bb->GetSuccessor(0)->AllInsts()) {
                        EXPECT_TRUE(inst->IsPhi());
                    }
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatchBegin());
                    EXPECT_TRUE(bb->GetSuccessor(1)->IsCatch());

                    EXPECT_TRUE(bb->IsEmpty());
                    continue;
                }

                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatchBegin());
                EXPECT_TRUE(bb->GetPredecessor(0)->IsCatch());

                EXPECT_TRUE(bb->GetSuccsBlocks().size() == 1);
                auto *successor = bb->GetSuccessor(0);
                EXPECT_TRUE(successor->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(successor->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(successor->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::RETURNUNDEFINED);
                for (auto inst : successor->GetPredecessor(0)->AllInsts()) {
                    EXPECT_TRUE(inst->IsPhi());
                }

                EXPECT_TRUE(bb->GetFirstInst()->IsSaveState());
                EXPECT_TRUE(bb->GetLastInst()->IsIntrinsic());
                EXPECT_TRUE((static_cast<IntrinsicInst *>(bb->GetLastInst()))->GetIntrinsicId() ==
                            RuntimeInterface::IntrinsicId::CALLARG1_IMM8_V8);
            }
        }

        EXPECT_EQ(numOfTry, 2);
    });
}
} // namespace panda::compiler