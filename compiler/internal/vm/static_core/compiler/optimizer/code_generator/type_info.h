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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TYPE_INFO_H
#define COMPILER_OPTIMIZER_CODEGEN_TYPE_INFO_H

/*
Arch-feature definitions
*/
#include <bitset>
#include <cstdint>
#include <type_traits>

#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/regmask.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/datatype.h"
#include "libarkbase/utils/type_helpers.h"

namespace ark::compiler {
constexpr uint8_t BYTE_SIZE = 8;
constexpr uint8_t HALF_SIZE = 16;
constexpr uint8_t WORD_SIZE = 32;
constexpr uint8_t DOUBLE_WORD_SIZE = 64;
constexpr uint8_t HALF_WORD_SIZE_BYTES = 2;
constexpr uint8_t WORD_SIZE_BYTES = 4;
constexpr uint8_t DOUBLE_WORD_SIZE_BYTES = 8;
constexpr uint8_t QUAD_WORD_SIZE_BYTES = 16;
/// Maximum possible registers count (for scalar and for vector):
constexpr uint8_t MAX_NUM_REGS = 32;
constexpr uint8_t MAX_NUM_VREGS = 32;

constexpr uint64_t NAN_DOUBLE = uint64_t(0x7ff8000000000000);
constexpr uint32_t NAN_FLOAT = uint32_t(0x7fc00000);
constexpr uint32_t NAN_FLOAT_BITS = NAN_FLOAT >> 16U;

// Constants for cast from float to int64:
// The number of the bit from which exponential part starts in float
constexpr uint8_t START_EXP_FLOAT = 23;
// Size exponential part in float
constexpr uint8_t SIZE_EXP_FLOAT = 8;
// The maximum exponential part of float that can be loaded in int64
constexpr uint32_t POSSIBLE_EXP_FLOAT = 0xbe;
// Mask say that float number is NaN by IEEE 754
constexpr uint32_t UP_BITS_NAN_FLOAT = 0xff;

// Constants for cast from double to int64:
// The number of the bit from which exponential part starts in double
constexpr uint8_t START_EXP_DOUBLE = 20;
// Size exponential part in double
constexpr uint8_t SIZE_EXP_DOUBLE = 11;
// The maximum exponential part of double that can be loaded in int64
constexpr uint32_t POSSIBLE_EXP_DOUBLE = 0x43e;
// Mask say that double number is NaN by IEEE 754
constexpr uint32_t UP_BITS_NAN_DOUBLE = 0x7ff;

constexpr uint32_t SHIFT_BITS_DOUBLE = 12;
constexpr uint32_t SHIFT_BITS_FLOAT = 9;

// Return true, if architecture can be encoded.
#ifdef PANDA_WITH_CODEGEN
bool BackendSupport(Arch arch);
#else
inline bool BackendSupport([[maybe_unused]] Arch arch)
{
    return false;
}
#endif

//  Arch-independent access types

/**
 * Template class for identify types compile-time (nortti - can't use typeid).
 * Used in register class. Immediate class support conversion to it.
 */
class TypeInfo final {
public:
    enum TypeId : uint8_t { INT8 = 0, INT16 = 1, INT32 = 2, INT64 = 3, FLOAT32 = 4, FLOAT64 = 5, INVALID = 6 };

    /// Template constructor - use template parameter for create object.
    template <class T>
    constexpr explicit TypeInfo(T /* unused */)
    {
#ifndef __clang_analyzer__
        static_assert(std::is_arithmetic_v<T>);
        if constexpr (std::is_same<T, uint8_t>()) {
            typeId_ = INT8;
        } else if constexpr (std::is_same<T, int8_t>()) {
            typeId_ = INT8;
        } else if constexpr (std::is_same<T, uint16_t>()) {
            typeId_ = INT16;
        } else if constexpr (std::is_same<T, int16_t>()) {
            typeId_ = INT16;
        } else if constexpr (std::is_same<T, uint32_t>()) {
            typeId_ = INT32;
        } else if constexpr (std::is_same<T, int32_t>()) {
            typeId_ = INT32;
        } else if constexpr (std::is_same<T, uint64_t>()) {
            typeId_ = INT64;
        } else if constexpr (std::is_same<T, int64_t>()) {
            typeId_ = INT64;
        } else if constexpr (std::is_same<T, float>()) {
            typeId_ = FLOAT32;
        } else if constexpr (std::is_same<T, double>()) {
            typeId_ = FLOAT64;
        } else {
            typeId_ = INVALID;
        }
#endif
    }

    constexpr explicit TypeInfo(TypeId type) : typeId_(type) {}

    DEFAULT_MOVE_SEMANTIC(TypeInfo);
    DEFAULT_COPY_SEMANTIC(TypeInfo);
    ~TypeInfo() = default;

    /// Constructor for create invalid TypeInfo
    constexpr TypeInfo() = default;

    /// Validation check
    constexpr bool IsValid() const
    {
        return typeId_ != INVALID;
    }

