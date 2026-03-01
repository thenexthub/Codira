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

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitStaticTryCatchTest : public ::testing::Test {};

namespace {
auto *g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

enum class TryCatchScenario {
    DEFAULT_POSITIVE = 0,
};
}  // namespace

// Test: test-kind=internal, abc-kind=ArkTS2, category=internal
TEST_F(LibAbcKitStaticTryCatchTest, TryCatchIrbuilder)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_static.abc",
                                            "insert_try_catch_static", "main");
    EXPECT_TRUE(helpers::Match(output, "TRY1\nTRY2\nCATCH\nTRY1\nTRY3\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_static_modified.abc", "foo",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {},
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/insert_try_catch/insert_try_catch_static_modified.abc",
                                  "insert_try_catch_static", "main");
    EXPECT_TRUE(helpers::Match(output, "TRY1\nTRY2\nCATCH\nTRY1\nTRY3\n"));
}

}  // namespace libabckit::test
