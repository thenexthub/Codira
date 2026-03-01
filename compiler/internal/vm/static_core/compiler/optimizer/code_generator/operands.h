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

#ifndef COMPILER_OPTIMIZER_CODEGEN_OPERANDS_H
#define COMPILER_OPTIMIZER_CODEGEN_OPERANDS_H

/*
Arch-feature definitions
*/
#include <bitset>
#include <cstdint>
#include <type_traits>

#include "type_info.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/regmask.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/datatype.h"
#include "libarkbase/utils/type_helpers.h"

namespace ark::compiler {
// Mapping model for registers:
// reg-reg - support getters for small parts of registers
// reg-other - mapping between types of registers
enum RegMapping : uint32_t {
    SCALAR_SCALAR = 1UL << 0UL,
    SCALAR_VECTOR = 1UL << 1UL,
    SCALAR_FLOAT = 1UL << 2UL,
    VECTOR_VECTOR = 1UL << 3UL,
    VECTOR_FLOAT = 1UL << 4UL,
    FLOAT_FLOAT = 1UL << 5UL
};

constexpr uint8_t INVALID_REG_ID = std::numeric_limits<uint8_t>::max();
constexpr uint8_t ACC_REG_ID = INVALID_REG_ID - 1U;
#ifdef ENABLE_LIBABCKIT
inline Register GetAccReg()
{
    return IsFrameSizeLarge() ? INVALID_REG_LARGE - 1U : ACC_REG_ID;
}
#else
inline Register GetAccReg()
{
    return ACC_REG_ID;
}
#endif

class Reg final {
public:
    using RegIDType = uint8_t;
    using RegSizeType = size_t;

    constexpr Reg() = default;
    DEFAULT_MOVE_SEMANTIC(Reg);
    DEFAULT_COPY_SEMANTIC(Reg);
    ~Reg() = default;

    // Default register constructor
    constexpr Reg(RegIDType id, TypeInfo type) : id_(id), type_(type) {}

    constexpr RegIDType GetId() const
    {
        return id_;
    }

    constexpr size_t GetMask() const
    {
        CHECK_LT(id_, 32U);
        return (1U << id_);
    }

    constexpr TypeInfo GetType() const
    {
        return type_;
    }

    RegSizeType GetSize() const
    {
        return GetType().GetSize();
    }

    bool IsScalar() const
    {
        return GetType().IsScalar();
    }

    bool IsFloat() const
    {
        return GetType().IsFloat();
    }

    constexpr bool IsValid() const
    {
        return type_ != INVALID_TYPE && id_ != INVALID_REG_ID;
    }

    Reg As(TypeInfo type) const
    {
        return Reg(GetId(), type);
    }

    constexpr bool operator==(Reg other) const
    {
        return (GetId() == other.GetId()) && (GetType() == other.GetType());
    }

    constexpr bool operator!=(Reg other) const
    {
        return !operator==(other);
    }

    void Dump()
    {
        std::cerr << " Reg: id = " << static_cast<int64_t>(id_) << ", ";
        type_.Dump();
        std::cerr << "\n";
    }

private:
    RegIDType id_ {INVALID_REG_ID};
    TypeInfo type_ {INVALID_TYPE};
};  // Reg

constexpr Reg INVALID_REGISTER = Reg();

static_assert(!INVALID_REGISTER.IsValid());
static_assert(sizeof(Reg) <= sizeof(uintptr_t));

/**
 * Immediate class may hold only int or float values (maybe vectors in future).
 * It knows nothing about pointers and bools (bools maybe be in future).
 */
class Imm final {
    static constexpr size_t UNDEFINED_SIZE = 0;
    static constexpr size_t INT64_SIZE = 64;
    static constexpr size_t FLOAT32_SIZE = 32;
    static constexpr size_t FLOAT64_SIZE = 64;

public:
    constexpr Imm() = default;

    template <typename T>
    constexpr explicit Imm(T value) : value_(static_cast<int64_t>(value))
    {
        using Type = std::decay_t<T>;
        static_assert(std::is_integral_v<Type> || std::is_enum_v<Type>);
    }

    // Partial template specialization
    constexpr explicit Imm(int64_t value) : value_(value) {};
#ifndef NDEBUG
    constexpr explicit Imm(double value) : value_(value) {};
    constexpr explicit Imm(float value) : value_(value) {};
#else
    explicit Imm(double value) : value_(bit_cast<uint64_t>(value)) {};
    explicit Imm(float value) : value_(bit_cast<uint32_t>(value)) {};
#endif  // !NDEBUG

    DEFAULT_MOVE_SEMANTIC(Imm);
    DEFAULT_COPY_SEMANTIC(Imm);
    ~Imm() = default;

#ifdef NDEBUG
    constexpr int64_t GetAsInt() const
    {
        return value_;
    }

    float GetAsFloat() const
    {
        return bit_cast<float>(static_cast<int32_t>(value_));
    }

    double GetAsDouble() const
    {
        return bit_cast<double>(value_);
    }

    constexpr int64_t GetRawValue() const
    {
        return value_;
    }

#else
    constexpr int64_t GetAsInt() const
    {
        ASSERT(std::holds_alternative<int64_t>(value_));
        return std::get<int64_t>(value_);
    }

    float GetAsFloat() const
    {
        ASSERT(std::holds_alternative<float>(value_));
        return std::get<float>(value_);
    }

    double GetAsDouble() const
    {
        ASSERT(std::holds_alternative<double>(value_));
        return std::get<double>(value_);
    }

