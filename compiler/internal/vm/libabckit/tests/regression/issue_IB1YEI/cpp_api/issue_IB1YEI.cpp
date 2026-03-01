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

#include "libabckit/cpp/abckit_cpp.h"

namespace {
class ErrorHandler final : public abckit::IErrorHandler {
    void HandleError(abckit::Exception &&e) override
    {
        EXPECT_TRUE(false) << "Abckit exception raised: " << e.what();
    }
};
}  // namespace

namespace libabckit::test {

class AbckitRegressionTestCppAPI : public ::testing::Test {};

// Test: test-kind=regression, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitRegressionTestCppAPI, LibAbcKitTestIssueIB1YEI)
{
    const std::string inputPath = ABCKIT_ABC_DIR "regression/issue_IB1YEI/issue_IB1YEI.abc";
    abckit::File file(inputPath, std::make_unique<ErrorHandler>());
    const std::string name = "new_module";

    abckit::arkts::Module arktsModule = file.AddExternalModuleArktsV1(name.c_str());
    abckit::core::Module coreModule = static_cast<abckit::core::Module>(arktsModule);
    EXPECT_TRUE(coreModule);
}

}  // namespace libabckit::test
