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

#ifndef PANDA_CODE_INFO_TABLES_H
#define PANDA_CODE_INFO_TABLES_H

#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/bit_table.h"
#include "libarkbase/utils/arch.h"
#include "vreg_info.h"

namespace ark::compiler {

class StackMap : public BitTableRow<8U, StackMap> {
public:
    BIT_TABLE_HEADER(8, StackMap);
    BIT_TABLE_COLUMN(0, Properties, PROPERTIES)
    BIT_TABLE_COLUMN(1, NativePc, NATIVE_PC)
    BIT_TABLE_COLUMN(2, BytecodePc, BYTECODE_PC)
    BIT_TABLE_COLUMN(3, RootsRegMaskIndex, ROOTS_REG_MASK_INDEX)
    BIT_TABLE_COLUMN(4, RootsStackMaskIndex, ROOTS_STACK_MASK_INDEX)
    BIT_TABLE_COLUMN(5, InlineInfoIndex, INLINE_INFO_INDEX)
    BIT_TABLE_COLUMN(6, VRegMaskIndex, VREG_MASK_INDEX)
    BIT_TABLE_COLUMN(7, VRegMapIndex, VREG_MAP_INDEX)

    DEFAULT_MOVE_SEMANTIC(StackMap);
    DEFAULT_COPY_SEMANTIC(StackMap);
    ~StackMap() = default;

    std::string GetColumnStr(size_t column) const
    {
        if (column != COLUMN_NATIVE_PC) {
            return Base::GetColumnStr(column);
        }
        if (Get(column) == NO_VALUE) {
            return "-";
        }
        return std::to_string(GetNativePcUnpacked());
    }

    static constexpr uintptr_t PackAddress(uintptr_t address, Arch arch)
    {
        ASSERT(IsAligned(address, GetInstructionAlignment(arch)));
        return address / GetInstructionAlignment(arch);
    }

    uint32_t GetNativePcUnpacked(Arch arch = RUNTIME_ARCH) const
    {
        return UnpackAddress(GetNativePc(), arch);
    }

    static constexpr uintptr_t UnpackAddress(uintptr_t address, Arch arch)
    {
        return address * GetInstructionAlignment(arch);
    }

    static constexpr uint32_t CreateProperties(bool isOsr, bool hasRegmap)
    {
        return FieldIsOsr::Encode(isOsr) | FieldHasRegMap::Encode(hasRegmap);
    }

    bool IsOsr() const
    {
        return FieldIsOsr::Get(GetProperties());
    }

    bool HasRegMap() const
    {
        return FieldHasRegMap::Get(GetProperties());
    }

private:
    using FieldIsOsr = BitField<bool, 0, 1>;
    using FieldHasRegMap = FieldIsOsr::NextFlag;
};

class InlineInfo : public BitTableRow<7U, InlineInfo> {
public:
    BIT_TABLE_HEADER(7, InlineInfo);
    BIT_TABLE_COLUMN(0, IsLast, IS_LAST)
    BIT_TABLE_COLUMN(1, BytecodePc, BYTECODE_PC)
    BIT_TABLE_COLUMN(2, MethodIdIndex, METHOD_ID_INDEX)
    BIT_TABLE_COLUMN(3, MethodHi, METHOD_HI)
    BIT_TABLE_COLUMN(4, MethodLow, METHOD_LOW)
    BIT_TABLE_COLUMN(5, VRegsCount, VREGS_COUNT)
    BIT_TABLE_COLUMN(6, FileIndex, FILE_INDEX)

    DEFAULT_MOVE_SEMANTIC(InlineInfo);
    DEFAULT_COPY_SEMANTIC(InlineInfo);
    ~InlineInfo() = default;
};

class RegisterMask : public BitTableRow<1, RegisterMask> {
public:
    BIT_TABLE_HEADER(1, RegisterMask);
    BIT_TABLE_COLUMN(0, Mask, MASK)

    DEFAULT_MOVE_SEMANTIC(RegisterMask);
    DEFAULT_COPY_SEMANTIC(RegisterMask);
    ~RegisterMask() = default;
};

class StackMask : public BitTableRow<1, StackMask> {
public:
    BIT_TABLE_HEADER(1, StackMask);
    BIT_TABLE_COLUMN(0, Mask, MASK)

