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

#ifndef PANDA_CODE_INFO_H
#define PANDA_CODE_INFO_H

#include "code_info_tables.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/bit_table.h"
#include "libarkbase/utils/cframe_layout.h"
#include "libarkbase/utils/small_vector.h"
#include "libarkbase/utils/span.h"

namespace ark::compiler {

/*
 *
 * Compiled code layout:
 * +-------------+
 * | CodePrefix  |
 * +-------------+ <- Method::CompiledCodeEntrypoint
 * | Code        |
 * +-------------+-----------------+
 * | CodeInfo    | CodeInfoHeader  |
 * |             |-----------------+----------------------+
 * |             |                 |  StackMap            |
 * |             |                 |  InlineInfo          |
 * |             |                 |  Roots Reg Mask      |
 * |             |                 |  Roots Stack Mask    |
 * |             |   Bit Tables    |  Method indexes      |
 * |             |                 |  VRegs mask          |
 * |             |                 |  VRegs map           |
 * |             |                 |  VRegs catalogue     |
 * |             |                 |  Implicit Nullchecks |
 * |             |                 |  Constants           |
 * |-------------+-----------------+----------------------+
 */

struct CodePrefix {
    static constexpr uint32_t MAGIC = 0xaccadeca;
    uint32_t magic {MAGIC};
    uint32_t codeSize {};
    uint32_t codeInfoOffset {};
    uint32_t codeInfoSize {};

    static constexpr size_t STRUCT_SIZE = 16;
};

static_assert(sizeof(CodePrefix) == CodePrefix::STRUCT_SIZE);

class CodeInfoHeader {
public:
    enum Elements { PROPERTIES, CALLEE_REG_MASK, CALLEE_FP_REG_MASK, TABLE_MASK, VREGS_COUNT, SIZE };

    void SetFrameSize(uint32_t size)
    {
        ASSERT(MinimumBitsToStore(size) <= FRAME_SIZE_FIELD_WIDTH);
        FieldFrameSize::Set(size, &data_[PROPERTIES]);
    }
    uint32_t GetFrameSize() const
    {
        return FieldFrameSize::Get(data_[PROPERTIES]);
    }

    void SetCalleeRegMask(uint32_t value)
    {
        data_[CALLEE_REG_MASK] = value;
    }
    uint32_t GetCalleeRegMask() const
    {
        return data_[CALLEE_REG_MASK];
    }

    void SetCalleeFpRegMask(uint32_t value)
    {
        data_[CALLEE_FP_REG_MASK] = value;
    }
    uint32_t GetCalleeFpRegMask() const
    {
        return data_[CALLEE_FP_REG_MASK];
    }

    void SetTableMask(uint32_t value)
    {
        data_[TABLE_MASK] = value;
    }
    uint32_t GetTableMask() const
    {
        return data_[TABLE_MASK];
    }

    void SetVRegsCount(uint32_t value)
    {
        data_[VREGS_COUNT] = value;
    }
    uint32_t GetVRegsCount() const
    {
        return data_[VREGS_COUNT];
    }

    void SetHasFloatRegs(bool value)
    {
        HasFloatRegsFlag::Set(value, &data_[PROPERTIES]);
    }
    bool HasFloatRegs() const
    {
        return HasFloatRegsFlag::Get(data_[PROPERTIES]);
    }

    template <typename Container>
    void Encode(BitMemoryStreamOut<Container> &out)
    {
        VarintPack::Write(out, data_);
    }
    void Decode(BitMemoryStreamIn *in)
    {
        data_ = VarintPack::Read<SIZE>(in);
    }

private:
    std::array<uint32_t, SIZE> data_;

    static constexpr size_t FRAME_SIZE_FIELD_WIDTH = 16;
    static constexpr size_t LANG_EXT_OFFSET_FIELD_WIDTH = 13;
    using FieldFrameSize = BitField<uint32_t, 0, FRAME_SIZE_FIELD_WIDTH>;
    using HasFloatRegsFlag = FieldFrameSize::NextFlag;
};

class CodeInfo final {
public:
    static constexpr size_t TABLES_COUNT = 10;
    static constexpr size_t VREG_LIST_STATIC_SIZE = 16;
    static constexpr size_t ALIGNMENT = sizeof(uint64_t);
    static constexpr size_t SIZE_ALIGNMENT = sizeof(uint64_t);

    template <typename Allocator>
    using VRegList = SmallVector<VRegInfo, VREG_LIST_STATIC_SIZE, Allocator, true>;
    using VRegNumberPair = std::pair<uint32_t *, uint32_t *>;
    using RegionType = BitMemoryRegion<const uint8_t>;

