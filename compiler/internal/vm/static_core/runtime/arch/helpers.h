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
#ifndef PANDA_RUNTIME_ARCH_HELPERS_H_
#define PANDA_RUNTIME_ARCH_HELPERS_H_

#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/span.h"
#include "runtime/include/value.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::arch {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARCH_COPY_METHOD_ARGS_DISPATCH                  \
    it.IncrementWithoutCheck();                         \
    encoding = (*it).GetEncoding();                     \
    ASSERT(!(encoding & ~SHORTY_ELEM_MAX));             \
    /* CC-OFFNXT(G.PRE.05, G.PRE.09) code generation */ \
    goto *dispatch_table[encoding];

// We use macro instead of function because it's impossible to inline a dispatch table
// We should inline the dispatch table for performance reasons.
// LABEL_TYPEID_INVALID before LABEL_TYPEID_REFERENCE refers to tagged type (types.yaml) and does not handles here
// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARCH_COPY_METHOD_ARGS(METHOD, ARG_READER, ARG_WRITER)                                                          \
    do {                                                                                                               \
        [[maybe_unused]] static constexpr size_t SHORTY_ELEM_MAX = 0xF;                                                \
        static constexpr std::array dispatch_table = {                                                                 \
            static_cast<const void *>(&&LABEL_TYPEID_END),       static_cast<const void *>(&&LABEL_TYPEID_VOID),       \
            static_cast<const void *>(&&LABEL_TYPEID_U8),        static_cast<const void *>(&&LABEL_TYPEID_I8),         \
            static_cast<const void *>(&&LABEL_TYPEID_U8),        static_cast<const void *>(&&LABEL_TYPEID_I16),        \
            static_cast<const void *>(&&LABEL_TYPEID_U16),       static_cast<const void *>(&&LABEL_TYPEID_I32),        \
            static_cast<const void *>(&&LABEL_TYPEID_U32),       static_cast<const void *>(&&LABEL_TYPEID_F32),        \
            static_cast<const void *>(&&LABEL_TYPEID_F64),       static_cast<const void *>(&&LABEL_TYPEID_I64),        \
            static_cast<const void *>(&&LABEL_TYPEID_U64),       static_cast<const void *>(&&LABEL_TYPEID_INVALID),    \
            static_cast<const void *>(&&LABEL_TYPEID_REFERENCE), static_cast<const void *>(&&LABEL_TYPEID_INVALID)};   \
                                                                                                                       \
        static_assert(dispatch_table.size() - 1 == SHORTY_ELEM_MAX);                                                   \
        ASSERT(dispatch_table[static_cast<uint8_t>((*panda_file::ShortyIterator()).GetId())] == &&LABEL_TYPEID_END);   \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::U1)] == &&LABEL_TYPEID_U8);               \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::I8)] == &&LABEL_TYPEID_I8);               \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::U8)] == &&LABEL_TYPEID_U8);               \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::I16)] == &&LABEL_TYPEID_I16);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::U16)] == &&LABEL_TYPEID_U16);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::I32)] == &&LABEL_TYPEID_I32);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::U32)] == &&LABEL_TYPEID_U32);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::F32)] == &&LABEL_TYPEID_F32);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::F64)] == &&LABEL_TYPEID_F64);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::I64)] == &&LABEL_TYPEID_I64);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::U64)] == &&LABEL_TYPEID_U64);             \
        ASSERT(dispatch_table[static_cast<uint8_t>(panda_file::Type::TypeId::REFERENCE)] == &&LABEL_TYPEID_REFERENCE); \
                                                                                                                       \
        uint8_t encoding = 0;                                                                                          \
        panda_file::ShortyIterator it((METHOD)->GetShorty());                                                          \
        /* Skip the return value */                                                                                    \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
                                                                                                                       \
    LABEL_TYPEID_VOID : {                                                                                              \
        LOG(FATAL, RUNTIME) << "Void argument is impossible";                                                          \
        UNREACHABLE();                                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_I8 : {                                                                                                \
        auto v = (ARG_READER).template Read<int8_t>();                                                                 \
        (ARG_WRITER).template Write<int8_t>(v);                                                                        \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_U8 : {                                                                                                \
        auto v = (ARG_READER).template Read<uint8_t>();                                                                \
        (ARG_WRITER).template Write<uint8_t>(v);                                                                       \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_I16 : {                                                                                               \
        auto v = (ARG_READER).template Read<int16_t>();                                                                \
        (ARG_WRITER).template Write<int16_t>(v);                                                                       \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_U16 : {                                                                                               \
        auto v = (ARG_READER).template Read<uint16_t>();                                                               \
        (ARG_WRITER).template Write<uint16_t>(v);                                                                      \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_I32 : {                                                                                               \
        auto v = (ARG_READER).template Read<int32_t>();                                                                \
        (ARG_WRITER).template Write<int32_t>(v);                                                                       \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_U32 : {                                                                                               \
        auto v = (ARG_READER).template Read<uint32_t>();                                                               \
        (ARG_WRITER).template Write<uint32_t>(v);                                                                      \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_F32 : {                                                                                               \
        auto v = (ARG_READER).template Read<float>();                                                                  \
        (ARG_WRITER).template Write<float>(v);                                                                         \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_F64 : {                                                                                               \
        auto v = (ARG_READER).template Read<double>();                                                                 \
        (ARG_WRITER).template Write<double>(v);                                                                        \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_I64 : {                                                                                               \
        auto v = (ARG_READER).template Read<int64_t>();                                                                \
        (ARG_WRITER).template Write<int64_t>(v);                                                                       \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_U64 : {                                                                                               \
        auto v = (ARG_READER).template Read<uint64_t>();                                                               \
        (ARG_WRITER).template Write<uint64_t>(v);                                                                      \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_REFERENCE : {                                                                                         \
        auto v = const_cast<ObjectHeader **>((ARG_READER).template ReadPtr<ObjectHeader *>());                         \
        (ARG_WRITER).template Write<ObjectHeader **>(v);                                                               \
        ARCH_COPY_METHOD_ARGS_DISPATCH                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_INVALID : {                                                                                           \
        LOG(FATAL, RUNTIME) << "Invalid method's shorty, unreachable type ID";                                         \
        UNREACHABLE();                                                                                                 \
    }                                                                                                                  \
    LABEL_TYPEID_END:;                                                                                                 \
    } while (false)

template <Arch A>
struct ExtArchTraits;

#if !defined(PANDA_TARGET_ARM32_ABI_HARD)
template <>
struct ExtArchTraits<Arch::AARCH32> {
    using SignedWordType = int32_t;
    using UnsignedWordType = uint32_t;

    static constexpr size_t NUM_GP_ARG_REGS = 4;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 0;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = 0;
    static constexpr bool HARDFP = false;
};
#else   // !defined(PANDA_TARGET_ARM32_ABI_HARD)
template <>
struct ExtArchTraits<Arch::AARCH32> {
    using SignedWordType = int32_t;
    using UnsignedWordType = uint32_t;

    static constexpr size_t NUM_GP_ARG_REGS = 4;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 16; /* s0 - s15 */
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::AARCH32>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};
#endif  // !defined(PANDA_TARGET_ARM32_ABI_HARD)

template <>
struct ExtArchTraits<Arch::AARCH64> {
    using SignedWordType = int64_t;
    using UnsignedWordType = uint64_t;

    static constexpr size_t NUM_GP_ARG_REGS = 8;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 8;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::AARCH64>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};

template <>
struct ExtArchTraits<Arch::X86_64> {
    using SignedWordType = int64_t;
    using UnsignedWordType = uint64_t;

    static constexpr size_t NUM_GP_ARG_REGS = 6;
    static constexpr size_t GP_ARG_NUM_BYTES = NUM_GP_ARG_REGS * ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t NUM_FP_ARG_REGS = 8;
    static constexpr size_t FP_ARG_NUM_BYTES = NUM_FP_ARG_REGS * ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t GPR_SIZE = ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr size_t FPR_SIZE = ArchTraits<Arch::X86_64>::POINTER_SIZE;
    static constexpr bool HARDFP = true;
};

template <class T>
inline uint8_t *AlignPtr(uint8_t *ptr)
{
    return reinterpret_cast<uint8_t *>(RoundUp(reinterpret_cast<uintptr_t>(ptr), sizeof(T)));
}

template <class T>
inline const uint8_t *AlignPtr(const uint8_t *ptr)
{
    return reinterpret_cast<const uint8_t *>(RoundUp(reinterpret_cast<uintptr_t>(ptr), sizeof(T)));
}

template <typename T>
typename std::enable_if<sizeof(T) < sizeof(uint32_t), uint8_t *>::type WriteToMem(T v, uint8_t *mem)
{
    /*
     * When the type is less than 4 bytes
     * We write 4 bytes to stack with 0 in high bytes
     * To avoid of unspecified behavior
     */
    static_assert(!std::is_floating_point<T>::value);
    ASSERT(reinterpret_cast<std::uintptr_t>(mem) % sizeof(std::uintptr_t) == 0);

    *reinterpret_cast<uint32_t *>(mem) = 0;
    mem = AlignPtr<T>(mem);
    *reinterpret_cast<T *>(mem) = v;

    return mem;
}

template <typename T>
typename std::enable_if<(sizeof(T) >= sizeof(uint32_t)), uint8_t *>::type WriteToMem(T v, uint8_t *mem)
{
    ASSERT(reinterpret_cast<std::uintptr_t>(mem) % sizeof(std::uintptr_t) == 0);

    mem = AlignPtr<T>(mem);
    *reinterpret_cast<T *>(mem) = v;
    return mem;
}

template <Arch A>
class ArgCounter {
public:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, void> Count()
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        fprArgSize_ = RoundUp(fprArgSize_, NUM_BYTES);
        if (fprArgSize_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            fprArgSize_ += NUM_BYTES;
        } else {
            stackSize_ = RoundUp(stackSize_, NUM_BYTES);
            stackSize_ += NUM_BYTES;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), void> Count()
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), PTR_SIZE);
        gprArgSize_ = RoundUp(gprArgSize_, NUM_BYTES);
        if (gprArgSize_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            gprArgSize_ += NUM_BYTES;
        } else {
            stackSize_ = RoundUp(stackSize_, NUM_BYTES);
            stackSize_ += NUM_BYTES;
        }
    }

    size_t GetOnlyStackSize() const
    {
        return stackSize_;
    }

    size_t GetStackSize() const
    {
        return GetStackSpaceSize() / ArchTraits<A>::POINTER_SIZE;
    }

    size_t GetStackSpaceSize() const
    {
        return RoundUp(ExtArchTraits<A>::FP_ARG_NUM_BYTES + ExtArchTraits<A>::GP_ARG_NUM_BYTES + stackSize_,
                       2U * ArchTraits<A>::POINTER_SIZE);
    }

private:
    static constexpr size_t PTR_SIZE = ArchTraits<A>::POINTER_SIZE;
    size_t gprArgSize_ = 0;
    size_t fprArgSize_ = 0;
    size_t stackSize_ = 0;
};

template <Arch A>
class ArgReader {
public:
    ArgReader(Span<uint8_t> gprArgs, Span<uint8_t> fprArgs, const uint8_t *stackArgs)
        : gprArgs_(gprArgs), fprArgs_(fprArgs), stackArgs_(stackArgs)
    {
    }

    template <class T>
    ALWAYS_INLINE T Read()
    {
        return *ReadPtr<T>();
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        fpArgBytesRead_ = RoundUp(fpArgBytesRead_, READ_BYTES);
        if (fpArgBytesRead_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            const T *v = reinterpret_cast<const T *>(fprArgs_.data() + fpArgBytesRead_);
            fpArgBytesRead_ += READ_BYTES;
            return v;
        }
        stackArgs_ = AlignPtr<T>(stackArgs_);
        const T *v = reinterpret_cast<const T *>(stackArgs_);
        stackArgs_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = std::max(sizeof(T), PTR_SIZE);
        gpArgBytesRead_ = RoundUp(gpArgBytesRead_, READ_BYTES);
        if (gpArgBytesRead_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            const T *v = reinterpret_cast<const T *>(gprArgs_.data() + gpArgBytesRead_);
            gpArgBytesRead_ += READ_BYTES;
            return v;
        }
        stackArgs_ = AlignPtr<T>(stackArgs_);
        const T *v = reinterpret_cast<const T *>(stackArgs_);
        stackArgs_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

private:
    static constexpr size_t PTR_SIZE = ArchTraits<A>::POINTER_SIZE;
    Span<uint8_t> gprArgs_;
    Span<uint8_t> fprArgs_;
    const uint8_t *stackArgs_;
    size_t gpArgBytesRead_ = 0;
    size_t fpArgBytesRead_ = 0;
};

template <Arch A, class T>
using ExtArchTraitsWorldType = std::conditional_t<std::is_signed_v<T>, typename ExtArchTraits<A>::SignedWordType,
                                                  typename ExtArchTraits<A>::UnsignedWordType>;

template <Arch A>
class ArgWriterBase {
public:
    ArgWriterBase(Span<uint8_t> gprArgs, Span<uint8_t> fprArgs, uint8_t *stackArgs)
        : gprArgs_(gprArgs), fprArgs_(fprArgs), stackArgs_(stackArgs)
    {
    }
    ~ArgWriterBase() = default;

protected:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_integral_v<T> && sizeof(T) < ArchTraits<A>::POINTER_SIZE, void>
    RegisterValueWrite(T v)
    {
        *reinterpret_cast<ExtArchTraitsWorldType<A, T> *>(gprArgs_.data() + gpArgBytesWritten_) = v;
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_integral_v<T> && sizeof(T) < ArchTraits<A>::POINTER_SIZE), void>
    RegisterValueWrite(T v)
    {
        *reinterpret_cast<T *>(gprArgs_.data() + gpArgBytesWritten_) = v;
    }

    template <class T>
    void WriteNonFloatingPointValue(T v)
    {
        static_assert(!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP));

        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);
        gpArgBytesWritten_ = RoundUp(gpArgBytesWritten_, WRITE_BYTES);
        if (gpArgBytesWritten_ < ExtArchTraits<A>::GP_ARG_NUM_BYTES) {
            ArgWriterBase<A>::RegisterValueWrite(v);
            gpArgBytesWritten_ += WRITE_BYTES;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            stackArgs_ = WriteToMem(v, stackArgs_) + WRITE_BYTES;
        }
    }

    NO_COPY_SEMANTIC(ArgWriterBase);
    NO_MOVE_SEMANTIC(ArgWriterBase);

    static constexpr size_t PTR_SIZE =
        ArchTraits<A>::POINTER_SIZE;  // NOLINT(misc-non-private-member-variables-in-classes)
    Span<uint8_t> gprArgs_;           // NOLINT(misc-non-private-member-variables-in-classes)
    Span<uint8_t> fprArgs_;           // NOLINT(misc-non-private-member-variables-in-classes)
    uint8_t *stackArgs_;              // NOLINT(misc-non-private-member-variables-in-classes)
    size_t gpArgBytesWritten_ = 0;    // NOLINT(misc-non-private-member-variables-in-classes)
    size_t fpArgBytesWritten_ = 0;    // NOLINT(misc-non-private-member-variables-in-classes)
};

template <Arch A>
class ArgWriter : private ArgWriterBase<A> {
public:
    using ArgWriterBase<A>::gprArgs_;
    using ArgWriterBase<A>::fprArgs_;
    using ArgWriterBase<A>::stackArgs_;
    using ArgWriterBase<A>::gpArgBytesWritten_;
    using ArgWriterBase<A>::fpArgBytesWritten_;
    using ArgWriterBase<A>::PTR_SIZE;

    // NOLINTNEXTLINE(readability-non-const-parameter)
    ArgWriter(Span<uint8_t> gprArgs, Span<uint8_t> fprArgs, uint8_t *stackArgs)
        : ArgWriterBase<A>(gprArgs, fprArgs, stackArgs)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, void> Write(T v)
    {
        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);

        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<A>::FPR_SIZE);
        if (fpArgBytesWritten_ < ExtArchTraits<A>::FP_ARG_NUM_BYTES) {
            *reinterpret_cast<T *>(fprArgs_.data() + fpArgBytesWritten_) = v;
            fpArgBytesWritten_ += NUM_BYTES;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            stackArgs_ = WriteToMem(v, stackArgs_) + WRITE_BYTES;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), void> Write(T v)
    {
        ArgWriterBase<A>::WriteNonFloatingPointValue(v);
    }

    NO_COPY_SEMANTIC(ArgWriter);
    NO_MOVE_SEMANTIC(ArgWriter);
};

// This class is required due to specific calling conventions in AARCH32
template <>
class ArgWriter<Arch::AARCH32> : private ArgWriterBase<Arch::AARCH32> {
public:
    using ArgWriterBase<Arch::AARCH32>::gprArgs_;
    using ArgWriterBase<Arch::AARCH32>::fprArgs_;
    using ArgWriterBase<Arch::AARCH32>::stackArgs_;
    using ArgWriterBase<Arch::AARCH32>::gpArgBytesWritten_;
    using ArgWriterBase<Arch::AARCH32>::fpArgBytesWritten_;
    using ArgWriterBase<Arch::AARCH32>::PTR_SIZE;

    // NOLINTNEXTLINE(readability-non-const-parameter)
    ArgWriter(Span<uint8_t> gprArgs, Span<uint8_t> fprArgs, uint8_t *stackArgs)
        : ArgWriterBase<Arch::AARCH32>(gprArgs, fprArgs, stackArgs)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<Arch::AARCH32>::HARDFP, void>
    Write(T v)
    {
        constexpr size_t WRITE_BYTES = std::max(sizeof(T), PTR_SIZE);

        if (fpArgBytesWritten_ < ExtArchTraits<Arch::AARCH32>::FP_ARG_NUM_BYTES &&
            (std::is_same_v<T, float> ||
             (fpArgBytesWritten_ < ExtArchTraits<Arch::AARCH32>::FP_ARG_NUM_BYTES - sizeof(float))) &&
            !isFloatArmStackHasBeenWritten_) {
            RegisterFloatingPointValueWriteArm32(v);
            return;
        }

        isFloatArmStackHasBeenWritten_ = true;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stackArgs_ = WriteToMem(v, stackArgs_) + WRITE_BYTES;
    }

    template <class T>
    ALWAYS_INLINE
        typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<Arch::AARCH32>::HARDFP), void>
        Write(T v)
    {
        ArgWriterBase<Arch::AARCH32>::WriteNonFloatingPointValue(v);
    }

    NO_COPY_SEMANTIC(ArgWriter);
    NO_MOVE_SEMANTIC(ArgWriter);

private:
    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<(std::is_same_v<T, float>), void> RegisterFloatingPointValueWriteArm32(T v)
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<Arch::AARCH32>::FPR_SIZE);
        if (halfEmptyRegisterOffset_ == 0) {
            halfEmptyRegisterOffset_ = fpArgBytesWritten_ + sizeof(float);
            *reinterpret_cast<T *>(fprArgs_.data() + fpArgBytesWritten_) = v;
            fpArgBytesWritten_ += NUM_BYTES;
        } else {
            *reinterpret_cast<T *>(fprArgs_.data() + halfEmptyRegisterOffset_) = v;
            if (halfEmptyRegisterOffset_ == fpArgBytesWritten_) {
                fpArgBytesWritten_ += NUM_BYTES;
            }
            halfEmptyRegisterOffset_ = 0;
        }
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_same_v<T, float>), void> RegisterFloatingPointValueWriteArm32(T v)
    {
        constexpr size_t NUM_BYTES = std::max(sizeof(T), ExtArchTraits<Arch::AARCH32>::FPR_SIZE);
        fpArgBytesWritten_ = RoundUp(fpArgBytesWritten_, sizeof(T));
        *reinterpret_cast<T *>(fprArgs_.data() + fpArgBytesWritten_) = v;
        fpArgBytesWritten_ += NUM_BYTES;
    }

    size_t halfEmptyRegisterOffset_ = 0;
    bool isFloatArmStackHasBeenWritten_ = false;
};

class ValueWriter {
public:
    explicit ValueWriter(PandaVector<Value> *values) : values_(values) {}
    ~ValueWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_same<T, ObjectHeader **>::value, void> Write(T v)
    {
        values_->push_back(Value(*v));
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!std::is_same<T, ObjectHeader **>::value, void> Write(T v)
    {
        values_->push_back(Value(v));
    }

    NO_COPY_SEMANTIC(ValueWriter);
    NO_MOVE_SEMANTIC(ValueWriter);

private:
    PandaVector<Value> *values_;
};

template <Arch A>
class ArgReaderStack {
public:
    explicit ArgReaderStack(const uint8_t *stackArgs) : stackArgs_(stackArgs) {}

    template <class T>
    ALWAYS_INLINE T Read()
    {
        return *ReadPtr<T>();
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP, const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = sizeof(uint64_t);
        const T *v = reinterpret_cast<const T *>(stackArgs_);
        stackArgs_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!(std::is_floating_point_v<T> && ExtArchTraits<A>::HARDFP), const T *>
    ReadPtr()
    {
        constexpr size_t READ_BYTES = sizeof(uint64_t);
        const T *v = reinterpret_cast<const T *>(stackArgs_);
        stackArgs_ += READ_BYTES;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return v;
    }

private:
    const uint8_t *stackArgs_;
};

}  // namespace ark::arch

#endif  // PANDA_RUNTIME_ARCH_HELPERS_H_
