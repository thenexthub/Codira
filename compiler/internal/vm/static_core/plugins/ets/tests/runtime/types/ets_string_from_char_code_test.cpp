/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "ets_coroutine.h"
#include "intrinsics.h"
#include "entrypoints_gen.h"
#include "types/ets_box_primitive.h"
#include "types/ets_box_primitive-inl.h"
#include "types/ets_array.h"
#include "types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_string_helpers.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {
class BasicEtsStringFromCharCodeTest : public testing::Test {
public:
    BasicEtsStringFromCharCodeTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~BasicEtsStringFromCharCodeTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(BasicEtsStringFromCharCodeTest);
    NO_MOVE_SEMANTIC(BasicEtsStringFromCharCodeTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    template <typename DoubleIter>
    EtsObjectArray *CreateCharCodeArray(DoubleIter first, DoubleIter last)
    {
        using CharCode = EtsBoxPrimitive<EtsDouble>;
        EtsClass *klass = CharCode::GetEtsBoxClass(coroutine_);
        ASSERT(klass != nullptr);
        auto length = std::distance(first, last);
        EtsObjectArray *charCodeArray = EtsObjectArray::Create(klass, length);
        std::for_each(first, last, [&charCodeArray, this, idx = 0U](double d) mutable {
            auto *boxedValue = CharCode::Create(coroutine_, d);
            charCodeArray->Set(idx++, boxedValue);
        });
        return charCodeArray;
    }

    EtsEscompatArray *CreateEtsEscompatArray(const std::vector<double> &codes)
    {
        // Allocate and create the buffer
        auto *buffer = CreateCharCodeArray(codes.begin(), codes.end());
        // Allocate the array object
        auto *array = EtsEscompatArray::Create(coroutine_, codes.size());
        // Fill the array with the pre-created buffer
        ObjectAccessor::SetObject(coroutine_, array, EtsEscompatArray::GetBufferOffset(), buffer->GetCoreType());
        return array;
    }

    template <typename DoubleIter>
    EtsString *CreateNewStringFromCharCodes(DoubleIter first, DoubleIter last)
    {
        EtsObjectArray *charCodeArray = CreateCharCodeArray<DoubleIter>(first, last);
        return intrinsics::helpers::CreateNewStringFromCharCode(charCodeArray, std::distance(first, last));
    }

    EtsString *CreateNewStringFromCharCodes(const std::vector<double> &codes)
    {
        return CreateNewStringFromCharCodes(codes.begin(), codes.end());
    }

    static EtsString *CreateNewStringFromCharCode(double code)
    {
        return EtsString::CreateNewStringFromCharCode(code);
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

class EtsStringFromCharCodeTest : public BasicEtsStringFromCharCodeTest {};

TEST_F(EtsStringFromCharCodeTest, CreateNewCompressedStringFromCharCodes)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("Helloff\n");
    EtsString *stringFromCompressedCharCodes =
        CreateNewStringFromCharCodes({0x48, 0x65, 0x6C, 0x6C, 0x6F, 4294901862, 0xffff0066, 10.316});
    EXPECT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewCompressedStringFromCharCode)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("A");
    EtsString *stringFromCompressedCharCodes = CreateNewStringFromCharCodes({0x41});
    EXPECT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));

    EtsString *stringFromCompressedCharCode = CreateNewStringFromCharCode(0x41);
    EXPECT_TRUE(stringFromCompressedCharCode->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewUncompressedStringFromCharCode)
{
    std::vector<uint16_t> data = {0x3B2};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromUncompressedCharCodes = CreateNewStringFromCharCodes({0x3B2});
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(0x3B2);
    EXPECT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewUncompressedStringFromCharCodes)
{
    std::vector<uint16_t> data = {0x3B2, 'A', 'B', 'C', 'D', 0xac, 0xff9c, 0, 0xffff, 1, 0xffff, 0, 0, 0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    std::vector<double> charCodes {0x3B2,
                                   0x41,
                                   66.3,
                                   67.00009,
                                   68.99998,
                                   172.9999,
                                   -100,
                                   static_cast<double>(0x7fffffffffffffff),
                                   static_cast<double>(0x1fffffffffffff),
                                   static_cast<double>(-0x1fffffffffffff),
                                   static_cast<double>(0xffff),
                                   static_cast<double>(0x10000),
                                   static_cast<double>(0x8000000000000000),
                                   0};
    EtsString *stringFromUncompressedCharCodes = CreateNewStringFromCharCodes(charCodes);
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewEmptyStringFromCharCode)
{
    EtsString *emptyString = EtsString::CreateNewEmptyString();
    EtsString *stringFromCharCodes = CreateNewStringFromCharCodes({});
    EXPECT_TRUE(stringFromCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(emptyString->GetCoreType(), stringFromCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromNaNCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::quiet_NaN()});
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(std::numeric_limits<double>::quiet_NaN());
    EXPECT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromInfinityCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::infinity()});
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(std::numeric_limits<double>::infinity());
    EXPECT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromNaNAndInfinityCharCodes)
{
    std::vector<uint16_t> data = {0, 0, 0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                                      -std::numeric_limits<double>::infinity()});
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMaxAvailableCharCode)
{
    std::vector<uint16_t> data = {0xffff};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({9007199254740991.0});
    EXPECT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(9007199254740991.0);
    EXPECT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMinAvailableCharCode)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("\x01");
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({-9007199254740991.0});
    EXPECT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(-9007199254740991.0);
    EXPECT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({18446744073709551616.0});
    EXPECT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(18446744073709551616.0);
    EXPECT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));

    EtsString *stringFromHugeCharCodes2 = CreateNewStringFromCharCodes({18446744073709551617.0});
    EXPECT_TRUE(stringFromHugeCharCodes2->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes2->GetCoreType()));

    EtsString *stringFromHugeCharCode2 = CreateNewStringFromCharCode(18446744073709551617.0);
    EXPECT_TRUE(stringFromHugeCharCode2->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode2->GetCoreType()));

    EtsString *stringFromHugeCharCodes3 = CreateNewStringFromCharCodes({9007199254740992.0});
    EXPECT_TRUE(stringFromHugeCharCodes3->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes3->GetCoreType()));

    EtsString *stringFromHugeCharCode3 = CreateNewStringFromCharCode(9007199254740992.0);
    EXPECT_TRUE(stringFromHugeCharCode3->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode3->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeNegativeCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({-18446744073709551616.0});
    EXPECT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(-18446744073709551616.0);
    EXPECT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));

    EtsString *stringFromHugeCharCodes2 = CreateNewStringFromCharCodes({-18446744073709551617.0});
    EXPECT_TRUE(stringFromHugeCharCodes2->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes2->GetCoreType()));

    EtsString *stringFromHugeCharCode2 = CreateNewStringFromCharCode(-18446744073709551617.0);
    EXPECT_TRUE(stringFromHugeCharCode2->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode2->GetCoreType()));

    EtsString *stringFromHugeCharCodes3 = CreateNewStringFromCharCodes({-9007199254740992.0});
    EXPECT_TRUE(stringFromHugeCharCodes3->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes3->GetCoreType()));

    EtsString *stringFromHugeCharCode3 = CreateNewStringFromCharCode(-9007199254740992.0);
    EXPECT_TRUE(stringFromHugeCharCode3->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode3->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCodes)
{
    std::vector<uint16_t> data = {0, 0, 0, 0, 0, 0, 0xffff, 0x1};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    std::vector<double> charCodes {18446744073709551616.0,  18446744073709551617.0,  9007199254740992.0,
                                   -18446744073709551616.0, -18446744073709551617.0, -9007199254740992.0,
                                   9007199254740991.0,      -9007199254740991.0};
    EtsString *stringFromHugeCharCodes = CreateNewStringFromCharCodes(charCodes);
    EXPECT_TRUE(stringFromHugeCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes->GetCoreType()));
}

