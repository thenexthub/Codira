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
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynDefineField : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinefieldbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynDefineField, IcreateDefinefieldbyname_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                                             "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nundefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "instance_initializer",
        [&](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *name = g_implM->createString(file, "b", strlen("b"));
            ASSERT_NE(name, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *constant = g_implG->gFindOrCreateConstantI32(graph, 4);
            ASSERT_NE(constant, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *par = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
            ASSERT_NE(par, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *field = g_dynG->iCreateDefinefieldbyname(graph, constant, name, par);
            ASSERT_NE(field, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
            ASSERT_NE(mainBB, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->bbAddInstFront(mainBB, field);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\n4\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinefieldbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynDefineField, IcreateDefinefieldbyname_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                                             "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nundefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "instance_initializer",
        [&](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *name = g_implM->createString(file, "b", strlen("b"));
            ASSERT_NE(name, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *str = g_implM->createString(file, "Hello", strlen("Hello"));
            ASSERT_NE(str, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *ldstr = g_dynG->iCreateLoadString(graph, str);
            ASSERT_NE(ldstr, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *par = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
            ASSERT_NE(par, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *field = g_dynG->iCreateDefinefieldbyname(graph, ldstr, name, par);
            ASSERT_NE(field, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
            ASSERT_NE(mainBB, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->bbAddInstFront(mainBB, field);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAddInstFront(mainBB, ldstr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nHello\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinepropertybyname, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineField, IcreateDefinepropertybyname_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                                             "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nundefined\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc",
                             "instance_initializer",
                             [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *name = g_implM->createString(file, "b", strlen("b"));
                                 ASSERT_NE(name, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *constant = g_implG->gFindOrCreateConstantI32(graph, 4);
                                 ASSERT_NE(constant, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *par = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
                                 ASSERT_NE(par, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *prop = g_dynG->iCreateDefinepropertybyname(graph, constant, name, par);
                                 ASSERT_NE(prop, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
                                 ASSERT_NE(mainBB, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 g_implG->bbAddInstFront(mainBB, prop);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\n4\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinepropertybyname, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineField, IcreateDefinepropertybyname_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                                             "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nundefined\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc",
                             "instance_initializer",
                             [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *name = g_implM->createString(file, "b", strlen("b"));
                                 ASSERT_NE(name, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *str = g_implM->createString(file, "World", strlen("World"));
                                 ASSERT_NE(str, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *ldstr = g_dynG->iCreateLoadString(graph, str);
                                 ASSERT_NE(ldstr, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *par = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
                                 ASSERT_NE(par, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *prop = g_dynG->iCreateDefinepropertybyname(graph, ldstr, name, par);
                                 ASSERT_NE(prop, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
                                 ASSERT_NE(mainBB, nullptr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

                                 g_implG->bbAddInstFront(mainBB, prop);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->bbAddInstFront(mainBB, ldstr);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefieldbyname_dynamic_modified.abc", "definefieldbyname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\nWorld\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
