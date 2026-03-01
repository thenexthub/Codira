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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WS_FRAME_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WS_FRAME_H

#include <string>
#include <cstdint>

namespace OHOS::ArkCompiler::Toolchain {
enum class FrameType : uint8_t {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xa,
};

constexpr inline bool IsControlFrame(uint8_t opcode)
{
    return opcode >= static_cast<uint8_t>(FrameType::CLOSE);
}

template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr inline auto EnumToNumber(T type)
{
    using UnderlyingT = std::underlying_type_t<T>;
    return static_cast<UnderlyingT>(type);
}

struct WebSocketFrame {
    static constexpr size_t MASK_LEN = 4;
    static constexpr size_t HEADER_LEN = 2;
    static constexpr size_t ONE_BYTE_LENTH_ENC_LIMIT = 125;
    static constexpr size_t TWO_BYTES_LENTH_ENC = 126;
    static constexpr size_t TWO_BYTES_LENTH = 2;
    static constexpr size_t TWO_BYTES_LENGTH_LIMIT = 65536;
    static constexpr size_t EIGHT_BYTES_LENTH_ENC = 127;
    static constexpr size_t EIGHT_BYTES_LENTH = 8;

    uint64_t payloadLen = 0;
    uint8_t fin = 0;
    uint8_t opcode = 0;
    uint8_t mask = 0;
    uint8_t maskingKey[MASK_LEN] = {0};
    std::string payload;

    WebSocketFrame() = default;
    explicit WebSocketFrame(const uint8_t headerRaw[HEADER_LEN])
        : payloadLen(static_cast<uint64_t>(headerRaw[1]) & 0x7f),
          fin(static_cast<uint8_t>((headerRaw[0] >> MSB_SHIFT_COUNT) & 0x1)),
          opcode(static_cast<uint8_t>(headerRaw[0] & 0xf)),
          mask(static_cast<uint8_t>((headerRaw[1] >> MSB_SHIFT_COUNT) & 0x1))
    {
    }

private:
    static constexpr int MSB_SHIFT_COUNT = 7;
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WS_FRAME_H
