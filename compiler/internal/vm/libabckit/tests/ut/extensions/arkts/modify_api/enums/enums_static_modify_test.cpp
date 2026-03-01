/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "libabckit/cpp/abckit_cpp.h"
#include "tests/helpers/helpers.h"
#include "tests/helpers/helpers_runtime.h"

namespace {
constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/enums/enums_static_modify.abc";
constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/enums/enums_static_modify_modified.abc";
constexpr auto NEW_NAME = "New";
}  // namespace

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiEnumsTest : public ::testing::Test {};

static AbckitCoreEnum *GetAbckitCoreEnum(AbckitFile *file, std::string moduleName, std::string enumName)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::EnumByNameContext ceFinder = {nullptr, enumName.c_str()};
    g_implI->moduleEnumerateEnums(module, &ceFinder, helpers::EnumByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(ceFinder.enm != nullptr);

    return ceFinder.enm;
}

// Test: test-kind=api, api=ArktsModifyApiImpl::enumSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiEnumsTest, EnumSetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "enums_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;

    helpers::EnumByNameContext enumFinder = {nullptr, "Enum1"};
    g_implI->moduleEnumerateEnums(module, &enumFinder, helpers::EnumByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(enumFinder.enm, nullptr);
    auto arkE = g_implArkI->coreEnumToArktsEnum(enumFinder.enm);
    g_implArkM->enumSetName(arkE, NEW_NAME);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    helpers::ModuleByNameContext newFinder = {nullptr, "enums_static_modify"};
    g_implI->fileEnumerateModules(file, &newFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newFinder.module, nullptr);
    auto newModule = newFinder.module;

    helpers::EnumByNameContext newEnumFinder = {nullptr, NEW_NAME};
    g_implI->moduleEnumerateEnums(newModule, &newEnumFinder, helpers::EnumByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newEnumFinder.enm, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "enums_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "enums_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::EnumAddField, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiEnumsTest, EnumAddField)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    auto coreEnum = GetAbckitCoreEnum(file, "enums_static_modify", "Enum1");

    struct AbckitArktsFieldCreateParams params;
    params.name = "YELLOW";
    params.type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    std::string moduleFieldValue = "hello";
    params.value = g_implM->createValueString(file, moduleFieldValue.c_str(), moduleFieldValue.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    params.isStatic = true;
    params.fieldVisibility = AbckitArktsFieldVisibility::PUBLIC;

    auto ret = g_implArkM->enumAddField(g_implArkI->coreEnumToArktsEnum(coreEnum), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);

    auto coreEnum2 = GetAbckitCoreEnum(file, "enums_static_modify", "Enum1");

    std::vector<std::string> FieldNames;
    g_implI->enumEnumerateFields(coreEnum2, &FieldNames, [](AbckitCoreEnumField *field, void *data) {
        auto ctx = static_cast<std::vector<std::string> *>(data);
        auto filedName = g_implI->abckitStringToString(g_implI->enumFieldGetName(field));
        (*ctx).emplace_back(filedName);
        return true;
    });
    std::vector<std::string> actualFieldNames = {
        "#ordinal",           "RED",         "BLUE",  "BLACK", "#NamesArray", "#ValuesArray",
        "#StringValuesArray", "#ItemsArray", "YELLOW"};
    ASSERT_EQ(FieldNames, actualFieldNames);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "enums_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "enums_static_modify", "main"));
}

}  // namespace libabckit::test
