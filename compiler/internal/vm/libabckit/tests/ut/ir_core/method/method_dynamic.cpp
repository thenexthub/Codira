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
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformSetSetCallMethod(AbckitGraph *graph, AbckitCoreFunction *foo)
{
    auto *defineProp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEPROPERTYBYNAME);
    ASSERT_NE(defineProp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *defineFunc = g_implG->iGetPrev(defineProp);
    ASSERT_NE(defineFunc, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(g_dynG->iGetOpcode(defineFunc), ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);

    g_implG->iSetFunction(defineFunc, foo);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitMethodDynamicTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::iSetFunction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitMethodDynamicTest, LibAbcKitTestSetCallMethod)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_dynamic.abc", "method_dynamic");
    EXPECT_TRUE(helpers::Match(output, "function\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/method/method_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/method/method_dynamic_modified.abc", "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
                                 auto *bar = helpers::FindMethodByName(g_implI->functionGetFile(method), "foo");
                                 ASSERT_NE(bar, nullptr);
                                 TransformSetSetCallMethod(graph, bar);
                             });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_dynamic_modified.abc", "method_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\n"));
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
