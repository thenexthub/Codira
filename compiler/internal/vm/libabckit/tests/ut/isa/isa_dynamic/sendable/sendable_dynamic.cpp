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

#include "ir_impl.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "metadata_inspect_impl.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynSendable : public ::testing::Test {};

static AbckitInst *CreateSendableClass(AbckitInst *klass, AbckitFile *file, AbckitGraph *graph)
{
    auto *m = g_implG->iGetFunction(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *litarr = g_implG->iGetLiteralArray(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto immClass = g_implG->iGetImmediate(klass, 0);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *input = g_implG->iGetInput(klass, 0);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::array<AbckitLiteral *, 2U> statlitarr = {g_implM->createLiteralU32(file, 0),
                                                  g_implM->createLiteralLiteralArray(file, litarr)};

    auto *newlitarr = g_implM->createLiteralArray(file, statlitarr.data(), 2U);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // creating new inst
    auto *sendableKlass = g_dynG->iCreateCallruntimeDefinesendableclass(graph, m, newlitarr, immClass, input);
    EXPECT_NE(sendableKlass, nullptr);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    return sendableKlass;
}

static void TransformIrSendableClass(AbckitFile *file, AbckitGraph *graph, bool isWideMode, uint64_t imm = 0x0)
{
    auto *klass = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *sendableKlass = CreateSendableClass(klass, file, graph);
    ASSERT_NE(sendableKlass, nullptr);

    // Slot number passed to create newsendableenv allows [0, slot number - 1] slots to be used by other instructions in
    // this env. Slot index imm used in stsendablevar and ldsendablevar must be less than the slot number.
    AbckitInst *newsendableenv = isWideMode ? g_dynG->iCreateCallruntimeWidenewsendableenv(graph, imm + 1)
                                            : g_dynG->iCreateCallruntimeNewsendableenv(graph, imm + 1);
    ASSERT_NE(newsendableenv, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *stsendablevar = isWideMode ? g_dynG->iCreateCallruntimeWidestsendablevar(graph, sendableKlass, 0x0, imm)
                                           : g_dynG->iCreateCallruntimeStsendablevar(graph, sendableKlass, 0x0, imm);
    ASSERT_NE(stsendablevar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // getting other insts
    auto *ldhole = g_implG->iGetPrev(klass);
    ASSERT_NE(ldhole, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *definefunc = g_implG->iGetPrev(ldhole);
    ASSERT_NE(definefunc, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *newlexenv = g_implG->iGetPrev(definefunc);
    ASSERT_NE(newlexenv, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *ldobjbyname = g_implG->iGetNext(klass);
    ASSERT_NE(ldobjbyname, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *stlexvar = g_implG->iGetNext(ldobjbyname);
    ASSERT_NE(stlexvar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // removing old insts
    g_implG->iRemove(klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(ldobjbyname);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(newsendableenv, newlexenv);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(newlexenv);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(sendableKlass, ldhole);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(stsendablevar, stlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(stlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // setting up insts' inputs
    g_implG->iSetInput(sendableKlass, ldhole, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrSendableClassFoo(AbckitGraph *graph, bool isWideMode, uint64_t imm)
{
    auto *ldlexvar = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR);
    ASSERT_NE(ldlexvar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *throwundef = g_implG->iGetNext(ldlexvar);
    ASSERT_NE(throwundef, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *newobjrange = g_implG->iGetNext(throwundef);
    ASSERT_NE(newobjrange, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *ret = g_implG->iGetNext(newobjrange);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ldsendablevar = nullptr;
    if (isWideMode) {
        ldsendablevar = g_dynG->iCreateCallruntimeWideldsendablevar(graph, 0x0, imm);
    } else {
        ldsendablevar = g_dynG->iCreateCallruntimeLdsendablevar(graph, 0x0, imm);
    }
    ASSERT_NE(ldsendablevar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(ldsendablevar, ldlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(ldlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(throwundef, ldsendablevar, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(newobjrange, ldsendablevar, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrLdSendableClass(AbckitFile *file, AbckitGraph *graph)
{
    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *str = g_implM->createString(file, "foo", strlen("foo"));
    ASSERT_NE(str, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // creating new inst
    auto *ldsendableclass = g_dynG->iCreateCallruntimeLdsendableclass(graph, 0x0);
    ASSERT_NE(ldsendableclass, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, ldsendableclass, str);
    ASSERT_NE(ldobjbyname, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *callthis0 = g_dynG->iCreateCallthis0(graph, ldobjbyname, ldsendableclass);
    ASSERT_NE(callthis0, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(callthis0, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(ldobjbyname, callthis0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(ldsendableclass, ldobjbyname);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrSendableClassCtor(AbckitGraph *graph, bool isWideMode)
{
    auto *ldexternalmodulevar = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR);
    ASSERT_NE(ldexternalmodulevar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *throwundefined = g_implG->iGetNext(ldexternalmodulevar);
    ASSERT_NE(throwundefined, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *callarg1 = g_implG->iGetNext(throwundefined);
    ASSERT_NE(callarg1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // creating new inst
    AbckitInst *ldsendableexternalmodulevar = nullptr;
    if (isWideMode) {
        ldsendableexternalmodulevar = g_dynG->iCreateCallruntimeWideldsendableexternalmodulevar(graph, 0x0);
    } else {
        ldsendableexternalmodulevar = g_dynG->iCreateCallruntimeLdsendableexternalmodulevar(graph, 0x0);
    }
    ASSERT_NE(ldsendableexternalmodulevar, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(ldsendableexternalmodulevar, ldexternalmodulevar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(throwundefined, ldsendableexternalmodulevar, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(callarg1, ldsendableexternalmodulevar, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iRemove(ldexternalmodulevar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->gGetStartBasicBlock(graph);
}

static void VerifyIrSendableClass(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode newsendableenv = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDENEWSENDABLEENV
                                                          : ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV;
    AbckitIsaApiDynamicOpcode stsendablevar = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDESTSENDABLEVAR
                                                         : ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_STSENDABLEVAR;
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {{8, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
        {{0},
         {2},
         {
             {0, newsendableenv, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, {}},
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS, {2}},
             {4, stsendablevar, {3}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {1}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void VerifyIrSendableClassFoo(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode ldsendable = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDSENDABLEVAR
                                                      : ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLEVAR;
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
        {{0},
         {2},
         {
             {0, ldsendable, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {0}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, {0}},
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {2}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void VerifyIrSendableClassCtor(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode ldsendableexternal =
        isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR
                   : ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR;
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
        {{0},
         {2},
         {
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {4, ldsendableexternal, {}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {4}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {3, 4}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
             {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {7, 8}},
             {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {2}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void VerifyIrLdSendableClass(AbckitGraph *graph)
{
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
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLECLASS, {}},
             {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {3}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {4, 3}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {2}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void ModifySendableClass(bool isWideMode, uint64_t imm = 0x0)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic.abc",
                                             "makesendableclass_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\nHello\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
        "makesendableclass_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            TransformIrSendableClass(file, graph, isWideMode, imm);
        },
        [&](AbckitGraph *graph) { VerifyIrSendableClass(graph, isWideMode); });

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
        "makesendableclass_dynamic.foo",
        [&](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            TransformIrSendableClassFoo(graph, isWideMode, imm);
        },
        [&](AbckitGraph *graph) { VerifyIrSendableClassFoo(graph, isWideMode); });

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
        "makesendableclass_dynamic.A",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrSendableClassCtor(graph, isWideMode);
        },
        [&](AbckitGraph *graph) { VerifyIrSendableClassCtor(graph, isWideMode); });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/makesendableclass_dynamic_modified.abc",
                                   "makesendableclass_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\nHello\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeDefinesendableclass, abc-kind=ArkTS1,
// category=positive, extension=c
TEST_F(LibAbcKitCreateDynSendable, DefineSendableClass)
{
    ModifySendableClass(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, LdSendableVar_imm4)
{
    ModifySendableClass(false, 0x0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, LdSendableVar_imm8)
{
    ModifySendableClass(false, 0x10);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeNewsendableenv, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, NewSendableEnv)
{
    ModifySendableClass(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeStsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, StSendableVar_imm4)
{
    ModifySendableClass(false, 0x0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeStsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, StSendableVar_imm8)
{
    ModifySendableClass(false, 0x10);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendableexternalmodulevar, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitCreateDynSendable, LdExternalModuleVar)
{
    ModifySendableClass(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeLdsendableclass, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, LdSendableClass)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/ldsendableclass_dynamic.abc",
                                             "ldsendableclass_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/ldsendableclass_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/ldsendableclass_dynamic_modified.abc", "ldsendableclass_dynamic.B",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrLdSendableClass(file, graph);
        },
        [&](AbckitGraph *graph) { VerifyIrLdSendableClass(graph); });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/sendable/ldsendableclass_dynamic_modified.abc", "ldsendableclass_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeWideldsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, WideLdSendableVar)
{
    ModifySendableClass(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeWidenewsendableenv, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, WideNewSendableEnv)
{
    ModifySendableClass(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeWidestsendablevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSendable, WideStSendableVar)
{
    ModifySendableClass(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeWideldsendableexternalmodulevar, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitCreateDynSendable, WideLdExternalModuleVar)
{
    ModifySendableClass(true);
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
