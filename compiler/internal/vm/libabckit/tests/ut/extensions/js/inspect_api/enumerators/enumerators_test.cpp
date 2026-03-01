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
#include <numeric>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitJSInspectApiEnumeratorsTest : public ::testing::Test {};

// clang-format off
std::unordered_map<std::string, std::vector<std::string>> g_jsM1Names = {
    {"m0N0N0F0", {"JSenumerators0_dynamic"}},
    {"M0N0C0", {"JSenumerators0_dynamic", "M0N0C0"}},
    {"M0N0C0F0", {"JSenumerators0_dynamic", "M0N0C0"}},
    {"M0N0C0F1", {"JSenumerators0_dynamic", "M0N0C0"}},
    {"M0N0C0F2", {"JSenumerators0_dynamic", "M0N0C0"}},
    {"C", {"JSenumerators0_dynamic", "C"}},
    {"D", {"JSenumerators0_dynamic", "C"}},
    {"e", {"JSenumerators0_dynamic"}},
    {"m0N0F0", {"JSenumerators0_dynamic"}},
    {"m0N0F1", {"JSenumerators0_dynamic"}},
    {"m0N0F2", {"JSenumerators0_dynamic"}},
    {"M0C0", {"JSenumerators0_dynamic", "M0C0"}},
    {"M0C0F0", {"JSenumerators0_dynamic", "M0C0"}},
    {"M0C0F1", {"JSenumerators0_dynamic", "M0C0"}},
    {"M0C0F2", {"JSenumerators0_dynamic", "M0C0"}},
    {"m0F0", {"JSenumerators0_dynamic"}},
    {"m0F0F0", {"JSenumerators0_dynamic", "m0F0"}},
    {"m0F0F1", {"JSenumerators0_dynamic", "m0F0"}},
    {"M0F0C0", {"JSenumerators0_dynamic", "m0F0", "M0F0C0"}},
    {"M0F0C0F0", {"JSenumerators0_dynamic", "m0F0", "M0F0C0"}},
    {"m0F1", {"JSenumerators0_dynamic"}},
    {"m0F2", {"JSenumerators0_dynamic"}},
    {"JSenumerators0_dynamic.#*@0*#", {"JSenumerators0_dynamic"}},
    {"JSenumerators0_dynamic.#*@0*#^1", {"JSenumerators0_dynamic"}},
    {"func_main_0", {"JSenumerators0_dynamic"}},
};

std::unordered_map<std::string, std::vector<std::string>> g_jsM2Names = {
    {"M1N0C0", {"modules/JSenumerators1_dynamic", "M1N0C0"}},
    {"M1N0C0F0", {"modules/JSenumerators1_dynamic", "M1N0C0"}},
    {"M1N0C0F1", {"modules/JSenumerators1_dynamic", "M1N0C0"}},
    {"M1N0C0F2", {"modules/JSenumerators1_dynamic", "M1N0C0"}},
    {"m1N0F0", {"modules/JSenumerators1_dynamic"}},
    {"m1N0F1", {"modules/JSenumerators1_dynamic"}},
    {"M1C0", {"modules/JSenumerators1_dynamic", "M1C0"}},
    {"M1C0F0", {"modules/JSenumerators1_dynamic", "M1C0"}},
    {"M1C0F1", {"modules/JSenumerators1_dynamic", "M1C0"}},
    {"M1C0F2", {"modules/JSenumerators1_dynamic", "M1C0"}},
    {"m1F0", {"modules/JSenumerators1_dynamic"}},
    {"m1F1", {"modules/JSenumerators1_dynamic"}},
    {"modules/JSenumerators1_dynamic.#*@0*#", {"modules/JSenumerators1_dynamic"}},
    {"func_main_0", {"modules/JSenumerators1_dynamic"}},
};
// clang-format on

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> g_jsnames = {
    {"JSenumerators0_dynamic", g_jsM1Names},
    {"modules/JSenumerators1_dynamic", g_jsM2Names},
};

std::function<void(AbckitCoreNamespace *)> GetCbNamespace(std::vector<std::string> &curNames,
                                                          std::function<void(AbckitCoreNamespace *)> &cbNamespace,
                                                          std::function<void(AbckitCoreClass *)> &cbClass,
                                                          std::function<void(AbckitCoreFunction *)> &cbFunc)
{
    return [&](AbckitCoreNamespace *n) {
        auto namespaceName = helpers::AbckitStringToString(g_implI->namespaceGetName(n));
        curNames.emplace_back(namespaceName);
        g_implI->namespaceEnumerateNamespaces(n, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(f);
            return true;
        });
        curNames.pop_back();
    };
}

