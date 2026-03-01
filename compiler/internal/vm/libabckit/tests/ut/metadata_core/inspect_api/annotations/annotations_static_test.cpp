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
#include "helpers/helpers_runtime.h"

namespace libabckit::test {

class LibAbcKitInspectApiAnnotationsTest : public ::testing::Test {};

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

constexpr auto TEST_ABC_PATH = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc";
constexpr auto MODIFY_TEST_ABC_PATH =
    ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static_modified.abc";
// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationInterfaceGetNameStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1", "AI2", "AI3", "AI4"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ai : module.GetAnnotationInterfaces()) {
            gotInterfaceNames.emplace(ai.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationGetNameStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &anno : klass.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationIsExternalStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInternalAnnotationNames;
    std::set<std::string> gotExternalAnnotationNames;
    std::set<std::string> expectInternalAnnotationNames = {"AI4"};
    std::set<std::string> expectExternalAnnotationNames = {"ets.annotation.EnclosingClass",
                                                           "ets.annotation.InnerClass"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetNamespaceByName(module, "Ns1");
        ASSERT_NE(result, std::nullopt);
        const auto &ns = result.value();
        const auto &classes = ns.GetClasses();
        const auto &klass = classes.front();
        for (const auto &anno : klass.GetAnnotations()) {
            if (anno.IsExternal()) {
                gotExternalAnnotationNames.emplace(anno.GetName());
                continue;
            }
            gotInternalAnnotationNames.emplace(anno.GetName());
        }
    }

    ASSERT_EQ(gotInternalAnnotationNames, expectInternalAnnotationNames);
    ASSERT_EQ(gotExternalAnnotationNames, expectExternalAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, ClassAnnotationGetInterfaceStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &klass : module.GetClasses()) {
            for (const auto &anno : klass.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, InterfaceAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &anno : iface.GetAnnotations()) {
            gotInterfaceNames.emplace(anno.GetInterface().GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, ClassFieldAnnotationGetInterfaceStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            for (const auto &anno : field.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, InterfaceFieldAnnotationGetInterfaceStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            for (const auto &anno : field.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, FunctionAnnotationGetInterfaceStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &function : module.GetTopLevelFunctions()) {
            for (const auto &anno : function.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationInterfaceGetAnnotationInterfaceFieldStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"f1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ai : module.GetAnnotationInterfaces()) {
            for (const auto &field : ai.GetFields()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetParentNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationInterfaceGetParentNamespaceStatic)
{
    abckit::File file(TEST_ABC_PATH);

    std::set<std::string> gotAINames;
    std::set<std::string> expectAINames = {"Ns1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetNamespaceByName(module, "Ns1");
        ASSERT_NE(result, std::nullopt);
        const auto &ns = result.value();
        for (const auto &ai : ns.GetAnnotationInterfaces()) {
            gotAINames.emplace(ai.GetParentNamespace().GetName());
        }
    }

    ASSERT_EQ(gotAINames, expectAINames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetParentNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, TestAyncFunctionFix_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH, &file);
    auto *method = helpers::FindMethodByName(file, "test");
    ASSERT_NE(method, nullptr);
    g_impl->writeAbc(file, MODIFY_TEST_ABC_PATH, strlen(MODIFY_TEST_ABC_PATH));
    g_impl->closeFile(file);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(TEST_ABC_PATH, "annotations_static", "main"),
              helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "annotations_static", "main"));
}

}  // namespace libabckit::test