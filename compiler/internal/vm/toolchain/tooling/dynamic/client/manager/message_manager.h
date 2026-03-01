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

#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_MESSAGE_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_MESSAGE_MANAGER_H

#include <iostream>

namespace OHOS::ArkCompiler::Toolchain {
class MessageManager final {
public:
    ~MessageManager()
    {
        uv_mutex_destroy(&messageMutex_);
    }

    static MessageManager& getInstance()
    {
        static MessageManager instance;
        return instance;
    }

    void MessagePush(uint32_t sessionId, std::string& message)
    {
        uv_mutex_lock(&messageMutex_);
        messageQue_.push(std::make_pair(sessionId, message));
        uv_mutex_unlock(&messageMutex_);
    }

    bool MessagePop(uint32_t& sessionId, std::string& message)
    {
        uv_mutex_lock(&messageMutex_);
        if (messageQue_.empty()) {
            uv_mutex_unlock(&messageMutex_);
            return false;
        }

        sessionId = messageQue_.front().first;
        message = messageQue_.front().second;
        messageQue_.pop();

        uv_mutex_unlock(&messageMutex_);
        return true;
    }

private:
    MessageManager()
    {
        uv_mutex_init(&messageMutex_);
    }
    MessageManager(const MessageManager&) = delete;
    MessageManager& operator=(const MessageManager&) = delete;

    uv_mutex_t messageMutex_;
    std::queue<std::pair<uint32_t, std::string>> messageQue_;
};
} // OHOS::ArkCompiler::Toolchain
#endif