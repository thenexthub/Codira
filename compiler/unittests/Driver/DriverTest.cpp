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

#include "gtest/gtest.h"
#include "Codira/Driver/Driver.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Codira/Driver/Utils.h"

using namespace Codira;

class DriverTest : public ::testing::Test {
public:
    std::unique_ptr<Driver> driver;

protected:
    void SetUp() override
    {
#ifdef PROJECT_SOURCE_DIR
        // Gets the absolute path of the project from the compile parameter.
        std::string projectPath = PROJECT_SOURCE_DIR;
#else
        // Just in case, give it a default value.
        // Assume the initial is in the build directory.
        std::string projectPath = "..";
#endif
#ifdef _WIN32
        sdkPath = FileUtil::JoinPath(projectPath, "unittests\\Driver");
#else
        sdkPath = FileUtil::JoinPath(projectPath, "unittests/Driver");
#endif
    }
    std::string sdkPath;
};

TEST_F(DriverTest, GetSingleQuotedTest)
{
    const std::string singleQuote{"\\'"};
    EXPECT_EQ(GetSingleQuoted("abcde"), "'abcde'");
    EXPECT_EQ(GetSingleQuoted("'; ls"), "''" + singleQuote + "'; ls'");
    EXPECT_EQ(GetSingleQuoted("'wrapped'"), std::string{"''" + singleQuote + "'wrapped'" + singleQuote + "''"});
    EXPECT_EQ(GetSingleQuoted("start'wrapped'end"),
        std::string{"'start'" + singleQuote + "'wrapped'" + singleQuote + "'end'"});
    EXPECT_EQ(GetSingleQuoted("end'"), std::string{"'end'" + singleQuote + "''"});
}

TEST_F(DriverTest, GetDarwinSDKVersion)
{
    std::optional<std::string> sdkVersion = GetDarwinSDKVersion(sdkPath);
    EXPECT_FALSE(sdkVersion.has_value());

    sdkVersion = GetDarwinSDKVersion(sdkPath + "/IncorrectSDKSettings1");
    EXPECT_FALSE(sdkVersion.has_value());

    sdkVersion = GetDarwinSDKVersion(sdkPath + "/IncorrectSDKSettings2");
    EXPECT_FALSE(sdkVersion.has_value());

    sdkVersion = GetDarwinSDKVersion(sdkPath + "/IncorrectSDKSettings3");
    EXPECT_FALSE(sdkVersion.has_value());

    sdkVersion = GetDarwinSDKVersion(sdkPath + "/CorrectSDKSettings");
    EXPECT_TRUE(sdkVersion.has_value());
    EXPECT_EQ(sdkVersion.value(), "14.5");
}
