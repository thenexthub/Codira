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

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

AbckitInst *CheckInstAndGetNext(AbckitInst *inst, AbckitIsaApiStaticOpcode opcode)
{
    EXPECT_EQ(g_statG->iGetOpcode(inst), opcode);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    inst = g_implG->iGetNext(inst);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    return inst;
}

void InspectIr(AbckitGraph *graph)
{
    auto *bb = g_implG->gGetBasicBlock(graph, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *inst = g_implG->bbGetFirstInst(bb);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_STOREARRAY);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_STOREARRAY);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_LOADARRAY);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT);
    inst = CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_CAST);
    CheckInstAndGetNext(inst, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
}
}  // namespace

class LibAbcKitIgetOpcodeStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iGetOpcode, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIgetOpcodeStaticTest, LibAbcKitTestIgetOpcode)
{
    helpers::InspectMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/get_opcode/get_opcode_static.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { InspectIr(graph); });
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
