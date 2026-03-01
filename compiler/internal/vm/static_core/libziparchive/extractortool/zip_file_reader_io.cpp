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

#include "zip_file_reader_io.h"

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include "libarkbase/utils/logger.h"

namespace ark::extractor {
namespace {
constexpr size_t BIG_FILE_SIZE = 1U << 31U;
}  // namespace
std::string ZipFileReaderIo::ReadBuffer(size_t startPos, size_t bufferSize)
{
    std::string result;
    if (fd_ < 0 || startPos >= fileLen_ || bufferSize > fileLen_ - startPos) {
        LOG(ERROR, ZIPARCHIVE) << "failed: " << filePath_ << ", startPos: " << startPos
                               << ", bufferSize: " << bufferSize << "fileLen: " << fileLen_;
        return result;
    }

    result.resize(bufferSize);
    if (!ReadBuffer(reinterpret_cast<uint8_t *>(result.data()), startPos, bufferSize)) {
        result.clear();
    }

    return result;
}

bool ZipFileReaderIo::ReadBuffer(uint8_t *dst, size_t startPos, size_t bufferSize)
{
    if (dst == nullptr || fd_ < 0 || startPos >= fileLen_ || bufferSize > fileLen_ - startPos) {
        LOG(ERROR, ZIPARCHIVE) << "failed: " << filePath_ << ", startPos: " << startPos
                               << ", bufferSize: " << bufferSize << "fileLen: " << fileLen_;
        return false;
    }

    auto remainSize = bufferSize;
    ssize_t nread = 0;
    do {
#ifdef PANDA_TARGET_WINDOWS
        nread = _lseek(fd_, startPos, SEEK_SET);
        nread = _read(fd_, dst, remainSize);
#else
        nread = pread(fd_, dst, remainSize, startPos);
#endif
        if (nread <= 0) {
            break;
        }
        startPos += static_cast<size_t>(nread);
        dst += nread;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (remainSize > static_cast<size_t>(nread)) {
            remainSize -= static_cast<size_t>(nread);
        } else {
            remainSize = 0;
        }
    } while (remainSize > 0);
    if (remainSize > 0) {
        LOG(ERROR, ZIPARCHIVE) << "readfile error: " << filePath_ << ", errno: " << errno;
        return false;
    }

    if (bufferSize > BIG_FILE_SIZE) {
        LOG(WARNING, ZIPARCHIVE) << "big file io success: " << bufferSize;
    }
    return true;
}
}  // namespace ark::extractor
