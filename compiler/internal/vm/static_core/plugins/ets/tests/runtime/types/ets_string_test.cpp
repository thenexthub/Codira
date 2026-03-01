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

#include "ets_coroutine.h"
#include "include/mem/panda_string.h"
#include "libarkbase/utils/utf.h"
#include "types/ets_array.h"
#include "types/ets_string.h"
#include <gtest/gtest.h>
#include <cstdint>

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {
class EtsStringTest : public testing::Test {
public:
    EtsStringTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsStringTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsStringTest);
    NO_MOVE_SEMANTIC(EtsStringTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    static EtsString *CreateString(std::string_view str)
    {
        std::vector<uint16_t> data(str.size());
        std::copy(std::begin(str), std::end(str), data.begin());
        return EtsString::CreateFromUtf16(data.data(), data.size());
    }

    static EtsString *CreateString(std::initializer_list<uint16_t> data)
    {
        return EtsString::CreateFromUtf16(data.begin(), data.size());
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsStringTest, CreateFromUtf16)
{
    std::vector<EtsChar> data {0xffc3, 0x33, 0x00};

    EtsString *firstEtsString = EtsString::CreateFromUtf16(data.data(), data.size());

    auto *firstString = reinterpret_cast<coretypes::String *>(firstEtsString);
    auto *secondString = reinterpret_cast<const uint16_t *>(data.data());

    ASSERT_TRUE(coretypes::String::StringsAreEqualUtf16(firstString, secondString, data.size()));
}

TEST_F(EtsStringTest, CreateFromMUtf8)
{
    std::vector<uint8_t> data {0x01, 0x41, 0x24, 0x00};
    auto *mutf8Data = reinterpret_cast<const char *>(data.data());

    EtsString *firstEtsString = EtsString::CreateFromMUtf8(mutf8Data);

    auto *firstString = reinterpret_cast<coretypes::String *>(firstEtsString);
    auto *secondString = reinterpret_cast<const uint8_t *>(data.data());

    ASSERT_TRUE(coretypes::String::StringsAreEqualMUtf8(
        firstString, secondString, data.size() - 1));  // need to subtract 1 'cause of 0 in the end of Mutf8 string
}

TEST_F(EtsStringTest, CreateFromMUtf8WithLenArg)
{
    std::vector<uint8_t> data {0x01, 0x41, 0x24, 0x00};
    auto *mutf8Data = reinterpret_cast<const char *>(data.data());

    EtsString *firstEtsString = EtsString::CreateFromMUtf8(
        mutf8Data, data.size() - 1);  // need to subtract 1 'cause of 0 in the end of Mutf8 string

    auto *firstString = reinterpret_cast<coretypes::String *>(firstEtsString);
    auto *secondString = reinterpret_cast<const uint8_t *>(data.data());

    ASSERT_TRUE(coretypes::String::StringsAreEqualMUtf8(
        firstString, secondString, data.size() - 1));  // need to subtract 1 'cause of 0 in the end of Mutf8 string
}

TEST_F(EtsStringTest, ToCharArray)
{
    std::vector<uint8_t> data {'a', 'b', 'c', 'd', 'e', 0};
    auto *mutf8Data = reinterpret_cast<const char *>(data.data());
    EtsString *utf8String = EtsString::CreateFromMUtf8(mutf8Data, data.size() - 1);
    EtsArray *newArray = utf8String->ToCharArray();

    for (uint32_t i = 0; i < newArray->GetLength(); ++i) {
        ASSERT_EQ(data[i], newArray->GetCoreType()->Get<uint16_t>(i));
    }

    std::vector<EtsChar> data1 {'f', 'g', 'h', 'a', 'b', 0x8ab, 0xdc, 'z', 0};
    EtsString *utf16String = EtsString::CreateFromUtf16(data1.data(), data1.size());
    EtsArray *newArray1 = utf16String->ToCharArray();

    for (uint32_t i = 0; i < newArray1->GetLength(); ++i) {
        ASSERT_EQ(data1[i], newArray1->GetCoreType()->Get<uint16_t>(i));
    }
}

TEST_F(EtsStringTest, CreateFromUtf8)
{
    std::vector<uint8_t> data {0x01, 0x41, 0x24, 0x32, 0x16, 0x08};
    size_t firstStringLength = data.size();
    auto *utf8Data = reinterpret_cast<const char *>(data.data());

    EtsString *firstEtsString = EtsString::CreateFromUtf8(utf8Data, firstStringLength);
    auto *firstString = reinterpret_cast<const uint8_t *>(data.data());

    // Test full string
    ASSERT_TRUE(utf::IsEqual({firstEtsString->GetDataMUtf8(), firstStringLength}, {firstString, firstStringLength}));
    ASSERT_TRUE(
        utf::IsEqual({firstEtsString->GetDataMUtf8(), firstStringLength - 2}, {firstString, firstStringLength - 2}));

    size_t thirdStringLength = firstStringLength / 2;
    EtsString *thirdEtsString = EtsString::CreateFromUtf8(utf8Data, thirdStringLength);
    auto *thirdString = reinterpret_cast<coretypes::String *>(thirdEtsString);

    // Utf8 format, no need to have \0 at the end, so check half string
    ASSERT_TRUE(utf::IsEqual({firstEtsString->GetDataMUtf8(), thirdStringLength},
                             {thirdString->GetDataMUtf8(), thirdStringLength}));
    ASSERT_TRUE(utf::IsEqual({firstEtsString->GetDataMUtf8(), thirdStringLength - 1},
                             {thirdString->GetDataMUtf8(), thirdStringLength - 1}));
}

TEST_F(EtsStringTest, CreateNewStringFromChars)
{
    std::vector<EtsChar> data {0x8ab, 0xdc, 'h', 'e', 'l', 'l', 'o', 0};

    EtsString *utf16String = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsArray *charArray = utf16String->ToCharArray();

    uint32_t beginOffset = 2;
    uint32_t length = data.size() - beginOffset;

    // make char array from subdata
    std::vector<EtsChar> subdata {'h', 'e', 'l', 'l', 'o', 0};

    EtsString *expectedString = EtsString::CreateFromUtf16(subdata.data(), subdata.size());

    EtsString *stringFromCharArray = EtsString::CreateNewStringFromChars(beginOffset, length, charArray);

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(expectedString),
                                                   reinterpret_cast<coretypes::String *>(stringFromCharArray)));
}

