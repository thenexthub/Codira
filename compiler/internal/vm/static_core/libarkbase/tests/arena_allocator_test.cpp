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

#include "libarkbase/mem/arena.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/arena_allocator_stl_adapter.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_config.h"

#include "libarkbase/utils/arena_containers.h"

#include "gtest/gtest.h"
#include "libarkbase/utils/logger.h"

#include <string>
#include <array>
#include <limits>
#include <cstdint>
#include <ctime>

namespace ark {

constexpr const int64_t DEFAULT_SEED = 123456;

class ArenaAllocatorTest : public testing::Test {
public:
    ArenaAllocatorTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        seed_ = DEFAULT_SEED;
#endif
    }

    ~ArenaAllocatorTest() override = default;

    NO_MOVE_SEMANTIC(ArenaAllocatorTest);
    NO_COPY_SEMANTIC(ArenaAllocatorTest);

protected:
    static constexpr size_t MIN_LOG_ALIGN_SIZE_T = static_cast<size_t>(LOG_ALIGN_MIN);
    static constexpr size_t MAX_LOG_ALIGN_SIZE_T = static_cast<size_t>(LOG_ALIGN_MAX);
    static constexpr size_t ARRAY_SIZE = 1024;

    template <class T>
    static constexpr T MaxValue()
    {
        return std::numeric_limits<T>::max();
    }

    static bool IsAligned(const void *ptr, size_t alignment)
    {
        return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
    }

    void SetUp() override
    {
        static constexpr size_t INTERNAL_SIZE = 128_MB;
        ark::mem::MemConfig::Initialize(0U, INTERNAL_SIZE, 0U, 0U, 0U, 0U);
        PoolManager::Initialize();
    }

    int GetRand() const
    {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        return rand();
    }

    template <class T>
    void AllocateWithAlignment() const
    {
        ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);

        for (Alignment align = LOG_ALIGN_MIN; align <= LOG_ALIGN_MAX;
             align = static_cast<Alignment>(static_cast<size_t>(align) + 1)) {
            std::array<T *, ARRAY_SIZE> arr {};

            size_t mask = GetAlignmentInBytes(align) - 1L;

            // Allocations
            srand(seed_);
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                arr[i] = static_cast<T *>(aa.Alloc(sizeof(T), align));
                *arr[i] = GetRand() % MaxValue<T>();
            }

            // Allocations checking
            srand(seed_);
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                ASSERT_NE(arr[i], nullptr) << "value of i: " << i << ", align: " << align;
                ASSERT_EQ(reinterpret_cast<size_t>(arr[i]) & mask, 0U) << "value of i: " << i << ", align: " << align;
                ASSERT_EQ(*arr[i], GetRand() % MaxValue<T>()) << "value of i: " << i << ", align: " << align;
            }
        }
    }

    template <class T>
    void AllocateWithDiffAlignment() const
    {
        ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);

        std::array<T *, ARRAY_SIZE> arr {};

        // Allocations
        srand(seed_);
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            auto randomValue = GetRand();
            size_t randAlign = MIN_LOG_ALIGN_SIZE_T + randomValue % (MAX_LOG_ALIGN_SIZE_T - MIN_LOG_ALIGN_SIZE_T);
            arr[i] = static_cast<T *>(aa.Alloc(sizeof(T), static_cast<Alignment>(randAlign)));
            *arr[i] = randomValue % MaxValue<T>();
        }

        // Allocations checking
        srand(seed_);
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            auto randomValue = GetRand();
            size_t align = MIN_LOG_ALIGN_SIZE_T + randomValue % (MAX_LOG_ALIGN_SIZE_T - MIN_LOG_ALIGN_SIZE_T);
            size_t mask = GetAlignmentInBytes(static_cast<Alignment>(align)) - 1L;

            ASSERT_NE(arr[i], nullptr);
            ASSERT_EQ(reinterpret_cast<size_t>(arr[i]) & mask, 0U) << "value of i: " << i << ", align: " << align;
            ASSERT_EQ(*arr[i], randomValue % MaxValue<T>()) << "value of i: " << i;
        }
    }

    void TearDown() override
    {
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    unsigned int GetSeed()
    {
        return seed_;
    }

private:
    unsigned int seed_;
};

