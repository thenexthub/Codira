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

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformCallVirtualIr(AbckitGraph *graph, AbckitCoreClass *classA)
{
    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *initObj = g_implG->iGetPrev(ret);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "foo"};
    g_implI->classEnumerateMethods(classA, &methodCtxFinder, helpers::MethodByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(methodCtxFinder.method, nullptr);

    auto *call = g_statG->iCreateCallVirtual(graph, initObj, methodCtxFinder.method, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(call, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace

class LibAbcKitCallVirtualStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCallVirtual, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCallVirtualStaticTest, LibAbcKitTestCallVirtual)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_virtual_static.abc",
                                            "call_virtual_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_virtual_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_virtual_static_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
            g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
            ASSERT_NE(classCtxFinder.klass, nullptr);
            TransformCallVirtualIr(graph, classCtxFinder.klass);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {0}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_virtual_static_modified.abc",
                                       "call_virtual_static", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
