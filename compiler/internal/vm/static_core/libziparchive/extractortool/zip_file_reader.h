/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_READER_H
#define LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_READER_H

#include <memory>
#include <string>
#include <unistd.h>
#include <cstdint>

namespace ark::extractor {
class ZipFileReader {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    static std::shared_ptr<ZipFileReader> CreateZipFileReader(const std::string &filePath);
    static size_t GetFileLen(const std::string &filePath);

    // NOLINTNEXTLINE(modernize-pass-by-value)
    explicit ZipFileReader(const std::string &filePath) : filePath_(filePath) {}
    ZipFileReader(ZipFileReader &) = delete;
    void operator=(ZipFileReader &) = delete;
    virtual ~ZipFileReader();

    virtual std::string ReadBuffer(size_t startPos, size_t bufferSize) = 0;
    virtual bool ReadBuffer(uint8_t *dst, size_t startPos, size_t bufferSize) = 0;
    size_t GetFileLen() const
    {
        return fileLen_;
    }
    int GetFd() const
    {
        return fd_;
    }
    void SetClosable(bool closable)
    {
        closable_ = closable;
    }

protected:
    virtual bool Init();

protected:
    std::string filePath_;  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t fileLen_ = 0;    // NOLINT(misc-non-private-member-variables-in-classes)

    // For safe memory, reserve this field and keep the file opened.
    int fd_ = -1;  // NOLINT(misc-non-private-member-variables-in-classes)
    // close fd in destructor
    bool closable_ = true;  // NOLINT(misc-non-private-member-variables-in-classes)
};
}  // namespace ark::extractor
#endif
