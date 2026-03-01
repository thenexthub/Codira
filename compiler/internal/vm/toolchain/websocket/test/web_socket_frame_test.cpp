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
#include "web_socket_frame.h"

using namespace OHOS::ArkCompiler::Toolchain;

namespace panda::test {
class WebSocketFrameTest : public testing::Test {
public:
    static constexpr uint8_t HEADER_RAW[WebSocketFrame::HEADER_LEN] = {0x01, 0x9a};
    static constexpr uint8_t EXPECTED_FIN           = 0;
    static constexpr uint8_t EXPECTED_OPCODE        = 0x1;
    static constexpr uint8_t EXPECTED_MASK_BIT      = 1;
    static constexpr uint8_t EXPECTED_PAYLOAD_LEN   = 0x1a;
};

HWTEST_F(WebSocketFrameTest, TestDecode, testing::ext::TestSize.Level0)
{
    WebSocketFrame wsFrame(HEADER_RAW);

    ASSERT_EQ(wsFrame.fin, EXPECTED_FIN);
    ASSERT_EQ(wsFrame.opcode, EXPECTED_OPCODE);
    ASSERT_EQ(wsFrame.mask, EXPECTED_MASK_BIT);
    ASSERT_EQ(wsFrame.payloadLen, EXPECTED_PAYLOAD_LEN);
    // these fields must not be filled
    ASSERT_TRUE(wsFrame.payload.empty());
    for (size_t i = 0; i < WebSocketFrame::MASK_LEN; ++i) {
        ASSERT_EQ(wsFrame.maskingKey[i], 0);
    }
}
}  // namespace panda::test
