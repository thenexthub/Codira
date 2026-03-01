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

void TransformIr(AbckitGraph *graph, AbckitCoreFunction *consoleLogInt)
{
    AbckitInst *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    AbckitInst *castToInt =
        g_statG->iCreateCast(graph, g_implG->iGetInput(callInst, 0), AbckitTypeId::ABCKIT_TYPE_ID_I32);
    g_implG->iInsertAfter(castToInt, callInst);
    AbckitInst *callLogInt = g_statG->iCreateCallStatic(graph, consoleLogInt, 1, castToInt);
    g_implG->iInsertAfter(callLogInt, castToInt);
}

void TransformCheckCastIr(AbckitGraph *graph, AbckitFile *file, AbckitCoreClass *classA)
{
    AbckitInst *initObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *type = g_implM->createReferenceType(file, classA);
    ASSERT_NE(type, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *checkCast = g_statG->iCreateCheckCast(graph, initObj, type);
    ASSERT_NE(checkCast, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(checkCast, initObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitCastStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCast, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestCreateCast)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/cast_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/cast_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitCoreFunction *consoleLogInt =
                helpers::FindMethodByName(g_implI->functionGetFile(method), "ConsoleLogInt");
            TransformIr(graph, consoleLogInt);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {2}},
                      {6, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {2}},
                      {8, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {6}},
                      {10, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCheckCast, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestCheckCast)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/checkcast_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/checkcast_static_modified.abc", "main",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
            g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
            ASSERT_NE(classCtxFinder.klass, nullptr);
            TransformCheckCastIr(graph, file, classCtxFinder.klass);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_CHECKCAST, {0}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cast/checkcast_static_modified.abc",
                                            "checkcast_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iGetTargetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestGetTargetType1)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *cast = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CAST);
            auto targetType = g_statG->iGetTargetType(cast);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_EQ(targetType, ABCKIT_TYPE_ID_I32);

            cast = g_implG->iGetNext(cast);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(cast, nullptr);
            targetType = g_statG->iGetTargetType(cast);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_EQ(targetType, ABCKIT_TYPE_ID_F64);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {2}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=IsaApiStaticImpl::iGetTargetType, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestGetTargetType2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
            auto targetType = g_statG->iGetTargetType(ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            ASSERT_EQ(targetType, ABCKIT_TYPE_ID_INVALID);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {2}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=IsaApiStaticImpl::iSetTargetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestSetTargetType1)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *cast = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CAST);
            g_statG->iSetTargetType(cast, ABCKIT_TYPE_ID_F32);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iVisitUsers(cast, nullptr, [](AbckitInst *user, void * /*data*/) {
                g_implG->iRemove(user);
                return true;
            });

            AbckitInst *newCast = g_statG->iCreateCast(graph, cast, ABCKIT_TYPE_ID_F64);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);

            g_implG->iInsertBefore(newCast, call);

            g_implG->iSetInput(call, newCast, 0);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {2}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=IsaApiStaticImpl::iSetTargetType, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCastStaticTest, LibAbcKitTestSetTargetType2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cast/target_type_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
            g_statG->iSetTargetType(ret, ABCKIT_TYPE_ID_F64);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_CAST, {2}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
