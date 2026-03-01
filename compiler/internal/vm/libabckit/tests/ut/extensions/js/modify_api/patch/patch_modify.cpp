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

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> g_expectedShema = {
    {{},
     {1},
     {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
      {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
      {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
      {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
    {{0},
     {2},
     {
         {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STPATCHVAR, {0}},
         {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDPATCHVAR, {}},
         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {2}},
     }},
    {{1}, {}, {}}};

static void LoadStorePatchTest(AbckitGraph *graph)
{
    AbckitInst *a = g_implG->gFindOrCreateConstantI32(graph, 4);
    AbckitInst *patch = g_dynG->iCreateWideStpatchvar(graph, a, 0);
    AbckitInst *lda = g_dynG->iCreateWideLdpatchvar(graph, 0);
    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    g_implG->iSetInput(ret, lda, 0);
    g_implG->iInsertBefore(patch, ret);
    g_implG->iInsertBefore(lda, ret);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStpatchvar, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateWideStpatchvar)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test.abc", "patch_test");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test_modified.abc", "foo",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { LoadStorePatchTest(graph); },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(g_expectedShema);
            helpers::VerifyGraph(graph, bbSchemas);
        });
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test_modified.abc",
                                        "patch_test");
    expected = "4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideLdpatchvar, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateWideLdpatchvar)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test.abc", "patch_test");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test_modified.abc", "foo",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { LoadStorePatchTest(graph); },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(g_expectedShema);
            helpers::VerifyGraph(graph, bbSchemas);
        });
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/patch/patch_test_modified.abc",
                                        "patch_test");
    expected = "4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
