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

#include <set>
#include <string>
#include <gtest/gtest.h>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiMethodsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::functionEnumerateParameters, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionGetParametersStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotParamTypeNames;
    std::set<std::string> expectParamTypeNames = {"std.core.String", "std.core.Array"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetFunctionByName(module, "m0F1:std.core.String;std.core.Array;void;");
        ASSERT_NE(result, std::nullopt);
        const auto &func = result.value();
        for (const auto &param : func.GetParameters()) {
            gotParamTypeNames.emplace(param.GetType().GetName());
        }
    }

    ASSERT_EQ(gotParamTypeNames, expectParamTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetReturnType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionGetReturnTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotReturnTypeNames;
    std::set<std::string> expectReturnTypeNames = {"void"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &func : module.GetTopLevelFunctions()) {
            if (func.GetName() == "m0F1:std.core.String;std.core.Array;void;") {
                gotReturnTypeNames.emplace(func.GetReturnType().GetName());
            }
        }
    }

    ASSERT_EQ(gotReturnTypeNames, expectReturnTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsPublic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsPublicStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c1F1:methods_static_other.C1;void;",
                                               "_ctor_:methods_static_other.C1;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsPublic()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsProtected, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsProtectedStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c1F2:methods_static_other.C1;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsProtected()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsPrivate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsPrivateStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c1F3:methods_static_other.C1;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsPrivate()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsInternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsInternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c1F4:methods_static_other.C1;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsInternal()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_NE(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsCtor, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsCtorStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"_ctor_:methods_static_other.C2;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C2");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsCtor()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsCctor, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsCctorStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"_cctor_:void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C2");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (method.IsCctor()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionIsExternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"_cctor_:void;", "m0F1:std.core.String;std.core.Array;void;",
                                               "main:void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &method : module.GetTopLevelFunctions()) {
            if (!method.IsExternal()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}
}  // namespace libabckit::test
