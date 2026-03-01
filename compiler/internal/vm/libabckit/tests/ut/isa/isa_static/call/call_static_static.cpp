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
#include "libabckit/c/ir_core.h"

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

void TransformCallStaticIr(AbckitGraph *graph, AbckitCoreFunction *func)
{
    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *call = g_statG->iCreateCallStatic(graph, func, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(call, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformCallStaticIrArgs(AbckitGraph *graph, AbckitCoreFunction *func)
{
    auto *module = g_implI->functionGetModule(func);

    helpers::ClassByNameContext classCtxFinder1 = {nullptr, "B"};
    g_implI->moduleEnumerateClasses(module, &classCtxFinder1, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classCtxFinder1.klass, nullptr);

    helpers::ClassByNameContext classCtxFinder2 = {nullptr, "C"};
    g_implI->moduleEnumerateClasses(module, &classCtxFinder2, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classCtxFinder2.klass, nullptr);

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *obj1 = g_statG->iCreateNewObject(graph, classCtxFinder1.klass);
    g_implG->iInsertBefore(obj1, ret);

    auto *obj2 = g_statG->iCreateNewObject(graph, classCtxFinder2.klass);
    g_implG->iInsertBefore(obj2, ret);

    auto *call = g_statG->iCreateCallStatic(graph, func, 2, obj1, obj2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(call, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace

class LibAbcKitCallStaticStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCallStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCallStaticStaticTest, LibAbcKitTestCallStatic1)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
                                            "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
            g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
            ASSERT_NE(classCtxFinder.klass, nullptr);
            helpers::MethodByNameContext methodCtxFinder = {nullptr, "foo"};
            g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(methodCtxFinder.method, nullptr);
            TransformCallStaticIr(graph, methodCtxFinder.method);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {1, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {}},
                {0, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc",
                                       "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, "A::foo\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCallStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCallStaticStaticTest, LibAbcKitTestCallStatic2)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
                                            "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::MethodByNameContext methodCtxFinder = {nullptr, "bar"};
            g_implI->moduleEnumerateTopLevelFunctions(module, &methodCtxFinder, helpers::MethodByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(methodCtxFinder.method, nullptr);
            TransformCallStaticIr(graph, methodCtxFinder.method);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {1, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {}},
                {0, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc",
                                       "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, "bar\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCallStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCallStaticStaticTest, LibAbcKitTestCallStatic3)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
                                            "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
            g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
            ASSERT_NE(classCtxFinder.klass, nullptr);
            helpers::MethodByNameContext methodCtxFinder = {nullptr, "fooArgs"};
            g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(methodCtxFinder.method, nullptr);
            TransformCallStaticIrArgs(graph, methodCtxFinder.method);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT, {}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {0, 1}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/call/call_static_static_modified.abc",
                                       "call_static_static", "main");
    EXPECT_TRUE(helpers::Match(output, "B::foo\nC::foo\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
