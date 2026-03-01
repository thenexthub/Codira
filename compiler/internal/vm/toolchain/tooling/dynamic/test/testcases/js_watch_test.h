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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsWatchTest : public TestActions {
public:
    JsWatchTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load sample.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "sample.js 22"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // hit breakpoint after resume second time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [] (auto, auto, auto) -> bool {
                return true; }, TestCase::WATCH},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return RecvWatchDescription(recv);
            }},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return RecvWatchType(recv);
            }},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "sample.js 33"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [] (auto, auto, auto) -> bool {
                return true;
            }, TestCase::WATCH},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return RecvWatchType(recv);
            }},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return RecvWatchObject(recv);
            }, TestCase::WATCH_OBJECT},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return RecvWatchDescriptionByJack(recv);
            }},
            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvWatchDescription(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        int id;
        ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> watchResult = nullptr;
        ret = result->GetObject("result", &watchResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string numResult;
        ret = watchResult->GetString("description", &numResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        if (numResult != "5") {
            return false;
        }
        return true;
    }

    bool RecvWatchType(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        int id;
        ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> watchResult = nullptr;
        ret = result->GetObject("result", &watchResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string type;
        ret = watchResult->GetString("type", &type);
        if (ret != Result::SUCCESS) {
            return false;
        }

        if (type != "undefined") {
            return false;
        }
        return true;
    }

    bool RecvWatchObject(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        int id;
        ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> watchResult = nullptr;
        ret = result->GetObject("result", &watchResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string type;
        ret = watchResult->GetString("type", &type);
        if (ret != Result::SUCCESS) {
            return false;
        }

        if (type != "object") {
            return false;
        }
        return true;
    }

    bool RecvWatchDescriptionByJack(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> watchResult = nullptr;
        ret = result->GetArray("result", &watchResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> value = nullptr;
        ret = watchResult->Get(0)->GetObject("value", &value);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string description;
        ret = value->GetString("description", &description);
        if (ret != Result::SUCCESS) {
            return false;
        }
        if (description != "jack") {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsWatchTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "sample.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "sample.js";
    std::string entryPoint_ = "sample";
};

std::unique_ptr<TestActions> GetJsWatchTest()
{
    return std::make_unique<JsWatchTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_TEST_H
