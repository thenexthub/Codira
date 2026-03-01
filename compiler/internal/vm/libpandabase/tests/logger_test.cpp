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

#include "os/file.h"
#include "os/thread.h"
#include "utils/logger.h"
#include "utils/string_helpers.h"

#include <cstdio>

#include <fstream>
#include <regex>
#include <streambuf>

#include <gtest/gtest.h>

namespace panda::test {

class LoggerTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
#ifndef HOST_UT
        system("mount -o rw,remount /");
#endif
    }
};

HWTEST_F(LoggerTest, Initialization, testing::ext::TestSize.Level0)
{
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::FLAGS_gtest_death_test_style = "fast";
    testing::internal::CaptureStderr();

    LOG(DEBUG, COMMON) << "1";
    LOG(INFO, COMMON) << "2";
    LOG(ERROR, COMMON) << "3";

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, "");

    EXPECT_DEATH_IF_SUPPORTED(LOG(FATAL, COMMON) << "4", "");

    Logger::InitializeStdLogging(Logger::Level::DEBUG, panda::LoggerComponentMaskAll);
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

HWTEST_F(LoggerTest, LoggingExceptionsFatal, testing::ext::TestSize.Level0)
{
    testing::FLAGS_gtest_death_test_style = "fast";

    panda::Logger::ComponentMask component_mask;
    component_mask.set(Logger::Component::COMPILER);

    Logger::InitializeStdLogging(Logger::Level::FATAL, component_mask);
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

HWTEST_F(LoggerTest, LoggingExceptionsError, testing::ext::TestSize.Level0)
{
    testing::FLAGS_gtest_death_test_style = "fast";

    panda::Logger::ComponentMask component_mask;
    component_mask.set(Logger::Component::COMPILER);

    Logger::InitializeStdLogging(Logger::Level::ERROR, component_mask);
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

HWTEST_F(LoggerTest, FilterInfo, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, panda::LoggerComponentMaskAll);
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

HWTEST_F(LoggerTest, FilterError, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::ERROR, panda::LoggerComponentMaskAll);
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

HWTEST_F(LoggerTest, FilterFatal, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::FATAL, panda::LoggerComponentMaskAll);
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

HWTEST_F(LoggerTest, ComponentFilter, testing::ext::TestSize.Level0)
{
    panda::Logger::ComponentMask component_mask;
    component_mask.set(Logger::Component::COMPILER);
    component_mask.set(Logger::Component::GC);

    Logger::InitializeStdLogging(Logger::Level::INFO, component_mask);
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

HWTEST_F(LoggerTest, FileLogging, testing::ext::TestSize.Level0)
{
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string log_filename = helpers::string::Format("/tmp/gtest_panda_logger_file_%06x", tid);

    auto file = os::file::Open(log_filename, os::file::Mode::READWRITECREATE);
    ASSERT_TRUE(file.IsValid());

    Logger::InitializeFileLogging(log_filename, Logger::Level::INFO,
                                  panda::Logger::ComponentMask().set(Logger::Component::COMMON));
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
        std::ifstream log_file_stream(log_filename);
        std::string log_file_content((std::istreambuf_iterator<char>(log_file_stream)),
                                     std::istreambuf_iterator<char>());
        EXPECT_TRUE(std::regex_match(log_file_content, e));
    }
#endif  // GTEST_HAS_DEATH_TEST

    EXPECT_EQ(std::remove(log_filename.c_str()), 0);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
}

HWTEST_F(LoggerTest, Multiline, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, panda::Logger::ComponentMask().set(Logger::Component::COMMON));
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

HWTEST_F(LoggerTest, LogIf, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, panda::LoggerComponentMaskAll);
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

HWTEST_F(LoggerTest, LogOnce, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, panda::LoggerComponentMaskAll);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    LOG_ONCE(INFO, COMMON) << "a";
    for (int i = 0; i < 10; ++i) {
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

}  // namespace panda::test
