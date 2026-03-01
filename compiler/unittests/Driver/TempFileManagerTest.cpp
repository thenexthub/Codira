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
#include "Codira/Driver/TempFileManager.h"
#include "Codira/Driver/TempFileInfo.h"
#include "Codira/Option/Option.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;

class TempFileManagerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
    }
};

TEST_F(TempFileManagerTest, WindowsOutputSuffixTest)
{
    GlobalOptions options;
    options.target.os = Triple::OSType::WINDOWS;
    TempFileManager::Instance().Init(options, false);
    TempFileInfo info;
    info.fileName = "test";
    auto newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_EXE);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "main.exe");
    newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_DYLIB);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "libtest.dll");
    newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_STATICLIB);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "libtest.a");
}

TEST_F(TempFileManagerTest, LinuxOutputSuffixTest)
{
    GlobalOptions options;
    options.target.os = Triple::OSType::LINUX;
    TempFileManager::Instance().Init(options, false);
    TempFileInfo info;
    info.fileName = "test";
    auto newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_EXE);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "main");
    newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_DYLIB);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "libtest.so");
    newInfo = TempFileManager::Instance().CreateNewFileInfo(info, TempFileKind::O_STATICLIB);
    ASSERT_EQ(FileUtil::GetFileName(newInfo.filePath), "libtest.a");
}
