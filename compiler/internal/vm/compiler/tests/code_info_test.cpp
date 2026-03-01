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

#include <fstream>
#include "unit_test.h"
#include "code_info/code_info.h"
#include "code_info/code_info_builder.h"
#include "mem/pool_manager.h"

using panda::panda_file::File;

namespace panda::compiler {

class CodeInfoTest : public ::testing::Test {
public:
    CodeInfoTest()
    {
        panda::mem::MemConfig::Initialize(0, 64_MB, 256_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    virtual ~CodeInfoTest()
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    auto EmitCode(CodeInfoBuilder &builder)
    {
        static constexpr size_t DUMMY_CODE_SIZE = 16;
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        size_t code_offset = CodeInfo::GetCodeOffset(RUNTIME_ARCH) + DUMMY_CODE_SIZE;
        data.resize(code_offset);
        builder.Encode(&data, data.size() * BITS_PER_BYTE);
        CodePrefix *prefix = new (data.data()) CodePrefix;
        prefix->code_size = data.size();
        prefix->code_info_offset = code_offset;
        prefix->code_info_size = data.size() - code_offset;
        return data;
    }

    template <typename Callback>
    void EnumerateVRegs(const CodeInfo &code_info, const StackMap &stack_map, int inline_depth, Callback callback)
    {
        auto list = code_info.GetVRegList(stack_map, inline_depth, GetAllocator());
        for (auto vreg : list) {
            callback(vreg);
        }
    }

    template <size_t N>
    void CompareVRegs(CodeInfo &code_info, StackMap stack_map, int inline_info_index, std::array<VRegInfo, N> vregs)
    {
        std::vector<VRegInfo> vregs_in_map;
        if (inline_info_index == -1) {
            EnumerateVRegs(code_info, stack_map, -1, [&vregs_in_map](auto vreg) { vregs_in_map.push_back(vreg); });
        } else {
            EnumerateVRegs(code_info, stack_map, inline_info_index,
                           [&vregs_in_map](auto vreg) { vregs_in_map.push_back(vreg); });
        }
        ASSERT_EQ(vregs_in_map.size(), vregs.size());
        for (size_t i = 0; i < vregs.size(); i++) {
            vregs_in_map[i].SetIndex(0);
            ASSERT_EQ(vregs_in_map[i], vregs[i]);
        }
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

TEST_F(CodeInfoTest, SingleStackmap)
{
    auto regs_count = GetCalleeRegsCount(RUNTIME_ARCH, false) + GetCalleeRegsCount(RUNTIME_ARCH, true) +
                      GetCallerRegsCount(RUNTIME_ARCH, false) + GetCallerRegsCount(RUNTIME_ARCH, true);
    std::array vregs = {VRegInfo(1, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::INT64, false),
                        VRegInfo(2, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, false),
                        VRegInfo(12, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, false)};
    if constexpr (!ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {  // NOLINT
        vregs[1].SetValue((vregs[1].GetValue() << 1) + 1 + regs_count);
    } else {  // NOLINT
        vregs[1].SetValue(vregs[1].GetValue() + regs_count);
    }
    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(1, 3);
    ArenaBitVector stack_roots(GetAllocator());
    stack_roots.resize(3);
    stack_roots.SetBit(2);
    std::bitset<32> reg_roots(0);
    reg_roots.set(12);
    builder.BeginStackMap(10, 20, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[0]);
    builder.AddVReg(vregs[1]);
    builder.AddVReg(vregs[2]);
    builder.EndStackMap();
    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    BitMemoryStreamIn in(data.data(), 0, data.size() * BITS_PER_BYTE);
    CodeInfo code_info(data.data());
    ASSERT_EQ(code_info.GetHeader().GetFrameSize(), 1);
    ASSERT_EQ(code_info.GetStackMaps().GetRowsCount(), 1);
    auto stack_map = code_info.GetStackMaps().GetRow(0);
    ASSERT_EQ(stack_map.GetBytecodePc(), 10);
    ASSERT_EQ(stack_map.GetNativePcUnpacked(), 20);
    ASSERT_FALSE(stack_map.HasInlineInfoIndex());
    ASSERT_TRUE(stack_map.HasRootsRegMaskIndex());
    ASSERT_TRUE(stack_map.HasRootsStackMaskIndex());
    ASSERT_TRUE(stack_map.HasVRegMaskIndex());
    ASSERT_TRUE(stack_map.HasVRegMapIndex());

    ASSERT_EQ(Popcount(code_info.GetRootsRegMask(stack_map)), 1);
    ASSERT_EQ(code_info.GetRootsRegMask(stack_map), 1 << 12);

    ASSERT_EQ(code_info.GetRootsStackMask(stack_map).Popcount(), 1);
    auto roots_region = code_info.GetRootsStackMask(stack_map);
    ASSERT_EQ(roots_region.Size(), 3);
    ASSERT_EQ(code_info.GetRootsStackMask(stack_map).Read(0, 3), 1 << 2);

    ASSERT_EQ(code_info.GetVRegMask(stack_map).Popcount(), 3);
    ASSERT_EQ(code_info.GetVRegMask(stack_map).Size(), 3);

    size_t index = 0;
    EnumerateVRegs(code_info, stack_map, -1, [&vregs, &index](auto vreg) {
        vreg.SetIndex(0);
        ASSERT_EQ(vreg, vregs[index++]);
    });

    std::bitset<vregs.size()> mask(0);
    code_info.EnumerateStaticRoots(stack_map, [&mask, &vregs](auto vreg) -> bool {
        auto it = std::find(vregs.begin(), vregs.end(), vreg);
        if (it != vregs.end()) {
            ASSERT(std::distance(vregs.begin(), it) < helpers::ToSigned(mask.size()));
            mask.set(std::distance(vregs.begin(), it));
        }
        return true;
    });
    ASSERT_EQ(Popcount(mask.to_ullong()), 2);
    ASSERT_TRUE(mask.test(1));
    ASSERT_TRUE(mask.test(2));
}

TEST_F(CodeInfoTest, MultipleStackmaps)
{
    uintptr_t method_stub;
    std::array vregs = {
        VRegInfo(1, VRegInfo::Location::REGISTER, VRegInfo::Type::INT64, false),
        VRegInfo(2, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, false),
        VRegInfo(3, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, false),
        VRegInfo(10, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::FLOAT64, false),
        VRegInfo(20, VRegInfo::Location::SLOT, VRegInfo::Type::BOOL, false),
        VRegInfo(30, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, false),
    };
    ArenaBitVector stack_roots(GetAllocator());
    stack_roots.resize(8);
    std::bitset<32> reg_roots(0);

    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(12, 3);

    stack_roots.SetBit(1);
    stack_roots.SetBit(2);
    builder.BeginStackMap(10, 20, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[0]);
    builder.AddVReg(vregs[1]);
    builder.AddVReg(VRegInfo());
    builder.BeginInlineInfo(&method_stub, 0, 1, 2);
    builder.AddVReg(vregs[2]);
    builder.AddVReg(vregs[3]);
    builder.EndInlineInfo();
    builder.BeginInlineInfo(nullptr, 0x123456, 2, 1);
    builder.AddVReg(vregs[3]);
    builder.EndInlineInfo();
    builder.EndStackMap();

    stack_roots.Reset();
    reg_roots.reset();
    reg_roots.set(5);
    builder.BeginStackMap(30, 40, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[3]);
    builder.AddVReg(vregs[5]);
    builder.AddVReg(vregs[4]);
    builder.BeginInlineInfo(nullptr, 0xabcdef, 3, 2);
    builder.AddVReg(vregs[1]);
    builder.AddVReg(VRegInfo());
    builder.EndInlineInfo();
    builder.EndStackMap();

    stack_roots.Reset();
    reg_roots.reset();
    reg_roots.set(1);
    builder.BeginStackMap(50, 60, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.EndStackMap();

    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    CodeInfo code_info(data.data());

    {
        auto stack_map = code_info.GetStackMaps().GetRow(0);
        ASSERT_EQ(stack_map.GetBytecodePc(), 10);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 20);
        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        CompareVRegs(code_info, stack_map, -1, std::array {vregs[0], vregs[1], VRegInfo()});

        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 2);
        auto it = inline_infos.begin();
        ASSERT_EQ(std::get<void *>(code_info.GetMethod(stack_map, 0)), &method_stub);
        ASSERT_TRUE(it->Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(code_info, stack_map, it->GetRow() - stack_map.GetInlineInfoIndex(), std::array {vregs[3]});
        ++it;
        ASSERT_FALSE(it->Get(InlineInfo::COLUMN_IS_LAST));
        ASSERT_EQ(std::get<uint32_t>(code_info.GetMethod(stack_map, 1)), 0x123456);
        CompareVRegs(code_info, stack_map, it->GetRow() - stack_map.GetInlineInfoIndex(),
                     std::array {vregs[2], vregs[3]});
    }
    {
        auto stack_map = code_info.GetStackMaps().GetRow(1);
        ASSERT_EQ(stack_map.GetBytecodePc(), 30);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 40);
        CompareVRegs(code_info, stack_map, -1, std::array {vregs[3], vregs[5], vregs[4]});

        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 1);
        ASSERT_EQ(std::get<uint32_t>(code_info.GetMethod(stack_map, 0)), 0xabcdef);
        ASSERT_TRUE(inline_infos[0].Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(code_info, stack_map, inline_infos[0].GetRow() - stack_map.GetInlineInfoIndex(),
                     std::array {vregs[1], VRegInfo()});
    }

    {
        auto stack_map = code_info.GetStackMaps().GetRow(2);
        ASSERT_EQ(stack_map.GetBytecodePc(), 50);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 60);
        CompareVRegs(code_info, stack_map, -1, std::array {VRegInfo(), VRegInfo(), VRegInfo()});

        ASSERT_FALSE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 0);
    }

    {
        auto stackmap = code_info.FindStackMapForNativePc(20);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 20);
        ASSERT_EQ(stackmap.GetBytecodePc(), 10);
        stackmap = code_info.FindStackMapForNativePc(40);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 40);
        ASSERT_EQ(stackmap.GetBytecodePc(), 30);
        stackmap = code_info.FindStackMapForNativePc(60);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 60);
        ASSERT_EQ(stackmap.GetBytecodePc(), 50);
        stackmap = code_info.FindStackMapForNativePc(90);
        ASSERT_FALSE(stackmap.IsValid());
    }
}

TEST_F(CodeInfoTest, Constants)
{
    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(12, 3);
    builder.BeginStackMap(1, 4, nullptr, 0, true, false);
    builder.AddConstant(0, VRegInfo::Type::INT64, false);
    builder.AddConstant(0x1234567890abcdef, VRegInfo::Type::INT64, true);
    builder.AddConstant(0x12345678, VRegInfo::Type::INT32, false);
    builder.EndStackMap();
    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    CodeInfo code_info(data.data());

    auto stack_map = code_info.GetStackMaps().GetRow(0);

    auto vreg_list = code_info.GetVRegList(stack_map, GetAllocator());
    ASSERT_EQ(code_info.GetConstant(vreg_list[0]), 0);
    ASSERT_EQ(code_info.GetConstant(vreg_list[1]), 0x1234567890abcdef);
    ASSERT_EQ(code_info.GetConstant(vreg_list[2]), 0x12345678);
    // 0 and 0x12345678 should be deduplicated
    ASSERT_EQ(code_info.GetConstantTable().GetRowsCount(), 3);
}
}  // namespace panda::compiler