TEST_F(EtsStringTest, CreateNewStringFromString)
{
    std::vector<EtsChar> data {0xffc3, 0x33, 0x00};

    EtsString *string1 = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *string2 = EtsString::CreateFromUtf16(data.data(), data.size() - 1);
    EtsString *createdString = EtsString::CreateNewStringFromString(string1);

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(string1),
                                                   reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_FALSE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(string2),
                                                    reinterpret_cast<coretypes::String *>(createdString)));
}

TEST_F(EtsStringTest, CreateNewStringFromConcatString)
{
    EtsString *str1 = EtsString::CreateFromMUtf8("canjie_language");
    EtsString *str2 = EtsString::CreateFromMUtf8("arkts_language");
    EtsString *concat = EtsString::Concat(str1, str2);
    EtsString *createdString = EtsString::CreateNewStringFromString(concat);
    EtsString *equal = EtsString::CreateFromMUtf8("canjie_languagearkts_language");

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(concat),
                                                   reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(equal),
                                                   reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_FALSE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(str1),
                                                    reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_FALSE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(str2),
                                                    reinterpret_cast<coretypes::String *>(createdString)));
}

TEST_F(EtsStringTest, CreateNewStringFromSlicedString)
{
    EtsString *string1 = EtsString::CreateFromMUtf8("canjie_languagearkts_language");
    EtsString *slicedStr = EtsString::FastSubString(string1, 0, 17);
    EtsString *createdString = EtsString::CreateNewStringFromString(slicedStr);
    EtsString *equal = EtsString::CreateFromMUtf8("canjie_languagear");

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(slicedStr),
                                                   reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(equal),
                                                   reinterpret_cast<coretypes::String *>(createdString)));

    ASSERT_FALSE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(string1),
                                                    reinterpret_cast<coretypes::String *>(createdString)));
}

TEST_F(EtsStringTest, CreateNewEmptyString)
{
    EtsChar data = 0;
    EtsString *str1 = EtsString::CreateFromUtf16(&data, 0);
    EtsString *str2 = EtsString::CreateFromUtf16(&data, 1);
    EtsString *str3 = EtsString::CreateNewEmptyString();

    ASSERT_TRUE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(str1),
                                                   reinterpret_cast<coretypes::String *>(str3)));
    ASSERT_FALSE(coretypes::String::StringsAreEqual(reinterpret_cast<coretypes::String *>(str2),
                                                    reinterpret_cast<coretypes::String *>(str3)));
}

