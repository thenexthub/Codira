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

#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/string_helpers.h"
#include "libarkbase/test_utilities.h"

#include <cstdio>

#include <fstream>
#include <regex>
#include <streambuf>

#include <gtest/gtest.h>

namespace ark::test {

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

DEATH_TEST(Logger, Initialization)
{
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "1";
    LOG(INFO, COMMON) << "2";
    LOG(ERROR, COMMON) << "3";

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMMON) << "4", "");

    Logger::InitializeStdLogging(Logger::Level::DEBUG, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "a";
    LOG(INFO, COMMON) << "b";
    LOG(ERROR, COMMON) << "c";

    err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
#ifndef NDEBUG
        "[TID %06x] D/common: a\n"
#endif
        "[TID %06x] I/common: b\n"
        "[TID %06x] E/common: c\n",
        tid, tid, tid);
    EXPECT_EQ(err, res);

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMMON) << "d", "\\[TID [0-9a-f]{6}\\] F/common: d");

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "1";
    LOG(INFO, COMMON) << "2";
    LOG(ERROR, COMMON) << "3";

    err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMMON) << "4", "");
}

DEATH_TEST(Logger, LoggingExceptionsFatal)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ark::Logger::ComponentMask componentMask;
    componentMask.set(Logger::Component::COMPILER);

    Logger::InitializeStdLogging(Logger::Level::FATAL, componentMask);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::COMPILER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ASSEMBLER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::DISASSEMBLER));
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::COMPILER));
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::ASSEMBLER));
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::DISASSEMBLER));

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMPILER) << "d1", "\\[TID [0-9a-f]{6}\\] F/compiler: d1");
    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, ASSEMBLER) << "d2", "\\[TID [0-9a-f]{6}\\] F/assembler: d2");
    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, DISASSEMBLER) << "d3", "\\[TID [0-9a-f]{6}\\] F/disassembler: d3");

    testing::internal::CaptureStderr();

    LOG(ERROR, COMPILER) << "c";
    LOG(ERROR, ASSEMBLER) << "a";
    LOG(ERROR, DISASSEMBLER) << "d";

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    Logger::Destroy();
}

DEATH_TEST(Logger, LoggingExceptionsError)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ark::Logger::ComponentMask componentMask;
    componentMask.set(Logger::Component::COMPILER);

    Logger::InitializeStdLogging(Logger::Level::ERROR, componentMask);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::COMPILER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ASSEMBLER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::DISASSEMBLER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::COMPILER));
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::ASSEMBLER));
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::ERROR, Logger::Component::DISASSEMBLER));

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMPILER) << "d1", "\\[TID [0-9a-f]{6}\\] F/compiler: d1");
    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, ASSEMBLER) << "d2", "\\[TID [0-9a-f]{6}\\] F/assembler: d2");
    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, DISASSEMBLER) << "d3", "\\[TID [0-9a-f]{6}\\] F/disassembler: d3");

    testing::internal::CaptureStderr();

    LOG(ERROR, COMPILER) << "c";
    LOG(ERROR, ASSEMBLER) << "a";
    LOG(ERROR, DISASSEMBLER) << "d";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format("[TID %06x] E/compiler: c\n", tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
}

TEST(Logger, FilterInfo)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "a";
    LOG(INFO, COMMON) << "b";
    LOG(ERROR, COMMON) << "c";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] I/common: b\n"
        "[TID %06x] E/common: c\n",
        tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, FilterError)
{
    Logger::InitializeStdLogging(Logger::Level::ERROR, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "a";
    LOG(INFO, COMMON) << "b";
    LOG(ERROR, COMMON) << "c";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format("[TID %06x] E/common: c\n", tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, FilterFatal)
{
    Logger::InitializeStdLogging(Logger::Level::FATAL, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "a";
    LOG(INFO, COMMON) << "b";
    LOG(ERROR, COMMON) << "c";

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, ComponentFilter)
{
    ark::Logger::ComponentMask componentMask;
    componentMask.set(Logger::Component::COMPILER);
    componentMask.set(Logger::Component::GC);

    Logger::InitializeStdLogging(Logger::Level::INFO, componentMask);
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::WARNING, Logger::Component::ALLOC));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::COMPILER));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::GC));

    testing::internal::CaptureStderr();

    LOG(INFO, COMMON) << "a";
    LOG(INFO, COMPILER) << "b";
    LOG(INFO, RUNTIME) << "c";
    LOG(INFO, GC) << "d";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] I/compiler: b\n"
        "[TID %06x] I/gc: d\n",
        tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

