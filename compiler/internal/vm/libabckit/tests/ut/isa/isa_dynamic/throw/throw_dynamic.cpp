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

class LibAbcKitCreateDynThrow : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynThrow(
    AbckitIsaApiDynamicOpcode opcode)
{
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> instVector {
        {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}}};
    switch (opcode) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW:
            instVector.push_back({4, opcode, {3}});
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_NOTEXISTS:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_PATTERNNONCOERCIBLE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_DELETESUPERPROPERTY:
            instVector.push_back({4, opcode, {}});
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFNOTOBJECT:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_CONSTASSIGNMENT:
            instVector.push_back({4, opcode, {3}});
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLE:
            instVector.push_back({4, opcode, {3, 3}});
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL:
            instVector.push_back({4, opcode, {3}});
            break;
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
    if (opcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW) {
        instVector.push_back({5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {3}});
    }
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
            {{0}, {2}, {instVector}},
            {{1}, {}, {}}};
}

static void TransformThrowValid(void (*transformIrThrow)(AbckitGraph *graph), AbckitIsaApiDynamicOpcode opcode,
                                const std::string &expectedOutput)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic.abc", "throw_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic_modified.abc", "bar",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            transformIrThrow(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynThrow(opcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic_modified.abc",
                                        "throw_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

static void TransformIr1ArgValid(AbckitGraph *graph, AbckitInst *(*throwInstToCheck)(AbckitGraph *graph))
{
    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = throwInstToCheck(graph);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIr2ArgValid(AbckitGraph *graph,
                                 AbckitInst *(*throwInstToCheck)(AbckitGraph *graph, AbckitInst *input0))
{
    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = throwInstToCheck(graph, loadStr);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIr3ArgValid(AbckitGraph *graph,
                                 AbckitInst *(*throwInstToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                                                 AbckitInst *input1))
{
    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = throwInstToCheck(graph, loadStr, loadStr);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrThrowValid(AbckitGraph *graph)
{
    TransformIr2ArgValid(graph, g_dynG->iCreateThrow);
}

void TransformIrThrowConstassignmentValid(AbckitGraph *graph)
{
    TransformIr2ArgValid(graph, g_dynG->iCreateThrowConstassignment);
}

void TransformIrThrowIfnotobjectValid(AbckitGraph *graph)
{
    TransformIr2ArgValid(graph, g_dynG->iCreateThrowIfnotobject);
}

void TransformIrThrowNotexistsValid(AbckitGraph *graph)
{
    TransformIr1ArgValid(graph, g_dynG->iCreateThrowNotexists);
}

void TransformIrThrowPatternnoncoercibleValid(AbckitGraph *graph)
{
    TransformIr1ArgValid(graph, g_dynG->iCreateThrowPatternnoncoercible);
}

void TransformIrThrowDeletesuperpropertyValid(AbckitGraph *graph)
{
    TransformIr1ArgValid(graph, g_dynG->iCreateThrowDeletesuperproperty);
}

void TransformIrThrowUndefinedifholeValid(AbckitGraph *graph)
{
    TransformIr3ArgValid(graph, g_dynG->iCreateThrowUndefinedifhole);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrow, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowValid)
{
    TransformThrowValid(TransformIrThrowValid, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW, "Ok\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowNotexists, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowNotexistsValid)
{
    TransformThrowValid(TransformIrThrowNotexistsValid, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_NOTEXISTS,
                        "TypeError: Throw method is not defined\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowPatternnoncoercible, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowPatternnoncoercibleValid)
{
    TransformThrowValid(TransformIrThrowPatternnoncoercibleValid,
                        ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_PATTERNNONCOERCIBLE, "TypeError: objectnotcoercible\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowDeletesuperproperty, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowDeletesuperpropertyValid)
{
    TransformThrowValid(TransformIrThrowDeletesuperpropertyValid,
                        ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_DELETESUPERPROPERTY,
                        "ReferenceError: Can not delete super property\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowConstassignment, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowConstassignmentValid)
{
    TransformThrowValid(TransformIrThrowConstassignmentValid, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_CONSTASSIGNMENT,
                        "TypeError: Assignment to const variable Ok\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowIfnotobject, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowIfnotobjectValid)
{
    TransformThrowValid(TransformIrThrowIfnotobjectValid, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFNOTOBJECT,
                        "TypeError: Inner return result is not object\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowUndefinedifhole, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowUndefinedifholeValid)
{
    TransformThrowValid(TransformIrThrowUndefinedifholeValid, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLE, "");
}
/*--------------------------------------------------------------\
|                    Not generalized cases                      |
\--------------------------------------------------------------*/

static void TransformIrThrowIfsupernotcorrectcallValid(AbckitGraph *graph)
{
    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = g_dynG->iCreateThrowIfsupernotcorrectcall(graph, loadStr, 0);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrThrowIfsupernotcorrectcallValidLarge(AbckitGraph *graph)
{
    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = g_dynG->iCreateThrowIfsupernotcorrectcall(graph, loadStr, 0x100);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrThrowUndefinedifholewithnameValid(AbckitGraph *graph, AbckitString *str)
{
    auto loadStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadStr, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = g_dynG->iCreateThrowUndefinedifholewithname(graph, loadStr, str);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowIfsupernotcorrectcall, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowIfsupernotcorrectcallValid_imm8)
{
    TransformThrowValid(TransformIrThrowIfsupernotcorrectcallValid,
                        ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, "");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowIfsupernotcorrectcall, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowIfsupernotcorrectcallValid_imm16)
{
    TransformThrowValid(TransformIrThrowIfsupernotcorrectcallValidLarge,
                        ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, "");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateThrowUndefinedifholewithname, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynThrow, CreateDynThrowUndefinedifholewithnameValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic.abc", "throw_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic_modified.abc", "bar",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *str = g_implM->createString(file, "GeneratedStr", strlen("GeneratedStr"));
            TransformIrThrowUndefinedifholewithnameValid(graph, str);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                CreateBBSchemaForDynThrow(ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/throw/throw_dynamic_modified.abc",
                                        "throw_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
