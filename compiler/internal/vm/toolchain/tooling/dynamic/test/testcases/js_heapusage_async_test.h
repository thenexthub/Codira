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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_HEAPUSAGE_ASYNC_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_HEAPUSAGE_ASYNC_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsHeapusageAsyncTest : public TestActions {
public:
    JsHeapusageAsyncTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load async_func.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "cpuprofile-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "async_func.js 18"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "heapusage"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvHeapusageInfo(recv); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "heapusage"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvHeapusageInfo(recv); }},

            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvHeapusageInfo(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        int id = 0;
        ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int usedSize = 0;
        ret = result->GetInt("usedSize", &usedSize);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int totalSize = 0;
        ret = result->GetInt("totalSize", &totalSize);
        if (ret != Result::SUCCESS) {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsHeapusageAsyncTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "async_func.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "async_func.js";
    std::string entryPoint_ = "async_func";
};

std::unique_ptr<TestActions> GetJsHeapusageAsyncTest()
{
    return std::make_unique<JsHeapusageAsyncTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_HEAPUSAGE_ASYNC_TEST_H
