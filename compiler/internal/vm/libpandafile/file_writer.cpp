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

#include "file_writer.h"
#include "zlib.h"

namespace panda::panda_file {

FileWriter::FileWriter(const std::string &file_name) : checksum_(adler32(0, nullptr, 0))
{
#ifdef PANDA_TARGET_WINDOWS
    constexpr char const *mode = "wb";
#else
    constexpr char const *mode = "wbe";
#endif

    file_ = fopen(file_name.c_str(), mode);
}

FileWriter::~FileWriter()
{
    if (file_ != nullptr) {
        fclose(file_);
    }
}

bool FileWriter::WriteByte(uint8_t data)
{
    if (LIKELY(count_checksum_)) {
        checksum_ = adler32(checksum_, &data, 1u);
    }
    buffer_.push_back(data);
    return true;
}

bool FileWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    if (UNLIKELY(bytes.empty())) {
        return true;
    }

    if (LIKELY(count_checksum_)) {
        checksum_ = adler32(checksum_, bytes.data(), bytes.size());
    }

    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
    return true;
}

bool FileWriter::FinishWrite()
{
    if (file_ == nullptr) {
        return false;
    }
    const auto &buf = GetBuffer();
    auto length = buf.size();
    bool ret = fwrite(buf.data(), sizeof(decltype(buf.back())), length, file_) == length;
    fclose(file_);
    file_ = nullptr;
    return ret;
}

}  // namespace panda::panda_file
