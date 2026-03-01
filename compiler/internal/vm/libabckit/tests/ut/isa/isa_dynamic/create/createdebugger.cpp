/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or g_implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitIcreateTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDebugger, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIcreateTest, IcreateDebugger)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createdebugger_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createdebugger_dynamic_modifyed.abc", "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *main = helpers::BBgetSuccBlocks(start)[0];
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *deb = g_dynG->iCreateDebugger(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAddInstFront(main, deb);

            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                 {{0},
                  {2},
                  {
                      {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEBUGGER, {}},
                      {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

}  // namespace libabckit::test
