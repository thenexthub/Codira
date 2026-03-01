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

#include <ctime>
#include "libarkbase/utils/hash.h"

#include "gtest/gtest.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/asan_interface.h"

namespace ark {

class HashTest : public testing::Test {
public:
    HashTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0xDEADBEEF;
#endif
    }

protected:
    template <class T>
    void OneObject32bitsHashTest();
    template <class T>
    void OneStringHashTest();
    template <class T>
    void StringMemHashTest();
    template <class T>
    void EndOfPageStringHashTest();
    static constexpr size_t KEY40INBYTES = 5;
    static constexpr size_t KEY32INBYTES = 4;
    static constexpr size_t KEY8INBYTES = 1;

    // Some platforms have this macro so do not redefine it.
#ifndef PAGE_SIZE
    static constexpr size_t PAGE_SIZE = SIZE_1K * 4U;
#endif
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    unsigned seed_;
};

template <class T>
void HashTest::OneObject32bitsHashTest()
{
    srand(seed_);

    // NOLINTNEXTLINE(cert-msc50-cpp)
    uint32_t object32 = rand();
    uint32_t firstHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object32), KEY32INBYTES);
    uint32_t secondHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object32), KEY32INBYTES);
    if (firstHash != secondHash) {
        std::cout << "Failed 32bit key hash on seed = 0x" << std::hex << seed_ << std::endl;
    }
    ASSERT_EQ(firstHash, secondHash);

    // NOLINTNEXTLINE(cert-msc50-cpp)
    uint8_t object8 = rand();
    firstHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object8), KEY8INBYTES);
    secondHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object8), KEY8INBYTES);
    if (firstHash != secondHash) {
        std::cout << "Failed 32bit key hash on seed = 0x" << std::hex << seed_ << std::endl;
    }
    ASSERT_EQ(firstHash, secondHash);

    // Set up 64 bits value and use only 40 bits from it
    // NOLINTNEXTLINE(cert-msc50-cpp)
    uint64_t object40 = rand();
    firstHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object40), KEY40INBYTES);
    secondHash = T::GetHash32(reinterpret_cast<uint8_t *>(&object40), KEY40INBYTES);
    if (firstHash != secondHash) {
        std::cout << "Failed 32bit key hash on seed = 0x" << std::hex << seed_ << std::endl;
    }
    ASSERT_EQ(firstHash, secondHash);
}

template <class T>
void HashTest::OneStringHashTest()
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char string[] = "Over 1000!\0";
    // Dummy check. Don't ask me why...
    if (sizeof(char) != sizeof(uint8_t)) {
        return;
    }
    auto mutf8String = reinterpret_cast<uint8_t *>(string);
    uint32_t firstHash = T::GetHash32String(mutf8String);
    uint32_t secondHash = T::GetHash32String(mutf8String);
    ASSERT_EQ(firstHash, secondHash);
}

template <class T>
void HashTest::StringMemHashTest()
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char string[] = "COULD YOU CREATE MORE COMPLEX TESTS,OK?\0";
    size_t stringSize = strlen(string);
    auto *mutf8String = reinterpret_cast<uint8_t *>(string);
    uint32_t secondHash = T::GetHash32(mutf8String, stringSize);
    uint32_t firstHash = T::GetHash32String(mutf8String);
    ASSERT_EQ(firstHash, secondHash);
}

template <class T>
void HashTest::EndOfPageStringHashTest()
{
    constexpr const int64_t IMM_TWO = 2;
    size_t stringSize = 3;
    constexpr size_t ALLOC_SIZE = PAGE_SIZE * 2U;
    void *mem = ark::os::mem::MapRWAnonymousRaw(ALLOC_SIZE);
    ASAN_UNPOISON_MEMORY_REGION(mem, ALLOC_SIZE);
    ark::os::mem::MakeMemProtected(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(mem) + PAGE_SIZE), PAGE_SIZE);
    auto string = reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(mem) + PAGE_SIZE) - sizeof(char) * stringSize);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    string[0U] = 'O';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    string[1U] = 'K';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    string[IMM_TWO] = '\0';
    auto mutf8String = reinterpret_cast<uint8_t *>(string);
    uint32_t secondHash = T::GetHash32(mutf8String, stringSize - 1L);
    uint32_t firstHash = T::GetHash32String(mutf8String);
    ASSERT_EQ(firstHash, secondHash);
    auto res = ark::os::mem::UnmapRaw(mem, ALLOC_SIZE);
    ASSERT_FALSE(res);
}

// If we hash an object twice, it must return the same value
// Do it for 8 bits key, 32 bits and 40 bits key.
TEST_F(HashTest, OneObjectHashTest)
{
    HashTest::OneObject32bitsHashTest<MurmurHash32<DEFAULT_SEED>>();
}

// If we hash a string twice, it must return the same value
TEST_F(HashTest, OneStringHashTest)
{
    HashTest::OneStringHashTest<MurmurHash32<DEFAULT_SEED>>();
}

// If we hash a string with out string method,
// we should get the same result as we use a pointer to string as a raw memory.
TEST_F(HashTest, StringMemHashTest)
{
    HashTest::StringMemHashTest<MurmurHash32<DEFAULT_SEED>>();
}

// Try to hash the string which located at the end of allocated page.
// Check that we will not have SEGERROR here.
TEST_F(HashTest, EndOfPageStringHashTest)
{
    HashTest::EndOfPageStringHashTest<MurmurHash32<DEFAULT_SEED>>();
}

}  // namespace ark
