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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_NETWORK_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_NETWORK_H

#include <string>

namespace OHOS::ArkCompiler::Toolchain {
// Receives a message of size `buffer.size()`. Clears the string buffer on error.
bool Recv(int32_t client, std::string& buffer, int32_t flags);

bool Recv(int32_t client, char* buf, size_t totalLen, int32_t flags);

bool Recv(int32_t client, uint8_t* buf, size_t totalLen, int32_t flags);

bool Send(int32_t client, const std::string& message, int32_t flags);

bool Send(int32_t client, const char* buf, size_t totalLen, int32_t flags);

uint64_t NetToHostLongLong(uint8_t* buf, uint32_t len);

constexpr inline size_t GetBase64EncodingLength(size_t inputLength)
{
    size_t paddingOffset = 2;
    // base64-encoding produces padded output of 4 characters for every 3 bytes input
    return ((inputLength + paddingOffset) / 3) * 4;
}
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_NETWORK_H
