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

#ifndef LIBZIPARCHIVE_EXTRACTORTOOL_FILE_MAPPER_H
#define LIBZIPARCHIVE_EXTRACTORTOOL_FILE_MAPPER_H

#include <memory>
#include <string>
#include <cstdint>

namespace ark::extractor {
class ZipFileReader;
enum class FileMapperType { NORMAL_MEM, SHARED_MMAP, SAFE_ABC };

class FileMapper {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    FileMapper();
    FileMapper(FileMapper &) = delete;
    void operator=(FileMapper &) = delete;
    ~FileMapper();
    // CC-OFFNXT(readability-function-size_parameters) function for CreateFileMapper
    bool CreateFileMapper(const std::string &fileName, bool compress, int32_t fd, size_t offset, size_t len,
                          FileMapperType type);

    bool CreateFileMapper(const std::shared_ptr<ZipFileReader> &fileReader, const std::string &fileName, size_t offset,
                          size_t len, bool compress);

    bool IsCompressed();
    uint8_t *GetDataPtr();
    size_t GetDataLen();
    std::string GetFileName();
    int32_t GetOffset();

private:
    std::string fileName_;
    bool isCompressed_ = false;

    std::unique_ptr<uint8_t[]> dataPtr_ = nullptr;  // NOLINT (modernize-avoid-c-arrays)
    size_t dataLen_ = 0;
    size_t offset_ = 0;

    uint8_t *basePtr_ = nullptr;
    uint8_t *usePtr_ = nullptr;
    size_t baseLen_ = 0;
    FileMapperType type_ = FileMapperType::NORMAL_MEM;
};
}  // namespace ark::extractor
#endif