    /// Type expected size
    constexpr size_t GetSize() const
    {
        ASSERT(IsValid());
        switch (typeId_) {
            case INT8:
                return BYTE_SIZE;
            case INT16:
                return HALF_SIZE;
            case INT32:
            case FLOAT32:
                return WORD_SIZE;
            case INT64:
            case FLOAT64:
                return DOUBLE_WORD_SIZE;
            default:
                return 0;
        }
        return 0;
    }

    constexpr bool IsFloat() const
    {
        ASSERT(IsValid());
        return typeId_ == FLOAT32 || typeId_ == FLOAT64;
    }

    constexpr bool IsScalar() const
    {
        // VOID - is scalar type here
        return !IsFloat();
    }

    constexpr bool operator==(const TypeInfo &other) const
    {
        return (typeId_ == other.typeId_);
    }

    constexpr bool operator!=(const TypeInfo &other) const
    {
        return !operator==(other);
    }

    static TypeInfo FromDataType(DataType::Type type, Arch arch)
    {
        switch (type) {
            case DataType::BOOL:
            case DataType::UINT8:
            case DataType::INT8: {
                return TypeInfo(INT8);
            }
            case DataType::UINT16:
            case DataType::INT16: {
                return TypeInfo(INT16);
            }
            case DataType::UINT32:
            case DataType::INT32: {
                return TypeInfo(INT32);
            }
            case DataType::UINT64:
            case DataType::INT64:
            case DataType::ANY: {
                return TypeInfo(INT64);
            }
            case DataType::FLOAT32: {
                return TypeInfo(FLOAT32);
            }
            case DataType::FLOAT64: {
                return TypeInfo(FLOAT64);
            }
            case DataType::REFERENCE: {
                return FromDataType(DataType::GetIntTypeForReference(arch), arch);
            }
            case DataType::POINTER: {
                return Is64BitsArch(arch) ? TypeInfo(INT64) : TypeInfo(INT32);
            }
            default:
                UNREACHABLE();
        }
    }

    DataType::Type ToDataType() const
    {
        switch (typeId_) {
            case INT8:
                return DataType::INT8;
            case INT16:
                return DataType::INT16;
            case INT32:
                return DataType::INT32;
            case INT64:
                return DataType::INT64;
            case FLOAT32:
                return DataType::FLOAT32;
            case FLOAT64:
                return DataType::FLOAT64;
            default:
                UNREACHABLE();
        }
    }

    static constexpr TypeInfo GetScalarTypeBySize(size_t sizeBits);

    void Dump()
    {
        std::cerr << "TypeInfo:";
        switch (typeId_) {
            case INT8:
                std::cerr << "INT8";
                break;
            case INT16:
                std::cerr << "INT16";
                break;
            case INT32:
                std::cerr << "INT32";
                break;
            case FLOAT32:
                std::cerr << "FLOAT32";
                break;
            case INT64:
                std::cerr << "INT64";
                break;
            case FLOAT64:
                std::cerr << "FLOAT64";
                break;
            default:
                std::cerr << "INVALID";
                break;
        }
        std::cerr << ", size = " << GetSize();
    }

private:
    TypeId typeId_ {INVALID};
};

constexpr TypeInfo INT8_TYPE {TypeInfo::INT8};
constexpr TypeInfo INT16_TYPE {TypeInfo::INT16};
constexpr TypeInfo INT32_TYPE {TypeInfo::INT32};
constexpr TypeInfo INT64_TYPE {TypeInfo::INT64};
constexpr TypeInfo FLOAT32_TYPE {TypeInfo::FLOAT32};
constexpr TypeInfo FLOAT64_TYPE {TypeInfo::FLOAT64};
constexpr TypeInfo INVALID_TYPE;

constexpr TypeInfo TypeInfo::GetScalarTypeBySize(size_t sizeBits)
{
    auto type = INT64_TYPE;
    if (sizeBits == BYTE_SIZE) {
        type = INT8_TYPE;
    } else if (sizeBits == HALF_SIZE) {
        type = INT16_TYPE;
    } else if (sizeBits == WORD_SIZE) {
        type = INT32_TYPE;
    }
    return type;
}

// Condition also used for tell comparison registers type
enum Condition {
    EQ,  // equal to 0
    NE,  // not equal to 0
    // signed
    LT,  // less
    LE,  // less than or equal
    GT,  // greater
    GE,  // greater than or equal
    // unsigned - checked from registers
    LO,  // less
    LS,  // less than or equal
    HI,  // greater
    HS,  // greater than or equal
    // Special arch-dependecy NOTE (igorban) Fix them
    MI,  // N set            Negative
    PL,  // N clear          Positive or zero
    VS,  // V set            Overflow.
    VC,  // V clear          No overflow.
    AL,  //                  Always.
    NV,  // Behaves as always/al.

    TST_EQ,
    TST_NE,

    INVALID_COND
};

inline bool IsTestCc(Condition cond)
{
    return cond == TST_EQ || cond == TST_NE;
}

}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_CODEGEN_TYPE_INFO_H
