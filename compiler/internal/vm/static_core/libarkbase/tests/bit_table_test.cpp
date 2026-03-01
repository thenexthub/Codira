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

#include "libarkbase/utils/bit_table.h"
#include "libarkbase/mem/pool_manager.h"

#include <gtest/gtest.h>

namespace ark::test {

// NOLINTBEGIN(readability-magic-numbers)

class BitTableTest : public ::testing::Test {
public:
    BitTableTest()
    {
        ark::mem::MemConfig::Initialize(0U, 64_MB, 256_MB, 32_MB, 0U, 0U);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~BitTableTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(BitTableTest);
    NO_MOVE_SEMANTIC(BitTableTest);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

template <typename C, size_t N>
typename C::Entry CreateEntry(std::array<uint32_t, N> data)
{
    typename C::Entry entry;
    static_assert(N == C::NUM_COLUMNS);
    for (size_t i = 0; i < N; i++) {
        entry[i] = data[i];
    }
    return entry;
}

TEST_F(BitTableTest, EmptyTable)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    BitTableBuilder<BitTableDefault<1U>> builder(GetAllocator());
    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<1U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 0U);
    ASSERT_EQ(table.begin(), table.end());
}

TEST_F(BitTableTest, SingleNoValue)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    BitTableBuilder<BitTableDefault<1U>> builder(GetAllocator());
    using Builder = decltype(builder);
    builder.Emplace(Builder::Entry({Builder::NO_VALUE}));
    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<1U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 1U);
    ASSERT_FALSE(table.GetRow(0U).Has(0U));
    ASSERT_EQ(table.GetRow(0U).Get(0U), Builder::NO_VALUE);
    ASSERT_TRUE(
        std::all_of(table.begin(), table.end(), [](const auto &row) { return row.Get(0) == Builder::NO_VALUE; }));
}

TEST_F(BitTableTest, SingleColumn)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    BitTableBuilder<BitTableDefault<1U>> builder(GetAllocator());
    using Builder = decltype(builder);
    builder.Emplace(Builder::Entry({9U}));
    builder.Emplace(Builder::Entry({Builder::NO_VALUE}));
    builder.Emplace(Builder::Entry({19U}));
    builder.Emplace(Builder::Entry({29U}));

    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<1U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 4U);
    ASSERT_EQ(table.GetRow(0U).Get(0U), 9U);
    ASSERT_FALSE(table.GetRow(1U).Has(0U));
    ASSERT_EQ(table.GetRow(2U).Get(0U), 19U);
    ASSERT_EQ(table.GetRow(3U).Get(0U), 29U);
}

TEST_F(BitTableTest, MultipleColumns)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    std::array<std::array<uint32_t, 10U>, 5U> values = {
        {{0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U},
         {10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U},
         {0_KB, 1_KB + 1U, 1_KB + 2U, 1_KB + 3U, 1_KB + 4U, 1_KB + 5U, 1_KB + 6U, 1_KB + 7U, 1_KB + 8U, 1_KB + 9U},
         {0_MB, 0_MB + 1U, 1_MB + 2U, 1_MB + 3U, 1_MB + 4U, 1_MB + 5U, 1_MB + 6U, 1_MB + 7U, 1_MB + 8U, 1_MB + 9U},
         {0_GB, 0_GB + 1U, 0_GB + 2U, 1_GB + 3U, 1_GB + 4U, 1_GB + 5U, 1_GB + 6U, 1_GB + 7U, 1_GB + 8U, 1_GB + 9U}}};

    BitTableBuilder<BitTableDefault<10U>> builder(GetAllocator());
    builder.Emplace(CreateEntry<decltype(builder)>(values[0U]));
    builder.Emplace(CreateEntry<decltype(builder)>(values[1U]));
    builder.Emplace(CreateEntry<decltype(builder)>(values[2U]));
    builder.Emplace(CreateEntry<decltype(builder)>(values[3U]));
    builder.Emplace(CreateEntry<decltype(builder)>(values[4U]));

    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<10U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 5U);

    size_t rowIndex = 0;
    for (auto row : table) {
        for (size_t i = 0; i < row.ColumnsCount(); i++) {
            ASSERT_EQ(row.Get(i), values[rowIndex][i]);
        }
        rowIndex++;
    }
}