TEST_F(EtsStringTest, Compare)
{
    // utf8 vs utf8
    std::vector<uint8_t> data1 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint8_t> data2 {'a', 'b', 'c', 'd', 'z', 'x', 0};
    std::vector<uint16_t> data3 {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<uint16_t> data4 {'a', 'b', 'd', 'c', 'z', 0};

    auto *mutf8Data1 = reinterpret_cast<const char *>(data1.data());
    auto *mutf8Data2 = reinterpret_cast<const char *>(data2.data());

    EtsString *string1 = EtsString::CreateFromMUtf8(mutf8Data1);
    EtsString *string2 = EtsString::CreateFromMUtf8(mutf8Data2);
    EtsString *string3 = EtsString::CreateFromUtf16(data3.data(), data3.size() - 1);
    EtsString *string4 = EtsString::CreateFromUtf16(data4.data(), data4.size() - 1);

    ASSERT_EQ(false, string1->IsUtf16());
    ASSERT_EQ(false, string2->IsUtf16());
    ASSERT_EQ(false, string3->IsUtf16());
    ASSERT_EQ(false, string4->IsUtf16());
    ASSERT_LT(string1->Compare(string2), 0);
    ASSERT_GT(string2->Compare(string1), 0);
    ASSERT_EQ(string1->Compare(string3), 0);
    ASSERT_EQ(string3->Compare(string1), 0);
    ASSERT_LT(string2->Compare(string4), 0);
    ASSERT_GT(string4->Compare(string2), 0);

    // utf8 vs utf16
    std::vector<uint16_t> data5 {'a', 'b', 0xab, 0xdc, 'z', 0};
    EtsString *string5 = EtsString::CreateFromUtf16(data5.data(), data5.size() - 1);

    ASSERT_EQ(true, string5->IsUtf16());
    ASSERT_LT(string2->Compare(string5), 0);
    ASSERT_GT(string5->Compare(string2), 0);
    ASSERT_LT(string4->Compare(string5), 0);
    ASSERT_GT(string5->Compare(string4), 0);

    // utf16 vs utf16
    std::vector<uint16_t> data6 {'a', 0xab, 0xab, 0};
    EtsString *string6 = EtsString::CreateFromUtf16(data6.data(), data6.size() - 1);
    EtsString *string7 = EtsString::CreateFromUtf16(data6.data(), data6.size() - 1);

    ASSERT_EQ(true, string6->IsUtf16());
    ASSERT_EQ(true, string7->IsUtf16());
    ASSERT_LT(string5->Compare(string6), 0);
    ASSERT_GT(string6->Compare(string5), 0);
    ASSERT_EQ(string6->Compare(string7), 0);
    ASSERT_EQ(string7->Compare(string6), 0);

    // compare with self
    ASSERT_EQ(string1->Compare(string1), 0);
    ASSERT_EQ(string2->Compare(string2), 0);
    ASSERT_EQ(string3->Compare(string3), 0);
    ASSERT_EQ(string4->Compare(string4), 0);
    ASSERT_EQ(string5->Compare(string5), 0);
    ASSERT_EQ(string6->Compare(string6), 0);
    ASSERT_EQ(string7->Compare(string7), 0);
}

TEST_F(EtsStringTest, Concat)
{
    // utf8 + utf8
    std::vector<uint8_t> data1 {'H', 'e', 'l', 'l', 'o', 'I', 'a', 'm', 'f', 'r', 'o', 'm', 'c', 'h', 'i', 'n', 'a', 0};
    std::vector<uint8_t> data2 {'w', 'o', 'r', 'l', 'd', '!', 0};
    std::vector<uint8_t> data3;

    data3.insert(data3.end(), data1.begin(), data1.end() - 1);
    data3.insert(data3.end(), data2.begin(), data2.end());

    auto *mutf8Data1 = reinterpret_cast<const char *>(data1.data());
    auto *mutf8Data2 = reinterpret_cast<const char *>(data2.data());
    auto *mutf8Data3 = reinterpret_cast<const char *>(data3.data());

    EtsString *str1 = EtsString::CreateFromMUtf8(mutf8Data1);
    EtsString *str2 = EtsString::CreateFromMUtf8(mutf8Data2);
    EtsString *str3 = EtsString::Concat(str1, str2);
    EtsString *expectedStr1 = EtsString::CreateFromMUtf8(mutf8Data3);

    ASSERT_EQ(str3->Compare(expectedStr1), 0);
    ASSERT_EQ(expectedStr1->Compare(str3), 0);

    // utf16 + utf16
    std::vector<uint16_t> data4 {'a', 'b', 'c', 'd', 0};
    std::vector<uint16_t> data5 {'e', 'f', 'g', 0};
    std::vector<uint16_t> data6;

    data6.insert(data6.end(), data4.begin(), data4.end() - 1);
    data6.insert(data6.end(), data5.begin(), data5.end());

    EtsString *str4 = EtsString::CreateFromUtf16(data4.data(), data4.size() - 1);
    EtsString *str5 = EtsString::CreateFromUtf16(data5.data(), data5.size());
    EtsString *str6 = EtsString::Concat(str4, str5);
    EtsString *expectedStr2 = EtsString::CreateFromUtf16(data6.data(), data6.size());

    ASSERT_EQ(str6->Compare(expectedStr2), 0);
    ASSERT_EQ(expectedStr2->Compare(str6), 0);

    // utf8 + utf16
    std::vector<uint16_t> data7;
    data7.insert(data7.end(), data1.begin(), data1.end() - 1);
    data7.insert(data7.end(), data4.begin(), data4.end() - 1);

    EtsString *str7 = EtsString::Concat(str1, str4);
    EtsString *expectedStr3 = EtsString::CreateFromUtf16(data7.data(), data7.size());

    ASSERT_EQ(str7->Compare(expectedStr3), 0);
    ASSERT_EQ(expectedStr3->Compare(str7), 0);
}

TEST_F(EtsStringTest, At)
{
    // utf8
    std::vector<uint8_t> data1 {'a', 'b', 'c', 'd', 'z', 0};
    auto *mutf8Data1 = reinterpret_cast<const char *>(data1.data());
    EtsString *string = EtsString::CreateFromMUtf8(mutf8Data1, data1.size() - 1);
    ASSERT_EQ(false, string->IsUtf16());
    for (uint32_t i = 0; i < data1.size() - 1; i++) {
        ASSERT_EQ(data1[i], string->At(i));
    }

    // utf16
    std::vector<uint16_t> data2 {'a', 'b', 0xab, 0xdc, 'z', 0};
    string = EtsString::CreateFromUtf16(data2.data(), data2.size() - 1);
    ASSERT_EQ(true, string->IsUtf16());
    for (uint32_t i = 0; i < data2.size() - 1; i++) {
        ASSERT_EQ(data2[i], string->At(i));
    }

    // utf16 -> utf8
    std::vector<uint16_t> data3 {'a', 'b', 121, 122, 'z', 0};
    string = EtsString::CreateFromUtf16(data3.data(), data3.size() - 1);
    ASSERT_EQ(false, string->IsUtf16());
    for (uint32_t i = 0; i < data3.size() - 1; i++) {
        ASSERT_EQ(data3[i], string->At(i));
    }
}

TEST_F(EtsStringTest, DoReplace)
{
    uint32_t len = 10;
    std::vector<char> data1(len + 1);
    std::vector<char> data2(len + 1);

    for (uint32_t i = 0; i < len; i++) {
        data1[i] = 'A' + i;
        data2[i] = 'A' + i;
    }

    data1.front() = 'Z';
    data1.back() = '\0';
    data2.back() = '\0';

    EtsString *str1 = EtsString::CreateFromMUtf8(data1.data());
    EtsString *str2 = EtsString::CreateFromMUtf8(data2.data());
    EtsString *str3 = EtsString::DoReplace(str1, 'Z', 'A');

    ASSERT_NE(str1->Compare(str2), 0);
    ASSERT_EQ(str2->Compare(str3), 0);
}

TEST_F(EtsStringTest, FastSubString)
{
    uint32_t strLen = 10;
    uint32_t subStrLen = 5;
    uint32_t subStrStart = 1;

    std::vector<char> data1(strLen + 1);
    std::vector<char> data2(subStrLen + 1);

    for (uint32_t i = 0; i < strLen; i++) {
        data1[i] = 'A' + i;
    }
    data1.back() = '\0';

    for (uint32_t i = 0; i < subStrLen; i++) {
        data2[i] = data1[subStrStart + i];
    }
    data2.back() = '\0';

    EtsString *str1 = EtsString::CreateFromMUtf8(data1.data());
    EtsString *str2 = EtsString::CreateFromMUtf8(data2.data());
    EtsString *str3 = EtsString::FastSubString(str1, subStrStart, subStrLen);

    ASSERT_EQ(str3->Compare(str2), 0);
}

TEST_F(EtsStringTest, GetLength)
{
    // utf16
    std::vector<uint16_t> data1 {0xffc3, 0x33, 0x00};
    EtsString *str = EtsString::CreateFromUtf16(data1.data(), data1.size());
    ASSERT_EQ(str->GetLength(), data1.size());

    // utf8
    std::vector<char> data2 {'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!', 0};
    str = EtsString::CreateFromMUtf8(data2.data());

    ASSERT_EQ(false, str->IsUtf16());
    ASSERT_EQ(str->GetLength(), data2.size() - 1);
}

TEST_F(EtsStringTest, GetMUtf8Length)
{
    std::vector<char> data2 {'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!', 0};
    EtsString *str = EtsString::CreateFromMUtf8(data2.data());
    ASSERT_EQ(false, str->IsUtf16());
    ASSERT_EQ(str->GetMUtf8Length(), data2.size());
    ASSERT_EQ(str->GetMUtf8Length(), str->GetLength() + 1);
}

TEST_F(EtsStringTest, GetUtf16Length)
{
    std::vector<uint16_t> data1 {0xffc3, 0x33, 'a', 'b', 'c', 0x00};
    EtsString *str = EtsString::CreateFromUtf16(data1.data(), data1.size());
    ASSERT_EQ(true, str->IsUtf16());
    ASSERT_EQ(str->GetUtf16Length(), data1.size());
    ASSERT_EQ(str->GetLength(), str->GetUtf16Length());
}

TEST_F(EtsStringTest, CopyDataMUtf8)
{
    std::vector<char> data {'a', 'b', 'c', 'd', 'z', 0};
    std::vector<char> copiedDataMutf8(data.size());
    EtsString *str = EtsString::CreateFromMUtf8(data.data());

    ASSERT_EQ(str->CopyDataMUtf8(copiedDataMutf8.data(), copiedDataMutf8.size(), true), data.size());
    ASSERT_EQ(copiedDataMutf8, data);
}

TEST_F(EtsStringTest, CopyDataUtf16)
{
    std::vector<char> data {'a', 'b', 'c', 'd', 'z', 0};
    EtsString *str = EtsString::CreateFromMUtf8(data.data());
    std::vector<uint16_t> resUtf16 {'a', 'b', 'c', 'd', 'z'};
    std::vector<uint16_t> copiedDataUtf16(resUtf16.size());

    ASSERT_EQ(str->CopyDataUtf16(copiedDataUtf16.data(), copiedDataUtf16.size()), resUtf16.size());
    ASSERT_EQ(copiedDataUtf16, resUtf16);
}

TEST_F(EtsStringTest, CopyDataRegionMUtf8)
{
    std::vector<char> data {'a', 'b', 'h', 'e', 'l', 'l', 'o', 'c', 'd', 'z', 0};
    std::vector<char> res {'h', 'e', 'l', 'l', 'o', 0};
    std::vector<char> copiedDataMutf8(res.size());
    size_t start = 2;
    size_t len = 5;
    EtsString *str = EtsString::CreateFromMUtf8(data.data());

    ASSERT_EQ(str->CopyDataRegionMUtf8(copiedDataMutf8.data(), start, len, copiedDataMutf8.size()), res.size() - 1);
    ASSERT_EQ(copiedDataMutf8, res);

    std::vector<uint16_t> res16 {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint16_t> copiedDataUtf16(res16.size());

    ASSERT_EQ(str->CopyDataRegionUtf16(copiedDataUtf16.data(), start, len, copiedDataUtf16.size()), res16.size());
    ASSERT_EQ(copiedDataUtf16, res16);
}

TEST_F(EtsStringTest, CopyDataRegionUtf16)
{
    std::vector<uint16_t> data {0xb7, 0xc7, 0xa4, 'h', 'e', 'l', 'l', 'o', 0xa7, 0};
    std::vector<uint8_t> res {'h', 'e', 'l', 'l', 'o', 0};
    std::vector<uint8_t> copiedDataMutf8(res.size());
    size_t start = 3;
    size_t len = 5;
    EtsString *str = EtsString::CreateFromUtf16(data.data(), data.size());

    ASSERT_EQ(str->CopyDataRegionMUtf8(copiedDataMutf8.data(), start, len, copiedDataMutf8.size()), res.size() - 1);
    ASSERT_EQ(copiedDataMutf8, res);

    std::vector<uint16_t> res16 {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint16_t> copiedDataUtf16(res16.size());

    ASSERT_EQ(str->CopyDataRegionUtf16(copiedDataUtf16.data(), start, len, copiedDataUtf16.size()), res16.size());
    ASSERT_EQ(copiedDataUtf16, res16);
}

TEST_F(EtsStringTest, IsEqual)
{
    std::vector<char> data1 {'a', 'b', 'c', 'd', 'e', 0};
    std::vector<char> data2 {'a', 'b', 'c', 'd', 'f', 0};
    char data3 = 0;

    EtsString *str1 = EtsString::CreateFromMUtf8(data1.data());
    EtsString *str2 = EtsString::CreateFromMUtf8(data2.data());
    EtsString *str3 = EtsString::CreateNewEmptyString();

    ASSERT_TRUE(str1->IsEqual(data1.data()));
    ASSERT_TRUE(str2->IsEqual(data2.data()));
    ASSERT_TRUE(str3->IsEqual(&data3));
    ASSERT_FALSE(str1->IsEqual(data2.data()));
    ASSERT_FALSE(str2->IsEqual(data1.data()));
    ASSERT_FALSE(str1->IsEqual(&data3));
    ASSERT_FALSE(str2->IsEqual(&data3));
    ASSERT_FALSE(str3->IsEqual(data1.data()));
    ASSERT_FALSE(str3->IsEqual(data2.data()));
}

TEST_F(EtsStringTest, IsEmpty)
{
    std::vector<char> data {'a', 'f', 'a', 0};
    EtsString *str1 = EtsString::CreateFromMUtf8(data.data());
    EtsString *str2 = EtsString::CreateNewEmptyString();

    ASSERT_FALSE(str1->IsEmpty());
    ASSERT_TRUE(str2->IsEmpty());
}

TEST_F(EtsStringTest, StringsAreEqual)
{
    std::vector<char> data1 {'a', 'b', 's', 'r', 'd', 'r', 0};
    std::vector<char> data2 {'a', 'b', 's', 0};

    EtsString *str1 = EtsString::CreateFromMUtf8(data1.data(), data1.size() - 1);
    data1[data1.size() - 4U] = 0;
    EtsString *str2 = EtsString::CreateFromMUtf8(data1.data(), data1.size() - 4U);
    EtsString *str3 = EtsString::CreateFromMUtf8(data2.data());

    ASSERT_TRUE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str1)));
    ASSERT_TRUE(str2->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
    ASSERT_TRUE(str3->StringsAreEqual(reinterpret_cast<EtsObject *>(str3)));
    ASSERT_TRUE(str2->StringsAreEqual(reinterpret_cast<EtsObject *>(str3)));
    ASSERT_TRUE(str3->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));

    ASSERT_FALSE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
    ASSERT_FALSE(str2->StringsAreEqual(reinterpret_cast<EtsObject *>(str1)));
}

