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

#include "api_scanner.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestApiScannerDynamic)
{
    const auto version = ABCKIT_VERSION_RELEASE_1_0_0;
    auto *impl = AbckitGetApiImpl(version);
    auto filePath = ABCKIT_ABC_DIR "scenarios/api_scanner/dynamic/api_scanner.abc";
    AbckitFile *file = impl->openAbc(filePath, strlen(filePath));
    std::vector<ApiInfo> apiList = {{"modules/apiNamespace", "ApiNamespace", "foo"},
                                    {"modules/toplevelApi", "bar", ""},
                                    {"@ohos.geolocation", "geolocation", "getCurrentLocation"}};
    ApiScanner a(version, file, apiList);

    const auto &usages = a.GetApiUsages();
    EXPECT_FALSE(usages.empty());

    auto usagesStr = a.ApiUsageMapGetString();
    std::vector<UsageInfo> expectedUsages = {
        {"api_scanner", "useAll", 0},
        {"api_scanner", "useFoo", 0},
        {"api_scanner", "Person.personUseAll", 0},
        {"api_scanner", "Person.personUseFoo", 0},
        {"modules/src2", "Animal.animalUseFoo", 0},
        {"modules/src2", "Animal.animalUseAll", 0},
        {"modules/src2", "src2UseAll", 0},
        {"modules/src2", "src2UseFoo", 0},
        {"api_scanner", "useAll", 1},
        {"api_scanner", "useBar", 1},
        {"api_scanner", "Person.personUseAll", 1},
        {"api_scanner", "Person.personUseBar", 1},
        {"api_scanner", "useAll", 2},
        {"api_scanner", "useLocation", 2},
        {"api_scanner", "Person.personUseLocation", 2},
        {"modules/src3", "src3UseLocation", 2},
        {"modules/src3", "Cat.catUseLocation", 2},
    };
    EXPECT_TRUE(a.IsUsagesEqual(expectedUsages));
    LIBABCKIT_LOG_TEST(DEBUG) << "====================================\n";
    LIBABCKIT_LOG_TEST(DEBUG) << usagesStr;

    impl->closeFile(file);
}

}  // namespace libabckit::test
