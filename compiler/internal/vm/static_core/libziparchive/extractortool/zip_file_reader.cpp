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

#include "zip_file_reader.h"

#include <fcntl.h>
#include <memory>
#include <sys/stat.h>

#include "libarkbase/utils/logger.h"
#include "zip_file_reader_io.h"

namespace ark::extractor {

std::shared_ptr<ZipFileReader> ZipFileReader::CreateZipFileReader(const std::string &filePath)
{
    size_t fileSize = GetFileLen(filePath);
    if (fileSize == 0) {
        LOG(ERROR, ZIPARCHIVE) << "fileSize error: " << filePath;
        return nullptr;
    }

    std::shared_ptr<ZipFileReader> result = std::make_shared<ZipFileReaderIo>(filePath);
    result->fileLen_ = fileSize;
    if (result->Init()) {
        return result;
    }
    return nullptr;
}

ZipFileReader::~ZipFileReader()
{
    if (fd_ >= 0 && closable_) {
        close(fd_);
        fd_ = -1;
    }
}

size_t ZipFileReader::GetFileLen(const std::string &filePath)
{
    if (filePath.empty()) {
        return 0;
    }

    struct stat fileStat {};
    if (stat(filePath.c_str(), &fileStat) == 0) {
        return fileStat.st_size;
    }

    LOG(ERROR, ZIPARCHIVE) << "error: " << errno << ", filePath: " << filePath;
    return 0;
}

bool ZipFileReader::Init()
{
    if (filePath_.empty()) {
        return false;
    }

    int flag = O_RDONLY;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    fd_ = open(filePath_.c_str(), flag);
    if (fd_ < 0) {
        LOG(ERROR, ZIPARCHIVE) << "open file error: " << filePath_ << ", errno: " << errno;
        return false;
    }

    return true;
}
}  // namespace ark::extractor