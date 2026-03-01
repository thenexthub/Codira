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

#include "file_mapper.h"

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "zip_file_reader.h"

namespace ark::extractor {
namespace {
int64_t g_pageSize = 0;
const uint32_t MAP_XPM = 0x40;
}  // namespace

FileMapper::FileMapper()
{
    if (g_pageSize <= 0) {
        g_pageSize = static_cast<int64_t>(os::mem::GetPageSize());
    }
}

FileMapper::~FileMapper()
{
    if (basePtr_ != nullptr && type_ == FileMapperType::SHARED_MMAP) {
        os::mem::MmapDeleter(reinterpret_cast<std::byte *>(basePtr_), baseLen_);
    }
}

// CC-OFFNXT(readability-function-size_parameters) function for CreateFileMapper
bool FileMapper::CreateFileMapper(const std::string &fileName, bool compress, int32_t fd, size_t offset, size_t len,
                                  FileMapperType type)
{
    if (fd < 0 || len == 0 || type == FileMapperType::NORMAL_MEM) {
        LOG(ERROR, ZIPARCHIVE) << "Invalid param fileName: " << fileName;
        return false;
    }
    if (g_pageSize <= 0) {
        LOG(ERROR, ZIPARCHIVE) << "Wrong Pagesize: " << g_pageSize;
        return false;
    }
    if (dataLen_ > 0) {
        LOG(ERROR, ZIPARCHIVE) << "data not empty fileName: " << fileName;
        return false;
    }

    size_t adjust = offset % static_cast<size_t>(g_pageSize);
    size_t adjOffset = offset - adjust;
    baseLen_ = len + adjust;
    int32_t mmapFlag = os::mem::MMAP_FLAG_PRIVATE | MAP_XPM;
    if (type == FileMapperType::SHARED_MMAP) {
        mmapFlag = os::mem::MMAP_FLAG_SHARED;
    }
#ifdef PANDA_TARGET_WINDOWS
    basePtr_ =
        reinterpret_cast<uint8_t *>(os::mem::mmap(nullptr, baseLen_, os::mem::MMAP_PROT_READ, mmapFlag, fd, adjOffset));
    if (basePtr_ == reinterpret_cast<void *>(-1)) {
#else
    basePtr_ = reinterpret_cast<uint8_t *>(mmap(nullptr, baseLen_, os::mem::MMAP_PROT_READ, mmapFlag, fd, adjOffset));
    if (basePtr_ == MAP_FAILED) {
#endif
        LOG(ERROR, ZIPARCHIVE) << "mmap failed, errno: " << errno << ", fileName: " << fileName
                               << ", offset: " << offset << ", pageSize: " << g_pageSize << ", mmapFlag: " << mmapFlag;
        baseLen_ = 0;
        return false;
    }

    isCompressed_ = compress;
    fileName_ = fileName;
    offset_ = offset;
    dataLen_ = len;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    usePtr_ = reinterpret_cast<uint8_t *>(basePtr_) + adjust;
    type_ = type;
    return true;
}

bool FileMapper::CreateFileMapper(const std::shared_ptr<ZipFileReader> &fileReader, const std::string &fileName,
                                  size_t offset, size_t len, bool compress)
{
    if (!fileReader) {
        LOG(ERROR, ZIPARCHIVE) << "file null fileName: " << fileName;
        return false;
    }
    if (!fileName_.empty()) {
        LOG(ERROR, ZIPARCHIVE) << "data not empty fileName: " << fileName;
        return false;
    }

    dataPtr_ = std::make_unique<uint8_t[]>(len);  // NOLINT (modernize-avoid-c-arrays)
    if (!fileReader->ReadBuffer(dataPtr_.get(), offset, len)) {
        LOG(ERROR, ZIPARCHIVE) << "read failed, len: " << len << ", fileName: " << fileName << ", offset: " << offset;
        dataPtr_.reset();
        return false;
    }
    isCompressed_ = compress;
    dataLen_ = len;
    offset_ = offset;
    fileName_ = fileName;

    return true;
}

bool FileMapper::IsCompressed()
{
    return isCompressed_;
}

uint8_t *FileMapper::GetDataPtr()
{
    return dataPtr_ ? dataPtr_.get() : usePtr_;
}

size_t FileMapper::GetDataLen()
{
    return dataLen_;
}

std::string FileMapper::GetFileName()
{
    return fileName_;
}

int32_t FileMapper::GetOffset()
{
    return offset_;
}
}  // namespace ark::extractor
