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
#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitAsyncTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncfunctionenter, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncfunctionenter)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
                                             "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_enter.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncEnterInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONENTER);
            helpers::ReplaceInst(asyncEnterInst, g_dynG->iCreateAsyncfunctionenter(graph));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_enter.abc",
                                   "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncfunctionresolve, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncfunctionresolve)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
                                             "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_resolve.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncResolveInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONRESOLVE);
            auto *inputOne = g_implG->iGetInput(asyncResolveInst, 0);
            auto *inputTwo = g_implG->iGetInput(asyncResolveInst, 1);
            helpers::ReplaceInst(asyncResolveInst, g_dynG->iCreateAsyncfunctionresolve(graph, inputOne, inputTwo));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_resolve.abc",
                                   "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncfunctionreject, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncfunctionreject)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
                                             "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_reject.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncResolveInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONREJECT);
            auto *inputOne = g_implG->iGetInput(asyncResolveInst, 0);
            auto *inputTwo = g_implG->iGetInput(asyncResolveInst, 1);
            helpers::ReplaceInst(asyncResolveInst, g_dynG->iCreateAsyncfunctionresolve(graph, inputOne, inputTwo));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/enter_resolve_reject_dynamic_reject.abc",
                                   "enter_resolve_reject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncfunctionawaituncaught, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncfunctionawaituncaught)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/awaituncaught_dynamic.abc",
                                             "awaituncaught_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/awaituncaught_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/awaituncaught_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncAwaitUncaughtInst =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONAWAITUNCAUGHT);
            auto *inputOne = g_implG->iGetInput(asyncAwaitUncaughtInst, 0);
            auto *inputTwo = g_implG->iGetInput(asyncAwaitUncaughtInst, 1);
            helpers::ReplaceInst(asyncAwaitUncaughtInst,
                                 g_dynG->iCreateAsyncfunctionresolve(graph, inputOne, inputTwo));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/awaituncaught_dynamic_modified.abc",
                                        "awaituncaught_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateasyncgeneratorobj, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateCreateasyncgeneratorobj)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGenObj =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEASYNCGENERATOROBJ);
            auto *input = g_implG->iGetInput(asyncCreateAsyncGenObj, 0);
            helpers::ReplaceInst(asyncCreateAsyncGenObj, g_dynG->iCreateCreateasyncgeneratorobj(graph, input));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_modified.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncgeneratorreject, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncgeneratorreject)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_reject.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGen =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCGENERATORREJECT);
            auto *input0 = g_implG->iGetInput(asyncCreateAsyncGen, 0);
            auto *input1 = g_implG->iGetInput(asyncCreateAsyncGen, 1);
            helpers::ReplaceInst(asyncCreateAsyncGen, g_dynG->iCreateAsyncgeneratorreject(graph, input0, input1));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_reject.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAsyncgeneratorresolve, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateAsyncgeneratorresolve)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_resolve.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGen =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCGENERATORRESOLVE);
            auto *input0 = g_implG->iGetInput(asyncCreateAsyncGen, 0);
            auto *input1 = g_implG->iGetInput(asyncCreateAsyncGen, 1);
            auto *input2 = g_implG->iGetInput(asyncCreateAsyncGen, 2);
            helpers::ReplaceInst(asyncCreateAsyncGen,
                                 g_dynG->iCreateAsyncgeneratorresolve(graph, input0, input1, input2));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_resolve.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateResumegenerator, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateResumegenerator)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_resume.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGen = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RESUMEGENERATOR);
            auto *input0 = g_implG->iGetInput(asyncCreateAsyncGen, 0);
            helpers::ReplaceInst(asyncCreateAsyncGen, g_dynG->iCreateResumegenerator(graph, input0));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_resume.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSetgeneratorstate, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateSetgeneratorstate)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_state.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGen = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_SETGENERATORSTATE);
            auto *input0 = g_implG->iGetInput(asyncCreateAsyncGen, 0);
            uint64_t imm0 = g_implG->iGetImmediate(asyncCreateAsyncGen, 0);
            helpers::ReplaceInst(asyncCreateAsyncGen, g_dynG->iCreateSetgeneratorstate(graph, input0, imm0));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_state.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSuspendgenerator, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateSuspendgenerator)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
                                             "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_suspend.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncGen = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUSPENDGENERATOR);
            auto *input0 = g_implG->iGetInput(asyncCreateAsyncGen, 0);
            auto *input1 = g_implG->iGetInput(asyncCreateAsyncGen, 1);
            helpers::ReplaceInst(asyncCreateAsyncGen, g_dynG->iCreateSuspendgenerator(graph, input0, input1));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncgenerator_dynamic_suspend.abc",
                                        "asyncgenerator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetasynciterator, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateGetasynciterator)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asynciterator_dynamic.abc",
                                             "asynciterator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asynciterator_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asynciterator_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *asyncCreateAsyncIter = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETASYNCITERATOR);
            auto *input = g_implG->iGetInput(asyncCreateAsyncIter, 0);
            helpers::ReplaceInst(asyncCreateAsyncIter, g_dynG->iCreateGetasynciterator(graph, input));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asynciterator_dynamic_modified.abc",
                                        "asynciterator_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeNotifyconcurrentresult, abc-kind=JS, category=positive
TEST_F(LibAbcKitAsyncTest, IcreateCallruntimeNotifyconcurrentresult)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncuseconcurrent_dynamic.abc",
                                             "asyncuseconcurrent_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncuseconcurrent_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncuseconcurrent_dynamic_modified.abc", "a",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *notifyconcurrentresult =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NOTIFYCONCURRENTRESULT);
            auto *input = g_implG->iGetInput(notifyconcurrentresult, 0);
            helpers::ReplaceInst(notifyconcurrentresult,
                                 g_dynG->iCreateCallruntimeNotifyconcurrentresult(graph, input));
            notifyconcurrentresult =
                helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NOTIFYCONCURRENTRESULT);
            input = g_implG->iGetInput(notifyconcurrentresult, 0);
            helpers::ReplaceInst(notifyconcurrentresult,
                                 g_dynG->iCreateCallruntimeNotifyconcurrentresult(graph, input));
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/async/asyncuseconcurrent_dynamic_modified.abc",
                                   "asyncuseconcurrent_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

}  // namespace libabckit::test
