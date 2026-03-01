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

#include "gtest/gtest.h"
#include "frame_builder.h"

using namespace OHOS::ArkCompiler::Toolchain;

namespace panda::test {
class FrameBuilderTest : public testing::Test {
public:
    // final message, ping-frame opcode
    static constexpr char PING_EXPECTED_FIRST_BYTE = 0x89;

    static constexpr std::array<uint8_t, WebSocketFrame::MASK_LEN> MASKING_KEY = {0xab};

    static constexpr size_t SHORT_MSG_SIZE = 10;
    static constexpr size_t LONG_MSG_SIZE = 1000;
    static constexpr size_t LONG_LONG_MSG_SIZE = 0xfffff;

    static const std::string SHORT_MSG;
    static const std::string LONG_MSG;
    static const std::string LONG_LONG_MSG;
};

const std::string FrameBuilderTest::SHORT_MSG      = std::string(SHORT_MSG_SIZE, 'f');
const std::string FrameBuilderTest::LONG_MSG       = std::string(LONG_MSG_SIZE, 'f');
const std::string FrameBuilderTest::LONG_LONG_MSG  = std::string(LONG_LONG_MSG_SIZE, 'f');

HWTEST_F(FrameBuilderTest, TestNoPayload, testing::ext::TestSize.Level0)
{
    ServerFrameBuilder frameBuilder(true, FrameType::PING);
    auto message = frameBuilder.Build();

    constexpr size_t EXPECTED_MESSAGE_SIZE = 2;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // unmasked, zero length
    ASSERT_EQ(message[1], 0);
}

HWTEST_F(FrameBuilderTest, TestShortPayload, testing::ext::TestSize.Level0)
{
    ServerFrameBuilder frameBuilder(true, FrameType::PING);
    auto message = frameBuilder
        .SetPayload(SHORT_MSG)
        .Build();

    constexpr size_t HEADER_LENGTH = 2;
    constexpr size_t EXPECTED_MESSAGE_SIZE = HEADER_LENGTH + SHORT_MSG_SIZE;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // length fits into [0, 126) range
    ASSERT_EQ(message[1], static_cast<char>(SHORT_MSG_SIZE));
    for (size_t i = HEADER_LENGTH; i < message.size(); ++i) {
        ASSERT_EQ(message[i], SHORT_MSG[i - HEADER_LENGTH]);
    }
}

HWTEST_F(FrameBuilderTest, TestLongPayload, testing::ext::TestSize.Level0)
{
    ServerFrameBuilder frameBuilder(true, FrameType::PING);
    auto message = frameBuilder
        .SetPayload(LONG_MSG)
        .Build();

    constexpr size_t HEADER_LENGTH = 2 + 2;
    constexpr size_t EXPECTED_MESSAGE_SIZE = HEADER_LENGTH + LONG_MSG_SIZE;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // length fits into [125, 65536) range - encoded with 126
    ASSERT_EQ(message[1], 126);
    // everything is encoded as big-endian
    ASSERT_EQ(message[2], static_cast<char>((LONG_MSG_SIZE >> 8) & 0xff));
    ASSERT_EQ(message[3], static_cast<char>(LONG_MSG_SIZE & 0xff));
    for (size_t i = HEADER_LENGTH; i < message.size(); ++i) {
        ASSERT_EQ(message[i], LONG_MSG[i - HEADER_LENGTH]);
    }
}

HWTEST_F(FrameBuilderTest, TestLongLongPayload, testing::ext::TestSize.Level0)
{
    ServerFrameBuilder frameBuilder(true, FrameType::PING);
    auto message = frameBuilder
        .SetPayload(LONG_LONG_MSG)
        .Build();

    constexpr size_t HEADER_LENGTH = 2 + 8;
    constexpr size_t EXPECTED_MESSAGE_SIZE = HEADER_LENGTH + LONG_LONG_MSG_SIZE;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // length is bigger than 65536 - encoded with 127
    ASSERT_EQ(message[1], 127);
    // everything is encoded as big-endian
    for (size_t idx = 2, shiftCount = 8 * (sizeof(uint64_t) - 1); idx < HEADER_LENGTH; ++idx, shiftCount -= 8) {
        ASSERT_EQ(message[idx], static_cast<char>((LONG_LONG_MSG_SIZE >> shiftCount) & 0xff));
    }
    for (size_t i = HEADER_LENGTH; i < message.size(); ++i) {
        ASSERT_EQ(message[i], LONG_LONG_MSG[i - HEADER_LENGTH]);
    }
}

HWTEST_F(FrameBuilderTest, TestAppendPayload, testing::ext::TestSize.Level0)
{
    ServerFrameBuilder frameBuilder(true, FrameType::PING);
    auto message = frameBuilder
        .SetPayload(SHORT_MSG)
        .AppendPayload(SHORT_MSG)
        .Build();

    constexpr size_t HEADER_LENGTH = 2;
    constexpr size_t PAYLOAD_SIZE = SHORT_MSG_SIZE * 2;
    constexpr size_t EXPECTED_MESSAGE_SIZE = HEADER_LENGTH + PAYLOAD_SIZE;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // length fits into [0, 126) range
    ASSERT_EQ(message[1], static_cast<char>(PAYLOAD_SIZE));
    for (size_t i = HEADER_LENGTH; i < message.size(); ++i) {
        ASSERT_EQ(message[i], SHORT_MSG[(i - HEADER_LENGTH) % SHORT_MSG_SIZE]);
    }
}

HWTEST_F(FrameBuilderTest, TestClientNoPayload, testing::ext::TestSize.Level0)
{
    ClientFrameBuilder frameBuilder(true, FrameType::PING, MASKING_KEY);
    auto message = frameBuilder.Build();

    constexpr size_t EXPECTED_MESSAGE_SIZE = 2 + WebSocketFrame::MASK_LEN;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // masked, even if no payload provided
    ASSERT_EQ(message[1], static_cast<char>(0x80));
}

HWTEST_F(FrameBuilderTest, TestClientMasking, testing::ext::TestSize.Level0)
{
    ClientFrameBuilder frameBuilder(true, FrameType::PING, MASKING_KEY);
    auto message = frameBuilder
        .SetPayload(LONG_MSG)
        .Build();

    constexpr size_t HEADER_LENGTH = 2 + 2 + WebSocketFrame::MASK_LEN;
    constexpr size_t EXPECTED_MESSAGE_SIZE = HEADER_LENGTH + LONG_MSG_SIZE;

    ASSERT_EQ(message.size(), EXPECTED_MESSAGE_SIZE);
    ASSERT_EQ(message[0], PING_EXPECTED_FIRST_BYTE);
    // masked, length fits into [125, 65536) range - encoded with 126
    ASSERT_EQ(message[1], static_cast<char>(0x80 | 126));
    // everything is encoded as big-endian
    ASSERT_EQ(message[2], static_cast<char>((LONG_MSG_SIZE >> 8) & 0xff));
    ASSERT_EQ(message[3], static_cast<char>(LONG_MSG_SIZE & 0xff));
    // message must be masked
    for (size_t i = HEADER_LENGTH; i < message.size(); ++i) {
        ASSERT_EQ(static_cast<uint8_t>(message[i] ^ MASKING_KEY[i % WebSocketFrame::MASK_LEN]),
                  static_cast<uint8_t>(LONG_MSG[i - HEADER_LENGTH]));
    }
}
}  // namespace panda::test