class TestAccessor : public BitTableRow<2U, TestAccessor> {
public:
    using Base = BitTableRow<2U, TestAccessor>;
    using Base::Base;

    static_assert(Base::NUM_COLUMNS == 2U);

    uint32_t GetField0()
    {
        return Base::Get(0U);
    }
    uint32_t GetField1()
    {
        return Base::Get(1U);
    }
    const char *GetName(size_t index)
    {
        ASSERT(index < Base::ColumnsCount());
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static constexpr const char *NAMES[] = {"field0", "field1"};
        return NAMES[index];
    }

    enum { FIELD0, FIELD1 };
};

TEST_F(BitTableTest, CustomAccessor)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    using Builder = BitTableBuilder<TestAccessor>;

    Builder builder(GetAllocator());
    {
        Builder::Entry entry;
        entry[TestAccessor::FIELD0] = 1U;
        entry[TestAccessor::FIELD1] = 2U;
        builder.Emplace(entry);
    }
    {
        Builder::Entry entry;
        entry[TestAccessor::FIELD0] = 3U;
        entry[TestAccessor::FIELD1] = 4U;
        builder.Emplace(entry);
    }

    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<TestAccessor> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 2U);
    ASSERT_EQ(table.GetRow(0U).GetField0(), 1U);
    ASSERT_EQ(table.GetRow(0U).GetField1(), 2U);
    ASSERT_EQ(table.GetRow(1U).GetField0(), 3U);
    ASSERT_EQ(table.GetRow(1U).GetField1(), 4U);

    int idx = 1;
    for (auto &row : table) {
        for (size_t i = 0; i < row.ColumnsCount(); i++) {
            ASSERT_EQ(row.Get(i), idx++);
        }
    }
}
void GetRange(BitTable<TestAccessor> table, std::array<std::array<uint32_t, 2U>, 10U> values)
{
    auto range = table.GetRangeReversed(4U, 10U);
    ASSERT_EQ(range[0U].GetField0(), values[9U][0U]);
    ASSERT_EQ(range[0U].GetField1(), values[9U][1U]);
    ASSERT_EQ(range[1U].GetField0(), values[8U][0U]);
    ASSERT_EQ(range[1U].GetField1(), values[8U][1U]);
    ASSERT_EQ(range[2U].GetField0(), values[7U][0U]);
    ASSERT_EQ(range[2U].GetField1(), values[7U][1U]);
    ASSERT_EQ(range[3U].GetField0(), values[6U][0U]);
    ASSERT_EQ(range[3U].GetField1(), values[6U][1U]);
    ASSERT_EQ(range[4U].GetField0(), values[5U][0U]);
    ASSERT_EQ(range[4U].GetField1(), values[5U][1U]);

    int i = 10U;
    for (auto &v : table.GetRangeReversed(4U, 10U)) {
        ASSERT_EQ(v.GetField0(), values[i - 1L][0U]);
        ASSERT_EQ(v.GetField1(), values[i - 1L][1U]);
        i--;
    }
    ASSERT_EQ(i, 4U);

    i = 10U;
    for (auto &v : table.GetRangeReversed()) {
        ASSERT_EQ(v.GetField0(), values[i - 1L][0U]);
        ASSERT_EQ(v.GetField1(), values[i - 1L][1U]);
        i--;
    }
    ASSERT_EQ(i, 0U);
}

void GetReversedRange(BitTable<TestAccessor> table, std::array<std::array<uint32_t, 2U>, 10U> values)
{
    auto range = table.GetRangeReversed(4U, 10U);
    ASSERT_EQ(range[0U].GetField0(), values[9U][0U]);
    ASSERT_EQ(range[0U].GetField1(), values[9U][1U]);
    ASSERT_EQ(range[1U].GetField0(), values[8U][0U]);
    ASSERT_EQ(range[1U].GetField1(), values[8U][1U]);
    ASSERT_EQ(range[2U].GetField0(), values[7U][0U]);
    ASSERT_EQ(range[2U].GetField1(), values[7U][1U]);
    ASSERT_EQ(range[3U].GetField0(), values[6U][0U]);
    ASSERT_EQ(range[3U].GetField1(), values[6U][1U]);
    ASSERT_EQ(range[4U].GetField0(), values[5U][0U]);
    ASSERT_EQ(range[4U].GetField1(), values[5U][1U]);

    int i = 10U;
    for (auto &v : table.GetRangeReversed(4U, 10U)) {
        ASSERT_EQ(v.GetField0(), values[i - 1L][0U]);
        ASSERT_EQ(v.GetField1(), values[i - 1L][1U]);
        i--;
    }
    ASSERT_EQ(i, 4U);

    i = 10U;
    for (auto &v : table.GetRangeReversed()) {
        ASSERT_EQ(v.GetField0(), values[i - 1L][0U]);
        ASSERT_EQ(v.GetField1(), values[i - 1L][1U]);
        i--;
    }
    ASSERT_EQ(i, 0U);
}

