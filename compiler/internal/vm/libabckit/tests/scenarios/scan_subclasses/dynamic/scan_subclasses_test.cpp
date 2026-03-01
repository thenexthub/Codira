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

#include <sstream>

#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "subclasses_scanner.h"

namespace libabckit::test {

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestScanSubclasses)
{
    // CC-OFFNXT(G.NAM.03) project code style
    constexpr auto VERSION = ABCKIT_VERSION_RELEASE_1_0_0;

    auto *impl = AbckitGetApiImpl(VERSION);
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "scenarios/scan_subclasses/dynamic/scan_subclasses.abc";
    AbckitFile *file = impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_NE(file, nullptr);

    const std::vector<ClassInfo> baseClassInfo = {{"modules/base", "Base"}};

    SubclassesScanner scanner(VERSION, impl, file, baseClassInfo);

    scanner.Run();

    auto subclasses = scanner.GetSubclasses();

    ASSERT_FALSE(subclasses.empty());

    const std::vector<ClassInfo> expectedSubClasses = {{"scan_subclasses", "Child1"}, {"scan_subclasses", "Child2"}};

    ASSERT_TRUE(scanner.IsEqualsSubClasses(expectedSubClasses));
    impl->closeFile(file);
}

}  // namespace libabckit::test
