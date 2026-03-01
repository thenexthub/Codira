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

#if defined(PANDA_TARGET_UNIX)
#include "unix/libpandabase/file.h"
#elif defined(PANDA_TARGET_WINDOWS)
#include "windows/libpandabase/file.h"
#else
#error "Unsupported platform"
#endif

namespace panda::os::file {

#if defined(PANDA_TARGET_UNIX)
using File = panda::os::unix::file::File;
#elif defined(PANDA_TARGET_WINDOWS)
using File = panda::os::windows::file::File;
#endif

class FileTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(FileTest, get_extended_file_path, testing::ext::TestSize.Level0)
{
#if defined(PANDA_TARGET_UNIX)
    const std::string pathStr = "/test/test/test/test/test/test/test/test/test/test/test/test/test/test/test/test";
    std::string filePath = pathStr;
    std::string longFilePath = pathStr + pathStr + pathStr + pathStr + pathStr;

    std::string expectFilePath = pathStr;
    std::string expectLongFilePath = longFilePath;

    EXPECT_EQ(File::GetExtendedFilePath(filePath), expectFilePath);
    EXPECT_EQ(File::GetExtendedFilePath(longFilePath), expectLongFilePath);
#elif defined(PANDA_TARGET_WINDOWS)
    const std::string pathStr = "\\test\\test\\test\\test\\test\\test\\test\\test\\test\\test\\test\\test\\test";
    std::string filePath = "D:";
    filePath += pathStr;
    std::string longFilePath = "D:";
    longFilePath += pathStr + pathStr + pathStr + pathStr + pathStr;

    std::string expectFilePath = "D:";
    expectFilePath += pathStr;
    std::string expectLongFilePath = "\\\\?\\D:";
    expectLongFilePath += pathStr + pathStr + pathStr + pathStr + pathStr;

    EXPECT_EQ(File::GetExtendedFilePath(filePath), expectFilePath);
    EXPECT_EQ(File::GetExtendedFilePath(longFilePath), expectLongFilePath);
#endif
}
}  // namespace panda