class ComplexClass final {
public:
    ComplexClass() : value_(0U), strValue_("0") {}
    explicit ComplexClass(size_t value) : value_(value), strValue_(std::to_string(value_)) {}
    ComplexClass(size_t value, std::string strValue) : value_(value), strValue_(std::move(strValue)) {}
    ComplexClass(const ComplexClass &other) = default;
    ComplexClass(ComplexClass &&other) noexcept = default;

    ComplexClass &operator=(const ComplexClass &other) = default;
    ComplexClass &operator=(ComplexClass &&other) = default;

    size_t GetValue() const noexcept
    {
        return value_;
    }
    std::string GetString() const noexcept
    {
        return strValue_;
    }

    void SetValue(size_t value)
    {
        value_ = value;
        strValue_ = std::to_string(value);
    }

    ~ComplexClass() = default;

private:
    size_t value_;
    std::string strValue_;
};

TEST_F(ArenaAllocatorTest, AllocateTest)
{
    static constexpr size_t FIRST_ALLOC_SIZE = 24;
    static constexpr size_t TEMP_ALLOC_SIZE = 1024;
    static constexpr char STASHED_VALUE = 33;
    void *addr;
    void *tmp;
    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);

    addr = aa.Alloc(FIRST_ALLOC_SIZE);
    ASSERT_NE(addr, nullptr);
    ASSERT_TRUE(IsAligned(addr, GetAlignmentInBytes(DEFAULT_ARENA_ALIGNMENT)));
    addr = aa.Alloc(4U);
    ASSERT_NE(addr, nullptr);
    ASSERT_TRUE(IsAligned(addr, GetAlignmentInBytes(DEFAULT_ARENA_ALIGNMENT)));
    tmp = aa.AllocArray<int>(TEMP_ALLOC_SIZE);
    ASSERT_NE(tmp, nullptr);
    // Make sure that we force to using dynamic pool if STACK pool enabled
    for (size_t i = 0; i < 5U; ++i) {
        void *mem = nullptr;
        mem = aa.Alloc(DEFAULT_ARENA_SIZE / 2U);
        ASSERT_NE(mem, nullptr);
        *(static_cast<char *>(mem)) = STASHED_VALUE;  // Try to catch segfault just in case something wrong
    }
    ASSERT_NE(tmp = aa.Alloc(DEFAULT_ARENA_SIZE - AlignUp(sizeof(Arena), GetAlignmentInBytes(DEFAULT_ARENA_ALIGNMENT))),
              nullptr);
    size_t maxAlignDrift =
        (DEFAULT_ALIGNMENT_IN_BYTES > alignof(Arena)) ? (DEFAULT_ALIGNMENT_IN_BYTES - alignof(Arena)) : 0U;
    ASSERT_EQ(tmp = aa.Alloc(DEFAULT_ARENA_SIZE + maxAlignDrift + 1U), nullptr);
}

TEST_F(ArenaAllocatorTest, AllocateVectorTest)
{
    constexpr size_t SIZE = 2048;
    constexpr size_t SMALL_MAGIC_CONSTANT = 3;

    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    ArenaVector<unsigned> vec(aa.Adapter());

    for (size_t i = 0; i < SIZE; ++i) {
        vec.push_back(i * SMALL_MAGIC_CONSTANT);
    }

    ASSERT_EQ(SIZE, vec.size());
    vec.shrink_to_fit();
    ASSERT_EQ(SIZE, vec.size());

    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(i * SMALL_MAGIC_CONSTANT, vec[i]) << "value of i: " << i;
    }
}

