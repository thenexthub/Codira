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
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/visit_helper/visit_helper-inl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArktsI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiMethodsTest : public ::testing::Test {};

// ========================================
// Helpers
// ========================================

static std::string_view MethodGetModuleTest(AbckitFile *file, const char *methodName)
{
    auto *method = helpers::FindMethodByName(file, methodName);
    auto *sourceRaw = g_implI->moduleGetName(g_implI->functionGetModule(method));
    return helpers::AbckitStringToString(sourceRaw);
}

// ========================================
// Methods Tests
// ========================================

// Test: test-kind=api, api=InspectApiImpl::functionGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticMethodGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    std::set<std::string> names = {
        "M0N0C0F1:void;",
        "M0N0C0F0:methods_static.M0N0C0;void;",
        "M0N0C0F2:methods_static.M0N0C0;std.core.Promise;",
        "_ctor_:methods_static.M0N0C0;void;",
        "M0C0F1:void;",
        "M0C0F0:methods_static.M0C0;void;",
        "M0C0F2:methods_static.M0C0;std.core.Promise;",
        "_ctor_:methods_static.M0C0;void;",
        "$_invoke:methods_static.%%lambda-lambda_invoke-1;void;",
        "_ctor_:methods_static.%%lambda-lambda_invoke-1;void;",
        "invoke0:methods_static.%%lambda-lambda_invoke-1;std.core.Object;",
        "$_invoke:methods_static.%%lambda-lambda_invoke-0;void;",
        "_ctor_:methods_static.%%lambda-lambda_invoke-0;void;",
        "invoke0:methods_static.%%lambda-lambda_invoke-0;std.core.Object;",
        "_cctor_:void;",
        "lambda_invoke-0:void;",
        "lambda_invoke-1:void;",
        "m0F0:void;",
        "m0F2:std.core.Promise;",
        "m0N0F0:void;",
        "m0N0F2:std.core.Promise;",
        "m0N0N0F0:void;",
        "main:void;",
    };

    helpers::EnumerateAllMethods(file, [&](AbckitCoreFunction *m) {
        auto *abckitname = g_implI->functionGetName(m);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(abckitname, nullptr);
        auto name = helpers::AbckitStringToString(abckitname);
        EXPECT_EQ(names.count(name.data()), 1);
        names.erase(name.data());
    });
    ASSERT_TRUE(names.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::set<std::string> names = {
        "M0C0",
        "M0C0F1",
        "M0C0F0",
        "M0C0F2",
        "M0N0C0",
        "M0N0C0F1",
        "M0N0C0F0",
        "M0N0C0F2",
        "m0N0N0F0",
        "m0N0F0",
        "m0N0F1",
        "m0N0F2",
        "m0F0",
        "methods_dynamic.#*@0*#m0F0F0",
        "m0F1",
        "m0F2",
        "methods_dynamic.#*@0*#",
        "methods_dynamic.#*@0*#^1",
        "func_main_0",
    };

    helpers::EnumerateAllMethods(file, [&](AbckitCoreFunction *m) {
        auto *abckitname = g_implI->functionGetName(m);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(abckitname, nullptr);
        auto name = helpers::AbckitStringToString(abckitname);
        EXPECT_EQ(names.count(name.data()), 1);
        names.erase(name.data());
    });
    ASSERT_TRUE(names.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxClassFinder = {nullptr, "M0C0"};
        g_implI->moduleEnumerateClasses(m, &ctxClassFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxClassFinder.klass, nullptr);

        helpers::MethodByNameContext ctxMethodFinder = {nullptr, "M0C0F0"};
        g_implI->classEnumerateMethods(ctxClassFinder.klass, &ctxMethodFinder, helpers::MethodByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxMethodFinder.method, nullptr);

        auto *ctxClass = g_implI->classGetFile(ctxClassFinder.klass);
        auto *ctxMethod = g_implI->functionGetFile(ctxMethodFinder.method);
        EXPECT_EQ(ctxClass, ctxMethod);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) -> bool {
        helpers::ClassByNameContext ctxClassFinder = {nullptr, "M0C0"};
        g_implI->moduleEnumerateClasses(m, &ctxClassFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxClassFinder.klass, nullptr);

        helpers::MethodByNameContext ctxMethodFinder = {nullptr, "M0C0F0"};
        g_implI->classEnumerateMethods(ctxClassFinder.klass, &ctxMethodFinder, helpers::MethodByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxMethodFinder.method, nullptr);

        auto *ctxClass = g_implI->classGetFile(ctxClassFinder.klass);
        auto *ctxMethod = g_implI->functionGetFile(ctxMethodFinder.method);
        EXPECT_EQ(ctxClass, ctxMethod);

        return true;
    });

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsCtor, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionIsCtor)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::set<std::string> ctorMethods = {
        "M0C0",
        "M0N0C0",
    };

    helpers::EnumerateAllMethods(file, [&ctorMethods](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        bool isCtor = g_implI->functionIsCtor(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (isCtor) {
            EXPECT_EQ(ctorMethods.count(methodName.data()), 1);
            ctorMethods.erase(methodName.data());
        } else {
            EXPECT_EQ(ctorMethods.count(methodName.data()), 0);
        }
    });
    ASSERT_TRUE(ctorMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsCtor, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticFunctionIsCtor)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    std::set<std::string> ctorMethods = {
        "_ctor_:methods_static.M0C0;void;",
        "_ctor_:methods_static.M0N0C0;void;",
        "_ctor_:methods_static.%%lambda-lambda_invoke-1;void;",
        "_ctor_:methods_static.%%lambda-lambda_invoke-0;void;",
    };

    helpers::EnumerateAllMethods(file, [&ctorMethods](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));

        bool isCtor = g_implI->functionIsCtor(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (isCtor) {
            EXPECT_EQ(ctorMethods.count(methodName.data()), 1);
            ctorMethods.erase(methodName.data());
        } else {
            EXPECT_EQ(ctorMethods.count(methodName.data()), 0);
        }
    });
    ASSERT_TRUE(ctorMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsStatic, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionIsStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::set<std::string> staticMethods = {
        "M0C0F1",
        "M0N0C0F1",
        "m0N0N0F0",
        "m0N0F0",
        "m0N0F1",
        "m0N0F2",
        "m0F0",
        "methods_dynamic.#*@0*#m0F0F0",
        "m0F1",
        "m0F2",
        "methods_dynamic.#*@0*#",
        "methods_dynamic.#*@0*#^1",
        "func_main_0",
    };

    helpers::EnumerateAllMethods(file, [&staticMethods](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        bool isStatic = g_implI->functionIsStatic(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (isStatic) {
            EXPECT_EQ(staticMethods.count(methodName.data()), 1);
            staticMethods.erase(methodName.data());
        } else {
            EXPECT_EQ(staticMethods.count(methodName.data()), 0);
        }
    });
    ASSERT_TRUE(staticMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticFunctionIsStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    std::set<std::string> staticMethods = {
        "main:void;",
        "M0N0C0F1:void;",
        "M0C0F1:void;",
        "m0F0:void;",
        "m0F2:std.core.Promise;",
        "m0N0F0:void;",
        "m0N0F2:std.core.Promise;",
        "m0N0N0F0:void;",
        "_cctor_:void;",
        "lambda_invoke-0:void;",
        "lambda_invoke-1:void;",
    };

    helpers::EnumerateAllMethods(file, [&staticMethods](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        bool isStatic = g_implI->functionIsStatic(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (isStatic) {
            EXPECT_EQ(staticMethods.count(methodName.data()), 1);
            staticMethods.erase(methodName.data());
        } else {
            EXPECT_EQ(staticMethods.count(methodName.data()), 0);
        }
    });
    ASSERT_TRUE(staticMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsAnonymous, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionIsAnonymous)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::set<std::string> anonymousMethods = {
        "methods_dynamic.#*@0*#",
        "methods_dynamic.#*@0*#^1",
        "methods_dynamic.#*@0*#m0F0F0",
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

// Test: test-kind=api, api=InspectApiImpl::functionIsAnonymous, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticFunctionIsAnonymous)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    std::set<std::string> anonymousMethods = {
        "lambda_invoke-0:void;",
        "lambda_invoke-1:void;",
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
        anonymousMethods.erase(methodName.data());
    });
    ASSERT_TRUE(anonymousMethods.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentFunction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionGetParentFunction)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::unordered_map<std::string, std::string> parentFunctions = {
        {"M0C0F1", ""},
        {"M0C0", ""},
        {"M0C0F0", ""},
        {"M0C0F2", ""},
        {"M0N0C0F1", ""},
        {"M0N0C0", ""},
        {"M0N0C0F0", ""},
        {"M0N0C0F2", ""},
        {"m0N0N0F0", ""},
        {"m0N0F0", ""},
        {"m0N0F1", ""},
        {"m0N0F2", ""},
        {"m0F0", ""},
        {"methods_dynamic.#*@0*#m0F0F0", ""},
        {"m0F1", ""},
        {"m0F2", ""},
        {"methods_dynamic.#*@0*#", ""},
        {"methods_dynamic.#*@0*#^1", ""},
        {"func_main_0", ""},
    };

    helpers::EnumerateAllMethods(file, [&parentFunctions](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        ASSERT_EQ(parentFunctions.count(methodName.data()), 1);
        AbckitCoreFunction *parentFunction = g_implI->functionGetParentFunction(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (parentFunction == nullptr) {
            ASSERT_EQ(parentFunctions.at(methodName.data()), "");
        } else {
            auto className = helpers::AbckitStringToString(g_implI->functionGetName(parentFunction));
            ASSERT_EQ(parentFunctions.at(methodName.data()), className);
        }
        parentFunctions.erase(methodName.data());
    });
    ASSERT_TRUE(parentFunctions.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentClass, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionGetParentClass)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);

    std::unordered_map<std::string, std::string> methodClasses = {
        {"M0C0F1", "M0C0"},
        {"M0C0", "M0C0"},
        {"M0C0F0", "M0C0"},
        {"M0C0F2", "M0C0"},
        {"M0N0C0F1", "M0N0C0"},
        {"M0N0C0", "M0N0C0"},
        {"M0N0C0F0", "M0N0C0"},
        {"M0N0C0F2", "M0N0C0"},
        {"m0N0N0F0", ""},
        {"m0N0F0", ""},
        {"m0N0F1", ""},
        {"m0N0F2", ""},
        {"m0F0", ""},
        {"m0F1", ""},
        {"m0F2", ""},
        {"methods_dynamic.#*@0*#m0F0F0", ""},
        {"methods_dynamic.#*@0*#", ""},
        {"methods_dynamic.#*@0*#^1", ""},
        {"func_main_0", ""},
    };

    helpers::EnumerateAllMethods(file, [&methodClasses](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        ASSERT_EQ(methodClasses.count(methodName.data()), 1);
        AbckitCoreClass *methodClass = g_implI->functionGetParentClass(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (methodClass == nullptr) {
            ASSERT_EQ(methodClasses.at(methodName.data()), "");
        } else {
            auto className = helpers::AbckitStringToString(g_implI->classGetName(methodClass));
            ASSERT_EQ(methodClasses.at(methodName.data()), className);
        }
        methodClasses.erase(methodName.data());
    });
    ASSERT_TRUE(methodClasses.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticFunctionGetParentClass)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);

    std::unordered_map<std::string, std::string> methodClasses = {
        {"_cctor_:void;", ""},
        {"lambda_invoke-0:void;", ""},
        {"lambda_invoke-1:void;", ""},
        {"m0F0:void;", ""},
        {"m0F2:std.core.Promise;", ""},
        {"m0N0F0:void;", ""},
        {"m0N0F2:std.core.Promise;", ""},
        {"m0N0N0F0:void;", ""},
        {"main:void;", ""},
        {"M0N0C0F1:void;", "M0N0C0"},
        {"M0N0C0F0:methods_static.M0N0C0;void;", "M0N0C0"},
        {"M0N0C0F2:methods_static.M0N0C0;std.core.Promise;", "M0N0C0"},
        {"_ctor_:methods_static.M0N0C0;void;", "M0N0C0"},
        {"M0C0F1:void;", "M0C0"},
        {"M0C0F0:methods_static.M0C0;void;", "M0C0"},
        {"M0C0F2:methods_static.M0C0;std.core.Promise;", "M0C0"},
        {"_ctor_:methods_static.M0C0;void;", "M0C0"},
        {"$_invoke:methods_static.%%lambda-lambda_invoke-1;void;", "%%lambda-lambda_invoke-1"},
        {"_ctor_:methods_static.%%lambda-lambda_invoke-0;void;", "%%lambda-lambda_invoke-0"},
        {"_ctor_:methods_static.%%lambda-lambda_invoke-1;void;", "%%lambda-lambda_invoke-1"},
        {"invoke0:methods_static.%%lambda-lambda_invoke-0;std.core.Object;", "%%lambda-lambda_invoke-0"},
        {"invoke0:methods_static.%%lambda-lambda_invoke-1;std.core.Object;", "%%lambda-lambda_invoke-1"},
        {"$_invoke:methods_static.%%lambda-lambda_invoke-0;void;", "%%lambda-lambda_invoke-0"},
    };

    helpers::EnumerateAllMethods(file, [&methodClasses](AbckitCoreFunction *method) {
        auto methodName = helpers::AbckitStringToString(g_implI->functionGetName(method));
        ASSERT_EQ(methodClasses.count(methodName.data()), 1);
        AbckitCoreClass *methodClass = g_implI->functionGetParentClass(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (methodClass == nullptr) {
            ASSERT_EQ(methodClasses.at(methodName.data()), "");
        } else {
            auto className = helpers::AbckitStringToString(g_implI->classGetName(methodClass));
            ASSERT_EQ(methodClasses.at(methodName.data()), className);
        }
        methodClasses.erase(methodName.data());
    });
    ASSERT_TRUE(methodClasses.empty());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetModule, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticFunctionGetModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static.abc", &file);
    auto moduleName = MethodGetModuleTest(file, "m0F0");
    ASSERT_EQ("methods_static", moduleName);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetModule, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, DynamicFunctionGetModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_dynamic.abc", &file);
    auto moduleName = MethodGetModuleTest(file, "m0F0");
    ASSERT_EQ("methods_dynamic", moduleName);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::functionIsNative, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, StaticMethodIsNative)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/native_method.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, [[maybe_unused]] void *data) {
        if (g_implI->moduleIsExternal(m)) {
            return false;
        }
        helpers::ClassByNameContext ctxFinder = {nullptr, "Y"};
        g_implI->moduleEnumerateClasses(m, &ctxFinder, helpers::ClassByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(ctxFinder.klass, nullptr);

        helpers::MethodByNameContext methodCtxFinder = {nullptr, "bar"};
        g_implI->classEnumerateMethods(ctxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(methodCtxFinder.method, nullptr);

        AbckitArktsFunction *methodArkTS = g_implArktsI->coreFunctionToArktsFunction(methodCtxFinder.method);
        bool isAbstract = g_implArktsI->functionIsNative(methodArkTS);
        EXPECT_EQ(isAbstract, true);

        return true;
    });

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
