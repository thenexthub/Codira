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

#ifndef LIBPANDAFILE_BYTECODE_INSTRUCTION_H_
#define LIBPANDAFILE_BYTECODE_INSTRUCTION_H_

#include "libarkfile/file.h"

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include "libarkbase/utils/bit_helpers.h"
#include "libarkbase/utils/utils.h"

#if !PANDA_TARGET_WINDOWS
#include "securec.h"
#endif

namespace ark {

enum class BytecodeInstMode { FAST, SAFE };

template <const BytecodeInstMode>
class BytecodeInstBase;

class BytecodeId {
public:
    constexpr explicit BytecodeId(uint32_t id) : id_(id) {}

    constexpr BytecodeId() = default;

    ~BytecodeId() = default;

    DEFAULT_COPY_SEMANTIC(BytecodeId);
    NO_MOVE_SEMANTIC(BytecodeId);

    panda_file::File::Index AsIndex() const
    {
        ASSERT(id_ < std::numeric_limits<uint16_t>::max());
        return id_;
    }

    panda_file::File::EntityId AsFileId() const
    {
        return panda_file::File::EntityId(id_);
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

template <>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class BytecodeInstBase<BytecodeInstMode::FAST> {
public:
    BytecodeInstBase() = default;
    explicit BytecodeInstBase(const uint8_t *pc) : pc_ {pc} {}
    ~BytecodeInstBase() = default;

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
        using UnalignedType __attribute__((aligned(1))) = T;
        return *reinterpret_cast<const UnalignedType *>(GetPointer(static_cast<int32_t>(offset)));
    }

    void Write(uint32_t value, uint32_t offset, uint32_t width)
    {
        auto *dst = const_cast<uint8_t *>(GetPointer(static_cast<int32_t>(offset)));
        MemcpyUnsafe(dst, &value, width);
    }

    uint8_t ReadByte(size_t offset) const
    {
        return Read<uint8_t>(offset);
    }

private:
    const uint8_t *pc_ {nullptr};
};

template <>
class BytecodeInstBase<BytecodeInstMode::SAFE> {
public:
    BytecodeInstBase() = default;
    explicit BytecodeInstBase(const uint8_t *pc, const uint8_t *from, const uint8_t *to)
        : pc_ {pc}, from_ {from}, to_ {to}, valid_ {true}
    {
        ASSERT(from_ <= to_ && pc_ >= from_ && pc_ < to_);
    }

protected:
    const uint8_t *GetPointer(int32_t offset) const
    {
        return GetPointer(offset, 1);
    }

    bool IsLast(size_t size) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t *ptrNext = pc_ + size;
        return ptrNext >= to_;
    }

    NO_UB_SANITIZE const uint8_t *GetPointer(int32_t offset, size_t size) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t *ptrFrom = pc_ + offset;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t *ptrTo = ptrFrom + size;
        if (from_ == nullptr || ptrFrom < from_ || ptrTo > to_) {
            valid_ = false;
            return from_;
        }
        return ptrFrom;
    }

    const uint8_t *GetAddress() const
    {
        return pc_;
    }

    const uint8_t *GetFrom() const
    {
        return from_;
    }

    const uint8_t *GetTo() const
    {
        return to_;
    }

    uint32_t GetOffset() const
    {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pc_) - reinterpret_cast<uintptr_t>(from_));
    }

    const uint8_t *GetAddress() volatile const
    {
        return pc_;
    }

    template <class T>
    T Read(size_t offset) const
    {
        using UnalignedType __attribute__((aligned(1))) = T;
        auto ptr = reinterpret_cast<const UnalignedType *>(GetPointer(static_cast<int32_t>(offset), sizeof(T)));
        if (IsValid()) {
            return *ptr;
        }
        return {};
    }

    bool IsValid() const
    {
        return valid_;
    }

private:
    const uint8_t *pc_ {nullptr};
    const uint8_t *from_ {nullptr};
    const uint8_t *to_ {nullptr};
    mutable bool valid_ {false};
};