TEST_F(EtsStringTest, GetCoreType)
{
    std::vector<char> data {'a', 'b', 'c', 'd', 0};
    EtsString *str = EtsString::CreateFromMUtf8(data.data());
    EtsString *emptyStr = EtsString::CreateNewEmptyString();

    ASSERT_EQ(reinterpret_cast<coretypes::String *>(str), str->GetCoreType());
    ASSERT_EQ(reinterpret_cast<coretypes::String *>(emptyStr), emptyStr->GetCoreType());
}

TEST_F(EtsStringTest, FromEtsObject)
{
    std::vector<char> data {'a', 'b', 'c', 'd', 'e', 0};

    EtsString *str1 = EtsString::CreateFromMUtf8(data.data());
    EtsString *str3 = EtsString::CreateNewEmptyString();

    ASSERT_EQ(str1->Compare(EtsString::FromEtsObject(reinterpret_cast<EtsObject *>(str1))), 0);
    ASSERT_EQ(str3->Compare(EtsString::FromEtsObject(reinterpret_cast<EtsObject *>(str3))), 0);
}

TEST_F(EtsStringTest, CreateNewStringFromBytes)
{
    std::vector<uint8_t> data {'f', 'g', 'h', 'a', 'b', 0xab, 0xdc, 'z', 0};
    uint32_t byteArrayLength = 5;
    uint32_t byteArrayOffset = 1;
    uint32_t highByte = 1;

    std::vector<uint16_t> data1(byteArrayLength);
    for (uint32_t i = 0; i < byteArrayLength; ++i) {
        data1[i] = (highByte << 8U) + (data[i + byteArrayOffset]);
    }

    EtsString *string1 = EtsString::CreateFromUtf16(data1.data(), byteArrayLength);
    EtsByteArray *byteArray = EtsByteArray::Create(data.size() - 1);

    Span<uint8_t> sp(data.data(), data.size() - 1);
    for (uint32_t i = 0; i < data.size() - 1; i++) {
        byteArray->Set(i, sp[i]);
    }

    EtsString *result = EtsString::CreateNewStringFromBytes(byteArray, highByte, byteArrayOffset, byteArrayLength);

    ASSERT_TRUE(result->StringsAreEqual(reinterpret_cast<EtsObject *>(string1)));
}

