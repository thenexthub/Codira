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

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformSetStringIrDynamic(AbckitGraph *graph, AbckitFile *file)
{
    auto *loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *newStr = g_implM->createString(file, "STRING", strlen("STRING"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetString(loadStr, newStr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void CheckGetStringIrDynamic(AbckitGraph *graph)
{
    auto *loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *str = g_implG->iGetString(loadStr);
    ASSERT_NE(str, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(helpers::AbckitStringToString(str), "string");
}

}  // namespace

class LibAbcKitStringDynamicTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::iSetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitStringDynamicTest, LibAbcKitTestSetString)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic.abc", "string_dynamic");
    EXPECT_TRUE(helpers::Match(output, "string\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic_modified.abc", "foo",
                             [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 TransformSetStringIrDynamic(graph, file);
                             });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic_modified.abc", "string_dynamic");
    EXPECT_TRUE(helpers::Match(output, "STRING\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iGetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitStringDynamicTest, LibAbcKitTestGetString)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/string/string_dynamic_modified.abc", "foo",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 CheckGetStringIrDynamic(graph);
                             });
}

}  // namespace libabckit::test
