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
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_staG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_arktsInspectApiImpl = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_arktsModifyApiImpl = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiTypesTest : public ::testing::Test {};

static constexpr auto INITIAL_STATIC = ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_static.abc";
static constexpr auto MODIFIED_STATIC = ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_static_modified.abc";

static void InspectMethodValid(AbckitFile *file)
{
    std::vector<AbckitTypeId> vecOfTypes = {
        ABCKIT_TYPE_ID_VOID, ABCKIT_TYPE_ID_U1,  ABCKIT_TYPE_ID_I8,  ABCKIT_TYPE_ID_U8,          ABCKIT_TYPE_ID_I16,
        ABCKIT_TYPE_ID_U16,  ABCKIT_TYPE_ID_I32, ABCKIT_TYPE_ID_U32, ABCKIT_TYPE_ID_F32,         ABCKIT_TYPE_ID_F64,
        ABCKIT_TYPE_ID_I64,  ABCKIT_TYPE_ID_U64, ABCKIT_TYPE_ID_ANY, ABCKIT_TYPE_ID_LITERALARRAY};
    for (AbckitTypeId typeId : vecOfTypes) {
        AbckitType *type = g_implM->createType(file, typeId);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitTypeId gotTypeId = g_implI->typeGetTypeId(type);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(gotTypeId, typeId);
    }
}

// Test: test-kind=api, api=ModifyApiImpl::createType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateTypesDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo",
                           [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) { InspectMethodValid(file); });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ModifyApiImpl::createType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateTypeStatic)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_static.abc", "foo",
                           [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) { InspectMethodValid(file); });
}

// Test: test-kind=api, api=ModifyApiImpl::createReferenceType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateReferenceTypeDynamic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/types/types_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::InspectMethod(file, "foo2", [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
        auto *meth = helpers::FindMethodByName(file, "foo");
        ASSERT_NE(meth, nullptr);
        auto *cls = g_implI->functionGetParentClass(meth);
        ASSERT_NE(cls, nullptr);
        auto *type = g_implM->createReferenceType(file, cls);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(g_implI->typeGetTypeId(type), ABCKIT_TYPE_ID_REFERENCE);
        ASSERT_EQ(g_implI->typeGetReferenceClass(type), cls);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ModifyApiImpl::typeSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeSetNameStatic)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "foo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
            AbckitType *type = g_implM->createType(file, ABCKIT_TYPE_ID_REFERENCE);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            const char *name = "std.core.Int";
            g_implM->typeSetName(type, name, strlen(name));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto typeName = g_implI->typeGetName(type);
            ASSERT_EQ(g_implI->abckitStringToString(typeName), std::string(name));
        },
        [](AbckitGraph *) {});
}

// Test: test-kind=api, api=ModifyApiImpl::typeSetRank, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, TypeSetRankStatic)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "foo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
            AbckitType *type = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implM->typeSetRank(type, 1);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_EQ(g_implI->typeGetRank(type), 1);
        },
        [](AbckitGraph *) {});
}

// Test: test-kind=api, api=ModifyApiImpl::createUnionType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiTypesTest, CreateUnionTypeStatic)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "foo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
            auto *types = new AbckitType *[2];
            const char *typeIntName = "std.core.Int";
            types[0] = g_implM->createType(file, ABCKIT_TYPE_ID_REFERENCE);
            g_implM->typeSetName(types[0], typeIntName, strlen(typeIntName));
            ASSERT_NE(types[0], nullptr);

            const char *typeStringName = "std.core.String";
            types[1] = g_implM->createType(file, ABCKIT_TYPE_ID_REFERENCE);
            g_implM->typeSetName(types[1], typeStringName, strlen(typeStringName));
            ASSERT_NE(types[1], nullptr);

            auto *unionType = g_implM->createUnionType(file, types, 2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(unionType, nullptr);

            ASSERT_TRUE(g_implI->typeIsUnion(unionType));
            std::vector<std::string> unionTypes;
            std::vector<std::string> expectedUnionTypes = {"std.core.Int", "std.core.String"};
            g_implI->typeEnumerateUnionTypes(unionType, &unionTypes, [](AbckitType *type, void *data) {
                auto unionTypes = (std::vector<std::string> *)data;
                auto typeName = g_implI->typeGetName(type);
                unionTypes->push_back(g_implI->abckitStringToString(typeName));
                return true;
            });
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_EQ(unionTypes, expectedUnionTypes);
            delete[] types;
        },
        [](AbckitGraph *) {});
}

}  // namespace libabckit::test
