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
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include "ir_impl.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsI = AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsM = AbckitGetJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitJSModifyApiModulesTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStarrayspread, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStarrayspread)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/arrays/starrayspread.abc",
                                             "starrayspread");
    auto expected = "helloworld\n1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/arrays/starrayspread.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/arrays/starrayspread_modified.abc", "starrayspread.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            AbckitInst *ind = g_implG->gFindOrCreateConstantI32(graph, 0);
            AbckitInst *str = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
            AbckitInst *emptyArray = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY);
            AbckitInst *spread = g_dynG->iCreateStarrayspread(graph, str, emptyArray, ind);

            AbckitInst *oldStown = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX);
            g_implG->iRemove(oldStown);
            g_implG->iInsertAfter(spread, emptyArray);
        });
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/arrays/starrayspread_modified.abc",
                                        "starrayspread");
    expected = "helloworld\n10\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
