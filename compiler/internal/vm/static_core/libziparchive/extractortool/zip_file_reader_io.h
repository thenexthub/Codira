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

#ifndef LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_READER_IO_H
#define LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_READER_IO_H

#include <string>

#include "zip_file_reader.h"

namespace ark::extractor {
class ZipFileReaderIo : public ZipFileReader {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    explicit ZipFileReaderIo(const std::string &filePath) : ZipFileReader(filePath) {}
    ~ZipFileReaderIo() override = default;

    std::string ReadBuffer(size_t startPos, size_t bufferSize) override;
    bool ReadBuffer(uint8_t *dst, size_t startPos, size_t bufferSize) override;
};
}  // namespace ark::extractor
#endif