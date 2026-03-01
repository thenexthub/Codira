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

#include "gtest/gtest.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/utils.h"
#include "runtime/mem/vm_handle-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::coretypes::test {

class StringTest : public testing::Test {
public:
    StringTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        seed_ = 0xDEADBEEF;
#endif
        srand(seed_);
        // We need to create a runtime instance to be able to create strings.
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
    }

    ~StringTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StringTest);
    NO_MOVE_SEMANTIC(StringTest);

    LanguageContext GetLanguageContext()
    {
        return Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    }

    void SetUp() override
    {
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
    }

protected:
    static constexpr uint32_t SIMPLE_UTF8_STRING_LENGTH = 13;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr char SIMPLE_UTF8_STRING[SIMPLE_UTF8_STRING_LENGTH + 1] = "Hello, world!";

private:
    ark::MTManagedThread *thread_ {};
    unsigned seed_ {};
    RuntimeOptions options_;
};

TEST_F(StringTest, EqualStringWithCompressedRawUtf8Data)
{
    std::vector<uint8_t> data {0x01, 0x05, 0x07, 0x00};
    uint32_t utf16Length = data.size() - 1;
    auto *firstString =
        String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_TRUE(String::StringsAreEqualMUtf8(firstString, data.data(), utf16Length));
}

TEST_F(StringTest, EqualStringWithNotCompressedRawUtf8Data)
{
    std::vector<uint8_t> data {0xc2, 0xa7};

    for (size_t i = 0; i < 20U; i++) {
        data.push_back(0x30 + i);
    }
    data.push_back(0);

    uint32_t utf16Length = data.size() - 2U;
    auto *firstString =
        String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_TRUE(String::StringsAreEqualMUtf8(firstString, data.data(), utf16Length));
}

