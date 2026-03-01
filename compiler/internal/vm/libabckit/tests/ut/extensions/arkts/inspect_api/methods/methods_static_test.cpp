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

#include <set>
#include <string>
#include <gtest/gtest.h>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitArkTSInspectApiMethods : public ::testing::Test {};

// Test: test-kind=api, api=ArktsInspectApiImpl::functionIsAbstract, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiMethods, ArktsFunctionIsAbstract)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/methods/methods_static.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c2F6:methods_static.C2;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C2");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (abckit::arkts::Function(method).IsAbstract()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::functionIsFinal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiMethods, ArktsFunctionIsFinal)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/methods/methods_static.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"c1F5:methods_static.C1;void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &method : klass.GetAllMethods()) {
            if (abckit::arkts::Function(method).IsFinal()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::functionIsNative, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiMethods, ArktsFunctionIsNative)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/methods/methods_static.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"m0F7:i32;", "m0F8:std.core.Promise;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &method : module.GetTopLevelFunctions()) {
            if (abckit::arkts::Function(method).IsNative()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::functionIsAsync, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiMethods, ArktsFunctionIsAsync)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/methods/methods_static.abc");

    std::set<std::string> gotMethodNames;
    std::set<std::string> expectMethodNames = {"m0F8:std.core.Promise;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &method : module.GetTopLevelFunctions()) {
            if (abckit::arkts::Function(method).IsAsync()) {
                gotMethodNames.emplace(method.GetName());
            }
        }
    }

    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

}  // namespace libabckit::test