    constexpr int64_t GetRawValue() const
    {
        if (value_.index() == 0) {
            UNREACHABLE();
        } else if (value_.index() == 1) {
            return std::get<int64_t>(value_);
        } else if (value_.index() == 2U) {
            return static_cast<int64_t>(bit_cast<int32_t>(std::get<float>(value_)));
        } else if (value_.index() == 3U) {
            return bit_cast<int64_t>(std::get<double>(value_));
        }
        UNREACHABLE();
    }

    enum VariantID {
        V_INVALID = 0,  // Pointer used for invalidate variants
        V_INT64 = 1,
        V_FLOAT32 = 2,
        V_FLOAT64 = 3
    };

    template <class T>
    constexpr bool CheckVariantID() const
    {
#ifndef __clang_analyzer__
        // Immediate could be only signed (int/float)
        // look at value_-type.
        static_assert(std::is_signed_v<T>);
        if constexpr (std::is_same<T, int64_t>()) {
            return value_.index() == V_INT64;
        }
        if constexpr (std::is_same<T, float>()) {
            return value_.index() == V_FLOAT32;
        }
        if constexpr (std::is_same<T, double>()) {
            return value_.index() == V_FLOAT64;
        }
        return false;
#else
        return true;
#endif  // !__clang_analyzer__
    }

    constexpr bool IsValid() const
    {
        return !std::holds_alternative<void *>(value_);
    }

    TypeInfo GetType() const
    {
        switch (value_.index()) {
            case V_INT64:
                return INT64_TYPE;
            case V_FLOAT32:
                return FLOAT32_TYPE;
            case V_FLOAT64:
                return FLOAT64_TYPE;
            default:
                UNREACHABLE();
                return INVALID_TYPE;
        }
    }

    constexpr size_t GetSize() const
    {
        switch (value_.index()) {
            case V_INT64:
                return INT64_SIZE;
            case V_FLOAT32:
                return FLOAT32_SIZE;
            case V_FLOAT64:
                return FLOAT64_SIZE;
            default:
                UNREACHABLE();
                return UNDEFINED_SIZE;
        }
    }
#endif  // NDEBUG

    bool operator==(Imm other) const
    {
        return value_ == other.value_;
    }

    bool operator!=(Imm other) const
    {
        return !(operator==(other));
    }

private:
#ifndef NDEBUG
    std::variant<void *, int64_t, float, double> value_ {nullptr};
#else
    int64_t value_ {0};
#endif  // NDEBUG
};      // Imm

class TypedImm final {
public:
    template <typename T>
    constexpr explicit TypedImm(T imm) : type_(imm), imm_(imm)
    {
    }

    TypeInfo GetType() const
    {
        return type_;
    }

    Imm GetImm() const
    {
        return imm_;
    }

private:
    TypeInfo type_ {INVALID_TYPE};
    Imm imm_ {0};
};

// Why memory ref - because you may create one link for one encode-session
// And when you see this one - you can easy understand, what type of memory
//   you use. But if you load/store dirrectly address - you need to decode it
//   each time, when you read code
// model -> base + index<<scale + disp
class MemRef final {
public:
    MemRef() = default;

    explicit MemRef(Reg base) : MemRef(base, 0) {}
    MemRef(Reg base, ssize_t disp) : MemRef(base, INVALID_REGISTER, 0, disp) {}
    MemRef(Reg base, Reg index, uint16_t scale) : MemRef(base, index, scale, 0) {}
    MemRef(Reg base, Reg index, uint16_t scale, ssize_t disp) : disp_(disp), scale_(scale), base_(base), index_(index)
    {
        CHECK_LE(disp, std::numeric_limits<decltype(disp_)>::max());
        CHECK_LE(scale, std::numeric_limits<decltype(scale_)>::max());
    }
    DEFAULT_MOVE_SEMANTIC(MemRef);
    DEFAULT_COPY_SEMANTIC(MemRef);
    ~MemRef() = default;

    Reg GetBase() const
    {
        return base_;
    }
    Reg GetIndex() const
    {
        return index_;
    }
    auto GetScale() const
    {
        return scale_;
    }
    auto GetDisp() const
    {
        return disp_;
    }

    bool HasBase() const
    {
        return base_.IsValid();
    }
    bool HasIndex() const
    {
        return index_.IsValid();
    }
    bool HasScale() const
    {
        return HasIndex() && scale_ != 0;
    }
    bool HasDisp() const
    {
        return disp_ != 0;
    }
    // Ref must contain at least one of field
    bool IsValid() const
    {
        return HasBase() || HasIndex() || HasScale() || HasDisp();
    }

    // return true if mem doesn't has index and scalar
    bool IsOffsetMem() const
    {
        return !HasIndex() && !HasScale();
    }

    bool operator==(MemRef other) const
    {
        return (base_ == other.base_) && (index_ == other.index_) && (scale_ == other.scale_) && (disp_ == other.disp_);
    }
    bool operator!=(MemRef other) const
    {
        return !(operator==(other));
    }

private:
    ssize_t disp_ {0};
    uint16_t scale_ {0};
    Reg base_ {INVALID_REGISTER};
    Reg index_ {INVALID_REGISTER};
};  // MemRef

class Shift final {
public:
    explicit Shift(Reg base, ShiftType type, uint32_t scale) : scale_(scale), base_(base), type_(type) {}
    explicit Shift(Reg base, uint32_t scale) : Shift(base, ShiftType::LSL, scale) {}

    DEFAULT_MOVE_SEMANTIC(Shift);
    DEFAULT_COPY_SEMANTIC(Shift);
    ~Shift() = default;

    Reg GetBase() const
    {
        return base_;
    }

    ShiftType GetType() const
    {
        return type_;
    }

    uint32_t GetScale() const
    {
        return scale_;
    }

private:
    uint32_t scale_ {0};
    Reg base_;
    ShiftType type_ {INVALID_SHIFT};
};

}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_CODEGEN_REGISTERS_H
