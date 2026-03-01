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

#include "connectserversendmessage_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "inspector/connect_server.h"

using namespace panda;
using namespace panda::ecmascript;
using namespace OHOS::ArkCompiler::Toolchain;

namespace OHOS {
    void TestFunction([[maybe_unused]]std::string &&message)
    {
        return;
    }
    void ConnectServerSendMessageFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        if (size <= 0) {
            return;
        }
        std::function<void(std::string&&)> testFunction = TestFunction;
        ConnectServer connectServer("com.example.myapplication", testFunction);
        std::string message(data, data + size);
        connectServer.SendMessage(message);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::ConnectServerSendMessageFuzzTest(data, size);
    return 0;
}