    NO_COPY_SEMANTIC(CodeInfo);
    NO_MOVE_SEMANTIC(CodeInfo);

    CodeInfo() = default;

    CodeInfo(const void *data, size_t size)
        : CodeInfo(Span<const uint8_t>(reinterpret_cast<const uint8_t *>(data), size))
    {
    }

    explicit CodeInfo(Span<const uint8_t> code) : CodeInfo(code.data())
    {
        ASSERT(GetDataSize() <= code.size());
    }

    explicit CodeInfo(Span<uint8_t> code) : CodeInfo(code.data())
    {
        ASSERT(GetDataSize() <= code.size());
    }

    explicit CodeInfo(const void *codeEntry)
    {
        ASSERT(codeEntry != nullptr);
        auto prefix = reinterpret_cast<const CodePrefix *>(codeEntry);
        ASSERT(prefix->magic == CodePrefix::MAGIC);
        data_ = Span(reinterpret_cast<const uint8_t *>(codeEntry), prefix->codeInfoOffset + prefix->codeInfoSize);
        auto codeInfo = Span<const uint8_t>(&data_[prefix->codeInfoOffset], prefix->codeInfoSize);
        Decode(codeInfo);
    }

    virtual ~CodeInfo() = default;

    static const void *GetCodeOriginFromEntryPoint(const void *data)
    {
        return reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(data) -
                                              CodeInfo::GetCodeOffset(RUNTIME_ARCH));
    }

