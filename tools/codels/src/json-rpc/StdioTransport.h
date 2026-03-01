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

#ifndef LSPSERVER_STDIOTRANSPORT_H
#define LSPSERVER_STDIOTRANSPORT_H

#include <iostream>
#include <sstream>

#include "Transport.h"

namespace ark {
constexpr int MAX_MESSAGE_LENGTH = 30;

class StdioTransport : public Transport {
public:
    static StdioTransport& Instance()
    {
        static StdioTransport instance {};
        return instance;
    }

    void SetIO(std::FILE *in, std::FILE *out) override ;

    void Notify(std::string method, ValueOrError params) override ;

    void Reply(nlohmann::json id, ValueOrError result) override ;

    LSPRet Loop(MessageHandler &handler) override ;

    std::mutex stdoutMutex {};

    ~StdioTransport() override {}
private:
    StdioTransport(): pFileIn(nullptr), pFileOut(nullptr)  {}
    StdioTransport(const StdioTransport &);
    const StdioTransport &operator=(const StdioTransport &);

    LSPRet HandleMessage(nlohmann::json message, MessageHandler &handler);

    void SendMsg(const nlohmann::json &message);

    std::string ReadRawMessage();

    std::string ReadStandardMessage();

    std::FILE *pFileIn = nullptr;
    std::FILE *pFileOut = nullptr;
};
}
#endif // LSPSERVER_STDIOTRANSPORT_H
