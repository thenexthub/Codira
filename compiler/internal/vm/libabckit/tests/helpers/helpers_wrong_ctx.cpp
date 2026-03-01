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

#include "libabckit/tests/helpers/helpers_wrong_ctx.h"
#include "helpers/helpers.h"

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "metadata_inspect_impl.h"
#include "ir_impl.h"

#include <gtest/gtest.h>

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers_wrong_ctx {

// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
static AbckitGraph *g_dummyGrapH1 = new AbckitGraph();
static AbckitGraph *g_dummyGrapH2 = new AbckitGraph();

static AbckitCoreModule *g_dummyModulE1 = new AbckitCoreModule();
static AbckitCoreModule *g_dummyModulE2 = new AbckitCoreModule();

static AbckitCoreFunction *g_dummyMethoD1 = new AbckitCoreFunction();
static AbckitCoreFunction *g_dummyMethoD2 = new AbckitCoreFunction();

static AbckitFile *g_dummyFilE1 = (AbckitFile *)(0x1);
static AbckitFile *g_dummyFilE2 = (AbckitFile *)(0x2);

static AbckitInst *g_dummyInsT1 = new AbckitInst();
static AbckitInst *g_dummyInsT2 = new AbckitInst(g_dummyGrapH2, (ark::compiler::Inst *)0x1);
static panda::pandasm::Function *g_dummyPandasmFunction = (panda::pandasm::Function *)(0x1);

static AbckitString *g_dummyString = (AbckitString *)(0x1);

static AbckitBasicBlock *g_dummyBB1 = new AbckitBasicBlock();
static AbckitBasicBlock *g_dummyBB2 = new AbckitBasicBlock();

static AbckitLiteralArray *g_dummyLitarr = (AbckitLiteralArray *)(0x1);

static AbckitCoreExportDescriptor *g_dummyExport = (AbckitCoreExportDescriptor *)(0x1);

static AbckitType *g_dummyType = (AbckitType *)(0x1);
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

void TestWrongCtx(void (*apiToCheck)(AbckitCoreFunction *method, AbckitGraph *code))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD2->owningModule = g_dummyModulE2;

    apiToCheck(g_dummyMethoD2, g_dummyGrapH1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitInst *inst))
{
    g_dummyBB1->graph = g_dummyGrapH1;
    g_dummyBB2->graph = g_dummyGrapH2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyBB1, g_dummyInsT2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(bool (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2))
{
    g_dummyBB1->graph = g_dummyGrapH1;
    g_dummyBB2->graph = g_dummyGrapH2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyBB1, g_dummyBB2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2, bool val))
{
    g_dummyBB1->graph = g_dummyGrapH1;
    g_dummyBB2->graph = g_dummyGrapH2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyBB1, g_dummyBB2, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2))
{
    g_dummyBB1->graph = g_dummyGrapH1;
    g_dummyBB2->graph = g_dummyGrapH2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyBB1, g_dummyBB2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2, uint32_t index))
{
    g_dummyBB1->graph = g_dummyGrapH1;
    g_dummyBB2->graph = g_dummyGrapH2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyBB1, g_dummyBB2, 0x0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(bool (*apiToCheck)(AbckitInst *inst, AbckitInst *dominator))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    bool var = apiToCheck(g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(var, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, size_t argCount, ...))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD1->owningModule = g_dummyModulE1;
    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyMethoD2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    // NOTE varargs processing
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2, AbckitInst *input3))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, size_t argCount, ...))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    // NOTE varargs processing
}
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                                            ...))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    // NOTE varargs processing
}
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, AbckitInst *inst1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, 0x0, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, 0x0, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *inst1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, 0x0, 0x0, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, 0x0, 0x0, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                            uint64_t val, AbckitInst *inst))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD1->owningModule = g_dummyModulE1;
    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyMethoD2, g_dummyLitarr, 0x0, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyMethoD1, g_dummyLitarr, 0x0, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, 0x0, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2, AbckitInst *input3, AbckitInst *input4))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitInst *inst0, AbckitInst *inst1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, 0x0, g_dummyInsT2, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, 0x0, g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1,
                                            AbckitInst *input2))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyString, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyString, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD1->owningModule = g_dummyModulE1;
    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyMethoD2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method,
                                            uint64_t imm0))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD1->owningModule = g_dummyModulE1;
    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyMethoD1, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyMethoD2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreModule *m))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyGrapH1->file = g_dummyFilE1;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyModulE2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                            AbckitIsaApiDynamicConditionCode cc))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyString);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                            AbckitInst *value, AbckitTypeId valueTypeId))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, g_dummyInsT1, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, g_dummyInsT1, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT1, g_dummyInsT2, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitTypeId typeId))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *field,
                                            AbckitTypeId returnTypeId))
{
    g_dummyInsT1->graph = g_dummyGrapH1;

    auto *instr = apiToCheck(g_dummyGrapH2, g_dummyInsT1, g_dummyString, ABCKIT_TYPE_ID_I32);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreExportDescriptor *e))
{
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyExport);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass))
{
    g_dummyModulE2->file = g_dummyFilE2;

    AbckitArktsClass arktsClass(g_dummyPandasmFunction);
    AbckitCoreClass klass(g_dummyModulE2, arktsClass);

    g_dummyGrapH1->file = g_dummyFilE1;

    apiToCheck(g_dummyGrapH1, &klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitInst *inst, AbckitInst *next))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    apiToCheck(g_dummyInsT1, g_dummyInsT2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitIsaApiStaticConditionCode))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyInsT1, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyInsT2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass, AbckitInst *input0))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    AbckitArktsClass arktsClass(g_dummyPandasmFunction);
    AbckitCoreClass clasS1(g_dummyModulE2, arktsClass);

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, &clasS1, g_dummyInsT1);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    AbckitCoreClass clasS2(g_dummyModulE1, arktsClass);

    instr = apiToCheck(g_dummyGrapH1, &clasS2, g_dummyInsT2);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(void (*apiToCheck)(AbckitInst *input0, AbckitCoreFunction *method))
{
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyInsT1->graph = g_dummyGrapH1;

    g_dummyGrapH1->file = g_dummyFilE1;

    apiToCheck(g_dummyInsT1, g_dummyMethoD2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitType *type))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyType);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreFunction *method,
                                            size_t argCount, ...))
{
    g_dummyModulE1->file = g_dummyFilE1;
    g_dummyModulE2->file = g_dummyFilE2;

    g_dummyMethoD1->owningModule = g_dummyModulE1;
    g_dummyMethoD2->owningModule = g_dummyModulE2;

    g_dummyInsT1->graph = g_dummyGrapH1;
    g_dummyInsT2->graph = g_dummyGrapH2;

    g_dummyGrapH1->file = g_dummyFilE1;
    g_dummyGrapH2->file = g_dummyFilE2;

    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyMethoD2, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyMethoD1, 0x0);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                            AbckitInst *value, AbckitTypeId typeId))
{
    g_dummyInsT1->graph = g_dummyGrapH1;
    auto *instr = apiToCheck(g_dummyGrapH1, g_dummyInsT2, g_dummyString, g_dummyInsT1, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);

    instr = apiToCheck(g_dummyGrapH1, g_dummyInsT1, g_dummyString, g_dummyInsT2, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
}

}  // namespace libabckit::test::helpers_wrong_ctx
