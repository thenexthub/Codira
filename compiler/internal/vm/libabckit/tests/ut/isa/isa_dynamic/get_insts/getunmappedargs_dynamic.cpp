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
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynGetunmappedargs : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetunmappedargs, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynGetunmappedargs, IcreateGetunmappedargs_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic.abc",
                                             "getunmappedargs_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *getunmappedargs = g_dynG->iCreateGetunmappedargs(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *ldobjbyvalue =
            g_dynG->iCreateLdobjbyvalue(graph, g_implG->gFindOrCreateConstantU64(graph, 0), getunmappedargs);
        // CC-OFFNXT(G.FMT.02)
        auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
        auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
        auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyvalue);
        auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
        // CC-OFFNXT(G.FMT.02)
        g_implG->iInsertBefore(getunmappedargs, returnundef);
        g_implG->iInsertBefore(ldobjbyvalue, returnundef);
        g_implG->iInsertBefore(tryldglobalbyname, returnundef);
        g_implG->iInsertBefore(callarg1, returnundef);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic_modified.abc",
                             "func1", cb);

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic_modified.abc", "getunmappedargs_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetunmappedargs, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynGetunmappedargs, IcreateGetunmappedargs_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic.abc",
                                             "getunmappedargs_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic_modified.abc", "func2",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *getunmappedargs = g_dynG->iCreateGetunmappedargs(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyvalue0 =
                g_dynG->iCreateLdobjbyvalue(graph, g_implG->gFindOrCreateConstantU64(graph, 0), getunmappedargs);
            auto *ldobjbyvalue1 =
                g_dynG->iCreateLdobjbyvalue(graph, g_implG->gFindOrCreateConstantU64(graph, 1), getunmappedargs);
            auto *ldobjbyvalue2 =
                g_dynG->iCreateLdobjbyvalue(graph, g_implG->gFindOrCreateConstantU64(graph, 2), getunmappedargs);
            auto *callarg10 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyvalue0);
            auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyvalue1);
            auto *callarg12 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyvalue2);

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(getunmappedargs, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(ldobjbyvalue0, returnundef);
            g_implG->iInsertBefore(ldobjbyvalue1, returnundef);
            g_implG->iInsertBefore(ldobjbyvalue2, returnundef);
            g_implG->iInsertBefore(callarg10, returnundef);
            g_implG->iInsertBefore(callarg11, returnundef);
            g_implG->iInsertBefore(callarg12, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getunmappedargs_dynamic_modified.abc", "getunmappedargs_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\nabc\n3.14\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
