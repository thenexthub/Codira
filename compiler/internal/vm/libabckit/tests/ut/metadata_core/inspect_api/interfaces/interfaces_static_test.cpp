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
#include <string>
#include <optional>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiInterfacesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::interfaceGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceA", "InterfaceB", "InterfaceC"};

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            gotInterfaceNames.emplace(iface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetAllMethodsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotMethodNames;

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &method : iface.GetAllMethods()) {
            gotMethodNames.emplace(method.GetName());
        }
    }

    std::set<std::string> expectMethodNames = {
        "%%get-fieldA:interfaces_static.InterfaceA;std.core.String;", "%%get-fieldB:interfaces_static.InterfaceA;f64;",
        "%%set-fieldA:interfaces_static.InterfaceA;std.core.String;void;", "bar:interfaces_static.InterfaceA;void;"};
    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotFieldNames;
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            gotFieldNames.emplace(field.GetName());
        }
    }

    std::set<std::string> expectFieldNames = {"%%property-fieldA", "%%property-fieldB"};
    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateSuperInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetSuperInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceA"};
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceB");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &superInterface : iface.GetSuperInterfaces()) {
            gotInterfaceNames.emplace(superInterface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateSubInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetSubInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceB"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &subInterface : iface.GetSubInterfaces()) {
            gotInterfaceNames.emplace(subInterface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateClasses, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetClassesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotClassNames;
    std::set<std::string> expectClassNames = {"ClassB"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceC");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &klass : iface.GetClasses()) {
            gotClassNames.emplace(klass.GetName());
        }
    }

    ASSERT_EQ(gotClassNames, expectClassNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetAnnotationsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceC");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &anno : iface.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceIsExternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceA", "InterfaceB", "InterfaceC"};

    for (const auto &module : file.GetModules()) {
        for (const auto &interface : module.GetInterfaces()) {
            if (!interface.IsExternal()) {
                gotInterfaceNames.emplace(interface.GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetFile, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetFile)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    auto abcFile = g_implI->interfaceGetFile(ifaceCtxFinder.iface);

    ASSERT_NE(abcFile, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetModule)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    auto module = g_implI->interfaceGetModule(ifaceCtxFinder.iface);

    ASSERT_NE(module, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetName)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    ifaceCtxFinder.iface->owningModule->target = ABCKIT_TARGET_ARK_TS_V1;
    auto name = g_implI->interfaceGetName(ifaceCtxFinder.iface);

    ASSERT_EQ(name, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetParentNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetParentNamespace)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::NamespaceByNameContext nameSapceCtx = {nullptr, "MyNamespace"};
    g_implI->moduleEnumerateNamespaces(ctx.module, &nameSapceCtx, helpers::NamespaceByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->namespaceEnumerateInterfaces(nameSapceCtx.n, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    auto petNamespace = g_implI->interfaceGetParentNamespace(ifaceCtxFinder.iface);

    ASSERT_NE(petNamespace, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateSuperInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateSuperInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "ChildInterface"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    ifaceCtxFinder.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;

    void *data = nullptr;
    auto ret = g_implI->interfaceEnumerateSuperInterfaces(ifaceCtxFinder.iface, data, helpers::InterfaceByNameFinder);

    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateSubInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateSubInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    ifaceCtxFinder.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;
    void *data = nullptr;
    auto ret = g_implI->interfaceEnumerateSubInterfaces(ifaceCtxFinder.iface, data, helpers::InterfaceByNameFinder);

    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateClasses, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateClasses)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtx = {nullptr, "Clickable"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtx, helpers::InterfaceByNameFinder);

    ifaceCtx.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;
    helpers::ClassByNameContext classCtx = {nullptr, "MyButton"};
    auto ret = g_implI->interfaceEnumerateClasses(ifaceCtx.iface, &classCtx, helpers::ClassByNameFinder);
    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateMethods, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateMethods)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    ifaceCtxFinder.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;

    helpers::MethodByNameContext methodCtx = {nullptr, ""};
    auto ret = g_implI->interfaceEnumerateMethods(ifaceCtxFinder.iface, &methodCtx, helpers::MethodByNameFinder);

    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateAnnotations)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    ifaceCtxFinder.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;

    helpers::AnnotationByNameContext annotationCtx = {nullptr, ""};
    auto ret =
        g_implI->interfaceEnumerateAnnotations(ifaceCtxFinder.iface, &annotationCtx, helpers::AnnotationByNameFinder);

    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceEnumerateFields, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateFields)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtx = {nullptr, "Animal"};
    g_implI->moduleEnumerateInterfaces(ctx.module, &ifaceCtx, helpers::InterfaceByNameFinder);

    ifaceCtx.iface->owningModule->target = ABCKIT_TARGET_UNKNOWN;
    helpers::FieldByNameContext fieldCtx = {nullptr, ""};
    auto ret = g_implI->interfaceEnumerateFields(ifaceCtx.iface, &fieldCtx, helpers::FieldByNameFinder);

    ASSERT_FALSE(ret);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetFile, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetFile02)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, ""};

    auto abcFile = g_implI->interfaceGetFile(ifaceCtxFinder.iface);

    ASSERT_EQ(abcFile, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetModule02)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, ""};

    auto module = g_implI->interfaceGetModule(ifaceCtxFinder.iface);

    ASSERT_EQ(module, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::InterfaceGetParentNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetParentNamespace02)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interface_test.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "interface_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);

    helpers::NamespaceByNameContext nameSapceCtx = {nullptr, "MyNamespace"};
    g_implI->moduleEnumerateNamespaces(ctx.module, &nameSapceCtx, helpers::NamespaceByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, ""};

    auto petNamespace = g_implI->interfaceGetParentNamespace(ifaceCtxFinder.iface);

    ASSERT_EQ(petNamespace, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace libabckit::test