    DEFAULT_MOVE_SEMANTIC(StackMask);
    DEFAULT_COPY_SEMANTIC(StackMask);
    ~StackMask() = default;
};

class VRegisterMask : public BitTableRow<1, VRegisterMask> {
public:
    BIT_TABLE_HEADER(1, VRegisterMask);
    BIT_TABLE_COLUMN(0, Mask, MASK)

    DEFAULT_MOVE_SEMANTIC(VRegisterMask);
    DEFAULT_COPY_SEMANTIC(VRegisterMask);
    ~VRegisterMask() = default;
};

class MethodId : public BitTableRow<1, MethodId> {
public:
    BIT_TABLE_HEADER(1, MethodId);
    BIT_TABLE_COLUMN(0, Id, ID)

    DEFAULT_MOVE_SEMANTIC(MethodId);
    DEFAULT_COPY_SEMANTIC(MethodId);
    ~MethodId() = default;
};

class VRegisterCatalogueIndex : public BitTableRow<1, VRegisterCatalogueIndex> {
public:
    BIT_TABLE_HEADER(1, VRegisterCatalogueIndex);
    BIT_TABLE_COLUMN(0, Index, INDEX)

    DEFAULT_MOVE_SEMANTIC(VRegisterCatalogueIndex);
    DEFAULT_COPY_SEMANTIC(VRegisterCatalogueIndex);
    ~VRegisterCatalogueIndex() = default;
};

class VRegisterInfo : public BitTableRow<2U, VRegisterInfo> {
public:
    BIT_TABLE_HEADER(2, VRegisterInfo);
    BIT_TABLE_COLUMN(0, Info, INFO)
    BIT_TABLE_COLUMN(1, Value, VALUE)

    DEFAULT_MOVE_SEMANTIC(VRegisterInfo);
    DEFAULT_COPY_SEMANTIC(VRegisterInfo);
    ~VRegisterInfo() = default;

    std::string GetColumnStr(size_t column) const
    {
        if (column != COLUMN_INFO || Get(column) == NO_VALUE) {
            return Base::GetColumnStr(column);
        }
        auto vreg = GetVRegInfo();
        return std::string(VRegInfo::VRegTypeToString(vreg.GetVRegType())) + ":" + vreg.GetTypeString() + ":" +
               vreg.GetLocationString();
    }

    VRegInfo GetVRegInfo() const
    {
        return VRegInfo(GetValue(), GetInfo());
    }

    uint32_t GetConstantLowIndex() const
    {
        ASSERT(GetVRegInfo().GetLocation() == VRegInfo::Location::CONSTANT);
        return GetValue() & ((1U << BITS_PER_UINT16) - 1);
    }

    uint32_t GetConstantHiIndex() const
    {
        ASSERT(GetVRegInfo().GetLocation() == VRegInfo::Location::CONSTANT);
        return (GetValue() >> BITS_PER_UINT16) & ((1U << BITS_PER_UINT16) - 1);
    }
};

class ImplicitNullChecks : public BitTableRow<2U, ImplicitNullChecks> {
public:
    BIT_TABLE_HEADER(2, ImplicitNullChecks);
    BIT_TABLE_COLUMN(0, InstNativePc, INST_NATIVE_PC)
    BIT_TABLE_COLUMN(1, Offset, OFFSET)

    DEFAULT_MOVE_SEMANTIC(ImplicitNullChecks);
    DEFAULT_COPY_SEMANTIC(ImplicitNullChecks);
    ~ImplicitNullChecks() = default;
};

class ConstantTable : public BitTableRow<1, ConstantTable> {
public:
    BIT_TABLE_HEADER(1, ConstantTable);
    BIT_TABLE_COLUMN(0, Value, VALUE)

    DEFAULT_MOVE_SEMANTIC(ConstantTable);
    DEFAULT_COPY_SEMANTIC(ConstantTable);
    ~ConstantTable() = default;
};

}  // namespace ark::compiler

#endif  // PANDA_CODE_INFO_TABLES_H