class EtsStringFromCharCodeIntrinsicTest : public BasicEtsStringFromCharCodeTest {};

TEST_F(EtsStringFromCharCodeIntrinsicTest, CreateNewCachedStringFromCharCode)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("A");
    EtsString *stringFromCompressedCharCodes = intrinsics::StdCoreStringFromCharCode(CreateEtsEscompatArray({0x41}));
    EXPECT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));

    EtsString *stringFromCompressedCharCode = intrinsics::StdCoreStringFromCharCodeSingle(0x41);
    EXPECT_TRUE(stringFromCompressedCharCode->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeIntrinsicTest, CreateNewNonCachedStringFromCharCode)
{
    std::vector<uint16_t> data = {0x3B2};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromUncompressedCharCodes = intrinsics::StdCoreStringFromCharCode(CreateEtsEscompatArray({0x3B2}));
    EXPECT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = intrinsics::StdCoreStringFromCharCodeSingle(0x3B2);
    EXPECT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

class EtsStringFromCharCodeEntrypointTest : public BasicEtsStringFromCharCodeTest {};

TEST_F(EtsStringFromCharCodeEntrypointTest, CreateNewCachedStringFromCharCodeNoCacheEntrypoint)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("A");
    coretypes::String *stringFromCompressedCharCode =
        CreateStringFromCharCodeSingleNoCacheEntrypoint(bit_cast<uint64_t>(double {0x41}));
    EXPECT_TRUE(stringFromCompressedCharCode->IsMUtf8());
    EXPECT_TRUE(
        coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(), stringFromCompressedCharCode));
}

TEST_F(EtsStringFromCharCodeEntrypointTest, CreateNewNonCachedStringFromCharCodeNoCacheEntrypoint)
{
    std::vector<uint16_t> data = {0x3B2};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    coretypes::String *stringFromUncompressedCharCode =
        CreateStringFromCharCodeSingleNoCacheEntrypoint(bit_cast<uint64_t>(double {0x3B2}));
    EXPECT_TRUE(stringFromUncompressedCharCode->IsUtf16());
    EXPECT_TRUE(
        coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(), stringFromUncompressedCharCode));
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