TEST_F(ArenaAllocatorTest, AllocateVectorWithComplexTypeTest)
{
    constexpr size_t SIZE = 512;
    constexpr size_t MAGIC_CONSTANT_1 = std::numeric_limits<size_t>::max() / (SIZE + 2U);
    srand(GetSeed());
    size_t magicConstant2 = GetRand() % SIZE;

    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    ArenaVector<ComplexClass> vec(aa.Adapter());

    // Allocate SIZE objects
    for (size_t i = 0; i < SIZE; ++i) {
        vec.emplace_back(i * MAGIC_CONSTANT_1 + magicConstant2, std::to_string(i));
    }

    // Size checking
    ASSERT_EQ(SIZE, vec.size());

    // Allocations checking
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(vec[i].GetValue(), i * MAGIC_CONSTANT_1 + magicConstant2) << "value of i: " << i;
        ASSERT_EQ(vec[i].GetString(), std::to_string(i)) << i;
    }
    Span<ComplexClass> dataPtr(vec.data(), SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(dataPtr[i].GetValue(), i * MAGIC_CONSTANT_1 + magicConstant2) << "value of i: " << i;
        ASSERT_EQ(dataPtr[i].GetString(), std::to_string(i)) << "value of i: " << i;
    }

    // Resizing and new elements assignment
    constexpr size_t SIZE_2 = SIZE << 1U;
    vec.assign(SIZE_2, ComplexClass(1U, "1"));

    // Size checking
    ASSERT_EQ(SIZE_2, vec.size());
    vec.shrink_to_fit();
    ASSERT_EQ(SIZE_2, vec.size());

    // Allocations and assignment checking
    for (size_t i = 0; i < SIZE_2; ++i) {
        ASSERT_EQ(vec[i].GetValue(), 1U) << "value of i: " << i;
        ASSERT_EQ(vec[i].GetString(), "1") << "value of i: " << i;
    }

    // Increasing size
    constexpr size_t SIZE_4 = SIZE_2 << 1U;
    vec.resize(SIZE_4, ComplexClass());

    // Size checking
    ASSERT_EQ(SIZE_4, vec.size());

    // Allocations checking
    for (size_t i = 0; i < SIZE_4 / 2U; ++i) {
        ASSERT_EQ(vec[i].GetValue(), 1U) << "value of i: " << i;
        ASSERT_EQ(vec[i].GetString(), "1") << "value of i: " << i;
    }
    for (size_t i = SIZE_4 / 2U; i < SIZE_4; ++i) {
        ASSERT_EQ(vec[i].GetValue(), 0U) << "value of i: " << i;
        ASSERT_EQ(vec[i].GetString(), "0") << "value of i: " << i;
    }

    // Decreasing size
    vec.resize(SIZE);

    // Size checking
    ASSERT_EQ(SIZE, vec.size());
    vec.shrink_to_fit();
    ASSERT_EQ(SIZE, vec.size());

    // Allocations checking
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(vec[i].GetValue(), 1U) << "value of i: " << i;
        ASSERT_EQ(vec[i].GetString(), "1") << "value of i: " << i;
    }
}

TEST_F(ArenaAllocatorTest, AllocateDequeWithComplexTypeTest)
{
    constexpr size_t SIZE = 2048;
    constexpr size_t MAGIC_CONSTANT_1 = std::numeric_limits<size_t>::max() / (SIZE + 2U);
    srand(GetSeed());
    size_t magicConstant2 = GetRand() % SIZE;

    size_t i;

    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    ArenaDeque<ComplexClass> deq(aa.Adapter());

    // Allocate SIZE objects
    for (size_t j = 0; j < SIZE; ++j) {
        deq.emplace_back(j * MAGIC_CONSTANT_1 + magicConstant2, std::to_string(j));
    }

    // Size checking
    ASSERT_EQ(SIZE, deq.size());

    // Allocations checking
    i = 0;
    for (auto it = deq.cbegin(); it != deq.cend(); ++it, ++i) {
        ASSERT_EQ(it->GetValue(), i * MAGIC_CONSTANT_1 + magicConstant2) << "value of i: " << i;
        ASSERT_EQ(it->GetString(), std::to_string(i)) << "value of i: " << i;
    }

    // Resizing and new elements assignment
    constexpr size_t SIZE_2 = SIZE << 1U;
    deq.assign(SIZE_2, ComplexClass(1U, "1"));

    // Size checking
    ASSERT_EQ(SIZE_2, deq.size());
    deq.shrink_to_fit();
    ASSERT_EQ(SIZE_2, deq.size());

    // Allocations and assignment checking
    i = SIZE_2 - 1L;
    for (auto it = deq.crbegin(); it != deq.crend(); ++it, --i) {
        ASSERT_EQ(it->GetValue(), 1U) << "value of i: " << i;
        ASSERT_EQ(it->GetString(), "1") << "value of i: " << i;
    }

    // Increasing size
    constexpr size_t SIZE_4 = SIZE_2 << 1U;
    deq.resize(SIZE_4, ComplexClass());

    // Size checking
    ASSERT_EQ(SIZE_4, deq.size());

    // Allocations checking
    auto it = deq.cbegin();
    for (size_t j = 0; j < SIZE_4 / 2U; ++j, ++it) {
        ASSERT_EQ(it->GetValue(), 1U) << "value of i: " << j;
        ASSERT_EQ(it->GetString(), "1") << "value of i: " << j;
    }
    for (size_t j = SIZE_4 / 2U; j < SIZE_4; ++j, ++it) {
        ASSERT_EQ(it->GetValue(), 0U) << "value of i: " << j;
        ASSERT_EQ(it->GetString(), "0") << "value of i: " << j;
    }

    // Decreasing size
    deq.resize(SIZE);

    // Size checking
    ASSERT_EQ(SIZE, deq.size());
    deq.shrink_to_fit();
    ASSERT_EQ(SIZE, deq.size());

    // Allocations checking
    i = 0;
    for (auto tIt = deq.cbegin(); tIt != deq.cend(); ++tIt, ++i) {
        ASSERT_EQ(tIt->GetValue(), 1U) << "value of i: " << i;
        ASSERT_EQ(tIt->GetString(), "1") << "value of i: " << i;
    }
}