    static CodeInfo CreateFromCodeEntryPoint(const void *data)
    {
        ASSERT(data != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return CodeInfo(reinterpret_cast<const uint8_t *>(data) - CodeInfo::GetCodeOffset(RUNTIME_ARCH));
    }

    void Decode(Span<const uint8_t> codeInfo)
    {
        BitMemoryStreamIn stream(const_cast<uint8_t *>(codeInfo.data()), codeInfo.size() * BITS_PER_BYTE);
        header_.Decode(&stream);
        EnumerateTables([this, &stream](size_t index, auto member) {
            if (HasTable(index)) {
                (this->*member).Decode(&stream);
            }
        });
    }

    const CodeInfoHeader &GetHeader() const
    {
        return header_;
    }
    CodeInfoHeader &GetHeader()
    {
        return header_;
    }

    const CodePrefix *GetPrefix() const
    {
        return reinterpret_cast<const CodePrefix *>(data_.data());
    }

    uint32_t GetFrameSize() const
    {
        return GetHeader().GetFrameSize();
    }

    const uint8_t *GetData()
    {
        return data_.data();
    }

    size_t GetDataSize()
    {
        return data_.size();
    }

    const uint8_t *GetCode() const
    {
        return &data_[CodeInfo::GetCodeOffset(RUNTIME_ARCH)];
    }

    size_t GetCodeSize() const
    {
        return GetPrefix()->codeSize;
    }

    Span<const uint8_t> GetCodeSpan() const
    {
        return {&data_[CodeInfo::GetCodeOffset(RUNTIME_ARCH)], GetCodeSize()};
    }

    size_t GetInfoSize() const
    {
        return GetPrefix()->codeInfoSize;
    }

    bool HasTable(size_t index) const
    {
        return (GetHeader().GetTableMask() & (1U << index)) != 0;
    }

    std::variant<void *, uint32_t> GetMethod(const StackMap &stackmap, int inlineDepth)
    {
        ASSERT(inlineDepth >= 0);
        auto inlineInfo = inlineInfos_.GetRow(stackmap.GetInlineInfoIndex() + inlineDepth);
        if (inlineInfo.HasMethodLow()) {
            if constexpr (ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {
                uintptr_t val =
                    inlineInfo.GetMethodLow() | (static_cast<uint64_t>(inlineInfo.GetMethodHi()) << BITS_PER_UINT32);
                return reinterpret_cast<void *>(val);
            } else {
                return reinterpret_cast<void *>(inlineInfo.GetMethodLow());
            }
        }
        return methodIds_.GetRow(inlineInfo.GetMethodIdIndex()).GetId();
    }

    uint64_t GetConstant(const VRegInfo &vreg) const
    {
        ASSERT(vreg.GetLocation() == VRegInfo::Location::CONSTANT);
        uint64_t low = constantTable_.GetRow(vreg.GetConstantLowIndex()).GetValue();
        uint64_t hi = constantTable_.GetRow(vreg.GetConstantHiIndex()).GetValue();
        return low | (hi << BITS_PER_UINT32);
    }

    static size_t GetCodeOffset(Arch arch)
    {
        return RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(arch));
    }

    uint32_t GetSavedCalleeRegsMask(bool isFp) const
    {
        return isFp ? GetHeader().GetCalleeFpRegMask() : GetHeader().GetCalleeRegMask();
    }

    auto GetVRegMask(const StackMap &stackMap)
    {
        return stackMap.HasVRegMaskIndex() ? vregMasks_.GetBitMemoryRegion(stackMap.GetVRegMaskIndex())
                                           : BitMemoryRegion<const uint8_t>();
    }

    auto GetVRegMask(const StackMap &stackMap) const
    {
        return const_cast<CodeInfo *>(this)->GetVRegMask(stackMap);
    }

    size_t GetVRegCount(const StackMap &stackMap) const
    {
        return GetVRegMask(stackMap).Popcount();
    }

    uint32_t GetRootsRegMask(const StackMap &stackMap) const
    {
        return stackMap.HasRootsRegMaskIndex() ? rootsRegMasks_.GetRow(stackMap.GetRootsRegMaskIndex()).GetMask() : 0;
    }

    auto GetRootsStackMask(const StackMap &stackMap) const
    {
        return stackMap.HasRootsStackMaskIndex()
                   ? rootsStackMasks_.GetBitMemoryRegion(stackMap.GetRootsStackMaskIndex())
                   : BitMemoryRegion<const uint8_t>();
    }

    auto GetInlineInfos(const StackMap &stackMap)
    {
        if (!stackMap.HasInlineInfoIndex()) {
            return inlineInfos_.GetRangeReversed(0, 0);
        }
        auto index = stackMap.GetInlineInfoIndex();
        uint32_t size = index;
        for (; inlineInfos_.GetRow(size).GetIsLast() == 0; size++) {
        }

        return inlineInfos_.GetRangeReversed(index, helpers::ToSigned(size) + 1);
    }

    auto GetInlineInfo(const StackMap &stackMap, int inlineDepth) const
    {
        ASSERT(stackMap.HasInlineInfoIndex());
        CHECK_GE(GetInlineDepth(stackMap), inlineDepth);
        return inlineInfos_.GetRow(stackMap.GetInlineInfoIndex() + inlineDepth);
    }

    int GetInlineDepth(const StackMap &stackMap) const
    {
        if (!stackMap.HasInlineInfoIndex()) {
            return -1;
        }
        int index = stackMap.GetInlineInfoIndex();
        int depth = index;
        for (; inlineInfos_.GetRow(depth).GetIsLast() == 0; depth++) {
        }
        return depth - index;
    }

    StackMap FindStackMapForNativePc(uint32_t pc, Arch arch = RUNTIME_ARCH) const
    {
        auto it = std::lower_bound(stackMaps_.begin(), stackMaps_.end(), pc, [arch](const auto &a, uintptr_t counter) {
            return a.GetNativePcUnpacked(arch) < counter;
        });
        return (it == stackMaps_.end() || it->GetNativePcUnpacked(arch) != pc) ? stackMaps_.GetInvalidRow() : *it;
    }

    StackMap FindOsrStackMap(uint32_t pc) const
    {
        auto it = std::find_if(stackMaps_.begin(), stackMaps_.end(),
                               [pc](const auto &a) { return a.GetBytecodePc() == pc && a.IsOsr(); });
        return it == stackMaps_.end() ? stackMaps_.GetInvalidRow() : *it;
    }

    auto GetStackMap(size_t index) const
    {
        return StackMap(&stackMaps_, index);
    }

    auto &GetStackMaps()
    {
        return stackMaps_;
    }

    auto &GetVRegCatalogue()
    {
        return vregsCatalogue_;
    }

    auto &GetVRegMapTable()
    {
        return vregsMap_;
    }

    auto &GetVRegMaskTable()
    {
        return vregMasks_;
    }

    auto &GetInlineInfosTable()
    {
        return inlineInfos_;
    }

    auto &GetConstantTable()
    {
        return constantTable_;
    }

    const auto &GetImplicitNullChecksTable() const
    {
        return implicitNullchecks_;
    }

    bool HasFloatRegs() const
    {
        return GetHeader().HasFloatRegs();
    }

    template <typename Func>
    static void EnumerateTables(Func func)
    {
        size_t index = 0;
        func(index++, &CodeInfo::stackMaps_);
        func(index++, &CodeInfo::inlineInfos_);
        func(index++, &CodeInfo::rootsRegMasks_);
        func(index++, &CodeInfo::rootsStackMasks_);
        func(index++, &CodeInfo::methodIds_);
        func(index++, &CodeInfo::vregMasks_);
        func(index++, &CodeInfo::vregsMap_);
        func(index++, &CodeInfo::vregsCatalogue_);
        func(index++, &CodeInfo::implicitNullchecks_);
        func(index++, &CodeInfo::constantTable_);
        ASSERT(index == TABLES_COUNT);
    }

    template <typename Callback>
    void EnumerateStaticRoots(const StackMap &stackMap, Callback callback)
    {
        return EnumerateRoots<Callback, false>(stackMap, callback);
    }

    template <typename Callback>
    void EnumerateDynamicRoots(const StackMap &stackMap, Callback callback)
    {
        return EnumerateRoots<Callback, true>(stackMap, callback);
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stackMap, uint32_t firstVreg, uint32_t vregsCount,
                                    Allocator *allocator = nullptr) const
    {
        if (vregsCount == 0 || !stackMap.HasRegMap()) {
            return CodeInfo::VRegList<Allocator>(allocator);
        }
        VRegList<Allocator> vregList(allocator);
        vregList.resize(vregsCount, VRegInfo());
        ASSERT(!vregList[0].IsLive());
        std::vector<bool> regSet(vregsCount);

        uint32_t remainingRegisters = vregsCount;
        for (int sindex = static_cast<int64_t>(stackMap.GetRow()); sindex >= 0 && remainingRegisters > 0; sindex--) {
            stackMap = GetStackMap(sindex);
            if (!stackMap.HasVRegMaskIndex()) {
                continue;
            }
            // Skip stackmaps that are not in the same inline depth
            auto vregMask = GetVRegMask(stackMap);
            if (vregMask.Size() <= firstVreg) {
                continue;
            }
            ASSERT(stackMap.HasVRegMapIndex());
            uint32_t mapIndex = stackMap.GetVRegMapIndex();

            mapIndex += vregMask.Popcount(0, firstVreg);
            vregMask = vregMask.Subregion(firstVreg, vregMask.Size() - firstVreg);

            FillVRegList<Allocator>(vregMask, vregList, regSet, {&vregsCount, &remainingRegisters}, &mapIndex);
        }
        return vregList;
    }

    template <typename Allocator>
    void FillVRegList(RegionType &vregMask, VRegList<Allocator> &vregList, std::vector<bool> &regSet,
                      VRegNumberPair vregPair, uint32_t *mapIndex) const
    {
        auto [vregsCount, remainingRegisters] = vregPair;
        uint32_t end = std::min<uint32_t>(vregMask.Size(), *vregsCount);
        for (size_t i = 0; i < end; i += BITS_PER_UINT32) {
            uint32_t mask = vregMask.Read(i, std::min<uint32_t>(end - i, BITS_PER_UINT32));
            while (mask != 0) {
                auto regIdx = static_cast<size_t>(Ctz(mask));
                if (regSet[i + regIdx]) {
                    (*mapIndex)++;
                    mask ^= 1U << regIdx;
                    continue;
                }
                auto vregIndex = vregsMap_.GetRow(*mapIndex);
                if (vregIndex.GetIndex() != StackMap::NO_VALUE) {
                    ASSERT(!vregList[i + regIdx].IsLive());
                    vregList[i + regIdx] = vregsCatalogue_.GetRow(vregIndex.GetIndex()).GetVRegInfo();
                    vregList[i + regIdx].SetIndex(i + regIdx);
                }
                (*remainingRegisters)--;
                regSet[i + regIdx] = true;
                (*mapIndex)++;
                mask ^= 1U << regIdx;
            }
        }
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stackMap, int inlineDepth, Allocator *allocator = nullptr) const
    {
        if (inlineDepth < 0) {
            return GetVRegList<Allocator>(stackMap, 0, GetHeader().GetVRegsCount(), allocator);
        }
        ASSERT(stackMap.HasInlineInfoIndex());
        auto inlineInfo = GetInlineInfo(stackMap, inlineDepth);
        if (inlineInfo.GetVRegsCount() == 0) {
            return VRegList<Allocator>(allocator);
        }
        auto depth = inlineInfo.GetRow() - stackMap.GetInlineInfoIndex();
        uint32_t first =
            depth == 0 ? GetHeader().GetVRegsCount() : inlineInfos_.GetRow(inlineInfo.GetRow() - 1).GetVRegsCount();
        ASSERT(inlineInfo.GetVRegsCount() >= first);
        return GetVRegList<Allocator>(stackMap, first, inlineInfo.GetVRegsCount() - first, allocator);
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stackMap, Allocator *allocator = nullptr) const
    {
        return GetVRegList<Allocator>(stackMap, -1, allocator);
    }

    static bool VerifyCompiledEntry(uintptr_t compiledEntry)
    {
        auto codeheader = compiledEntry - GetCodeOffset(RUNTIME_ARCH);
        return (*reinterpret_cast<const uint32_t *>(codeheader) == CodePrefix::MAGIC);
    }

    void Dump(std::ostream &stream) const;

    void Dump(std::ostream &stream, const StackMap &stackMap, Arch arch = RUNTIME_ARCH) const;

    void DumpInlineInfo(std::ostream &stream, const StackMap &stackMap, int depth) const;

    size_t CountSpillSlots()
    {
        size_t frameSlots = GetFrameSize() / PointerSize(RUNTIME_ARCH);
        static_assert(CFrameSlots::Start() >= 0);
        size_t spillsCount = frameSlots - (static_cast<size_t>(CFrameSlots::Start()) + GetRegsCount(RUNTIME_ARCH) + 1U);
        // Reverse 'CFrameLayout::AlignSpillCount' counting
        if (RUNTIME_ARCH == Arch::AARCH32) {
            spillsCount = spillsCount / 2U - 1;
        }
        if (spillsCount % 2U != 0) {
            spillsCount--;
        }
        return spillsCount;
    }

private:
    template <typename Callback, bool IS_DYNAMIC>
    void EnumerateRoots(const StackMap &stackMap, Callback callback);

    BitTable<StackMap> stackMaps_;
    BitTable<InlineInfo> inlineInfos_;
    BitTable<RegisterMask> rootsRegMasks_;
    BitTable<StackMask> rootsStackMasks_;
    BitTable<MethodId> methodIds_;
    BitTable<VRegisterInfo> vregsCatalogue_;
    BitTable<VRegisterCatalogueIndex> vregsMap_;
    BitTable<VRegisterMask> vregMasks_;
    BitTable<ImplicitNullChecks> implicitNullchecks_;
    BitTable<ConstantTable> constantTable_;

    CodeInfoHeader header_ {};

    Span<const uint8_t> data_;
};

template <typename Callback, bool IS_DYNAMIC>
void CodeInfo::EnumerateRoots(const StackMap &stackMap, Callback callback)
{
    auto rootType = IS_DYNAMIC ? VRegInfo::Type::ANY : VRegInfo::Type::OBJECT;

    if (stackMap.HasRootsRegMaskIndex()) {
        auto regMask = rootsRegMasks_.GetRow(stackMap.GetRootsRegMaskIndex()).GetMask();
        ArenaBitVectorSpan vec(&regMask, BITS_PER_UINT32);
        for (auto regIdx : vec.GetSetBitsIndices()) {
            if (!callback(VRegInfo(regIdx, VRegInfo::Location::REGISTER, rootType, VRegInfo::VRegType::VREG))) {
                return;
            }
        }
    }
    if (!stackMap.HasRootsStackMaskIndex()) {
        return;
    }
    // Simplify after renumbering stack slots
    auto stackSlotsCount = CountSpillSlots();
    auto regMask = rootsStackMasks_.GetBitMemoryRegion(stackMap.GetRootsStackMaskIndex());
    for (auto regIdx : regMask) {
        if (regIdx >= stackSlotsCount) {
            // Parameter-slots' indexes are added to the root-mask with `stack_slots_count` offset to distinct them
            // from spill-slots
            auto paramSlotIdx = regIdx - stackSlotsCount;
            regIdx = static_cast<size_t>(CFrameLayout::StackArgSlot::Start()) - paramSlotIdx -
                     static_cast<size_t>(CFrameSlots::Start());
        } else {
            if constexpr (!ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {  // NOLINT
                regIdx = (regIdx << 1U) + 1;
            }
            // Stack roots are began from spill/fill stack origin, so we need to adjust it according to registers
            // buffer
            regIdx += GetRegsCount(RUNTIME_ARCH);
        }
        VRegInfo vreg(regIdx, VRegInfo::Location::SLOT, rootType, VRegInfo::VRegType::VREG);
        if (!callback(vreg)) {
            return;
        }
    }
}

}  // namespace ark::compiler

#endif  // PANDA_CODE_INFO_H
