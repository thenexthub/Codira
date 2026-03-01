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

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNewobjrange, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateNewobjrange)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj.abc", "newobj");
    auto expected = "1 2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj_modified.abc", "newobj.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            AbckitInst *defineClass =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);

            AbckitInst *a = g_implG->gFindOrCreateConstantI32(graph, 3);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(a, nullptr);
            AbckitInst *storeThis = g_dynG->iCreateNewobjrange(graph, 3, defineClass, a, a);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(storeThis, nullptr);

            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ret, nullptr);

            g_implG->iInsertBefore(storeThis, ret);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj_modified.abc", "newobj");
    expected = "1 2\n3 3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideNewobjrange, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateWideNewobjrange)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj.abc", "newobj");
    auto expected = "1 2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj_modified.abc", "newobj.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            AbckitInst *defineClass =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);

            AbckitInst *a = g_implG->gFindOrCreateConstantI32(graph, 4);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(a, nullptr);
            AbckitInst *storeThis = g_dynG->iCreateWideNewobjrange(graph, 3, defineClass, a, a);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(storeThis, nullptr);

            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ret, nullptr);

            g_implG->iInsertBefore(storeThis, ret);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/newobj_modified.abc", "newobj");
    expected = "1 2\n4 4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDelobjprop, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateDelobjprop)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/delobjprop.abc", "delobjprop");
    auto expected = "undefined 2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitInst *param = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        // CC-OFFNXT(G.FMT.02)
        AbckitInst *firstDelobj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DELOBJPROP);
        // CC-OFFNXT(G.FMT.02)
        AbckitString *str = g_implM->createString(file, "y", strlen("y"));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(str, nullptr);

        AbckitInst *loadString = g_dynG->iCreateLoadString(graph, str);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(loadString, nullptr);

        g_implG->iInsertAfter(loadString, firstDelobj);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        AbckitInst *secondDelobj = g_dynG->iCreateDelobjprop(graph, loadString, param);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(secondDelobj, nullptr);

        g_implG->iInsertAfter(secondDelobj, loadString);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/delobjprop.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/delobjprop_modified.abc", "foo", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/delobjprop_modified.abc",
                                        "delobjprop");
    expected = "undefined undefined\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdprivateproperty, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdprivateproperty)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop.abc", "privateprop");
    auto expected = "1\n2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitInst *param2 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        // CC-OFFNXT(G.FMT.02)
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param2, nullptr);

        AbckitInst *load = g_dynG->iCreateLdprivateproperty(graph, param2, 0, 0);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(load, nullptr);

        AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ret, nullptr);
        g_implG->iInsertBefore(load, ret);
        g_implG->iRemove(ret);

        ret = g_dynG->iCreateReturn(graph, load);

        g_implG->iInsertAfter(ret, load);

        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop_modified.abc", "getT", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop_modified.abc",
                                        "privateprop");
    expected = "2\n2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStprivateproperty, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStprivateproperty)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop.abc", "privateprop");
    auto expected = "1\n2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitInst *param3 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param3, nullptr);

        AbckitInst *param2 = g_implG->iGetPrev(param3);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param2, nullptr);

        AbckitInst *store = g_dynG->iCreateStprivateproperty(graph, param3, 0, 0, param2);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(store, nullptr);

        AbckitInst *load = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDPRIVATEPROPERTY);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(load, nullptr);

        g_implG->iInsertBefore(store, load);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop_modified.abc", "changeT", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/privateprop_modified.abc",
                                        "privateprop");
    expected = "1\n3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSetobjectwithproto, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateSetobjectwithproto)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/proto.abc", "proto");
    auto expected = "false\nfalse\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0, 0);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ldlexvar, nullptr);

        auto *emptyObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(emptyObj, nullptr);

        auto *set = g_dynG->iCreateSetobjectwithproto(graph, emptyObj, ldlexvar);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(set, nullptr);

        g_implG->iInsertAfter(ldlexvar, emptyObj);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implG->iInsertAfter(set, ldlexvar);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/proto.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/proto_modified.abc", "setProto", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/proto_modified.abc", "proto");
    expected = "false\ntrue\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeCreateprivateproperty, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateCallruntimeCreateprivateproperty)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private.abc",
                                             "callruntime_private");
    auto expected = "30\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private_modified.abc",
        "callruntime_private.func_main_0", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *create =
                helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_CREATEPRIVATEPROPERTY);
            uint64_t imm0 = g_implG->iGetImmediate(create, 0);
            auto *litArr = g_implG->iGetLiteralArray(create);
            auto *newCreate = g_dynG->iCreateCallruntimeCreateprivateproperty(graph, imm0, litArr);
            helpers::ReplaceInst(create, newCreate);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private_modified.abc", "callruntime_private");
    expected = "30\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefineprivateproperty, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateCallruntimeDefineprivateproperty)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private.abc",
                                             "callruntime_private");
    auto expected = "30\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        // CC-OFFNXT(G.FMT.02)
        auto *oldDefine = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINEPRIVATEPROPERTY);
        uint64_t imm1 = g_implG->iGetImmediate(oldDefine, 1);
        auto *acc = g_implG->iGetInput(oldDefine, 0);
        auto *input = g_implG->iGetInput(oldDefine, 1);
        // CC-OFFNXT(G.FMT.02)
        auto *newDefine = g_dynG->iCreateCallruntimeDefineprivateproperty(graph, acc, imm1, 2, input);
        helpers::ReplaceInst(oldDefine, newDefine);
    };
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private.abc",
                             ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private_modified.abc",
                             "instance_initializer", cb);

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/obj/callruntime_private_modified.abc", "callruntime_private");
    expected = "30\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
