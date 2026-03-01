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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_ASYNC_STACK_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_ASYNC_STACK_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsAsynStackTest : public TestActions {
public:
    JsAsynStackTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // open stack trace
            {SocketAction::SEND, "setAsyncStackDepth"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load promise.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 21"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 24"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 32"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 35"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 50"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "promise.js 69"},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvAsyncMessageInfo(recv, 20, 19); }},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvAsyncMessageInfo(recv, 23, 22); }},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvAsyncMessageInfo(recv, 31, 30); }},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvAsyncMessageInfo(recv, 34, 33); }},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvAsyncMessageInfo(recv, 68, 61); }},
            // hit breakpoint
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // promise2.then -> promise3.then
            // 46: promise2.then
            // 48: promise3.then
            // 49: current location
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvMultipleAsyncMessageInfo(recv, 49, 48, 46); }},
            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvAsyncMessageInfo(std::string recv, int line, int asyncLine)
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
            std::cout << breakpoint << std::endl;
            LOG_DEBUGGER(ERROR) << breakpoint;
            return false;
        }

        std::unique_ptr<PtJson> asyncStackTrace = nullptr;
        ret = params->GetObject("asyncStackTrace", &asyncStackTrace);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> callFrames = nullptr;
        ret = asyncStackTrace->GetArray("callFrames", &callFrames);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> callFrame = nullptr;
        callFrame = callFrames->Get(0);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int lineNumber = 0;
        callFrame->GetInt("lineNumber", &lineNumber);
        if (ret != Result::SUCCESS) {
            return false;
        }
        std::string sourceFile;
        ret = callFrame->GetString("url", &sourceFile);
        if (ret != Result::SUCCESS) {
            return false;
        }
        if (ret != Result::SUCCESS || sourceFile.find(sourceFile_) == std::string::npos || lineNumber != asyncLine) {
            return false;
        }

        std::unique_ptr<PtJson> asyncParent = nullptr;

        return true;
    }

    bool RecvMultipleAsyncMessageInfo(std::string recv, int line, int asyncLine, int asyncParentLine)
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
            std::cout << breakpoint << std::endl;
            LOG_DEBUGGER(ERROR) << breakpoint;
            return false;
        }

        // test promise3.then
        std::unique_ptr<PtJson> asyncStackTrace = nullptr;
        ret = params->GetObject("asyncStackTrace", &asyncStackTrace);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> callFrames = nullptr;
        ret = asyncStackTrace->GetArray("callFrames", &callFrames);
        if (ret != Result::SUCCESS) {
            return false;
        }

        ret = asyncStackTrace->GetArray("callFrames", &callFrames);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> callFrame = nullptr;
        callFrame = callFrames->Get(0);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int lineNumber = 0;
        ret = callFrame->GetInt("lineNumber", &lineNumber);
        if (ret != Result::SUCCESS) {
            return false;
        }
        std::string sourceFile;
        ret = callFrame->GetString("url", &sourceFile);
        if (ret != Result::SUCCESS) {
            return false;
        }

        if (ret != Result::SUCCESS || sourceFile.find(sourceFile_) == std::string::npos ||
            lineNumber != asyncLine) {
            return false;
        }

        // test promise2.then
        std::unique_ptr<PtJson> asyncParent = nullptr;
        ret = asyncStackTrace->GetObject("parent", &asyncParent);
        if (ret != Result::SUCCESS) {
            return false;
        }

        ret = asyncParent->GetArray("callFrames", &callFrames);
        if (ret != Result::SUCCESS) {
            return false;
        }

        callFrame = callFrames->Get(0);
        if (ret != Result::SUCCESS) {
            return false;
        }

        ret = callFrame->GetInt("lineNumber", &lineNumber);
        if (ret != Result::SUCCESS) {
            return false;
        }

        ret = callFrame->GetString("url", &sourceFile);
        if (ret != Result::SUCCESS) {
            return false;
        }

        if (ret != Result::SUCCESS || sourceFile.find(sourceFile_) == std::string::npos ||
            lineNumber != asyncParentLine) {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsAsynStackTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "promise.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "promise.js";
    std::string entryPoint_ = "promise";
};

std::unique_ptr<TestActions> GetJsAsynStackTest()
{
    return std::make_unique<JsAsynStackTest>();
}
} // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_ASYNC_STACK_TEST_H
