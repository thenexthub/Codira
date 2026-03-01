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

#include "helpers_arithmetic.h"
#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"
#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers::arithmetic {

void TransformIrUnaryInstValid(AbckitGraph *graph,
                               AbckitInst *(*unaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0),
                               uint32_t operandId)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    AbckitInst *curInst = g_implG->bbGetFirstInst(startBB);

    AbckitInst *firstInst = nullptr;
    while (curInst != nullptr) {
        if (g_implG->iGetId(curInst) == operandId) {
            firstInst = curInst;
        }
        curInst = g_implG->iGetNext(curInst);
    }

    AbckitInst *retOp = nullptr;
    if (g_implG->gGetIsa(graph) == ABCKIT_ISA_TYPE_DYNAMIC) {
        retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    } else {
        retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    }
    ASSERT_NE(retOp, nullptr);
    AbckitInst *mainInst = unaryInstToCheck(graph, firstInst);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(retOp, mainInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static bool IsMulOrAdd(AbckitGraph *graph, AbckitInst *inst)
{
    if (g_implG->gGetIsa(graph) == ABCKIT_ISA_TYPE_DYNAMIC) {
        AbckitIsaApiDynamicOpcode curOpcode = g_dynG->iGetOpcode(inst);
        if (curOpcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_MUL2 && curOpcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2) {
            return true;
        }
    } else if (g_implG->gGetIsa(graph) == ABCKIT_ISA_TYPE_STATIC) {
        AbckitIsaApiStaticOpcode curOpcode = g_statG->iGetOpcode(inst);
        if (curOpcode != ABCKIT_ISA_API_STATIC_OPCODE_MUL && curOpcode != ABCKIT_ISA_API_STATIC_OPCODE_ADD) {
            return true;
        }
    } else {
        LIBABCKIT_UNREACHABLE;
    }
    return false;
}

void TransformIrBinInstrValid(AbckitGraph *graph,
                              AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                                               AbckitInst *input1),
                              uint32_t lhsId, uint32_t rhsId)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *curInst = g_implG->bbGetFirstInst(startBB);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *firstInst = nullptr;
    AbckitInst *secondInst = nullptr;
    while (curInst != nullptr) {
        if (g_implG->iGetId(curInst) == lhsId) {
            firstInst = curInst;
        }
        if (g_implG->iGetId(curInst) == rhsId) {
            secondInst = curInst;
        }
        curInst = g_implG->iGetNext(curInst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }
    auto *bb = helpers::BBgetSuccBlocks(startBB)[0];
    g_implG->bbGetFirstInst(bb);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    curInst = g_implG->bbGetFirstInst(bb);
    while (curInst != nullptr) {
        if (IsMulOrAdd(graph, curInst)) {
            curInst = g_implG->iGetNext(curInst);
            continue;
        }
        auto extraInst = curInst;
        curInst = g_implG->iGetNext(curInst);
        g_implG->iRemove(extraInst);
    }

    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    if (retOp == nullptr) {
        retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    }
    ASSERT_NE(retOp, nullptr);
    AbckitInst *mainInst = binaryInstToCheck(graph, firstInst, secondInst);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(retOp, mainInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrBinInstrWithImmValid(AbckitGraph *graph,
                                     AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                                                      uint64_t imm))
{
    uint64_t imm = 0x5;

    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    AbckitInst *curInst = g_implG->bbGetFirstInst(startBB);

    AbckitInst *firstInst = nullptr;
    AbckitInst *mainInst = nullptr;
    while (curInst != nullptr) {
        if (g_implG->iGetId(curInst) == 1) {
            firstInst = curInst;
        }
        curInst = g_implG->iGetNext(curInst);
    }
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    auto *bb = succBBs[0];
    curInst = g_implG->bbGetFirstInst(bb);

    while (curInst != nullptr) {
        if (IsMulOrAdd(graph, curInst)) {
            curInst = g_implG->iGetNext(curInst);
            continue;
        }
        auto extraInst = curInst;
        curInst = g_implG->iGetNext(curInst);
        g_implG->iRemove(extraInst);
    }

    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    mainInst = binaryInstToCheck(graph, firstInst, imm);
    ASSERT_NE(graph, nullptr);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    g_implG->iSetInput(retOp, mainInst, 0);
}

std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForBinary(AbckitIsaApiStaticOpcode opcode)
{
    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> instVector {};
    if (opcode != ABCKIT_ISA_API_STATIC_OPCODE_MOD && opcode != ABCKIT_ISA_API_STATIC_OPCODE_DIV) {
        instVector = {
            {4, opcode, {1, 2}},
            {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {4}},
        };
    } else {
        instVector = {
            {6, opcode, {1, 2}},
            {7, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {6}},
        };
    }
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
            {{0}, {2}, instVector},
            {{1}, {}, {}}};
}

std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynBinary(AbckitIsaApiDynamicOpcode opcode)
{
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> instVector {};
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> paramInstVector {};
    instVector = {
        {6, opcode, {4, 3}},
        {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {6}},
    };
    paramInstVector = {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                       {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                       {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                       {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                       {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}};
    return {{{}, {1}, paramInstVector}, {{0}, {2}, instVector}, {{1}, {}, {}}};
}

std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForBinaryWithImm(AbckitIsaApiStaticOpcode opcode)
{
    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> instVector {};
    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> startInstVector {};
    if (opcode != ABCKIT_ISA_API_STATIC_OPCODE_ADDI && opcode != ABCKIT_ISA_API_STATIC_OPCODE_SUBI &&
        opcode != ABCKIT_ISA_API_STATIC_OPCODE_MULI && opcode != ABCKIT_ISA_API_STATIC_OPCODE_DIVI &&
        opcode != ABCKIT_ISA_API_STATIC_OPCODE_MODI) {
        startInstVector = {{0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                           {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                           {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}};
    } else {
        startInstVector = {{0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                           {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                           {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                           {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}};
    }
    if (opcode != ABCKIT_ISA_API_STATIC_OPCODE_MODI && opcode != ABCKIT_ISA_API_STATIC_OPCODE_DIVI) {
        instVector = {
            {4, opcode, {1}},
            {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {4}},
        };
    } else {
        instVector = {
            {5, opcode, {1}},
            {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {5}},
        };
    }
    return {{{}, {1}, startInstVector}, {{0}, {2}, instVector}, {{1}, {}, {}}};
}

std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForUnary(AbckitIsaApiStaticOpcode opcode)
{
    return {
        {{}, {1}, {{0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
        {{0},
         {2},
         {
             {3, opcode, {1}},
             {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
         }},
        {{1}, {}, {}}};
}

std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynUnary(AbckitIsaApiDynamicOpcode opcode)
{
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
            {{0},
             {2},
             {
                 {5, opcode, {3}},
                 {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {5}},
             }},
            {{1}, {}, {}}};
}

}  // namespace libabckit::test::helpers::arithmetic
// NOLINTEND(readability-magic-numbers)
