/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libarkfile/debug_info_extractor.h"
#include "libarkfile/file.h"
#include "gtest/gtest.h"
#include "libarkbase/os/filesystem.h"

namespace ark::test {

class DebugInfoTest : public testing::Test {
public:
    void SetUp() override {}

    void TearDown() override {}

protected:
    std::string_view GetTestFile()
    {
        auto testFile = std::getenv("TEST_FILE");
        if (testFile == nullptr) {
            std::cerr << "Environment variable 'TEST_FILE' must be set" << std::endl;
            std::abort();
        }
        return testFile;
    }
};

static std::string_view GetFileName(std::string_view path)
{
    std::string_view fileName;
    auto pathDelimiterPos = path.rfind('/');
    fileName = pathDelimiterPos == std::string_view::npos ? path : path.substr(pathDelimiterPos + 1);
    auto extDelimiterPos = fileName.rfind('.');
    return fileName.substr(0, extDelimiterPos);
}

TEST_F(DebugInfoTest, GetSourceFile)
{
    auto file = panda_file::OpenPandaFile(GetTestFile());
    panda_file::DebugInfoExtractor extractor(file.get());
    auto methods = extractor.GetMethodIdList();
    ASSERT_FALSE(methods.empty());
    std::string_view sourceFilePath {extractor.GetSourceFile(methods[0])};
    ASSERT_EQ(GetFileName(sourceFilePath), GetFileName(GetTestFile()));
}

}  // namespace ark::test
