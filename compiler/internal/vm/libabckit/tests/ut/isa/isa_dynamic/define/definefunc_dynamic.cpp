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
#include <string_view>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

auto g_icreateDefineFuncTransformCb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    AbckitCoreFunction *coreFunc = nullptr;
    g_implI->fileEnumerateModules(file, &coreFunc, [](AbckitCoreModule *module, void *data) {
        g_implI->moduleEnumerateTopLevelFunctions(module, data, [](AbckitCoreFunction *func, void *data) {
            if (std::string_view(g_implI->abckitStringToString(g_implI->functionGetName(func))) == "foo") {
                *reinterpret_cast<AbckitCoreFunction **>(data) = func;
            }
            return true;
        });
        return true;
    });

    auto *newFunc = g_dynG->iCreateDefinefunc(graph, coreFunc, 0x0);
    ASSERT_NE(newFunc, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);

    helpers::ReplaceInst(func, newFunc);
};

}  // namespace

class LibAbcKitCreateDynDefineFunc : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinefunc, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynDefineFunc, IcreateDefinefunc)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefunc_dynamic.abc",
                                             "definefunc_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\nbar\n"));

    auto cb = [](AbckitGraph *graph) {
        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
            {{},
             {1},
             {
                 {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             }},
            {{0},
             {2},
             {
                 {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                 {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                 {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {4}},
                 {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {9}},
                 {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
             }},
            {{1}, {}, {}}};
        // CC-OFFNXT(G.FMT.02)
        helpers::VerifyGraph(graph, bbSchema);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefunc_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefunc_dynamic_modified.abc", "func_main_0",
                             g_icreateDefineFuncTransformCb, cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definefunc_dynamic_modified.abc",
                                        "definefunc_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\nfoo\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
