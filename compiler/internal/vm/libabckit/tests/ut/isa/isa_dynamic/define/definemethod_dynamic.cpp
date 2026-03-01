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

}  // namespace

class LibAbcKitCreateDynDefineMethod : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchema()
{
    return {{{},
             {1},
             {
                 {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             }},
            {{0},
             {2},
             {
                 {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                 {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, {}},
                 {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER, {4}},
                 {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEMETHOD, {5}},
                 {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {12, 5}},
                 {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
                 {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
                 {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {7, 5}},
                 {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {3}},
                 {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
             }},
            {{1}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinemethod, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynDefineMethod, IcreateDefinemethod)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definemethod_dynamic.abc",
                                             "definemethod_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\nbar\n"));

    auto modulesEnumerator = [](AbckitCoreModule *module, void *data) {
        g_implI->moduleEnumerateTopLevelFunctions(module, data, [](AbckitCoreFunction *func, void *data) {
            if (std::string_view(g_implI->abckitStringToString(g_implI->functionGetName(func))) == "bar") {
                *reinterpret_cast<AbckitCoreFunction **>(data) = func;
            }
            return true;
        });
        return true;
    };

    auto userTransformer = [&modulesEnumerator](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        AbckitCoreFunction *coreFunc = nullptr;
        g_implI->fileEnumerateModules(file, &coreFunc, modulesEnumerator);

        auto *klass = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
        ASSERT_NE(klass, nullptr);

        auto *newDef = g_dynG->iCreateDefinemethod(graph, klass, coreFunc, 0x0);
        ASSERT_NE(newDef, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        auto *callThis0 = g_dynG->iCreateCallthis0(graph, newDef, klass);
        ASSERT_NE(callThis0, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implG->iInsertAfter(newDef, klass);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->iInsertAfter(callThis0, newDef);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definemethod_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definemethod_dynamic_modified.abc",
                             "func_main_0", userTransformer,
                             [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, CreateBBSchema()); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definemethod_dynamic_modified.abc",
                                        "definemethod_dynamic");
    EXPECT_TRUE(helpers::Match(output, "bar\nfoo\nbar\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
