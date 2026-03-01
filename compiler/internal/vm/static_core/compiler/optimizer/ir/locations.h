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

#ifndef PANDA_LOCATIONS_H
#define PANDA_LOCATIONS_H

#include "compiler/optimizer/code_generator/target_info.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/datatype.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/regmask.h"

#include <ostream>

namespace ark::compiler {
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
#define LOCATION_DEF(name, short) name, /* CC-OFF(G.PRE.02) list generation */
        LOCATIONS(LOCATION_DEF)
#undef LOCATION_DEF
            LAST = STACK_ARGUMENT,
        COUNT
    };

    Location() : Location(Kind::INVALID) {}
    explicit Location(Kind type, uintptr_t value = 0) : bitFields_(KindField::Encode(type) | ValueField::Encode(value))
    {
        CHECK_LT(value, ValueField::Mask());
    }

    const char *GetName()
    {
        static constexpr std::array NAMES = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOCATION_DEF(name, short_name) short_name, /* CC-OFF(G.PRE.02) list generation */
            LOCATIONS(LOCATION_DEF)
#undef LOCATION_DEF
        };
        return NAMES[static_cast<unsigned>(GetKind())];
    }

    bool operator==(Location rhs) const
    {
        return bitFields_ == rhs.bitFields_;
    }

    bool operator!=(Location rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(Location rhs) const
    {
        return bitFields_ < rhs.bitFields_;
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
        return GetValue() != GetInvalidReg();
    }

    Kind GetKind() const
    {
        return KindField::Get(bitFields_);
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
        return IsAnyRegister() && GetValue() == GetInvalidReg();
    }

    bool IsFixedRegister() const
    {
        return IsAnyRegister() && GetValue() != GetInvalidReg();
    }

    unsigned GetValue() const
    {
        return ValueField::Get(bitFields_);
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
        return Location(Kind::REGISTER, GetInvalidReg());
    }

    void Dump(std::ostream &stm, Arch arch);
    std::string ToString(Arch arch);

private:
    uint16_t bitFields_;

    using KindField = BitField<Kind, 0, MinimumBitsToStore(Kind::LAST)>;
    using ValueField =
        KindField::NextField<uintptr_t, sizeof(bitFields_) * BITS_PER_BYTE - MinimumBitsToStore(Kind::LAST)>;
};

static_assert(sizeof(Location) <= sizeof(uint16_t));

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
        return dstLocation_;
    }

    Location GetTmpLocation() const
    {
        return tmpLocation_;
    }

    void SetLocation(size_t index, Location location)
    {
        locations_[index] = location;
    }

    void SetDstLocation(Location location)
    {
        dstLocation_ = location;
    }

    void SetTmpLocation(Location location)
    {
        tmpLocation_ = location;
    }

private:
    Span<Location> locations_;
    Location dstLocation_;
    Location tmpLocation_;
};
}  // namespace ark::compiler

#endif  // PANDA_LOCATIONS_H
