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
#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitGetTypeStaticTest : public ::testing::Test {};

static void CheckTypeOfInst(AbckitInst *inst, AbckitTypeId typeId)
{
    AbckitType *instType = g_implG->iGetType(inst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitTypeId instTypeId = g_implI->typeGetTypeId(instType);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(instTypeId, typeId);
}

static void InspectMethodStaticValid(AbckitGraph *graph)
{
    AbckitInst *firstConst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
    ASSERT_NE(firstConst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *secondConst = g_implG->iGetNext(firstConst);
    ASSERT_NE(secondConst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *addInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_ADD);
    ASSERT_NE(addInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    CheckTypeOfInst(firstConst, ABCKIT_TYPE_ID_F64);
    CheckTypeOfInst(secondConst, ABCKIT_TYPE_ID_I64);
    CheckTypeOfInst(addInst, ABCKIT_TYPE_ID_F64);
}

// Test: test-kind=api, api=GraphApiImpl::iGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitGetTypeStaticTest, GetTypeStatic)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/types_api/get_type_static.abc", "foo",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               InspectMethodStaticValid(graph);
                           });
}

}  // namespace libabckit::test
