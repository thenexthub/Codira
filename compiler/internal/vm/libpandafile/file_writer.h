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

#ifndef LIBPANDAFILE_FILE_WRITER_H
#define LIBPANDAFILE_FILE_WRITER_H

#include "os/file.h"
#include "utils/span.h"
#include "utils/type_helpers.h"
#include "utils/leb128.h"
#include "securec.h"

#include <cstdint>
#include <cerrno>

#include <limits>
#include <vector>

namespace panda::panda_file {

class Writer {
public:
    virtual bool WriteByte(uint8_t byte) = 0;

    virtual bool WriteBytes(const std::vector<uint8_t> &bytes) = 0;

    virtual size_t GetOffset() const = 0;

    virtual void CountChecksum(bool /* counting */) {}

    virtual bool RewriteChecksum(size_t /* offset */)
    {
        return false;
    }

    bool Align(size_t alignment)
    {
        size_t offset = GetOffset();
        size_t n = RoundUp(offset, alignment) - offset;
        while (n-- > 0) {
            if (!WriteByte(0)) {
                return false;
            }
        }
        return true;
    }

    template <class T>
    bool Write(T data)
    {
        static constexpr size_t BYTE_MASK = 0xff;
        static constexpr size_t BYTE_WIDTH = std::numeric_limits<uint8_t>::digits;

        for (size_t i = 0; i < sizeof(T); i++) {
            if (!WriteByte(data & BYTE_MASK)) {
                return false;
            }

            if (sizeof(T) > sizeof(uint8_t)) {
                data >>= BYTE_WIDTH;
            }
        }
        return true;
    }

    template <class T>
    bool WriteUleb128(T v)
    {
        size_t n = leb128::UnsignedEncodingSize(v);
        std::vector<uint8_t> out(n);
        leb128::EncodeUnsigned(v, out.data());
        return WriteBytes(out);
    }

    template <class T>
    bool WriteSleb128(T v)
    {
        size_t n = leb128::SignedEncodingSize(v);
        std::vector<uint8_t> out(n);
        leb128::EncodeSigned(v, out.data());
        return WriteBytes(out);
    }

    virtual void ReserveBufferCapacity([[maybe_unused]] size_t size) {}

    virtual bool FinishWrite()
    {
        return true;
    }

    // default methods
    Writer() = default;
    virtual ~Writer() = default;

    NO_COPY_SEMANTIC(Writer);
    NO_MOVE_SEMANTIC(Writer);
};

class MemoryWriter : public Writer {
public:
    bool WriteByte(uint8_t byte) override
    {
        data_.push_back(byte);
        return true;
    }

    bool WriteBytes(const std::vector<uint8_t> &bytes) override
    {
        data_.insert(data_.end(), bytes.cbegin(), bytes.cend());
        return true;
    }

    const std::vector<uint8_t> &GetData()
    {
        return data_;
    }

    size_t GetOffset() const override
    {
        return data_.size();
    }

private:
    std::vector<uint8_t> data_;
};

class MemoryBufferWriter : public Writer {
public:
    explicit MemoryBufferWriter(uint8_t *buffer, size_t size) : sp_(buffer, size) {}

    ~MemoryBufferWriter() override = default;

    NO_COPY_SEMANTIC(MemoryBufferWriter);
    NO_MOVE_SEMANTIC(MemoryBufferWriter);

    bool WriteByte(uint8_t byte) override
    {
        sp_[offset_++] = byte;
        return true;
    }

    bool WriteBytes(const std::vector<uint8_t> &bytes) override
    {
        if (bytes.empty()) {
            return true;
        }

        auto sub_sp = sp_.SubSpan(offset_, bytes.size());
        if (memcpy_s(sub_sp.data(), sub_sp.size(), bytes.data(), bytes.size()) != 0) {
            return false;
        }
        offset_ += bytes.size();
        return true;
    }

    size_t GetOffset() const override
    {
        return offset_;
    }

private:
    Span<uint8_t> sp_;
    size_t offset_ {0};
};

class FileWriter : public Writer {
public:
    explicit FileWriter(const std::string &file_name);

    ~FileWriter() override;

    NO_COPY_SEMANTIC(FileWriter);
    NO_MOVE_SEMANTIC(FileWriter);

    void CountChecksum(bool counting) override
    {
        count_checksum_ = counting;
    }

    bool RewriteChecksum(size_t offset) override
    {
        static constexpr size_t MASK = 0xff;
        static constexpr size_t WIDTH = std::numeric_limits<uint8_t>::digits;

        size_t length = sizeof(uint32_t);
        if (offset + length > buffer_.size()) {
            return false;
        }
        uint32_t temp = checksum_;
        for (size_t i = 0; i < length; i++) {
            buffer_[offset + i] = temp & MASK;
            temp >>= WIDTH;
        }
        return true;
    }

    bool WriteByte(uint8_t data) override;

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

    size_t GetOffset() const override
    {
        return buffer_.size();
    }

    uint32_t GetChecksum() const
    {
        return checksum_;
    }

    explicit operator bool() const
    {
        return file_ != nullptr;
    }

    void ReserveBufferCapacity(size_t size) override
    {
        buffer_.reserve(size);
    }

    const std::vector<uint8_t> &GetBuffer() const
    {
        return buffer_;
    }

    bool FinishWrite() override;

private:
    FILE *file_;
    uint32_t checksum_;
    bool count_checksum_ {false};
    std::vector<uint8_t> buffer_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_FILE_WRITER_H
