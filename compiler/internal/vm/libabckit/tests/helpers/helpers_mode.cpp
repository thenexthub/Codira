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

#include "adapter_dynamic/abckit_dynamic.h"
#include "adapter_static/abckit_static.h"
#include "adapter_dynamic/metadata_inspect_dynamic.h"
#include "adapter_static/metadata_inspect_static.h"
#include "helpers/helpers.h"
#include "helpers/helpers_mode.h"

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "ir_impl.h"

#include <gtest/gtest.h>
#include <cstddef>

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers_mode {

// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
static AbckitCoreModule *g_dummyModule = new AbckitCoreModule();
static AbckitString *g_dummyString = (AbckitString *)(0x1);
static AbckitBasicBlock *g_dummyBb = new AbckitBasicBlock();
static AbckitCoreFunction *g_dummyMethod = new AbckitCoreFunction();
static AbckitLiteralArray *g_dummyLitarr = (AbckitLiteralArray *)(0x1);
static AbckitInst *g_dummyInsT1 = new AbckitInst();
static AbckitInst *g_dummyInsT2 = new AbckitInst();
static AbckitInst *g_dummyInsT3 = new AbckitInst();
static AbckitInst *g_dummyInsT4 = new AbckitInst();
static AbckitInst *g_dummyInsT5 = new AbckitInst();
static AbckitCoreImportDescriptor *g_dummyImport = (AbckitCoreImportDescriptor *)(0x1);
static AbckitCoreExportDescriptor *g_dummyExport = (AbckitCoreExportDescriptor *)(0x1);
static AbckitType *g_dummyType = (AbckitType *)(0x1);
static panda::pandasm::Function *g_dummyPandasmFunction = (panda::pandasm::Function *)(0x1);
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

AbckitGraph *OpenWrongModeFile(bool isDynamic)
{
    if (isDynamic) {
        constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "wrong_mode_tests/mode_test_static.abc";
        auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
        auto *foo = helpers::FindMethodByName(file, "foo");
        auto *graph = CreateGraphFromFunctionStatic(foo);
        return graph;
    }
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "wrong_mode_tests/mode_test_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *foo = helpers::FindMethodByName(file, "foo");
    auto *graph = g_implI->createGraphFromFunction(foo);
    return graph;
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph);

    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(void (*apiToCheck)(AbckitGraph *graph), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    apiToCheck(graph);

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(void (*apiToCheck)(AbckitGraph *graph, int fd), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    apiToCheck(graph, 0);

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint32_t index), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, int64_t value), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    int64_t val = 0;
    AbckitInst *inst = apiToCheck(graph, val);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t value), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    uint64_t val = 0;
    AbckitInst *inst = apiToCheck(graph, val);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph, 0, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitInst *str), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, 0, g_dummyInsT1);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitLiteralArray *str), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitInst *inst = apiToCheck(graph, 0, g_dummyLitarr);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t value, AbckitInst *inst1, AbckitInst *inst2),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    auto *inst = apiToCheck(graph, 0, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, double value), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    double val = 0;
    AbckitInst *inst = apiToCheck(graph, val);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitString *str), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitInst *inst = apiToCheck(graph, g_dummyString);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitString *str, uint64_t imm), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitInst *inst = apiToCheck(graph, g_dummyString, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreExportDescriptor *e), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitInst *inst = apiToCheck(graph, g_dummyExport);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreImportDescriptor *i), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitInst *inst = apiToCheck(graph, g_dummyImport);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                        AbckitTypeId typeId),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreExportDescriptor *e),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, g_dummyExport);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitIsaApiDynamicConditionCode cc),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, 0, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, AbckitInst *inst1),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, 0, g_dummyInsT2);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1,
                                        AbckitInst *input1),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, 0, 0, g_dummyInsT2);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, AbckitCoreFunction *method,
                                        size_t argCount, ...),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyMethod, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input, size_t argCount, ...), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, ...),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, size_t argCount, ...), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitBasicBlock *catchBegin, size_t argCount, ...),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph, g_dummyBb, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    auto *inst = apiToCheck(graph, g_dummyMethod, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, size_t argCount, ...),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    auto *inst = apiToCheck(graph, g_dummyMethod, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreModule *m), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyModule->file = graph->file;
    auto *inst = apiToCheck(graph, g_dummyModule);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitArktsClass arktsClass(g_dummyPandasmFunction);
    AbckitCoreClass klass(g_dummyModule, arktsClass);
    g_dummyModule->file = graph->file;
    auto *inst = apiToCheck(graph, &klass);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass1, AbckitCoreClass *klass2),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    AbckitArktsClass arktsClass(g_dummyPandasmFunction);
    AbckitCoreClass klass(g_dummyModule, arktsClass);
    g_dummyModule->file = graph->file;
    auto *inst = apiToCheck(graph, &klass, &klass);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass, AbckitInst *inst), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitArktsClass arktsClass(g_dummyPandasmFunction);
    AbckitCoreClass klass(g_dummyModule, arktsClass);
    g_dummyModule->file = graph->file;
    auto *inst = apiToCheck(graph, &klass, g_dummyInsT1);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitLiteralArray *litarr), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    auto *inst = apiToCheck(graph, g_dummyLitarr);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                        uint64_t val, AbckitInst *inst),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    auto *inst = apiToCheck(graph, g_dummyMethod, g_dummyLitarr, 0, g_dummyInsT1);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, AbckitTypeId::ABCKIT_TYPE_ID_I8);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                        AbckitIsaApiStaticConditionCode),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    AbckitInst *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1), bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyString);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    g_dummyInsT3->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, g_dummyInsT3);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                                        AbckitInst *input3),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    g_dummyInsT3->graph = graph;
    g_dummyInsT4->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, g_dummyInsT3, g_dummyInsT4);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                                        AbckitInst *input3, AbckitInst *input4),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    g_dummyInsT3->graph = graph;
    g_dummyInsT4->graph = graph;
    g_dummyInsT5->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, g_dummyInsT3, g_dummyInsT4, g_dummyInsT5);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1,
                                        AbckitInst *input2),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyString, g_dummyInsT2);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method, uint64_t imm0),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyModule->file = graph->file;
    g_dummyMethod->owningModule = g_dummyModule;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyMethod, 0);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                        AbckitTypeId valueTypeId),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    g_dummyInsT2->graph = graph;
    g_dummyInsT3->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyInsT2, g_dummyInsT3, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyType);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field,
                                        AbckitTypeId returnTypeId),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyString, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                        AbckitInst *value, AbckitTypeId typeId),
              bool isDynamic)
{
    auto *graph = OpenWrongModeFile(isDynamic);
    g_dummyInsT1->graph = graph;
    auto *inst = apiToCheck(graph, g_dummyInsT1, g_dummyString, g_dummyInsT1, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(inst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_MODE);
    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

}  // namespace libabckit::test::helpers_mode
