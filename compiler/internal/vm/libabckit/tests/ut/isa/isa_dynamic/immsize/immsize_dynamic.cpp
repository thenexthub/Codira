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

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "ir_impl.h"

#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static const uint8_t TEST_UINT4 = 0x1;
static const uint8_t TEST_UINT8 = 0x10;
static const uint16_t TEST_UINT16 = 0x100;

class LibAbcKitCheckDynInstImmSize : public ::testing::Test {};

AbckitGraph *OpenImmSizeFile()
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/isa/isa_dynamic/immsize/immsize_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *bar = helpers::FindMethodByName(file, "bar");
    auto *graph = g_implI->createGraphFromFunction(bar);
    return graph;
}

static void CreateImmInst(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1), uint64_t imm0,
                          uint64_t imm1, AbckitBitImmSize expBitSize)
{
    auto *graph = OpenImmSizeFile();

    auto *instr = apiToCheck(graph, imm0, imm1);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto imm0BitSize = g_implG->iGetImmediateSize(instr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(imm0BitSize, expBitSize);

    auto imm1BitSize = g_implG->iGetImmediateSize(instr, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(imm1BitSize, expBitSize);

    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

static void CreateImmInst(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0,
                                                    uint64_t imm1),
                          uint64_t imm0, uint64_t imm1, AbckitBitImmSize expBitSize)
{
    auto *graph = OpenImmSizeFile();

    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto *instr = apiToCheck(graph, loadStr, imm0, imm1);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto imm0BitSize = g_implG->iGetImmediateSize(instr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(imm0BitSize, expBitSize);

    auto imm1BitSize = g_implG->iGetImmediateSize(instr, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(imm1BitSize, expBitSize);

    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

static void CreateImmInst(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0),
                          uint64_t imm0, AbckitBitImmSize expBitSize)
{
    auto *graph = OpenImmSizeFile();

    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto *instr = apiToCheck(graph, loadStr, imm0);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto immBitSize = g_implG->iGetImmediateSize(instr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(immBitSize, expBitSize);

    auto *file = graph->file;
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStlexvar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateStlexvarImm4)
{
    CreateImmInst(g_dynG->iCreateStlexvar, TEST_UINT4, TEST_UINT4, AbckitBitImmSize::BITSIZE_4);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStlexvar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateStlexvarImm8)
{
    CreateImmInst(g_dynG->iCreateStlexvar, TEST_UINT8, TEST_UINT8, AbckitBitImmSize::BITSIZE_8);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdlexvar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateLdlexvarImm4)
{
    CreateImmInst(g_dynG->iCreateLdlexvar, TEST_UINT4, TEST_UINT4, AbckitBitImmSize::BITSIZE_4);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdlexvar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateLdlexvarImm8)
{
    CreateImmInst(g_dynG->iCreateLdlexvar, TEST_UINT8, TEST_UINT8, AbckitBitImmSize::BITSIZE_8);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeStsendablevar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateCallruntimeStsendablevarImm4)
{
    CreateImmInst(g_dynG->iCreateCallruntimeStsendablevar, TEST_UINT4, TEST_UINT4, AbckitBitImmSize::BITSIZE_4);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeStsendablevar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateCallruntimeStsendablevarImm8)
{
    CreateImmInst(g_dynG->iCreateCallruntimeStsendablevar, TEST_UINT8, TEST_UINT8, AbckitBitImmSize::BITSIZE_8);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendablevar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateCallruntimeLdsendablevarImm4)
{
    CreateImmInst(g_dynG->iCreateCallruntimeLdsendablevar, TEST_UINT4, TEST_UINT4, AbckitBitImmSize::BITSIZE_4);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendablevar,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateCallruntimeLdsendablevarImm8)
{
    CreateImmInst(g_dynG->iCreateCallruntimeLdsendablevar, TEST_UINT8, TEST_UINT8, AbckitBitImmSize::BITSIZE_8);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowIfsupernotcorrectcall,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateThrowIfsupernotcorrectcallImm8)
{
    CreateImmInst(g_dynG->iCreateThrowIfsupernotcorrectcall, TEST_UINT8, AbckitBitImmSize::BITSIZE_8);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowIfsupernotcorrectcall,
// abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitCheckDynInstImmSize, iCreateThrowIfsupernotcorrectcallImm16)
{
    CreateImmInst(g_dynG->iCreateThrowIfsupernotcorrectcall, TEST_UINT16, AbckitBitImmSize::BITSIZE_16);
}

}  // namespace libabckit::test