TEST_F(StringTest, NotEqualStringWithNotCompressedRawUtf8Data)
{
    std::vector<uint8_t> data1 {0xc2, 0xa7, 0x33, 0x00};
    std::vector<uint8_t> data2 {0xc2, 0xa7, 0x34, 0x00};
    uint32_t utf16Length = 2;
    auto *firstString =
        String::CreateFromMUtf8(data1.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(String::StringsAreEqualMUtf8(firstString, data2.data(), utf16Length));
}

TEST_F(StringTest, NotEqualStringNotCompressedStringWithCompressedRawData)
{
    std::vector<uint8_t> data1 {0xc2, 0xa7, 0x33, 0x00};
    std::vector<uint8_t> data2 {0x02, 0x07, 0x04, 0x00};
    uint32_t utf16Length1 = 2;
    uint32_t utf16Length2 = 3;
    auto *firstString =
        String::CreateFromMUtf8(data1.data(), utf16Length1, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(String::StringsAreEqualMUtf8(firstString, data2.data(), utf16Length2));
}

TEST_F(StringTest, NotEqualCompressedStringWithUncompressedRawUtf8Data)
{
    std::vector<uint8_t> data1 {0x02, 0x07, 0x04, 0x00};
    std::vector<uint8_t> data2 {0xc2, 0xa7, 0x33, 0x00};
    uint32_t utf16Length1 = 3;
    uint32_t utf16Length2 = 2;
    auto *firstString =
        String::CreateFromMUtf8(data1.data(), utf16Length1, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(String::StringsAreEqualMUtf8(firstString, data2.data(), utf16Length2));
}

TEST_F(StringTest, EqualStringWithMUtf8DifferentLength)
{
    std::vector<uint8_t> data1 {0xc2, 0xa7, 0x33, 0x00};
    std::vector<uint8_t> data2 {0xc2, 0xa7, 0x00};
    uint32_t utf16Length = 2;
    auto *firstString =
        String::CreateFromMUtf8(data1.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(String::StringsAreEqualMUtf8(firstString, data2.data(), utf16Length - 1));
}

TEST_F(StringTest, EqualStringWithRawUtf16Data)
{
    std::vector<uint16_t> data {0xffc3, 0x33, 0x00};
    auto *firstString =
        String::CreateFromUtf16(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto secondString = reinterpret_cast<const uint16_t *>(data.data());
    ASSERT_TRUE(String::StringsAreEqualUtf16(firstString, secondString, data.size()));
}

TEST_F(StringTest, CompareCompressedStringWithRawUtf16)
{
    std::vector<uint16_t> data;

    for (size_t i = 0; i < 30U; i++) {
        data.push_back(i + 1);
    }
    data.push_back(0);

    auto *firstString = String::CreateFromUtf16(data.data(), data.size() - 1, GetLanguageContext(),
                                                Runtime::GetCurrent()->GetPandaVM());
    auto secondString = reinterpret_cast<const uint16_t *>(data.data());
    ASSERT_TRUE(String::StringsAreEqualUtf16(firstString, secondString, data.size() - 1));
}

TEST_F(StringTest, EqualStringWithRawUtf16DifferentLength)
{
    std::vector<uint16_t> data1 {0xffc3, 0x33, 0x00};
    std::vector<uint16_t> data2 {0xffc3, 0x33, 0x55, 0x00};
    auto *firstString =
        String::CreateFromUtf16(data1.data(), data1.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto secondString = reinterpret_cast<const uint16_t *>(data2.data());
    ASSERT_FALSE(String::StringsAreEqualUtf16(firstString, secondString, data2.size()));
}

TEST_F(StringTest, NotEqualStringWithRawUtf16Data)
{
    std::vector<uint16_t> data1 {0xffc3, 0x33, 0x00};
    std::vector<uint16_t> data2 {0xffc3, 0x34, 0x00};
    auto *firstString =
        String::CreateFromUtf16(data1.data(), data1.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());

    auto secondString = reinterpret_cast<const uint16_t *>(data2.data());
    ASSERT_FALSE(String::StringsAreEqualUtf16(firstString, secondString, data2.size()));
}

TEST_F(StringTest, compressedHashCodeUtf8)
{
    String *firstString =
        String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(SIMPLE_UTF8_STRING), SIMPLE_UTF8_STRING_LENGTH,
                                GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto stringHashCode = firstString->GetHashcode();
    auto rawHashCode =
        String::ComputeHashcodeMutf8(reinterpret_cast<const uint8_t *>(SIMPLE_UTF8_STRING), SIMPLE_UTF8_STRING_LENGTH);

    ASSERT_EQ(stringHashCode, rawHashCode);
}
TEST_F(StringTest, notCompressedHashCodeUtf8)
{
    std::vector<uint8_t> data {0xc2, 0xa7};

    size_t size = 1;
    for (size_t i = 0; i < 20U; i++) {
        data.push_back(0x30 + i);
        size += 1;
    }
    data.push_back(0);

    String *firstString = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(data.data()), size,
                                                  GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto stringHashCode = firstString->GetHashcode();
    auto rawHashCode = String::ComputeHashcodeMutf8(reinterpret_cast<const uint8_t *>(data.data()), size);

    ASSERT_EQ(stringHashCode, rawHashCode);
}

TEST_F(StringTest, compressedHashCodeUtf16)
{
    std::vector<uint16_t> data;

    size_t size = 30;
    for (size_t i = 0; i < size; i++) {
        data.push_back(i + 1);
    }
    data.push_back(0);

    auto *firstString =
        String::CreateFromUtf16(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto stringHashCode = firstString->GetHashcode();
    auto rawHashCode = String::ComputeHashcodeUtf16(data.data(), data.size());
    ASSERT_EQ(stringHashCode, rawHashCode);
}

TEST_F(StringTest, notCompressedHashCodeUtf16)
{
    std::vector<uint16_t> data {0xffc3, 0x33, 0x00};
    auto *firstString =
        String::CreateFromUtf16(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    auto stringHashCode = firstString->GetHashcode();
    auto rawHashCode = String::ComputeHashcodeUtf16(data.data(), data.size());
    ASSERT_EQ(stringHashCode, rawHashCode);
}

TEST_F(StringTest, lengthUtf8)
{
    String *string =
        String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(SIMPLE_UTF8_STRING), SIMPLE_UTF8_STRING_LENGTH,
                                GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetLength(), SIMPLE_UTF8_STRING_LENGTH);
}

TEST_F(StringTest, lengthUtf16)
{
    std::vector<uint16_t> data {0xffc3, 0x33, 0x00};
    auto *string =
        String::CreateFromUtf16(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetLength(), data.size());
}

TEST_F(StringTest, DifferentLengthStringCompareTest)
{
    static constexpr uint32_t F_STRING_LENGTH = 8;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr char F_STRING[F_STRING_LENGTH + 1] = "Hello, w";
    String *firstString =
        String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(SIMPLE_UTF8_STRING), SIMPLE_UTF8_STRING_LENGTH,
                                GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(firstString->GetLength(), SIMPLE_UTF8_STRING_LENGTH);
    String *secondString = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(F_STRING), F_STRING_LENGTH,
                                                   GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(secondString->GetLength(), F_STRING_LENGTH);
    ASSERT_EQ(String::StringsAreEqual(firstString, secondString), false);
}

TEST_F(StringTest, ForeignLengthAndCopyTest1b0)
{
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'z', 0xc0, 0x80, 0x00};
    uint32_t utf16Length = data.size();
    String *string = String::CreateFromMUtf8(data.data(), utf16Length - 2U, GetLanguageContext(),
                                             Runtime::GetCurrent()->GetPandaVM());  // c080 is U+0000
    ASSERT_EQ(string->GetMUtf8Length(), data.size());
    ASSERT_EQ(string->GetUtf16Length(), data.size() - 2U);  // \0 doesn't counts for UTF16
    std::vector<uint8_t> out8(data.size());
    ASSERT_EQ(string->CopyDataMUtf8(out8.data(), out8.size(), true), data.size());
    ASSERT_EQ(out8, data);
    std::vector<uint16_t> res16 {'a', 'b', 'c', 'd', 'z', 0x00};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataUtf16(out16.data(), out16.size()), res16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, ForeignLengthAndCopyTest1b)
{
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'z', 0x7f, 0x00};
    uint32_t utf16Length = data.size();
    String *string = String::CreateFromMUtf8(data.data(), utf16Length - 1, GetLanguageContext(),
                                             Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetMUtf8Length(), data.size());
    ASSERT_EQ(string->GetUtf16Length(), data.size() - 1);  // \0 doesn't counts for UTF16
    std::vector<uint8_t> out8(data.size());
    ASSERT_EQ(string->CopyDataMUtf8(out8.data(), out8.size(), true), data.size());
    ASSERT_EQ(out8, data);
    std::vector<uint16_t> res16 {'a', 'b', 'c', 'd', 'z', 0x7f};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataUtf16(out16.data(), out16.size()), res16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, ForeignLengthAndCopyTest2b)
{
    std::vector<uint8_t> data {0xc2, 0xa7, 0x33, 0x00};  // UTF-16 size is 2
    String *string =
        String::CreateFromMUtf8(data.data(), 2U, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetMUtf8Length(), data.size());
    ASSERT_EQ(string->GetUtf16Length(), 2U);  // \0 doesn't counts for UTF16
    std::vector<uint8_t> out8(data.size());
    ASSERT_EQ(string->CopyDataMUtf8(out8.data(), out8.size(), true), data.size());
    ASSERT_EQ(out8, data);
    std::vector<uint16_t> res16 {0xa7, 0x33};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataUtf16(out16.data(), out16.size()), res16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, ForeignLengthAndCopyTest3b)
{
    std::vector<uint8_t> data {0xef, 0xbf, 0x83, 0x33, 0x00};  // UTF-16 size is 2
    String *string =
        String::CreateFromMUtf8(data.data(), 2U, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetMUtf8Length(), data.size());
    ASSERT_EQ(string->GetUtf16Length(), 2U);  // \0 doesn't counts for UTF16
    std::vector<uint8_t> out8(data.size());
    ASSERT_EQ(string->CopyDataMUtf8(out8.data(), out8.size(), true), data.size());
    ASSERT_EQ(out8, data);
    std::vector<uint16_t> res16 {0xffc3, 0x33};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataUtf16(out16.data(), out16.size()), res16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, ForeignLengthAndCopyTest6b)
{
    std::vector<uint8_t> data {0xed, 0xa0, 0x81, 0xed, 0xb0, 0xb7, 0x20, 0x00};  // UTF-16 size is 3
    // We support 4-byte utf-8 sequences, so {0xd801, 0xdc37} is encoded to 4 bytes instead of 6
    std::vector<uint8_t> utf8Data {0xf0, 0x90, 0x90, 0xb7, 0x20, 0x00};
    String *string =
        String::CreateFromMUtf8(data.data(), 3U, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string->GetMUtf8Length(), utf8Data.size());
    ASSERT_EQ(string->GetUtf16Length(), 3U);  // \0 doesn't counts for UTF16
    std::vector<uint8_t> out8(utf8Data.size());
    string->CopyDataMUtf8(out8.data(), out8.size(), true);
    ASSERT_EQ(out8, utf8Data);
    std::vector<uint16_t> res16 {0xd801, 0xdc37, 0x20};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataUtf16(out16.data(), out16.size()), res16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, RegionCopyTestMutf8)
{
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'z', 0x00};
    uint32_t utf16Length = data.size() - 1;
    String *string =
        String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    size_t start = 2;
    size_t len = string->GetMUtf8Length();
    std::vector<uint8_t> res = {'c', 'd', 0x00};
    std::vector<uint8_t> out8(res.size());
    ASSERT_EQ(string->CopyDataRegionMUtf8(out8.data(), start, len - start - 1 - 1, out8.size()), out8.size() - 1);
    out8[out8.size() - 1] = '\0';
    ASSERT_EQ(out8, res);
    size_t len16 = string->GetUtf16Length();
    std::vector<uint16_t> res16 = {'c', 'd'};
    std::vector<uint16_t> out16(res16.size());
    ASSERT_EQ(string->CopyDataRegionUtf16(out16.data(), start, len16 - start - 1, out16.size()), out16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, RegionCopyTestUtf8)
{
    std::vector<uint8_t> data {'a', 'b', 'h', 'e', 'l', 'l', 'o', 'c', 'd', 'z', 0};
    std::vector<uint8_t> res {'h', 'e', 'l', 'l', 'o', 0};
    std::vector<uint8_t> copiedDataUtf8(res.size());
    size_t start = 2;
    size_t len = 5;
    String *str =
        String::CreateFromUtf8(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());

    ASSERT_EQ(str->CopyDataRegionUtf8(copiedDataUtf8.data(), start, len, copiedDataUtf8.size() - 1), res.size() - 1);
    ASSERT_EQ(copiedDataUtf8, res);

    std::vector<uint16_t> res16 {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint16_t> copiedDataUtf16(res16.size());

    ASSERT_EQ(str->CopyDataRegionUtf16(copiedDataUtf16.data(), start, len, copiedDataUtf16.size()), res16.size());
    ASSERT_EQ(copiedDataUtf16, res16);
}

TEST_F(StringTest, RegionCopyTestUtf16)
{
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'z', 0xc2, 0xa7, 0x00};
    uint32_t utf16Length = data.size() - 1 - 1;
    String *string =
        String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    size_t start = 2;
    std::vector<uint8_t> res = {'c', 'd', 'z', 0x00};
    std::vector<uint8_t> out8(res.size());
    ASSERT_EQ(string->CopyDataRegionMUtf8(out8.data(), start, 3U, out8.size()), out8.size() - 1);
    out8[out8.size() - 1] = '\0';
    ASSERT_EQ(out8, res);
    size_t len16 = string->GetUtf16Length();
    std::vector<uint16_t> out16(len16 - start - 1);
    std::vector<uint16_t> res16 = {'c', 'd', 'z'};
    ASSERT_EQ(string->CopyDataRegionUtf16(out16.data(), start, 3U, out16.size()), out16.size());
    ASSERT_EQ(out16, res16);
}

TEST_F(StringTest, GetUtf8Length)
{
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!', 0};
    String *str =
        String::CreateFromUtf8(data.data(), data.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(str->GetUtf8Length(), data.size());
}

TEST_F(StringTest, GetDataUtf8)
{
    std::vector<uint8_t> example = {'e', 'x', 'a', 'm', 'p', 'l', 'e'};
    String *string1 = String::CreateFromUtf8(example.data(), example.size(), GetLanguageContext(),
                                             Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(string1->IsUtf16());
    std::vector<uint8_t> data2(string1->GetDataUtf8(), string1->GetDataUtf8() + example.size());  // NOLINT

    String *string2 =
        String::CreateFromUtf8(data2.data(), data2.size(), GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_FALSE(string2->IsUtf16());
    ASSERT_TRUE(String::StringsAreEqual(string1, string2));
}

TEST_F(StringTest, SameLengthStringCompareTest)
{
    static constexpr uint32_t STRING_LENGTH = 10;
    char *fString = new char[STRING_LENGTH + 1];
    char *sString = new char[STRING_LENGTH + 1];

    for (uint32_t i = 0; i < STRING_LENGTH; i++) {
        // Hack for ConvertMUtf8ToUtf16 call.
        // We should use char from 0x7f to 0x0 if we want to
        // generate one utf16 (0x00xx) from this mutf8.
        // NOLINTNEXTLINE(cert-msc50-cpp)
        uint8_t val1 = rand();
        val1 = val1 >> 1U;
        if (val1 == 0) {
            val1++;
        }

        // NOLINTNEXTLINE(cert-msc50-cpp)
        uint8_t val2 = rand();
        val2 = val2 >> 1U;
        if (val2 == 0) {
            val2++;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        fString[i] = val1;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        sString[i] = val2;
    }
    // Set the last elements in strings with size more than 0x8 to disable compressing.
    // This will leads to count two MUtf-8 bytes as one UTF-16 so length = string_length - 1
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[STRING_LENGTH - 2U] = uint8_t(0x80);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sString[STRING_LENGTH - 2U] = uint8_t(0x80);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[STRING_LENGTH - 1] = uint8_t(0x01);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sString[STRING_LENGTH - 1] = uint8_t(0x01);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[STRING_LENGTH] = '\0';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sString[STRING_LENGTH] = '\0';

    String *firstUtf16String = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(fString), STRING_LENGTH - 1,
                                                       GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    // Try to use function with automatic length detection
    String *secondUtf16String = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(sString),
                                                        GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(firstUtf16String->GetLength(), STRING_LENGTH - 1);
    ASSERT_EQ(secondUtf16String->GetLength(), STRING_LENGTH - 1);

    // Dirty hack to not create utf16 for our purpose, just reuse old one
    // Try to create compressed strings.
    String *firstUtf8String = String::CreateFromUtf16(firstUtf16String->GetDataUtf16(), STRING_LENGTH - 1,
                                                      GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    String *secondUtf8String = String::CreateFromUtf16(firstUtf16String->GetDataUtf16(), STRING_LENGTH - 1,
                                                       GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(firstUtf8String->GetLength(), STRING_LENGTH - 1);
    ASSERT_EQ(secondUtf8String->GetLength(), STRING_LENGTH - 1);

    ASSERT_EQ(String::StringsAreEqual(firstUtf16String, secondUtf16String), strcmp(fString, sString) == 0);
    ASSERT_EQ(String::StringsAreEqual(firstUtf16String, secondUtf8String),
              firstUtf16String->IsUtf16() != secondUtf8String->IsUtf16());
    ASSERT_EQ(String::StringsAreEqual(firstUtf8String, secondUtf8String), true);
    ASSERT_TRUE(firstUtf16String->IsUtf16());
    ASSERT_TRUE(String::StringsAreEqualUtf16(firstUtf16String, firstUtf16String->GetDataUtf16(),
                                             firstUtf16String->GetLength()));

    delete[] fString;
    delete[] sString;
}

TEST_F(StringTest, ObjectSize)
{
    {
        std::vector<uint8_t> data {'1', '2', '3', '4', '5', 0x00};
        uint32_t utf16Length = data.size() - 1;
        String *string = String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(string->ObjectSize(), String::ComputeSizeMUtf8(utf16Length));
    }

    {
        std::vector<uint8_t> data {0x80, 0x01, 0x80, 0x02, 0x00};
        uint32_t utf16Length = data.size() / 2U;
        String *string = String::CreateFromMUtf8(data.data(), utf16Length, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(string->ObjectSize(), String::ComputeSizeUtf16(utf16Length));
    }
}

TEST_F(StringTest, AtTest)
{
    // utf8
    std::vector<uint8_t> data1 {'a', 'b', 'c', 'd', 'z', 0};
    String *string = String::CreateFromMUtf8(data1.data(), data1.size() - 1, GetLanguageContext(),
                                             Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(false, string->IsUtf16());
    for (uint32_t i = 0; i < data1.size() - 1; i++) {
        ASSERT_EQ(data1[i], string->At(i));
    }

    // utf16
    std::vector<uint16_t> data2 {'a', 'b', 0xab, 0xdc, 'z', 0};
    string = String::CreateFromUtf16(data2.data(), data2.size() - 1, GetLanguageContext(),
                                     Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(true, string->IsUtf16());
    for (uint32_t i = 0; i < data2.size() - 1; i++) {
        ASSERT_EQ(data2[i], string->At(i));
    }

    // utf16 -> utf8
    std::vector<uint16_t> data3 {'a', 'b', 121, 122, 'z', 0};
    string = String::CreateFromUtf16(data3.data(), data3.size() - 1, GetLanguageContext(),
                                     Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(false, string->IsUtf16());
    for (uint32_t i = 0; i < data3.size() - 1; i++) {
        ASSERT_EQ(data3[i], string->At(i));
    }
}

TEST_F(StringTest, IndexOfTest)
{
    std::vector<uint8_t> data1 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint8_t> data2 {'b', 'c', 'd', 0};
    std::vector<uint16_t> data3 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint16_t> data4 {'b', 'c', 'd', 0};
    String *string1 = String::CreateFromMUtf8(data1.data(), data1.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string2 = String::CreateFromMUtf8(data2.data(), data2.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string3 = String::CreateFromUtf16(data3.data(), data3.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string4 = String::CreateFromUtf16(data4.data(), data4.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());

    auto index = string1->IndexOf(string2, GetLanguageContext(), 1);
    auto index1 = string1->IndexOf(string4, GetLanguageContext(), 1);
    auto index2 = string3->IndexOf(string2, GetLanguageContext(), 1);
    auto index3 = string3->IndexOf(string4, GetLanguageContext(), 1);
    std::cout << index << std::endl;
    ASSERT_EQ(index, index2);
    ASSERT_EQ(index1, index3);
    index = string1->IndexOf(string2, GetLanguageContext(), 2_I);
    index1 = string1->IndexOf(string4, GetLanguageContext(), 2_I);
    index2 = string3->IndexOf(string2, GetLanguageContext(), 2_I);
    index3 = string3->IndexOf(string4, GetLanguageContext(), 2_I);
    std::cout << index << std::endl;
    ASSERT_EQ(index, index2);
    ASSERT_EQ(index1, index3);
}

TEST_F(StringTest, IndexOfTest2)
{
    {
        std::vector<uint8_t> stringData {'a', 'b', 'a', 'c', 'a', 'b', 'a', 0};
        std::vector<uint8_t> patternData {'a', 'b', 'a', 0};
        String *string = String::CreateFromMUtf8(stringData.data(), stringData.size() - 1, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
        String *pattern = String::CreateFromMUtf8(patternData.data(), patternData.size() - 1, GetLanguageContext(),
                                                  Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(0, string->IndexOf(pattern, GetLanguageContext(), -1));
        ASSERT_EQ(0, string->IndexOf(pattern, GetLanguageContext(), 0));
        ASSERT_EQ(4_I, string->IndexOf(pattern, GetLanguageContext(), 1));
        ASSERT_EQ(4_I, string->IndexOf(pattern, GetLanguageContext(), 4_I));
        ASSERT_EQ(-1, string->IndexOf(pattern, GetLanguageContext(), 5_I));
        ASSERT_EQ(-1, string->IndexOf(pattern, GetLanguageContext(), 6_I));

        String *emptyString = String::CreateEmptyString(GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(-1, emptyString->IndexOf(string, GetLanguageContext(), 0));
        ASSERT_EQ(0, string->IndexOf(emptyString, GetLanguageContext(), -3_I));
        ASSERT_EQ(2_I, string->IndexOf(emptyString, GetLanguageContext(), 2_I));
        ASSERT_EQ(7_I, string->IndexOf(emptyString, GetLanguageContext(), 10_I));
    }
    {
        std::vector<uint8_t> stringData {'a', 'b', 'c', 'd', 'e', 'f', 'g', 0};
        std::vector<uint8_t> patternData {'d', 'e', 'f', 0};
        String *string = String::CreateFromMUtf8(stringData.data(), stringData.size() - 1, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
        String *pattern = String::CreateFromMUtf8(patternData.data(), patternData.size() - 1, GetLanguageContext(),
                                                  Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(3_I, string->IndexOf(pattern, GetLanguageContext(), 0));
    }
    {
        std::vector<uint8_t> stringData {'a', 'b', 'a', 'a', 'a', 'a', 'a', 0};
        std::vector<uint8_t> patternData {'a', 'a', 'a', 0};
        String *string = String::CreateFromMUtf8(stringData.data(), stringData.size() - 1, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
        String *pattern = String::CreateFromMUtf8(patternData.data(), patternData.size() - 1, GetLanguageContext(),
                                                  Runtime::GetCurrent()->GetPandaVM());
        ASSERT_EQ(2_I, string->IndexOf(pattern, GetLanguageContext(), 0));
        ASSERT_EQ(2_I, string->IndexOf(pattern, GetLanguageContext(), 2_I));
        ASSERT_EQ(3_I, string->IndexOf(pattern, GetLanguageContext(), 3_I));
        ASSERT_EQ(4_I, string->IndexOf(pattern, GetLanguageContext(), 4_I));
        ASSERT_EQ(-1, string->IndexOf(pattern, GetLanguageContext(), 5_I));
    }
}

TEST_F(StringTest, CompareTestUtf8)
{
    // utf8
    std::vector<uint8_t> data1 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint8_t> data2 {'a', 'b', 'c', 'd', 'z', 'x', 0};
    std::vector<uint16_t> data3 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint16_t> data4 {'a', 'b', 'd', 'c', 'z', 0};
    String *string1 = String::CreateFromMUtf8(data1.data(), data1.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string2 = String::CreateFromMUtf8(data2.data(), data2.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string3 = String::CreateFromUtf16(data3.data(), data3.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string4 = String::CreateFromUtf16(data4.data(), data4.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(false, string1->IsUtf16());
    ASSERT_EQ(false, string2->IsUtf16());
    ASSERT_EQ(false, string3->IsUtf16());
    ASSERT_EQ(false, string4->IsUtf16());
    ASSERT_LT(string1->Compare(string2, GetLanguageContext()), 0);
    ASSERT_GT(string2->Compare(string1, GetLanguageContext()), 0);
    ASSERT_EQ(string1->Compare(string3, GetLanguageContext()), 0);
    ASSERT_EQ(string3->Compare(string1, GetLanguageContext()), 0);
    ASSERT_LT(string2->Compare(string4, GetLanguageContext()), 0);
    ASSERT_GT(string4->Compare(string2, GetLanguageContext()), 0);

    // utf8 vs utf16
    std::vector<uint16_t> data5 {'a', 'b', 0xab, 0xdc, 'z', 0};
    String *string5 = String::CreateFromUtf16(data5.data(), data5.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(true, string5->IsUtf16());
    ASSERT_LT(string2->Compare(string5, GetLanguageContext()), 0);
    ASSERT_GT(string5->Compare(string2, GetLanguageContext()), 0);
    ASSERT_LT(string4->Compare(string5, GetLanguageContext()), 0);
    ASSERT_GT(string5->Compare(string4, GetLanguageContext()), 0);

    // compare with self
    ASSERT_EQ(string1->Compare(string1, GetLanguageContext()), 0);
    ASSERT_EQ(string2->Compare(string2, GetLanguageContext()), 0);
    ASSERT_EQ(string3->Compare(string3, GetLanguageContext()), 0);
    ASSERT_EQ(string4->Compare(string4, GetLanguageContext()), 0);
    ASSERT_EQ(string5->Compare(string5, GetLanguageContext()), 0);
}

TEST_F(StringTest, CompareTestUtf16)
{
    std::vector<uint16_t> data5 {'a', 'b', 0xab, 0xdc, 'z', 0};
    String *string5 = String::CreateFromUtf16(data5.data(), data5.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    std::vector<uint16_t> data6 {'a', 0xab, 0xab, 0};
    String *string6 = String::CreateFromUtf16(data6.data(), data6.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string7 = String::CreateFromUtf16(data6.data(), data6.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(true, string5->IsUtf16());
    ASSERT_EQ(true, string6->IsUtf16());
    ASSERT_EQ(true, string7->IsUtf16());

    ASSERT_LT(string5->Compare(string6, GetLanguageContext()), 0);
    ASSERT_GT(string6->Compare(string5, GetLanguageContext()), 0);
    ASSERT_EQ(string6->Compare(string7, GetLanguageContext()), 0);
    ASSERT_EQ(string7->Compare(string6, GetLanguageContext()), 0);

    // compare with self
    ASSERT_EQ(string5->Compare(string5, GetLanguageContext()), 0);
    ASSERT_EQ(string6->Compare(string6, GetLanguageContext()), 0);
    ASSERT_EQ(string7->Compare(string7, GetLanguageContext()), 0);
}

TEST_F(StringTest, CompareTestLongUtf8)
{
    // long utf8 string vs long utf8 string
    // utf8
    std::vector<uint8_t> data8(16U, 'a');
    data8.push_back(0);

    std::vector<uint8_t> data9(16U, 'a');
    std::vector<uint8_t> tmp1 {'x', 'z'};
    data9.insert(data9.end(), tmp1.begin(), tmp1.end());
    data9.push_back(0);

    std::vector<uint8_t> data10(16U, 'a');
    std::vector<uint8_t> tmp2 {'x', 'x', 'x', 'y', 'y', 'a', 'a'};
    data10.insert(data10.end(), tmp2.begin(), tmp2.end());
    data10.insert(data10.end(), 16U, 'a');
    data10.push_back(0);

    std::vector<uint8_t> data11(16U, 'a');
    std::vector<uint8_t> tmp3 {'x', 'x', 'x', 'y', 'y', 'y', 'y'};
    data11.insert(data11.end(), tmp3.begin(), tmp3.end());
    data11.insert(data11.end(), 16U, 'a');
    data11.push_back(0);

    String *string8 = String::CreateFromMUtf8(data8.data(), data8.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string9 = String::CreateFromMUtf8(data9.data(), data9.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string10 = String::CreateFromMUtf8(data10.data(), data10.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string11 = String::CreateFromMUtf8(data11.data(), data11.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string12 = String::CreateFromMUtf8(data8.data(), data8.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string13 = String::CreateFromMUtf8(data9.data(), data9.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());

    // utf8 vs utf8
    ASSERT_EQ(string8->Compare(string12, GetLanguageContext()), 0);
    ASSERT_EQ(string12->Compare(string8, GetLanguageContext()), 0);
    ASSERT_EQ(string9->Compare(string13, GetLanguageContext()), 0);
    ASSERT_EQ(string13->Compare(string9, GetLanguageContext()), 0);
    ASSERT_LT(string10->Compare(string11, GetLanguageContext()), 0);
    ASSERT_GT(string11->Compare(string10, GetLanguageContext()), 0);
    ASSERT_LT(string10->Compare(string9, GetLanguageContext()), 0);
    ASSERT_GT(string9->Compare(string10, GetLanguageContext()), 0);
}

TEST_F(StringTest, CompareTestLongUtf16)
{
    // long utf16 string vs long utf16 string
    // utf16
    std::vector<uint16_t> data14(16U, 0xab);
    data14.push_back(0);

    std::vector<uint16_t> data15(16U, 0xab);
    std::vector<uint16_t> tmp4 {'a', 0xbb};
    data15.insert(data15.end(), tmp4.begin(), tmp4.end());
    data15.push_back(0);

    std::vector<uint16_t> data16(16U, 0xab);
    std::vector<uint16_t> tmp5 {'a', 'a', 0xcc, 0xcc, 0xdd, 0xdd, 0xdd};
    data16.insert(data16.end(), tmp5.begin(), tmp5.end());
    data16.insert(data16.end(), 16U, 0xab);
    data16.push_back(0);

    std::vector<uint16_t> data17(16U, 0xab);
    std::vector<uint16_t> tmp6 {'a', 'a', 0xdd, 0xdd, 0xdd, 0xdd, 0xdd};
    data17.insert(data17.end(), tmp6.begin(), tmp6.end());
    data17.insert(data17.end(), 16U, 0xab);
    data17.push_back(0);

    String *string14 = String::CreateFromUtf16(data14.data(), data14.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string15 = String::CreateFromUtf16(data15.data(), data15.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string16 = String::CreateFromUtf16(data16.data(), data16.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string17 = String::CreateFromUtf16(data17.data(), data17.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string18 = String::CreateFromUtf16(data14.data(), data14.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string19 = String::CreateFromUtf16(data15.data(), data15.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());

    // utf16 vs utf16
    ASSERT_EQ(string14->Compare(string18, GetLanguageContext()), 0);
    ASSERT_EQ(string18->Compare(string14, GetLanguageContext()), 0);
    ASSERT_EQ(string15->Compare(string19, GetLanguageContext()), 0);
    ASSERT_EQ(string19->Compare(string15, GetLanguageContext()), 0);
    ASSERT_LT(string16->Compare(string17, GetLanguageContext()), 0);
    ASSERT_GT(string17->Compare(string16, GetLanguageContext()), 0);
    ASSERT_LT(string16->Compare(string15, GetLanguageContext()), 0);
    ASSERT_GT(string15->Compare(string16, GetLanguageContext()), 0);
}

TEST_F(StringTest, ConcatTest)
{
    // utf8 + utf8
    std::vector<uint8_t> data1 {'f', 'g', 'h', 0};
    std::vector<uint8_t> data2 {'a', 'b', 'c', 'd', 'e', 0};
    std::vector<uint8_t> data3;
    data3.insert(data3.end(), data1.begin(), data1.end() - 1);
    data3.insert(data3.end(), data2.begin(), data2.end());

    String *string1 = String::CreateFromMUtf8(data1.data(), data1.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string2 = String::CreateFromMUtf8(data2.data(), data2.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string30 = String::CreateFromMUtf8(data3.data(), data3.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(false, string1->IsUtf16());
    ASSERT_EQ(false, string2->IsUtf16());
    String *string31 = String::Concat(string1, string2, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string30->Compare(string31, GetLanguageContext()), 0);
    ASSERT_EQ(string31->Compare(string30, GetLanguageContext()), 0);

    // utf8 + utf16
    std::vector<uint16_t> data4 {'a', 'b', 0xab, 0xdc, 'z', 0};
    std::vector<uint16_t> data5 {'f', 'g', 'h', 'a', 'b', 0xab, 0xdc, 'z', 0};  // data1 + data4
    String *string4 = String::CreateFromUtf16(data4.data(), data4.size() - 1, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());
    String *string50 = String::CreateFromUtf16(data5.data(), data5.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string51 = String::Concat(string1, string4, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string50->GetLength(), string51->GetLength());
    ASSERT_EQ(string50->Compare(string51, GetLanguageContext()), 0);
    ASSERT_EQ(string51->Compare(string50, GetLanguageContext()), 0);

    // utf16 + utf16
    std::vector<uint16_t> data6;
    data6.insert(data6.end(), data4.begin(), data4.end() - 1);
    data6.insert(data6.end(), data5.begin(), data5.end());
    String *string60 = String::CreateFromUtf16(data6.data(), data6.size() - 1, GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *string61 = String::Concat(string4, string50, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(string60->Compare(string61, GetLanguageContext()), 0);
    ASSERT_EQ(string61->Compare(string60, GetLanguageContext()), 0);
}

TEST_F(StringTest, DoReplaceTest0)
{
    static constexpr uint32_t STRING_LENGTH = 10;
    char *fString = new char[STRING_LENGTH + 1];
    char *sString = new char[STRING_LENGTH + 1];

    for (uint32_t i = 0; i < STRING_LENGTH; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        fString[i] = 'A' + i;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        sString[i] = 'A' + i;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[0] = 'Z';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[STRING_LENGTH] = '\0';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sString[STRING_LENGTH] = '\0';

    String *fStringS = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(fString), GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *sStringS = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(sString), GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *tStringS = String::DoReplace(fStringS, 'Z', 'A', GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(String::StringsAreEqual(tStringS, sStringS), true);

    delete[] fString;
    delete[] sString;
}

TEST_F(StringTest, FastSubstringTest0)
{
    uint32_t stringLength = 10;
    char *fString = new char[stringLength + 1];
    for (uint32_t i = 0; i < stringLength; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        fString[i] = 'A' + i;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    fString[stringLength] = '\0';

    uint32_t subStringLength = 5;
    uint32_t subStringStart = 1;
    char *sString = new char[subStringLength + 1];
    for (uint32_t j = 0; j < subStringLength; j++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        sString[j] = fString[subStringStart + j];
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sString[subStringLength] = '\0';

    String *fStringS = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(fString), GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *sStringS = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(sString), GetLanguageContext(),
                                               Runtime::GetCurrent()->GetPandaVM());
    String *tStringS = String::FastSubString(fStringS, subStringStart, subStringLength, GetLanguageContext(),
                                             Runtime::GetCurrent()->GetPandaVM());
    ASSERT_EQ(String::StringsAreEqual(tStringS, sStringS), true);

    delete[] fString;
    delete[] sString;
}

TEST_F(StringTest, ToCharArray)
{
    // utf8
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'e', 0};
    String *utf8String = String::CreateFromMUtf8(data.data(), data.size() - 1, GetLanguageContext(),
                                                 Runtime::GetCurrent()->GetPandaVM());
    Array *newArray = utf8String->ToCharArray(GetLanguageContext());
    for (uint32_t i = 0; i < newArray->GetLength(); ++i) {
        ASSERT_EQ(data[i], newArray->Get<uint16_t>(i));
    }

    std::vector<uint16_t> data1 {'f', 'g', 'h', 'a', 'b', 0x8ab, 0xdc, 'z', 0};
    String *utf16String = String::CreateFromUtf16(data1.data(), data1.size() - 1, GetLanguageContext(),
                                                  Runtime::GetCurrent()->GetPandaVM());
    Array *newArray1 = utf16String->ToCharArray(GetLanguageContext());
    for (uint32_t i = 0; i < newArray1->GetLength(); ++i) {
        ASSERT_EQ(data1[i], newArray1->Get<uint16_t>(i));
    }
}

TEST_F(StringTest, CreateNewStingFromCharArray)
{
    std::vector<uint16_t> data {'f', 'g', 'h', 'a', 'b', 0x8ab, 0xdc, 'z', 0};
    String *utf16String = String::CreateFromUtf16(data.data(), data.size() - 1, GetLanguageContext(),
                                                  Runtime::GetCurrent()->GetPandaVM());
    Array *charArray = utf16String->ToCharArray(GetLanguageContext());

    uint32_t charArrayLength = 5;
    uint32_t charArrayOffset = 1;
    std::vector<uint16_t> data1(charArrayLength + 1);
    for (uint32_t i = 0; i < charArrayLength; ++i) {
        data1[i] = data[i + charArrayOffset];
    }
    data1[charArrayLength] = 0;
    String *utf16String1 = String::CreateFromUtf16(data1.data(), data1.size() - 1, GetLanguageContext(),
                                                   Runtime::GetCurrent()->GetPandaVM());

    String *result = String::CreateNewStringFromChars(charArrayOffset, charArrayLength, charArray, GetLanguageContext(),
                                                      Runtime::GetCurrent()->GetPandaVM());

    ASSERT_EQ(String::StringsAreEqual(result, utf16String1), true);
}

TEST_F(StringTest, CreateNewStingFromByteArray)
{
    std::vector<uint8_t> data {'f', 'g', 'h', 'a', 'b', 0xab, 0xdc, 'z', 0};
    uint32_t byteArrayLength = 5;
    uint32_t byteArrayOffset = 1;
    uint32_t highByte = 0;

    std::vector<uint16_t> data1(byteArrayLength);
    for (uint32_t i = 0; i < byteArrayLength; ++i) {
        data1[i] = (highByte << 8U) + (data[i + byteArrayOffset] & 0xFFU);
    }
    // NB! data1[byte_array_length] = 0; NOT NEEDED
    String *string1 = String::CreateFromUtf16(data1.data(), byteArrayLength, GetLanguageContext(),
                                              Runtime::GetCurrent()->GetPandaVM());

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ark::ClassRoot::ARRAY_I8);
    Array *byteArray = Array::Create(klass, data.size() - 1);
    Span<uint8_t> sp(data.data(), data.size() - 1);
    for (uint32_t i = 0; i < data.size() - 1; i++) {
        byteArray->Set<uint8_t>(i, sp[i]);
    }

    String *result = String::CreateNewStringFromBytes(byteArrayOffset, byteArrayLength, highByte, byteArray,
                                                      GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());

    ASSERT_EQ(String::StringsAreEqual(result, string1), true);
}

TEST_F(StringTest, SlicedStringGC)
{
    // 1. create SlicedString
    auto thread = ManagedThread::GetCurrent();
    HandleScope<ObjectHeader *> scope(thread);
    String *string = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>("abcdefghijklmnopkrstuvwxyz"),
                                             GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> stringHandle(thread, string);

    String *expected = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>("abcdefghijklmno"),
                                               GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> expectedHandle(thread, expected);

    String *result = String::FastSubString(string, 0, 15, GetLanguageContext(), Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> resultHandle(thread, result);

    ASSERT_EQ(stringHandle->IsLineString(), true);
    ASSERT_EQ(resultHandle->IsSlicedString(), true);

    auto firstReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };
    const common::SlicedString *slicedString = resultHandle->ToSlicedString();
    String *parentString = String::Cast(slicedString->GetParent<common::BaseString *>(std::move(firstReadBarrier)));
    ASSERT_EQ(parentString->IsLineString(), true);
    ASSERT_EQ(slicedString->IsSlicedString(), true);
    ASSERT_EQ(String::StringsAreEqual(parentString, stringHandle.GetPtr()), true);
    ASSERT_EQ(String::StringsAreEqual(resultHandle.GetPtr(), expectedHandle.GetPtr()), true);

    // 2. trigger gc
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));

    // 3. check SlicedString not changed
    auto secondReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };
    slicedString = resultHandle->ToSlicedString();
    parentString = String::Cast(slicedString->GetParent<common::BaseString *>(std::move(secondReadBarrier)));
    ASSERT_EQ(parentString->IsLineString(), true);
    ASSERT_EQ(slicedString->IsSlicedString(), true);
    ASSERT_EQ(String::StringsAreEqual(parentString, stringHandle.GetPtr()), true);
    ASSERT_EQ(String::StringsAreEqual(resultHandle.GetPtr(), expectedHandle.GetPtr()), true);
}

TEST_F(StringTest, TreeStringGC)
{
    // 1. create TreeString
    auto thread = ManagedThread::GetCurrent();
    HandleScope<ObjectHeader *> scope(thread);
    String *sub1 = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>("subLeftString"), GetLanguageContext(),
                                           Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> sub1Handle(thread, sub1);

    String *sub2 = String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>("subRightString"), GetLanguageContext(),
                                           Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> sub2Handle(thread, sub2);

    String *result = String::Concat(sub1Handle.GetPtr(), sub2Handle.GetPtr(), GetLanguageContext(),
                                    Runtime::GetCurrent()->GetPandaVM());
    VMHandle<String> resultHandle(thread, result);

    ASSERT_EQ(sub1Handle->IsLineString(), true);
    ASSERT_EQ(sub2Handle->IsLineString(), true);
    ASSERT_EQ(resultHandle->IsTreeString(), true);

    auto leftFirstReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };

    auto rightFirstReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };

    common::TreeString *tree = resultHandle->ToTreeString();
    String *left = String::Cast(tree->GetLeftSubString<common::BaseString *>(std::move(leftFirstReadBarrier)));
    String *right = String::Cast(tree->GetRightSubString<common::BaseString *>(std::move(rightFirstReadBarrier)));
    ASSERT_EQ(left->IsLineString(), true);
    ASSERT_EQ(right->IsLineString(), true);
    ASSERT_EQ(String::StringsAreEqual(left, sub1Handle.GetPtr()), true);
    ASSERT_EQ(String::StringsAreEqual(right, sub2Handle.GetPtr()), true);

    // 2. trigger gc
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));

    // 3. check TreeString not changed
    auto leftSecondReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };

    auto rightSecondReadBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };
    tree = resultHandle->ToTreeString();
    left = String::Cast(tree->GetLeftSubString<common::BaseString *>(std::move(leftSecondReadBarrier)));
    right = String::Cast(tree->GetRightSubString<common::BaseString *>(std::move(rightSecondReadBarrier)));
    ASSERT_EQ(left->IsLineString(), true);
    ASSERT_EQ(right->IsLineString(), true);
    ASSERT_EQ(String::StringsAreEqual(left, sub1Handle.GetPtr()), true);
    ASSERT_EQ(String::StringsAreEqual(right, sub2Handle.GetPtr()), true);
}
}  // namespace ark::coretypes::test

// NOLINTEND(readability-magic-numbers)
