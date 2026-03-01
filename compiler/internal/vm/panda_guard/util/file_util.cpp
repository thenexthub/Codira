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

#include "file_util.h"

#include <fstream>
#include <sstream>

#include "os/filesystem.h"
#include "utils/logger.h"

#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[FileUtil]";
}

std::string panda::guard::FileUtil::GetFileContent(const std::string &filePath)
{
    std::string content;
    std::string realPath = os::GetAbsolutePath(filePath);
    if (realPath.empty()) {
        LOG(INFO, PANDAGUARD) << TAG << "get absolute path failed, path:" << filePath;
        return content;
    }

    std::ifstream inputStream(realPath);
    if (!inputStream.is_open()) {
        LOG(INFO, PANDAGUARD) << TAG << "open real file path failed, path:" << realPath;
        return content;
    }

    std::stringstream ss;
    ss << inputStream.rdbuf();
    content = ss.str();
    inputStream.close();
    return content;
}

std::vector<std::string> panda::guard::FileUtil::GetLineDataFromFile(const std::string &filePath)
{
    std::vector<std::string> lineDataList;
    std::string realPath = os::GetAbsolutePath(filePath);
    if (realPath.empty()) {
        LOG(INFO, PANDAGUARD) << TAG << "get absolute path failed, path:" << filePath;
        return lineDataList;
    }

    std::ifstream inputStream(realPath);
    if (!inputStream.is_open()) {
        LOG(INFO, PANDAGUARD) << TAG << "open real file path failed, path:" << realPath;
        return lineDataList;
    }

    std::string line;
    while (std::getline(inputStream, line)) {
        lineDataList.emplace_back(line);
    }
    inputStream.close();
    return lineDataList;
}

void panda::guard::FileUtil::WriteFile(const std::string &filePath, const std::string &content)
{
    std::ofstream ofs;
    ofs.open(filePath, std::ios::trunc | std::ios::out);
    PANDA_GUARD_ASSERT_PRINT(!ofs.is_open(), TAG, ErrorCode::GENERIC_ERROR,
                             "can not open file, invalid filePath:" << filePath);
    ofs << content;
    ofs.close();
}
