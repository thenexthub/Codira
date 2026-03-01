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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CPUPROFILE_LOOP_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CPUPROFILE_LOOP_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsCpuprofileLoopTest : public TestActions {
public:
    JsCpuprofileLoopTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load common_func.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "cpuprofile-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "cpuprofile"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "common_func.js 64"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "common_func.js 65"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "cpuprofile-stop"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvCpuprofileInfo(recv); }},

            {SocketAction::SEND, "cpuprofile"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "cpuprofile-stop"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvCpuprofileInfo(recv); }},

            {SocketAction::SEND, "cpuprofile-disable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},

            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvCpuprofileInfo(std::string recv)
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

        std::unique_ptr<PtJson> profile = nullptr;
        ret = result->GetObject("profile", &profile);
        if (ret != Result::SUCCESS) {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsCpuprofileLoopTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "common_func.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "common_func.js";
    std::string entryPoint_ = "common_func";
};

std::unique_ptr<TestActions> GetJsCpuprofileLoopTest()
{
    return std::make_unique<JsCpuprofileLoopTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CPUPROFILE_LOOP_TEST_H