TEST_F(ArenaAllocatorTest, LongRandomTest)
{
    constexpr size_t SIZE = 3250000;
    constexpr size_t HALF_SIZE = SIZE >> 1U;
    constexpr size_t DOUBLE_SIZE = SIZE << 1U;
    constexpr auto MAX_VAL = MaxValue<uint32_t>();

    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    ArenaDeque<uint32_t> st(aa.Adapter());
    size_t i = 0;

    srand(GetSeed());

    // Allocations
    for (size_t j = 0; j < SIZE; ++j) {
        st.push_back(GetRand() % MAX_VAL);
    }

    // Size checking
    ASSERT_EQ(st.size(), SIZE);

    // Allocations checking
    srand(GetSeed());
    i = 0;
    for (unsigned int &tIt : st) {
        ASSERT_EQ(tIt, GetRand() % MAX_VAL) << "value of i: " << i;
    }

    // Decreasing size
    st.resize(HALF_SIZE);

    // Size chcking
    ASSERT_EQ(st.size(), HALF_SIZE);

    // Allocations checking
    srand(GetSeed());
    i = 0;
    for (unsigned int &tIt : st) {
        ASSERT_EQ(tIt, GetRand() % MAX_VAL) << "value of i: " << i;
    }

    // Increasing size
    st.resize(DOUBLE_SIZE, GetRand() % MAX_VAL);

    // Allocations checking
    srand(GetSeed());
    auto it = st.cbegin();
    for (i = 0; i < HALF_SIZE; ++it, ++i) {
        ASSERT_EQ(*it, GetRand() % MAX_VAL) << "value of i: " << i;
    }
    for (uint32_t value = GetRand() % MAX_VAL; it != st.cend(); ++it, ++i) {
        ASSERT_EQ(*it, value) << "value of i: " << i;
    }

    // Change values
    srand(GetSeed() >> 1U);
    for (unsigned int &tIt : st) {
        tIt = GetRand() % MAX_VAL;
    }

    // Changes checking
    srand(GetSeed() >> 1U);
    i = 0;
    for (auto tIt = st.cbegin(); tIt != st.cend(); ++tIt, ++i) {
        ASSERT_EQ(*tIt, GetRand() % MAX_VAL) << "value of i: " << i;
    }
}

TEST_F(ArenaAllocatorTest, LogAlignmentSmallSizesTest)
{
    constexpr size_t MAX_SMALL_SIZE = 32;

    for (size_t size = 1; size < MAX_SMALL_SIZE; ++size) {
        ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);

        for (Alignment align = LOG_ALIGN_MIN; align <= LOG_ALIGN_MAX;
             align = static_cast<Alignment>(static_cast<size_t>(align) + 1)) {
            void *ptr = aa.Alloc(size, align);
            size_t mask = GetAlignmentInBytes(align) - 1L;

            ASSERT_NE(ptr, nullptr);
            ASSERT_EQ(reinterpret_cast<size_t>(ptr) & mask, 0U)
                << "alignment: " << align << "addr: " << reinterpret_cast<size_t>(ptr);
        }
    }
}

