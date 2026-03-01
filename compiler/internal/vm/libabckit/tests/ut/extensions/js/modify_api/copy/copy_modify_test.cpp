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

static void CopyrestargsTest(AbckitGraph *graph, bool isWide)
{
    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    AbckitInst *cp;
    if (isWide) {
        cp = g_dynG->iCreateWideCopyrestargs(graph, 0);
    } else {
        cp = g_dynG->iCreateCopyrestargs(graph, 0);
    }
    g_implG->iInsertBefore(cp, ret);
    g_implG->iSetInput(ret, cp, 0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCopydataproperties, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, IcreateCopydatapropertiesTest)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copy_properties_test.abc",
                                             "copy_properties_test");
    auto expected = "a\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copy_properties_test.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copy_properties_test_modified.abc", "foo",
                             [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
                                 auto *add = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2);
                                 auto *param = g_implG->iGetInput(add, 1);

                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 ASSERT_NE(param, nullptr);
                                 auto *emptyObj = g_dynG->iCreateCreateemptyobject(graph);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 ASSERT_NE(emptyObj, nullptr);
                                 AbckitInst *cp = g_dynG->iCreateCopydataproperties(graph, param, emptyObj);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 ASSERT_NE(cp, nullptr);
                                 AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 ASSERT_NE(ret, nullptr);

                                 g_implG->iInsertBefore(emptyObj, ret);
                                 g_implG->iInsertBefore(cp, ret);
                                 g_implG->iSetInput(ret, cp, 0);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copy_properties_test_modified.abc", "copy_properties_test");
    expected = "b\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCopyrestargs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateCopyrestargs)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test.abc",
                                             "copyrestargs_test");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test_modified.abc", "f",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { CopyrestargsTest(graph, false); });
    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test_modified.abc", "copyrestargs_test");
    expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCopyrestargs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateWideCopyrestargs)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test.abc",
                                             "copyrestargs_test");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test_modified.abc", "f",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { CopyrestargsTest(graph, true); },
        [&](AbckitGraph *) {});
    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/copy/copyrestargs_test_modified.abc", "copyrestargs_test");
    expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
