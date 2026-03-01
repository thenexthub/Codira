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
#include <string>
#include <optional>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiFieldsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::moduleFieldGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ModuleFieldGetTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotTypeNames;
    std::set<std::string> expectTypeNames = {"std.core.String"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &field : module.GetFields()) {
            gotTypeNames.emplace(field.GetType().GetName());
        }
    }

    ASSERT_EQ(gotTypeNames, expectTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::moduleFieldGetValue, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ModuleFieldGetValueStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    using FieldVal = std::variant<bool, int, double, std::string>;
    std::vector<FieldVal> gotFieldValues;
    std::vector<FieldVal> expectFieldValues = {std::string("Hello")};
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &field : module.GetFields()) {
            auto value = field.GetValue();
            auto typeId = value.GetType().GetTypeId();
            static const std::map<AbckitTypeId, std::function<void()>> valueHandlers = {
                {ABCKIT_TYPE_ID_U1,
                 [&]() {
                     bool val = value.GetU1();
                     gotFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_I32,
                 [&]() {
                     int val = value.GetInt();
                     gotFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_F64,
                 [&]() {
                     double val = value.GetDouble();
                     gotFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_STRING, [&]() {
                     std::string val = value.GetString();
                     gotFieldValues.push_back(val);
                 }}};
            auto handler = valueHandlers.find(typeId);
            if (handler != valueHandlers.end()) {
                handler->second();
            }
        }
    }

    ASSERT_EQ(gotFieldValues, expectFieldValues);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceFieldGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, NamespaceFieldGetTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotTypeNames;
    std::set<std::string> expectTypeNames = {"std.core.Array"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &field : ns.GetFields()) {
                gotTypeNames.emplace(field.GetType().GetName());
            }
        }
    }

    ASSERT_EQ(gotTypeNames, expectTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceFieldGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, NamespaceFieldGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotNames;
    std::set<std::string> expectNames = {"nS1F1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &field : ns.GetFields()) {
                gotNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotNames, expectNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceFieldGetNamespace, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, NamespaceFieldGetNamespaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotNames;
    std::set<std::string> expectNames = {"NS1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &field : ns.GetFields()) {
                gotNames.emplace(field.GetNamespace().GetName());
            }
        }
    }

    ASSERT_EQ(gotNames, expectNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldGetTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotTypeNames;
    std::set<std::string> expectTypeNames = {"std.core.String", "i32"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &klass : module.GetClasses()) {
            for (const auto &field : klass.GetFields()) {
                gotTypeNames.emplace(field.GetType().GetName());
            }
        }
    }

    ASSERT_EQ(gotTypeNames, expectTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldGetValue, abc-kind=ArkTS2, category=positive, extension=c

TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldGetValueStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");
    using FieldVal = std::variant<bool, int, double, std::string>;
    std::vector<FieldVal> gotClassFieldValues;
    std::vector<FieldVal> expectClassFieldValues = {
        std::string("public"), std::string("protected"), std::string("private"), std::string("static"), 2, 1};
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            auto value = field.GetValue();
            auto type = field.GetType();
            auto typeId = value.GetType().GetTypeId();
            static const std::map<AbckitTypeId, std::function<void()>> valueHandlers = {
                {ABCKIT_TYPE_ID_U1,
                 [&]() {
                     bool val = value.GetU1();
                     gotClassFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_F64,
                 [&]() {
                     double val = value.GetDouble();
                     gotClassFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_STRING,
                 [&]() {
                     std::string val = value.GetString();
                     gotClassFieldValues.push_back(val);
                 }},
                {ABCKIT_TYPE_ID_I32, [&]() {
                     int val = value.GetInt();
                     gotClassFieldValues.push_back(val);
                 }}};

            auto handler = valueHandlers.find(typeId);
            if (handler != valueHandlers.end()) {
                handler->second();
            } else {
                std::string val = "[unsupported type " + std::to_string(static_cast<int>(typeId)) + "]";
                gotClassFieldValues.push_back(val);
            }
        }
    }

    ASSERT_EQ(gotClassFieldValues, expectClassFieldValues);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsPublic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsPublicStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F1", "c1F4", "c1F6", "c1F7"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsPublic()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsProtected, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsProtectedStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsProtected()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsPrivate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsPrivateStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsPrivate()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsInternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsInternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F5"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsInternal()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_NE(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsStaticStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F4"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsStatic()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsFinal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldIsFinalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"c1F7"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsFinal()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldEnumerateAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, ClassFieldGetAnnotationsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            for (const auto &annotation : field.GetAnnotations()) {
                gotAnnotationNames.emplace(annotation.GetName());
            }
        }
    }

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceFieldGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, InterfaceFieldGetTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotTypeNames;
    std::set<std::string> expectTypeNames = {"f64", "std.core.String"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &iface : module.GetInterfaces()) {
            for (const auto &field : iface.GetFields()) {
                gotTypeNames.emplace(field.GetType().GetName());
            }
        }
    }

    ASSERT_EQ(gotTypeNames, expectTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceFieldIsReadonly, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, InterfaceFieldIsReadonlyStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"%%property-i1F2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            if (field.IsReadonly()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceFieldEnumerateAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, InterfaceFieldGetAnnotationsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"AI3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            for (const auto &annotation : field.GetAnnotations()) {
                gotAnnotationNames.emplace(annotation.GetName());
            }
        }
    }

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceFieldGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, InterfaceFieldGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"I1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            gotInterfaceNames.emplace(field.GetInterface().GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumFieldGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, EnumFieldGetTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotTypeNames;
    std::set<std::string> expectTypeNames = {"fields_static.E1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetEnumByName(module, "E1");
        ASSERT_NE(result, std::nullopt);
        const auto &enm = result.value();
        for (const auto &field : enm.GetFields()) {
            if (field.GetName() == "ONE") {
                gotTypeNames.emplace(field.GetType().GetName());
            }
        }
    }

    ASSERT_EQ(gotTypeNames, expectTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumFieldGetEnum, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, EnumFieldGetEnumStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotNames;
    std::set<std::string> expectNames = {"E1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &enm : module.GetEnums()) {
            for (const auto &field : enm.GetFields()) {
                gotNames.emplace(field.GetEnum().GetName());
            }
        }
    }

    ASSERT_EQ(gotNames, expectNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumFieldGetValue, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFieldsTest, EnumFieldGetValueStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<int> gotFieldValues;
    std::set<int> expectFieldValues = {0};
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetEnumByName(module, "E1");
        ASSERT_NE(result, std::nullopt);
        const auto &enm = result.value();
        for (const auto &field : enm.GetFields()) {
            if (field.GetName() == "ONE") {
                gotFieldValues.emplace(field.GetValue().GetInt());
            }
        }
    }

    ASSERT_EQ(gotFieldValues, expectFieldValues);
}

}  // namespace libabckit::test
