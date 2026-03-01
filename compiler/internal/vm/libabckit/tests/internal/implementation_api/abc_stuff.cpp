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

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitAbcStuff : public ::testing::Test {};

struct ClassData {
    const char *className = nullptr;
    std::vector<const char *> classMethods = {};
};

struct ModuleData {
    const char *moduleName = nullptr;
    std::vector<const char *> moduleMethods = {};
};

// Test: test-kind=api, api=ApiImpl::openAbc, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, OpenAbcStatic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_static.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::vector<struct ClassData> userData = {{"ClassA", {"foo", "bar"}}, {"ClassB", {"baz", "func"}}};

    g_implI->fileEnumerateModules(file, &userData, [](AbckitCoreModule *m, void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        auto *userData = reinterpret_cast<std::vector<struct ClassData> *>(data);

        for (const auto &classData : *userData) {
            helpers::ClassByNameContext classCtxFinder = {nullptr, classData.className};
            g_implI->moduleEnumerateClasses(m, &classCtxFinder, helpers::ClassByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(classCtxFinder.klass, nullptr);

            for (auto *method : classData.classMethods) {
                helpers::MethodByNameContext methodCtxFinder = {nullptr, method};
                g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
                EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                EXPECT_NE(methodCtxFinder.method, nullptr);
            }
        }

        return true;
    });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ApiImpl::writeAbc, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, WriteAbcStatic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_static.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::vector<struct ClassData> userData = {{"ClassA", {"foo", "bar"}}, {"ClassB", {"baz", "func"}}};

    g_implI->fileEnumerateModules(file, &userData, [](AbckitCoreModule *m, void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        auto *userData = reinterpret_cast<std::vector<struct ClassData> *>(data);

        for (const auto &classData : *userData) {
            helpers::ClassByNameContext classCtxFinder = {nullptr, classData.className};
            g_implI->moduleEnumerateClasses(m, &classCtxFinder, helpers::ClassByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(classCtxFinder.klass, nullptr);

            for (auto *method : classData.classMethods) {
                helpers::MethodByNameContext methodCtxFinder = {nullptr, method};
                g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
                EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                EXPECT_NE(methodCtxFinder.method, nullptr);
            }
        }

        return true;
    });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ApiImpl::closeFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, CloseFileStatic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_static.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ApiImpl::openAbc, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, OpenAbcDynamic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    struct ModuleData userData = {"abc_dynamic", {"foo", "bar", "baz", "func"}};

    helpers::ModuleByNameContext moduleCtxFinder = {nullptr, userData.moduleName};
    g_implI->fileEnumerateModules(file, &moduleCtxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(moduleCtxFinder.module, nullptr);

    for (auto method : userData.moduleMethods) {
        auto *found = helpers::FindMethodByName(file, method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(found, nullptr);
    }

    std::set<std::string> anonymousMethods = {
        "abc_dynamic.#*#",
        "abc_dynamic.#*#^1",
        "abc_dynamic.#*#anonymousfunc",
    };
    helpers::EnumerateAllMethods(file, [&anonymousMethods](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        bool isAnonymous = g_implI->functionIsAnonymous(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (isAnonymous) {
            EXPECT_EQ(anonymousMethods.count(methodName.data()), 1);
            anonymousMethods.erase(methodName.data());
        } else {
            EXPECT_EQ(anonymousMethods.count(methodName.data()), 0);
        }
    });
    ASSERT_TRUE(anonymousMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ApiImpl::writeAbc, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, WriteAbcDynamic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    struct ModuleData userData = {"abc_dynamic", {"foo", "bar", "baz", "func"}};

    helpers::ModuleByNameContext moduleCtxFinder = {nullptr, userData.moduleName};
    g_implI->fileEnumerateModules(file, &moduleCtxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(moduleCtxFinder.module, nullptr);

    for (auto method : userData.moduleMethods) {
        auto *found = helpers::FindMethodByName(file, method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(found, nullptr);
    }

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ApiImpl::closeFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitAbcStuff, CloseFileDynamic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "internal/implementation_api/abc_dynamic.abc";
    auto *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