TEST_F(BitTableTest, Ranges)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    std::array<std::array<uint32_t, 2U>, 10U> values = {
        {{0U, 10U}, {1U, 11U}, {2U, 12U}, {3U, 13U}, {4U, 14U}, {5U, 15U}, {6U, 16U}, {7U, 17U}, {8U, 18U}, {9U, 19U}}};

    BitTableBuilder<TestAccessor> builder(GetAllocator());
    for (auto &v : values) {
        builder.Emplace(CreateEntry<decltype(builder)>(v));
    }

    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<TestAccessor> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 10U);
    ASSERT_EQ(table.GetColumnsCount(), 2U);

    {
        GetRange(table, values);
    }

    {
        GetReversedRange(table, values);
    }
}

TEST_F(BitTableTest, GetRanges)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    std::array<std::array<uint32_t, 2U>, 10U> values = {
        {{0U, 10U}, {1U, 11U}, {2U, 12U}, {3U, 13U}, {4U, 14U}, {5U, 15U}, {6U, 16U}, {7U, 17U}, {8U, 18U}, {9U, 19U}}};

    BitTableBuilder<TestAccessor> builder(GetAllocator());
    for (auto &v : values) {
        builder.Emplace(CreateEntry<decltype(builder)>(v));
    }

    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<TestAccessor> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 10U);
    ASSERT_EQ(table.GetColumnsCount(), 2U);

    {
        auto range = table.GetRange(0U, 0U);
        ASSERT_EQ(range.begin(), range.end());

        size_t i = 0;
        for ([[maybe_unused]] auto &v : range) {
            i++;
        }
        ASSERT_EQ(i, 0U);
    }

    {
        auto range = table.GetRangeReversed(0U, 0U);
        ASSERT_EQ(range.begin(), range.end());

        size_t i = 0;
        for ([[maybe_unused]] auto &v : range) {
            i++;
        }
        ASSERT_EQ(i, 0U);
    }
}

TEST_F(BitTableTest, Deduplication)
{
    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(1_KB);

    using Builder = BitTableBuilder<TestAccessor>;
    using Entry = Builder::Entry;

    Builder builder(GetAllocator());

    std::array<Entry, 3U> values = {Entry {1U}, Entry {2U}, Entry {3U}};

    ASSERT_EQ(0U, builder.Add(values[0U]));
    ASSERT_EQ(1U, builder.Add(values[1U]));
    ASSERT_EQ(0U, builder.Add(values[0U]));
    ASSERT_EQ(2U, builder.Add(values[2U]));
    ASSERT_EQ(1U, builder.Add(values[1U]));
    ASSERT_EQ(2U, builder.Add(values[2U]));

    ASSERT_EQ(3U, builder.AddArray(Span<Entry>(values.begin(), 2U)));
    ASSERT_EQ(1U, builder.AddArray(Span<Entry>(values.begin() + 1U, 1U)));
    ASSERT_EQ(5U, builder.AddArray(Span<Entry>(values.begin() + 1U, 2U)));
    ASSERT_EQ(3U, builder.AddArray(Span<Entry>(values.begin(), 2U)));
    ASSERT_EQ(5U, builder.AddArray(Span<Entry>(values.begin() + 1U, 2U)));
}

