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
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiNamespacesTests : public ::testing::Test {};
static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_dynamic.abc";

enum TestTypes {
    CLASS,
    FUNCTION,
    NAMESPACE,
};

static void TestNamespaces(const std::string &namespaceName, const std::string &expectedNamespaceName)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *n = helpers::FindNamespaceByName(file, namespaceName);
    ASSERT_NE(n, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *parent = g_implI->namespaceGetParentNamespace(n);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    if (expectedNamespaceName.empty()) {
        ASSERT_EQ(parent, nullptr);
    } else {
        ASSERT_NE(parent, nullptr);
        auto parentName = helpers::AbckitStringToString(g_implI->namespaceGetName(parent));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(parentName, expectedNamespaceName);
    }

    g_impl->closeFile(file);
}

static void TestFunctions(const std::string &functionName, const std::string &expectedNamespaceName)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *func = helpers::FindMethodByName(file, functionName);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *parent = g_implI->functionGetParentNamespace(func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    if (expectedNamespaceName.empty()) {
        ASSERT_EQ(parent, nullptr);
    } else {
        ASSERT_NE(parent, nullptr);
        auto parentName = helpers::AbckitStringToString(g_implI->namespaceGetName(parent));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(parentName, expectedNamespaceName);
    }

    g_impl->closeFile(file);
}

static void TestClasses(const std::string &className, const std::string &expectedNamespaceName)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *func = helpers::FindMethodByName(file, className);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *klass = g_implI->functionGetParentClass(func);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *parent = g_implI->classGetParentNamespace(klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    if (expectedNamespaceName.empty()) {
        ASSERT_EQ(parent, nullptr);
    } else {
        ASSERT_NE(parent, nullptr);
        auto parentName = helpers::AbckitStringToString(g_implI->namespaceGetName(parent));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(parentName, expectedNamespaceName);
    }

    g_impl->closeFile(file);
}

template <TestTypes TYPE>
static void GeneralTestNamespaces(const std::string &entityName, const std::string &expectedNamespaceName = "")
{
    if constexpr (TYPE == NAMESPACE) {
        TestNamespaces(entityName, expectedNamespaceName);
    } else if (TYPE == CLASS) {
        TestClasses(entityName, expectedNamespaceName);
    } else {
        TestFunctions(entityName, expectedNamespaceName);
    }
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, NamespaceGetParentNamespace_1)
{
    GeneralTestNamespaces<NAMESPACE>("N1");
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, NamespaceGetParentNamespace_2)
{
    GeneralTestNamespaces<NAMESPACE>("N2", "N1");
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, FunctionGetParentNamespace_1)
{
    GeneralTestNamespaces<FUNCTION>("f0");
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, FunctionGetParentNamespace_2)
{
    GeneralTestNamespaces<FUNCTION>("f1", "N1");
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, FunctionGetParentNamespace_3)
{
    GeneralTestNamespaces<FUNCTION>("f2", "N2");
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, FunctionGetParentNamespace_4)
{
    GeneralTestNamespaces<FUNCTION>("M1");
}

// Test: test-kind=api, api=InspectApiImpl::functionGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, FunctionGetParentNamespace_5)
{
    GeneralTestNamespaces<FUNCTION>("M2");
}

// Test: test-kind=api, api=InspectApiImpl::classGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, ClassGetParentNamespace_1)
{
    GeneralTestNamespaces<CLASS>("C2", "N2");
}

// Test: test-kind=api, api=InspectApiImpl::classGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, ClassGetParentNamespace_2)
{
    GeneralTestNamespaces<CLASS>("C1", "N1");
}

// Test: test-kind=api, api=InspectApiImpl::classGetParentNamespace, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTests, ClassGetParentNamespace_3)
{
    GeneralTestNamespaces<CLASS>("C0");
}

}  // namespace libabckit::test
