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

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchema()
{
    helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
        {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
        {2, ABCKIT_ISA_API_STATIC_OPCODE_ISINSTANCE, {0}},
        {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
    });
    helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
    helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
    return {bb1, bb2, bb3};
}

void IsetClass(AbckitGraph *graph, AbckitFile *file, AbckitCoreClass *klass, AbckitCoreClass *startKlass)
{
    AbckitInst *initObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *type = g_implM->createReferenceType(file, startKlass);
    ASSERT_NE(type, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *isInstance = g_statG->iCreateIsInstance(graph, initObj, type);
    ASSERT_NE(isInstance, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_statG->iSetClass(isInstance, klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(isInstance, initObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ReplaceInst(ret, g_statG->iCreateReturn(graph, isInstance));
}

AbckitCoreClass *IgetClass(AbckitGraph *graph)
{
    AbckitInst *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_ISINSTANCE);
    EXPECT_TRUE(inst != nullptr);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto *klass = g_statG->iGetClass(inst);
    EXPECT_TRUE(klass != nullptr);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    return klass;
}

void TransformIrForIsetClass(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *module = g_implI->functionGetModule(method);
    helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
    ASSERT_NE(classCtxFinder.klass, nullptr);
    helpers::ClassByNameContext startClassCtxFinder = {nullptr, "B"};
    g_implI->moduleEnumerateClasses(module, &startClassCtxFinder, helpers::ClassByNameFinder);
    ASSERT_NE(startClassCtxFinder.klass, nullptr);
    IsetClass(graph, file, classCtxFinder.klass, startClassCtxFinder.klass);
}

void TransformIrForIgetClass(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *module = g_implI->functionGetModule(method);
    helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
    ASSERT_NE(classCtxFinder.klass, nullptr);
    helpers::ClassByNameContext startClassCtxFinder = {nullptr, "B"};
    g_implI->moduleEnumerateClasses(module, &startClassCtxFinder, helpers::ClassByNameFinder);
    ASSERT_NE(startClassCtxFinder.klass, nullptr);
    IsetClass(graph, file, classCtxFinder.klass, startClassCtxFinder.klass);
    auto *klass = IgetClass(graph);
    ASSERT_EQ(klass, classCtxFinder.klass);
}

void TemplateTestFunction(const std::string &inputResult, const std::string &outputResult, const std::string &funcName,
                          void (*irTransformer)(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph))
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/classes/classes_api.abc", "classes_api", "main");
    EXPECT_TRUE(helpers::Match(output, inputResult));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/classes/classes_api.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/classes/classes_api_modified.abc", funcName,
        [&irTransformer](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            irTransformer(file, method, graph);
        },
        [](AbckitGraph *graph) {
            const auto &bbSchemas = CreateBBSchema();
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/classes/classes_api_modified.abc",
                                       "classes_api", "main");
    EXPECT_TRUE(helpers::Match(output, outputResult));
}
}  // namespace

class LibAbcKitClassesAPITest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iSetClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitClassesAPITest, LibAbcKitTestIsetClassIsInstanceTrue)
{
    TemplateTestFunction("false\ntrue\n", "true\ntrue\n", "foo", TransformIrForIsetClass);
}

// Test: test-kind=api, api=IsaApiStaticImpl::iSetClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitClassesAPITest, LibAbcKitTestIsetClassIsInstanceFalse)
{
    TemplateTestFunction("false\ntrue\n", "false\nfalse\n", "bar", TransformIrForIsetClass);
}

// Test: test-kind=api, api=IsaApiStaticImpl::iGetClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitClassesAPITest, LibAbcKitTestIgetClassIsInstance)
{
    TemplateTestFunction("false\ntrue\n", "true\ntrue\n", "foo", TransformIrForIgetClass);
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