TEST_F(BitTableTest, Bitmap)
{
    uint64_t pattern = 0xBADDCAFEDEADF00DULL;

    BitmapTableBuilder builder(GetAllocator());

    ArenaVector<std::pair<uint32_t, uint64_t>> values(GetAllocator()->Adapter());
    for (size_t i = 0; i <= 64U; i++) {
        uint64_t mask = (i == 64U) ? std::numeric_limits<uint64_t>::max() : ((1ULL << i) - 1L);
        uint64_t value = pattern & mask;
        BitVector<ArenaAllocator> vec(MinimumBitsToStore(value), GetAllocator());
        vec.Reset();
        for (size_t j = 0; j < i; j++) {
            if ((value & (1ULL << j)) != 0U) {
                vec.SetBit(j);
            }
        }
        auto fixedVec = vec.GetFixed();
        values.push_back({builder.Add(fixedVec), value});
    }

    // Zero value also occupies row in the table
    ASSERT_EQ(Popcount(pattern), builder.GetRowsCount());

    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(10_KB);
    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<1U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowSizeInBits(), MinimumBitsToStore(pattern));

    for (auto &[index, value] : values) {
        if (index != BitTableDefault<1U>::NO_VALUE) {
            ASSERT_EQ(table.GetBitMemoryRegion(index).Read<uint64_t>(0U, table.GetRowSizeInBits()), value);
        }
    }
}

template <typename T>
void FillVector(T &vector, uint32_t value)
{
    auto sp = vector.GetDataSpan();
    std::fill(sp.begin(), sp.end(), value);
}

TEST_F(BitTableTest, BitmapDeduplication)
{
    BitmapTableBuilder builder(GetAllocator());
    std::array<uint32_t, 128U> buff {};
    std::array fixedVectors = {ArenaBitVectorSpan(&buff[0U], 23U), ArenaBitVectorSpan(&buff[1U], 48U),
                               ArenaBitVectorSpan(&buff[3U], 0U), ArenaBitVectorSpan(&buff[4U], 123U),
                               ArenaBitVectorSpan(&buff[8U], 48U)};
    std::array vectors = {ArenaBitVector(GetAllocator()), ArenaBitVector(GetAllocator()),
                          ArenaBitVector(GetAllocator()), ArenaBitVector(GetAllocator()),
                          ArenaBitVector(GetAllocator())};
    FillVector(fixedVectors[0U], 0x23232323U);
    FillVector(fixedVectors[1U], 0x48484848U);
    FillVector(fixedVectors[2U], 0U);
    FillVector(fixedVectors[3U], 0x23123123U);
    FillVector(fixedVectors[4U], 0x48484848U);
    ASSERT_EQ(fixedVectors[1U], fixedVectors[4U]);
    vectors[0U].resize(1U);
    vectors[1U].resize(23U);
    vectors[2U].resize(123U);
    vectors[3U].resize(234U);
    vectors[4U].resize(0U);
    FillVector(vectors[0U], 0x1U);
    FillVector(vectors[1U], 0x11111111U);
    FillVector(vectors[2U], 0x23123123U);
    FillVector(vectors[3U], 0x34234234U);
    ASSERT_EQ(builder.Add(fixedVectors[0U].GetFixed()), 0U);
    ASSERT_EQ(builder.Add(fixedVectors[1U].GetFixed()), 1U);
    ASSERT_EQ(builder.Add(fixedVectors[2U].GetFixed()), BitTableDefault<1U>::NO_VALUE);
    ASSERT_EQ(builder.Add(fixedVectors[3U].GetFixed()), 2U);
    ASSERT_EQ(builder.Add(fixedVectors[4U].GetFixed()), 1U);
    ASSERT_EQ(builder.Add(vectors[0U].GetFixed()), 3U);
    ASSERT_EQ(builder.Add(vectors[1U].GetFixed()), 4U);
    ASSERT_EQ(builder.Add(vectors[2U].GetFixed()), 2U);
    ASSERT_EQ(builder.Add(vectors[3U].GetFixed()), 5U);
    ASSERT_EQ(builder.Add(vectors[4U].GetFixed()), BitTableDefault<1U>::NO_VALUE);

    ArenaVector<uint8_t> data(GetAllocator()->Adapter());
    data.reserve(10_KB);
    BitMemoryStreamOut out(&data);
    builder.Encode(out);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    BitTable<BitTableDefault<1U>> table;
    table.Decode(&in);

    ASSERT_EQ(table.GetRowsCount(), 6U);
    ASSERT_EQ(table.GetRowSizeInBits(), 234U);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
