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

#ifndef COMPILER_OPTIMIZER_IR_LOCATIONS_H
#define COMPILER_OPTIMIZER_IR_LOCATIONS_H

#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/datatype.h"
#include "utils/arena_containers.h"
#include "utils/bit_field.h"
#include "utils/arch.h"
#include "utils/regmask.h"

#include <ostream>

namespace panda::compiler {
class Inst;

// NOLINTNEXTLINE:e(cppcoreguidelines-macro-usage)
#define LOCATIONS(DEF)            \
    DEF(INVALID, "?")             \
    DEF(REGISTER, "r")            \
    DEF(FP_REGISTER, "vr")        \
    DEF(IMMEDIATE, "i")           \
    DEF(STACK, "s")               \
    DEF(STACK_PARAMETER, "param") \
    DEF(STACK_ARGUMENT, "arg")

class Location {
public:
    enum class Kind : uint8_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOCATION_DEF(name, short) name,
        LOCATIONS(LOCATION_DEF)
#undef LOCATION_DEF
            LAST = STACK_ARGUMENT,
        COUNT
    };

    Location() : Location(Kind::INVALID) {}
    explicit Location(Kind type, uintptr_t value = 0) : bit_fields_(KindField::Encode(type) | ValueField::Encode(value))
    {
        CHECK_LT(value, ValueField::Mask());
    }

    const char *GetName()
    {
        static constexpr std::array names = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOCATION_DEF(name, short_name) short_name,
            LOCATIONS(LOCATION_DEF)
#undef LOCATION_DEF
        };
        return names[static_cast<unsigned>(GetKind())];
    }

    bool operator==(Location rhs) const
    {
        return bit_fields_ == rhs.bit_fields_;
    }

    bool operator!=(Location rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(Location rhs) const
    {
        return bit_fields_ < rhs.bit_fields_;
    }

    bool IsInvalid() const
    {
        return GetKind() == Kind::INVALID;
    }

    bool IsConstant() const
    {
        return GetKind() == Kind::IMMEDIATE;
    }

    bool IsRegister() const
    {
        return GetKind() == Kind::REGISTER;
    }

    bool IsFpRegister() const
    {
        return GetKind() == Kind::FP_REGISTER;
    }

    bool IsStack() const
    {
        return GetKind() == Kind::STACK;
    }

    bool IsStackArgument() const
    {
        return GetKind() == Kind::STACK_ARGUMENT;
    }

    bool IsStackParameter() const
    {
        return GetKind() == Kind::STACK_PARAMETER;
    }

    bool IsAnyStack() const
    {
        return IsStack() || IsStackArgument() || IsStackParameter();
    }

    bool IsAnyRegister() const
    {
        return IsRegister() || IsFpRegister();
    }

    bool IsRegisterValid() const
    {
        ASSERT(IsRegister() || IsFpRegister());
        return GetValue() != INVALID_REG;
    }

    Kind GetKind() const
    {
        return KindField::Get(bit_fields_);
    }

    Register GetRegister() const
    {
        ASSERT(IsRegister());
        ASSERT(GetValue() < RegMask::Size());
        return GetValue();
    }
    Register GetFpRegister() const
    {
        ASSERT(IsFpRegister());
        ASSERT(GetValue() < VRegMask::Size());
        return GetValue();
    }

    bool IsUnallocatedRegister() const
    {
        return IsAnyRegister() && GetValue() == INVALID_REG;
    }

    bool IsFixedRegister() const
    {
        return IsAnyRegister() && GetValue() != INVALID_REG;
    }

    unsigned GetValue() const
    {
        return ValueField::Get(bit_fields_);
    }

    static Location MakeRegister(size_t id)
    {
        return Location(Kind::REGISTER, id);
    }

    static Location MakeFpRegister(size_t id)
    {
        return Location(Kind::FP_REGISTER, id);
    }

    static Location MakeRegister(size_t id, DataType::Type type)
    {
        return DataType::IsFloatType(type) ? MakeFpRegister(id) : MakeRegister(id);
    }

    static Location MakeStackSlot(size_t id)
    {
        return Location(Kind::STACK, id);
    }

    static Location MakeStackArgument(size_t id)
    {
        return Location(Kind::STACK_ARGUMENT, id);
    }

    static Location MakeStackParameter(size_t id)
    {
        return Location(Kind::STACK_PARAMETER, id);
    }

    static Location MakeConstant(size_t value)
    {
        return Location(Kind::IMMEDIATE, value);
    }

    static Location Invalid()
    {
        return Location();
    }

    static Location RequireRegister()
    {
        return Location(Kind::REGISTER, INVALID_REG);
    }

    void Dump(std::ostream &stm, Arch arch);
    std::string ToString(Arch arch);

private:
#ifdef IR_FOR_LIBARK_DEFECT_SCAN_AUX
    uint32_t bit_fields_;
#else
    uint16_t bit_fields_;
#endif

    using KindField = BitField<Kind, 0, MinimumBitsToStore(Kind::LAST)>;
    using ValueField =
        KindField::NextField<uintptr_t, sizeof(bit_fields_) * BITS_PER_BYTE - MinimumBitsToStore(Kind::LAST)>;
};

#ifndef IR_FOR_LIBARK_DEFECT_SCAN_AUX
static_assert(sizeof(Location) <= sizeof(uint16_t));
#endif

class LocationsInfo {
public:
    LocationsInfo(ArenaAllocator *allocator, Inst *inst);

    Location GetLocation(size_t index) const
    {
        ASSERT(index < locations_.size());
        return locations_[index];
    }

    Location GetDstLocation() const
    {
        return dst_location_;
    }

    void SetLocation(size_t index, Location location)
    {
        locations_[index] = location;
    }

    void SetDstLocation(Location location)
    {
        dst_location_ = location;
    }

private:
    Span<Location> locations_;
    Location dst_location_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_LOCATIONS_H
