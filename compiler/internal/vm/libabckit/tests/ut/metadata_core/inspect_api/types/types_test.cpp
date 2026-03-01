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

#include <gtest/gtest.h>
#include <cstring>

#include "libabckit/c/abckit.h"
#include "libabckit/c/statuses.h"
#include "helpers/helpers.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiTypesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::typeGetTypeId, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetTypeIdDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_ANY);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetTypeId, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetTypeIdStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->iGetNext(g_implG->bbGetFirstInst(startBB));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_I32);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *function, AbckitGraph *) {
        AbckitType *type = g_implI->functionGetReturnType(function);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitString *name = g_implI->typeGetName(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_STREQ(g_implI->abckitStringToString(name), "i32");
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetRank, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetRankStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "bar", [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
        AbckitType *type = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
        g_implM->typeSetRank(type, 1);
        ASSERT_EQ(g_implI->typeGetRank(type), 1);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetReferenceClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetReferenceClassStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        inst = g_implG->iGetNext(g_implG->iGetNext(inst));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, ABCKIT_TYPE_ID_REFERENCE);
        AbckitCoreClass *klass = g_implI->typeGetReferenceClass(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(klass, nullptr);  // Fix to get valid AbckitCoreClass *, not nullptr
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeIsUnion, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeIsUnionStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/union_types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *function, AbckitGraph *) {
        AbckitType *type = g_implI->functionGetReturnType(function);
        auto name = g_implI->typeGetName(type);
        std::string nameStr = g_implI->abckitStringToString(name);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_TRUE(g_implI->typeIsUnion(type));
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeEnumerateUnionTypes, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitModifyApiTypesTest, TypeEnumerateUnionTypesStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/union_types_static.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *function, AbckitGraph *) {
        AbckitType *type = g_implI->functionGetReturnType(function);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        std::vector<std::string> unionTypes;
        std::vector<std::string> expectedUnionTypes = {"std.core.Int", "std.core.String"};
        g_implI->typeEnumerateUnionTypes(type, &unionTypes, [](AbckitType *type, void *data) {
            auto unionTypes = (std::vector<std::string> *)data;
            auto typeName = g_implI->typeGetName(type);
            unionTypes->push_back(g_implI->abckitStringToString(typeName));
            return true;
        });
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(unionTypes, expectedUnionTypes);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::typeGetReferenceClass, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitModifyApiTypesTest, TypeGetReferenceClassDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitInst *inst = g_implG->bbGetFirstInst(startBB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitType *type = g_implG->iGetType(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(gotTypeId, ABCKIT_TYPE_ID_REFERENCE);
        AbckitCoreClass *klass = g_implI->typeGetReferenceClass(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(klass, nullptr);
    });
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