TEST_F(ArenaAllocatorTest, LogAlignmentBigSizeTest)
{
    constexpr size_t SIZE = 0.3_KB;
    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);

    for (Alignment align = LOG_ALIGN_MIN; align <= LOG_ALIGN_MAX;
         align = static_cast<Alignment>(static_cast<size_t>(align) + 1)) {
        void *ptr = aa.Alloc(SIZE, align);
        size_t mask = GetAlignmentInBytes(align) - 1L;

        ASSERT_NE(ptr, nullptr);
        ASSERT_EQ(reinterpret_cast<size_t>(ptr) & mask, 0U)
            << "alignment: " << align << "addr: " << reinterpret_cast<size_t>(ptr);
    }
}

TEST_F(ArenaAllocatorTest, ArrayUINT16AlignmentTest)
{
    AllocateWithAlignment<uint16_t>();
}

TEST_F(ArenaAllocatorTest, ArrayUINT32AlignmentTest)
{
    AllocateWithAlignment<uint32_t>();
}

TEST_F(ArenaAllocatorTest, ArrayUINT64AlignmentTest)
{
    AllocateWithAlignment<uint64_t>();
}

TEST_F(ArenaAllocatorTest, ArrayUINT16WithDiffAlignmentTest)
{
    AllocateWithDiffAlignment<uint16_t>();
}

TEST_F(ArenaAllocatorTest, ArrayUINT32WithDiffAlignmentTest)
{
    AllocateWithDiffAlignment<uint32_t>();
}

TEST_F(ArenaAllocatorTest, ArrayUINT64WithDiffAlignmentTest)
{
    AllocateWithDiffAlignment<uint64_t>();
}

TEST_F(ArenaAllocatorTest, FunctionNewTest)
{
    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    std::array<ComplexClass *, ARRAY_SIZE> arr {};

    // Allocations
    srand(GetSeed());
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        arr[i] = aa.New<ComplexClass>(GetRand() % MaxValue<size_t>());
    }

    // Allocations checking
    srand(GetSeed());
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        ASSERT_NE(arr[i], nullptr);
        size_t randomValue = GetRand() % MaxValue<size_t>();
        ASSERT_EQ(arr[i]->GetValue(), randomValue);
        ASSERT_EQ(arr[i]->GetString(), std::to_string(randomValue));
    }

    // Change values
    srand(GetSeed() >> 1U);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        arr[i]->SetValue(GetRand() % MaxValue<size_t>());
    }

    // Changes checking
    srand(GetSeed() >> 1U);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        size_t randomValue = GetRand() % MaxValue<size_t>();
        ASSERT_EQ(arr[i]->GetValue(), randomValue);
        ASSERT_EQ(arr[i]->GetString(), std::to_string(randomValue));
    }
}

TEST_F(ArenaAllocatorTest, ResizeTest)
{
    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    static constexpr size_t ALLOC_COUNT = 1000;
    static constexpr size_t INIT_VAL = 0xdeadbeef;
    size_t *firstVar = aa.New<size_t>(INIT_VAL);
    {
        size_t initSize = aa.GetAllocatedSize();
        for (size_t i = 0; i < ALLOC_COUNT; i++) {
            [[maybe_unused]] void *tmp = aa.Alloc(sizeof(size_t));
        }
        EXPECT_DEATH(aa.Resize(aa.GetAllocatedSize() + 1U), "");
        aa.Resize(initSize);
        ASSERT_EQ(aa.GetAllocatedSize(), initSize);
    }
    ASSERT_EQ(*firstVar, INIT_VAL);
}

TEST_F(ArenaAllocatorTest, ResizeWrapperTest)
{
    static constexpr size_t VECTOR_SIZE = 1000;
    ArenaAllocator aa(SpaceType::SPACE_TYPE_INTERNAL);
    size_t oldSize = aa.GetAllocatedSize();
    {
        ArenaResizeWrapper<false> wrapper(&aa);
        ArenaVector<size_t> vector(aa.Adapter());
        for (size_t i = 0; i < VECTOR_SIZE; i++) {
            vector.push_back(i);
        }
    }
    ASSERT_EQ(oldSize, aa.GetAllocatedSize());
}

}  // namespace ark