void EnumerateAllMethodsInModule(AbckitFile *file, std::function<void(AbckitCoreClass *)> &cbClass,
                                 std::function<void(AbckitCoreFunction *)> &cbFunc, std::vector<std::string> &curNames,
                                 std::string &curModuleName)
{
    std::function<void(AbckitCoreNamespace *)> cbNamespace = GetCbNamespace(curNames, cbNamespace, cbClass, cbFunc);

    std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
        curModuleName = helpers::AbckitStringToString(g_implI->moduleGetName(m));
        curNames.emplace_back(curModuleName);
        g_implI->moduleEnumerateNamespaces(m, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
        curNames.pop_back();
    };

    g_implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreModule *)> *>(cb))(m);
        return true;
    });
}

struct TestStructType {
    size_t anonFuncsCounter = 0;
    std::string nameModuleToSearch;
    std::vector<std::string> namesOfAnonFuncs;
};

}  // namespace

static void EnumerateAllMethods(
    const char *abcFilePath,
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> expectedNames)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(abcFilePath, &file);

    size_t funcsCounter = 0;
    std::vector<std::string> curNames {};
    std::string curModuleName {};

    std::function<void(AbckitCoreClass *)> cbClass;
    std::function<void(AbckitCoreFunction *)> cbFunc;

    cbFunc = [&](AbckitCoreFunction *f) {
        auto funcName = helpers::AbckitStringToString(g_implI->functionGetName(f));
        ASSERT_EQ(expectedNames.count(curModuleName), 1);
        auto &moduleNames = expectedNames[curModuleName];
        ASSERT_EQ(moduleNames.count(funcName.data()), 1);
        auto &funcNames = moduleNames[funcName.data()];
        ASSERT_EQ(funcNames.size(), curNames.size());
        ASSERT_TRUE(std::equal(funcNames.begin(), funcNames.end(), curNames.begin()));
        funcsCounter++;
        curNames.emplace_back(funcName);
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(f);
            return true;
        });
        g_implI->functionEnumerateNestedClasses(f, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        curNames.pop_back();
    };

    cbClass = [&](AbckitCoreClass *c) {
        auto className = helpers::AbckitStringToString(g_implI->classGetName(c));
        curNames.emplace_back(className);
        g_implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
        curNames.pop_back();
    };

    EnumerateAllMethodsInModule(file, cbClass, cbFunc, curNames, curModuleName);
    ASSERT_EQ(funcsCounter, std::accumulate(expectedNames.begin(), expectedNames.end(), 0,
                                            [](size_t x, auto y) { return x + y.second.size(); }));

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void EnumerateAnonymousFunctions(const char *abcFilePath, TestStructType &testStruct)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(abcFilePath, &file);

    g_implI->fileEnumerateModules(file, &testStruct, [](AbckitCoreModule *m, void *data) {
        auto testStruct = reinterpret_cast<TestStructType *>(data);

        auto strModule = g_implI->moduleGetName(m);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto nameModule = helpers::AbckitStringToString(strModule);
        if (nameModule != testStruct->nameModuleToSearch) {
            return true;
        }
        g_implI->moduleEnumerateAnonymousFunctions(m, data, [](AbckitCoreFunction *func, void *data1) {
            auto testStruct = reinterpret_cast<TestStructType *>(data1);

            auto strFunc = g_implI->functionGetName(func);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto nameFunc = helpers::AbckitStringToString(strFunc);

            auto &funcs = testStruct->namesOfAnonFuncs;

            auto iter = std::find_if(funcs.begin(), funcs.end(),
                                     [&nameFunc](const std::string &name) { return nameFunc == name; });

            EXPECT_TRUE(iter != funcs.end());
            testStruct->anonFuncsCounter++;
            return true;
        });
        return true;
    });

    ASSERT_EQ(testStruct.anonFuncsCounter, testStruct.namesOfAnonFuncs.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnonymousFunctions, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSClassEnumerateAnonymousFunctions)
{
    TestStructType testStruct;
    testStruct.nameModuleToSearch = "JSenumerators0_dynamic";
    testStruct.namesOfAnonFuncs =
        std::vector<std::string> {"JSenumerators0_dynamic.#*@0*#", "JSenumerators0_dynamic.#*@0*#^1"};

    EnumerateAnonymousFunctions(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                                testStruct);
}

// Test: test-kind=api, api=InspectApiImpl::fileEnumerateModules, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSFileEnumerateModules)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateNamespaces, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSModuleEnumerateNamespaces)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSModuleEnumerateClasses)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateTopLevelFunctions, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSModuleEnumerateTopLevelFunctions)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetName, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSNamespaceGetName)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateNamespaces, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSNamespaceEnumerateNamespaces)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateClasses, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSNamespaceEnumerateClasses)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateTopLevelFunctions, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSNamespaceEnumerateTopLevelFunctions)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiEnumeratorsTest, JSClassEnumerateMethods)
{
    EnumerateAllMethods(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/enumerators/JSenumerators0_dynamic.abc",
                        g_jsnames);
}

}  // namespace libabckit::test
