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

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

namespace libabckit::test {

class LibAbcKitInternalTest : public ::testing::Test {};

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct UserData {
    AbckitString *print = nullptr;
    AbckitString *date = nullptr;
    AbckitString *getTime = nullptr;
    AbckitString *str = nullptr;
    AbckitString *consume = nullptr;
};

struct ICSlotDescription {
    AbckitIsaApiDynamicOpcode op = AbckitIsaApiDynamicOpcode::ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID;
    int32_t icSlot = -1;
};

static void CheckICSlots(AbckitBasicBlock *bb, std::vector<ICSlotDescription> icSlotDescr)
{
    ASSERT_EQ(g_implG->bbGetNumberOfInstructions(bb), icSlotDescr.size());

    uint32_t counter = 0;
    for (AbckitInst *inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst), ++counter) {
        if (icSlotDescr[counter].icSlot == -1) {
            continue;
        }

        ASSERT_EQ(g_dynG->iGetOpcode(inst), icSlotDescr[counter].op);
        ASSERT_NE(g_implG->iGetImmediateCount(inst), 0);
        ASSERT_EQ(g_implG->iGetImmediate(inst, 0), icSlotDescr[counter].icSlot);
    }
}

static void TransformIrEpilog(AbckitGraph *graph, UserData *userData, AbckitInst *time, AbckitInst *callInst)
{
    auto *dateClass2 = g_dynG->iCreateTryldglobalbyname(graph, userData->date);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateClass2, callInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *dateObj2 = g_dynG->iCreateNewobjrange(graph, 1, dateClass2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateObj2, dateClass2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *getTime2 = g_dynG->iCreateLdobjbyname(graph, dateObj2, userData->getTime);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(getTime2, dateObj2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *time2 = g_dynG->iCreateCallthis0(graph, getTime2, dateObj2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(time2, getTime2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *consume = g_dynG->iCreateLoadString(graph, userData->consume);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(consume, time2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *print2 = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(print2, consume);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *callArg2 = g_dynG->iCreateCallarg1(graph, print2, consume);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(callArg2, print2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *sub = g_dynG->iCreateSub2(graph, time, time2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(sub, callArg2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *print3 = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(print3, sub);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *callArg3 = g_dynG->iCreateCallarg1(graph, print3, sub);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(callArg3, print3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    auto *bb = succBBs[0];
    auto *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);

    // Prolog
    auto *str = g_dynG->iCreateLoadString(graph, userData->str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(bb, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *print = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(print, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *callArg = g_dynG->iCreateCallarg1(graph, print, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(callArg, print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *dateClass = g_dynG->iCreateTryldglobalbyname(graph, userData->date);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateClass, callArg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *dateObj = g_dynG->iCreateNewobjrange(graph, 1, dateClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateObj, dateClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *getTime = g_dynG->iCreateLdobjbyname(graph, dateObj, userData->getTime);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(getTime, dateObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *time = g_dynG->iCreateCallthis0(graph, getTime, dateObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(time, getTime);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    TransformIrEpilog(graph, userData, time, callInst);
}

// Test: test-kind=internal, abc-kind=ArkTS1, category=internal, extension=c
TEST_F(LibAbcKitInternalTest, LibAbcKitTestICSlotAllocator)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "internal/ICSlotAllocator/ICSlotAllocator.abc", "ICSlotAllocator");
    EXPECT_TRUE(helpers::Match(output, "abckit\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "internal/ICSlotAllocator/ICSlotAllocator.abc",
        ABCKIT_ABC_DIR "internal/ICSlotAllocator/ICSlotAllocator_modified.abc", "handle",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            UserData data = {};
            data.print = g_implM->createString(file, "print", strlen("print"));
            data.date = g_implM->createString(file, "Date", strlen("Date"));
            data.getTime = g_implM->createString(file, "getTime", strlen("getTime"));
            data.str = g_implM->createString(file, "file: src/MyClass, function: MyClass.handle",
                                             strlen("file: src/MyClass, function: MyClass.handle"));
            data.consume = g_implM->createString(file, "Ellapsed time:", strlen("Ellapsed time:"));
            TransformIr(graph, &data);
        },
        [](AbckitGraph *graph) {
            AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
            // NOLINTBEGIN(readability-magic-numbers)
            CheckICSlots(g_implG->bbGetTrueBranch(startBB), {{ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, -1},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0XD},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, 0X0},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0XE},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, 0XF},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, 0X11},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, 0X2},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0X13},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, -1},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, 0X4},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0X14},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, 0X15},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, 0X17},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, 0X6},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, -1},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0X19},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, 0X8},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_SUB2, 0XA},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, 0X1A},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, 0XB},
                                                             {ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, -1}});
            // NOLINTEND(readability-magic-numbers)
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "internal/ICSlotAllocator/ICSlotAllocator_modified.abc",
                                        "ICSlotAllocator");
    EXPECT_TRUE(helpers::Match(output,
                               "file: src/MyClass, function: MyClass.handle\n"
                               "abckit\n"
                               "Ellapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
