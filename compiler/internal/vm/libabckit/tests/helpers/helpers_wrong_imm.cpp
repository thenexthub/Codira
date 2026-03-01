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

#include "helpers/helpers_wrong_imm.h"

#include "libabckit/c/abckit.h"

#include <gtest/gtest.h>

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers_wrong_imm {

// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
static AbckitCoreModule *g_dummyModule = new AbckitCoreModule();
static AbckitCoreFunction *g_dummyMethod = new AbckitCoreFunction();
static AbckitInst *g_dummyInsT1 = new AbckitInst();
static AbckitInst *g_dummyInsT2 = new AbckitInst();
static AbckitLiteralArray *g_dummyLitarr = (AbckitLiteralArray *)(0x1);
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

AbckitGraph *OpenWrongImmFile()
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "wrong_imm_tests/wrong_imm_test_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *foo = helpers::FindMethodByName(file, "foo");
    auto *graph = g_implI->createGraphFromFunction(foo);
    return graph;
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0), AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    auto *instr = apiToCheck(graph, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1), AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    auto *instr = apiToCheck(graph, 1UL << bitsize, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, 0x0, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, 1UL << bitsize, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                                            ...),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);

    // NOTE varargs processing
}
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, AbckitInst *inst1),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *inst1),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize, 0x0, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, g_dummyInsT1, 0x0, 1UL << bitsize, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize, 1UL << bitsize, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, g_dummyInsT1, 0x0, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    instr = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, AbckitInst *inst0, AbckitInst *inst1),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;

    auto *instr = apiToCheck(graph, 1UL << bitsize, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;

    auto *instr = apiToCheck(graph, g_dummyMethod, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method,
                                            uint64_t imm0),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    g_dummyInsT1->graph = graph;
    auto *instr = apiToCheck(graph, g_dummyInsT1, g_dummyMethod, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, 1UL << bitsize);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *litarr),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    AbckitInst *inst = apiToCheck(graph, 1UL << bitsize, g_dummyLitarr);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                            uint64_t imm0, AbckitInst *inst),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    g_dummyInsT1->graph = graph;

    auto *instr = apiToCheck(graph, g_dummyMethod, g_dummyLitarr, 1UL << bitsize, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, size_t argCount, ...), AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    auto *inst = apiToCheck(graph, 1UL << bitsize);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input, size_t argCount, ...),
                  AbckitBitImmSize bitsize)
{
    auto *graph = OpenWrongImmFile();
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, 1UL << bitsize);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

}  // namespace libabckit::test::helpers_wrong_imm
