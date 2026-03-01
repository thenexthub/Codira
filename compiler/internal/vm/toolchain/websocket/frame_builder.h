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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_FRAME_BUILDER_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_FRAME_BUILDER_H

#include "web_socket_frame.h"

#include <string>
#include <array>

namespace OHOS::ArkCompiler::Toolchain {
template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
inline void PushNumberPerByte(std::string& message, T number)
{
    constexpr size_t bytesCount = sizeof(T);
    constexpr size_t bitsCount = 8;
    size_t shiftCount = (bytesCount - 1) * bitsCount;
    for (size_t i = 0; i < bytesCount; ++i, shiftCount -= bitsCount) {
        message.push_back((number >> shiftCount) & 0xff);
    }
}

class ServerFrameBuilder {
public:
    // force users to specify opcode and final bit
    ServerFrameBuilder() = delete;
    ServerFrameBuilder(bool final, FrameType opcode) : fin_(final), opcode_(opcode)
    {
    }
    virtual ~ServerFrameBuilder() noexcept = default;

    ServerFrameBuilder& SetFinal(bool fin);
    ServerFrameBuilder& SetOpcode(FrameType opcode);
    ServerFrameBuilder& SetPayload(const std::string& payload);
    ServerFrameBuilder& SetPayload(std::string&& payload);
    ServerFrameBuilder& AppendPayload(const std::string& payload);

    std::string Build() const;

protected:
    void PushHeader(std::string& message, uint8_t payloadLenField) const;
    void PushPayloadLength(std::string& message, uint8_t payloadLenField) const;
    virtual void PushFullHeader(std::string& message, size_t additionalReservedMem) const;
    virtual void PushPayload(std::string& message) const;

protected:
    bool fin_;
    FrameType opcode_;
    std::string payload_;
};

class ClientFrameBuilder final : public ServerFrameBuilder {
public:
    ClientFrameBuilder(bool final, FrameType opcode, const std::array<uint8_t, WebSocketFrame::MASK_LEN>& maskingKey);

    ClientFrameBuilder& SetMask(const std::array<uint8_t, WebSocketFrame::MASK_LEN>& maskingKey);

private:
    void PushMask(std::string& message) const;
    void PushFullHeader(std::string& message, size_t additionalReservedMem) const override;
    void PushPayload(std::string& message) const override;

private:
    uint8_t maskingKey_[WebSocketFrame::MASK_LEN] = {0};
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_FRAME_BUILDER_H
