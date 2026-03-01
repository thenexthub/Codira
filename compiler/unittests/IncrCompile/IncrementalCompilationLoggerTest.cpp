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

#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "Codira/IncrementalCompilation/IncrementalCompilationLogger.h"

using namespace Codira;

class IncrementalCompilationLoggerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
    }
};

TEST_F(IncrementalCompilationLoggerTest, InvidPath)
{
    auto& logger = IncrementalCompilationLogger::GetInstance();
    logger.InitLogFile("");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile(".cache");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile(".codeo");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile("xxx");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile("xxx");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile(".log");
    EXPECT_EQ(logger.IsEnable(), false);
    Codira::FileUtil::CreateDirs("log/");
    logger.InitLogFile("log");
    EXPECT_EQ(logger.IsEnable(), false);
    logger.InitLogFile("log/");
    EXPECT_EQ(logger.IsEnable(), false);
    std::string nomalIncrLogPath = ".cached/2343242355.log";
    if (Codira::FileUtil::FileExist(nomalIncrLogPath)) {
        logger.InitLogFile(nomalIncrLogPath);
        EXPECT_EQ(logger.IsEnable(), true);
    } else {
        logger.InitLogFile(nomalIncrLogPath);
        EXPECT_EQ(logger.IsEnable(), false);
        Codira::FileUtil::CreateDirs(".cached/");
        logger.InitLogFile(nomalIncrLogPath);
        EXPECT_EQ(logger.IsEnable(), true);
    }
}
