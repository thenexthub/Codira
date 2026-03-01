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

#ifndef COMPILER_OPTIMIZER_IR_DATATYPE_H
#define COMPILER_OPTIMIZER_IR_DATATYPE_H

#include "macros.h"
#include "utils/arch.h"
#include "mem/mem.h"

namespace panda::compiler {
/*
 * Type of the param/instruction
 */
// NOLINTNEXTLINE(readability-identifier-naming)
namespace DataType {
enum Type : uint8_t {
    NO_TYPE = 0,
    REFERENCE,
    BOOL,
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    UINT64,
    INT64,
    FLOAT32,
    FLOAT64,
    ANY,
    VOID,
    POINTER,
    LAST = POINTER
};

class Any {
public:
    explicit Any(uint64_t value) : value_(value) {}

    uint64_t Raw() const
    {
        return value_;
    }

private:
    uint64_t value_;
};

namespace internal {
inline constexpr std::array<const char *, Type::LAST + 1> TYPE_NAMES = {
    "",      // NO_TYPE
    "ref",   // REFERENCE
    "b",     // BOOL
    "u8",    // UINT8
    "i8",    // INT8
    "u16",   // UINT16
    "i16",   // INT16
    "u32",   // UINT32
    "i32",   // INT32
    "u64",   // UINT64
    "i64",   // INT64
    "f32",   // FLOAT32
    "f64",   // FLOAT64
    "any",   // ANY
    "void",  // VOID
    "ptr"    // POINTER
};
}  // namespace internal

inline const char *ToString(Type type)
{
    ASSERT(type <= Type::LAST);
    return internal::TYPE_NAMES[type];
}

constexpr inline Type GetCommonType(Type type)
{
    switch (type) {
        case Type::BOOL:
        case Type::UINT8:
        case Type::INT8:
        case Type::UINT16:
        case Type::INT16:
        case Type::UINT32:
        case Type::INT32:
        case Type::UINT64:
        case Type::INT64:
            return Type::INT64;
        default:
            return type;
    };
}

inline bool IsInt32Bit(Type type)
{
    switch (type) {
        case Type::BOOL:
        case Type::UINT8:
        case Type::INT8:
        case Type::UINT16:
        case Type::INT16:
        case Type::UINT32:
        case Type::INT32:
            return true;
        default:
            return false;
    };
}

inline bool IsTypeNumeric(Type type)
{
    switch (type) {
        case BOOL:
        case UINT8:
        case INT8:
        case UINT16:
        case INT16:
        case UINT32:
        case INT32:
        case UINT64:
        case INT64:
        case FLOAT32:
        case FLOAT64:
        case POINTER:
            return true;
        default:
            return false;
    }
}

inline bool IsLessInt32(Type type)
{
    switch (type) {
        case BOOL:
        case UINT8:
        case INT8:
        case UINT16:
        case INT16:
            return true;
        default:
            return false;
    }
}

inline Type GetIntTypeForReference([[maybe_unused]] Arch arch)
{
    // This is necessary for arm32 cross compilation
    if (arch == Arch::AARCH32) {
        return UINT32;
    }

    if constexpr (panda::OBJECT_POINTER_SIZE == sizeof(uint64_t)) {
        return UINT64;
    } else if constexpr (panda::OBJECT_POINTER_SIZE == sizeof(uint32_t)) {
        return UINT32;
    } else if constexpr (panda::OBJECT_POINTER_SIZE == sizeof(uint16_t)) {
        UNREACHABLE_CONSTEXPR();
        return UINT16;
    } else {
        UNREACHABLE_CONSTEXPR();
        return UINT8;
    }
}

constexpr inline Type GetIntTypeBySize(size_t size_in_bytes, bool is_signed = false)
{
    if (size_in_bytes <= sizeof(uint8_t)) {
        return is_signed ? INT8 : UINT8;
    }
    if (size_in_bytes <= sizeof(uint16_t)) {
        return is_signed ? INT16 : UINT16;
    }
    if (size_in_bytes <= sizeof(uint32_t)) {
        return is_signed ? INT32 : UINT32;
    }
    if (size_in_bytes <= sizeof(uint64_t)) {
        return is_signed ? INT64 : UINT64;
    }
    UNREACHABLE_CONSTEXPR();
    return NO_TYPE;
}

inline bool Is32Bits(Type type, Arch arch)
{
    switch (type) {
        case BOOL:
        case UINT8:
        case INT8:
        case UINT16:
        case INT16:
        case UINT32:
        case INT32:
        case FLOAT32:
            return true;
        case POINTER:
            return !Is64BitsArch(arch);
        case REFERENCE:
            return Is32Bits(GetIntTypeForReference(arch), arch);
        default:
            return false;
    }
}

inline bool Is64Bits(Type type, Arch arch)
{
    return !Is32Bits(type, arch);
}

inline bool IsFloatType(Type type)
{
    switch (type) {
        case FLOAT32:
        case FLOAT64:
            return true;
        default:
            return false;
    }
}

inline bool IsTypeSigned(Type type)
{
    switch (type) {
        case INT8:
        case INT16:
        case INT32:
        case INT64:
        case FLOAT32:
        case FLOAT64:
            return true;
        default:
            return false;
    }
}

inline bool IsReference(Type type)
{
    return type == REFERENCE;
}

inline bool IsTypeCollectable(DataType::Type type)
{
    return DataType::IsReference(type) || type == DataType::ANY;
}

inline uint8_t ShiftByType(Type type, Arch arch)
{
    switch (type) {
        case VOID:
        case BOOL:
        case UINT8:
        case INT8:
            return 0;
        case UINT16:
        case INT16:
            return 1;
        case UINT32:
        case INT32:
        case FLOAT32:
            return 2U;
        case UINT64:
        case INT64:
        case FLOAT64:
        case ANY:
            return 3U;
        case REFERENCE:
            return ShiftByType(GetIntTypeForReference(arch), arch);
        case POINTER:
            return Is64BitsArch(arch) ? 3U : 2U;
        default:
            UNREACHABLE();
            return 0;
    }
}

inline uint8_t GetTypeSize(Type type, Arch arch)
{
    // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
    return 8U << ShiftByType(type, arch);
}
}  // namespace DataType
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_DATATYPE_H
