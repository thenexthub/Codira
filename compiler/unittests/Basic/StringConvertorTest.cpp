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

#include <string>
#include <utility>
#include <vector>
#include "Codira/Basic/StringConvertor.h"

#include "gtest/gtest.h"
using namespace Codira;

namespace {
unsigned char g_gbkCharArray[] = {176, 162, 203, 174};            // 阿水 gbk 编码
unsigned char g_utf8CharArray[] = {233, 152, 191, 230, 176, 180}; // 阿水 utf8 编码
unsigned char g_errorCharArray[] = {129};                         // unknow 编码
unsigned char g_error2CharArray[] = {240, 169, 184};              // 𩸽 utf8 编码 丢失最后一位: 189
} // namespace

TEST(StringConvertorTest, GetStringEncoding)
{
    std::string gbkString(reinterpret_cast<const char*>(g_gbkCharArray), sizeof(g_gbkCharArray));
    EXPECT_EQ(StringConvertor::GetStringEncoding(gbkString), Codira::StringConvertor::GBK);
    std::string utf8String(reinterpret_cast<const char*>(g_utf8CharArray), sizeof(g_utf8CharArray));
    EXPECT_EQ(StringConvertor::GetStringEncoding(utf8String), Codira::StringConvertor::UTF8);
    std::string errorString(reinterpret_cast<const char*>(g_errorCharArray), sizeof(g_errorCharArray));
    EXPECT_EQ(StringConvertor::GetStringEncoding(errorString), Codira::StringConvertor::UNKNOWN);
    std::string error2String(reinterpret_cast<const char*>(g_error2CharArray), sizeof(g_error2CharArray));
    EXPECT_EQ(StringConvertor::GetStringEncoding(error2String), Codira::StringConvertor::UNKNOWN);
}

TEST(StringConvertorTest, GBKToUTF8)
{
    std::string gbkString(reinterpret_cast<const char*>(g_gbkCharArray), sizeof(g_gbkCharArray));
    std::string utf8String(reinterpret_cast<const char*>(g_utf8CharArray), sizeof(g_utf8CharArray));
    std::optional<std::string> str = StringConvertor::GBKToUTF8(gbkString);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), utf8String);
}

TEST(StringConvertorTest, UTF8ToGBK)
{
    std::string gbkString(reinterpret_cast<const char*>(g_gbkCharArray), sizeof(g_gbkCharArray));
    std::string utf8String(reinterpret_cast<const char*>(g_utf8CharArray), sizeof(g_utf8CharArray));
    std::optional<std::string> str = StringConvertor::UTF8ToGBK(utf8String);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), gbkString);
}

TEST(StringConvertorTest, NormalizeStringToUTF8)
{
    std::string gbkString(reinterpret_cast<const char*>(g_gbkCharArray), sizeof(g_gbkCharArray));
    std::string utf8String(reinterpret_cast<const char*>(g_utf8CharArray), sizeof(g_utf8CharArray));
    std::optional<std::string> str = StringConvertor::NormalizeStringToUTF8(gbkString);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), utf8String);

    str = StringConvertor::NormalizeStringToUTF8(utf8String);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), utf8String);
}

TEST(StringConvertorTest, NormalizeStringToGBK)
{
    std::string gbkString(reinterpret_cast<const char*>(g_gbkCharArray), sizeof(g_gbkCharArray));
    std::string utf8String(reinterpret_cast<const char*>(g_utf8CharArray), sizeof(g_utf8CharArray));
    std::optional<std::string> str = StringConvertor::NormalizeStringToGBK(gbkString);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), gbkString);

    str = StringConvertor::NormalizeStringToGBK(utf8String);
    EXPECT_EQ(str.has_value(), true);
    EXPECT_EQ(str.value(), gbkString);
}
