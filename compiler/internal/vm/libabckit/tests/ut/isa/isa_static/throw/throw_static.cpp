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
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/src/logger.h"
#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

class LibAbcKitThrowStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateThrow, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitThrowStaticTest, LibAbcKitTestThrow)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/throw/throw_static.abc", "throw_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/throw/throw_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/throw/throw_static_modified.abc", "bar",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);

            auto input = g_implG->iGetInput(ret, 0);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *thr = g_statG->iCreateThrow(graph, input);
            ASSERT_NE(thr, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iInsertBefore(thr, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/throw/throw_static_modified.abc",
                                       "throw_static", "main");
    EXPECT_TRUE(helpers::Match(output, "THROW\n"));
}

}  // namespace libabckit::test
   // NOLINTEND(readability-magic-numbers)
