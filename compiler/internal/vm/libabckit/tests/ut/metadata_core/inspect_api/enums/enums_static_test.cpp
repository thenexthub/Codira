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
#include <vector>
#include <algorithm>
#include <string>
#include <set>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiEnumsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::enumGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetFileStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");
    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            [[maybe_unused]] auto filePtr = enm.GetFile();
            ASSERT_EQ(filePtr, &file);
        }
    }
}

// Test: test-kind=api, api=InspectApiImpl::enumGetModule, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetModuleStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");
    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            [[maybe_unused]] auto modulePtr = enm.GetModule();
            ASSERT_EQ(modulePtr, module);
        }
    }
}

// Test: test-kind=api, api=InspectApiImpl::enumGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> enumNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            enumNames.emplace_back(enm.GetName());
        }
    }

    ASSERT_EQ(enumNames[0], "EnumA");
    ASSERT_EQ(enumNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::enumEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetAllMethodsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> methodNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            for (const auto &method : enm.GetAllMethods()) {
                methodNames.emplace_back(method.GetName());
            }
        }
    }

    std::vector<std::string> actualMethodNames = {"$_get:enums_static.EnumA;std.core.String;",
                                                  "_cctor_:void;",
                                                  "fromValue:i32;enums_static.EnumA;",
                                                  "getValueOf:std.core.String;enums_static.EnumA;",
                                                  "values:enums_static.EnumA[];",
                                                  "_ctor_:enums_static.EnumA;i32;i32;void;",
                                                  "getName:enums_static.EnumA;std.core.String;",
                                                  "getOrdinal:enums_static.EnumA;i32;",
                                                  "toString:enums_static.EnumA;std.core.String;",
                                                  "valueOf:enums_static.EnumA;i32;"};
    sort(actualMethodNames.begin(), actualMethodNames.end());
    sort(methodNames.begin(), methodNames.end());
    ASSERT_EQ(methodNames, actualMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> fieldNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            for (const auto &field : enm.GetFields()) {
                fieldNames.emplace_back(field.GetName());
            }
        }
    }

    std::vector<std::string> actualFieldName = {
        "#ordinal", "ONE", "TWO", "THREE", "#NamesArray", "#ValuesArray", "#StringValuesArray", "#ItemsArray"};

    sort(actualFieldName.begin(), actualFieldName.end());
    sort(fieldNames.begin(), fieldNames.end());
    ASSERT_EQ(fieldNames, actualFieldName);
}

// Test: test-kind=api, api=InspectApiImpl::enumIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumIsExternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::set<std::string> gotEnumNames;
    std::set<std::string> expectEnumNames = {"EnumA"};

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            if (!enm.IsExternal()) {
                gotEnumNames.insert(enm.GetName());
            }
        }
    }

    ASSERT_EQ(gotEnumNames, expectEnumNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumGetParentNamespace, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetParentNamespaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::set<std::string> gotNames;
    std::set<std::string> expectNames = {"Ns1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetNamespaceByName(module, "Ns1");
        ASSERT_NE(result, std::nullopt);
        const auto &ns = result.value();
        for (const auto &enm : ns.GetEnums()) {
            gotNames.emplace(enm.GetParentNamespace().GetName());
        }
    }

    ASSERT_EQ(gotNames, expectNames);
}
}  // namespace libabckit::test