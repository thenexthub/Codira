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

class LibAbcKitGetTypeDynamicTest : public ::testing::Test {};

static void CheckTypeOfInst(AbckitInst *inst, AbckitTypeId typeId)
{
    AbckitType *instType = g_implG->iGetType(inst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitTypeId instTypeId = g_implI->typeGetTypeId(instType);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(instTypeId, typeId);
}

static void InspectMethodDynamicValid(AbckitGraph *graph)
{
    AbckitInst *firstInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *secondInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2);
    ASSERT_NE(secondInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    CheckTypeOfInst(firstInst, ABCKIT_TYPE_ID_I32);
    CheckTypeOfInst(secondInst, ABCKIT_TYPE_ID_ANY);
}

// Test: test-kind=api, api=GraphApiImpl::iGetType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetTypeDynamicTest, GetTypeDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/types_api/get_type_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        InspectMethodDynamicValid(graph);
    });
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
