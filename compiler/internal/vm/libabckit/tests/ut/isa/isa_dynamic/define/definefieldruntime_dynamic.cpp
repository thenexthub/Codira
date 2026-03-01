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

#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynDefineFieldRuntime : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinefieldbyvalue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineFieldRuntime, IcreateCallruntimeDefinefieldbyvalue_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                                             "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nundefined\nundefined\n"));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);

        // CC-OFFNXT(G.FMT.02)
        auto *keyStr = g_implM->createString(file, "key3", strlen("key3"));
        auto *key = g_dynG->iCreateLoadString(graph, keyStr);

        auto *valStr = g_implM->createString(file, "val3", strlen("val3"));
        auto *val = g_dynG->iCreateLoadString(graph, valStr);

        auto *field = g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, val, key, obj);

        g_implG->iInsertAfter(field, obj);
        g_implG->iInsertAfter(key, obj);
        g_implG->iInsertAfter(val, obj);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                             "func_main_0", cb);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                                   "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "val3\nundefined\nundefined\nundefined\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinefieldbyvalue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineFieldRuntime, IcreateCallruntimeDefinefieldbyvalue_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                                             "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nundefined\nundefined\n"));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER);
        // CC-OFFNXT(G.FMT.02)
        auto *keyStr = g_implM->createString(file, "key3", strlen("key3"));
        auto *key = g_dynG->iCreateLoadString(graph, keyStr);

        auto *valStr = g_implM->createString(file, "val3", strlen("val3"));
        auto *val = g_dynG->iCreateLoadString(graph, valStr);

        auto *field = g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, val, key, obj);

        g_implG->iInsertAfter(field, obj);
        g_implG->iInsertAfter(key, obj);
        g_implG->iInsertAfter(val, obj);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                             "func_main_0", cb);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                                   "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nval3\nundefined\nundefined\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinefieldbyindex, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineFieldRuntime, IcreateCallruntimeDefinefieldbyindex_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                                             "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nundefined\nundefined\n"));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
        // CC-OFFNXT(G.FMT.02)
        auto *valStr = g_implM->createString(file, "val3", strlen("val3"));
        auto *val = g_dynG->iCreateLoadString(graph, valStr);

        auto *field = g_dynG->iCreateCallruntimeDefinefieldbyindex(graph, val, 0x4, obj);

        g_implG->iInsertAfter(field, obj);
        g_implG->iInsertAfter(val, obj);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                             "func_main_0", cb);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                                   "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nval3\nundefined\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinefieldbyindex, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineFieldRuntime, IcreateCallruntimeDefinefieldbyindex_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                                             "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nundefined\nundefined\n"));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER);

        auto *valStr = g_implM->createString(file, "val3", strlen("val3"));
        auto *val = g_dynG->iCreateLoadString(graph, valStr);

        auto *field = g_dynG->iCreateCallruntimeDefinefieldbyindex(graph, val, 0x4, obj);

        g_implG->iInsertAfter(field, obj);
        g_implG->iInsertAfter(val, obj);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                             "func_main_0", cb);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldruntime_dynamic_modified.abc",
                                   "definefieldruntime_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nundefined\nval3\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
