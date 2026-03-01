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
#include "libabckit/src/logger.h"

namespace libabckit::cpp_test {

class LibAbcKitLoggerTest : public ::testing::Test {};

TEST_F(LibAbcKitLoggerTest, CheckPermission_Case1)
{
    EXPECT_FALSE(Logger::CheckPermission("INVALID_LEVEL"));
    EXPECT_FALSE(Logger::CheckPermission(""));
    EXPECT_FALSE(Logger::CheckPermission("debug"));
}

TEST_F(LibAbcKitLoggerTest, CheckPermission_Case2)
{
    Logger::Initialize(Logger::MODE::DEBUG_MODE);
    EXPECT_FALSE(Logger::CheckPermission("INCORRECT_LOG_LVL"));
}

TEST_F(LibAbcKitLoggerTest, GetLoggerStream_Case)
{
    std::ostream *stream = libabckit::Logger::GetLoggerStream("INVALID_LEVEL");
    EXPECT_EQ(stream, &libabckit::g_nullStream);
}
}  // namespace libabckit::cpp_test