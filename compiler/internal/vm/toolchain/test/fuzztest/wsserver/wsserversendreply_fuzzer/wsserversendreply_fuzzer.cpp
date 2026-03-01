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

#include "wsserversendreply_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "inspector/ws_server.h"

using namespace panda;
using namespace panda::ecmascript;
using namespace OHOS::ArkCompiler::Toolchain;

namespace OHOS {
    void TestFun([[maybe_unused]]std::string &&message)
    {
        return;
    }
    void WsServerSendReplyFuzzTest(const uint8_t* data, size_t size)
    {
        if (size <= 0) {
            return;
        }
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        std::function<void(std::string&&)> fun = TestFun;
        int32_t instanceId = 10001; // 10001:test instanceId
        int port = 9230; // 9230:connection port for test
        int fd = -2; // -2 : old debug process
        DebugInfo debugInfo = {fd, "toolchain", instanceId, port};
        WsServer wsServer(debugInfo, fun);
        std::string message(data, data + size);
        wsServer.SendReply(message);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::WsServerSendReplyFuzzTest(data, size);
    return 0;
}