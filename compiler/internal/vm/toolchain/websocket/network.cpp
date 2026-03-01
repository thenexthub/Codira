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

#include "common/log_wrapper.h"
#include "define.h"
#include "network.h"

namespace OHOS::ArkCompiler::Toolchain {
bool Recv(int32_t client, std::string& buffer, int32_t flags)
{
    if (buffer.empty()) {
        return false;
    }
    auto succeeded = Recv(client, buffer.data(), buffer.size(), flags);
    if (!succeeded) {
        buffer.clear();
    }
    return succeeded;
}

bool Recv(int32_t client, char* buf, size_t totalLen, int32_t flags)
{
    size_t recvLen = 0;
    while (recvLen < totalLen) {
        ssize_t len = 0;
        while ((len = recv(client, buf + recvLen, totalLen - recvLen, flags)) < 0 &&
               (errno == EINTR || errno == EAGAIN)) {
            LOGW("Recv payload failed, errno = %{public}d", errno);
        }
        if (len <= 0) {
            LOGE("Recv payload in while failed, len = %{public}ld, errno = %{public}d", static_cast<long>(len), errno);
            return false;
        }
        recvLen += static_cast<size_t>(len);
    }
    return true;
}

bool Recv(int32_t client, uint8_t* buf, size_t totalLen, int32_t flags)
{
    return Recv(client, reinterpret_cast<char *>(buf), totalLen, flags);
}

bool Send(int32_t client, const std::string& message, int32_t flags)
{
    return Send(client, message.c_str(), message.size(), flags);
}

bool Send(int32_t client, const char* buf, size_t totalLen, int32_t flags)
{
    size_t sendLen = 0;
    while (sendLen < totalLen) {
        ssize_t len = send(client, buf + sendLen, totalLen - sendLen, flags);
        if (len <= 0) {
            LOGE("Send Message in while failed, len = %{public}ld, errno = %{public}d", static_cast<long>(len), errno);
            return false;
        }
        sendLen += static_cast<size_t>(len);
    }
    return true;
}

uint64_t NetToHostLongLong(uint8_t* buf, uint32_t len)
{
    if (buf == nullptr) {
        return 0;
    }
    uint64_t result = 0;
    for (uint32_t i = 0; i < len; i++) {
        result |= buf[i];
        if ((i + 1) < len) {
            result <<= 8; // 8: result need shift left 8 bits in order to big endian convert to int
        }
    }
    return result;
}
} // OHOS::ArkCompiler::Toolchain
