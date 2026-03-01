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
#include <set>
#include <optional>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiNamespacesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectedInterfaceNames = {"I1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &iface : ns.GetInterfaces()) {
                gotInterfaceNames.emplace(iface.GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectedInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateEnums, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetEnumsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotEnumNames;
    std::set<std::string> expectedEnumNames = {"E1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &enm : ns.GetEnums()) {
                gotEnumNames.emplace(enm.GetName());
            }
        }
    }

    ASSERT_EQ(gotEnumNames, expectedEnumNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectedFieldNames = {"f1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &field : ns.GetFields()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectedFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateTopLevelFunctions, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceEnumerateTopLevelFunctionsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotFunctionNames;
    std::set<std::string> expectedFunctionNames = {"m1:void;", "_cctor_:void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &function : ns.GetTopLevelFunctions()) {
                gotFunctionNames.emplace(function.GetName());
            }
        }
    }

    ASSERT_EQ(gotFunctionNames, expectedFunctionNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateNamespaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetNamespacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotNamespaceNames;
    std::set<std::string> expectedNamespaceNames = {"N2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &childNs : ns.GetNamespaces()) {
                gotNamespaceNames.emplace(childNs.GetName());
            }
        }
    }

    ASSERT_EQ(gotNamespaceNames, expectedNamespaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateAnnotationInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetAnnotationInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotAIs;
    std::set<std::string> expectedAIs = {"Anno"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &ai : ns.GetAnnotationInterfaces()) {
                gotAIs.emplace(ai.GetName());
            }
        }
    }

    ASSERT_EQ(gotAIs, expectedAIs);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceIsExternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotNamespaceNames;
    std::set<std::string> expectedNamespaceNames = {"N1"};

    for (const auto &module : file.GetModules()) {
        for (const auto &ns : module.GetNamespaces()) {
            if (!ns.IsExternal()) {
                gotNamespaceNames.emplace(ns.GetName());
            }
        }
    }

    ASSERT_EQ(gotNamespaceNames, expectedNamespaceNames);
}

}  // namespace libabckit::test