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

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/asan_interface.h"

#include "gtest/gtest.h"

namespace ark::os::mem::test {

class MmapTest : public ::testing::Test {
public:
    MmapTest() = default;
    NO_COPY_SEMANTIC(MmapTest);
    NO_MOVE_SEMANTIC(MmapTest);
    ~MmapTest() override = default;

protected:
    void FillMemory(void *start, size_t size)
    {
        size_t itEnd = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < itEnd; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pointer[i] = MAGIC_VALUE;
        }
    }

    bool IsZeroMemory(void *start, size_t size)
    {
        size_t itEnd = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < itEnd; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (pointer[i] != 0U) {
                return false;
            }
        }
        return true;
    }
    static void *DEFAULT_MMAP_TEST_HINT;  // NOLINT(readability-identifier-naming)
    static constexpr size_t DEFAULT_MMAP_TEST_SIZE = PANDA_DEFAULT_POOL_SIZE;

private:
    static constexpr size_t MAGIC_VALUE = 0xDEADBEAF;
};

void *MmapTest::DEFAULT_MMAP_TEST_HINT = ToVoidPtr(PANDA_32BITS_HEAP_START_ADDRESS);

TEST_F(MmapTest, MapRWAnonymousFixedRawTest)
{
    void *result = MapRWAnonymousFixedRaw(DEFAULT_MMAP_TEST_HINT, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_EQ(result, DEFAULT_MMAP_TEST_HINT) << "mmap with fixed flag must return hint address";
    auto res = UnmapRaw(result, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_FALSE(res) << "Unmup for mmaped address must be correct";
}

TEST_F(MmapTest, MapRWAnonymousFixedRawZeroSizeTest)
{
    void *result = MapRWAnonymousFixedRaw(DEFAULT_MMAP_TEST_HINT, 0U);
    ASSERT_EQ(result, nullptr) << "result for 0 requested size must be nullptr";
}

TEST_F(MmapTest, MapCorrectInFirst4GBTest)
{
    void *result = MapRWAnonymousInFirst4GB(DEFAULT_MMAP_TEST_HINT, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_NE(result, nullptr);
    EXPECT_GE(ToUintPtr(result), ToUintPtr(DEFAULT_MMAP_TEST_HINT)) << "mmaped address can't be less then hint address";
    EXPECT_LE(ToUintPtr(result) + DEFAULT_MMAP_TEST_SIZE, 4_GB) << "mmaped sapce must be placed into first 4GB";
    auto res = UnmapRaw(result, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_FALSE(res) << "Unmup for mmaped address must be correct";
}

TEST_F(MmapTest, MapFailedInFirst4GBTest)
{
#ifdef PANDA_TARGET_64
    void *result = MapRWAnonymousInFirst4GB(ToVoidPtr(4_GB), DEFAULT_MMAP_TEST_SIZE);
    ASSERT_EQ(result, nullptr) << "MapRWAnonymousInFirst4GB must not map memory equal 4GB";
    result = MapRWAnonymousInFirst4GB(ToVoidPtr(4_GB + GetPageSize()), DEFAULT_MMAP_TEST_SIZE);
    ASSERT_EQ(result, nullptr) << "MapRWAnonymousInFirst4GB must not map memory greater 4GB";
#endif  // PANDA_TARGET_64
}

TEST_F(MmapTest, MadviseTest)
{
#ifndef PANDA_QEMU_BUILD
    // Unfortunately, we have an issue with QEMU, where madvise works differently.
    void *result = MapRWAnonymousInFirst4GB(DEFAULT_MMAP_TEST_HINT, DEFAULT_MMAP_TEST_SIZE);
    ASAN_UNPOISON_MEMORY_REGION(result, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_NE(result, nullptr);
    EXPECT_GE(ToUintPtr(result), ToUintPtr(DEFAULT_MMAP_TEST_HINT)) << "mmaped address can't be less then hint address";
    EXPECT_LE(ToUintPtr(result) + DEFAULT_MMAP_TEST_SIZE, 4_GB) << "mmaped sapce must be placed into first 4GB";
    FillMemory(result, DEFAULT_MMAP_TEST_SIZE);
    os::mem::ReleasePages(ToUintPtr(result), ToUintPtr(result) + DEFAULT_MMAP_TEST_SIZE);
    ASSERT_TRUE(IsZeroMemory(result, DEFAULT_MMAP_TEST_SIZE));
    auto res = UnmapRaw(result, DEFAULT_MMAP_TEST_SIZE);
    ASSERT_FALSE(res) << "Unmup for mmaped address must be correct";
#endif
}

class MMapFixedTest : public testing::Test {
protected:
    static constexpr uint64_t MAGIC_VALUE = 0xDEADBEAF;
    void DeathWrite64(uintptr_t addr)
    {
        auto pointer = static_cast<uint64_t *>(ToVoidPtr(addr));
        *pointer = MAGIC_VALUE;
    }
};

TEST_F(MMapFixedTest, MMapAsanTsanTest)
{
    static constexpr size_t OFFSET = 4_KB;
    static constexpr size_t MMAP_ALLOC_SIZE = OFFSET * 2U;
    size_t pageSize = ark::os::mem::GetPageSize();
    static_assert(OFFSET < ark::os::mem::MMAP_FIXED_MAGIC_ADDR_FOR_SANITIZERS);
    static_assert(MMAP_ALLOC_SIZE > OFFSET);
    ASSERT_TRUE((MMAP_ALLOC_SIZE % pageSize) == 0U);
    uintptr_t curAddr = ark::os::mem::MMAP_FIXED_MAGIC_ADDR_FOR_SANITIZERS - OFFSET;
    curAddr = AlignUp(curAddr, pageSize);
    ASSERT_TRUE((curAddr % pageSize) == 0U);
    uintptr_t endAddr = ark::os::mem::MMAP_FIXED_MAGIC_ADDR_FOR_SANITIZERS;
    endAddr = AlignUp(endAddr, sizeof(uint64_t));
    void *result =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        mmap(ToVoidPtr(curAddr), MMAP_ALLOC_SIZE, MMAP_PROT_READ | MMAP_PROT_WRITE,
             MMAP_FLAG_PRIVATE | MMAP_FLAG_ANONYMOUS | MMAP_FLAG_FIXED, -1L, 0U);
    ASSERT_TRUE(result != nullptr);

#if (defined(PANDA_TSAN_ON) || defined(USE_THREAD_SANITIZER)) && defined(PANDA_TARGET_ARM64)
    // TSAN+Aarch64 stores its data near the address,
    // mmap cannot return the same address, as it is used by TSAN,
    // so the result should differ from a hint (currAddr)
    ASSERT_NE(ToUintPtr(result), curAddr);
#else
    ASSERT_TRUE(ToUintPtr(result) == curAddr);
    while (curAddr < endAddr) {
        DeathWrite64(curAddr);
        curAddr += sizeof(uint64_t);
    }
#if defined(PANDA_ASAN_ON)
    // Check Death:
    EXPECT_DEATH(DeathWrite64(endAddr), "");
#else
    // Writing must be finished successfully
    DeathWrite64(endAddr);
#endif
#endif
    munmap(result, MMAP_ALLOC_SIZE);
}

}  // namespace ark::os::mem::test
