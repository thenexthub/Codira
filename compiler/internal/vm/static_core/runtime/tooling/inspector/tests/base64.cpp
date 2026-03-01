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

#include <string_view>
#include <utility>

#include "gtest/gtest.h"

#include "evaluation/base64.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {

class Base64DecoderTest : public testing::Test {
public:
    static constexpr std::array TEST_CASES = {
        std::pair<std::string_view, std::string_view> {"bGlnaHQgd29yay4=", "light work."},
        std::pair<std::string_view, std::string_view> {"bGlnaHQgd29yaw==", "light work"},
        std::pair<std::string_view, std::string_view> {"bGlnaHQgd29y", "light wor"},
        std::pair<std::string_view, std::string_view> {"TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu",
                                                       "Many hands make light work."},
    };
};

static void CheckTestCase(std::string_view encoded, std::string_view decoded)
{
    Span encodedData {reinterpret_cast<const uint8_t *>(encoded.data()), encoded.size()};
    auto decodedSize = Base64Decoder::DecodedSize(encodedData);
    ASSERT_TRUE(decodedSize.has_value());
    ASSERT_EQ(*decodedSize, decoded.size());
    std::string decodedBuffer(decoded.size(), 0);
    ASSERT_TRUE(Base64Decoder::Decode(reinterpret_cast<uint8_t *>(decodedBuffer.data()), encodedData));
    ASSERT_EQ(decodedBuffer, decoded);
}

TEST_F(Base64DecoderTest, TestDecoding)
{
    for (auto [encoded, decoded] : TEST_CASES) {
        CheckTestCase(encoded, decoded);
    }
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND
