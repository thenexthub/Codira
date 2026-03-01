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

#include "frame_builder.h"

namespace OHOS::ArkCompiler::Toolchain {
ServerFrameBuilder& ServerFrameBuilder::SetFinal(bool fin)
{
    fin_ = fin;
    return *this;
}

ServerFrameBuilder& ServerFrameBuilder::SetOpcode(FrameType opcode)
{
    opcode_ = opcode;
    return *this;
}

ServerFrameBuilder& ServerFrameBuilder::SetPayload(const std::string& payload)
{
    payload_ = payload;
    return *this;
}

ServerFrameBuilder& ServerFrameBuilder::SetPayload(std::string&& payload)
{
    payload_ = std::move(payload);
    return *this;
}

ServerFrameBuilder& ServerFrameBuilder::AppendPayload(const std::string& payload)
{
    payload_.append(payload);
    return *this;
}

std::string ServerFrameBuilder::Build() const
{
    std::string message;
    PushFullHeader(message, 0);
    PushPayload(message);
    return message;
}

void ServerFrameBuilder::PushFullHeader(std::string& message, size_t additionalReservedMem) const
{
    auto headerBytes = WebSocketFrame::HEADER_LEN;
    auto payloadBytes = payload_.size();
    uint8_t payloadLenField = 0;

    if (payloadBytes <= WebSocketFrame::ONE_BYTE_LENTH_ENC_LIMIT) {
        payloadLenField = static_cast<uint8_t>(payloadBytes);
    } else if (payloadBytes < WebSocketFrame::TWO_BYTES_LENGTH_LIMIT) {
        payloadLenField = WebSocketFrame::TWO_BYTES_LENTH_ENC;
        headerBytes += WebSocketFrame::TWO_BYTES_LENTH;
    } else {
        payloadLenField = WebSocketFrame::EIGHT_BYTES_LENTH_ENC;
        headerBytes += WebSocketFrame::EIGHT_BYTES_LENTH;
    }

    message.reserve(headerBytes + payloadBytes + additionalReservedMem);
    PushHeader(message, payloadLenField);
    PushPayloadLength(message, payloadLenField);
}

void ServerFrameBuilder::PushHeader(std::string& message, uint8_t payloadLenField) const
{
    uint8_t byte = EnumToNumber(opcode_);
    if (fin_) {
        byte |= 0x80;
    }
    message.push_back(byte);

    // A server MUST NOT mask any frames that it sends to the client,
    // hence mask bit must be set to zero (see https://www.rfc-editor.org/rfc/rfc6455#section-5.1)
    byte = payloadLenField & 0x7f;
    message.push_back(byte);
}

void ServerFrameBuilder::PushPayloadLength(std::string& message, uint8_t payloadLenField) const
{
    uint64_t payloadLen = payload_.size();
    if (payloadLenField == WebSocketFrame::TWO_BYTES_LENTH_ENC) {
        PushNumberPerByte(message, static_cast<uint16_t>(payloadLen));
    } else if (payloadLenField == WebSocketFrame::EIGHT_BYTES_LENTH_ENC) {
        PushNumberPerByte(message, payloadLen);
    }
}

void ServerFrameBuilder::PushPayload(std::string& message) const
{
    message.append(payload_);
}

ClientFrameBuilder::ClientFrameBuilder(bool final, FrameType opcode,
    const std::array<uint8_t, WebSocketFrame::MASK_LEN>& maskingKey)
    : ServerFrameBuilder(final, opcode)
{
    SetMask(maskingKey);
}

ClientFrameBuilder& ClientFrameBuilder::SetMask(const std::array<uint8_t, WebSocketFrame::MASK_LEN>& maskingKey)
{
    for (size_t i = 0; i < WebSocketFrame::MASK_LEN; ++i) {
        maskingKey_[i] = maskingKey[i];
    }
    return *this;
}

void ClientFrameBuilder::PushFullHeader(std::string& message, size_t additionalReservedMem) const
{
    // reserve additional 4 bytes for mask
    ServerFrameBuilder::PushFullHeader(message, additionalReservedMem + WebSocketFrame::MASK_LEN);
    // If the data is being sent by the client, the frame(s) MUST be masked
    // (see https://www.rfc-editor.org/rfc/rfc6455#section-6.1)
    message[1] |= 0x80;
    PushMask(message);
}

void ClientFrameBuilder::PushPayload(std::string& message) const
{
    // push masked payload
    for (size_t i = 0, end = payload_.size(); i < end; ++i) {
        char c = payload_[i] ^ maskingKey_[i % WebSocketFrame::MASK_LEN];
        message.push_back(c);
    }
}

void ClientFrameBuilder::PushMask(std::string& message) const
{
    for (size_t i = 0; i < WebSocketFrame::MASK_LEN; ++i) {
        message.push_back(static_cast<char>(maskingKey_[i]));
    }
}
} // OHOS::ArkCompiler::Toolchain
