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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SPECIAL_LOCATION_BREAKPOINT_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SPECIAL_LOCATION_BREAKPOINT_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsSpecialLocationBreakpointTest : public TestActions {
public:
    JsSpecialLocationBreakpointTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load step.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 16"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakpointInfo(recv, 15); }},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 98"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakpointInfo(recv, 97); }},
            // hit breakpoint on first line
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvHitBreakInfo(recv, 15); }},
            // hit breakpoint on the end of line
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvHitBreakInfo(recv, 97); }},
            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvBreakpointInfo(std::string recv, int line)
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

        std::string breakpointId = "";
        ret = result->GetString("breakpointId", &breakpointId);
        if (ret != Result::SUCCESS || breakpointId.find(sourceFile_) == std::string::npos ||
            breakpointId.find(std::to_string(line)) == std::string::npos) {
            return false;
        }

        std::unique_ptr<PtJson> locations = nullptr;
        ret = result->GetArray("locations", &locations);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int lineNumber = 0;
        ret = locations->Get(0)->GetInt("lineNumber", &lineNumber);
        if (ret != Result::SUCCESS || lineNumber != line) {
            return false;
        }
        return true;
    }

    bool RecvHitBreakInfo(std::string recv, int line)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        std::string method = "";
        ret = json->GetString("method", &method);
        if (ret != Result::SUCCESS || method != "Debugger.paused") {
            return false;
        }

        std::unique_ptr<PtJson> params = nullptr;
        ret = json->GetObject("params", &params);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> hitBreakpoints = nullptr;
        ret = params->GetArray("hitBreakpoints", &hitBreakpoints);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string breakpoint = "";
        breakpoint = hitBreakpoints->Get(0)->GetString();
        if (ret != Result::SUCCESS || breakpoint.find(sourceFile_) == std::string::npos ||
            breakpoint.find(std::to_string(line)) == std::string::npos) {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsSpecialLocationBreakpointTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "step.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "step.js";
    std::string entryPoint_ = "step";
};

std::unique_ptr<TestActions> GetJsSpecialLocationBreakpointTest()
{
    return std::make_unique<JsSpecialLocationBreakpointTest>();
}
} // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SPECIAL_LOCATION_BREAKPOINT_TEST_H
