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

#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/helpers.h"
#include "metadata_inspect_impl.h"

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

namespace libabckit::test {

class AbckitRegressionTestCAPI : public ::testing::Test {};

// Test: test-kind=regression, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitRegressionTestCAPI, LibAbcKitTestIssueIB1YEI)
{
    std::string inputPath = ABCKIT_ABC_DIR "regression/issue_IB1YEI/issue_IB1YEI.abc";
    struct AbckitArktsV1ExternalModuleCreateParams params {
        "new_module"
    };

    AbckitFile *file = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto newModule = g_implArkM->fileAddExternalModuleArktsV1(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto arktsModule = g_implArkI->arktsModuleToCoreModule(newModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(arktsModule, nullptr);

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
