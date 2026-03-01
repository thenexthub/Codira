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

#include <cassert>
#include <cstring>
#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "logger.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto MODIFIED_DYNAMIC =
    ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic_modified.abc";
static constexpr auto MODIFIED_STATIC =
    ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static_modified.abc";
static constexpr auto MODIFIED_EMPTY_DYNAMIC =
    ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_empty_dynamic_modified.abc";

class LibAbcKitInspectApiClassesTest : public ::testing::Test {};

static bool ClassCountrer(AbckitCoreClass *klass, void *data)
{
    helpers::AssertClassVisitor(klass, data);
    (*(reinterpret_cast<std::size_t *>(data)))++;
    return true;
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, StaticModuleEnumerateClasses)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::size_t gotNumClasses = 0;
        g_implI->moduleEnumerateClasses(m, &gotNumClasses, ClassCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_CLASSES = 4;
        EXPECT_EQ(gotNumClasses, EXPECTED_NUM_CLASSES);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicModuleEnumerateClasses)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::size_t gotNumClasses = 0;
        g_implI->moduleEnumerateClasses(m, &gotNumClasses, ClassCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_CLASSES = 1;
        EXPECT_EQ(gotNumClasses, EXPECTED_NUM_CLASSES);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, StaticModuleEnumerateClassesEmpty)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_empty_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::size_t gotNumClasses = 0;
        g_implI->moduleEnumerateClasses(m, &gotNumClasses, ClassCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_CLASSES = 1;
        EXPECT_EQ(gotNumClasses, EXPECTED_NUM_CLASSES);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicModuleEnumerateClassesEmpty)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_empty_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::size_t gotNumClasses = 0;
        g_implI->moduleEnumerateClasses(m, &gotNumClasses, ClassCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_CLASSES = 1;
        EXPECT_EQ(gotNumClasses, EXPECTED_NUM_CLASSES);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassGetNameSmoke)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        AbckitString *name = nullptr;
        name = g_implI->classGetName(nullptr);
        EXPECT_EQ(name, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        name = g_implI->classGetName(ctxFinder.klass);
        EXPECT_NE(name, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *file = g_implI->moduleGetFile(m);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(file, nullptr);
        auto nameStr = helpers::AbckitStringToString(name);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(nameStr, "C1");

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassGetNameSmoke)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        AbckitString *name = nullptr;
        name = g_implI->classGetName(nullptr);
        EXPECT_EQ(name, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        name = g_implI->classGetName(ctxFinder.klass);
        EXPECT_NE(name, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *file = g_implI->moduleGetFile(m);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(file, nullptr);
        auto nameStr = helpers::AbckitStringToString(name);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(nameStr, "C1");

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static bool ClassNameCollector(AbckitCoreClass *klass, void *data)
{
    helpers::AssertClassVisitor(klass, data);

    AbckitFile *file = g_implI->classGetFile(klass);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::set<std::string> *>(data);
    auto className = g_implI->classGetName(klass);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(className);
    LIBABCKIT_LOG(DEBUG) << "class name: " << name << std::endl;
    EXPECT_TRUE(names->find(name.data()) == names->end());
    names->insert(name.data());

    return true;
}

// Test: test-kind=api, api=InspectApiImpl::classGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::set<std::string> gotNames;

        g_implI->moduleEnumerateClasses(m, &gotNames, ClassNameCollector);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        std::set<std::string> expectedNames = {"C1", "C2", "C3", "C4"};
        EXPECT_EQ(expectedNames.size(), gotNames.size());
        for (auto &expectedName : expectedNames) {
            EXPECT_NE(gotNames.find(expectedName), gotNames.end());
        }

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        std::set<std::string> gotNames;
        g_implI->moduleEnumerateClasses(m, &gotNames, ClassNameCollector);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        std::set<std::string> expectedNames = {"C1"};
        EXPECT_EQ(expectedNames.size(), gotNames.size());
        for (auto &expectedName : expectedNames) {
            EXPECT_NE(gotNames.find(expectedName), gotNames.end());
        }

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static bool DummyMethodVisitor(AbckitCoreFunction *method, void * /*data*/)
{
    EXPECT_TRUE(method != nullptr);
    [[maybe_unused]] AbckitFile *file = g_implI->functionGetFile(method);
    EXPECT_TRUE(file != nullptr);
    return true;
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassEnumerateMethodsSmoke)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);
        auto *klass = ctxFinder.klass;

        g_implI->classEnumerateMethods(nullptr, nullptr, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        g_implI->classEnumerateMethods(klass, nullptr, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        const auto implSomeData = 0xcafef00d;
        void *someData = reinterpret_cast<void *>(implSomeData);
        g_implI->classEnumerateMethods(klass, someData, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        g_implI->classEnumerateMethods(ctxFinder.klass, nullptr, DummyMethodVisitor);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implI->classEnumerateMethods(ctxFinder.klass, someData, DummyMethodVisitor);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassEnumerateMethodsSmoke)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        auto *klass = ctxFinder.klass;

        g_implI->classEnumerateMethods(nullptr, nullptr, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        g_implI->classEnumerateMethods(klass, nullptr, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        const auto implSomeData = 0xcafef00d;
        void *someData = reinterpret_cast<void *>(implSomeData);
        g_implI->classEnumerateMethods(klass, someData, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

        g_implI->classEnumerateMethods(ctxFinder.klass, nullptr, DummyMethodVisitor);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implI->classEnumerateMethods(ctxFinder.klass, someData, DummyMethodVisitor);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static bool MethodCountrer(AbckitCoreFunction *method, void *data)
{
    helpers::AssertMethodVisitor(method, data);
    (*(reinterpret_cast<std::size_t *>(data)))++;
    return true;
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassEnumerateMethodsEmpty)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_empty_static.abc", &file);

    // class C1 {
    //     // Empty body, no explicit methods
    //     <ctor>
    // }

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);
        auto *klass = ctxFinder.klass;

        std::size_t gotNumMethods = 0;
        g_implI->classEnumerateMethods(klass, &gotNumMethods, MethodCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_METHODS = 1;
        EXPECT_EQ(gotNumMethods, EXPECTED_NUM_METHODS);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassEnumerateMethodsEmpty)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_empty_dynamic.abc", &file);

    // class C1 {
    //     // Empty body, no explicit methods
    //     <ctor>
    // }

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);
        auto *klass = ctxFinder.klass;

        std::size_t gotNumMethods = 0;
        g_implI->classEnumerateMethods(klass, &gotNumMethods, MethodCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_METHODS = 1;
        EXPECT_EQ(gotNumMethods, EXPECTED_NUM_METHODS);

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_EMPTY_DYNAMIC, strlen(MODIFIED_EMPTY_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassEnumerateMethodsSeveralMethods)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    // class C1 {
    //     public C1M1()
    //     static C1M2()
    //     <ctor>
    // }

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);
        auto *klass = ctxFinder.klass;

        std::size_t gotNumMethods = 0;
        g_implI->classEnumerateMethods(klass, &gotNumMethods, MethodCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_METHODS = 3;
        EXPECT_EQ(gotNumMethods, EXPECTED_NUM_METHODS);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassEnumerateMethodsSeveralMethods)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    // class C1 {
    //     public C1M1()
    //     static C1M2()
    //     <ctor>
    // }

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);
        auto *klass = ctxFinder.klass;

        std::size_t gotNumMethods = 0;
        g_implI->classEnumerateMethods(klass, &gotNumMethods, MethodCountrer);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        constexpr std::size_t EXPECTED_NUM_METHODS = 3;
        EXPECT_EQ(gotNumMethods, EXPECTED_NUM_METHODS);

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_EMPTY_DYNAMIC, strlen(MODIFIED_EMPTY_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, ClassGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        auto *ctxI1 = g_implI->moduleGetFile(m);

        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        auto *ctxI2 = g_implI->classGetFile(nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        EXPECT_EQ(ctxI2, nullptr);

        ctxI2 = g_implI->classGetFile(ctxFinder.klass);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(ctxI1, ctxI2);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        auto *ctxI1 = g_implI->moduleGetFile(m);

        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        auto *ctxI2 = g_implI->classGetFile(nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        EXPECT_EQ(ctxI2, nullptr);

        ctxI2 = g_implI->classGetFile(ctxFinder.klass);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(ctxI1, ctxI2);

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetModule, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassGetModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        auto moduleNameRaw = g_implI->moduleGetName(g_implI->classGetModule(ctxFinder.klass));
        EXPECT_NE(moduleNameRaw, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        auto moduleName = helpers::AbckitStringToString(moduleNameRaw);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(moduleName, "classes_dynamic");

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetModule, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, StaticClassGetModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        auto moduleNameRaw = g_implI->moduleGetName(g_implI->classGetModule(ctxFinder.klass));
        EXPECT_NE(moduleNameRaw, nullptr);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        auto moduleName = helpers::AbckitStringToString(moduleNameRaw);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(moduleName, "classes_static");

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classGetParentFunction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiClassesTest, DynamicClassGetParentFunction)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/classes/classes_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext c0Finder = {nullptr, "C1"};
        g_implI->moduleEnumerateClasses(m, &c0Finder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(c0Finder.klass, nullptr);

        auto parentFunction1 = g_implI->classGetParentFunction(c0Finder.klass);
        EXPECT_EQ(parentFunction1, nullptr);

        helpers::MethodByNameContext methodFinder = {nullptr, "f"};
        g_implI->moduleEnumerateTopLevelFunctions(m, &methodFinder, helpers::MethodByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(methodFinder.method, nullptr);

        helpers::ClassByNameContext c1Finder = {nullptr, "C2"};
        g_implI->functionEnumerateNestedClasses(methodFinder.method, &c1Finder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(c1Finder.klass, nullptr);

        auto parentFunc = g_implI->classGetParentFunction(c1Finder.klass);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(parentFunc, methodFinder.method);

        return true;
    });

    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