TEST_F(EtsStringTest, GetDataUtf16)
{
    std::vector<uint16_t> data = {'a', 'b', 'c', 'd', 'e', 0xac, 0};

    EtsString *string1 = EtsString::CreateFromUtf16(data.data(), data.size());
    ASSERT_TRUE(string1->IsUtf16());
    EtsString *string2 = EtsString::CreateFromUtf16(string1->GetDataUtf16(), data.size());
    ASSERT_TRUE(string2->IsUtf16());

    ASSERT_TRUE(string1->StringsAreEqual(reinterpret_cast<EtsObject *>(string2)));
}

TEST_F(EtsStringTest, GetDataMUtf8)
{
    std::vector<char> data = {'a', 'b', 'c', 'd', 'e', 0};

    EtsString *string1 = EtsString::CreateFromMUtf8(data.data());
    ASSERT_FALSE(string1->IsUtf16());

    std::vector<uint8_t> data2(string1->GetDataMUtf8(), string1->GetDataMUtf8() + data.size() - 1);  // NOLINT
    data2.push_back(0);  // GetDataMUtf8 return array without 0

    EtsString *string2 = EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(data2.data()));
    ASSERT_FALSE(string2->IsUtf16());

    ASSERT_TRUE(string1->StringsAreEqual(reinterpret_cast<EtsObject *>(string2)));
}