template <const BytecodeInstMode MODE = BytecodeInstMode::FAST>

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class BytecodeInst : public BytecodeInstBase<MODE> {
    using Base = BytecodeInstBase<MODE>;

public:
#include <libarkfile/include/bytecode_instruction_enum_gen.h>

    BytecodeInst() = default;

    ~BytecodeInst() = default;

    template <const BytecodeInstMode M = MODE, typename = std::enable_if_t<M == BytecodeInstMode::FAST>>
    explicit BytecodeInst(const uint8_t *pc) : Base {pc}
    {
    }

    template <const BytecodeInstMode M = MODE, typename = std::enable_if_t<M == BytecodeInstMode::SAFE>>
    explicit BytecodeInst(const uint8_t *pc, const uint8_t *from, const uint8_t *to) : Base {pc, from, to}
    {
    }

    template <Format FORMAT, typename EnumT = BytecodeInst<MODE>::Opcode, size_t IDX = 0>
    BytecodeId GetId() const;

    template <Format FORMAT, size_t IDX = 0>
    uint16_t GetVReg() const;

    template <Format FORMAT>
    uint16_t GetVReg(size_t idx) const;

    template <Format FORMAT, size_t IDX = 0>
    auto GetImm() const;

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    BytecodeId GetId(size_t idx = 0) const;

    void UpdateId(BytecodeId newId, uint32_t idx = 0);

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    uint16_t GetVReg(size_t idx = 0) const;

    // Read imm and return it as int64_t/uint64_t
    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    auto GetImm64(size_t idx = 0) const;

    /// Return profile id if instruction supports profiling, otherwise return -1.
    int GetProfileId() const;

    /**
     * Primary and Secondary Opcodes are used in interpreter/verifier instruction dispatch
     * while full Opcode is typically used for various instruction property query.
     *
     * Implementation note: one can describe Opcode in terms of Primary/Secondary opcodes
     * or vice versa. The first way is more preferable, because Primary/Secondary opcodes
     * are more performance critical and compiler is not always clever enough to reduce them
     * to simple byte reads.
     */
    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    EnumT GetOpcode() const;

    uint8_t GetPrimaryOpcode() const
    {
        return ReadByte(0);
    }

    bool IsPrimaryOpcodeValid() const;

    uint8_t GetSecondaryOpcode() const;

    bool IsPrefixed() const;

    static constexpr uint8_t GetMinPrefixOpcodeIndex();

    template <const BytecodeInstMode M = MODE>
    auto JumpTo(int32_t offset) const -> std::enable_if_t<M == BytecodeInstMode::FAST, BytecodeInst>
    {
        return BytecodeInst(Base::GetPointer(offset));
    }

    template <const BytecodeInstMode M = MODE>
    auto JumpTo(int32_t offset) const -> std::enable_if_t<M == BytecodeInstMode::SAFE, BytecodeInst>
    {
        if (!IsValid()) {
            return {};
        }
        const uint8_t *ptr = Base::GetPointer(offset);
        if (!IsValid()) {
            return {};
        }
        return BytecodeInst(ptr, Base::GetFrom(), Base::GetTo());
    }

    template <const BytecodeInstMode M = MODE>
    auto IsLast() const -> std::enable_if_t<M == BytecodeInstMode::SAFE, bool>
    {
        return Base::IsLast(GetSize());
    }

    template <const BytecodeInstMode M = MODE>
    auto IsValid() const -> std::enable_if_t<M == BytecodeInstMode::SAFE, bool>
    {
        return Base::IsValid();
    }

    template <Format FORMAT>
    BytecodeInst GetNext() const
    {
        return JumpTo(Size(FORMAT));
    }

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    BytecodeInst GetNext() const
    {
        return JumpTo(GetSize<EnumT>());
    }

    const uint8_t *GetAddress() const
    {
        return Base::GetAddress();
    }

    const uint8_t *GetAddress() volatile const
    {
        return Base::GetAddress();
    }

    template <const BytecodeInstMode M = MODE>
    auto GetFrom() const -> std::enable_if_t<M == BytecodeInstMode::SAFE, const uint8_t *>
    {
        return Base::GetFrom();
    }

    template <const BytecodeInstMode M = MODE>
    auto GetTo() const -> std::enable_if_t<M == BytecodeInstMode::SAFE, const uint8_t *>
    {
        return Base::GetTo();
    }

    template <const BytecodeInstMode M = MODE>
    auto GetOffset() const -> std::enable_if_t<M == BytecodeInstMode::SAFE, uint32_t>
    {
        return Base::GetOffset();
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

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    size_t GetSize() const;

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    Format GetFormat() const;

    template <typename EnumT = BytecodeInst<MODE>::Opcode>
    bool HasFlag(Flags flag) const;

    bool IsThrow(Exceptions exception) const;

    bool IsJump() const
    {
        return HasFlag(Flags::JUMP);
    }

    bool CanThrow() const;

    bool IsTerminator() const
    {
        return HasFlag(Flags::RETURN) || HasFlag(Flags::JUMP) || IsThrow(Exceptions::X_THROW);
    }

    static constexpr bool HasId(Format format, size_t idx);

    static constexpr bool HasVReg(Format format, size_t idx);

    static constexpr bool HasImm(Format format, size_t idx);

    static constexpr size_t Size(Format format);

    /* Checks if format is used to pass arguments in vregisters (without accumulator) */
    static constexpr bool IsVregArgsShort(Format format);
    static constexpr bool IsVregArgs(Format format);
    static constexpr bool IsVregArgsRange(Format format);

    template <BytecodeInst<MODE>::Opcode OPCODE>
    static constexpr auto GetQuickened();

    template <BytecodeInst<MODE>::Format FORMAT>
    static constexpr auto GetQuickened();
};

template <const BytecodeInstMode MODE>
std::ostream &operator<<(std::ostream &os, const BytecodeInst<MODE> &inst);

using BytecodeInstruction = BytecodeInst<BytecodeInstMode::FAST>;
using BytecodeInstructionSafe = BytecodeInst<BytecodeInstMode::SAFE>;

template <bool IS_QUICKENED>
class BytecodeInstructionResolver {
public:
    template <BytecodeInstruction::Opcode OPCODE>
    static constexpr auto Get()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (IS_QUICKENED) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            return BytecodeInstruction::GetQuickened<OPCODE>();
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            // NOLINTNEXTLINE(readability-magic-numbers)
            return OPCODE;
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    static constexpr auto Get()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (IS_QUICKENED) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            return BytecodeInstruction::GetQuickened<FORMAT>();
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            // NOLINTNEXTLINE(readability-magic-numbers)
            return FORMAT;
        }
    }
};

}  // namespace ark

#endif  // LIBANDAFILE_BYTECODE_INSTRUCTION_H_
