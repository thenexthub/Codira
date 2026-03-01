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

#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/base/pt_base64.h"

using namespace panda::ecmascript::tooling;

namespace panda::test {
class PtBase64Test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


HWTEST_F_L0(PtBase64Test, EncodeAndDecodeEmptyString)
{
    std::string src;
    std::string dest;

    dest.resize(PtBase64::DecodedSize(src.size()));
    auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
    dest.resize(numOctets);
    EXPECT_EQ(numOctets, 0u);
    EXPECT_FALSE(done);

    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 0u);
    EXPECT_EQ(dest, "");
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeShortText001)
{
    std::string src;
    std::string dest;
    size_t len = 0;

    {
        src = "1";
        dest.resize(PtBase64::EncodedSize(src.size()));
        len = PtBase64::Encode(dest.data(), src.data(), src.size());
        EXPECT_EQ(len, 4u);
        EXPECT_EQ(dest, "MQ==");
        src = dest;
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 1u);
        EXPECT_EQ(dest, "1");
    }

    {
        src = "12";
        dest.resize(PtBase64::EncodedSize(src.size()));
        len = PtBase64::Encode(dest.data(), src.data(), src.size());
        EXPECT_EQ(len, 4u);
        EXPECT_EQ(dest, "MTI=");
        src = dest;
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 2u);
        EXPECT_EQ(dest, "12");
    }

    {
        src = "123";
        dest.resize(PtBase64::EncodedSize(src.size()));
        len = PtBase64::Encode(dest.data(), src.data(), src.size());
        EXPECT_EQ(len, 4u);
        EXPECT_EQ(dest, "MTIz");
        src = dest;
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 3u);
        EXPECT_EQ(dest, "123");
    }
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeShortText002)
{
    std::string dest;
    std::string src = "hello";
    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 8u);
    EXPECT_EQ(dest, "aGVsbG8=");
    src = dest;
    dest.resize(PtBase64::DecodedSize(src.size()));
    auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
    dest.resize(numOctets);
    EXPECT_EQ(numOctets, 5u);
    EXPECT_TRUE(done);
    EXPECT_EQ(dest, "hello");
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeLongText)
{
    std::string src = "If you can see this message, it means that PtBase64Decode works well";
    std::string dest;
    std::string encoded = "SWYgeW91IGNhbiBzZWUgdGhpcyBtZXNzYWdlLCBpdCBtZWFucyB0aGF0IFB0QmFzZ" \
                          "TY0RGVjb2RlIHdvcmtzIHdlbGw=";

    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 92u);
    EXPECT_EQ(dest, encoded);

    std::string decoded;
    decoded.resize(PtBase64::DecodedSize(encoded.size()));
    auto [numOctets, done] = PtBase64::Decode(decoded.data(), encoded.data(), encoded.size());
    decoded.resize(numOctets);
    EXPECT_EQ(numOctets, 68u);
    EXPECT_TRUE(done);
    EXPECT_EQ(decoded, src);
}

HWTEST_F_L0(PtBase64Test, EncodeUnprintableCharacter)
{
    std::string src = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::string dest;
    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 12u);
    EXPECT_EQ(dest, "AQIDBAUGBwgJ");
}

HWTEST_F_L0(PtBase64Test, DecodeErrorNumberBase64Text)
{
    std::string src;
    std::string dest;
    {
        src = "ABC";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 0u);
        EXPECT_EQ(dest, "");
    }

    {
        src = "ABCDE";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 0u);
        EXPECT_EQ(dest, "");
    }

    {
        src = "ABCDEF";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        dest.resize(numOctets);
        EXPECT_EQ(numOctets, 0u);
        EXPECT_EQ(dest, "");
    }
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeBytesArray)
{
    std::vector<uint8_t> src = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };
    std::string dest;
    std::string encoded = "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4O" \
                          "To7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3Bxcn" \
                          "N0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6y" \
                          "trq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm" \
                          "5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==";
    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 344u);
    EXPECT_EQ(dest, encoded);

    std::vector<uint8_t> decoded;
    decoded.resize(PtBase64::DecodedSize(encoded.size()));
    auto [numOctets, done] = PtBase64::Decode(decoded.data(), encoded.data(), encoded.size());
    decoded.resize(numOctets);
    EXPECT_EQ(numOctets, 256u);
    EXPECT_TRUE(done);
    EXPECT_EQ(memcmp(decoded.data(), src.data(), numOctets), 0);
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeUnicodeCharacter001)
{
    std::string src = u8"😀";
    std::string dest;
    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 8u);
    EXPECT_EQ(dest, "8J+YgA==");
    src = dest;
    dest.resize(PtBase64::DecodedSize(src.size()));
    auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
    dest.resize(numOctets);
    EXPECT_EQ(numOctets, 4u);
    EXPECT_TRUE(done);
    EXPECT_EQ(dest, u8"😀");
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeUnicodeCharacter002)
{
    std::string src = "测试Base64加密解密";
    std::string dest;
    dest.resize(PtBase64::EncodedSize(src.size()));
    size_t len = PtBase64::Encode(dest.data(), src.data(), src.size());
    EXPECT_EQ(len, 32u);
    EXPECT_EQ(dest, "5rWL6K+VQmFzZTY05Yqg5a+G6Kej5a+G");
    src = dest;
    dest.resize(PtBase64::DecodedSize(src.size()));
    auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
    dest.resize(numOctets);
    EXPECT_EQ(numOctets, 24u);
    EXPECT_TRUE(done);
    EXPECT_EQ(dest, "测试Base64加密解密");
}

HWTEST_F_L0(PtBase64Test, EncodeAndDecodeInvalidCharacters)
{
    std::string src;
    std::string dest;
    {
        src = "AB#=";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 1u);
        EXPECT_FALSE(done);
    }

    {
        src = "#AB=";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 0u);
        EXPECT_FALSE(done);
    }

    {
        src = "AB=#";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 1u);
        EXPECT_FALSE(done);
    }

    {
        src = "Zm9vYmFy \r\n\t";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 6u);
        EXPECT_FALSE(done);
    }

    {
        src = "=ABC";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 0u);
        EXPECT_FALSE(done);
    }

    {
        src = "==AB";
        dest.resize(PtBase64::DecodedSize(src.size()));
        auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
        EXPECT_EQ(numOctets, 0u);
        EXPECT_FALSE(done);
    }
}
}