TEST_F(EtsStringTest, GetMutf8)
{
    std::vector<char> data = {'h', 'e', 'l', 'l', 'o', 0};

    EtsString *string = EtsString::CreateFromMUtf8(data.data());
    PandaString pandaString = string->GetMutf8();

    ASSERT_EQ(strcmp(pandaString.data(), "hello"), 0);
}

TEST_F(EtsStringTest, ConvertToStringView)
{
    PandaVector<uint8_t> buf;
    auto emptyString = EtsString::CreateNewEmptyString();
    auto emptyStringView = emptyString->ConvertToStringView(&buf);
    ASSERT_EQ(emptyStringView, "");

    std::vector<uint8_t> data1 {0, 'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!'};
    std::vector<uint8_t> data2 {'H', 'e', 'l', 'l', 'o', 0, 'w', 'o', 'r', 'l', 'd', '!'};
    std::vector<uint8_t> data3 {'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!', 0};
    auto testString1 = EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(data1.data()));
    auto testString2 = EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(data2.data()));
    auto testString3 = EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(data3.data()));

    auto testString1View = testString1->ConvertToStringView(&buf);
    ASSERT_EQ(testString1View, "");
    auto testString2View = testString2->ConvertToStringView(&buf);
    ASSERT_EQ(testString2View, "Hello");
    auto testString3View = testString3->ConvertToStringView(&buf);
    ASSERT_EQ(testString3View, "Helloworld!");
}