DEATH_TEST(Logger, FileLogging)
{
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string logFilename = helpers::string::Format("/tmp/gtest_panda_logger_file_%06x", tid);

    Logger::InitializeFileLogging(logFilename, Logger::Level::INFO,
                                  ark::Logger::ComponentMask().set(Logger::Component::COMMON));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::COMMON));

    LOG(DEBUG, COMMON) << "a";
    LOG(INFO, COMMON) << "b";
    LOG(ERROR, COMPILER) << "c";
    LOG(ERROR, COMMON) << "d";

#if GTEST_HAS_DEATH_TEST
    testing::FLAGS_gtest_death_test_style = "fast";

    EXPECT_DEATH(LOG(FATAL, COMMON) << "e", "");

    std::string res = helpers::string::Format(
        "\\[TID %06x\\] I/common: b\n"
        "\\[TID %06x\\] E/common: d\n"
        "\\[TID [0-9a-f]{6}\\] F/common: e\n",
        tid, tid);
    std::regex e(res);
    {
        std::ifstream logFileStream(logFilename);
        std::string logFileContent((std::istreambuf_iterator<char>(logFileStream)), std::istreambuf_iterator<char>());
        EXPECT_TRUE(std::regex_match(logFileContent, e));
    }
#endif  // GTEST_HAS_DEATH_TEST

    EXPECT_EQ(std::remove(logFilename.c_str()), 0U);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, Multiline)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::Logger::ComponentMask().set(Logger::Component::COMMON));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::COMMON));

    testing::internal::CaptureStderr();

    LOG(INFO, COMMON) << "a\nb\nc\n\nd\n";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] I/common: a\n"
        "[TID %06x] I/common: b\n"
        "[TID %06x] I/common: c\n"
        "[TID %06x] I/common: \n"
        "[TID %06x] I/common: d\n"
        "[TID %06x] I/common: \n",
        tid, tid, tid, tid, tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, PLog)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    int errnum = errno;

    errno = EEXIST;
    PLOG(ERROR, COMMON) << "a";
    errno = EACCES;
    PLOG(INFO, COMPILER) << "b";
    errno = errnum;

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] E/common: a: File exists\n"
        "[TID %06x] I/compiler: b: Permission denied\n",
        tid, tid, tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, LogIf)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG_IF(true, INFO, COMMON) << "a";
    LOG_IF(false, INFO, COMMON) << "b";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format("[TID %06x] I/common: a\n", tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, LogOnce)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG_ONCE(INFO, COMMON) << "a";
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 10U; ++i) {
        LOG_ONCE(INFO, COMMON) << "b";
    }
    LOG_ONCE(INFO, COMMON) << "c";

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] I/common: a\n"
        "[TID %06x] I/common: b\n"
        "[TID %06x] I/common: c\n",
        tid, tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, LogDfx)
{
    Logger::InitializeStdLogging(Logger::Level::ERROR, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    DfxController::Initialize();
    EXPECT_TRUE(DfxController::IsInitialized());
    EXPECT_EQ(DfxController::GetOptionValue(DfxOptionHandler::DFXLOG), 0U);

    testing::internal::CaptureStderr();

    LOG_DFX(COMMON) << "a";
    LOG_DFX(COMMON) << "b";
    LOG_DFX(COMMON) << "c";

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    DfxController::ResetOptionValueFromString("dfx-log:1");
    EXPECT_EQ(DfxController::GetOptionValue(DfxOptionHandler::DFXLOG), 1U);

    testing::internal::CaptureStderr();

    LOG_DFX(COMMON) << "a";
    LOG_DFX(COMMON) << "b";
    LOG_DFX(COMMON) << "c";

    err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format(
        "[TID %06x] E/dfx: common log:a\n"
        "[TID %06x] E/dfx: common log:b\n"
        "[TID %06x] E/dfx: common log:c\n",
        tid, tid, tid);
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    DfxController::Destroy();
    EXPECT_FALSE(DfxController::IsInitialized());
}

TEST(Logger, ProcessLogLevelFromString1)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    testing::internal::CaptureStderr();
    std::string s = "info";
    Logger::ProcessLogLevelFromString(s);

    std::string err = testing::internal::GetCapturedStderr();
    std::string res = helpers::string::Format("");
    EXPECT_EQ(err, res);
    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

TEST(Logger, ProcessLogLevelFromString2)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    testing::internal::CaptureStderr();

    std::string s = "hello";
    Logger::ProcessLogLevelFromString(s);
    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string res = helpers::string::Format("[TID %06x] E/runtime: Unknown level hello\n", tid);
    EXPECT_EQ(err, res);
    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

}  // namespace ark::test
