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

#ifndef PANDA_COMPILER_CODE_INFO_VREG_INFO_H
#define PANDA_COMPILER_CODE_INFO_VREG_INFO_H

#include "libarkbase/utils/bit_field.h"

namespace ark::compiler {

class VRegInfo final {
public:
    enum class Location : int8_t { NONE, SLOT, REGISTER, FP_REGISTER, CONSTANT, COUNT = CONSTANT };

    static constexpr size_t INVALID_LOCATION = (1U << MinimumBitsToStore(static_cast<size_t>(Location::COUNT))) - 1;

    enum class Type : uint8_t { UNDEFINED, OBJECT, INT32, INT64, FLOAT32, FLOAT64, BOOL, ANY, COUNT = ANY };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VREGS_ENV_TYPE_DEFS(V) \
    V(THIS_FUNC)               \
    V(CONST_POOL)              \
    V(LEX_ENV)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VREG_ENV_TYPE(ENV_TYPE) ENV_TYPE,  // CC-OFF(G.PRE.02) list generation
    enum VRegType : uint8_t { VREG, ACC, VREGS_ENV_TYPE_DEFS(VREG_ENV_TYPE) COUNT, ENV_BEGIN = THIS_FUNC };
#undef VREG_ENV_TYPE

    static constexpr uint8_t ENV_COUNT = VRegType::COUNT - VRegType::ENV_BEGIN;

    VRegInfo()
    {
        FieldLocation::Set(Location::NONE, &info_);
        ASSERT(!IsLive());
    }
    VRegInfo(uint32_t value, VRegInfo::Location location, Type type, VRegType vregType)
        : value_(value),
          info_(FieldLocation::Encode(location) | FieldType::Encode(type) | FieldVRegType::Encode(vregType))
    {
    }
    VRegInfo(uint32_t value, VRegInfo::Location location, Type type, VRegType vregType, uint32_t index)
        : VRegInfo(value, location, type, vregType)
    {
        FieldVRegIndex::Set(index, &info_);
    }
    VRegInfo(uint32_t value, uint32_t packedInfo) : value_(value), info_(packedInfo) {}

    static VRegInfo Invalid()
    {
        return VRegInfo(0, static_cast<Location>(INVALID_LOCATION), Type::UNDEFINED, VRegType::VREG);
    }

    ~VRegInfo() = default;

    DEFAULT_COPY_SEMANTIC(VRegInfo);
    DEFAULT_MOVE_SEMANTIC(VRegInfo);

    uint32_t GetValue() const
    {
        return value_;
    }

    void SetValue(uint32_t value)
    {
        value_ = value;
    }

    Location GetLocation() const
    {
        return FieldLocation::Get(info_);
    }

    Type GetType() const
    {
        return FieldType::Get(info_);
    }

    uint16_t GetIndex() const
    {
        return FieldVRegIndex::Get(info_);
    }
    void SetIndex(uint16_t value)
    {
        FieldVRegIndex::Set(value, &info_);
    }

    VRegType GetVRegType() const
    {
        return FieldVRegType::Get(info_);
    }

    bool IsAccumulator() const
    {
        return FieldVRegType::Get(info_) == VRegType::ACC;
    }

    bool IsSpecialVReg() const
    {
        return GetVRegType() != VRegType::VREG;
    }

    bool IsLive() const
    {
        return GetLocation() != Location::NONE;
    }

    bool IsObject() const
    {
        return GetType() == Type::OBJECT;
    }

    bool IsFloat() const
    {
        return GetType() == Type::FLOAT32 || GetType() == Type::FLOAT64;
    }

    bool Has64BitValue() const
    {
        return GetType() == VRegInfo::Type::FLOAT64 || GetType() == VRegInfo::Type::INT64;
    }

    bool IsLocationRegister() const
    {
        auto location = GetLocation();
        return location == Location::REGISTER || location == Location::FP_REGISTER;
    }

    uint32_t GetConstantLowIndex() const
    {
        ASSERT(GetLocation() == Location::CONSTANT);
        return GetValue() & ((1U << BITS_PER_UINT16) - 1);
    }

    uint32_t GetConstantHiIndex() const
    {
        ASSERT(GetLocation() == Location::CONSTANT);
        return (GetValue() >> BITS_PER_UINT16) & ((1U << BITS_PER_UINT16) - 1);
    }

    void SetConstantIndices(uint16_t low, uint16_t hi)
    {
        value_ = low | (static_cast<uint32_t>(hi) << BITS_PER_UINT16);
    }

    bool operator==(const VRegInfo &rhs) const
    {
        return value_ == rhs.value_ && info_ == rhs.info_;
    }
    bool operator!=(const VRegInfo &rhs) const
    {
        return !(*this == rhs);
    }

    uint32_t GetInfo()
    {
        return info_;
    }

    const char *GetTypeString() const
    {
        switch (GetType()) {
            case Type::OBJECT:
                return "OBJECT";
            case Type::INT64:
                return "INT64";
            case Type::INT32:
                return "INT32";
            case Type::FLOAT32:
                return "FLOAT32";
            case Type::FLOAT64:
                return "FLOAT64";
            case Type::BOOL:
                return "BOOL";
            case Type::ANY:
                return "ANY";
            default:
                break;
        }
        UNREACHABLE();
    }

    const char *GetLocationString() const
    {
        switch (GetLocation()) {
            case Location::NONE:
                return "NONE";
            case Location::SLOT:
                return "SLOT";
            case Location::REGISTER:
                return "REGISTER";
            case Location::FP_REGISTER:
                return "FP_REGISTER";
            case Location::CONSTANT:
                return "CONSTANT";
            default:
                break;
        }
        UNREACHABLE();
    }

    static inline const char *VRegTypeToString(VRegType type)
    {
        static constexpr auto N = static_cast<uint8_t>(VRegType::COUNT);
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VREG_ENV_TYPE_TO_STR(ENV_TYPE) #ENV_TYPE,
        static constexpr std::array<const char *, N> VREG_TYPE_NAMES = {"VREG", "ACC",
                                                                        VREGS_ENV_TYPE_DEFS(VREG_ENV_TYPE_TO_STR)};
#undef VREG_ENV_TYPE_TO_STR
        auto idx = static_cast<uint8_t>(type);
        ASSERT(idx < N);
        return VREG_TYPE_NAMES[idx];
    }

    void Dump(std::ostream &os) const
    {
        os << "VReg #" << GetIndex() << ":" << GetTypeString() << ", " << VRegTypeToString(GetVRegType()) << ", "
           << GetLocationString() << "=" << helpers::ToSigned(GetValue());
    }

private:
    uint32_t value_ {0};
    uint32_t info_ {0};

    using FieldLocation = BitField<Location, 0, MinimumBitsToStore(static_cast<uint32_t>(Location::COUNT))>;
    using FieldType = FieldLocation::NextField<Type, MinimumBitsToStore(static_cast<uint32_t>(Type::COUNT))>;
    using FieldVRegType = FieldType::NextField<VRegType, MinimumBitsToStore(static_cast<uint32_t>(VRegType::COUNT))>;
    using FieldVRegIndex = FieldVRegType::NextField<uint16_t, BITS_PER_UINT16>;
};

static_assert(sizeof(VRegInfo) <= sizeof(uint64_t));

inline std::ostream &operator<<(std::ostream &os, const VRegInfo &vreg)
{
    vreg.Dump(os);
    return os;
}

}  // namespace ark::compiler

#endif  // PANDA_COMPILER_CODE_INFO_VREG_INFO_H