TEST_F(EtsStringTest, Resolve)
{
    std::vector<uint8_t> data {'H', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!', 0};

    EtsString *string1 = EtsString::Resolve(data.data(), data.size() - 1);
    EtsString *string2 = EtsString::Resolve(data.data(), data.size() - 1);
    EtsString *string3 = EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(data.data()));

    ASSERT_EQ(string1, string2);
    ASSERT_TRUE(string1->StringsAreEqual(reinterpret_cast<EtsObject *>(string3)));
    ASSERT_TRUE(string2->StringsAreEqual(reinterpret_cast<EtsObject *>(string3)));
}

void CheckConvertString(EtsString *str)
{
    const std::vector<uint16_t> referenceUtf16 {'a', '\0', 'b', 0x20AC, 'z', '\n'};
    const std::vector<uint8_t> referenceUtf8 {'a', '\0', 'b', 0xe2, 0x82, 0xac, 'z', '\n'};
    const std::vector<uint8_t> referenceMUtf8 {'a', 0xc0, 0x80, 'b', 0xe2, 0x82, 0xac, 'z', '\n'};

    PandaString utf8 = str->GetUtf8();
    PandaString mutf8 = str->GetMutf8();
    ASSERT_EQ(utf8.size(), referenceUtf8.size());
    ASSERT_EQ(mutf8.size(), referenceMUtf8.size());

    for (size_t i = 0; i < referenceUtf8.size(); ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT_EQ(static_cast<uint8_t>(utf8[i]), referenceUtf8[i]);
    }
    for (size_t i = 0; i < referenceMUtf8.size(); ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT_EQ(static_cast<uint8_t>(mutf8[i]), referenceMUtf8[i]);
    }

    auto utf16Data = str->GetDataUtf16();
    auto utf16Len = str->GetUtf16Length();
    ASSERT_EQ(utf16Len, referenceUtf16.size());

    for (size_t i = 0; i < utf16Len; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT_EQ(utf16Data[i], referenceUtf16[i]);
    }
}

