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

#include "common_components/log/log.h"
#include "common_components/log/log_base.h"
#include "common_components/tests/test_helper.h"

using namespace common;

// ==================== Test Fixture ====================
namespace common::test {
class LogTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ==================== Test Case ====================
HWTEST_F_L0(LogTest, ConvertFromRuntime_Info_ReturnsInfo) {
    Level result = Log::ConvertFromRuntime(LOG_LEVEL::INFO);
    EXPECT_EQ(result, Level::INFO);
}

HWTEST_F_L0(LogTest, ConvertFromRuntime_Info_ReturnsWarn) {
    Level result = Log::ConvertFromRuntime(LOG_LEVEL::WARN);
    EXPECT_EQ(result, Level::WARN);
}

HWTEST_F_L0(LogTest, ConvertFromRuntime_Debug_ReturnsDebug) {
    Level result = Log::ConvertFromRuntime(LOG_LEVEL::DEBUG);
    EXPECT_EQ(result, Level::DEBUG);
}

HWTEST_F_L0(LogTest, ConvertFromRuntime_Debug_ReturnsError) {
    Level result = Log::ConvertFromRuntime(LOG_LEVEL::ERROR);
    EXPECT_EQ(result, Level::ERROR);
}

HWTEST_F_L0(LogTest, ConvertFromRuntime_Fatal_ReturnsFatal) {
    Level result = Log::ConvertFromRuntime(LOG_LEVEL::FATAL);
    EXPECT_EQ(result, Level::FATAL);
}


HWTEST_F_L0(LogTest, ConvertFromRuntime_Default_ReturnsDebug) {
    Level result = Log::ConvertFromRuntime(static_cast<LOG_LEVEL>(999));
    EXPECT_EQ(result, Level::DEBUG);
}

HWTEST_F_L0(LogTest, PrettyOrderMathNano) {
    std::string result = PrettyOrderMathNano(1000000000000, "s");
    EXPECT_EQ(result, "1000s");
}
}

namespace common {
class TestLogRedirect {
public:
    TestLogRedirect()
    {
#ifndef ENABLE_HILOG
        originalCoutBuffer = std::cout.rdbuf();
        originalCerrBuffer = std::cerr.rdbuf();

        std::cout.rdbuf(buffer.rdbuf());
        std::cerr.rdbuf(buffer.rdbuf());
#endif
    }

    ~TestLogRedirect()
    {
#ifndef ENABLE_HILOG
        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);
#endif
    }

    std::string GetOutput() const
    {
        return buffer.str();
    }

    void ClearOutput()
    {
        buffer.str(std::string());
    }

private:
    std::stringstream buffer;
#ifndef ENABLE_HILOG
    std::streambuf* originalCoutBuffer;
    std::streambuf* originalCerrBuffer;
#endif
};
}  // namespace common

namespace common::test {
class TimerTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override
    {
        redirect.ClearOutput();

        LogOptions options;
        options.level = Level::DEBUG;
        options.component = static_cast<ComponentMark>(Component::ALL);

        Log::Initialize(options);
    }
    void TearDown() override {}

    TestLogRedirect redirect;
};

static constexpr uint32_t SECOND_TIME = 1000000;
HWTEST_F_L0(TimerTest, Timer_BasicUsage_LogsTime)
{
    {
        Timer t("TestScope");
        for (volatile int i = 0; i < SECOND_TIME; ++i);
    }

#ifndef ENABLE_HILOG
    std::string output = redirect.GetOutput();
    EXPECT_NE(output.find("TestScope time:"), std::string::npos);
    EXPECT_NE(output.find("us"), std::string::npos);
#endif
}

HWTEST_F_L0(TimerTest, Timer_LevelNotDebug_NoLogging)
{
    LogOptions options;
    options.level = Level::INFO;
    options.component = static_cast<ComponentMark>(Component::ALL);
    Log::Initialize(options);

    {
        Timer t("SilentScope");
        for (volatile int i = 0; i < SECOND_TIME; ++i);
    }

    std::string output = redirect.GetOutput();
    EXPECT_EQ(output.find("SilentScope"), std::string::npos);
}

HWTEST_F_L0(TimerTest, Timer_LongName_CorrectFormat)
{
    {
        Timer t("VeryLongTimerNameForTesting");
        for (volatile int i = 0; i < SECOND_TIME; ++i);
    }

#ifndef ENABLE_HILOG
    std::string output = redirect.GetOutput();
    EXPECT_NE(output.find("VeryLongTimerNameForTesting time:"), std::string::npos);
#endif
}

HWTEST_F_L0(TimerTest, Timer_MultipleInstances_DistinctOutput)
{
    {
        Timer t1("First");
        for (volatile int i = 0; i < SECOND_TIME; ++i);
    }

    {
        Timer t2("Second");
        for (volatile int i = 0; i < SECOND_TIME; ++i);
    }

#ifndef ENABLE_HILOG
    std::string output = redirect.GetOutput();
    EXPECT_NE(output.find("First time:"), std::string::npos);
    EXPECT_NE(output.find("Second time:"), std::string::npos);
#endif
}
}