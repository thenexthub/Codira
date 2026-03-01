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

#ifndef LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_BYTECODE_INST_H
#define LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_BYTECODE_INST_H

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <memory>
#include <array>
#include "libabckit/src/macros.h"

#include "libarkbase/utils/bit_helpers.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/os/filesystem.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"

#include <iostream>

namespace libabckit {

using Index = uint16_t;

class EntityId {
public:
    explicit constexpr EntityId(uint32_t offset) : offset_(offset) {}

    uint32_t GetOffset() const
    {
        return offset_;
    }

    static constexpr size_t GetSize()
    {
        return sizeof(uint32_t);
    }

    friend bool operator<(const EntityId &l, const EntityId &r)
    {
        return l.offset_ < r.offset_;
    }

    friend bool operator==(const EntityId &l, const EntityId &r)
    {
        return l.offset_ == r.offset_;
    }

    friend std::ostream &operator<<(std::ostream &stream, const EntityId &id)
    {
        return stream << id.offset_;
    }

private:
    uint32_t offset_ {0};
};

class BytecodeId {
public:
    constexpr explicit BytecodeId(uint32_t id) : id_(id) {}

    constexpr BytecodeId() = default;

    ~BytecodeId() = default;

    DEFAULT_COPY_SEMANTIC(BytecodeId);
    NO_MOVE_SEMANTIC(BytecodeId);

    Index AsIndex() const
    {
        ASSERT(id_ < std::numeric_limits<uint16_t>::max());
        return id_;
    }

    EntityId AsFileId() const
    {
        return EntityId(id_);
    }

    uint32_t AsRawValue() const
    {
        return id_;
    }

    bool IsValid() const
    {
        return id_ != INVALID;
    }

    bool operator==(BytecodeId id) const noexcept
    {
        return id_ == id.id_;
    }

    friend std::ostream &operator<<(std::ostream &stream, BytecodeId id)
    {
        return stream << id.id_;
    }

private:
    static constexpr size_t INVALID = std::numeric_limits<uint32_t>::max();

    uint32_t id_ {INVALID};
};

class BytecodeInstBase {
public:
    BytecodeInstBase() = default;
    explicit BytecodeInstBase(const uint8_t *pc) : pc_ {pc} {}
    DEFAULT_COPY_SEMANTIC(BytecodeInstBase);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BytecodeInstBase);
    virtual ~BytecodeInstBase() = default;

protected:
    const uint8_t *GetPointer(int32_t offset) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return pc_ + offset;
    }

    const uint8_t *GetAddress() const
    {
        return pc_;
    }

    const uint8_t *GetAddress() volatile const
    {
        return pc_;
    }

    template <class T>
    T Read(size_t offset) const
    {
        using UnalignedType __attribute__((aligned(1))) = const T;
        return *reinterpret_cast<UnalignedType *>(GetPointer(offset));
    }

    void Write(uint32_t value, uint32_t offset, uint32_t width)
    {
        auto *dst = const_cast<uint8_t *>(GetPointer(offset));
        if (memcpy_s(dst, width, &value, width) != 0) {
            std::cerr << "Cannot write value : " << value << "at the dst offset : " << offset << std::endl;
        }
    }

private:
    const uint8_t *pc_ {nullptr};
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class BytecodeInst : public BytecodeInstBase {
    using Base = BytecodeInstBase;

public:
#include <generated/bytecode_inst_enum_gen.h>

    BytecodeInst() = default;
    DEFAULT_COPY_SEMANTIC(BytecodeInst);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BytecodeInst);
    ~BytecodeInst() override = default;

    explicit BytecodeInst(const uint8_t *pc) : Base {pc} {}

    template <Format FORMAT, size_t IDX = 0, bool IS_SIGNED = false>
    auto GetImm() const;

    BytecodeId GetId(size_t idx = 0) const;

    uint16_t GetVReg(size_t idx = 0) const;

    BytecodeInst::Opcode GetOpcode() const;

    uint8_t GetPrimaryOpcode() const
    {
        return ReadByte(0);
    }

    uint8_t GetSecondaryOpcode() const;

    auto JumpTo(int32_t offset) const
    {
        return BytecodeInst(Base::GetPointer(offset));
    }

    template <Format FORMAT>
    BytecodeInst GetNext() const
    {
        return JumpTo(Size(FORMAT));
    }

    BytecodeInst GetNext() const
    {
        return JumpTo(GetSize());
    }

    const uint8_t *GetAddress() const
    {
        return Base::GetAddress();
    }

    const uint8_t *GetAddress() volatile const
    {
        return Base::GetAddress();
    }

    uint8_t ReadByte(size_t offset) const
    {
        return Base::template Read<uint8_t>(offset);
    }

    template <class R, class S>
    auto ReadHelper(size_t byteoffset, size_t bytecount, size_t offset, size_t width) const;

    template <size_t OFFSET, size_t WIDTH, bool IS_SIGNED = false>
    auto Read() const;

    template <bool IS_SIGNED = false>
    auto Read64(size_t offset, size_t width) const;

    size_t GetSize() const;

    Format GetFormat() const;

    bool HasFlag(Flags flag) const;

    bool IsThrow(Exceptions exception) const;

    bool IsTerminator() const
    {
        return HasFlag(Flags::RETURN) || HasFlag(Flags::JUMP) || IsThrow(Exceptions::X_THROW);
    }

    bool IsSuspend() const
    {
        return HasFlag(Flags::SUSPEND);
    }

    static constexpr bool HasId(Format format, size_t idx);

    static constexpr bool HasVReg(Format format, size_t idx);

    static constexpr bool HasImm(Format format, size_t idx);

    static constexpr Format GetFormat(Opcode opcode);

    static constexpr size_t Size(Format format);
};

std::ostream &operator<<(std::ostream &os, const BytecodeInst &inst);

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_BYTECODE_INST_H