TEST_F(EtsStringTest, ConvertFromUtf16)
{
    std::vector<uint16_t> data {'a', '\0', 'b', 0x20AC, 'z', '\n'};
    EtsString *str = EtsString::CreateFromUtf16(data.data(), data.size());
    CheckConvertString(str);
}

TEST_F(EtsStringTest, ConvertFromUtf8)
{
    std::vector<uint8_t> data {'a', '\0', 'b', 0xe2, 0x82, 0xac, 'z', '\n'};
    auto *utf8Data = reinterpret_cast<const char *>(data.data());
    EtsString *str = EtsString::CreateFromUtf8(utf8Data, data.size());
    CheckConvertString(str);
}

TEST_F(EtsStringTest, CreateCompressedString)
{
    std::vector<uint8_t> asciiData = {'h', 'e', 'l', 'l', 'o', 0};
    EXPECT_TRUE(ark::coretypes::String::CanBeCompressedMUtf8(asciiData.data()));

    std::vector<uint8_t> extendedData = {'h', 'e', 'l', 'l', 0xeb, 0};
    EXPECT_FALSE(ark::coretypes::String::CanBeCompressedMUtf8(extendedData.data()));

    EtsString *compressibleString =
        CreateString("aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890./!?-+_:*'\"&@#$%^()=[]{}` ~");
    EXPECT_TRUE(compressibleString->IsUtf8());

    EtsString *fromUtf16String = CreateString({'i', 'k', 'I'});
    EXPECT_TRUE(fromUtf16String->IsUtf8());
}

TEST_F(EtsStringTest, CreateUncomressedString)
{
    EtsString *dottedIString = CreateString({'i', 0x0307});  // i (this is not the same as `i`!)
    EXPECT_FALSE(dottedIString->IsUtf8());
    EXPECT_TRUE(dottedIString->IsUtf16());

    // Only chars with codes from 0x01 to 0x7F is allowed for using in compressed strings, see the
    // common::BaseString::IsASCIICharacter function from
    // runtime_core/common_interfaces/objects/string/base_string-inl.h
    EtsString *fromLatin1Suppl = CreateString({0xcc, 0xcd});  // ÌÍ
    EXPECT_FALSE(fromLatin1Suppl->IsUtf8());
    EXPECT_TRUE(fromLatin1Suppl->IsUtf16());
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
